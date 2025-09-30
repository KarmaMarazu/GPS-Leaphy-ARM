/*
 * gps_parser.c
 *
 *  Created on: Sep 25, 2025
 *      Author: Twan
 */

#include <admin.h>
#include "main.h"
#include "cmsis_os.h"
#include "gps.h"

extern GNRMC gnrmc; // global struct for GNRMC-messages
Data_Parser GNRMC_data;

Data_Parser waypoints[MAX_WAYPOINTS]; //{latitude,  longitude,  speed, course}


void GNRMC_Parser(void)
{
	if(!(gnrmc.status == 'A'))
		return;

    xSemaphoreTake(hGNRMC_Struct_Sem, portMAX_DELAY); // wacht op toegang tot de mutex;
    memset(&GNRMC_data, 0, sizeof(Data_Parser)); // clear the struct

	GNRMC_data.latitude = atof(gnrmc.latitude);
	GNRMC_data.longitude = atof(gnrmc.longitude);
	GNRMC_data.speed = atof(gnrmc.speed);
	GNRMC_data.course = atof(gnrmc.course);

	if (Uart_debug_out & GPS_DEBUG_OUT)
	 	{
			char lat[sizeof(GNRMC_data.latitude)];
	 		snprintf(lat,sizeof(GNRMC_data.latitude)+1, "%f", GNRMC_data.latitude);

	 		char lon[sizeof(GNRMC_data.longitude)];
	 		snprintf(lon, sizeof(GNRMC_data.longitude)+1, "%f", GNRMC_data.longitude);

	 		char spe[sizeof(GNRMC_data.speed)];
	 		snprintf(spe, sizeof(GNRMC_data.speed)+1, "%f", GNRMC_data.speed);

	 		char cou[sizeof(GNRMC_data.course)];
	 		snprintf(cou, sizeof(GNRMC_data.course)+1, "%f", GNRMC_data.course);

			UART_puts("\r\n\t latitude double:\t\t"); UART_puts(lat);
			UART_puts("\r\n\t longitude double:\t");  UART_puts(lon);
			UART_puts("\r\n\t speed double:    \t");  UART_puts(spe);
			UART_puts("\r\n\t course double:   \t");  UART_puts(cou);
	 	}

 	xSemaphoreGive(hGNRMC_Struct_Sem); // geef de mutex weer vrij voor een ander
}

void data_opslaanTask(void *argument)
{
	uint32_t key;
	int i = 0;
	while(TRUE)
	{
		xTaskNotifyWait (0x00, 0xffffffff, &key, portMAX_DELAY);
		if (i<=MAX_WAYPOINTS)
		{
		    xSemaphoreTake(hGNRMC_Struct_Sem, portMAX_DELAY); // wacht op toegang tot de mutex;

			waypoints[i] = GNRMC_data; //array vullen met struct

		    xSemaphoreGive(hGNRMC_Struct_Sem); // wacht op toegang tot de mutex;

			i++;
		}
	}
}
