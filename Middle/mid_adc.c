#include "mid_adc.h"
#include "stdio.h"
#include "string.h"

static uint8_t adc_convert_bit = ADC_CONVERT_UN_FINSIH;

/*
 * Get single ADC conversion value
 */
static uint16_t Get_ADC_Val(void)
{
	uint16_t val = 0;

	adc_software_trigger_enable(ADC_REGULAR_CHANNEL);
	while ( adc_flag_get(ADC_FLAG_EOC) == RESET ){
	}
	val = adc_regular_data_read();

	return val;
}

/*
 * Get ADC average value with outlier removal
 * Removes top N and bottom N samples for robust averaging
 */
uint16_t Get_ADC_Average(uint16_t num)
{
    uint16_t i = 0, j = 0;
    uint32_t sum_val = 0;
    uint16_t buf[256];
    uint16_t temp = 0;
    uint16_t trim = 0;

    if(num > 256) num = 256;
    if(num < 6) num = 6;

    /* Use longest sample time for calibration accuracy */
    adc_regular_channel_config(0, ADC_CHANNEL_17, ADC_SAMPLETIME_239POINT5);

    /* Discard first few samples after channel switch */
    for(i = 0; i < 4; i++)
    {
        Get_ADC_Val();
    }

    /* Collect samples */
    for(i = 0; i < num; i++)
    {
        buf[i] = Get_ADC_Val();
    }

    /* Simple insertion sort for small arrays */
    for(i = 1; i < num; i++)
    {
        temp = buf[i];
        j = i;
        while(j > 0 && buf[j - 1] > temp)
        {
            buf[j] = buf[j - 1];
            j--;
        }
        buf[j] = temp;
    }

    /* Trim top 10% and bottom 10% outliers */
    trim = num / 10;
    if(trim < 2) trim = 2;

    sum_val = 0;
    for(i = trim; i < (num - trim); i++)
    {
        sum_val += buf[i];
    }
    sum_val = sum_val / (num - 2 * trim);

    return (uint16_t)sum_val;
}

/*
 * �������ݣ�����ADCͨ��
 * ������������
 * ����ֵ����
 */
void Set_ADC_Channel(uint8_t channel)
{
	adc_regular_channel_config(0, channel, ADC_SAMPLETIME_28POINT5);
}

/*
 * �������ݣ�ADCת������жϻص�
 * ������������
 * ����ֵ����
 */
void DMA_Channel0_IRQHandler(void)
{
	if(dma_interrupt_flag_get(DMA_CH0, DMA_INT_FLAG_FTF)){
		adc_convert_bit = ADC_CONVERT_FINSIH;
		dma_channel_disable(DMA_CH0);	//�ر�dma
		dma_interrupt_flag_clear(DMA_CH0, DMA_INT_FLAG_G);	//����жϱ�־λ
	}
}

/*
 * �������ݣ��õ�adcת����ɱ�־λ
 * ������������
 * ����ֵ��uint8_t
 */
uint8_t get_adc_convert_value(void)
{
	return adc_convert_bit;
}

/*
 * �������ݣ�����adcת����ɱ�־λ
 * ������������
 * ����ֵ��uint8_t
 */
void set_adc_convert_value(uint8_t bit)
{
	adc_convert_bit = bit;
}
