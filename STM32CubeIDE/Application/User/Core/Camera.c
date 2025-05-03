/**
 ******************************************************************************
 * @file    Camera.c
 * @author  Kevin Stacey
 * @brief   driver for OSPI camera
 ******************************************************************************
 * @attention
 *
 * Copyright 2024 Team of Defenders
 *
 ******************************************************************************
 */
#ifndef CAMERA_C_
#define CAMERA_C_

/******************************************************
 Includes
 Note:
 ******************************************************/

#include "Camera.h"
#include "ArducamCamera.h"
#include <string.h>

/******************************************************
 Global Variables
 Note:
 ******************************************************/
bool powerStatus = false;
bool isArcucamConfigured = false;
extern RTC_HandleTypeDef hrtc;
CAM_STATUS_REG cameraDataAndState;

/******************************************************
 Define Controls
 Note:
 ******************************************************/

#define Arducamera cameraDataAndState.arducamInstance
#define ARDUCAM_COMMAND_DELAY 130

#define LOG_Image_Data "\1 Base64 Encoded data \r\n __________________________________ \r\n\0"
#define CAM_Success "\1 Camera Setting Success\r\n\0"
#define CAM_Fail "\1 Camera Setting Fail\r\n\0"
#define ARDRU_XFER_SIZE 1024
#define CAMERA_UART_BUF_SIZE 64
/******************************************************/

void cameraInitialize ( void )
{
	if ((powerStatus) & (!isArcucamConfigured))
	{
		HAL_Delay(100);    //GAV is this necessary?
		// stateStatus is set in ArducamCamra.c file in the cameraWaitI2cIdle function not working
		cameraDataAndState.stateStatus = CAM_OK; // have to initialize this to "OK"
		cameraDataAndState.readyStatus = CAM_IS_NOT_READY;

        // chip select is 1
		(Arducamera) = createArducamCamera ( 1 );

		begin ( &(Arducamera) );
		printCameraStatus (  );

		Refresh_Watchdog;

		setAutoExposure ( &(Arducamera) , 1 );
		printCameraStatus (  );

		setAutoISOSensitive ( &(Arducamera) , 1 );
		printCameraStatus (  );

		//GAV are the following two settings reversed?
		setAutoWhiteBalance ( &(Arducamera) , 1 );
		printCameraStatus (  );

		setAutoWhiteBalanceMode ( &(Arducamera) , CAM_WHITE_BALANCE_MODE_OFFICE );
		printCameraStatus (  );

		setColorEffect ( &(Arducamera) , CAM_COLOR_FX_NONE );
		printCameraStatus (  );

		setSaturation ( &(Arducamera) , CAM_STAURATION_LEVEL_DEFAULT );
		printCameraStatus (  );

		setEV ( &(Arducamera) , CAM_EV_LEVEL_DEFAULT );
		printCameraStatus (  );

		setContrast ( &(Arducamera) , CAM_CONTRAST_LEVEL_DEFAULT );
		printCameraStatus (  );

		setBrightness ( &(Arducamera) , CAM_BRIGHTNESS_LEVEL_DEFAULT );
		printCameraStatus (  );

		setSharpness ( &(Arducamera) , CAM_SHARPNESS_LEVEL_AUTO );
		printCameraStatus (  );

		setImageQuality ( &(Arducamera) , LOW_QUALITY );
		printCameraStatus (  );

		//		lowPowerOn ( &(Arducamera) );
		//		printCameraStatus (  );
		//
		//		lowPowerOff ( &(Arducamera) );
		//		printCameraStatus (  );
		//
		//		cameraHeartBeat ( &(Arducamera) );
		//		printCameraStatus (  );

		Refresh_Watchdog;

		HAL_Delay ( 500 );
		stopPreview ( &(Arducamera) );
		printCameraStatus ( );

		cameraDataAndState.readyStatus = CAM_IS_READY;

		Refresh_Watchdog;

		isArcucamConfigured = true;
	}
}

void printCameraStatus ( void )
{
	Refresh_Watchdog;

	if (cameraDataAndState.stateStatus == CAM_OK) Log_Single ( CAM_Success );
	else Log_Single ( CAM_Fail );

	HAL_Delay ( ARDUCAM_COMMAND_DELAY );
}

