#include <admin.h>
#include "main.h"
#include "cmsis_os.h"
#include "gps.h"
#include <math.h>

extern GNRMC gnrmc;
extern Data_Parser GNRMC_data;
extern Data_Parser waypoints[MAX_WAYPOINTS];
extern Data_Parser average[3];

Data_Parser Gem;

// Struct voor het bijelkaar houden van de vector tussen Leaphy en Waypoint
typedef struct Vector
{
	double lengte;
	double course;
} Vector;

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

// Functie voor het bepalen van de vector tussen de Leaphy en de waypoint
void Afstand_Course_Bepalen(void)
{
	double radPosLong = DtoR(Gem.longitude);
	double radPosLati = DtoR(Gem.latitude);
	double radWayLong = DtoR(waypoints[0].longitude); // 0 later vervangen met het te zoeken waypoint
	double radWayLati = DtoR(waypoints[0].latitude);

	// Equirectangular approximation toepassen om de afstand en course te vinden
	//Omdat de afstand niet gigantisch is kan dit nog.
	double x = (radWayLong-radPosLong) * cos((radWayLati+radPosLati)/2);
	double y = radWayLati-radPosLati;
	vector.lengte = r_aarde * sqrt(x*x + y*y);
	vector.course = fmod(RtoD(atan2(x, y)) + 360.0, 360.0);

	// Print voor het testen

	UART_puts("\r\rAfstand tussen huidige positie en waypoint = "); UART_putint((int)vector.lengte);
	UART_puts("\rCourse Richting waypoint vanaf huidige positie = "); UART_putint((int)vector.course);
	UART_puts("\rHuidige Course = "); UART_putint((int)GNRMC_data.course);
}

char Leaphy_Actie_Bepalen(int)
{
	if(!(GNRMC_data.course))
		return 0x0F;

	double courseDiff;
	if(vector.course > GNRMC_data.course)
		courseDiff = vector.course - GNRMC_data.course;
	else if(vector.course < GNRMC_data.course)
		courseDiff = GNRMC_data.course - vector.course;
	else if(vector.course == GNRMC_data.course)
		courseDiff = 0;

	UART_puts("\rCourseDiff = "); UART_putint((int)courseDiff);
	if(courseDiff >= -25 && courseDiff <= 25) // Rechtdoor snel
		return 0x01;
	if(courseDiff >= 25 && courseDiff <= 90) // langzame bocht rechts
		return 0x02;
	if(courseDiff >= 90 && courseDiff <= 180) // snelle bocht rechts
		return 0x03;
	if(courseDiff <= -25 && courseDiff >= -90) // langzame bocht links
		return 0x04;
	if(courseDiff >= -90 && courseDiff <= -180) // snelle bocht links
		return 0x05;

	return 0x00;
}

void Leaphy_Data_Sturen(char data)
{
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

void Average_Bepalen_Drive(void)
{
	xSemaphoreTake(hGNRMC_Struct_Sem, portMAX_DELAY);
	Gem.latitude = (average[0].latitude + average[1].latitude + average[2].latitude)/3; 	//gemiddelde wordt berekend en opgeslagen
	Gem.longitude = (average[0].longitude + average[1].longitude + average[2].longitude)/3;
	Gem.speed = (average[0].speed + average[1].speed + average[2].speed)/3;
	//Gem.course = (average[0].course + average[1].course + average[2].course)/3;
	xSemaphoreGive(hGNRMC_Struct_Sem);
}

void drive_task(void*)
{
	while(TRUE)
	{
		Average_Bepalen_Drive();
		Afstand_Course_Bepalen();
		UART_puts("\rAfstand = "); UART_putint((int)GetDistance());
		char Data = Leaphy_Actie_Bepalen(1);
		Leaphy_Data_Sturen(Data);
		osDelay(200);
	}
}
