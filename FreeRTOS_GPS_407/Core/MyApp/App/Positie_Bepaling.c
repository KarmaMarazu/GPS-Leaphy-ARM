#include <admin.h>
#include "main.h"
#include "cmsis_os.h"
#include "gps.h"
#include <math.h>

extern GNRMC gnrmc;
extern Data_Parser GNRMC_data;
extern Data_Parser waypoints[MAX_WAYPOINTS];

// Struct voor het bijelkaar houden van de vector tussen Leaphy en Waypoint
typedef struct Vector
{
	double lengte;
	double course;
} Vector;

Vector vector;

#define PI 3.1415926535

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
	double radPosLong = DtoR(GNRMC_data.longitude);
	double radPosLati = DtoR(GNRMC_data.latitude);
	double radWayLong = DtoR(waypoints[0].longitude); // 0 later vervangen met het te zoeken waypoint
	double radWayLati = DtoR(waypoints[0].latitude);

	// Equirectangular approximation toepassen om de afstand en course te vinden
	//Omdat de afstand niet gigantisch is kan dit nog.
	double x = (radPosLong-radWayLong) * cos((radWayLati+radPosLati)/2);
	double y = radPosLati-radWayLati;
	vector.lengte = 6371000 * sqrt(x*x + y*y);
	//double radCoG = atan2(x, y);
	vector.course = fmod(RtoD(atan2(x, y)) + 360.0, 360.0);

	// Print voor het testen
	UART_puts("\nAfstand tussen huidige positie en waypoint = "); UART_putint((int)vector.lengte);
	UART_puts("\nCourse Richting waypoint vanaf huidige positie = "); UART_putint((int)vector.course);
}
