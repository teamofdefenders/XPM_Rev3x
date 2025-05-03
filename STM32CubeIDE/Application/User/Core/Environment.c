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

}

