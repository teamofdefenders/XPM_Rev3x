/**
 ******************************************************************************
 * @file    Environment.c
 * @author  Team of Defenders
 * @brief   Source file for environmental monitoring
 ******************************************************************************
 *
 * Copyright 2025 Team of Defenders
 *
 ******************************************************************************
 */

#include "Environment.h"

ENVIRONMENT_PARAM_TYPE privateEnvironmentParams;

void environmentParametersInit()
{
	privateEnvironmentParams.alarmSamplePeriod = DEF_ENV_ALARM_PER;
	privateEnvironmentParams.alarmTime = DEF_ENV_ALARM_TIME;
	privateEnvironmentParams.samplePeriod = DEF_ENV_SAMPLE;
	privateEnvironmentParams.coolDownPeriod = DEF_ENV_COOL_DOWN;
	privateEnvironmentParams.sensorWarmUp = DEF_WARM_UP;
	privateEnvironmentParams.chargerDisableSetting = DEF_DISABLE_TEMP;
	privateEnvironmentParams.temperature.mode = DEF_TEMP_MODE;
	privateEnvironmentParams.temperature.lowSet = DEF_LOW_TEMP;
	privateEnvironmentParams.temperature.highSet = DEF_HIGH_TEMP;
	privateEnvironmentParams.temperature.hysteresis = DEF_TEMP_HYSTERESIS;
	privateEnvironmentParams.humidity.mode = DEF_HUM_MODE;
	privateEnvironmentParams.humidity.lowSet = DEF_LOW_HUM;
	privateEnvironmentParams.humidity.highSet = DEF_HIGH_HUM;
	privateEnvironmentParams.humidity.hysteresis = DEF_HUM_HYSTERESIS;
}

void getEnvironmentParameters(ENVIRONMENT_PARAM_TYPE *extParams)
{
	extParams->alarmSamplePeriod = privateEnvironmentParams.alarmSamplePeriod;
	extParams->alarmTime = privateEnvironmentParams.alarmTime;
	extParams->samplePeriod = privateEnvironmentParams.samplePeriod;
	extParams->chargerDisableSetting = privateEnvironmentParams.chargerDisableSetting;
	extParams->coolDownPeriod = privateEnvironmentParams.coolDownPeriod;
	extParams->sensorWarmUp = privateEnvironmentParams.sensorWarmUp;
	extParams->temperature.mode = privateEnvironmentParams.temperature.mode;
	extParams->temperature.lowSet = privateEnvironmentParams.temperature.lowSet;
	extParams->temperature.highSet = privateEnvironmentParams.temperature.highSet;
	extParams->temperature.hysteresis = privateEnvironmentParams.temperature.hysteresis;
	extParams->humidity.mode = privateEnvironmentParams.humidity.mode;
	extParams->humidity.lowSet = privateEnvironmentParams.humidity.lowSet;
	extParams->humidity.highSet = privateEnvironmentParams.humidity.highSet;
	extParams->humidity.hysteresis = privateEnvironmentParams.humidity.hysteresis;
}

void setEnvironmentParameters(ENVIRONMENT_PARAM_TYPE extParams)
{
	privateEnvironmentParams.alarmSamplePeriod = extParams.alarmSamplePeriod;
	privateEnvironmentParams.alarmTime = extParams.alarmTime;
	privateEnvironmentParams.samplePeriod = extParams.samplePeriod;
	privateEnvironmentParams.coolDownPeriod = extParams.coolDownPeriod;
	privateEnvironmentParams.chargerDisableSetting = extParams.chargerDisableSetting;
	privateEnvironmentParams.sensorWarmUp = extParams.sensorWarmUp;
	privateEnvironmentParams.temperature.mode = extParams.temperature.mode;
	privateEnvironmentParams.temperature.lowSet = extParams.temperature.lowSet;
	privateEnvironmentParams.temperature.highSet = extParams.temperature.highSet;
	privateEnvironmentParams.temperature.hysteresis = extParams.temperature.hysteresis;
	privateEnvironmentParams.humidity.mode = extParams.humidity.mode;
	privateEnvironmentParams.humidity.lowSet = extParams.humidity.lowSet;
	privateEnvironmentParams.humidity.highSet = extParams.humidity.highSet;
	privateEnvironmentParams.humidity.hysteresis = extParams.humidity.hysteresis;
}

