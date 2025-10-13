/**
* @file ARM_keys.c
* @brief Behandelt de communicatie met de ARM-toetsjes met: Eventgroups, TaskNotify, Interrupt-handling.<br>
* <b>Demonstreert: xEventGroupWaitBits(), xTaskGetHandle(), xTaskNotify(), xTaskNotifyWait(),xSemaphoregive(), xSemaphoreTake(). </b><br>
*
* Aan de ARM-keys is een interrupt gekoppeld (zie stm32f4xx_it.c). Die stuurt een event
* door die opgevangen wordt door task ARM_keys_IRQ().
* @author MSC
*
* @date 5/5/2022
*/

#include <admin.h>
#include "main.h"
#include "cmsis_os.h"


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
* @brief Deze task handelt de ARM-toets af, die ontvangen is van de ISR-handler (zie: stm32f4xx_it.c).
* @param *argument Niet gebruikt, eventueel een waarde of string om te testen
* @return void.
*/
/**
* @brief Deze task handelt de ARM-toets af, die ontvangen is van de ISR-handler (zie: stm32f4xx_it.c).
* @param *argument Niet gebruikt, eventueel een waarde of string om te testen
* @return void.
*/
void ARM_keys_IRQ (void *argument)
{
	unsigned int key;
	unsigned int j = 0;
	osThreadId_t hARM_keys;
	osThreadId_t hData_opslaanTask;
	TaskStatus_t    TaskDetails;

	UART_puts("\r\n"); UART_puts((char *)__func__); UART_puts(" started");

	vTaskSuspend(GetTaskhandle("drive_task"));					// stopt de drivetask

	if (!(hARM_keys = GetTaskhandle("ARM_keys_task")))
		error_HaltOS("Err:ARM_hndle");
	if (!(hData_opslaanTask = GetTaskhandle("data_opslaanTask")))
		error_HaltOS("Err:data_opslaan handle");

    while (1)
	{

		// wait for ISR (EXTI0_IRQHandler()) to signal that a key is pressed
		key = xEventGroupWaitBits(hKEY_Event, 0xffff, pdTRUE, pdFALSE, portMAX_DELAY );

		vTaskGetInfo(GetTaskhandle("data_opslaanTask"), &TaskDetails, pdFALSE, eInvalid);

		if (((key == 0x0001) || (key == 0x0002)) && ((TaskDetails.eCurrentState) != eSuspended) )
			xTaskNotify(hData_opslaanTask, key, eSetValueWithOverwrite); // notify Data_opslaanTask with value

		if (key == 0x0004)
		{
			j++;

			if(j%2 == 0)
			{
				xSemaphoreTake(hGNRMC_Struct_Sem, portMAX_DELAY);
				vTaskResume(GetTaskhandle("data_opslaanTask"));				// start de waypoints opslaan task
				vTaskSuspend(GetTaskhandle("drive_task"));					// stopt de drivetask
				HAL_GPIO_WritePin(GPIOD, LEDRED, GPIO_PIN_SET);
				HAL_GPIO_WritePin(GPIOD, LEDGREEN, GPIO_PIN_RESET);
				xSemaphoreGive(hGNRMC_Struct_Sem);
			}
			else if(j%2 == 1)
			{
				xSemaphoreTake(hGNRMC_Struct_Sem, portMAX_DELAY);
				vTaskSuspend(GetTaskhandle("data_opslaanTask"));			// start de waypoints opslaan task
				vTaskResume(GetTaskhandle("drive_task"));					// start de drivetask
				HAL_GPIO_WritePin(GPIOD, LEDRED, GPIO_PIN_RESET);
				HAL_GPIO_WritePin(GPIOD, LEDGREEN, GPIO_PIN_SET);
				xSemaphoreGive(hGNRMC_Struct_Sem);
			}
		}


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
	    //BUZZER_put (500);
		osDelay(500);

		if (Uart_debug_out & ARMKEYS_DEBUG_OUT)
		{
			UART_puts("\r\n\tARM_key pressed to leds: "); UART_putint(key);
		}

	    xSemaphoreGive(hLED_Sem); // geef toegang (mutex) vrij

	    // tot slot, laat de gekleurde ledjes meedoen
	    // maar niet blauw, want die is ingezet voor de timer
	    // kijk naar de manier waarop de if-elses er uitzien
		/*for (i=0; i<3; i++)
		{
			led = (i==0 ? LEDRED : (i==1 ? LEDORANGE : LEDGREEN));
			toggle_led(led);
	  	}*/
     	taskYIELD(); // done, force context switch
	}
}

