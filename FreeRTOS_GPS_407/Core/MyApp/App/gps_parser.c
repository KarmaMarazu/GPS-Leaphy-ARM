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

Data_Parser average[3];
Data_Parser waypoints[MAX_WAYPOINTS]; //{latitude,  longitude,  speed, course}

/**
* @brief De chars van de ingevulde GNRMC struct moeten omgezet worden naar rekenbare cijfers
* om van chars naar doubles te gaan gebruik je atof().<BR>
* Vervolgens wordt deze data in een Data_Parser struct gestopt
* @return void
*/
void GNRMC_Parser(void* argument)
{
	int i = 0; //teller voor schrijven naar average array voor nauwkeurigere waypoint opslaan

	while(TRUE)
	{
		xTaskNotifyWait(0,0, NULL, portMAX_DELAY);
		if(!(gnrmc.status == 'A'))
			continue;

		xSemaphoreTake(hGNRMC_Struct_Sem, portMAX_DELAY); // wacht op toegang tot de mutex;
		memset(&GNRMC_data, 0, sizeof(Data_Parser)); // clear the struct

		GNRMC_data.latitude = atof(gnrmc.latitude);
		GNRMC_data.longitude = atof(gnrmc.longitude);
		GNRMC_data.speed = atof(gnrmc.speed);
		GNRMC_data.course = atof(gnrmc.course);

		average[i] = GNRMC_data;

		xSemaphoreGive(hGNRMC_Struct_Sem); // geef de mutex weer vrij voor een ander

		i++;
		if(i >= 3)
			i = 0;
	}
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

			waypoints[i].latitude = (average[0].latitude + average[1].latitude + average[2].latitude)/3; //array vullen met struct
			waypoints[i].longitude = (average[0].longitude + average[1].longitude + average[2].longitude)/3; //array vullen met struct
			waypoints[i].speed = (average[0].speed + average[1].speed + average[2].speed)/3; //array vullen met struct
			waypoints[i].course = (average[0].course + average[1].course + average[2].course)/3; //array vullen met struct

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