bool decodeEnvironmentConfigs(uint8_t* mqttMsg)
{
	bool isError = false;
	uint8_t version = 256;
	uint16_t sampPeriod = 256;
	uint32_t alarmWindow = 65536;
	uint32_t alarmSample = 65536;
	uint32_t mutePeriod = 65536;
	uint16_t warmUp = 256;
	uint16_t chargeDisableT = 256;
	ENVIRONMENT_PARAM_TYPE decodedEnvironmentParams;
	char envBuff[MEMORY_MAX] = " ";
	char test[] = "\"environment\":{";
	char verTest[] = "\"version\":";
	char muteTest[] = "\"muting_period\":";
	char warmTest[] = "\"warm_up\":";
	char chargerTest[] = "\"charge_disable_temp\":";
	char alarmSampleTest[] = "\"alarm_sample_period\":";
	char alarmWindowTest[] = "\"alarm_window\":";
	char sampPeriodTest[] = "\"sample_period\":";
	char envErrStr[CONFIG_ERR_MSG_SIZE] = "";
	int buffSize = 0;


	//Need to get first to prevent overwriting non passed data such as even counter
	getEnvironmentParameters(&decodedEnvironmentParams);
	// Transfer MQTT message to a local buffer
	Word_Transfer(envBuff, (char*)mqttMsg);

	char* substr = strstr(envBuff, test);
	if(substr)
	{
		Refresh_Watchdog;
		buffSize += snprintf(envErrStr, CONFIG_ERR_MSG_SIZE, "\"environment\":[\"config_error\",");
		char *verStr = strstr(substr, verTest);
		if(verStr)
		{
			verStr += strlen(verTest);
			if(isdigit((unsigned char)verStr[0]))
			{
				version = atoi(verStr);
				if(version == 0)
				{
					char* sampPeriodStr = strstr(substr, sampPeriodTest);
					if(sampPeriodStr)
					{
						sampPeriodStr += strlen(sampPeriodTest);
						if(isdigit((unsigned char)sampPeriodStr[0]))
						{
							sampPeriod = atoi(sampPeriodStr);
							if(sampPeriod < 0 || sampPeriod > 255)
							{
								isError = true;
								buffSize += snprintf((envErrStr + buffSize), (CONFIG_ERR_MSG_SIZE - buffSize), "\"sample_period_out_of_range\",");
							}
						}
						else
						{
							isError = true;
							buffSize += snprintf((envErrStr + buffSize), (CONFIG_ERR_MSG_SIZE - buffSize), "\"invalid_sample_period_type\",");
						}
					}
					else
					{
						isError = true;
						buffSize += snprintf((envErrStr + buffSize), (CONFIG_ERR_MSG_SIZE - buffSize), "\"missing_sample_period\",");
					}

					char* alarmWindowStr = strstr(substr, alarmWindowTest);
					if(alarmWindowStr)
					{
						alarmWindowStr += strlen(alarmWindowTest);
						if(isdigit((unsigned char)alarmWindowStr[0]))
						{
							alarmWindow = atoi(alarmWindowStr);
							if(alarmWindow < 0 || alarmWindow > 65535)
							{
								isError = true;
								buffSize += snprintf((envErrStr + buffSize), (CONFIG_ERR_MSG_SIZE - buffSize), "\"alarm_window_out_of_range\",");
							}
						}
						else
						{
							isError = true;
							buffSize += snprintf((envErrStr + buffSize), (CONFIG_ERR_MSG_SIZE - buffSize), "\"invalid_alarm_window_type\",");
						}
					}
					else
					{
						isError = true;
						buffSize += snprintf((envErrStr + buffSize), (CONFIG_ERR_MSG_SIZE - buffSize), "\"missing_alarm_window\",");
					}

					char* alarmSampleStr = strstr(substr, alarmSampleTest);
					if(alarmSampleStr)
					{
						alarmSampleStr += strlen(alarmSampleTest);
						if(isdigit((unsigned char)alarmSampleStr[0]))
						{
							alarmSample = atoi(alarmSampleStr);
							if(alarmSample < 0 || alarmSample > 65535)
							{
								isError = true;
								buffSize += snprintf((envErrStr + buffSize), (CONFIG_ERR_MSG_SIZE - buffSize), "\"alarm_sample_period_out_of_range\",");
							}
						}
						else
						{
							isError = true;
							buffSize += snprintf((envErrStr + buffSize), (CONFIG_ERR_MSG_SIZE - buffSize), "\"invalid_alarm_sample_period_type\",");
						}
					}
					else
					{
						isError = true;
						buffSize += snprintf((envErrStr + buffSize), (CONFIG_ERR_MSG_SIZE - buffSize), "\"missing_alarm_sample_period_window\",");
					}

					char* muteStr = strstr(substr, muteTest);
					if(muteStr)
					{
						muteStr += strlen(muteTest);
						if(isdigit((unsigned char)muteStr[0]))
						{
							mutePeriod = atoi(muteStr);
							if(mutePeriod < 0 || mutePeriod > 65535)
							{
								isError = true;
								buffSize += snprintf((envErrStr + buffSize), (CONFIG_ERR_MSG_SIZE - buffSize), "\"muting_period_out_of_range\",");
							}
						}
						else
						{
							isError = true;
							buffSize += snprintf((envErrStr + buffSize), (CONFIG_ERR_MSG_SIZE - buffSize), "\"invalid_muting_period_type\",");
						}
					}
					else
					{
						isError = true;
						buffSize += snprintf((envErrStr + buffSize), (CONFIG_ERR_MSG_SIZE - buffSize), "\"missing_muting_period_window\",");
					}

					char* warmStr = strstr(substr, warmTest);
					if(warmStr)
					{
						warmStr += strlen(warmTest);
						if(isdigit((unsigned char)warmStr[0]))
						{
							warmUp = atoi(warmStr);
							if(warmUp < 0 || warmUp > 255)
							{
								isError = true;
								buffSize += snprintf((envErrStr + buffSize), (CONFIG_ERR_MSG_SIZE - buffSize), "\"warm_up_out_of_range\",");
							}
						}
						else
						{
							isError = true;
							buffSize += snprintf((envErrStr + buffSize), (CONFIG_ERR_MSG_SIZE - buffSize), "\"invalid_warm_up_type\",");
						}
					}
					else
					{
						isError = true;
						buffSize += snprintf((envErrStr + buffSize), (CONFIG_ERR_MSG_SIZE - buffSize), "\"missing_warm_up_window\",");
					}

					char* chargerStr = strstr(substr, chargerTest);
					if(chargerStr)
					{
						chargerStr += strlen(chargerTest);
						if(isdigit((unsigned char)warmStr[0]))
						{
							chargeDisableT = atoi(chargerStr);
							if(chargeDisableT < 0 || chargeDisableT > 255)
							{
								isError = true;
								buffSize += snprintf((envErrStr + buffSize), (CONFIG_ERR_MSG_SIZE - buffSize), "\"charger_disable_temp_out_of_range\",");
							}
						}
						else
						{
							isError = true;
							buffSize += snprintf((envErrStr + buffSize), (CONFIG_ERR_MSG_SIZE - buffSize), "\"invalid_charger_disable_temp_type\",");
						}
					}
					else
					{
						isError = true;
						buffSize += snprintf((envErrStr + buffSize), (CONFIG_ERR_MSG_SIZE - buffSize), "\"missing_charger_disable_temp_window\",");
					}
				}
				else
				{
					isError = true;
					buffSize += snprintf((envErrStr + buffSize), (CONFIG_ERR_MSG_SIZE - buffSize), "\"version_mismatch\",");
				}
			}
			else
			{
				isError = true;
				buffSize += snprintf((envErrStr + buffSize), (CONFIG_ERR_MSG_SIZE - buffSize), "\"invalid_version_type\",");
			}
		}
		else
		{
			isError = true;
			buffSize += snprintf((envErrStr + buffSize), (CONFIG_ERR_MSG_SIZE - buffSize), "\"missing_version\",");
		}
	}
	else
	{
		isError = true;
	}

	if(!isError)
	{
		decodedEnvironmentParams.alarmSamplePeriod = alarmSample;
		decodedEnvironmentParams.alarmTime = alarmWindow;
		decodedEnvironmentParams.chargerDisableSetting = chargeDisableT;
		decodedEnvironmentParams.sensorWarmUp = warmUp;
		decodedEnvironmentParams.samplePeriod = sampPeriod;
		decodedEnvironmentParams.coolDownPeriod = mutePeriod;
		setEnvironmentParameters(decodedEnvironmentParams);
	}
	else
	{
		if(buffSize > 0 && buffSize < CONFIG_ERR_MSG_SIZE - 2 && envErrStr[0] != '\0')
		{
			if(envErrStr[buffSize - 1] == ',')
			{
				envErrStr[buffSize - 1] = ']';
				addErrorString(envErrStr);
			}
		}
	}

	return isError;
}

