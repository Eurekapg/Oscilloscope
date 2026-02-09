#include "hw_key.h"

/*
 * Function: Initialize key instance
 * Parameters:
 *     key_gpio - GPIO port for the key
 *     key_pin - GPIO pin for the key
 *     key_press_level - Logic level when key is pressed
 * Return: struct key_class - Key handle
 */
struct key_class key_init(uint32_t key_gpio,uint32_t key_pin,uint8_t key_press_level)
{
	struct key_class key_handle;
	key_handle.key_gpio = key_gpio;
	key_handle.key_pin = key_pin;
	key_handle.key_press_level = key_press_level;
	key_handle.key_state = KEY_NoPress;
	key_handle.keyCnt = 0;
	key_handle.keyCount = 0;
	key_handle.keyFcnt = 0;
	key_handle.keyLongFlag = 0;
	return key_handle;
}

/*
 * Function: Scan key instance - Call every 10ms
 * Parameters:
 *     key_handle - Pointer to key handle
 * Return: None
 */
void key_scanf(struct key_class *key_handle)
{
	key_handle->key_state = KEY_NoPress;	/* Default: no key pressed */
	/* If key is pressed */
	if(gpio_input_bit_get(key_handle->key_gpio,key_handle->key_pin) == key_handle->key_press_level)
	{
		key_handle->keyCnt++;						/* Record key press duration */
		if(key_handle->keyCnt >= 120)
		{
			key_handle->keyCnt = 120;			/* Prevent counter overflow */
		}
	}
	else
	{
		/* Key has been released */
		if((key_handle->keyCnt >= 100) && (key_handle->keyLongFlag == 0)){
			/* Pressed for 1s without short press - long press detected */
			key_handle->key_state = KeyLongPress;
		}
		else if(key_handle->keyCnt >= 2)
		{
			/* Press duration > 20ms - valid press detected */
			key_handle->keyCount++;     			/* Count short press */
			key_handle->keyFcnt = DOUBLE_TIME;   	/* Window for double press detection */
			key_handle->keyLongFlag = 1;    		/* Short press occurred, disable long press until release */
		}
		key_handle->keyCnt = 0;
		if(key_handle->keyFcnt)
		{
			key_handle->keyFcnt--;  /* Decrement short press timer */
			if(key_handle->keyFcnt <= 0)
			{
					/* Short press window expired */
					if(key_handle->keyCount == 1){
							/* Single press */
							key_handle->key_state = KeyPress;
					}
					else if(key_handle->keyCount == 2){
							/* Double press */
							key_handle->key_state = KeyDoublePress;
					}
					key_handle->keyFcnt = 0;
					key_handle->keyCount = 0;
					key_handle->keyLongFlag = 0;
			}
		}
	}
}

/*
 * Function: Global key scan - Check all keys - Call every 10ms
 * Parameters: None
 * Return: Which key was pressed (supports long press, double press, single press)
 *         Only one key event returned at a time
 */
