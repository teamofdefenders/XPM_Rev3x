/*
 ******************************************************************************
 * @file    Buzzer.h
 * @author  Team of Defenders
 * @brief   Header file for buzzer functions
 ******************************************************************************
 *
 * Copyright 2025 Team of Defenders
 *
 ******************************************************************************
 */

#ifndef BUZZER_H_
#define BUZZER_H_

/* Includes ------------------------------------------------------------------*/

#include "stdbool.h"
#include "stdlib.h"
#include "stdio.h"
#include <stdint.h>

/* Private typedef -----------------------------------------------------------*/

//Buzzer Mode Bit Definition
//
//0 - Night Enabled/Disable
//1 - Day Enable/Disable
//2 - Start Motion Enable/Disable
//3 - Start Movement Enable/Disable
//4 - Cell Warning Enable/Disable
//5 - Server Warning Enable/Disable
//6 - Reserved
//7 - Reserved
//0 - always disable
typedef struct
{
	uint8_t mode;      // See above
} BUZZER_PARAMETER_TYPE;

/* Private define ------------------------------------------------------------*/
#define BUZ_MSG_SIZE 500
#define BUZ_LENGTH_MOTION_msec   3001

#define DEF_BUZZ_MODE 0
#define BUZZ_CONFIG_VERSION 0

/* Private macro -------------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/

#endif /* BUZZER_H_ */
