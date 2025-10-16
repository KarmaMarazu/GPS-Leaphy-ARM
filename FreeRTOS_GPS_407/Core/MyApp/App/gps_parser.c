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

GNRMC gnrmc; 						  // global struct for GNRMC-messages
Data_Parser GNRMC_data;
Data_Parser Gem;					  // struct om huidige positie in op te slaan maar dan als gemiddelde van 3 punten
Data_Parser average[3];				  // struct om een gemiddelde van de opgeslagen data te nemen als waypoint.
Data_Parser waypoints[MAX_WAYPOINTS]; // struct waar in nuttige data in bruikbare cijfers wordt gezet.
Data_Log Log;

/**
* @brief functie om gemiddelde van 3 datapunten op de slaan voor nauwkeurigere locatie.
* @param int i
* @return void
*/
void Average_Bepalen_Waypoints(int i)
{
	waypoints[i].latitude = (average[0].latitude + average[1].latitude + average[2].latitude)/3; 	//gemiddelde wordt berekend en opgeslagen
	waypoints[i].longitude = (average[0].longitude + average[1].longitude + average[2].longitude)/3;
	waypoints[i].speed = (average[0].speed + average[1].speed + average[2].speed)/3;
}


/**
* @brief De chars van de ingevulde GNRMC struct moeten omgezet worden naar rekenbare cijfers
* om van chars naar doubles te gaan gebruik je atof().<BR>
* Vervolgens wordt deze data in een Data_Parser struct gestopt.
* Deze struct wordt door een teller in een nieuwe array gezet om hiervan het gemiddelde uit te rekenen.
* @return void
*/
void GNRMC_Parser(void* argument)
{
	int i = 0; 												// teller voor schrijven naar average array voor nauwkeurigere waypoint opslaan
	char val1[10];
	char val2[10];

	while(TRUE)
	{
		xTaskNotifyWait(0,0, NULL, portMAX_DELAY);			// wacht op notify van fill_GNRMC in gps.c

		xSemaphoreTake(hGNRMC_Struct_Sem, portMAX_DELAY); 	// wacht op toegang tot de mutex;

		if(!(gnrmc.status == 'A'))							// status check
		{
			GNRMC_data.status = 'V';
			xSemaphoreGive(hGNRMC_Struct_Sem);
			continue;
		}

		memset(&GNRMC_data, 0, sizeof(Data_Parser)); 		// clear the struct
		memset(val1, 0, sizeof(val1));						// val1 en val2 clearen
		memset(val2, 0, sizeof(val2));

		memcpy(val1, gnrmc.latitude, 2);					// val1 en val2 vullen val1 krijgt eerste 2 karakters en val2 krijgt 3 t/m 8
		memcpy(val2, gnrmc.latitude+2, 8);
		GNRMC_data.latitude = (atof(val1))+(atof(val2)/60);	// atof conversie bij val1 en val2, val2 nog delen door 60 om tot juiste conversie te komen
		memset(val1, 0, sizeof(val1));						// val1 en val2 clearen
		memset(val2, 0, sizeof(val2));
		memcpy(val1, gnrmc.longitude, 3);					// val1 en val2 vullen val1 krijgt eerste 3 karakters en val2 krijgt 4 t/m 8
		memcpy(val2, gnrmc.longitude+3, 8);
		GNRMC_data.longitude = (atof(val1))+(atof(val2)/60);// atof conversie bij val1 en val2, val2 nog delen door 60 om tot juiste conversie te komen
		GNRMC_data.speed = atof(gnrmc.speed)*0.514444;		// speed omzetten van knopen naar m/s
		GNRMC_data.course = atof(gnrmc.course);
		GNRMC_data.status = 'A';							// Valid status meegeven voor later gebruik

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

		if (i<MAX_WAYPOINTS && key == 0x0001)
		{
		    xSemaphoreTake(hGNRMC_Struct_Sem, portMAX_DELAY);			// wacht op toegang tot de mutex;

		    if(!(GNRMC_data.status == 'A'))								// status check
		    {
		    	LCD_clear(); 											// LCD legen
		    	LCD_putint(i); 											// waypoint nummer op LCD
		    	LCD_put("/30 waypoints Geen Data");
		    	xSemaphoreGive(hGNRMC_Struct_Sem);
		    	continue;
		    }
		    Average_Bepalen_Waypoints(i);
		    xSemaphoreGive(hGNRMC_Struct_Sem); 							// wacht op toegang tot de mutex;

			i++;

		    LCD_clear(); 												// LCD legen
		    LCD_putint(i); 												// waypoint nummer op LCD
		    LCD_put("/30 waypoints");
		    osDelay(100);												// spam beveiliging
		}
		else if (i == MAX_WAYPOINTS)									// het maximaal aantal waypoints is bereikt
		{
			 LCD_put("Limit reached!");
			 osDelay(100);												// zonder dit loopt de lcd vast
			 i++;														// ga hierna uit de if statement zodat niet heel de lcd vol komt te staan
		}
	}
}

