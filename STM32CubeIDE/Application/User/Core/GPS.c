/*
 * GPS.c
 *
 *  Created on: Jan 02, 2025
 *      Author: SDITD - Connor Coultas
 *  @Brief: Driver file for controlling GPS operations.
 */

/******************************************************
 Includes
 Note:
 ******************************************************/
#include "GPS.h"
#include "Functions.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>

GPS_PARAMETER_TYPE privateGpsParameters;
GPS_DATA_TYPE privateGpsData;
TIME_DATE_TYPE checkGps;
TIME_DATE_TYPE nextGps;
TIME_DATE_TYPE checkAcquireGPS;
TIME_DATE_TYPE nextAcquireGPS;
CELL_STATUS_REG CELL;
char gpsLocationBuff[GPS_SIZE] = "";
bool gpsError = true;
bool gpsBoot = true;
/************************ Public Function Definitions ************************/
void gpsParametersInit(void)
{
	privateGpsParameters.alarmSamplePeriod = DEFAULT_GPS_ALARM_SAMPLE_PERIOD;
	privateGpsParameters.alarmTime = DEFAULT_GPS_ALARM_TIME;
	privateGpsParameters.geofenceDistance = DEFAULT_GPS_GEOFENCE;
	privateGpsParameters.gpsInterval = DEFAULT_GPS_INTERVAL;
	privateGpsParameters.mode = DEFAULT_GPS_MODE;
	privateGpsParameters.movementHysteresis = DEFAULT_GPS_HYSTERESIS;
}

void gpsDataInit(void)
{
	privateGpsData.CurrentPosition.latitude = 0.0;
	privateGpsData.CurrentPosition.longitude = 0.0;
	privateGpsData.LastPosition.latitude = 0.0;
	privateGpsData.LastPosition.longitude = 0.0;
	privateGpsData.HDOP = 0.0;
	strncpy(privateGpsData.UTC, "\0", sizeof(privateGpsData.UTC));
	strncpy(privateGpsData.date, "\0", sizeof(privateGpsData.date));
	strncpy(privateGpsData.COG, "\0", sizeof(privateGpsData.COG));
	privateGpsData.altitude = 0.0;
	privateGpsData.fix = 0;
	privateGpsData.nsat = 0;
	privateGpsData.spkm = 0.0;
	privateGpsData.spkn = 0.0;
}

// Function to get GPS parameters
void getGpsParameters(GPS_PARAMETER_TYPE *extGpsParameters)
{
	extGpsParameters->alarmSamplePeriod = privateGpsParameters.alarmSamplePeriod;
	extGpsParameters->alarmTime = privateGpsParameters.alarmTime;
	extGpsParameters->geofenceDistance = privateGpsParameters.geofenceDistance;
	extGpsParameters->gpsInterval = privateGpsParameters.gpsInterval;
	extGpsParameters->mode = privateGpsParameters.mode;
	extGpsParameters->movementHysteresis = privateGpsParameters.movementHysteresis;
}

// Function to set GPS parameters
void setGpsParameters(GPS_PARAMETER_TYPE extGpsParameters)
{
	privateGpsParameters.alarmSamplePeriod = extGpsParameters.alarmSamplePeriod;
	privateGpsParameters.alarmTime = extGpsParameters.alarmTime;
	privateGpsParameters.geofenceDistance = extGpsParameters.geofenceDistance;
	privateGpsParameters.gpsInterval = extGpsParameters.gpsInterval;
	privateGpsParameters.mode = extGpsParameters.mode;
	privateGpsParameters.movementHysteresis = extGpsParameters.movementHysteresis;

	PRINTF("Set GPS mode is %d\r\n", privateGpsParameters.mode);
}

bool isGpsTimeSyncEnabled(void)
{
	bool isEnabled = privateGpsParameters.mode & 0x04;
	return isEnabled;
}

