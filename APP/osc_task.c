#include "osc_task.h"
#include "mid_lcd.h"
#include "stdio.h"
#include "string.h"
#include "mid_pwm.h"
#include "mid_timer.h"
#include "mid_adc.h"
#include "hw_key.h"
#include "hw_adc.h"
#include "kanade_img.h"
#include "hw_lcdinit.h"

#define volThreshold 40								/* Trigger threshold display position */
#define wave_width	 155							/* Waveform display width (full screen) */
#define wave_height	 128							/* Waveform display height */

static uint8_t current_page = PAGE_OSC;				/* Current display page */
static uint8_t step_value = 6;						/* Waveform scan step value */
static int16_t oldWave[wave_width];					/* Previous displayed waveform data */
static int16_t newWave[wave_width];					/* New waveform data to display */
static float maxVol = 0;							/* Input signal maximum voltage */
static float minVol = 0;							/* Input signal minimum voltage */
static uint16_t adc_value[ADC_NUM];					/* Store ADC raw data */
static uint16_t adc_value2[ADC_NUM];				/* Store ADC raw data */
static uint8_t osc_stop_bit = OSC_RUN;				/* Oscilloscope pause flag */


/*
*   Register DMA for ADC data acquisition (first time init)
*/
void Register_oscShowData(void)
{
	mx_adc_dma_init((uint32_t)(&adc_value), ADC_NUM);
}

/*
*   Restart DMA for next acquisition (lightweight)
*/
static void Restart_oscShowData(void)
{
	mx_adc_dma_restart((uint32_t)(&adc_value), ADC_NUM);
}

/*
*   Function: Clear ADC display data
*   Parameters: None
*   Return: None
*/
void clear_adc_value(void)
{
	set_adc_convert_value(ADC_CONVERT_UN_FINSIH);
	memset(adc_value, 0, ADC_NUM * sizeof(uint16_t));
}

/*
*   Function: Set scan step value
*   Parameters: value - Step value
*   Return: None
*/
void set_step_value(uint8_t value)
{
	step_value = value;
}

/*
*   Function: Get scan step value
*   Parameters: None
*   Return: Step value
*/
uint8_t get_step_value(void)
{
	return step_value;
}

/*
*   Function: Set oscilloscope pause flag
*   Parameters: value - OSC_PAUSE or OSC_RUN
*   Return: None
*/
void set_osc_stop_bit(uint8_t value)
{
	osc_stop_bit = value;
}

/*
*   Function: Get oscilloscope pause flag
*   Parameters: None
*   Return: OSC_PAUSE or OSC_RUN
*/
uint8_t get_osc_stop_bit(void)
{
	return osc_stop_bit;
}

uint8_t get_current_page(void)
{
	return current_page;
}

static void Restart_oscShowData(void);

void switch_page(void)
{
	current_page = (current_page + 1) % PAGE_MAX;
	TFT_Fill(0, 0, 160, 128, BLACK);
	switch(current_page)
	{
		case PAGE_OSC:
			TFT_OscStaticUI();
			memset(oldWave, 0, sizeof(oldWave));
			memset(newWave, 0, sizeof(newWave));
			clear_adc_value();
			Restart_oscShowData();
			break;
		case PAGE_PWM:
			TFT_PwmStaticUI();
			break;
		case PAGE_ART:
			TFT_DrawKanade();
			break;
		default:
			break;
	}
}


/*
 * Full-screen oscilloscope static UI (PAGE_OSC)
 */
void TFT_OscStaticUI(void)
{
    uint16_t i=0;

    /* Grid lines across full screen */
    for(i=0;i<155;i++)
    {
        TFT_DrawPoint(i,56,GREEN);  /* center horizontal line */
    }
    for(i=0;i<=110;i++)
    {
        TFT_DrawPoint(0,i,GREEN);   /* left vertical axis */
    }
    /* Horizontal tick marks every ~15 pixels */
    for(i=0;i<15;i++)
    {
        TFT_DrawPoint((i*10)+2,57,GREEN);
        TFT_DrawPoint((i*10)+3,57,GREEN);
    }
    /* Bottom info bar background */
    TFT_Fill(0,112,160,128,DARKBLUE);
    TFT_ShowString(2,112,(uint8_t *)"Vpp:",WHITE,DARKBLUE,16,0);
    TFT_ShowString(80,112,(uint8_t *)"F:",WHITE,DARKBLUE,16,0);
}

/*
 * TFT_StaticUI - backward compatible, calls full-screen OSC UI
 */
void TFT_StaticUI(void)
{
    TFT_OscStaticUI();
}

/*
 * Show oscilloscope bottom info (Vpp + Freq)
 */
