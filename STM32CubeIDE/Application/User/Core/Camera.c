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
CAMERA_PARAMETER_TYPE privateCameraParameters;
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


void cameraParametersInit()
{
	privateCameraParameters.mode = DEF_CAM_MODE;
	privateCameraParameters.warmUp = DEF_CAM_WARMUP;
	privateCameraParameters.picturesPerEvent = DEF_CAM_PIC_PER_MOTION;
	privateCameraParameters.pictureInterval = DEF_CAM_PIC_INTERVAL;
	privateCameraParameters.samplePictureTime.hours = DEF_CAM_SAMPLE_HOUR;
	privateCameraParameters.samplePictureTime.minutes = DEF_CAM_SAMPLE_MIN;
	privateCameraParameters.samplePicNumber = DEF_CAM_NUM_SAMPLES;
}

void getCameraParameters(CAMERA_PARAMETER_TYPE *extCamParams)
{
	extCamParams->mode = privateCameraParameters.mode;
	extCamParams->warmUp = privateCameraParameters.warmUp;
	extCamParams->picturesPerEvent = privateCameraParameters.picturesPerEvent;
	extCamParams->pictureInterval = privateCameraParameters.pictureInterval;
	extCamParams->samplePictureTime.hours = privateCameraParameters.samplePictureTime.hours;
	extCamParams->samplePictureTime.minutes = privateCameraParameters.samplePictureTime.minutes;
	extCamParams->samplePicNumber = privateCameraParameters.samplePicNumber;
}

void setCameraParameters(CAMERA_PARAMETER_TYPE extCamParams)
{
	privateCameraParameters.mode = extCamParams.mode;
	privateCameraParameters.warmUp = extCamParams.warmUp;
	privateCameraParameters.picturesPerEvent = extCamParams.picturesPerEvent;
	privateCameraParameters.pictureInterval = extCamParams.pictureInterval;
	privateCameraParameters.samplePictureTime.hours = extCamParams.samplePictureTime.hours;
	privateCameraParameters.samplePictureTime.minutes = extCamParams.samplePictureTime.minutes;
	privateCameraParameters.samplePicNumber = extCamParams.samplePicNumber;
}

/**
 * @brief  Decodes camera parameters from server
 * @note
 * @param  uint8_t *mqttMsg
 * 		   Represents message to be decoded
 * @retval bool isError
 * 		   If no error, will return false
 */
