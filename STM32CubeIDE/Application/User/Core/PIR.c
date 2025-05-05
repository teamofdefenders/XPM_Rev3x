/**
 ******************************************************************************
 * @file    PIR.c
 * @author  Team of Defenders
 * @brief   Passive IR detector driver
 ******************************************************************************
 *
 * Copyright 2023 Team of Defenders
 *
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include <string.h>
#include <stdlib.h>
#include "main.h"
#include "PIR.h"
#include "Functions.h"

/* External variables --------------------------------------------------------*/
extern LPTIM_HandleTypeDef hlptim2;
extern RTC_HandleTypeDef hrtc;


/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
//#define PIR_Control _PIR_Control

/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

PIR_PARAMETER_TYPE pirData2;
PIR_FILTER_TYPE dayTimeParam;
PIR_FILTER_TYPE nightTimeParam;

uint8_t percent; 	// Percentage counter for windows
uint8_t state; 	    // state machine PIR state
uint8_t counter; 	// window counter of PIR motions
uint8_t eventCounter;
/* Private function prototypes -----------------------------------------------*/

void pirDataInit ( void )
{
	state = PIR_IDLE;
	counter = 0;
	eventCounter = 0;
	percent = 0;
}

void pirParametersInit(void)
{
	pirData2.mode = PIR_MODE_DEF;
	PRINTF("Setting PIR mode to default: %d\r\n",pirData2.mode);
	pirData2.coolDown = PIR_MUTE_DEFAULT;

	dayTimeParam.Motion_Blackout            = DAY_BLACKOUT_DEF;
	dayTimeParam.Motion_Confirm_Window      = DAY_CONF_MOTION_WINDOW_DEF;
	dayTimeParam.Motion_Threshhold          = DAY_THRESH_DEF;
	dayTimeParam.Near_Motion_Threshhold     = DAY_NEAR_MOTION_DEF;
	dayTimeParam.No_Motion_Detection_Window = DAY_NO_MOTION_DEF;

	nightTimeParam.Motion_Blackout            = NIGHT_BLACKOUT_DEF;
	nightTimeParam.Motion_Confirm_Window      = NIGHT_CONF_MOTION_WINDOW_DEF;
	nightTimeParam.Motion_Threshhold          = NIGHT_THRESH_DEF;
	nightTimeParam.Near_Motion_Threshhold     = NIGHT_NEAR_MOTION_DEF;
	nightTimeParam.No_Motion_Detection_Window = NIGHT_NO_MOTION_DEF;
}

void setPirParameters(PIR_PARAMETER_TYPE extPirParameters)
{
	pirData2.mode = extPirParameters.mode;
	pirData2.coolDown = extPirParameters.coolDown;
}

void getPirParameters(PIR_PARAMETER_TYPE *extPirParameters)
{
	extPirParameters->mode = pirData2.mode;
	extPirParameters->coolDown = pirData2.coolDown;
}

void setFilterParameters(PIR_FILTER_TYPE extFilterParameters, FILTER_TYPE time)
{
	if (time == PIR_DAY)
	{
		dayTimeParam.Motion_Blackout            = extFilterParameters.Motion_Blackout;
		dayTimeParam.Motion_Confirm_Window      = extFilterParameters.Motion_Confirm_Window;
		dayTimeParam.Motion_Threshhold          = extFilterParameters.Motion_Threshhold;
		dayTimeParam.Near_Motion_Threshhold     = extFilterParameters.Near_Motion_Threshhold;
		dayTimeParam.No_Motion_Detection_Window = extFilterParameters.No_Motion_Detection_Window;
	}
	else
	{
		nightTimeParam.Motion_Blackout            = extFilterParameters.Motion_Blackout;
		nightTimeParam.Motion_Confirm_Window      = extFilterParameters.Motion_Confirm_Window;
		nightTimeParam.Motion_Threshhold          = extFilterParameters.Motion_Threshhold;
		nightTimeParam.Near_Motion_Threshhold     = extFilterParameters.Near_Motion_Threshhold;
		nightTimeParam.No_Motion_Detection_Window = extFilterParameters.No_Motion_Detection_Window;
	}
}

