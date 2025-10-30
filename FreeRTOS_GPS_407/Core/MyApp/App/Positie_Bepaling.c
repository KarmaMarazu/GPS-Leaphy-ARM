/**
* @file Positie_bepaling.c
* @brief Deze file bevat de functies en de task drive_task. <BR>
* De leaphy moet zich richting de waypoint bewegen op basis van zijn huidige positie in richting.<BR>
* Dit wordt gedaan door een vector op te stellen van de huidige richting van de leaphy en deze te vergelijken met de vector naar de waypoint.
* @author Twan Ton, Mika Dahlkamp, Thomas van Ooijen en Jasper Verduin
*
* @date 1/10/2025.
*/



#include <admin.h>
#include "main.h"
#include "cmsis_os.h"
#include "gps.h"
#include <math.h>

Vector vector;

#define PI 3.1415926535
#define r_aarde 6371000
#define Waypoint_Drempel 3
#define COURSEINDEXDREMPEL 3

int WaypointIndex = 0; // globale teller voor de behaalde waypoints
int GemCourseIndex = 0;
int TotaleCourse = 0;

void ResetCourseIndex(void)
{
	GemCourseIndex = 0;
	TotaleCourse = 0;
}

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
int Afstand_Course_Bepalen(void)
{
	double radPosLong = DtoR(GNRMC_data.longitude);
	double radPosLati = DtoR(GNRMC_data.latitude);

	double radWayLong = DtoR(waypoints[WaypointIndex].longitude);
	double radWayLati = DtoR(waypoints[WaypointIndex].latitude);

	// Equirectangular approximation toepassen om de afstand tot waypoint en course te vinden naar waypoint
	// De gebruikte formules zijn te vinden op "https://www.movable-type.co.uk/scripts/latlong.html"
	// Omdat de afstanden tussen de punten relatief klein zijn zou de bolling van de aarde ook verwaarloosd worden
	double x_l = (radWayLong-radPosLong) * cos((radWayLati+radPosLati)/2);
	double y_l = radWayLati-radPosLati;
	vector.lengte = r_aarde * sqrt(x_l*x_l + y_l*y_l);

	// Bearing vanaf de leaphy richting de waypoint berekenen
	// De gebruikte formules zijn te vinden op "https://www.movable-type.co.uk/scripts/latlong.html"
	double deltaLong = radWayLong - radPosLong;
	double y_c = sin(deltaLong) * cos(radWayLati);
	double x_c = cos(radPosLati) * sin(radWayLati) - sin(radPosLati) * cos(radWayLati) * cos(deltaLong);
	vector.course = (int)(fmod(RtoD(atan2(y_c, x_c)) + 360.0, 360.0));

	// Print voor het testen
	UART_puts("\r\rAfstand tussen huidige positie en waypoint = "); UART_putint((int)vector.lengte);
	//UART_puts("\rCourse Richting waypoint vanaf huidige positie = "); UART_putint((int)vector.course);
	//UART_puts("\rlongi = "); UART_putint((int)(Gem.longitude*100000));
	//UART_puts("\rlati = "); UART_putint((int)(Gem.latitude*100000));
	//UART_puts("\rWPlongi = "); UART_putint((int)(waypoints[l].longitude));
	//UART_puts("\rWPlati = "); UART_putint((int)(waypoints[l].latitude));
	//UART_puts("\rHuidige Course = "); UART_putint((int)GNRMC_data.course);
	if(vector.lengte < Waypoint_Drempel)
		return 1;
	return 0;
}


