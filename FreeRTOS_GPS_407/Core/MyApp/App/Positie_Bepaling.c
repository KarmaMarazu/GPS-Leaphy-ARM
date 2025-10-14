#include <admin.h>
#include "main.h"
#include "cmsis_os.h"
#include "gps.h"
#include <math.h>

Vector vector;

#define PI 3.1415926535
#define r_aarde 6371000

// Graden naar radialen
double DtoR(double Graden)
{
	return(PI * Graden / 180);
}

// Radialen naar graden
double RtoD(double Radialen)
{
    return (Radialen * 180.0 / PI);
}

/**
* @brief Functie voor het bepalen van de afstand tussen huidige positie en waypoint. <BR>
* Eigelijk ben je aan het pythagorassen maar dan met de extra stap van de bolling van de aarde.
* @return void
*/
void Afstand_Course_Bepalen(void)
{
	double radPosLong = DtoR(Gem.longitude);
	double radPosLati = DtoR(Gem.latitude);
	double radWayLong = DtoR(waypoints[0].longitude); // 0 later vervangen met het te zoeken waypoint
	double radWayLati = DtoR(waypoints[0].latitude);

	// Equirectangular approximation toepassen om de afstand tot waypoint en course te vinden naar waypoint
	// Omdat de afstanden tussen de punten relatief klein zijn zou de bolling van de aarde ook verwaarloosd worden
	double x = (radWayLong-radPosLong) * cos((radWayLati+radPosLati)/2);
	double y = radWayLati-radPosLati;
	vector.lengte = r_aarde * sqrt(x*x + y*y);
	vector.course = fmod(RtoD(atan2(x, y)) + 360.0, 360.0);

	// Print voor het testen
	UART_puts("\r\rAfstand tussen huidige positie en waypoint = "); UART_putint((int)vector.lengte);
	UART_puts("\rCourse Richting waypoint vanaf huidige positie = "); UART_putint((int)vector.course);
	UART_puts("\rHuidige Course = "); UART_putint((int)GNRMC_data.course);
}


/**
* @brief functie om te bepalen wat de actie van de leaphy moet zijn op basis van de eerder berekende course naar de waypoint en de huidige course.
* @param int afstand gelezen door de sensor
* @return void
*/
char Leaphy_Actie_Bepalen(int)
{
	if(!(GNRMC_data.course))
		return 0x0F;

	double courseDiff;

	// Deze statements zorgen ervoor dat er geen negatieve komen door de kleine waarde - de grote waarde te doen.
	if(vector.course > GNRMC_data.course)
		courseDiff = vector.course - GNRMC_data.course;
	else if(vector.course < GNRMC_data.course)
		courseDiff = GNRMC_data.course - vector.course;
	else if(vector.course == GNRMC_data.course)
		courseDiff = 0;

	// Bepaal aan de hand van het verschil in course hoe erg er gecorrigeerd moet worden.
	UART_puts("\rCourseDiff = "); UART_putint((int)courseDiff);
	if(courseDiff >= -25 && courseDiff <= 25) 		// Rechtdoor snel
		return 0x01;
	if(courseDiff >= 25 && courseDiff <= 90) 		// langzame bocht rechts
		return 0x02;
	if(courseDiff >= 90 && courseDiff <= 180) 		// snelle bocht rechts
		return 0x03;
	if(courseDiff <= -25 && courseDiff >= -90) 		// langzame bocht links
		return 0x04;
	if(courseDiff >= -90 && courseDiff <= -180) 	// snelle bocht links
		return 0x05;

	return 0x00;
}

/**
* @brief functie om de pins aan te sturen die de arduino op de leaphy kan uitlezen.
* @param char data die naar de arduino gestuurd moet worden.
* @return void
*/
void Leaphy_Data_Sturen(char data)
{
	// 4 bits data naar de Arduino sturen. Bits worden bepaald afhankelijk van de functie Leaphy_actie_Bepalen
	if(0b0001 & data)
		HAL_GPIO_WritePin(GPIOE, Ard_Bit1_Pin, SET);
	else
		HAL_GPIO_WritePin(GPIOE, Ard_Bit1_Pin, RESET);
	if(0b0010 & data)
		HAL_GPIO_WritePin(GPIOE, Ard_Bit2_Pin, SET);
	else
		HAL_GPIO_WritePin(GPIOE, Ard_Bit2_Pin, RESET);
	if(0b0100 & data)
		HAL_GPIO_WritePin(GPIOE, Ard_Bit3_Pin, SET);
	else
		HAL_GPIO_WritePin(GPIOE, Ard_Bit3_Pin, RESET);
	if(0b1000 & data)
		HAL_GPIO_WritePin(GPIOE, Ard_Bit4_Pin, SET);
	else
		HAL_GPIO_WritePin(GPIOE, Ard_Bit4_Pin, RESET);

	// print voor het testen
	switch(data)
	{
	case 0x01:	UART_puts("\rRechtdoor");
				break;
	case 0x02:	UART_puts("\rLangzaam naar rechts");
				break;
	case 0x03:	UART_puts("\rSnel naar rechts");
				break;
	case 0x04:	UART_puts("\rLangzaam naar links");
				break;
	case 0x05:	UART_puts("\rSnel naar links");
				break;
	case 0x0F:  UART_puts("\rGeen course data beschikbaar");
				break;
	default:	UART_puts("\rError kan geen keuze maken");
				break;
	}
}

/**
* @brief functie om gemiddelde van 3 datapunten op de slaan voor nauwkeurigere locatie.
* @return void
*/
void Average_Bepalen_Drive(void)
{
	// Gemiddelde nemen van de laatste 3 ingekomen gps berichten
	xSemaphoreTake(hGNRMC_Struct_Sem, portMAX_DELAY);										// neem de mutex zodat average[] thread safe blijft
	Gem.latitude = (average[0].latitude + average[1].latitude + average[2].latitude)/3; 	// gemiddelde wordt berekend en opgeslagen
	Gem.longitude = (average[0].longitude + average[1].longitude + average[2].longitude)/3;
	Gem.speed = (average[0].speed + average[1].speed + average[2].speed)/3;
	xSemaphoreGive(hGNRMC_Struct_Sem);														// geef de mutex op average[] weer terug
}


/**
* @brief Deze drive_task moet worden gestart als knopje hiervoor wordt ingedrukt.<BR>
* Dan moet de leaphy de waypoints volgens zonder tegen een muur aan te botsen.
* @return void
*/
void drive_task(void*)
{
	while(TRUE)
	{
		Average_Bepalen_Drive();
		Afstand_Course_Bepalen();
		UART_puts("\rAfstand = "); UART_putint((int)GetDistance()); // tijdelijke print voor de HC-SR04 sensor
		char Data = Leaphy_Actie_Bepalen(1); // 1 moet vervangen worden met GetDistance() zodra deze daarvoor klaar is
		Leaphy_Data_Sturen(Data);
		osDelay(200);						// tijdelijke delay
	}
}