/******************************************************/
void takeCameraImage ( CAMERA_DATA_TYPE * cameraData,  bool printImage )
{
	cameraDataAndState.readyStatus = CAM_IS_NOT_READY;

	// High Definition mode is  CAM_IMAGE_MODE_HD
	takePicture ( &(Arducamera) , CAM_IMAGE_MODE_QVGA , CAM_IMAGE_PIX_FMT_JPG );

	HAL_Delay ( 100 );

	Refresh_Watchdog;

	uint32_t cameraDataReadSize = 0;
	uint32_t cameraReadItterator = 0;
	while (Arducamera.receivedLength)
	{
		cameraDataReadSize += readBuff ( &(Arducamera) , &(cameraDataAndState.pictureBuffer [ cameraReadItterator ]) , ARDRU_XFER_SIZE );
		cameraReadItterator += ARDRU_XFER_SIZE;
	}

	// KCS ADD XPS SAVE for multiple pictures taken per PIR event
	//	HAL_GPIO_WritePin ( USB_Power_Enable_GPIO_Port , USB_Power_Enable_Pin , GPIO_PIN_RESET );    //for simulated battery power operation

	// encode requires the destination buffer size to be passed in and passes out in the same variable the actual encoded size
	size_t encodBuffSize = PIC_MAX;

	base64_encode ( cameraData->encoded , &encodBuffSize ,
			(const unsigned char*) cameraDataAndState.pictureBuffer ,
			Arducamera.totalLength );

	cameraDataAndState.pictureSize = (uint32_t) encodBuffSize;
	cameraData->encodedSize = (uint32_t) encodBuffSize;;

	Refresh_Watchdog;

	Log_Single ( LOG_Image_Data );

	if (printImage)
	{
		for (size_t iter = 0; iter <= encodBuffSize; iter += CAMERA_UART_BUF_SIZE)
		{
			vcom_Trace ( &cameraData->encoded [ iter ] , CAMERA_UART_BUF_SIZE , 1 );
		}
		vcom_Trace ( (uint8_t*) "\r\n\r\n" , 4 , 1 );
	}

	// KCS remove before release of 0.4.x
	// Print read size and compare to arducam structure value
	PRINTF("Camera read size, calculated %ul vs Stored %ul\r\n" , cameraDataReadSize, Arducamera.totalLength );

	cameraDataAndState.readyStatus = CAM_IS_READY;
}

CAM_STATE CAM_Get_State ( void )
{
	return cameraDataAndState.stateStatus;
}

CAM_READY CAM_Get_Ready ( void )
{
	return cameraDataAndState.readyStatus;
}


bool getCameraPower ( void )
{
	return powerStatus;
}

void setCameraPower ( bool cameraPower )
{
	powerStatus =  cameraPower;
	if (!powerStatus)
	{
		isArcucamConfigured = false;
	}
}

bool getCameraConfigStatus ( void )
{
	return isArcucamConfigured;
}


// KCS reference for adding Camera configuration
// void cameraParametersInit(void)
//

// void getCameraParameters(GPS_PARAMETER_TYPE *extGpsParameters)

// void setCameraParameters(GPS_PARAMETER_TYPE extGpsParameters)

//bool decodeCameraConfigs(uint8_t *mqttMsg)
//{
//
//}

//
//char* getCameraConfigStr(void)
//{
//	//CSC change hardcoded 500
//	static char buffer[GPS_MSG_SIZE] = {0};
//	static char failed[500] = "Failed to build GPS message.";
//	bool valid = false;
//
//	int buffSize = snprintf(buffer, GPS_MSG_SIZE, "\"gps\":{\"version\":%u,\"mode\":%u,\"gps_acquisition_period\":%u,\"threshold_distance\":%u,\"alarm_window\":%u,\"alarm_sample_period\":%u,\"hysteresis\":%u}",
//			GPS_CONFIG_VERSION, privateGpsParameters.mode, privateGpsParameters.gpsInterval, privateGpsParameters.geofenceDistance, privateGpsParameters.alarmTime, privateGpsParameters.alarmSamplePeriod, privateGpsParameters.movementHysteresis);
//
//	if(buffSize > 0 && buffSize < GPS_MSG_SIZE)
//	{
//		valid = true;
//	}
//
//	if(valid)
//	{
//		return buffer;
//	}
//	else
//	{
//		return failed;
//	}
//}

#endif  //CAMERA_C_