bool decodeCameraConfigs(uint8_t* mqttMsg)
{
	bool isError = false;
	uint8_t version = 255;
	uint8_t mode = 255;
	uint16_t warmup = 0;
	uint16_t picPerMotion = 0;
	uint16_t picInterval = 0;
	uint16_t samplePerDay = 0;
	uint8_t sampleTimeH = 0;
	uint8_t sampleTimeM = 0;
	bool picPerMotionValid = false;
	bool warmupValid = false;
	bool samplePerDayValid = false;
	bool picIntervalValid = false;
	bool sampleTimeValid = false;
	bool modeValid = false;
	char localBuff[MEMORY_MAX] = " ";
	char camTest [] = "\"camera\":{";
	char modeTest [] = "\"mode\":";
	char versionTest [] = "\"version\":";
	char warmTest [] = "\"warm_up\":";
	char picPerMotionTest [] = "\"pictures_per_motion\":";
	char picIntervalTest [] = "\"picture_interval\":";
	char samplePerDayTest [] = "\"sample_pictures_day\":";
	char sampleTimeTest [] = "\"sample_picture_time\":";
	char camErrStr[CONFIG_ERR_MSG_SIZE] = "";
	int buffSize = 0;

	// Transfer MQTT message to a local buffer to prevent tampering original mqtt message
	Word_Transfer(localBuff, (char*)mqttMsg);

	char *subStr = strstr(localBuff, camTest);
	if(subStr && !isError)
	{
		buffSize += snprintf(camErrStr, CONFIG_ERR_MSG_SIZE, "\"camera\":[\"config_error\",");
		char *verStr = strstr(subStr, versionTest);
		if(verStr)
		{
			verStr += strlen(versionTest);
			if(isdigit((unsigned char)verStr[0]))
			{
				version = atoi(verStr);
				if(version == 0)
				{
					char *warmStr = strstr(subStr, warmTest);
					if(warmStr)
					{
						warmStr += strlen(warmTest);
						if(isdigit((unsigned char)warmStr[0]))
						{
							warmup = atoi(warmStr);
							if(warmup >= 1 && warmup <= 255)
							{
								warmupValid = true;
							}
							else
							{
								PRINTF("Warm up out of range: %d\r\n", warmup);
								buffSize += snprintf((camErrStr + buffSize), (CONFIG_ERR_MSG_SIZE - buffSize), "\"warm_up_out_of_range_[1-255]\",");
								isError = true;
							}
						}
						else
						{
							PRINTF("Invalid data type for warm up\r\n");
							isError = true;
							buffSize += snprintf((camErrStr + buffSize), (CONFIG_ERR_MSG_SIZE - buffSize), "\"invalid_warm_up_type_NAN\",");
						}
					}
					else
					{
						PRINTF("Warm up parameter not found in camera configs\r\n");
						isError = true;
						buffSize += snprintf((camErrStr + buffSize), (CONFIG_ERR_MSG_SIZE - buffSize), "\"missing_warm_up_string\",");
					}

					char *modeStr = strstr(subStr, modeTest);
					if(modeStr)
					{
						modeStr += strlen(modeTest);
						if(isdigit((unsigned char)modeStr[0]))
						{
							mode = atoi(modeStr);
							if(warmup >= 0 && warmup <= 255)
							{
								modeValid = true;
							}
							else
							{
								PRINTF("Mode out of range: %d\r\n", mode);
								isError = true;
								buffSize += snprintf((camErrStr + buffSize), (CONFIG_ERR_MSG_SIZE - buffSize), "\"mode_out_of_range_[0-255]\",");
							}
						}
						else
						{
							PRINTF("Invalid data type for mode\r\n");
							isError = true;
							buffSize += snprintf((camErrStr + buffSize), (CONFIG_ERR_MSG_SIZE - buffSize), "\"invalid_mode_type_NAN\",");
						}
					}
					else
					{
						PRINTF("Mode parameter not found in camera configs\r\n");
						buffSize += snprintf((camErrStr + buffSize), (CONFIG_ERR_MSG_SIZE - buffSize), "\"missing_mode_string\",");
						isError = true;
					}

					char *picPerMotionStr = strstr(subStr, picPerMotionTest);
					if(picPerMotionStr)
					{
						picPerMotionStr += strlen(picPerMotionTest);
						if(isdigit((unsigned char)warmStr[0]))
						{
							picPerMotion = atoi(picPerMotionStr);
							if(picPerMotion >= 0 && picPerMotion <= 255)
							{
								picPerMotionValid = true;
							}
							else
							{
								PRINTF("Pictures per motion out of range: %d\r\n", picPerMotion);
								isError = true;
								buffSize += snprintf((camErrStr + buffSize), (CONFIG_ERR_MSG_SIZE - buffSize), "\"picture_per_motion_out_of_range_[0-255]\",");
							}
						}
						else
						{
							PRINTF("Invalid data type for pictures per motion\r\n");
							isError = true;
							buffSize += snprintf((camErrStr + buffSize), (CONFIG_ERR_MSG_SIZE - buffSize), "\"invalid_picture_per_motion_type_NAN\",");
						}
					}
					else
					{
						PRINTF("Pictures per motion parameter not found in camera configs\r\n");
						buffSize += snprintf((camErrStr + buffSize), (CONFIG_ERR_MSG_SIZE - buffSize), "\"missing_picture_per_motion_string\",");
						isError = true;
					}

					char *picIntervalStr = strstr(subStr, picIntervalTest);
					if(picIntervalStr)
					{
						picIntervalStr += strlen(picIntervalTest);
						if(isdigit((unsigned char)picIntervalStr[0]))
						{
							picInterval = atoi(picIntervalStr);
							if(picInterval >= 0 && picInterval <= 255)
							{
								picIntervalValid = true;
							}
							else
							{
								PRINTF("Picture interval out of range: %d\r\n", picInterval);
								buffSize += snprintf((camErrStr + buffSize), (CONFIG_ERR_MSG_SIZE - buffSize), "\"picture_interval_out_of_range_[0-255]\",");
							}
						}
						else
						{
							PRINTF("Invalid data type for picture interval\r\n");
							buffSize += snprintf((camErrStr + buffSize), (CONFIG_ERR_MSG_SIZE - buffSize), "\"invalid_picture_interval_type_NAN\",");
						}
					}
					else
					{
						PRINTF("Picture interval parameter not found in camera configs\r\n");
						isError = true;
						buffSize += snprintf((camErrStr + buffSize), (CONFIG_ERR_MSG_SIZE - buffSize), "\"missing_picture_interval_string\",");
					}

					char *samplePerDayStr = strstr(subStr, samplePerDayTest);
					if(samplePerDayStr)
					{
						samplePerDayStr += strlen(samplePerDayTest);
						if(isdigit((unsigned char)samplePerDayStr[0]))
						{
							samplePerDay = atoi(samplePerDayStr);
							if(samplePerDay >= 0 && samplePerDay <= 255)
							{
								samplePerDayValid = true;
							}
							else
							{
								PRINTF("Sample pictures per day out of range: %d\r\n", samplePerDay);
								isError = true;
								buffSize += snprintf((camErrStr + buffSize), (CONFIG_ERR_MSG_SIZE - buffSize), "\"sample_picture_day_out_of_range_[0-255]\",");
							}
						}
						else
						{
							PRINTF("Invalid data type for sample pictures per day\r\n");
							isError = true;
							buffSize += snprintf((camErrStr + buffSize), (CONFIG_ERR_MSG_SIZE - buffSize), "\"invalid_sample_picture_day_type_NAN\",");
						}
					}
					else
					{
						PRINTF("Sample pictures per day parameter not found in camera configs\r\n");
						isError = true;
						buffSize += snprintf((camErrStr + buffSize), (CONFIG_ERR_MSG_SIZE - buffSize), "\"missing_sample_picture_day_string\",");
					}

					char *sampleTimeStr = strstr(subStr, sampleTimeTest);
					if(sampleTimeStr)
					{
						char *sampleTimeStrH = sampleTimeStr + (strlen(sampleTimeTest) + 1);
						char *sampleTimeStrM = sampleTimeStr + (strlen(sampleTimeTest) + 4);
						if(isdigit((unsigned char)sampleTimeStrH[0]) && isdigit((unsigned char)sampleTimeStrM[0]))
						{
							sampleTimeH = atoi(sampleTimeStrH);
							sampleTimeM = atoi(sampleTimeStrM);
							if(sampleTimeH >= 0 && sampleTimeH <= 23 && sampleTimeH >= 0 && sampleTimeH <= 59)
							{
								sampleTimeValid = true;
							}
							else
							{
								PRINTF("Sample time out out of range: %d hours, %d minutes\r\n", sampleTimeH, sampleTimeM);
								isError = true;
								buffSize += snprintf((camErrStr + buffSize), (CONFIG_ERR_MSG_SIZE - buffSize), "\"sample_picture_time_out_of_range\",");
							}
						}
						else
						{
							PRINTF("Invalid data type for sample time\r\n");
							isError = true;
							buffSize += snprintf((camErrStr + buffSize), (CONFIG_ERR_MSG_SIZE - buffSize), "\"invalid_sample_picture_time_type_NAN\",");
						}
					}
					else
					{
						PRINTF("Sample time parameter not found in camera configs\r\n");
						isError = true;
						buffSize += snprintf((camErrStr + buffSize), (CONFIG_ERR_MSG_SIZE - buffSize), "\"missing_sample_picture_time_string\",");
					}
				}
				else
				{
					PRINTF("Wrong version. Expecting 0, received: %d", version);
					isError = true;
					buffSize += snprintf((camErrStr + buffSize), (CONFIG_ERR_MSG_SIZE - buffSize), "\"version_mismatch\",");
				}
			}
			else
			{
				PRINTF("Invalid type for version\r\n");
				isError = true;
				buffSize += snprintf((camErrStr + buffSize), (CONFIG_ERR_MSG_SIZE - buffSize), "\"invalid_versioin_type\",");
			}
		}
		else
		{
			PRINTF("Version not found in Camera downlink string\r\n");
			isError = true;
			buffSize += snprintf((camErrStr + buffSize), (CONFIG_ERR_MSG_SIZE - buffSize), "\"missing_version\",");
		}
	}
	else
	{
		PRINTF("Camera configurations not found in configuration string\r\n");
		isError = true;
	}


	if(!isError && picPerMotionValid && warmupValid && samplePerDayValid && picIntervalValid && sampleTimeValid && modeValid)
	{
		privateCameraParameters.mode = mode;
		privateCameraParameters.pictureInterval = picInterval;
		privateCameraParameters.picturesPerEvent = picPerMotion;
		privateCameraParameters.warmUp = warmup;
		privateCameraParameters.samplePicNumber = samplePerDay;
		privateCameraParameters.samplePictureTime.hours = sampleTimeH;
		privateCameraParameters.samplePictureTime.hours = sampleTimeM;
	}
	else
	{
		if(buffSize > 0 && buffSize < CONFIG_ERR_MSG_SIZE - 2 && camErrStr[0] != '\0')
		{
			if(camErrStr[buffSize - 1] == ',')
			{
				camErrStr[buffSize - 1] = ']';
				addErrorString(camErrStr);
			}
		}
	}

	return isError;
}

