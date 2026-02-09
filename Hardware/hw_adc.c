#include "hw_adc.h"
#include "systick.h"

void mx_adc_init(void)
{
	/* Enable GPIO clock */
	rcu_periph_clock_enable(RCU_GPIOA);

	/* Enable ADC clock */
	rcu_periph_clock_enable(RCU_ADC);

	/* Enable clock configuration, ADC clock = 28MHz */
	rcu_adc_clock_config(RCU_ADCCK_AHB_DIV9);

	/* GPIO configuration: PA3 as analog input, no pull-up/down */
	gpio_mode_set(GPIOA, GPIO_MODE_ANALOG, GPIO_PUPD_NONE, GPIO_PIN_3);

	/* ADC continuous conversion mode enable */
	adc_special_function_config(ADC_CONTINUOUS_MODE, ENABLE);

	/* ADC scan mode disable - single channel only */
	adc_special_function_config(ADC_SCAN_MODE, DISABLE);

	/* ADC injected channel auto-conversion disable - regular channel only */
	adc_special_function_config(ADC_INSERTED_CHANNEL_AUTO, DISABLE);

	/* ADC data alignment - right aligned */
	adc_data_alignment_config(ADC_DATAALIGN_RIGHT);

	/* ADC channel length configuration */
	adc_channel_length_config(ADC_REGULAR_CHANNEL, 1U);

	/* ADC regular channel configuration - PA3, sequence 0, channel 3, sample time 28.5 cycles */
	adc_regular_channel_config(0, ADC_CHANNEL_3, ADC_SAMPLETIME_28POINT5);

	/* ADC temperature sensor and Vrefint enable */
  adc_tempsensor_vrefint_enable();

	/* ADC external trigger configuration - software trigger */
	adc_external_trigger_source_config(ADC_REGULAR_CHANNEL, ADC_EXTTRIG_REGULAR_NONE);
	adc_external_trigger_config(ADC_REGULAR_CHANNEL, ENABLE);

	/* Enable ADC */
	adc_enable();
	delay_1ms(2U);

	/* Enable calibration and reset */
	adc_calibration_enable();
	delay_1ms(1U);

	/* DMA mode enable */
	adc_dma_mode_enable();

	/* ADC software trigger enable */
	adc_software_trigger_enable(ADC_REGULAR_CHANNEL);
}

void mx_adc_dma_init(uint32_t adc_value,uint32_t number)
{
    rcu_periph_clock_enable(RCU_DMA);	/* DMA clock enable */

    dma_parameter_struct dma_data_parameter;	/* DMA parameter structure */

    nvic_irq_enable(DMA_Channel0_IRQn, 0U);	/* Enable DMA interrupt */

    dma_deinit(DMA_CH0);	/* Channel 0 reset */

    dma_data_parameter.periph_addr  = (uint32_t)(&ADC_RDATA);       /* Peripheral address */
    dma_data_parameter.periph_inc   = DMA_PERIPH_INCREASE_DISABLE;  /* Peripheral address no increment */
    dma_data_parameter.memory_addr  = (uint32_t)(adc_value);       /* Memory address */
    dma_data_parameter.memory_inc   = DMA_MEMORY_INCREASE_ENABLE;   /* Memory address increment */
    dma_data_parameter.periph_width = DMA_PERIPHERAL_WIDTH_16BIT;   /* Peripheral width */
    dma_data_parameter.memory_width = DMA_MEMORY_WIDTH_16BIT;       /* Memory width */
    dma_data_parameter.direction    = DMA_PERIPHERAL_TO_MEMORY;     /* Peripheral to memory */
    dma_data_parameter.number       = number;                				/* Transfer count */
    dma_data_parameter.priority     = DMA_PRIORITY_HIGH;            /* DMA priority */

    dma_init(DMA_CH0, &dma_data_parameter);                         /* DMA channel 0 initialization */

    dma_circulation_enable(DMA_CH0);                               	/* DMA circular mode enable */

    dma_channel_enable(DMA_CH0);                                    /* DMA channel 0 enable */

    dma_interrupt_enable(DMA_CH0, DMA_CHXCTL_FTFIE);								/* Enable DMA full transfer interrupt */
}

void mx_adc_dma_restart(uint32_t adc_value, uint32_t number)
{
    dma_channel_disable(DMA_CH0);
    dma_interrupt_flag_clear(DMA_CH0, DMA_INT_FLAG_G);
    dma_memory_address_config(DMA_CH0, adc_value);
    dma_transfer_number_config(DMA_CH0, number);
    dma_interrupt_enable(DMA_CH0, DMA_CHXCTL_FTFIE);
    dma_channel_enable(DMA_CH0);
}
