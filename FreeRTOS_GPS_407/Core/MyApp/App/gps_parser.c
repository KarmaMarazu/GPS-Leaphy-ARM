/**
* @file gps_parser.c
* @brief Veranderd de data die binnen komt in gps.c zodat dit bruikbare cijfers worden. <BR>
* Bevat functie die het opslaan van waypoints regelt
* @author Twan Ton, Mika Dahlkamp, Thomas van Ooijen en Jasper Verduin
*
* @date 1/10/2025.
*/

#include <admin.h>
#include "main.h"
#include "cmsis_os.h"
#include "gps.h"

extern GNRMC gnrmc; // global struct for GNRMC-messages
Data_Parser GNRMC_data;

Data_Parser waypoints[MAX_WAYPOINTS]; //{latitude,  longitude,  speed, course}

/**
* @brief De chars van de ingevulde GNRMC struct moeten omgezet worden naar rekenbare cijfers
* om van chars naar doubles te gaan gebruik je atof().<BR>
* Vervolgens wordt deze data in een Data_Parser struct gestopt
* @return void
*/
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

/**
* @brief de opgeslagen rekenbare data kan worden opgeslagen met een knop op het ARM-bordje.<BR>
* Ook kan deze opgeslagen waypoints gerest kunnen worden met een andere knop. Deze knop krijgt de task door van ARM_keys_IRQ
* @return void
*/
void data_opslaanTask(void *argument)
{
	uint32_t key;
	int i = 1;
	while(TRUE)
	{
		xTaskNotifyWait (0x00, 0xffffffff, &key, portMAX_DELAY);

		if(key == 0x0002) // als resetknop is ingedrukt dan moet de arrat geleegd worden en de teller moet op 0
		{
			memset(&waypoints, 0, sizeof(waypoints));
			i = 0;
		}

		if (i<MAX_WAYPOINTS)
		{
		    LCD_clear(); //LCD legen
		    LCD_putint(i); // waypoint nummer op LCD
		    LCD_put("/30 waypoints");
		    osDelay(100);

		    xSemaphoreTake(hGNRMC_Struct_Sem, portMAX_DELAY); // wacht op toegang tot de mutex;

			waypoints[i] = GNRMC_data; //array vullen met struct

		    xSemaphoreGive(hGNRMC_Struct_Sem); // wacht op toegang tot de mutex;

			i++;
		} else if (i == MAX_WAYPOINTS)
		{
			 LCD_put("Limit reached!");
			 osDelay(100);
			 i++;
		}
	}
}