void getFilterParameters(PIR_FILTER_TYPE *extFilterParameters, FILTER_TYPE time )
{
	if (time == PIR_DAY)
	{
		extFilterParameters->Motion_Blackout            = dayTimeParam.Motion_Blackout;
		extFilterParameters->Motion_Confirm_Window      = dayTimeParam.Motion_Confirm_Window;
		extFilterParameters->Motion_Threshhold          = dayTimeParam.Motion_Threshhold;
		extFilterParameters->Near_Motion_Threshhold     = dayTimeParam.Near_Motion_Threshhold;
		extFilterParameters->No_Motion_Detection_Window = dayTimeParam.No_Motion_Detection_Window;
	}
	else
	{
		extFilterParameters->Motion_Blackout            = nightTimeParam.Motion_Blackout;
		extFilterParameters->Motion_Confirm_Window      = nightTimeParam.Motion_Confirm_Window;
		extFilterParameters->Motion_Threshhold          = nightTimeParam.Motion_Threshhold;
		extFilterParameters->Near_Motion_Threshhold     = nightTimeParam.Near_Motion_Threshhold;
		extFilterParameters->No_Motion_Detection_Window = nightTimeParam.No_Motion_Detection_Window;
	}
}


// Call in main loop after everything is setup
//void setPIRTrigger ( void )
//{
////	HAL_RTC_DeactivateAlarm ( &hrtc , RTC_ALARM_B );
//	HAL_NVIC_EnableIRQ ( PIR_EXTI_IRQn );
//}

// Utilizing Timer B for PIR black out
void PIR_ReCall ( MEM_PTR *Data_Ptr )
{
//	RTC_AlarmTypeDef sAlarm;
//	RTC_TimeTypeDef cTime;
//	RTC_DateTypeDef unlockRtc = {0};
//
//	HAL_RTC_GetAlarm (&hrtc , &sAlarm , RTC_ALARM_B , FORMAT_BIN );
//	HAL_RTC_GetTime (&hrtc , &cTime , FORMAT_BIN );
//	HAL_RTC_GetDate (&hrtc, &unlockRtc, FORMAT_BIN);
//
//	sAlarm.AlarmTime.Minutes = cTime.Minutes;
//	sAlarm.AlarmTime.Seconds = cTime.Seconds;
//	//GAV set them to one second if latencyMin is true time TBD (around a second or so, experiment with rolling door)
//
//	uint16_t time;
//	// KCS need to handle day and night time switching of parameters
//
//	if (control & CONFIRM_MOTION) time = dayTimeParam.Motion_Confirm_Window;
//	else if (control & MOTION_BLACKOUT) time = dayTimeParam.Motion_Blackout;
//	else if (control & CONFIRM_NO_MOTION) time = dayTimeParam.No_Motion_Detection_Window;
//	else return;
//
//	//time -= (time % 1000); kcs what was this doing? Reverse Truncation of the milliseconds?
//
//	sAlarm.AlarmTime.Minutes += ((time) / (60));
//	sAlarm.AlarmTime.Seconds += (((time) / 1) % 60);
//
//	if (sAlarm.AlarmTime.Seconds > 59)
//	{
//		sAlarm.AlarmTime.Seconds -= 59;
//		sAlarm.AlarmTime.Minutes++;
//	}
//	if (sAlarm.AlarmTime.Minutes > 59) sAlarm.AlarmTime.Minutes -= 59;
//
//	while (HAL_RTC_SetAlarm_IT ( &hrtc , &sAlarm , FORMAT_BIN ) != HAL_OK);
}