float GPSIntToFloat (int32_t positionInt)
{
	int divider = 100000;

	//test values below
	//Data_Ptr->LastPosition.latitudeInt = 3429648;
	//Data_Ptr->LastPosition.longitudeInt  = 9565477;
	positionInt = 9565477;

	//Data_Ptr->LastPosition.latitude = (float)Data_Ptr->LastPosition.latitudeInt / divider;
	//Data_Ptr->LastPosition.longitude = (float)Data_Ptr->LastPosition.longitudeInt / divider;

	return (float)(positionInt/ divider);
}

//GAV need to handle negative values
int32_t GPSFloatToInt (float position)
{
	int32_t multiplier = 100000;
	//	convert out the decimal

	//test values below
	//Data_Ptr->LastPosition.latitude = 33.29647;
	//Data_Ptr->LastPosition.longitude  = -97.65473;
	position = -97.65473;

	float posInt = position * multiplier;
	//float lonbuf = Data_Ptr->LastPosition.longitude * multiplier;

	//truncate
	//Data_Ptr->LastPosition.latitudeInt = (int)latbuf;
	//Data_Ptr->LastPosition.longitudeInt = (int)lonbuf;
	return (int32_t)posInt;

}

bool decodeGPSConfigs(uint8_t *mqttMsg)
{
	bool isError = false;
	uint8_t version = 255;
	uint16_t mode = 255;
	uint32_t acPeriod = 0;
	uint32_t distance = 0;
	uint32_t alarmWindow = 0;
	uint16_t alarmSample = 0;
	uint32_t hysteresis = 0;
	GPS_PARAMETER_TYPE decodedGpsParams;
	char Buff[MEMORY_MAX] = " ";
	char test [] = "\"gps\":{";
	char verTest[] = "\"version\":";
	char modeTest [] = "\"mode\":";
	char acPeriodTest [] = "\"gps_acquisition_period\":";
	char geoDistTest [] = "\"threshold_distance\":";
	char alarmWindowTest [] = "\"alarm_window\":";
	char alarmSampleTest [] = "\"alarm_sample_period\":";
	char hysteresisTest [] = "\"hysteresis\":";
	char gpsErrStr[CONFIG_ERR_MSG_SIZE] = "";
	int buffSize = 0;

	// Transfer MQTT message to a local buffer
	Word_Transfer(Buff, (char*)mqttMsg);

	char *substr = strstr(Buff, test);
	Refresh_Watchdog;
	if(substr)
	{
		buffSize += snprintf(gpsErrStr, CONFIG_ERR_MSG_SIZE, "\"gps\":[\"config_error\",");
		char *verStr = strstr(substr, verTest);
		if(verStr)
		{
			Refresh_Watchdog;
			verStr += strlen(verTest);
			if(isdigit((unsigned char)verStr[0]))
			{
				version = atoi(verStr);
				if(version == 0)
				{
					char *modeStr = strstr(substr, modeTest);
					if(modeStr)
					{
						modeStr += strlen(modeTest);
						if(isdigit((unsigned char)modeStr[0]))
						{
							mode = atoi(modeStr);
							if(mode >= 0 && mode <= 255)
							{
								decodedGpsParams.mode = mode;
							}
							else
							{
								isError = true;
								PRINTF("GPS mode is out of range [0-255]: %d\r\n", mode);
								buffSize += snprintf((gpsErrStr + buffSize), (CONFIG_ERR_MSG_SIZE - buffSize), "\"mode_out_of_range\",");
							}
						}
						else
						{
							isError = true;
							//"Invalid data type for mode"
							buffSize += snprintf((gpsErrStr + buffSize), (CONFIG_ERR_MSG_SIZE - buffSize), "\"invalid_mode_type\",");
						}
					}
					else
					{
						isError = true;
						buffSize += snprintf((gpsErrStr + buffSize), (CONFIG_ERR_MSG_SIZE - buffSize), "\"missing_mode\",");
						//"Mode not found in GPS message"
					}

					char *acPeriodStr = strstr(substr, acPeriodTest);
					Refresh_Watchdog;
					if(acPeriodStr)
					{
						Refresh_Watchdog;
						acPeriodStr += strlen(acPeriodTest);
						if(isdigit((unsigned char)acPeriodStr[0]))
						{
							acPeriod = atoi(acPeriodStr);
							if(acPeriod >= 0 && acPeriod <= 65535)
							{
								decodedGpsParams.gpsInterval = acPeriod;
							}
							else
							{
								isError = true;
								PRINTF("GPS acquisition period is out of range [0-65535]: %d\r\n", acPeriod);
								buffSize += snprintf((gpsErrStr + buffSize), (CONFIG_ERR_MSG_SIZE - buffSize), "\"acquisition_period_out_of_range\",");
							}
						}
						else
						{
							isError = true;
							//"Invalid data type for acquisition period"
							buffSize += snprintf((gpsErrStr + buffSize), (CONFIG_ERR_MSG_SIZE - buffSize), "\"invalid_acquisition_period_type\",");
						}
					}
					else
					{
						isError = true;
						//"Acquisition period not found in GPS message"
						buffSize += snprintf((gpsErrStr + buffSize), (CONFIG_ERR_MSG_SIZE - buffSize), "\"missing_acquisition_period\",");
					}

					char *geoDistStr = strstr(substr, geoDistTest);
					Refresh_Watchdog;
					if(geoDistStr)
					{
						Refresh_Watchdog;
						geoDistStr += strlen(geoDistTest);
						if(isdigit((unsigned char)geoDistStr[0]))
						{
							distance = atoi(geoDistStr);
							if(distance >= 0 && distance <= 65535)
							{
								decodedGpsParams.geofenceDistance = distance;
							}
							else
							{
								isError = true;
								PRINTF("GPS threshold distance is out of range [0-65535]: %d\r\n", distance);
								buffSize += snprintf((gpsErrStr + buffSize), (CONFIG_ERR_MSG_SIZE - buffSize), "\"threshold_distance_out_of_range\",");
							}
						}
						else
						{
							isError = true;
							//"Invalid data type for threshold distance"
							buffSize += snprintf((gpsErrStr + buffSize), (CONFIG_ERR_MSG_SIZE - buffSize), "\"invalid_threshold_distance_type\",");
						}
					}
					else
					{
						isError = true;
						buffSize += snprintf((gpsErrStr + buffSize), (CONFIG_ERR_MSG_SIZE - buffSize), "\"missing_threshold_distance\",");
					}

					char *alarmWindowStr = strstr(substr, alarmWindowTest);
					Refresh_Watchdog;
					if(geoDistStr)
					{
						alarmWindowStr += strlen(alarmWindowTest);
						if(isdigit((unsigned char)alarmWindowStr[0]))
						{
							alarmWindow = atoi(alarmWindowStr);
							if(alarmWindow >= 0 && alarmWindow <= 65535)
							{
								Refresh_Watchdog;
								decodedGpsParams.alarmTime = alarmWindow;
							}
							else
							{
								isError = true;
								PRINTF("GPS alarm window is out of range [0-65535]: %d\r\n", distance);
								buffSize += snprintf((gpsErrStr + buffSize), (CONFIG_ERR_MSG_SIZE - buffSize), "\"alarm_window_out_of_range\",");
							}
						}
						else
						{
							isError = true;
							//"Invalid data type for alarm window"
							buffSize += snprintf((gpsErrStr + buffSize), (CONFIG_ERR_MSG_SIZE - buffSize), "\"invalid_alarm_window_type\",");
						}
					}
					else
					{
						isError = true;
						buffSize += snprintf((gpsErrStr + buffSize), (CONFIG_ERR_MSG_SIZE - buffSize), "\"missing_alarm_window\",");
					}

					char *alarmSampleStr = strstr(substr, alarmSampleTest);
					Refresh_Watchdog;
					if(alarmSampleStr)
					{
						alarmSampleStr += strlen(alarmSampleTest);
						if(isdigit((unsigned char)alarmSampleStr[0]))
						{
							Refresh_Watchdog;
							alarmSample = atoi(alarmSampleStr);
							if(alarmSample >= 0 && alarmSample <= 255)
							{
								decodedGpsParams.alarmSamplePeriod = alarmSample;
							}
							else
							{
								isError = true;
								PRINTF("GPS alarm sample period is out of range [0-255]: %d\r\n", distance);
								buffSize += snprintf((gpsErrStr + buffSize), (CONFIG_ERR_MSG_SIZE - buffSize), "\"alarm_sample_period_out_of_range\",");
							}
						}
						else
						{
							isError = true;
							buffSize += snprintf((gpsErrStr + buffSize), (CONFIG_ERR_MSG_SIZE - buffSize), "\"invalid_alarm_sample_period_type\",");
						}
					}
					else
					{
						isError = true;
						buffSize += snprintf((gpsErrStr + buffSize), (CONFIG_ERR_MSG_SIZE - buffSize), "\"missing_alarm_sample_period\",");
					}

					char *hysteresisStr = strstr(substr, hysteresisTest);
					Refresh_Watchdog;
					if(hysteresisStr)
					{
						Refresh_Watchdog;
						hysteresisStr += strlen(hysteresisTest);
						if(isdigit((unsigned char)hysteresisStr[0]))
						{
							hysteresis = atoi(hysteresisStr);
							if(hysteresis >= 0 && hysteresis <= 65535)
							{
								decodedGpsParams.movementHysteresis = hysteresis;
							}
							else
							{
								isError = true;
								PRINTF("GPS hysteresis is out of range [0-65535]: %d\r\n", distance);
								buffSize += snprintf((gpsErrStr + buffSize), (CONFIG_ERR_MSG_SIZE - buffSize), "\"hysteresis_out_of_range\",");
							}
						}
						else
						{
							isError = true;
							buffSize += snprintf((gpsErrStr + buffSize), (CONFIG_ERR_MSG_SIZE - buffSize), "\"invalid_hysteresis_type\",");
						}
					}
					else
					{
						isError = true;
						buffSize += snprintf((gpsErrStr + buffSize), (CONFIG_ERR_MSG_SIZE - buffSize), "\"missing_hysteresis\",");
					}
				}
				else
				{
					isError = true;
					PRINTF("Invalid version number decoded: %d\r\n", version);
					buffSize += snprintf((gpsErrStr + buffSize), (CONFIG_ERR_MSG_SIZE - buffSize), "\"version_mismatch\",");
				}
			}
			else
			{
				isError = true;
				//"Invalid data type for version"
				buffSize += snprintf((gpsErrStr + buffSize), (CONFIG_ERR_MSG_SIZE - buffSize), "\"invalid_version_type\",");
			}
		}
		else
		{
			isError = true;
			buffSize += snprintf((gpsErrStr + buffSize), (CONFIG_ERR_MSG_SIZE - buffSize), "\"missing_version\",");
		}
	}
	else
	{
		isError = true;
		//"GPS data not found in configuration"
	}

	if(!isError)
	{
		Refresh_Watchdog;
		PRINTF("Setting GPS Values; mode %d, alarmSamplePeriod %d, alarmTime %d, gpsInterval %d, geofenceDistance %d, movementHysteresis %d\r\n",
				decodedGpsParams.mode, decodedGpsParams.alarmSamplePeriod, decodedGpsParams.alarmTime, decodedGpsParams.gpsInterval, decodedGpsParams.geofenceDistance,decodedGpsParams.movementHysteresis);

		setGpsParameters(decodedGpsParams);
	}
	else
	{
		if(buffSize > 0 && buffSize < CONFIG_ERR_MSG_SIZE - 2 && gpsErrStr[0] != '\0')
		{
			if(gpsErrStr[buffSize - 1] == ',')
			{
				gpsErrStr[buffSize - 1] = ']';
				addErrorString(gpsErrStr);
			}
		}
	}

	PRINTF("GPS decode error is %d\r\n", isError);
	return isError;
}

