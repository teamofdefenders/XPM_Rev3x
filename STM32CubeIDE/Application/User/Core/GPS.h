/*
 * GPS.h
 *
 *  Created on: Nov 20, 2024
 *      Author: SDITD - Connor Coultas
 *  @Brief: This header file is used for managing day/night functionality.
 */

#ifndef GPS_H_
#define GPS_H_

//******************************************************
// Includes
// Note:
//******************************************************

#include "Functions.h"
#include "Skywire.h"
#include "main.h"


//************************ Private Defines ************************
#define DEFAULT_GPS_MODE 7
#define DEFAULT_GPS_ALARM_SAMPLE_PERIOD 180
#define DEFAULT_GPS_ALARM_TIME 14400
#define DEFAULT_GPS_GEOFENCE 500
#define DEFAULT_GPS_INTERVAL 1440
#define DEFAULT_GPS_HYSTERESIS 150
#define GPS_UTC_LENGTH 11
#define GPS_FRACTION_SIZE 4
#define GPS_SIZE 256
#define GPS_DATE_LENGTH 7
#define GPS_DATE_WIDTH 2
#define GPS_DATE_SIZE 3
#define GPS_MSG_SIZE 500
#define GPS_TIMEOUT_59S 59
#define GPS_CONFIG_VERSION 0
#define GPS_LOCATION_VERSION 1
#define UNIQUE_Device_ID (*(volatile uint32_t*)0x0BFA0700)


//************************ Public Function Prototypes ************************
int32_t GPSFloatToInt(float position);
float GPSIntToFloat (int32_t positionInt);
char* gpsLocationMessage(void);

#endif /* APPLICATION_USER_CORE_GPS_H_ */