bool decodeTemperatureConfigs(uint8_t* mqttMsg)
{
	bool isError = false;
	uint8_t version = 255;
	uint16_t mode = 256;
	int16_t lowThresh = 256;
	int16_t highThresh = 256;
	uint16_t hysteresis = 256;
	char tempBuff[MEMORY_MAX] = " ";
	char test[] = "\"temperature\":{";
	char verTest[] = "\"version\":";
	char modeTest[] = "\"mode\":";
	char lowTest[] = "\"low_threshold\":";
	char highTest[] = "\"high_threshold\":";
	char hysterTest[] = "\"hysteresis\":";
	char tempErrStr[CONFIG_ERR_MSG_SIZE] = "";
	int buffSize = 0;
	ENVIRONMENT_PARAM_TYPE decodedEnvironmentParams;


	//Need to get first to prevent overwriting non passed data
	getEnvironmentParameters(&decodedEnvironmentParams);
	// Transfer MQTT message to a local buffer
	Word_Transfer(tempBuff, (char*)mqttMsg);

	char* substr = strstr(tempBuff, test);
	if(substr)
	{
		buffSize += snprintf(tempErrStr, CONFIG_ERR_MSG_SIZE, "\"temperature\":[\"config_error\",");
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
					char* modeStr = strstr(substr, modeTest);
					if(modeStr)
					{
						modeStr += strlen(modeTest);
						if(isdigit((unsigned char)modeStr[0]))
						{
							mode = atoi(modeStr);
							if(mode < 0 || mode > 255)
							{
								isError = true;
								buffSize += snprintf((tempErrStr + buffSize), (CONFIG_ERR_MSG_SIZE - buffSize), "\"mode_out_of_range\",");
							}
						}
						else
						{
							isError = true;
							buffSize += snprintf((tempErrStr + buffSize), (CONFIG_ERR_MSG_SIZE - buffSize), "\"invalid_mode_type\",");
						}
					}
					else
					{
						isError = true;
						buffSize += snprintf((tempErrStr + buffSize), (CONFIG_ERR_MSG_SIZE - buffSize), "\"missing_mode\",");
					}

					char* lowStr = strstr(substr, lowTest);
					if(lowStr)
					{
						lowStr += strlen(lowTest);
						if(isdigit((unsigned char)lowStr[0]))
						{
							lowThresh = atoi(lowStr);
							if(lowThresh < 0 || lowThresh > 255)
							{
								isError = true;
								buffSize += snprintf((tempErrStr + buffSize), (CONFIG_ERR_MSG_SIZE - buffSize), "\"low_threshold_out_of_range\",");
							}
						}
						else
						{
							isError = true;
							buffSize += snprintf((tempErrStr + buffSize), (CONFIG_ERR_MSG_SIZE - buffSize), "\"invalid_low_threshold_type\",");
						}
					}
					else
					{
						isError = true;
						buffSize += snprintf((tempErrStr + buffSize), (CONFIG_ERR_MSG_SIZE - buffSize), "\"missing_low_threshold\",");
					}

					char* highStr = strstr(substr, highTest);
					if(highStr)
					{
						highStr += strlen(highTest);
						if(isdigit((unsigned char)lowStr[0]))
						{
							highThresh = atoi(highStr);
							if(highThresh < 0 || highThresh > 255)
							{
								isError = true;
								buffSize += snprintf((tempErrStr + buffSize), (CONFIG_ERR_MSG_SIZE - buffSize), "\"high_threshold_out_of_range\",");
							}
						}
						else
						{
							isError = true;
							buffSize += snprintf((tempErrStr + buffSize), (CONFIG_ERR_MSG_SIZE - buffSize), "\"invalid_high_threshold_type\",");
						}
					}
					else
					{
						isError = true;
						buffSize += snprintf((tempErrStr + buffSize), (CONFIG_ERR_MSG_SIZE - buffSize), "\"missing_high_threshold\",");
					}

					char* hysterStr = strstr(substr, hysterTest);
					if(hysterStr)
					{
						hysterStr += strlen(hysterTest);
						if(isdigit((unsigned char)hysterStr[0]))
						{
							hysteresis = atoi(highStr);
							if(hysteresis < 0 || hysteresis > 255)
							{
								isError = true;
								buffSize += snprintf((tempErrStr + buffSize), (CONFIG_ERR_MSG_SIZE - buffSize), "\"hysteresis_out_of_range\",");
							}
						}
						else
						{
							isError = true;
							buffSize += snprintf((tempErrStr + buffSize), (CONFIG_ERR_MSG_SIZE - buffSize), "\"invalid_hysteresis_type\",");
						}
					}
					else
					{
						isError = true;
						buffSize += snprintf((tempErrStr + buffSize), (CONFIG_ERR_MSG_SIZE - buffSize), "\"missing_hysteresis\",");
					}
				}
				else
				{
					isError = true;
					buffSize += snprintf((tempErrStr + buffSize), (CONFIG_ERR_MSG_SIZE - buffSize), "\"version_mismatch\",");
				}
			}
			else
			{
				isError = true;
				buffSize += snprintf((tempErrStr + buffSize), (CONFIG_ERR_MSG_SIZE - buffSize), "\"invalid_version_type\",");
			}
		}
		else
		{
			isError = true;
			buffSize += snprintf((tempErrStr + buffSize), (CONFIG_ERR_MSG_SIZE - buffSize), "\"missing_version\",");
		}
	}
	else
	{
		isError = true;
	}

	if(!isError)
	{
		decodedEnvironmentParams.temperature.mode = mode;
		decodedEnvironmentParams.temperature.lowSet = lowThresh;
		decodedEnvironmentParams.temperature.highSet = highThresh;
		decodedEnvironmentParams.temperature.hysteresis = hysteresis;
		setEnvironmentParameters(decodedEnvironmentParams);
	}
	else
	{
		if(buffSize > 0 && buffSize < CONFIG_ERR_MSG_SIZE - 2 && tempErrStr[0] != '\0')
		{
			if(tempErrStr[buffSize - 1] == ',')
			{
				tempErrStr[buffSize - 1] = ']';
				addErrorString(tempErrStr);
			}
		}
	}
	return isError;
}