bool gpsGetData(char* gpsMsg)
{
	//Test string
	//	char gpstest[GPS_SIZE] = {"AT+QGPSLOC=2   +QGPSLOC: 225102.000,33.21912,-97.14844,2.0,173.6,3,0.00,0.0,0.0,030724,03"};

	bool isError = false;
	uint8_t nsat = 0;
	int fix = 0.0;
	float latitude = 0.0;
	float longitude = 0.0;
	float hdop = 0.0;
	float altitude = 0.0;
	float spkm = 0.0;
	float spkn = 0.0;
	char test[] = "+QGPSLOC: ";
	char dateStr[GPS_DATE_LENGTH] = "";
	char timeStr[GPS_UTC_LENGTH] = "";
	char cogStr[GPS_DATE_LENGTH] = "";
	char monthStr[GPS_DATE_SIZE] = "";
	char dayStr[GPS_DATE_SIZE] = "";
	char yearStr[GPS_DATE_SIZE] = "";
	char hourStr[GPS_DATE_SIZE] = "";
	char minStr[GPS_DATE_SIZE] = "";
	char secStr[GPS_DATE_SIZE] = "";
	char fracStr[GPS_FRACTION_SIZE] = "";

	Word_Transfer(gpsLocationBuff, (char*)gpsMsg);

	char *dataStr = strstr(gpsLocationBuff, test);
	if(dataStr)
	{
		//UTC
		dataStr += strlen(test);

		//Lattitude
		int lenToLat = strcspn(dataStr, ",");
		strncpy(timeStr, dataStr, lenToLat);
		dataStr += lenToLat + 1;
		latitude = strtof(dataStr, NULL);

		//Longitude
		int lenToLon = strcspn(dataStr, ",");
		dataStr += lenToLon + 1;
		longitude = strtof(dataStr, NULL);

		//HDOP
		int lenToHDOP = strcspn(dataStr, ",");
		dataStr += lenToHDOP + 1;
		hdop = strtof(dataStr, NULL);

		//Altitude
		int lenToAlt = strcspn(dataStr, ",");
		dataStr += lenToAlt + 1;
		altitude = strtof(dataStr, NULL);

		//Fix
		int lenToFix = strcspn(dataStr, ",");
		dataStr += lenToFix + 1;
		fix = strtof(dataStr, NULL);
		if(isdigit((unsigned char)dataStr[0]))
		{
			fix = atoi(dataStr);
		}

		//COG
		int lenToCOG = strcspn(dataStr, ",");
		dataStr += lenToCOG + 1;

		//spkm
		int lenToSPKM = strcspn(dataStr, ",");
		strncpy(cogStr, dataStr, lenToSPKM);

		dataStr += lenToSPKM + 1;
		spkm = strtof(dataStr, NULL);

		//spkn
		int lenToSPKN = strcspn(dataStr, ",");
		dataStr += lenToSPKN + 1;
		spkn = strtof(dataStr, NULL);

		//date
		int lenToDate = strcspn(dataStr, ",");
		dataStr += lenToDate + 1;

		//nsat
		int lenToNSAT = strcspn(dataStr, ",");
		strncpy(dateStr, dataStr, lenToNSAT);

		dataStr += lenToNSAT + 1;
		if(isdigit((unsigned char)dataStr[0]))
		{
			nsat = atoi(dataStr);
		}
	}
	else
	{
		isError = true;
		PRINTF("GPS Data not found in get location function.\r\n");
	}

	if(!isError && latitude != 0.0 && longitude != 0.0)
	{
		privateGpsData.LastPosition.latitude = privateGpsData.CurrentPosition.latitude;
		privateGpsData.LastPosition.longitude = privateGpsData.CurrentPosition.longitude;
		privateGpsData.CurrentPosition.latitude = latitude;
		privateGpsData.CurrentPosition.longitude = longitude;
		privateGpsData.HDOP = hdop;
		privateGpsData.altitude = altitude;
		privateGpsData.fix = fix;
		privateGpsData.spkm = spkm;
		privateGpsData.spkn = spkn;
		privateGpsData.nsat = nsat;
		if(cogStr[0] != '\0' && dateStr[0] != '\0' && timeStr[0] != '\0')
		{
			strncpy(privateGpsData.COG, cogStr, GPS_DATE_LENGTH);
			strncpy(privateGpsData.date, dateStr, GPS_DATE_LENGTH);

			//Seperate day/month/year
			strncpy(dayStr, dateStr, GPS_DATE_WIDTH);
			strncpy(monthStr, dateStr + GPS_DATE_WIDTH, GPS_DATE_WIDTH);
			strncpy(yearStr, dateStr + (GPS_DATE_WIDTH + GPS_DATE_WIDTH), GPS_DATE_WIDTH);

			//Seperate hour/min/sec/frac
			strncpy(hourStr, timeStr, GPS_DATE_WIDTH);
			strncpy(minStr, timeStr + GPS_DATE_WIDTH, GPS_DATE_WIDTH);
			strncpy(secStr, timeStr + (GPS_DATE_WIDTH + GPS_DATE_WIDTH), GPS_DATE_WIDTH);
			strncpy(fracStr, timeStr + (GPS_DATE_WIDTH + GPS_DATE_WIDTH + GPS_DATE_WIDTH + 1), GPS_DATE_WIDTH + 1);

			//Set day/month/year
			if(dayStr[0] != '\0' && monthStr[0] != '\0' && yearStr[0] != '\0' && hourStr[0] != '\0' && minStr[0] != '\0' && secStr[0] != '\0' && fracStr[0] != '\0')
			{
				if(isdigit((unsigned char) dayStr[0]) && isdigit((unsigned char) monthStr[0]) && isdigit((unsigned char) yearStr[0]) && isdigit((unsigned char)hourStr[0]) && isdigit((unsigned char)minStr[0]) && isdigit((unsigned char)secStr[0]) && isdigit((unsigned char)fracStr[0]))
				{
					RTC_TimeTypeDef GPS_Time = {0};
					RTC_DateTypeDef GPS_Date = {0};
					HAL_RTC_GetTime(&hrtc, &GPS_Time, FORMAT_BIN);
					HAL_RTC_GetDate(&hrtc, &GPS_Date, FORMAT_BIN);

					GPS_Date.Date = atoi(dayStr);
					GPS_Date.Month = atoi(monthStr);
					GPS_Date.Year = atoi(yearStr);
					GPS_Time.Hours = atoi(hourStr);
					GPS_Time.Minutes = atoi(minStr);
					GPS_Time.Seconds = atoi(secStr);
					GPS_Time.SecondFraction = atoi(fracStr);
					if (HAL_RTC_SetDate(&hrtc, &GPS_Date, FORMAT_BIN) != HAL_OK)
					{
						Error_Handler();
						isError = true;
					}

					if (HAL_RTC_SetTime(&hrtc, &GPS_Time, FORMAT_BIN) != HAL_OK)
					{
						Error_Handler();
						isError = true;
					}

					HAL_RTC_GetTime(&hrtc, &GPS_Time, FORMAT_BIN);
					HAL_RTC_GetDate(&hrtc, &GPS_Date, FORMAT_BIN);
				}
			}
			else
			{
				isError = true;
			}
		}
		else
		{
			isError = true;
		}
	}

	if(isError)
	{
		PRINTF("GPS Data Failed\r\n");
	}
	else
	{
		PRINTF("GPS Data Validated\r\n");
	}

	gpsError = isError;
	return isError;
}


