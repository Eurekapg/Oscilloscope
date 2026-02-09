#ifndef __OSC_TASK_H
#define __OSC_TASK_H

#include "main.h"

#define OSC_PAUSE 	0x01
#define OSC_RUN		0x02

#define PAGE_OSC    0
#define PAGE_PWM    1
#define PAGE_ART    2
#define PAGE_MAX    3

void TFT_StaticUI(void);
void TFT_ShowUI(void);
void osc_waveShow(uint16_t vref_value);
void key_scanf_handle(const uint16_t key_pin,const uint8_t key_state);
void Register_oscShowData(void);
void clear_adc_value(void);
void set_step_value(uint8_t value);
uint8_t get_step_value(void);
void set_osc_stop_bit(uint8_t value);
uint8_t get_osc_stop_bit(void);

void switch_page(void);
uint8_t get_current_page(void);
void TFT_OscStaticUI(void);
void TFT_ShowOscInfo(void);
void TFT_PwmStaticUI(void);
void TFT_ShowPwmUI(void);
void TFT_DrawKanade(void);
#endif

