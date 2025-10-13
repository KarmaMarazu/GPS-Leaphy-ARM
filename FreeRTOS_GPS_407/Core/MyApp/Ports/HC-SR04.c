/*
 * 	HC-SR04.c
 *
 *
 *
 *  Created on: Oct 13, 2025
 *      Author: Mika Dahlkamp
 */

#include <admin.h>
#include "main.h"

extern TIM_HandleTypeDef htim11;

uint32_t pMillis;
uint32_t val1 = 0;
uint32_t val2 = 0;
uint16_t distance = 0;

int GetDistance(void)
{
	HAL_TIM_Base_Start(&htim11); // starten timer
	HAL_GPIO_WritePin(GPIOB, Trig_Pin, GPIO_PIN_SET);
	__HAL_TIM_SET_COUNTER(&htim11, 0);
	while (__HAL_TIM_GET_COUNTER(&htim11)<10); // 10 micro seconden wachten om een pulse van 10 micro te sturen via de trig pin
	HAL_GPIO_WritePin(GPIOB, Trig_Pin, GPIO_PIN_RESET);

	pMillis = HAL_GetTick();
	while (!(HAL_GPIO_ReadPin (GPIOB, Echo_Pin)) && pMillis + 10 >  HAL_GetTick());
	val1 = __HAL_TIM_GET_COUNTER (&htim11); // Tijdpunt krijgen van begin van de echo pulse

	pMillis = HAL_GetTick();
	while ((HAL_GPIO_ReadPin (GPIOB, Echo_Pin)) && pMillis + 50 > HAL_GetTick());
	val2 = __HAL_TIM_GET_COUNTER (&htim11); // Tijdpunt krijgen van de eind tijd van de echo pulse
	HAL_TIM_Base_Stop(&htim11);

	distance = (val2-val1)* 0.034/2; // Afstand in cm berekenen met standaard formule
	return distance;
}