char* getGpsConfigStr(void)
{
	static char buffer[GPS_MSG_SIZE] = {0};
	bool valid = false;

	int buffSize = snprintf(buffer, GPS_MSG_SIZE, "\"gps\":{\"version\":%u,\"mode\":%u,\"gps_acquisition_period\":%u,\"threshold_distance\":%u,\"alarm_window\":%u,\"alarm_sample_period\":%u,\"hysteresis\":%u}",
			GPS_CONFIG_VERSION, privateGpsParameters.mode, privateGpsParameters.gpsInterval, privateGpsParameters.geofenceDistance, privateGpsParameters.alarmTime, privateGpsParameters.alarmSamplePeriod, privateGpsParameters.movementHysteresis);

	if(buffSize > 0 && buffSize < GPS_MSG_SIZE)
	{
		valid = true;
	}

	if(valid)
	{
		return buffer;
	}
	else
	{
		return "Failed to build GPS message\r\n";
	}
}

char* gpsGetLocationBuff()
{
	return gpsLocationBuff;
}

void gpsClearLocation()
{
	for(int locIndex = 0; locIndex < GPS_SIZE; locIndex++)
	{
		gpsLocationBuff[locIndex] = '\0';
	}
}

char* gpsLocationMessage()
{
	static char gpsBuff[GPS_MSG_SIZE] = {0};
	char timebuff[ISO_TIMESTAMP_LENGTH] = {""};
	int buffSize = 0;

	Time_StampISO(timebuff);

	buffSize = snprintf(gpsBuff, GPS_MSG_SIZE, "\1{\"dev_id\":\"%lu\",\"type\":\"location\",\"timestamp\":\"%s\",\"version\":%d,\"latitude\":%f,\"longitude\":%f,\"altitude\":%f,\"speed\":%f,\"course\":%s,\"nsat\":%d}",
			UNIQUE_Device_ID, timebuff, GPS_LOCATION_VERSION, privateGpsData.CurrentPosition.latitude, privateGpsData.CurrentPosition.longitude, privateGpsData.altitude, privateGpsData.spkm, privateGpsData.COG, privateGpsData.nsat);

	if(buffSize > 0 && buffSize < GPS_MSG_SIZE)
	{
		return gpsBuff;
	}
	else
	{
		return "Failed to build location message\r\n";
	}
}


