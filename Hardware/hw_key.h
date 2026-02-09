#ifndef __HW_KEY_H
#define __HW_KEY_H

#include "main.h"

/* Key macro definitions */

#define DOUBLE_TIME 40  	/* 400ms window for double press detection */
#define KEY_NoPress 0xFF
#define KEY_ERROR   0x00
#define KEY_OK      0x01

/* For single key instance usage */

#define KeyPress 0x10
#define KeyDoublePress 0x20
#define KeyLongPress  0x30

struct key_class
{
	uint32_t key_gpio;				/* KEY GPIO port */
	uint32_t key_pin;					/* KEY GPIO pin */
	uint8_t key_state;				/* KEY state */
	uint8_t key_press_level;	/* Logic level when pressed */
	uint8_t keyCnt;						/* Key press duration counter */
	uint8_t keyFcnt; 					/* Short press timer for double press detection */
	uint8_t keyCount; 		   	/* Short press count */
	uint8_t keyLongFlag; 			/* Long press flag - valid only on release */
};

/* For scanning all keys simultaneously */
enum KEYNum
{
    KEY1 = 0,
    KEY2,
	KEY3,
	KEY_NUM
};

#define GET_KEY1_IN        gpio_input_bit_get(KEY1_GPIO_Port,KEY1_Pin)
#define GET_KEY2_IN        gpio_input_bit_get(KEY2_GPIO_Port,KEY2_Pin)
#define GET_KEY3_IN        gpio_input_bit_get(KEY3_GPIO_Port,KEY3_Pin)

#define Key1Press 0x10
#define Key2Press 0x11
#define Key3Press 0x12

#define Key1DoublePress 0x20
#define Key2DoublePress 0x21
#define Key3DoublePress 0x22

#define Key1LongPress  0x30
#define Key2LongPress  0x31
#define Key3LongPress  0x32

/* For scanning all keys simultaneously */

uint8_t key_scanf_all(void);
void key_scanf(struct key_class *key_handle);
struct key_class key_init(uint32_t key_gpio,uint32_t key_pin,uint8_t key_press_level);

#endif

