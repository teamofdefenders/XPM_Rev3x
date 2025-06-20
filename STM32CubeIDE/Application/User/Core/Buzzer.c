/*
 ******************************************************************************
 * @file    Buzzer.c
 * @author  Team of Defenders
 * @brief   Source file for buzzer functions
 ******************************************************************************
 *
 * Copyright 2025 Team of Defenders
 *
 ******************************************************************************
 */

#ifndef BUZZER_C_
#define BUZZER_C_

#include <stdio.h>
#include <string.h>
#include "Buzzer.h"
#include "Functions.h"

/* Private variables ---------------------------------------------------------*/

//  ********************  Public Functions  ****************************************************

BUZZER_PARAMETER_TYPE privateBuzzerParams;

/**
 * @brief  Initializes the buzzer mode parameters
 * @note   Default is 0
 * @retval no return value
 */
void buzzerParametersInit()
{
	privateBuzzerParams.mode = DEF_BUZZ_MODE;
	setBuzzerDefault(false);
}

/**
 * @brief  Gets the local buzzer parameters
 * @param  BUZZER_PARAMETER_TYPE extParams
 *         Pointer to the buzzer parameter structure
 * @retval none
 */
void getBuzzerParameters(BUZZER_PARAMETER_TYPE *extParams)
{
	extParams->mode = privateBuzzerParams.mode;
	extParams->valueDefault = privateBuzzerParams.valueDefault;
}

/**
 * @brief  Sets the local buzzer parameters
 * @param  BUZZER_PARAMETER_TYPE extParams
 *         Pointer to the buzzer parameter structure
 * @retval none
 */
void setBuzzerParameters(BUZZER_PARAMETER_TYPE extParams)
{
	privateBuzzerParams.mode = extParams.mode;
	privateBuzzerParams.valueDefault = extParams.valueDefault;
}

void setBuzzerDefault(bool isTrue)
{
	privateBuzzerParams.valueDefault = 200;
	if (isTrue)
		privateBuzzerParams.valueDefault = 0;
}

/**
 * @brief  decodes the mqtt downlink message for the buzzer parameters
 * @param  mqttMsg - pointer to basically a character string
 * @retval return value isError - true is error occurred, false is everything decoded fine
 */
bool decodeBuzzerConfigs(uint8_t* mqttMsg)
{
	bool isError = false;
	uint8_t version = 255;
	uint16_t mode = 256;
	char tempBuff[MEMORY_MAX] = " ";
	char test [] = "\"buzzer\":{";
	char verTest[] = "\"version\":";
	char modeTest [] = "\"mode\":";
	char buzzerErrStr[CONFIG_ERR_MSG_SIZE] = "";

	int buffSize = 0;
	BUZZER_PARAMETER_TYPE decodedBuzzerParams;

	//Need to get first to prevent overwriting non passed data
	getBuzzerParameters(&decodedBuzzerParams);
	// Transfer MQTT message to a local buffer
	Word_Transfer(tempBuff, (char*)mqttMsg);

	char* substr = strstr(tempBuff, test);
	if(substr)
	{
		buffSize += snprintf(buzzerErrStr, CONFIG_ERR_MSG_SIZE, "\"buzzer\":[\"config_error\",");
		Refresh_Watchdog;
		char *verStr = strstr(substr, verTest);
		if(verStr)
		{
			Refresh_Watchdog;
			verStr += strlen(verTest);
			if(isdigit((unsigned char)verStr[0]))
			{
				version = atoi(verStr);
				if(version == 1)
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
								decodedBuzzerParams.mode = mode;
							}
							else
							{
								isError = true;
								PRINTF("Buzzer mode is out of range [0-255]: %d\r\n", mode);
								buffSize += snprintf((buzzerErrStr + buffSize), (CONFIG_ERR_MSG_SIZE - buffSize), "\"mode_out_of_range_[0-255]\",");
							}
						}
						else
						{
							isError = true;
							//"Invalid data type for mode"
							buffSize += snprintf((buzzerErrStr + buffSize), (CONFIG_ERR_MSG_SIZE - buffSize), "\"invalid_mode_type_NAN\",");
						}
					}
					else
					{
						isError = true;
						buffSize += snprintf((buzzerErrStr + buffSize), (CONFIG_ERR_MSG_SIZE - buffSize), "\"missing_mode_string\",");
						//"Mode not found in buzzer message"
					}
				}
				else
				{
					isError = true;
					PRINTF("Invalid version number decoded: %d\r\n", version);
					buffSize += snprintf((buzzerErrStr + buffSize), (CONFIG_ERR_MSG_SIZE - buffSize), "\"version_mismatch\",");
				}
			}
			else
			{
				isError = true;
				//"Invalid data type for version"
				buffSize += snprintf((buzzerErrStr + buffSize), (CONFIG_ERR_MSG_SIZE - buffSize), "\"invalid_version_type_NAN\",");
			}
		}
		else
		{
			isError = true;
			buffSize += snprintf((buzzerErrStr + buffSize), (CONFIG_ERR_MSG_SIZE - buffSize), "\"missing_version_string\",");
		}
	}
	else
	{
		isError = true;
		//"buzzer data not found in configuration"
	}

	if(!isError)
	{
		decodedBuzzerParams.mode = mode;
		setBuzzerParameters(decodedBuzzerParams);
		setBuzzerDefault(false);
	}
	else
	{
		if(buffSize > 0 && buffSize < CONFIG_ERR_MSG_SIZE - 2 && buzzerErrStr[0] != '\0')
		{
			if(buzzerErrStr[buffSize - 1] == ',')
			{
				buzzerErrStr[buffSize - 1] = ']';
				addErrorString(buzzerErrStr);
			}
		}
	}
	return isError;
}

/**
 * @brief  Gets the buzzer configuration in a string format
 * @retval return Buzzer configuration string
 */
char* getBuzzerConfigStr(void)
{
	static char buffer[BUZ_MSG_SIZE] = {0};
	bool valid = false;

	int buffSize = snprintf(buffer, BUZ_MSG_SIZE, "\"buzzer\":{\"version\":%u,\"mode\":%u}",
			BUZZ_CONFIG_VERSION, privateBuzzerParams.mode);

	if(buffSize > 0 && buffSize < BUZ_MSG_SIZE)
	{
		valid = true;
	}

	if(valid)
	{
		return buffer;
	}
	else
	{
		return "Failed to build Buzzer message\r\n";
	}
}


/* Private functions ---------------------------------------------------------*/

#endif  // BUZZER_C_