/**
 * @brief  Returns configuration string with camera configs for status message
 * @note
 * @param  void
 * @retval returns config string or an error message if something goes wrong
 */
char* getCamConfigStr(void)
{
	static char buffer[CAM_MSG_SIZE] = {0};
	bool valid = false;

	int buffSize = snprintf(buffer, CAM_MSG_SIZE, "\"camera\":{\"version\":%u,\"mode\":%u,\"warm_up\":%u,\"pictures_per_motion\":%u,\"picture_interval\":%u,\"sample_picture_time\":\"%2u:%2u\",\"sample_pictures_day\":%u,\"exposure\":%u,\"iso\":%u,\"white_balance\":%u,\"white_balance_mode\":%u,\"ev\":%u,\"image_quality\":%u}",
			CAM_CONFIG_VERSION, privateCameraParameters.mode, privateCameraParameters.warmUp, privateCameraParameters.picturesPerEvent, privateCameraParameters.pictureInterval, privateCameraParameters.samplePictureTime.hours, privateCameraParameters.samplePictureTime.minutes, privateCameraParameters.samplePicNumber,123,123,123,123,123,123);

	if(buffSize > 0 && buffSize < CAM_MSG_SIZE)
	{
		valid = true;
	}

	if(valid)
	{
		return buffer;
	}
	else
	{
		return "Failed to build Camera message\r\n";
	}
}

#endif  //CAMERA_C_