/**
* @brief Functie om te bepalen wat de actie van de leaphy moet zijn op basis van de eerder berekende course naar de waypoint en de huidige course.
* @param int afstand gelezen door de sensor
* @return void
*/
char Leaphy_Actie_Bepalen(void)
{
	//return 0x01;
	//UART_puts("\rAfstand = "); UART_putint(distance);
	if(distance < 40)
	{
		ResetCourseIndex();
		return 0x06;
	}

	if(GNRMC_data.course && GemCourseIndex < COURSEINDEXDREMPEL)
	{
		TotaleCourse += GNRMC_data.course;
		GemCourseIndex++;
		UART_puts("\rGemCourseIndex = "); UART_putint(GemCourseIndex);
		if(GemCourseIndex < COURSEINDEXDREMPEL)
			return 0x01;
	}
	else if(!(GNRMC_data.course) && GemCourseIndex < COURSEINDEXDREMPEL)
		return 0x01;
	if(GemCourseIndex >= COURSEINDEXDREMPEL)
	{
		TotaleCourse = TotaleCourse / GemCourseIndex;
		UART_puts("\rTotaleCourse = "); UART_putint(TotaleCourse);
	}

	int course = ((int)TotaleCourse + 360) % 360;
	UART_puts("\rHuidigeCourse = "); UART_putint(course);
	UART_puts("\rVectorCourse = "); UART_putint(vector.course);

	int courseDiff = vector.course - course;
	UART_puts("\rCourseDiff = "); UART_putint(courseDiff);
	// Bepaal aan de hand van het verschil in course hoe erg er gecorrigeerd moet worden.

	int absDiff = abs(courseDiff);
	if(courseDiff < 0)
	{
		if(absDiff >= 180)
			return 0x03;			// Snel Rechts
		if(absDiff < 10)
			return 0x01;		// Rechtdoor
		if(absDiff < 150)
			return 0x04;		// Links
		return 0x05;			// Snel Links
	}
	else if(courseDiff > 0)
	{
		if(courseDiff >= 180)
			return 0x05;			// Snel Links
		if(courseDiff < 10)
			return 0x01;		// Rechtdoor
		if(courseDiff < 150)
			return 0x02;		// Rechts
		return 0x03;			// Snel Rechts
	}
	return 0x01;
}

/**
* @brief Functie om de pins aan te sturen die de arduino op de leaphy kan uitlezen.
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
* @brief Functie om gemiddelde van 3 datapunten op de slaan voor nauwkeurigere locatie alleen de coordinaten want course kan ook 0 zijn waardoor het gemiddelde niet werkt.
* @return void
*//*
void Average_Bepalen_Drive(void)
{
	// Gemiddelde nemen van de laatste 3 ingekomen gps berichten
	Gem.latitude = (average[0].latitude + average[1].latitude + average[2].latitude)/3; 	// gemiddelde wordt berekend en opgeslagen
	Gem.longitude = (average[0].longitude + average[1].longitude + average[2].longitude)/3;
}*/

/**
* @brief Deze drive_task moet worden gestart als knopje hiervoor wordt ingedrukt.<BR>
* Dan moet de leaphy de waypoints volgens zonder tegen een muur aan te botsen.
* @return void
*/
void drive_task(void* argument)
{
	while(TRUE)
	{
		ulTaskNotifyTake(0x00, portMAX_DELAY);
		xSemaphoreTake(hGNRMC_Struct_Sem, portMAX_DELAY);

		//Average_Bepalen_Drive();
		int WPBehaald = Afstand_Course_Bepalen();

		xSemaphoreGive(hGNRMC_Struct_Sem);

		if((WPBehaald == 1) && (WaypointIndex <= HoeveelheidWaypoints) && (HoeveelheidWaypoints > 0)) // als WPBehaald 1 is wordt de teller l verhoogd om het volgende waypoint aantegeven voor Afstand_course_Bepalen
		{
			WaypointIndex++;
			ResetCourseIndex();
			if(WaypointIndex >= HoeveelheidWaypoints)
			{
				LCD_clear();
				LCD_put("Alle Waypoints behaald yay");
				xEventGroupSetBits(hKEY_Event, 0x0004);
				continue;
			}
		}
		Leaphy_Data_Sturen(Leaphy_Actie_Bepalen());

		if(GemCourseIndex >= COURSEINDEXDREMPEL)
		{
			osDelay(1000);
			ResetCourseIndex();
		}

		LCD_clear(); 						// LCD legen
		LCD_putint(WaypointIndex); 			// waypoint nummer op LCD
		LCD_put("/");
		LCD_putint(HoeveelheidWaypoints);
		LCD_put(" Behaald");
		LCD_put("    Afstand = ");
		LCD_putint((int)vector.lengte);

		osDelay(50);
	}
}
