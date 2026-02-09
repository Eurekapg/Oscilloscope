#include "gd32e23x.h"
#include "systick.h"
#include <string.h>
#include <stdio.h>
#include "hw_led.h"
#include "main.h"
#include "hw_gpio.h"
#include "hw_key.h"
#include "hw_ec11.h"
#include "hw_lcdinit.h"
#include "hw_spi.h"
#include "hw_tim.h"
#include "hw_adc.h"
#include "mid_timer.h"
#include "mid_pwm.h"
#include "mid_adc.h"
#include "mid_lcd.h"
#include "osc_task.h"

enum led_instance
{
	led1 = 0,
	led2
};
enum key_instance
{
	key1 = 0,
	key2,
	key3,
	keyd
};

static struct led_class led_hanlde[2];	/* Create 2 LED instances */
static struct key_class key_handle[4];	/* Create 4 key instances */
static struct ec11_class ec11_handle;		/* Create 1 EC11 encoder instance */

int main(void)
{
	uint16_t key_timer_value = 0;
	uint16_t tft_timer_value = 0;
	uint16_t adc_vref_value = 0;
	uint8_t step_value = 0;
	uint8_t osc_stop_bit = 0;

  systick_config();		/* Clock initialization */

	mx_gpio_init();			/* GPIO configuration initialization */
	mx_spi0_init();			/* SPI0 configuration initialization */
	mx_adc_init();			/* ADC configuration initialization */
	mx_tim2_init();			/* TIM2 configuration initialization */
	mx_tim14_init();		/* TIM14 configuration initialization */
	mx_tim15_init();		/* TIM15 configuration initialization */

	led_hanlde[led1] = led_init(LED1_GPIO_Port,LED1_Pin,RESET);	/* Register LED instance */
	led_hanlde[led2] = led_init(LED2_GPIO_Port,LED2_Pin,RESET);	/* Register LED instance */

	key_handle[key1] = key_init(KEY1_GPIO_Port,KEY1_Pin,RESET);	/* Register key instance */
	key_handle[key2] = key_init(KEY2_GPIO_Port,KEY2_Pin,RESET);	/* Register key instance */
	key_handle[key3] = key_init(KEY3_GPIO_Port,KEY3_Pin,RESET);	/* Register key instance */
	key_handle[keyd] = key_init(KEYD_GPIO_Port,KEYD_Pin,RESET);	/* Register key instance */

	ec11_handle = ec11_init(KEYA_GPIO_Port,KEYA_Pin,KEYB_GPIO_Port,KEYB_Pin);	/* Register EC11 encoder instance */

	Set_ADC_Channel(ADC_CHANNEL_17);
	adc_vref_value = Get_ADC_Average(200);


	delay_1ms(1000);	/* Startup delay to avoid power supply issues and ensure TFT reset synchronization - add delay before system initialization for better stability */
	TFT_Init();
	TFT_Fill(0,0,160,128,BLACK);

	TFT_ShowString(8,35,(uint8_t *)"Oscilloscope",WHITE,BLACK,24,0);
	TFT_ShowString(28,70,(uint8_t *)"Press any key",WHITE,BLACK,16,0);
	TFT_ShowString(48,90,(uint8_t *)"to enter",WHITE,BLACK,16,0);

	while(1)
	{
		if(gpio_input_bit_get(KEY1_GPIO_Port,KEY1_Pin) == RESET ||
		   gpio_input_bit_get(KEY2_GPIO_Port,KEY2_Pin) == RESET ||
		   gpio_input_bit_get(KEY3_GPIO_Port,KEY3_Pin) == RESET ||
		   gpio_input_bit_get(KEYD_GPIO_Port,KEYD_Pin) == RESET)
		{
			delay_1ms(200);
			break;
		}
	}

	TFT_Fill(0,0,160,128,BLACK);
	TFT_StaticUI();

	Set_ADC_Channel(ADC_CHANNEL_3);
	clear_adc_value();
	Register_oscShowData();

	timer_enable(TIMER15);	/* Start millisecond timer */
	timer_enable(TIMER2);		/* Start frequency measurement timer */
	while(1)
	{
		key_timer_value = get_key_timer_value();
		tft_timer_value = get_tft_timer_value();
		if(key_timer_value >= 10)
		{
			key_scanf(&key_handle[key1]);
			key_scanf(&key_handle[key2]);
			key_scanf(&key_handle[key3]);
			key_scanf(&key_handle[keyd]);
			set_key_timer_value(0);
		}
		if(tft_timer_value >= 500)
		{
			set_key_bit_value(PAUSE_MS_TIMER);
			set_tft_bit_value(PAUSE_MS_TIMER);
			osc_stop_bit = get_osc_stop_bit();
			if(get_current_page() == PAGE_OSC && osc_stop_bit == OSC_RUN){
				osc_waveShow(adc_vref_value);
			}
			TFT_ShowUI();
			set_tft_timer_value(0);
			set_key_bit_value(RUN_MS_TIMER);
			set_tft_bit_value(RUN_MS_TIMER);
		}
		if(key_handle[key1].key_state != KEY_NoPress)
		{
			key_scanf_handle(key_handle[key1].key_pin,key_handle[key1].key_state);
			key_handle[key1].key_state = KEY_NoPress;
		}
		if(key_handle[key2].key_state != KEY_NoPress)
		{
			key_scanf_handle(key_handle[key2].key_pin,key_handle[key2].key_state);
			key_handle[key2].key_state = KEY_NoPress;
		}
		if(key_handle[key3].key_state != KEY_NoPress)
		{
			key_scanf_handle(key_handle[key3].key_pin,key_handle[key3].key_state);
			key_handle[key3].key_state = KEY_NoPress;
		}
		if(key_handle[keyd].key_state != KEY_NoPress)
		{
			key_scanf_handle(key_handle[keyd].key_pin,key_handle[keyd].key_state);
			key_handle[keyd].key_state = KEY_NoPress;
		}
		if(ec11_handle.ec11_direction != ec11_static)
		{
			if(ec11_handle.ec11_direction == ec11_forward)
			{
				step_value = get_step_value();
				step_value = step_value - 1;
				if(step_value <= 1)
				{
					step_value = 1;
				}
				set_step_value(step_value);
			}
			else
			{
				step_value = get_step_value();
				step_value = step_value + 1;
				if(step_value > 6)
				{
					step_value = 6;
				}
				set_step_value(step_value);
			}
			ec11_handle.ec11_direction = ec11_static;
		}
	}
}

void EXTI4_15_IRQHandler(void)
{
	if(exti_interrupt_flag_get(EXTI_4) != RESET)
	{
		ec11_exti_callback(&ec11_handle);
	}
}