void GPSActiveAntennaON (MEM_PTR *Data_Ptr)
{
	Clear_Memory(Data_Ptr);
	gpsError = false;

	PRINTF("Turning on active GPS antenna\r\n\r\n");

	Clear_Memory(Data_Ptr);

	CELL.COMMAND = GPS_ACTIVE_ANTENNA_ENABLE;
	CELL_COMMAND(Data_Ptr);
	cellResponseCheck(Data_Ptr);
	Clear_Memory(Data_Ptr);

	CELL.COMMAND = GPS_ACTIVE_ANTENNA_SET;
	CELL_COMMAND(Data_Ptr);
	cellResponseCheck(Data_Ptr);
	Clear_Memory(Data_Ptr);

}

void EnableGPSXtraFunctionality (MEM_PTR *Data_Ptr)
{
	Clear_Memory(Data_Ptr);
	gpsError = false;

	PRINTF("Beginning XTRA GPS file check\r\n\r\n");
	Clear_Memory(Data_Ptr);

	CELL.COMMAND = XTRA_ENABLE;
	CELL_COMMAND(Data_Ptr);
	cellResponseCheck(Data_Ptr);
	Clear_Memory(Data_Ptr);

	CELL.COMMAND = XTRA_AUTODL_ENABLE;
	CELL_COMMAND(Data_Ptr);
	cellResponseCheck(Data_Ptr);
	Clear_Memory(Data_Ptr);

	PRINTF("Restarting modem\r\n\r\n");
	restartModem();

	CELL.COMMAND = CELLID;
	CELL_COMMAND(Data_Ptr);
	cellResponseCheck(Data_Ptr);
	Clear_Memory(Data_Ptr);

	CELL.COMMAND = XTRA_TIME_QUERY;
	CELL_COMMAND(Data_Ptr);
	cellResponseCheck(Data_Ptr);
	Clear_Memory(Data_Ptr);

	CELL.COMMAND = GPS_ENABLE;
	CELL_COMMAND(Data_Ptr);
	cellResponseCheck(Data_Ptr);
	Clear_Memory(Data_Ptr);

	CELL.STATE = CELL_OK;

}