bool decodePIRConfigs(uint8_t *mqttMsg)
{
	bool isError = false;
	uint8_t version = 255;
	uint16_t mode = 255;
	uint32_t mutePeriod = 0;
	PIR_PARAMETER_TYPE decodedPirParams;
	char Buff[MEMORY_MAX] = " ";
	char test [] = "\"pir\":{";
	char verTest[] = "\"version\":";
	char modeTest [] = "\"mode\":";
	char muteTest [] = "\"muting_period\":";
	char pirErrStr[CONFIG_ERR_MSG_SIZE] = "";
	int buffSize = 0;

	//Need to get first to prevent overwriting non passed data such as even counter
	getPirParameters(&decodedPirParams);
	// Transfer MQTT message to a local buffer
	Word_Transfer(Buff, (char*)mqttMsg);

	char* substr = strstr(Buff, test);
	if(substr)
	{
		buffSize += snprintf(pirErrStr, CONFIG_ERR_MSG_SIZE, "\"pir\":[\"config_error\",");
		Refresh_Watchdog;
		char *verStr = strstr(substr, verTest);
		if(verStr)
		{
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
								Refresh_Watchdog;
								decodedPirParams.mode = mode;
							}
							else
							{
								PRINTF("PIR mode is out of range [0-255]: %d\r\n", mode);
								buffSize += snprintf((pirErrStr + buffSize), (CONFIG_ERR_MSG_SIZE - buffSize), "\"mode_out_of_range\",");
								isError = true;
							}
						}
						else
						{
							isError = true;
							//"Invalid data type for mode"
							buffSize += snprintf((pirErrStr + buffSize), (CONFIG_ERR_MSG_SIZE - buffSize), "\"invalid_mode_type\",");
						}
					}
					else
					{
						isError = true;
						buffSize += snprintf((pirErrStr + buffSize), (CONFIG_ERR_MSG_SIZE - buffSize), "\"missing_mode\",");
					}

					char* muteStr = strstr(substr, muteTest);
					if(muteStr)
					{
						muteStr += strlen(muteTest);
						if(isdigit((unsigned char)muteStr[0]))
						{
							mutePeriod = atoi(muteStr);
							if(mutePeriod >= 0 && mutePeriod <= 65535)
							{
								Refresh_Watchdog;
								decodedPirParams.coolDown = mutePeriod;
							}
							else
							{
								PRINTF("PIR muting period is out of range [0-255]: %d\r\n", mutePeriod);
								buffSize += snprintf((pirErrStr + buffSize), (CONFIG_ERR_MSG_SIZE - buffSize), "\"muting_period_out_of_range\",");
								isError = true;
							}
						}
						else
						{
							isError = true;
							buffSize += snprintf((pirErrStr + buffSize), (CONFIG_ERR_MSG_SIZE - buffSize), "\"invalid_muting_period_type\",");
						}
					}
					else
					{
						isError = true;
						buffSize += snprintf((pirErrStr + buffSize), (CONFIG_ERR_MSG_SIZE - buffSize), "\"missing_muting_period\",");
					}
				}
				else
				{
					isError = true;
					buffSize += snprintf((pirErrStr + buffSize), (CONFIG_ERR_MSG_SIZE - buffSize), "\"version_mismatch\",");
				}
			}
			else
			{
				isError = true;
				buffSize += snprintf((pirErrStr + buffSize), (CONFIG_ERR_MSG_SIZE - buffSize), "\"invalid_version_type\",");
			}
		}
		else
		{
			isError = true;
			buffSize += snprintf((pirErrStr + buffSize), (CONFIG_ERR_MSG_SIZE - buffSize), "\"missing_version\",");
		}
	}
	else
	{
		//no pir decode found
		isError = true;
	}

	if(!isError)
	{
		Refresh_Watchdog;
		PRINTF("Setting PIR Params:mode %d, cool down %d\r\n", decodedPirParams.mode, decodedPirParams.coolDown);
		setPirParameters(decodedPirParams);
	}
	else
	{
		if(buffSize > 0 && buffSize < CONFIG_ERR_MSG_SIZE - 2 && pirErrStr[0] != '\0')
		{
			if(pirErrStr[buffSize - 1] == ',')
			{
				pirErrStr[buffSize - 1] = ']';
				addErrorString(pirErrStr);
			}
		}
	}

	return isError;
}

