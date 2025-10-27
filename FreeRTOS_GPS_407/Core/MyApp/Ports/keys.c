/**
* @file keys.c
* @brief Keyboard-driver for SpARM-board v2
* @details     pin-info: <br>
           PD1  - R2 <br>
           PD2  - R3 <br>
           PD3  - R4 <br>
           PD6  - K1 <br>
           PD7  - K2 <br>
           PD8  - K3 <br>
           PD9  - K4 <br>
           PD11 - R1 <br>
<br>
	Key B1 = 1  ----- key B16 = 16 <br>
<br>
<br>
<br>
To use the keyboard first initialize the ports: <br>
	KEYS_init(); <br>
After that, you can use polling to read a key with: <br>
	KEYS_read(); </br>
* @author J.F. van der Bent
* @author W Pielage & E Helmond
* @version 5
* @date 24-11-2021
*/

#include "main.h"

void KEYS_init(void)
/* Keys initialize
 * In this function the ports are set for the keyboard.
 * The rows are outputs and the columns are input.
 */
{
	__HAL_RCC_GPIOD_CLK_ENABLE();
	GPIO_InitTypeDef gpio;
	gpio.Mode = GPIO_MODE_OUTPUT_PP;
	gpio.Pull = GPIO_NOPULL;
	gpio.Speed = GPIO_SPEED_FREQ_VERY_HIGH;

	gpio.Pin = KEY_ROW;
	HAL_GPIO_Init(PKEY_ROW, &gpio);

	gpio.Pin = KEY_COL;
	gpio.Mode = GPIO_MODE_INPUT;
	gpio.Pull = GPIO_PULLDOWN;
	HAL_GPIO_Init(PKEY_COL, &gpio);
	HAL_GPIO_WritePin(PKEY_ROW, KEY_ROW, 0);
}

void KEYS_initISR(int hilo)
/* Keys initialize for ISR
 * Set all lines high
 * */
{
	HAL_GPIO_WritePin(PKEY_ROW, KEY_R1, hilo);
	HAL_GPIO_WritePin(PKEY_ROW, KEY_R2, hilo);
	HAL_GPIO_WritePin(PKEY_ROW, KEY_R4, hilo);
	HAL_GPIO_WritePin(PKEY_ROW, KEY_R3, hilo);
}


unsigned int KEYS_read(void)
/* Keys read
 * This function reads which key is pressed.
 * It does so by making a row high and check if there is a connection with a column.
 * If there is a connection the value is returned.
 */
{
	unsigned int key = 0;
	unsigned out = 0;

	HAL_GPIO_WritePin(PKEY_ROW, KEY_R1, 0);
	HAL_GPIO_WritePin(PKEY_ROW, KEY_R4, 1);
	key = KEYS_kolom();
	if( key != 0 ) out= 12+key;

	HAL_GPIO_WritePin(PKEY_ROW, KEY_R4, 0);
	HAL_GPIO_WritePin(PKEY_ROW, KEY_R3, 1);
	key = KEYS_kolom();
	if( key != 0 ) out = 8+key;

	HAL_GPIO_WritePin(PKEY_ROW, KEY_R3, 0);
	HAL_GPIO_WritePin(PKEY_ROW, KEY_R2, 1);
	key = KEYS_kolom();
	if( key != 0 ) out = 4+key;

	HAL_GPIO_WritePin(PKEY_ROW, KEY_R2, 0);
	HAL_GPIO_WritePin(PKEY_ROW, KEY_R1, 1);
	key = KEYS_kolom();
	if( key != 0 ) out = key;

	return out;
}

unsigned int KEYS_kolom(void)
/* Keys Column
 * In this function the columns are checked if there is a connection.
 * If a column has a connection the return value is the column number.
 */
{
	unsigned int key = 0;
	if (HAL_GPIO_ReadPin(PKEY_COL, KEY_K1) == 1) key = 1;
	if (HAL_GPIO_ReadPin(PKEY_COL, KEY_K2) == 1) key = 2;
	if (HAL_GPIO_ReadPin(PKEY_COL, KEY_K3) == 1) key = 3;
	if (HAL_GPIO_ReadPin(PKEY_COL, KEY_K4) == 1) key = 4;
	return key;
}