void TFT_ShowOscInfo(void)
{
    uint32_t freq = 0;
    char showData[32]={0};

    sprintf(showData,"%1.2fV ",(maxVol - minVol) * 2);
    TFT_ShowString(34,112,(uint8_t *)showData,YELLOW,DARKBLUE,16,0);
    memset(showData,0,32);

    freq = get_freq_value();
    if(freq >= 1000)
    {
        sprintf(showData,"%3.1fKHz",(freq / 1000.0f));
        TFT_ShowString(96,112,(uint8_t *)showData,YELLOW,DARKBLUE,16,0);
        memset(showData,0,32);
    }
    else
    {
        sprintf(showData,"%4dHz",freq);
        TFT_ShowString(96,112,(uint8_t *)showData,YELLOW,DARKBLUE,16,0);
        memset(showData,0,32);
    }
}

/*
 * PWM page static UI (PAGE_PWM)
 */
void TFT_PwmStaticUI(void)
{
    TFT_ShowString(28,4,(uint8_t *)"PWM Setting",WHITE,PURPLE,16,0);
    TFT_DrawLine(0,22,160,22,PURPLE);

    TFT_ShowString(10,30,(uint8_t *)"State:",WHITE,BLACK,16,0);
    TFT_ShowString(10,52,(uint8_t *)"Freq :",WHITE,BLACK,16,0);
    TFT_ShowString(10,74,(uint8_t *)"Duty :",WHITE,BLACK,16,0);

    TFT_Fill(0,108,160,128,DARKBLUE);
    TFT_ShowString(2,110,(uint8_t *)"K1:SW K2:Frq K3:Dty",YELLOW,DARKBLUE,16,0);
}

/*
 * PWM page dynamic update
 */
void TFT_ShowPwmUI(void)
{
    uint8_t pwm_state = 0;
    char showData[32]={0};

    pwm_state = get_pwm_state();
    if(pwm_state == PWM_ON)
    {
        TFT_ShowString(74,30,(uint8_t *)" ON ",BLACK,GREEN,16,0);
    }
    else
    {
        TFT_ShowString(74,30,(uint8_t *)" OFF",BLACK,RED,16,0);
    }

    if(get_pwm_out_freq() >= 1000)
    {
        sprintf(showData,"%3dKHz ",get_pwm_out_freq()/1000);
        TFT_ShowString(74,52,(uint8_t *)showData,BLACK,YELLOW,16,0);
        memset(showData,0,32);
    }
    else
    {
        sprintf(showData,"%4dHz ",get_pwm_out_freq());
        TFT_ShowString(74,52,(uint8_t *)showData,BLACK,YELLOW,16,0);
        memset(showData,0,32);
    }
    sprintf(showData," %2d%% ",(uint16_t)((get_pwm_duty()*1.0f / get_pwm_period()) * 100));
    TFT_ShowString(74,74,(uint8_t *)showData,BLACK,YELLOW,16,0);
    memset(showData,0,32);
}

/*
 * Draw Kanade image (PAGE_ART)
 * RLE-compressed palette-indexed image, decoded to TFT
 */
void TFT_DrawKanade(void)
{
    uint16_t i = 0;
    uint8_t count, idx;

    TFT_Address_Set(0, 0, KANADE_IMG_W - 1, KANADE_IMG_H - 1);

    for(i = 0; i < KANADE_RLE_SIZE; i += 2)
    {
        count = kanade_rle_data[i];
        idx   = kanade_rle_data[i + 1];
        while(count--)
        {
            TFT_WR_DATA(kanade_palette[idx]);
        }
    }
}

/*
 * Function: Key scan handler
 * Parameters: key_pin - Key pin number, key_state - Key state
 * Return: None
 */