uint8_t key_scanf_all(void)
{
    uint8_t ret = KEY_ERROR;    /* Key return value */
    static uint8_t keyCnt[3] = {0};  /* Key press duration counter */
    static uint8_t keyFcnt[3] = {0}; /* Short press timer for double press detection */
    static uint8_t keyCount[3] = {0};    /* Short press count */
    static uint8_t keyLongFlag[3] = {0}; /* Long press flag - valid only on release */
    if(GET_KEY1_IN == RESET){
        keyCnt[KEY1]++;           /* Timer interrupt - record low level duration */
        if(keyCnt[KEY1] >= 120){
            /* Prevent counter overflow */
            keyCnt[KEY1] = 120;
        }
    }
    if(GET_KEY1_IN == SET)
    {
        /* Key released */
        if((keyCnt[KEY1] >= 100) && (keyLongFlag[KEY1] == 0)){
            /* Pressed for 1s without short press - long press */
            ret = Key1LongPress;
        }
        else if(keyCnt[KEY1] >= 2)
        {
            /* Press duration > 20ms - valid press */
            keyCount[KEY1]++;     /* Count short press */
            keyFcnt[KEY1] = DOUBLE_TIME;   /* 300ms window for double press */
            keyLongFlag[KEY1] = 1;    /* Short press occurred, disable long press until release */
        }
        keyCnt[KEY1] = 0;
        if(keyFcnt[KEY1])
        {
            keyFcnt[KEY1]--;  /* Decrement short press timer */
            if(keyFcnt[KEY1] <= 0)
            {
                /* Short press window expired */
                if(keyCount[KEY1] == 1){
                    /* Single press */
                    ret = Key1Press;
                }
                else if(keyCount[KEY1] ==2){
                    /* Double press */
                    ret = Key1DoublePress;
                }
                keyFcnt[KEY1] = 0;
                keyCount[KEY1] = 0;
                keyLongFlag[KEY1] = 0;
            }
        }
    }

    if(GET_KEY2_IN == RESET){
        keyCnt[KEY2]++;           /* Timer interrupt - record low level duration */
        if(keyCnt[KEY2] >= 120){
            /* Prevent counter overflow */
            keyCnt[KEY2] = 120;
        }
    }
    if(GET_KEY2_IN == SET)
    {
        /* Key released */
        if((keyCnt[KEY2] >= 100) && (keyLongFlag[KEY2] == 0)){
            /* Pressed for 1s without short press - long press */
            ret = Key2LongPress;
        }
        else if(keyCnt[KEY2] >= 2)
        {
            /* Press duration > 20ms - valid press */
            keyCount[KEY2]++;     /* Count short press */
            keyFcnt[KEY2] = DOUBLE_TIME;   /* 300ms window for double press */
            keyLongFlag[KEY2] = 1;    /* Short press occurred, disable long press until release */
        }
        keyCnt[KEY2] = 0;
        if(keyFcnt[KEY2])
        {
            keyFcnt[KEY2]--;  /* Decrement short press timer */
            if(keyFcnt[KEY2] <= 0)
            {
                /* Short press window expired */
                if(keyCount[KEY2] == 1){
                    /* Single press */
                    ret = Key2Press;
                }
                else if(keyCount[KEY2] ==2){
                    /* Double press */
                    ret = Key2DoublePress;
                }
                keyFcnt[KEY2] = 0;
                keyCount[KEY2] = 0;
                keyLongFlag[KEY2] = 0;
            }
        }
    }

	if(GET_KEY3_IN == RESET){
        keyCnt[KEY3]++;           /* Timer interrupt - record low level duration */
        if(keyCnt[KEY3] >= 120){
            /* Prevent counter overflow */
            keyCnt[KEY3] = 120;
        }
    }
    if(GET_KEY3_IN == SET)
    {
        /* Key released */
        if((keyCnt[KEY3] >= 100) && (keyLongFlag[KEY3] == 0)){
            /* Pressed for 1s without short press - long press */
            ret = Key3LongPress;
        }
        else if(keyCnt[KEY3] >= 2)
        {
            /* Press duration > 20ms - valid press */
            keyCount[KEY3]++;     /* Count short press */
            keyFcnt[KEY3] = DOUBLE_TIME;   /* 300ms window for double press */
            keyLongFlag[KEY3] = 1;    /* Short press occurred, disable long press until release */
        }
        keyCnt[KEY3] = 0;
        if(keyFcnt[KEY3])
        {
            keyFcnt[KEY3]--;  /* Decrement short press timer */
            if(keyFcnt[KEY3] <= 0)
            {
                /* Short press window expired */
                if(keyCount[KEY3] == 1){
                    /* Single press */
                    ret = Key3Press;
                }
                else if(keyCount[KEY3] ==2){
                    /* Double press */
                    ret = Key3DoublePress;
                }
                keyFcnt[KEY3] = 0;
                keyCount[KEY3] = 0;
                keyLongFlag[KEY3] = 0;
            }
        }
    }
    return ret;
}