bool decodeHumidityConfigs(uint8_t* mqttMsg)
{
	bool isError = false;
	uint8_t version = 255;
	uint16_t mode = 256;
	int16_t lowThresh = 256;
	int16_t highThresh = 256;
	uint16_t hysteresis = 256;
	char tempBuff[MEMORY_MAX] = " ";
	char test[] = "\"humidity\":{";
	char verTest[] = "\"version\":";
	char modeTest[] = "\"mode\":";
	char lowTest[] = "\"low_threshold\":";
	char highTest[] = "\"high_threshold\":";
	char hysterTest[] = "\"hysteresis\":";
	char humErrStr[CONFIG_ERR_MSG_SIZE] = "";
	int buffSize = 0;
	ENVIRONMENT_PARAM_TYPE decodedEnvironmentParams;

	//Need to get first to prevent overwriting non passed data
	getEnvironmentParameters(&decodedEnvironmentParams);
	// Transfer MQTT message to a local buffer
	Word_Transfer(tempBuff, (char*)mqttMsg);

	char* substr = strstr(tempBuff, test);
	if(substr)
	{
		buffSize += snprintf(humErrStr, CONFIG_ERR_MSG_SIZE, "\"humidity\":[\"config_error\",");
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
					char* modeStr = strstr(substr, modeTest);
					if(modeStr)
					{
						modeStr += strlen(modeTest);
						if(isdigit((unsigned char)modeStr[0]))
						{
							mode = atoi(modeStr);
							if(mode < 0 || mode > 255)
							{
								isError = true;
								buffSize += snprintf((humErrStr + buffSize), (CONFIG_ERR_MSG_SIZE - buffSize), "\"mode_out_of_range\",");
							}
						}
						else
						{
							isError = true;
							buffSize += snprintf((humErrStr + buffSize), (CONFIG_ERR_MSG_SIZE - buffSize), "\"invalid_mode_type\",");
						}
					}
					else
					{
						isError = true;
						buffSize += snprintf((humErrStr + buffSize), (CONFIG_ERR_MSG_SIZE - buffSize), "\"missing_mode\",");
					}

					char* lowStr = strstr(substr, lowTest);
					if(lowStr)
					{
						lowStr += strlen(lowTest);
						if(isdigit((unsigned char)lowStr[0]))
						{
							lowThresh = atoi(lowStr);
							if(lowThresh < 0 || lowThresh > 255)
							{
								isError = true;
								buffSize += snprintf((humErrStr + buffSize), (CONFIG_ERR_MSG_SIZE - buffSize), "\"low_threshold_out_of_range\",");
							}
						}
						else
						{
							isError = true;
							buffSize += snprintf((humErrStr + buffSize), (CONFIG_ERR_MSG_SIZE - buffSize), "\"invalid_low_threshold_type\",");
						}
					}
					else
					{
						isError = true;
						buffSize += snprintf((humErrStr + buffSize), (CONFIG_ERR_MSG_SIZE - buffSize), "\"missing_low_threshold\",");
					}

					char* highStr = strstr(substr, highTest);
					if(highStr)
					{
						highStr += strlen(highTest);
						if(isdigit((unsigned char)lowStr[0]))
						{
							highThresh = atoi(highStr);
							if(highThresh < 0 || highThresh > 255)
							{
								isError = true;
								buffSize += snprintf((humErrStr + buffSize), (CONFIG_ERR_MSG_SIZE - buffSize), "\"high_threshold_out_of_range\",");
							}
						}
						else
						{
							isError = true;
							buffSize += snprintf((humErrStr + buffSize), (CONFIG_ERR_MSG_SIZE - buffSize), "\"invalid_high_threshold_type\",");
						}
					}
					else
					{
						isError = true;
						buffSize += snprintf((humErrStr + buffSize), (CONFIG_ERR_MSG_SIZE - buffSize), "\"missing_high_threshold\",");
					}

					char* hysterStr = strstr(substr, hysterTest);
					if(hysterStr)
					{
						hysterStr += strlen(hysterTest);
						if(isdigit((unsigned char)hysterStr[0]))
						{
							hysteresis = atoi(highStr);
							if(hysteresis < 0 || hysteresis > 255)
							{
								isError = true;
								buffSize += snprintf((humErrStr + buffSize), (CONFIG_ERR_MSG_SIZE - buffSize), "\"hysteresis_out_of_range\",");
							}
						}
						else
						{
							isError = true;
							buffSize += snprintf((humErrStr + buffSize), (CONFIG_ERR_MSG_SIZE - buffSize), "\"invalid_hysteresis_type\",");
						}
					}
					else
					{
						isError = true;
						buffSize += snprintf((humErrStr + buffSize), (CONFIG_ERR_MSG_SIZE - buffSize), "\"missing_hysteresis\",");
					}
				}
				else
				{
					isError = true;
					buffSize += snprintf((humErrStr + buffSize), (CONFIG_ERR_MSG_SIZE - buffSize), "\"version_mismatch\",");
				}
			}
			else
			{
				isError = true;
				buffSize += snprintf((humErrStr + buffSize), (CONFIG_ERR_MSG_SIZE - buffSize), "\"invalid_version_type\",");
			}
		}
		else
		{
			isError = true;
			buffSize += snprintf((humErrStr + buffSize), (CONFIG_ERR_MSG_SIZE - buffSize), "\"missing_version\",");
		}
	}
	else
	{
		isError = true;
	}

	if(!isError)
	{
		decodedEnvironmentParams.temperature.mode = mode;
		decodedEnvironmentParams.temperature.lowSet = lowThresh;
		decodedEnvironmentParams.temperature.highSet = highThresh;
		decodedEnvironmentParams.temperature.hysteresis = hysteresis;
		setEnvironmentParameters(decodedEnvironmentParams);
	}
	else
	{
		if(buffSize > 0 && buffSize < CONFIG_ERR_MSG_SIZE - 2 && humErrStr[0] != '\0')
		{
			if(humErrStr[buffSize - 1] == ',')
			{
				humErrStr[buffSize - 1] = ']';
				addErrorString(humErrStr);
			}
		}
	}
	return isError;
}


