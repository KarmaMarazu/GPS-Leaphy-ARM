/**
* @file gps.c
* @brief Behandelt de gps input-strings (NMEA-protocol) van UART1.<br>
* <b>Demonstreert: xMessageBufferRead() </b><br>
* Aan UART1 is een interrupt gekoppeld (zie main.c: HAL_UART_RxCpltCallback(),
* die de inkomende string op een messagebuffer zet, die we hier uitlezen en verwerken.<br>
* @author MSC
*
* @date 5/5/2023
*/
#include <admin.h>
#include "main.h"
#include "cmsis_os.h"
#include "gps.h"

GNRMC gnrmc; // global struct for GNRMC-messages


/**
* @brief De chars van de binnengekomen GNRMC-string worden in data omgezet, dwz in een
* GNRMC-struct, mbv strtok(); De struct bevat nu alleen chars - je kunt er ook voor kiezen
* om gelijk met doubles te werken, die je dan met atof(); omzet.
* @return void
*/

void split_nmea_fields(char *message, char *fields[], int max_fields)
{
    int i = 0;
    char *start = message;
    while (i < max_fields)
    {
        fields[i++] = start;
        char *comma = strchr(start, ',');
        if (!comma)
			break;
        *comma = '\0';
        start = comma + 1;
    }
}

void fill_GNRMC(char *message)
{
	// example: $GNRMC,164435.000,A,5205.9505,N,00507.0873,E,0.49,21.70,140423,,,A
	//          id    , time     ,s,

	char *fields[20];
	split_nmea_fields(message, fields, 20);

	strcpy(gnrmc.head, fields[0]);
	gnrmc.status = fields[2][0];
	strcpy(gnrmc.latitude, fields[3]);
	strcpy(gnrmc.longitude, fields[5]);
	strcpy(gnrmc.speed, fields[7]);
	strcpy(gnrmc.course, fields[8]);
/*
	char *tok = ",";

    char *s;
    UART_puts("\r\nmessage = ");
    UART_puts(message);
    memset(&gnrmc, 0, sizeof(GNRMC)); // clear the struct

    s = strsep(&message, tok); // 1. header;

    strcpy(gnrmc.head, s);

    s = strsep(&message, tok); // 1. header;
    strcpy(gnrmc.time, s);

    s = strsep(&message, tok);    // 3. valid;
    gnrmc.status = s[0];

    s = strsep(&message, tok);    // 4. latitude;
    strcpy(gnrmc.latitude, s);

    s = strsep(&message, tok);    // 5. N/S; not used
    gnrmc.NS_ind =s[0];

    s = strsep(&message, tok);    // 6. longitude;
    strcpy(gnrmc.longitude, s);

    s = strsep(&message, tok);    // 7. E/W; not used
    gnrmc.EW_ind =s[0];

    s = strsep(&message, tok);    // 8. speed;
    strcpy(gnrmc.speed, s);

    s = strsep(&message, tok);    // 9. course;
    strcpy(gnrmc.course, s);

    s = strsep(&message, tok);    // 10. date;
    strcpy(gnrmc.date, s);

    s = strsep(&message, tok);    // 11. mag_var
    strcpy(gnrmc.mag_var, s);

    s = strsep(&message, tok);    // 12. mag_var_pos
    gnrmc.mag_var_pos = s[0];

    s = strsep(&message, tok);    // 13. mode
    gnrmc.mode = s[0];

	if (Uart_debug_out & GPS_DEBUG_OUT)
	{
		UART_puts("\r\n\t GPS type: \t");  UART_puts(gnrmc.head);
		UART_puts("\r\n\t status: \t\t");  UART_putchar(gnrmc.status);
		UART_puts("\r\n\t latitude:\t\t"); UART_puts(gnrmc.latitude);
		UART_puts("\r\n\t longitude:\t");  UART_puts(gnrmc.longitude);
		UART_puts("\r\n\t speed:    \t");  UART_puts(gnrmc.speed);
		UART_puts("\r\n\t course:   \t");  UART_puts(gnrmc.course);
	}*/
	xTaskNotify(GetTaskhandle("GNRMC_Parser"), 0, eNoAction);
}

