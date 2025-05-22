/*
 * DayNight.c
 *
 *  Created on: Nov 20, 2024
 *      Author: SDITD - Connor Coultas
 *  @Brief: Driver file for controlling day and night operations.
 */

/******************************************************
 Includes
 Note:
 ******************************************************/
#include "DayNight.h"
#include <string.h>

BUSINESS_DATA_TYPE privateBusiness;

/************************ Public Function Definitions ************************/
void dayNightInit(void)
{
	privateBusiness.DayNightSwitchEnabled = false;
	privateBusiness.dayTime.hours = DEFAULT_DAY_HOURS;
	privateBusiness.dayTime.minutes = DEFAULT_DAY_MINUTES;
	privateBusiness.nightTime.hours = DEFAULT_NIGHT_HOURS;
	privateBusiness.nightTime.minutes = DEFAULT_NIGHT_MINUTES;
	privateBusiness.lastEventTime.hours = DEFAULT_LAST_COOLDOWN;
	privateBusiness.lastEventTime.minutes = DEFAULT_LAST_COOLDOWN;
}

void getBusinessHours(BUSINESS_DATA_TYPE *extBusinessHours)
{
	extBusinessHours->DayNightSwitchEnabled = privateBusiness.DayNightSwitchEnabled;
	extBusinessHours->dayTime.hours = privateBusiness.dayTime.hours;
	extBusinessHours->dayTime.minutes = privateBusiness.dayTime.minutes;
	extBusinessHours->nightTime.hours = privateBusiness.nightTime.hours;
	extBusinessHours->nightTime.minutes = privateBusiness.nightTime.minutes;
	extBusinessHours->lastEventTime.hours = privateBusiness.lastEventTime.hours;
	extBusinessHours->lastEventTime.minutes = privateBusiness.lastEventTime.minutes;
}

// Function to set from flash memory
void setBusinessHours(BUSINESS_DATA_TYPE extBusinessHours)
{
	privateBusiness.DayNightSwitchEnabled =	extBusinessHours.DayNightSwitchEnabled;
	privateBusiness.dayTime.hours = extBusinessHours.dayTime.hours;
	privateBusiness.dayTime.minutes = extBusinessHours.dayTime.minutes;
	privateBusiness.nightTime.hours = extBusinessHours.nightTime.hours;
	privateBusiness.nightTime.minutes = extBusinessHours.nightTime.minutes;
	privateBusiness.lastEventTime.hours = extBusinessHours.lastEventTime.hours;
	privateBusiness.lastEventTime.minutes = extBusinessHours.lastEventTime.minutes;

}

void setConfigurationTime(uint8_t dhours, uint8_t dmin, uint8_t nhours, uint8_t nmin, bool dnsEnable)
{
	privateBusiness.DayNightSwitchEnabled = dnsEnable;
	privateBusiness.dayTime.hours = dhours;
	privateBusiness.dayTime.minutes = dmin;
	privateBusiness.nightTime.hours = nhours;
	privateBusiness.nightTime.minutes = nmin;
	privateBusiness.lastEventTime.hours = DEFAULT_LAST_COOLDOWN;
	privateBusiness.lastEventTime.minutes = DEFAULT_LAST_COOLDOWN;

}

/**
  * @brief  Checks current time against business hours to determine if it is night
  * @note Also some logic for day/night enabled and checks for default time
  * @retval return value isNight - true if night, false if day
  */
bool isNight(void)
{
	bool isNightTime = false;

	if( isTimeDefault() )
	{
		PRINTF("Time not set by GPS\r\n");
		isNightTime = true; //Switching disabled - "always night"
	}
	else if(privateBusiness.DayNightSwitchEnabled == false)
	{
		isNightTime = true; //Switching disabled - "always night"
	}
	else
	{
		isNightTime = isNight2();
	}

	return isNightTime;
}


/**
  * @brief  Checks current time against business hours to determine if it is night
  * @note
  * @retval return value isNight - true if night, false if day
  */