char* getEnvironmentConfigStr(void)
{
	static char envBuff[ENV_CONFIG_MSG_SIZE] = {0};
	static char failed[500] = "Failed to build location message\r\n";
	char timebuff[100] = {""}; //
	int buffSize = 0;

	Time_StampISO(timebuff);

	buffSize = snprintf(envBuff, ENV_CONFIG_MSG_SIZE, "\"environment\":{\"version\":%u,\"sample_period\":%u,\"alarm_window\":%u,\"alarm_sample_period\":%u,\"muting_period\":%u,\"warm_up\":%u,\"charger_disable_temp\":%u}",
			ENV_CONFIG_VERSION, privateEnvironmentParams.samplePeriod, privateEnvironmentParams.alarmTime, privateEnvironmentParams.alarmSamplePeriod, privateEnvironmentParams.coolDownPeriod, privateEnvironmentParams.sensorWarmUp, privateEnvironmentParams.chargerDisableSetting);

	if(buffSize > 0 && buffSize < ENV_CONFIG_MSG_SIZE)
	{
		return envBuff;
	}
	else
	{
		return failed;
	}
}


char* getTemperatureConfigStr(void)
{
	static char tempBuff[ENV_CONFIG_MSG_SIZE] = {0};
	static char failed[500] = "Failed to build location message\r\n";
	char timebuff[100] = {""}; //
	int buffSize = 0;

	Time_StampISO(timebuff);

	buffSize = snprintf(tempBuff, ENV_CONFIG_MSG_SIZE, "\"temperature\":{\"version\":%u,\"mode\":%u,\"low_threshold\":%d,\"high_threshold\":%d,\"hysteresis\":%d}",
			TEMP_CONFIG_VERSION, privateEnvironmentParams.temperature.mode, privateEnvironmentParams.temperature.lowSet, privateEnvironmentParams.temperature.highSet, privateEnvironmentParams.temperature.hysteresis);

	if(buffSize > 0 && buffSize < ENV_CONFIG_MSG_SIZE)
	{
		return tempBuff;
	}
	else
	{
		return failed;
	}
}


char* getHumidityConfigStr(void)
{
	static char humBuff[ENV_CONFIG_MSG_SIZE] = {0};
	static char failed[500] = "Failed to build location message\r\n";
	char timebuff[100] = {""}; //
	int buffSize = 0;

	Time_StampISO(timebuff);

	buffSize = snprintf(humBuff, ENV_CONFIG_MSG_SIZE, "\"humidity\":{\"version\":%u,\"mode\":%u,\"low_threshold\":%d,\"high_threshold\":%d,\"hysteresis\":%d}",
			HUM_CONFIG_VERSION, privateEnvironmentParams.humidity.mode, privateEnvironmentParams.humidity.lowSet, privateEnvironmentParams.humidity.highSet, privateEnvironmentParams.humidity.hysteresis);

	if(buffSize > 0 && buffSize < ENV_CONFIG_MSG_SIZE)
	{
		return humBuff;
	}
	else
	{
		return failed;
	}
}


