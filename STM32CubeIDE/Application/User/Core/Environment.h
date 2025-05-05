/**
 ******************************************************************************
 * @file    Environment.h
 * @author  Team of Defenders
 * @brief   Header file for environmental monitoring
 ******************************************************************************
 *
 * Copyright 2025 Team of Defenders
 *
 ******************************************************************************
 */
#ifndef ENVIRONMENT_H_
#define ENVIRONMENT_H_

#include "Functions.h"
#include "string.h"

//Private definitions
#define DEF_ENV_SAMPLE 1800 //Default sample period seconds
#define DEF_ENV_ALARM_TIME 1800 //Default alarm time seconds
#define DEF_ENV_ALARM_PER 200 //Default alarm sample period seconds
#define DEF_ENV_COOL_DOWN 60 //Default alarm cool down seconds
#define DEF_WARM_UP 5 //Default sensor warm up seconds
#define DEF_DISABLE_TEMP 70 //Default charger disable temperature degrees C
#define DEF_TEMP_MODE 7
#define DEF_LOW_TEMP 20
#define DEF_HIGH_TEMP 60
#define DEF_TEMP_HYSTERESIS 5
#define DEF_HUM_MODE 7
#define DEF_LOW_HUM 20
#define DEF_HIGH_HUM 90
#define DEF_HUM_HYSTERESIS 5
#define ENV_CONFIG_MSG_SIZE 500
#define ENV_CONFIG_VERSION 0
#define TEMP_CONFIG_VERSION 0
#define HUM_CONFIG_VERSION 0

#endif