/**
 * @brief  Decodes the motion filter downlink and stores the parameters accordingly
 * @note
 * @param  uint8_t *mqttMsg
 *         Downlink data buffer from Cell modemime
 * @retval bool is error - true = error, false = passed
 */
bool decodeMotionFilter(uint8_t *mqttMsg, FILTER_TYPE sunPeriod)
{
	bool isError = false;
	// these have to bigger to prevent overflow and correct checking of parameters
	uint32_t version = 255;
	uint32_t motionWindow = 0;
	uint32_t motionThreshold = 0;
	uint32_t blackout = 0;
	uint32_t noMotion = 0;
	uint32_t nearMotion = 0;

	PIR_FILTER_TYPE decodedFilterParams;
	char Buff[MEMORY_MAX] = " ";
	char dayFilterTest [] = "\"motion_filter_day\":{";
	char nightFilterTest [] = "\"motion_filter_night\":{";
	char motionWindowTest [] = "\"motion_confirm_window\":";
	char motionThreshTest [] = "\"motion_threshold\":";
	char blackoutTest [] = "\"motion_blackout\":";
	char noMotionTest [] = "\"no_motion_window\":";
	char nearMotionTest [] = "\"near_motion\":";
	char verTest[] = "\"version\":";
	char filterErrStr[CONFIG_ERR_MSG_SIZE] = "";
	int buffSize = 0;


	//Need to get first for completeness
	getFilterParameters(&decodedFilterParams, sunPeriod );

	// Transfer MQTT message to a local buffer
	Word_Transfer(Buff, (char*)mqttMsg);

	char* filterStr = '\0';

	if (sunPeriod == PIR_NIGHT)
	{
		filterStr = strstr(Buff, nightFilterTest);
		if(filterStr)
		{
			PRINTF("Decoding Night Filter Parameters\r\n");
			buffSize += snprintf(filterErrStr, CONFIG_ERR_MSG_SIZE, "\"motion_filter_night\":[\"config_error\",");
		}
		else
		{
			isError = true;
		}
	}
	else if(sunPeriod == PIR_DAY)
	{
		filterStr = strstr(Buff, dayFilterTest);
		if(filterStr)
		{
			PRINTF("Decoding Day Filter Parameters\r\n");
			buffSize += snprintf(filterErrStr, CONFIG_ERR_MSG_SIZE, "\"motion_filter_day\":[\"config_error\",");
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
	Refresh_Watchdog;

	if(!isError)
	{
		char* verStr = strstr(filterStr, verTest);
		if(verStr)
		{
			verStr += strlen(verTest);
			if(isdigit((unsigned char)verStr[0]))
			{
				version = atoi(verStr);
			}
			else
			{
				isError = true;
				buffSize += snprintf((filterErrStr + buffSize), (CONFIG_ERR_MSG_SIZE - buffSize), "\"invalid_version_type\",");
			}
		}
		else
		{
			isError = true;
		}

		char* motionWindowStr = strstr(filterStr, motionWindowTest);
		if(motionWindowStr)
		{
			motionWindowStr += strlen(motionWindowTest);
			if(isdigit((unsigned char)motionWindowStr[0]))
			{
				Refresh_Watchdog;
				motionWindow = atoi(motionWindowStr);
				if(motionWindow >= 0 && motionWindow <= 65535)
				{
					decodedFilterParams.Motion_Confirm_Window = (uint16_t)motionWindow;
				}
				else
				{
					PRINTF("Motion window is out of range [0-65535]: %d\r\n", motionWindow);
					buffSize += snprintf((filterErrStr + buffSize), (CONFIG_ERR_MSG_SIZE - buffSize), "\"motion_confirm_window_out_of_range\",");
					isError = true;
				}
			}
			else
			{
				isError = true;
				buffSize += snprintf((filterErrStr + buffSize), (CONFIG_ERR_MSG_SIZE - buffSize), "\"invalid_motion_confirm_window_type\",");
			}
		}
		else
		{
			isError = true;
			buffSize += snprintf((filterErrStr + buffSize), (CONFIG_ERR_MSG_SIZE - buffSize), "\"missing_motion_confirm_window\",");
		}

		char* motionThreshStr = strstr(filterStr, motionThreshTest);
		if(motionThreshStr)
		{
			Refresh_Watchdog;
			motionThreshStr += strlen(motionThreshTest);
			if(isdigit((unsigned char)motionThreshStr[0]))
			{
				motionThreshold = atoi(motionThreshStr);
				if(motionThreshold >= 0 && motionThreshold <= 100)
				{
					decodedFilterParams.Motion_Threshhold = (uint8_t)motionThreshold;
				}
				else
				{
					PRINTF("Motion threshold is out of range [0-100]: %d\r\n", motionThreshold);
					isError = true;
					buffSize += snprintf((filterErrStr + buffSize), (CONFIG_ERR_MSG_SIZE - buffSize), "\"motion_threshold_out_of_range\",");
				}
			}
			else
			{
				isError = true;
				buffSize += snprintf((filterErrStr + buffSize), (CONFIG_ERR_MSG_SIZE - buffSize), "\"invalid_motion_threshold_type\",");
			}
		}
		else
		{
			isError = true;
			buffSize += snprintf((filterErrStr + buffSize), (CONFIG_ERR_MSG_SIZE - buffSize), "\"missing_motion_threshold\",");
		}

		char* balckoutStr = strstr(filterStr, blackoutTest);
		if(balckoutStr)
		{
			Refresh_Watchdog;
			balckoutStr += strlen(blackoutTest);
			if(isdigit((unsigned char)balckoutStr[0]))
			{
				blackout = atoi(balckoutStr);
				// Change Range to uint16_t max
				if(blackout >= 0 && blackout <= 65535)
				{
					decodedFilterParams.Motion_Blackout = (uint16_t)blackout;
				}
				else
				{
					PRINTF("Blackout is out of range [0-65535]: %d\r\n", blackout);
					isError = true;
					buffSize += snprintf((filterErrStr + buffSize), (CONFIG_ERR_MSG_SIZE - buffSize), "\"motion_blackout_out_of_range\",");
				}
			}
			else
			{
				isError = true;
				buffSize += snprintf((filterErrStr + buffSize), (CONFIG_ERR_MSG_SIZE - buffSize), "\"invald_motion_blackout_type\",");
			}
		}
		else
		{
			isError = true;
			buffSize += snprintf((filterErrStr + buffSize), (CONFIG_ERR_MSG_SIZE - buffSize), "\"missing_motion_blackout\",");
		}

		char* noMotionStr = strstr(filterStr, noMotionTest);
		if(noMotionStr)
		{
			Refresh_Watchdog;
			noMotionStr += strlen(noMotionTest);
			if(isdigit((unsigned char)noMotionStr[0]))
			{
				noMotion = atoi(noMotionStr);
				if(noMotion >= 0 && noMotion <= 255)
				{
					decodedFilterParams.No_Motion_Detection_Window = (uint8_t)noMotion;
				}
				else
				{
					PRINTF("No motion is out of range [0-255]: %d\r\n", noMotion);
					isError = true;
					buffSize += snprintf((filterErrStr + buffSize), (CONFIG_ERR_MSG_SIZE - buffSize), "\"no_motion_window_out_of_range\",");
				}
			}
			else
			{
				isError = true;
				buffSize += snprintf((filterErrStr + buffSize), (CONFIG_ERR_MSG_SIZE - buffSize), "\"invalid_no_motion_window_type\",");
			}
		}
		else
		{
			isError = true;
			buffSize += snprintf((filterErrStr + buffSize), (CONFIG_ERR_MSG_SIZE - buffSize), "\"missing_no_motion_window\",");
		}

		char* nearMotionStr = strstr(filterStr, nearMotionTest);
		if(nearMotionStr)
		{
			Refresh_Watchdog;
			nearMotionStr += strlen(nearMotionTest);
			if(isdigit((unsigned char)nearMotionStr[0]))
			{
				nearMotion = atoi(nearMotionStr);
				if(nearMotion >= 0 && nearMotion <= 100)
				{
					decodedFilterParams.Near_Motion_Threshhold = (uint8_t)nearMotion;
				}
				else
				{
					PRINTF("Near motion is out of range [0-100]: %d\r\n", nearMotion);
					isError = true;
					buffSize += snprintf((filterErrStr + buffSize), (CONFIG_ERR_MSG_SIZE - buffSize), "\"near_motion_out_of_range\",");
				}
			}
			else
			{
				isError = true;
				buffSize += snprintf((filterErrStr + buffSize), (CONFIG_ERR_MSG_SIZE - buffSize), "\"invalid_near_motion_type\",");
			}
		}
		else
		{
			isError = true;
			buffSize += snprintf((filterErrStr + buffSize), (CONFIG_ERR_MSG_SIZE - buffSize), "\"missing_near_motion\",");
		}
	}
	else
	{
		isError = true;
	}

	if(!isError)
	{
		Refresh_Watchdog;

		PRINTF("Setting Filter Values; Motion_Confirm_Window %d, Motion_Threshhold %d, Motion_Blackout %d, No_Motion_Detection_Window %d, Near_Motion_Threshhold %d\r\n",
				decodedFilterParams.Motion_Confirm_Window,
				decodedFilterParams.Motion_Threshhold,
				decodedFilterParams.Motion_Blackout,
				decodedFilterParams.Near_Motion_Threshhold,
				decodedFilterParams.No_Motion_Detection_Window );

		setFilterParameters(decodedFilterParams, sunPeriod);
	}
	else
	{
		if(buffSize > 0 && buffSize < CONFIG_ERR_MSG_SIZE - 2 && filterErrStr[0] != '\0')
		{
			if(filterErrStr[buffSize - 1] == ',')
			{
				filterErrStr[buffSize - 1] = ']';
				addErrorString(filterErrStr);
			}
		}
	}

	return isError;
}

char* getPirConfigStr(void)
{
	static char pirBuff[PIR_DOWNLINK_TEST_MSG_SIZE] = {0};
	char timebuff[ISO_TIMESTAMP_LENGTH] = {""};
	int buffSize = 0;

	Time_StampISO(timebuff);

	buffSize = snprintf(pirBuff, PIR_DOWNLINK_TEST_MSG_SIZE, "\"pir\":{\"version\":%u,\"mode\":%u,\"muting_period\":%u},\r\n\"motion_filter_day\":{\"version\":%u,\"motion_confirm_window\":%u,\"motion_threshold\":%u,\"motion_blackout\":%u,\"no_motion_window\":%u,\"near_motion\":%u},\r\n\"motion_filter_night\":{\"version\":%u,\"motion_confirm_window\":%u,\"motion_threshold\":%u,\"motion_blackout\":%u,\"no_motion_window\":%u,\"near_motion\":%u}",
			PIR_CONFIG_VERSION, pirData2.mode, pirData2.coolDown, PIR_DAY_VERSION, dayTimeParam.Motion_Confirm_Window, dayTimeParam.Motion_Threshhold, dayTimeParam.Motion_Blackout, dayTimeParam.No_Motion_Detection_Window, dayTimeParam.Near_Motion_Threshhold,
			PIR_NIGHT_VERSION, nightTimeParam.Motion_Confirm_Window, nightTimeParam.Motion_Threshhold, nightTimeParam.Motion_Blackout, nightTimeParam.No_Motion_Detection_Window, nightTimeParam.Near_Motion_Threshhold);

	if(buffSize > 0 && buffSize < PIR_DOWNLINK_TEST_MSG_SIZE)
	{
		return pirBuff;
	}
	else
	{
		return "Failed to build PIR test Configure String";
	}
}


/**
 * @brief  Accessor function for PIR cooldown (mute) period
 * @note
 * @retval uint16_t mutePeriod
 */
uint16_t getPirMutePeriod(void)
{
	return pirData2.coolDown;
}

/*********************   END OF FILE     ***************/
