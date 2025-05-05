/**
 ******************************************************************************
 * @file    PIR.h
 * @author  Team of Defenders
 * @brief   Passive IR detector driver
 ******************************************************************************
 *  Demo version of PIR
 *
 * Copyright 2019 Team of Defenders
 *
 ******************************************************************************
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __PIR_H__
#define __PIR_H__

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/

#include "Functions.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/

#define PIR_CONFIG_VERSION 0
#define PIR_DAY_VERSION 0
#define PIR_NIGHT_VERSION 0
#define PIR_MODE_DEF 3
#define PIR_MUTE_DEFAULT 1800

#define DAY_CONF_MOTION_WINDOW_DEF 10
#define DAY_THRESH_DEF 80
#define DAY_BLACKOUT_DEF 300
#define DAY_NO_MOTION_DEF 60
#define DAY_NEAR_MOTION_DEF 70

#define NIGHT_CONF_MOTION_WINDOW_DEF 10
#define NIGHT_THRESH_DEF 60
#define NIGHT_BLACKOUT_DEF 120
#define NIGHT_NO_MOTION_DEF 60
#define NIGHT_NEAR_MOTION_DEF 50

#define PIR_MSG_SIZE 500

/* External variables --------------------------------------------------------*/

/* Exported functions ------------------------------------------------------- */

typedef enum
{
	PIR_CONFIRM_MOTION,
	MOTION_BLACKOUT,
	CONFIRM_NO_MOTION,
	PIR_IDLE
} PIR_STATE_TYPE;


typedef enum
{
	PIR_DAY,
	PIR_NIGHT
} FILTER_TYPE;

typedef struct
{
	// KCS fix these names
	uint16_t Motion_Confirm_Window; 		 // Window time in seconds to confirm motion
	uint8_t Motion_Threshhold; 			 // percentage of motion in Window to confimr motion, not implemented yet
	uint16_t Motion_Blackout; 			 // Time in seconds to ignore PIR used
	uint16_t No_Motion_Detection_Window; // Window time in seconds to confirm motion stopped used
	uint8_t Near_Motion_Threshhold; 	 // percentage of motion in Confirm Window which is below threshold not implemented yet
} PIR_FILTER_TYPE;

//PIR mode Bit Definition
//0 - Night Enabled/Disable
//1 - Day Enable/Disable
//2 - Reserved
//3 - Reserved
//4 - Reserved
//5 - Reserved
//6 - Reserved
//7 - Reserved
//0 - always disable, 3 - always enable, 1 - night only, 2 - day only
typedef struct
{
	uint8_t mode; 	    // PIR Mode, see above
	uint16_t coolDown;	// Time in seconds between sending Motion start events
} PIR_PARAMETER_TYPE;


void pirDataInit ( void );
void pirParametersInit(void);
void setPirParameters(PIR_PARAMETER_TYPE extPirParameters);
void getPirParameters(PIR_PARAMETER_TYPE *extPirParameters);
void setFilterParameters(PIR_FILTER_TYPE extFilterParameters, FILTER_TYPE time);
void getFilterParameters(PIR_FILTER_TYPE *extFilterParameters, FILTER_TYPE time );
bool decodePIRConfigs( uint8_t *mqttMsg );
bool decodeMotionFilter( uint8_t *mqttMsg , FILTER_TYPE sunPeriod);
char* getPirConfigStr( void );
uint16_t getPirMutePeriod(void);

void setPIRTrigger ( void );
void PIR_ReCall ( MEM_PTR *Data_Ptr );

/* PIR event callback function*/
//static void OnPIREvent( void* context );
#ifdef __cplusplus
}
#endif

#endif /* __PIR_H__ */
/*********************   END OF FILE     ***************/
