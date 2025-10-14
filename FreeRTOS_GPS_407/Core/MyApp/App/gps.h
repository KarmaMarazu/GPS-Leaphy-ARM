/**
* @file my_gps.h
* @brief Bevat basic defines & externals voor de gps routines
* @attention
* <h3>&copy; Copyright (c) 2023 (HU)
* @author MSC, Twan Ton, Mika Dahlkamp, Jasper Verduin en Thomas van Ooijen
*
* @date 5/9/2023
*/
int hex2int(char *c);
int hexchar2int(char c);
int checksum_valid(char *string);

/// GNRMC struct: all with char-members - should/could be improved with proper data-elements
typedef struct _GNRMC
{
	char    head[7];       // 0. header
	char    time[11];      // 1. hhmmss.sss
	char    status;        // 2. A=valid, V=not valid
	char    latitude[13];  // 3. ddmm.mmmm (double)
	char    NS_ind;        // 4. N,S
	char    longitude[13]; // 5. ddmm.mmmm (double)
	char    EW_ind;        // 6. E,W
	char    speed[9];      // 7. 0.13 knots (double)
	char    course[9];     // 8. 309.62 degrees (double)
	char    date[7];       // 9. ddmmyy
	char    mag_var[9];    // 10.E,W degrees (double)
	char    mag_var_pos;   // 11.
	char    mode;          // 12.A=autonomous, D,E
	char    cs[4];         // 13.checkum *34
} GNRMC;

/// Data_Parser struct: slaat alleen de benodigde data op in correcte datatypen
typedef struct _Data_Parser
{
	char 	status;
	double  latitude;  		// 3. ddmm.mmmm (double)
	double  longitude; 		// 5. ddmm.mmmm (double)
	double  speed;      	// 7. 0.13 knots (double)
	double  course;   	// 8. 309.62 degrees (double)
} Data_Parser;

// Struct voor het bijelkaar houden van de vector tussen Leaphy en Waypoint
typedef struct Vector
{
	double lengte;
	double course;
} Vector;

// enum voor NMEA protocolstrings (starting 'e' for enum)
enum NMEA
{
	eGNRMC = 1,
	eGPGSA,
	eGNGGA
};

/// struct waar gps string in wordt opgeslagen
extern GNRMC gnrmc;
/// struct om de string om te zetten naar cijfers.
extern Data_Parser GNRMC_data;
/// struct om huidige positie in op te slaan
extern Data_Parser Gem;
/// array van structs om de waypoints van de route in op te slaan
extern Data_Parser waypoints[MAX_WAYPOINTS];
/// array van structs om hiervan een 1 gemiddelde waarde te maken
extern Data_Parser average[3];