void getGPS (MEM_PTR *Data_Ptr)
{

	Clear_Memory(Data_Ptr);
	gpsError = false;

#ifdef Log_Level_2
	Log_Single( LOG_MSG_CELL_GPSUPDT);
#endif // Log_Level_2

//	CELL.COMMAND = CHECK;
//	CELL_COMMAND(Data_Ptr);
//	if (CELL.STATE != CELL_OK)
//		return;

	CELL.COMMAND = CHECK;
	CELL_COMMAND(Data_Ptr);
	CELL.COMMAND = GPS_ENABLE;
	CELL_COMMAND(Data_Ptr);
	CELL.COMMAND = CHECK;
	CELL_COMMAND(Data_Ptr);

	//GAV GPS timeout
	//	TIME_DATE_TYPE stopTime;
	//	uint16_t timeDelay = GPS_Timeout;
	//
	//	calculateStopTime ( &stopTime, timeDelay);
	Clear_Memory(Data_Ptr);   //Added by Kevin
	CELL.COMMAND = CHECK;
	CELL_COMMAND(Data_Ptr);
	CELL.COMMAND = LOCATION;
	CELL_COMMAND(Data_Ptr);

	calculateNextTime(&nextGps, GPS_TIMEOUT_59S);
	while (IsError2(Data_Ptr) && (!checkNextTime(checkGps, nextGps)))
	{
		Clear_Memory(Data_Ptr);
		CELL.COMMAND = CHECK;
		CELL_COMMAND(Data_Ptr);
		CELL.COMMAND = LOCATION;
		CELL_COMMAND(Data_Ptr);
	}

	if (!gpsGetData((char*)Data_Ptr->Memory))
	{
#ifdef Log_Level_2
		if (Data_Ptr->Setting & LOG_LEVEL_1) // && Setting & LOG_LEVEL_2)
		{
			Write_Log( LOG_DEVICE_ID);
			Write_Log(gpsGetLocationBuff());
			Write_Log( LOG_TAIL);
#ifdef UART_USB
			Read_Last_UART_Log(Data_Ptr);
#endif
		}
#endif
		CELL.COMMAND = CHECK;
		CELL_COMMAND(Data_Ptr);

	}
	else
	{
		Log_Single ("\1 MODEM : GPS Update Failed Timeout\r\n\0" );
		gpsError = true;
	}
}

void sendGPS (MEM_PTR *Data_Ptr)
{
	if (!gpsError)
	{
		Clear_Memory(Data_Ptr);
		Data_Ptr->Buffer = 0;
		Build_MSG(Data_Ptr, gpsLocationMessage());
		Data_Ptr->Size = Data_Ptr->Buffer;
		CELL.COMMAND = PUBLISH;
		CELL_COMMAND(Data_Ptr);
		CELL.COMMAND = PUBLISH_MEM;
		CELL_COMMAND(Data_Ptr);

		uint32_t seconds = (privateGpsParameters.gpsInterval * 60);
		calculateNextTime( &nextAcquireGPS, seconds);
	}
}


uint16_t getMutePeriodGPS(void)
{
	return privateGpsParameters.alarmSamplePeriod;
}

