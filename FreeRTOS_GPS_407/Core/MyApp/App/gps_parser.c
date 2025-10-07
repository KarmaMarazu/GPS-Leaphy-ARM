/**
* @file gps_parser.c
* @brief Veranderd de data die binnen komt in gps.c zodat dit bruikbare cijfers worden. <BR>
* Bevat functie die het opslaan van waypoints regelt.
* De waypoint die wordt opgeslagen is het gemiddelde van de 3 laatst opgeslagen gps punten.
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

Data_Parser average[3];				  // struct om een gemiddelde van de opgeslagen data te nemen als waypoint.
Data_Parser waypoints[MAX_WAYPOINTS]; // struct waar in nuttige data in bruikbare cijfers wordt gezet.

/**
* @brief De chars van de ingevulde GNRMC struct moeten omgezet worden naar rekenbare cijfers
* om van chars naar doubles te gaan gebruik je atof().<BR>
* Vervolgens wordt deze data in een Data_Parser struct gestopt.
* Deze struct wordt door een teller in een nieuwe array gezet om hiervan het gemiddelde uit te rekenen.
* @return void
*/
void GNRMC_Parser(void* argument)
{
	int i = 0; // teller voor schrijven naar average array voor nauwkeurigere waypoint opslaan

	while(TRUE)
	{
		xTaskNotifyWait(0,0, NULL, portMAX_DELAY);			// wacht op notify van fill_GNRMC in gps.c
		if(!(gnrmc.status == 'A'))							// status check
			continue;

		xSemaphoreTake(hGNRMC_Struct_Sem, portMAX_DELAY); 	// wacht op toegang tot de mutex;
		memset(&GNRMC_data, 0, sizeof(Data_Parser)); 		// clear the struct

		GNRMC_data.latitude = atof(gnrmc.latitude);
		GNRMC_data.longitude = atof(gnrmc.longitude);
		GNRMC_data.speed = atof(gnrmc.speed);
		GNRMC_data.course = atof(gnrmc.course);

		average[i] = GNRMC_data;							// zet data round robin in average[].

		xSemaphoreGive(hGNRMC_Struct_Sem); 					// geef de mutex weer vrij voor een ander

		i++;
		if(i >= 3)											// als average[] vol is, dan opnieuw beginnen met vullen
			i = 0;
	}
}

/**
* @brief het gemiddelde van wat er in average[] staat wordt opgeslagen met een knop op het ARM-bordje voor meer nauwkeurigheid.<BR>
* Ook kan deze opgeslagen waypoints gerest kunnen worden met een andere knop. Deze knop krijgt de task door van ARM_keys_IRQ
* @return void
*/
void data_opslaanTask(void *argument)
{
	uint32_t key;
	int i = 0;
	while(TRUE)
	{
		xTaskNotifyWait (0x00, 0xffffffff, &key, portMAX_DELAY);		// wacht op ARM_keys_IRQ

		if(key == 0x0002) 												// als resetknop is ingedrukt dan moet de arrat geleegd worden en de teller moet op 0
		{
			memset(&waypoints, 0, sizeof(waypoints));
			i = 0;
			LCD_clear(); 												// LCD legen
			LCD_putint(i); 												// waypoint nummer op LCD
			LCD_put("/30 waypoints");
			continue;													// begin boven aan en wacht weer op een arm-key
		}

		if (i<MAX_WAYPOINTS)
		{
		    xSemaphoreTake(hGNRMC_Struct_Sem, portMAX_DELAY);			// wacht op toegang tot de mutex;

			waypoints[i].latitude = (average[0].latitude + average[1].latitude + average[2].latitude)/3; 	//gemiddelde wordt berekend en opgeslagen
			waypoints[i].longitude = (average[0].longitude + average[1].longitude + average[2].longitude)/3;
			waypoints[i].speed = (average[0].speed + average[1].speed + average[2].speed)/3;
			waypoints[i].course = (average[0].course + average[1].course + average[2].course)/3;

		    xSemaphoreGive(hGNRMC_Struct_Sem); 							// wacht op toegang tot de mutex;

			i++;

		    LCD_clear(); 												// LCD legen
		    LCD_putint(i); 												// waypoint nummer op LCD
		    LCD_put("/30 waypoints");
		    osDelay(100);												// spam beveiliging


		} else if (i == MAX_WAYPOINTS)									// het maximaal aantal waypoints is bereikt
		{
			 LCD_put("Limit reached!");
			 osDelay(100);												// zonder dit loopt de lcd vast
			 i++;														// ga hierna uit de if statement zodat niet heel de lcd vol komt te staan
		}
	}
}

