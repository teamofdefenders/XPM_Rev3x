/**
 ******************************************************************************
 * @file    Camera.h
 * @author  Kevin Stacey
 * @brief   driver for OSPI camera
 ******************************************************************************
 * @attention
 *
 * Copyright 2025 Team of Defenders
 *
 ******************************************************************************
 */

#ifndef CAMERA_H_
#define CAMERA_H_

/******************************************************
 File Includes
 Note: n/a
 ******************************************************/

#include "Functions.h"
#include "ArducamCamera.h"

/******************************************************
 Define Values
 Note: Values Gathered from Datasheet
 ******************************************************/

#define DEF_CAM_MODE 0
#define DEF_CAM_WARMUP 1
#define DEF_CAM_PIC_PER_MOTION 1
#define DEF_CAM_PIC_INTERVAL 1
#define DEF_CAM_SAMPLE_HOUR 13
#define DEF_CAM_SAMPLE_MIN 0
#define DEF_CAM_NUM_SAMPLES 2
#define CAM_MSG_SIZE 500
#define CAM_CONFIG_VERSION 0
/******************************************************
 Global Enum
 Note: Values are used for error codes,
 command types, and XPS state
 ******************************************************/

typedef enum
{		// Error codes used for manage debugging
	CAM_OK = 0,  // Chip OK - Execution fine
	CAM_NOT_OK,  // Chip NOT OK - Execution NOTE fine
} CAM_STATE;

typedef enum
{		// Error codes used for manage debugging
	CAM_IS_READY = 0, // Chip OK - Execution fine
	CAM_IS_NOT_READY, // Chip NOT OK - Execution NOT fine
} CAM_READY;

typedef struct
{
	uint8_t pictureBuffer [ PIC_MAX ]; // Picture storage buffer
	uint32_t pictureSize;              // Picture size
	CAM_READY readyStatus;             // Camera Execution State
	CAM_STATE stateStatus;             // Camera Execution State
	ArducamCamera arducamInstance;     // Arducam structure
} CAM_STATUS_REG;

/******************************************************
 Function Prototypes
 Note: Typical void functions are placed with ptr
 pointer due to logic flaw in the ucontroller
 ******************************************************/

void cameraInitialize ( void );
void printCameraStatus ( void );
void takeCameraImage ( CAMERA_DATA_TYPE * cameraData, bool printImage );
CAM_STATE CAM_Get_State ( void );
CAM_READY CAM_Get_Ready ( void );
bool getCameraPower ( void );
void setCameraPower ( bool cameraPower );

// Future camera downlink and configuration handling
// void cameraParametersInit(void);
//bool decodeCameraConfigs(uint8_t *mqttMsg);
//char* getCameraConfigStr(void);
/******************************************************
 Shared Global Variables
 Note:
 ******************************************************/
extern I2C_HandleTypeDef hi2c1;			// External SPI configuration
extern IWDG_HandleTypeDef hiwdg;		// External IWDG configuration

#endif  //CAMERA_H_