/**
* @brief Leest de GPS-NMEA-strings die via de UART via interrupt-handler (HAL_UART_RxCpltCallback)
* binnenkomen. * De handler zet elk inkomende character gelijk op een queue, die hier uitgelezen wordt.
* Vervolgens wordt hiervan een GPS-message opgebouwd en verwerkt.
* @return void
*/
void GPS_getNMEA (void *argument)
{
    char  Q_char;   			// char to receive from queue
	char  MSG_buff[GPS_MAXLEN]; // buffer for GPS-string
	int   pos = 0;
	int   cs;                   // checksum-flag
	int   new_msg = FALSE;      // do we encounter a '$'-char?
	int   msg_type = 0;         // do we want this message to be interpreted?

	UART_puts((char *)__func__); UART_puts("started\n\r");

	while (TRUE)
	{
		xQueueReceive(hGPS_Queue, &Q_char, portMAX_DELAY); // get one char from the q

		//UART_putchar(Q_buff);  // echo, for testing

		if (Q_char == '$') // gotcha, new datastring started
		{
			memset(MSG_buff, 0, sizeof(MSG_buff)); // clear buff
			pos = 0;
			new_msg = TRUE; // from now on, chars are valid to receive
		}

		if (new_msg == FALSE) // char only valid if started by $
			continue;

		MSG_buff[pos] = Q_char; // copy char read from Q into the msg-buf

		// if pos==5, the message type (f.i. "$GPGSA) is complete, so we now we can determine
		// if we want the rest of the message... else we skip the rest characters
		if (pos == 5)
		{
			msg_type = 0; // reset

			// next, we decide which message types we want to interpret
			// and we set the message-type for later use...
			if      (!strncmp(&MSG_buff[1], "GNRMC", 5)) msg_type = eGNRMC;
			else if (!strncmp(&MSG_buff[1], "GPGSA", 5)) msg_type = eGPGSA;
			else if (!strncmp(&MSG_buff[1], "GNGGA", 5)) msg_type = eGNGGA;

			if (!msg_type) // not an interesting message type
			{
				new_msg = FALSE;
				continue;
			}
		}

		// if we are here, we are reading the rest of the message into the msg_buff
		////////////////////////////////////////////////////////////////////////////
		if (pos >= GPS_MAXLEN - 1) // avoid overflow (should not happen, but still...)
		{
			new_msg = FALSE; // ignore it
			continue;
		}

		if (MSG_buff[pos] == '\r') // end of message encountered - all messages end with <CR-13><LF-10>
		{
			MSG_buff[pos] = '\0';          // close string
			cs = checksum_valid(MSG_buff); // note, checksumchars (eg "*43") are removed from string

			if (Uart_debug_out & GPS_DEBUG_OUT) // output to uart if wanted
			{
				UART_puts("\r\nGPS (UART4): "); UART_puts(MSG_buff);
				UART_puts( cs ? " [cs:OK]\r\n" : " [cs:ERR]\r\n");
			}

			if (cs) // checksum okay, so interpret the message
			{
				switch(msg_type) // extract data from msg into right struct
				{
				case eGNRMC: fill_GNRMC(MSG_buff);
						     // use the data...
						     break;
				case eGPGSA:
				case eGNGGA: break;
				default:     break;
				}
				/*
				if (msg_type == eGNRMC)
				{
					UART_puts("\rHallo");
				}*/
			}

			new_msg = FALSE; // new message possible
			continue;
		}
		pos++; // proceed reading next char from the queue
	}
}


// source: file:///C:/craigpeacock/NMEA-GPS
int hex2int(char *c)
{
	int value;

	value = hexchar2int(c[0]);
	value = value << 4;
	value += hexchar2int(c[1]);

	return value;
}


int hexchar2int(char c)
{
    if (c >= '0' && c <= '9')
        return (c - '0');
    if (c >= 'A' && c <= 'F')
        return (c - 'A' + 10);
    if (c >= 'a' && c <= 'f')
        return (c - 'a' + 10);
    return (-1);
}


// source: file:///C:/craigpeacock/NMEA-GPS
int checksum_valid(char *string)
{
	char *checksum_str;
	int checksum, i;
	unsigned char calculated_checksum = 0;

	// Checksum is postcede by *
	if ((checksum_str = strchr(string, '*')))
	{
		*checksum_str = '\0'; // Remove checksum from string
		// Calculate checksum, starting after $ (i = 1)
		for (i = 1; i < strlen(string); i++)
			calculated_checksum = calculated_checksum ^ string[i];

		checksum = hex2int((char *)checksum_str+1);
		//printf("Checksum Str [%s], Checksum %02X, Calculated Checksum %02X\r\n",(char *)checksum_str+1, checksum, calculated_checksum);
		if (checksum == calculated_checksum)
			return (1);
	}

	return (0);
}
