/**
* @file my_gps.h
* @brief Bevat basic (provisorische) defines & externals voor de gps routines
* @attention
* <h3>&copy; Copyright (c) 2023 (HU) Michiel Scager.</h3>
* @author MSC
*
* @date 5/9/2023
*/
int hex2int(char *c);
int hexchar2int(char c);
int checksum_valid(char *string);

//extern void GNRMC_Parser(void);



/// GNRMC struct: all with char-members - should/could be improved with proper data-elements
typedef struct _GNRMC
{
	char    head[7];       // 0. header
	char    time[10];      // 1. hhmmss.sss
	char    status;        // 2. A=valid, V=not valid
	char    latitude[10];  // 3. ddmm.mmmm (double)
	char    NS_ind;        // 4. N,S
	char    longitude[10]; // 5. ddmm.mmmm (double)
	char    EW_ind;        // 6. E,W
	char    speed[6];      // 7. 0.13 knots (double)
	char    course[6];     // 8. 309.62 degrees (double)
	char    date[7];       // 9. ddmmyy
	char    mag_var[6];    // 10.E,W degrees (double)
	char    mag_var_pos;   // 11.
	char    mode;          // 12.A=autonomous, D,E
	char    cs[4];         // 13.checkum *34
} GNRMC;

/// Data_Parser struct: slaat alleen de benodigde data op in correcte datatypen
typedef struct _Data_Parser
{
	double  latitude;  		// 3. ddmm.mmmm (double)
	double  longitude; 		// 5. ddmm.mmmm (double)
	double  speed;      	// 7. 0.13 knots (double)
	double  course;   	// 8. 309.62 degrees (double)
} Data_Parser;


// enum voor NMEA protocolstrings (starting 'e' for enum)
enum NMEA
{
	eGNRMC = 1,
	eGPGSA,
	eGNGGA
};