bool isNight2(void)
{
	bool isNightTime = false;

	TIME_DATE_TYPE currentTime;

	// Get the current time from the RTC
	HAL_RTC_GetTime(&hrtc, &currentTime.rtcTime, RTC_FORMAT_BIN);
	HAL_RTC_GetDate ( &hrtc , &currentTime.rtcDate , RTC_FORMAT_BIN );

	//This converts the time to minutes to handle business hours spanning midnight
	uint16_t currentMinutes = currentTime.rtcTime.Hours * 60 + currentTime.rtcTime.Minutes;
	uint16_t nightStartMinutes = privateBusiness.nightTime.hours * 60 + privateBusiness.nightTime.minutes;
	uint16_t dayStartMinutes = privateBusiness.dayTime.hours * 60 + privateBusiness.dayTime.minutes;

	PRINTF("Night Minutes %d, Current Minutes %d \r\n", nightStartMinutes, currentMinutes);
	if (nightStartMinutes > dayStartMinutes) // Night time spans across midnight
	{
		if (currentMinutes >= nightStartMinutes || currentMinutes < dayStartMinutes)
		{
			isNightTime = true;
		}
	}
	else // Night time does not span across midnight
	{
		if (currentMinutes >= nightStartMinutes && currentMinutes < dayStartMinutes)
		{
			isNightTime = true;
		}
	}
	if (isNightTime)
	{
		PRINTF("isNight2 returning Night\r\n");
	}
	else
	{
		PRINTF("isNight2 returning Day\r\n");
	}

	return isNightTime;
}


bool isTimeDefault(void)
{
	bool isDefaultTime = false;
	TIME_DATE_TYPE currentTime;

	// Get the current time from the RTC
	HAL_RTC_GetTime(&hrtc, &currentTime.rtcTime, RTC_FORMAT_BIN);
	HAL_RTC_GetDate ( &hrtc , &currentTime.rtcDate , RTC_FORMAT_BIN );

	uint32_t sysTime = currentTime.rtcDate.Year;
	if (sysTime <= 23) // check that RTC has set system clock (our date defaults to 08/07/23)
	{
		isDefaultTime = true;
	}
	return isDefaultTime;
}


//*****TODO******

//void storeToXPSArray()
//{
//
//}


// kcs not tested
void setLastEvent(BUSINESS_DATA_TYPE *extBusinessHours)
{
	TIME_DATE_TYPE currentTime;

	// Get the current time from the RTC
	HAL_RTC_GetTime(&hrtc, &currentTime.rtcTime, RTC_FORMAT_BIN);
	HAL_RTC_GetDate ( &hrtc , &currentTime.rtcDate , RTC_FORMAT_BIN );
	extBusinessHours->lastEventTime.hours = currentTime.rtcTime.Hours;
	extBusinessHours->lastEventTime.minutes = currentTime.rtcTime.Minutes;
}

