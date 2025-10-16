/**
* @file ARM_keys.c
* @brief Behandelt de communicatie met de ARM-toetsjes met: Eventgroups, TaskNotify, Interrupt-handling.<br>
* <b>Demonstreert: xEventGroupWaitBits(), xTaskGetHandle(), xTaskNotify(), xTaskNotifyWait(),xSemaphoregive(), xSemaphoreTake(). </b><br>
*
* Aan de ARM-keys is een interrupt gekoppeld (zie stm32f4xx_it.c). Die stuurt een event
* door die opgevangen wordt door task ARM_keys_IRQ().<BR><BR>
* In ARM_keys_IRQ wordt er aan de hand van welke key, bepaald wat er gebeurd moet worden. waypoint set, reset, wissel modus
* @author  MSC, Twan Ton, Mika Dahlkamp, Jasper Verduin en Thomas van Ooijen
*
* @date 10/10/2025
*/

#include <admin.h>
#include "main.h"
#include "cmsis_os.h"
#include "gps.h"

void PrintLog(void)
{
	UART_puts("\r\n===============================================================\r\n");
	for(int i = 0; i <= k; i++)
	{
		UART_puts("\r\n=============================================================== Log Number = "); UART_putint(i);
		UART_puts("\rStatus = "); UART_putchar(Log.Route[i].status);
		UART_puts("\rLatitude = "); UART_putint((int)Log.Route[i].latitude);
		UART_puts("\rLongitude = "); UART_putint((int)Log.Route[i].longitude);
		UART_puts("\rCourse = "); UART_putint((int)Log.Route[i].course);
		UART_puts("\rLeaphy Actie = "); UART_putint((int)Log.LeaphyActie[i]);
		UART_puts("\r\n===============================================================");
	}
	k = 0;
}


/**
* @brief Zet een kleurenledje aan en uit.
* @param color De kleur.
* @return void.
*/
void toggle_led (uint32_t color)
{
	HAL_GPIO_TogglePin(GPIOD, color);   // turns led on or off
	osDelay(20);
	HAL_GPIO_TogglePin(GPIOD, color);   // turns led on or off
}


/**
* @brief Deze task handelt de ARM-toets af, die ontvangen is van de ISR-handler (zie: stm32f4xx_it.c). <BR>
* Hier wordt gewisseld tussen drive mode en oplaan mode. Dit wordt via een notify doorgestuurd naar bijbehorende task.
* @param *argument Niet gebruikt, eventueel een waarde of string om te testen
* @return void.
*/

void ARM_keys_IRQ (void *argument)
{
	unsigned int 	key;
	unsigned int 	j = 0;
	TickType_t		start = 0;
	TickType_t		stop;
	osThreadId_t 	hARM_keys;
	osThreadId_t 	hData_opslaanTask;
	TaskStatus_t    TaskDetails;

	UART_puts("\r\n"); UART_puts((char *)__func__); UART_puts(" started");

	vTaskSuspend(GetTaskhandle("drive_task"));					// stopt de drivetask bij opstart

	if (!(hARM_keys = GetTaskhandle("ARM_keys_task")))
		error_HaltOS("Err:ARM_hndle");
	if (!(hData_opslaanTask = GetTaskhandle("data_opslaanTask")))
		error_HaltOS("Err:data_opslaan handle");

    while (1)
	{

		// wait for ISR (EXTI0_IRQHandler()) to signal that a key is pressed
		key = xEventGroupWaitBits(hKEY_Event, 0xffff, pdTRUE, pdFALSE, portMAX_DELAY );

		vTaskGetInfo(GetTaskhandle("data_opslaanTask"), &TaskDetails, pdFALSE, eInvalid);

		if (((key == 0x0001) || (key == 0x0002)) && ((TaskDetails.eCurrentState) != eSuspended) )// verzend alleen als task niet suspended is.
			xTaskNotify(hData_opslaanTask, key, eSetValueWithOverwrite); 	// notify Data_opslaanTask with value

		if (key == 0x0004)
		{
			j++;

			if(j%2 == 0)
			{
				xSemaphoreTake(hGNRMC_Struct_Sem, portMAX_DELAY);			// wacht totdat de task klaar is met de mutex
				vTaskResume(GetTaskhandle("data_opslaanTask"));				// start de waypoints opslaan task
				vTaskSuspend(GetTaskhandle("drive_task"));					// stopt de drivetask
				HAL_GPIO_WritePin(GPIOD, LEDRED, GPIO_PIN_SET);				// rode led is route opslaan
				HAL_GPIO_WritePin(GPIOD, LEDGREEN, GPIO_PIN_RESET);			// groene led is drive mode
				xSemaphoreGive(hGNRMC_Struct_Sem);

				stop = xTaskGetTickCount() - start;							// slaat de vertreken tijd op sinds drive_task is gestart. max 4294967 sec
				UART_puts("\rtijds sinds start drive mode: ");	UART_putint(stop/1000); 	UART_puts(" seconden");

			}
			else if(j%2 == 1)
			{
				xSemaphoreTake(hGNRMC_Struct_Sem, portMAX_DELAY);			// wacht totdat de task klaar is met de mutex
				vTaskSuspend(GetTaskhandle("data_opslaanTask"));			// start de waypoints opslaan task
				vTaskResume(GetTaskhandle("drive_task"));					// start de drivetask
				HAL_GPIO_WritePin(GPIOD, LEDRED, GPIO_PIN_RESET);			// rode led is route opslaan
				HAL_GPIO_WritePin(GPIOD, LEDGREEN, GPIO_PIN_SET);			// groene led is drive mode
				xSemaphoreGive(hGNRMC_Struct_Sem);

				start = xTaskGetTickCount();								// als de drive_task wordt gestart wordt de begintijd opgeslagen
			}
		}
		if(key == 0x000D)
			PrintLog();

		xTaskNotify(hARM_keys, key, eSetValueWithOverwrite); // notify task2 with value
	}
}


/**
* @brief Task krijgt ARM-key met notificatie binnen, en zet ledjes op die waarde.
* Ook de gekleurde ledjes (behalve blauw, die wordt door de timer gebruikt) krijgen
* een schwung...
* @param *argument Niet gebruikt, eventueel een waarde of string om te testen.
* @return void.
*/
void ARM_keys_task (void *argument)
{
	uint32_t 	 key;
	//int			 i, led;

	while(TRUE)
	{
		// WAITING FOR users key
        xTaskNotifyWait (0x00,      		// Don't clear any notification bits on entry.
        		 	 	 0xffffffff, 		// ULONG_MAX, reset the notification value to 0 on exit.
    	                 &key, 				// Notified value.
    	                 portMAX_DELAY);  	// Block indefinitely.

	    xSemaphoreTake(hLED_Sem, portMAX_DELAY); // krijg toegang (mutex) tot leds

    	LED_put((unsigned char)key); // set 8 leds-byte to key-value
		osDelay(500);

		if (Uart_debug_out & ARMKEYS_DEBUG_OUT)
		{
			UART_puts("\r\n\tARM_key pressed to leds: "); UART_putint(key);
		}

	    xSemaphoreGive(hLED_Sem); // geef toegang (mutex) vrij

     	taskYIELD(); // done, force context switch
	}
}

