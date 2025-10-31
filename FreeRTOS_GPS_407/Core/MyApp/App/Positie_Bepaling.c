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

extern TIM_HandleTypeDef htim8;

int WaypointIndex = 0; // globale teller voor de behaalde waypoints
int GemCourseIndex = 0;
int TotaleCourse = 0;

// Resetten van de gemmidelde course die wordt berekend
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

	// Bepalen of de Leaphy op de waypoint is
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
	// Kijken of we binnen bereik zijn van de drempelwaarde met de HC-SR04
	if(distance < 40)
	{
		ResetCourseIndex();
		return 0x06;
	}

	// Als er course data is bijvoegen in totale course en index verhogen om een gemmidelde course te kunnen berekenen
	if(GNRMC_data.course && (GemCourseIndex < COURSEINDEXDREMPEL))
	{
		TotaleCourse += GNRMC_data.course;	// Afgelopen courses bij elkaar optellen
		GemCourseIndex++;

		if (Uart_debug_out & DRIVEMODE_DEBUG_OUT)
		{
			UART_puts("\rGemCourseIndex = "); UART_putint(GemCourseIndex);
		}

		// Als de drempel waarde van hoeveelheid punten nog niet overschreden is return 0x01
		if(GemCourseIndex < COURSEINDEXDREMPEL)
			return 0x01;
	}
	// Als er geen course data is rechtdoor bljven lopen tot er wel data is
	else if(!(GNRMC_data.course) && (GemCourseIndex < COURSEINDEXDREMPEL))
		return 0x01;

	// Gemmidelde course berekenen als sample size bereikt is
	if(GemCourseIndex >= COURSEINDEXDREMPEL)
	{
		TotaleCourse = TotaleCourse / GemCourseIndex;
	}

	int course = ((int)TotaleCourse + 360) % 360;	// Normaliseren van de course
	int courseDiff = vector.course - course;

	if (Uart_debug_out & DRIVEMODE_DEBUG_OUT)
		{
			UART_puts("\rHuidigeCourse = "); UART_putint(course);
			UART_puts("\rTotaleCourse = "); UART_putint(TotaleCourse);
			UART_puts("\rCourseDiff = "); UART_putint(courseDiff);
		}

	// Bepaal aan de hand van het verschil in course hoe erg er gecorrigeerd moet worden.
	int absDiff = abs(courseDiff);
	if(courseDiff < 0)
	{
		if(absDiff >= 180)
			return 0x03;		// Snel Rechts
		if(absDiff < 10)
			return 0x01;		// Rechtdoor
		if(absDiff < 150)
			return 0x04;		// Links
		return 0x05;			// Snel Links
	}
	else if(courseDiff > 0)
	{
		if(courseDiff >= 180)
			return 0x05;		// Snel Links
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
	if (Uart_debug_out & DRIVEMODE_DEBUG_OUT)
	{
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
}


/**
* @brief Deze drive_task moet worden gestart als knopje hiervoor wordt ingedrukt.<BR>
* Dan moet de leaphy de waypoints volgens zonder tegen een muur aan te botsen.
* @return void
*/
void drive_task(void* argument)
{
	HAL_TIM_PWM_Start(&htim8, TIM_CHANNEL_3);				// Starten PWM timer voor buzzer
	uint16_t timer_arr = __HAL_TIM_GET_AUTORELOAD(&htim8);	// ARR ophalen van de timer

	while(TRUE)
	{
		ulTaskNotifyTake(0x00, portMAX_DELAY);				// Task notify ontvangen of van GNRMC_Parser of GetDistance
		xSemaphoreTake(hGNRMC_Struct_Sem, portMAX_DELAY);

		// Buzzer aansturen als er geen course data is
		if(!(GNRMC_data.course))
		{
			// Buzzer aansturen
			__HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_3, timer_arr * 0.20f);
			osDelay(50);
			__HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_3, timer_arr * 0.00f);
		}
		int WPBehaald = Afstand_Course_Bepalen();			// Berekenen wat de afstand en course is vanaf de Leaphy naar de huidige waypoint en returnt 1 of 0 afhankelijk van waypoint behaald

		xSemaphoreGive(hGNRMC_Struct_Sem);

		// als WPBehaald 1 is wordt de teller l verhoogd om het volgende waypoint aantegeven voor Afstand_course_Bepalen
		if((WPBehaald == 1) && (WaypointIndex <= HoeveelheidWaypoints) && (HoeveelheidWaypoints > 0))
		{
			WaypointIndex++;
			ResetCourseIndex();

			// Buzzer aansturen
			__HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_3, timer_arr * 0.50f);
			osDelay(100);
			__HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_3, timer_arr * 0.00f);

			if(WaypointIndex >= HoeveelheidWaypoints)
			{
				LCD_clear();
				LCD_put("Alle Waypoints behaald");
				xEventGroupSetBits(hKEY_Event, 0x0004); // Knop simuleren om drive mode uiteschakelen als alle waypoints behaald zijn
				continue;
			}
		}

		Leaphy_Data_Sturen(Leaphy_Actie_Bepalen());	  	// Leaphy aansturen op basis van gemmidelde course, huidige locatie en waypoint

		// Resetten van de course index als de sample size is bereikt
		if(GemCourseIndex >= COURSEINDEXDREMPEL)
		{
			osDelay(90);
			ResetCourseIndex();
		}

		// Displayen van hoeveel waypoints behaald zijn en afstand tot volgende waypoint
		LCD_clear();
		LCD_putint(WaypointIndex);
		LCD_put("/");
		LCD_putint(HoeveelheidWaypoints);
		LCD_put(" Behaald");
		LCD_put("    Afstand = ");
		LCD_putint((int)vector.lengte);
	}
}