// pass in MQTTMsg
bool decodeDayNightConfigs(uint8_t *mqttMsg)
{
	bool isError = false;
	bool validDay = false;
	bool validNight = false;
	uint8_t version = 255;
	uint8_t dayH = 0;
	uint8_t dayM = 0;
	uint8_t nightH = 0;
	uint8_t nightM = 0;
	BUSINESS_DATA_TYPE decodedBusHours;
	char Buff[MEMORY_MAX] = " ";
	char test[] = "\"day_night\":{";
	char verTest [] = "\"version\":";
	char dayTest [] = "\"day_start_time\":";
	char nightTest [] = "\"night_start_time\":";
	char dayNightErrStr[CONFIG_ERR_MSG_SIZE] = "";
	int buffSize = 0;

	// Transfer MQTT message to a local buffer
	Word_Transfer(Buff, (char *)mqttMsg);

	// Check for day_night configuration
	char *substr = strstr(Buff, test);
	if (substr)
	{
		buffSize += snprintf(dayNightErrStr, CONFIG_ERR_MSG_SIZE, "\"day_night\":[\"config_error\",");
		// Find version
		char *verStr = strstr(substr, verTest);
		if (verStr)
		{
			verStr += strlen(verTest);
			if(isdigit((unsigned char)verStr[0]))
			{
				version = atoi(verStr);
			}
			else
			{
				isError = true;
				buffSize += snprintf((dayNightErrStr + buffSize), (CONFIG_ERR_MSG_SIZE - buffSize), "\"invalid_version_type_NAN\",");
			}

			if(version == 1)
			{
				// Find day start time (HH:MM)
				char *dayStr = strstr(substr, dayTest);
				if (dayStr)
				{
					dayStr += strlen(dayTest) + 1;
					char *dayMin = dayStr + 3;
					// Convert hours and minutes if they are integers. Set validDay to true
					if(isdigit((unsigned char)dayStr[0]))
					{
						dayH = atoi(dayStr);
						if(isdigit((unsigned char)dayMin[0]))
						{
							dayM = atoi(dayMin);
							validDay = true;
						}
						else
						{
							isError = true;
							buffSize += snprintf((dayNightErrStr + buffSize), (CONFIG_ERR_MSG_SIZE - buffSize), "\"invalid_day_minute_type_NAN\",");
						}
					}
					else
					{
						isError = true;
						buffSize += snprintf((dayNightErrStr + buffSize), (CONFIG_ERR_MSG_SIZE - buffSize), "\"invalid_day_hour_type_NAN\",");
					}
				}

				// Find night start time (HH:MM)
				char *nightStr = strstr(substr, nightTest);
				if (nightStr && !isError)
				{
					nightStr += strlen(nightTest) + 1;
					char *nightMin = nightStr + 3;
					// Convert hours and minutes if they are integers. Set validDay to true
					if(isdigit((unsigned char)nightStr[0]))
					{
						nightH = atoi(nightStr);
						if(isdigit((unsigned char)nightMin[0]))
						{
							nightM = atoi(nightMin);
							validNight = true;
						}
						else
						{
							isError = true;
							buffSize += snprintf((dayNightErrStr + buffSize), (CONFIG_ERR_MSG_SIZE - buffSize), "\"invalid_night_minute_type_NAN\",");
						}
					}
					else
					{
						isError = true;
						buffSize += snprintf((dayNightErrStr + buffSize), (CONFIG_ERR_MSG_SIZE - buffSize), "\"invalid_night_hour_type_NAN\",");
					}
				}
			}
			else
			{
				buffSize += snprintf((dayNightErrStr + buffSize), (CONFIG_ERR_MSG_SIZE - buffSize), "\"version_mismatch\",");
				isError = true;
			}
		}
//		Refresh_Watchdog; // For debugging

		if(validDay && validNight && !isError)
		{
			if((dayH >= 0 && dayH <=  23) && (dayM >= 0 && dayM <= 59) && (nightH >= 0 && nightH <= 23) && (nightM >= 0 && nightM <= 59))
			{
				// KCS fix this
				decodedBusHours.DayNightSwitchEnabled = true;
				decodedBusHours.dayTime.hours = dayH;
				decodedBusHours.dayTime.minutes = dayM;
				decodedBusHours.nightTime.hours = nightH;
				decodedBusHours.nightTime.minutes = nightM;
				setBusinessHours(decodedBusHours);

				// KCS configuration should no longer contain dayTime/Nighttime switching enable; should be individual modes
				PRINTF("Values for hours and minutes; Day Hours %02d:%02d, Night Hours %02d:%02d, Version is %d\r\n", decodedBusHours.dayTime.hours, decodedBusHours.dayTime.minutes, decodedBusHours.nightTime.hours, decodedBusHours.nightTime.minutes, version);

			}
			else
			{
				PRINTF("Values for hours and minutes in day night configs are out of range\r\n");
				buffSize += snprintf((dayNightErrStr + buffSize), (CONFIG_ERR_MSG_SIZE - buffSize), "\"day_night_hours_out_of_range\",");
				isError = true;
			}
		}
		else
		{
			PRINTF("Invalid parsing of day night configs, valid day is %d and valid night is %d\r\n", validDay, validNight);
			buffSize += snprintf((dayNightErrStr + buffSize), (CONFIG_ERR_MSG_SIZE - buffSize), "\"invalid_parsing\",");
		}

		if(isError)
		{
			if(buffSize > 0 && buffSize < CONFIG_ERR_MSG_SIZE - 2 && dayNightErrStr[0] != '\0')
			{
				if(dayNightErrStr[buffSize - 1] == ',')
				{
					dayNightErrStr[buffSize - 1] = ']';
					addErrorString(dayNightErrStr);
				}
			}
		}
	} //End of big if subStr
	return isError;
}

char* dayNightConfigStr()
{
	static char buffer[500] = " ";
	bool valid = false;

	int buffSize = snprintf(buffer, 500, "\"day_night\":{\"version\":%u,\"day_start_time\":\"%02u:%02u\",\"night_start_time\":\"%02u:%02u\"}",
			DAY_NIGHT_CONFIG_VERSION ,privateBusiness.dayTime.hours, privateBusiness.dayTime.minutes, privateBusiness.nightTime.hours, privateBusiness.nightTime.minutes);

	if(buffSize > 0 && buffSize < 500)
	{
		valid = true;
	}

	if(valid)
	{
		return buffer;
	}
	else
	{
		return "Failed to build Day/Night test Configure String";
	}
}