void key_scanf_handle(const uint16_t key_pin,const uint8_t key_state)
{
	uint16_t temp_period = 0;
	float temp_duty = 0;
	if(key_pin == KEY1_Pin)
	{
		switch(key_state)
		{
			case KeyPress:
				if(current_page == PAGE_PWM)
				{
					if(get_pwm_state() == PWM_OFF)
					{
						set_pwm_period(get_pwm_period());
						set_pwm_duty(get_pwm_duty());
						set_pwm_state(PWM_ON);
					}
					else
					{
						set_pwm_state(PWM_OFF);
					}
				}
				break;
			case KeyDoublePress:
				switch_page();
				break;
			default:
				break;
		}
	}
	else if(key_pin == KEY2_Pin)
	{
		switch(key_state)
		{
			case KeyPress:
				temp_duty = (get_pwm_duty()*1.0f / get_pwm_period());
				temp_period = get_pwm_period();
				temp_period = temp_period / 2;
				if(temp_period < 125)
				{
					temp_period = 1000;
				}
				set_pwm_period(temp_period);
				set_pwm_duty(temp_period * temp_duty);
				break;
			default:
				break;
		}
	}
	else if(key_pin == KEY3_Pin)
	{
		switch(key_state)
		{
			case KeyPress:
				temp_duty = get_pwm_period() * 0.04f + get_pwm_duty();
				if(temp_duty > get_pwm_period())
				{
					temp_duty = get_pwm_period();
				}
				set_pwm_duty(temp_duty);
				break;
			case KeyDoublePress:
				temp_duty = get_pwm_duty() - get_pwm_period() * 0.04f;
				if(temp_duty < 0)
				{
					temp_duty = 0;
				}
				set_pwm_duty(temp_duty);
				break;
			default:
				break;
		}
	}
	else if(key_pin == KEYD_Pin)
	{
		switch(key_state)
		{
			case KeyPress:
				if(get_osc_stop_bit() == OSC_RUN)
				{
					set_osc_stop_bit(OSC_PAUSE);
				}
				else
				{
					set_osc_stop_bit(OSC_RUN);
				}
				break;
			default:
				break;
		}
	}
}

/*
*   TFT_ShowUI - dispatches to current page update
*/
void TFT_ShowUI(void)
{
	if(current_page == PAGE_OSC)
	{
		TFT_ShowOscInfo();
	}
	else if(current_page == PAGE_PWM)
	{
		TFT_ShowPwmUI();
	}
	/* PAGE_ART: no dynamic update needed */
}

/*
*   Waveform display with improved ADC accuracy
*/
void osc_waveShow(uint16_t vref_value)
{
	uint16_t i = 0, t = 0, num = 0;
	int16_t prePos = 0;
	uint16_t v_max = 0, v_min = 4095;
	uint16_t sample_count = 0;
	float voltage_scale = 0.0f;

	if(get_adc_convert_value() != ADC_CONVERT_FINSIH)
		return;

	/* Pre-compute voltage scale factor: (1.20 / vref_value) * 20 */
	/* 1.20V is the typical Vrefint for GD32E230 */
	if(vref_value > 0)
		voltage_scale = (1.20f * 20.0f) / (float)vref_value;
	else
		voltage_scale = (3.3f * 20.0f) / 4095.0f;

	/* Step 1: Convert raw ADC to display values, track min/max on raw data */
	for(i = 0; i < ADC_NUM && num < ADC_NUM; i += step_value)
	{
		/* Track min/max on raw ADC values for accurate voltage measurement */
		if(adc_value[i] < v_min)
			v_min = adc_value[i];
		if(adc_value[i] > v_max)
			v_max = adc_value[i];

		/* Convert to display coordinates with proper rounding */
		adc_value2[num] = (uint16_t)(adc_value[i] * voltage_scale + 0.5f);

		/* Clamp to display range before coordinate transform */
		if(adc_value2[num] > 110)
			adc_value2[num] = 110;

		/* Transform to screen Y coordinate (inverted) */
		adc_value2[num] = 110 - adc_value2[num];
		num++;
	}

	sample_count = num;

	/* Step 2: Rising edge trigger detection within valid range */
	t = 0;
	if(sample_count > wave_width)
	{
		for(i = 0; i < (sample_count - wave_width - 1); i++)
		{
			/* Rising edge: current above threshold, next below (inverted Y) */
			if((adc_value2[i] > volThreshold) && (adc_value2[i + 1] <= volThreshold))
			{
				t = i;
				break;
			}
		}
	}

	/* Step 3: Extract display data from trigger point */
	for(i = 0; i < wave_width; i++)
	{
		if((t + i) < sample_count)
			newWave[i] = adc_value2[t + i];
		else
			newWave[i] = 110;
	}

	/* Step 4: Draw waveform */
	prePos = newWave[0];
	for(i = 1; i < wave_width - 2; i++)
	{
		TFT_DrawLine(i, oldWave[i], i + 1, oldWave[i + 1], BLACK);
		TFT_DrawLine(i, prePos, i + 1, newWave[i], GREEN);
		prePos = newWave[i];
	}

	/* Step 5: Save current waveform for next erase */
	for(i = 1; i < wave_width; i++)
	{
		oldWave[i] = newWave[i - 1];
	}

	/* Step 6: Calculate actual voltage from raw ADC min/max */
	maxVol = (float)v_max * voltage_scale / 20.0f;
	minVol = (float)v_min * voltage_scale / 20.0f;

	clear_adc_value();
	Restart_oscShowData();
}
