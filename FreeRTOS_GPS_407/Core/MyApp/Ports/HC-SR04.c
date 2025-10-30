/**
* @file HC-SR04.c
* @brief Dit bestand bevat de functie en variabelen voor het aansturen van de ultrasonic afstandsensor. <BR>
* hiervoor moet de .IOC zo zijn aangepast dat de sensor werkt met Timer11.
* @author Mika Dahlkamp
*
* @date 10/10/2025
*/

#include <admin.h>
#include "main.h"
#include "gps.h"

extern TIM_HandleTypeDef htim11;

uint32_t Millis;
uint32_t val1 = 0;
uint32_t val2 = 0;
uint16_t distance = 0;

/**
* @brief Lees de afstand van sensor tot voorwerp.
* @return int distance
*/
void GetDistance(void* argument)
{
	TaskStatus_t    TaskDetails;

	while(TRUE)
	{
		HAL_TIM_Base_Start(&htim11); 													// starten timer
		HAL_GPIO_WritePin(GPIOB, Trig_Pin, GPIO_PIN_SET);
		__HAL_TIM_SET_COUNTER(&htim11, 0);
		while (__HAL_TIM_GET_COUNTER(&htim11)<10); 										// 10 micro seconden wachten om een pulse van 10 micro te sturen via de trig pin
		HAL_GPIO_WritePin(GPIOB, Trig_Pin, GPIO_PIN_RESET);

		Millis = HAL_GetTick();
		while (!(HAL_GPIO_ReadPin (GPIOB, Echo_Pin)) && Millis + 10 >  HAL_GetTick()); 	// Wachten totdat de echo pin hoog wordt
		val1 = __HAL_TIM_GET_COUNTER (&htim11); 										// Tijdpunt krijgen van begin van de echo pulse

		Millis = HAL_GetTick();
		while ((HAL_GPIO_ReadPin (GPIOB, Echo_Pin)) && Millis + 50 > HAL_GetTick()); 	// Wachten todat de echo pin laag wordt
		val2 = __HAL_TIM_GET_COUNTER (&htim11); 										// Tijdpunt krijgen van de eind tijd van de echo pulse
		HAL_TIM_Base_Stop(&htim11);

		distance = (val2-val1)* 0.034/2; 	// Afstand in cm berekenen met standaard formule

		if(distance < 40)
		{
			vTaskGetInfo(GetTaskhandle("drive_task"), &TaskDetails, pdFALSE, eInvalid);
			if((TaskDetails.eCurrentState) != eSuspended)		// kijken of drivetask suspended is als ie dat niet is wordt een tasknotify gestuurd
				xTaskNotifyGive(GetTaskhandle("drive_task"));
		}
		osDelay(100);
	}
}

