/**
 ******************************************************************************
 * @file    XPS.c
 * @author  Kevin Stacey
 * @brief   driver for PCB FLASH MEMORY
 ******************************************************************************
 * @attention
 *
 * Copyright 2024 Team of Defenders
 *
 ******************************************************************************
 */
#ifndef __XPS_C
#define __XPS_C

//******************************************************
// Includes
// Note:
//******************************************************

#include "XPS.h"
#include <string.h>
#include "GPS.h"
#include "DayNight.h"
#include "PIR.h"

/******************************************************
 Global Variables
 Note:
 ******************************************************/
uint8_t paramPage1[256];  // shadow Register
XPS_STATUS_REG XPS;
OSPI_RegularCmdTypeDef OSPI_Setting;		// OPSI controller buffer
bool isDataValid = false;
/******************************************************
 Control Defines
 Note:
 ******************************************************/

#define Command XPS.COMMAND
#define WEL 	XPS.WEL		 	// Write enable latch (1 - write allowed)
#define QE 		XPS.QE			// Quad SPI mode
#define SUS 	XPS.SUS 		// Suspend Status
#define ADS 	XPS.ADS 		// Current addr mode (0-3 byte / 1-4 byte)
#define ADP 	XPS.ADP 		// Power-up addr mode
#define Busy 	XPS.BUSY  		// Erase/Write in progress
#define Sleep 	XPS.SLEEP 		// Sleep Status
#define Type 	XPS.TYPE		// type command for control
#define State 	XPS.STATE		// Current state of XPS IC

#define Setting _Setting
#define Size 	_Size
#define Memory 	_Memory
#define Page 	_Page
#define Shift 	_Shift
#define Encoded_Picture _Encoded_Picture
#define Encoded_Size _Encoded_Size
#define XPS_FLASH_SIZE 256

ENVIRONMENT_PARAM_TYPE thSensorParams;
ACCELERATION_PARAM_TYPE accParams;
CAMERA_PARAMETER_TYPE camParams;

// Private functions prototypes
void clearShadowMemory(void);

/******************************************************
 XPS Initializing Function
 Configures the XPS to receive information and
 verifies that the unit contains proper connections
 ******************************************************/
void XPS_initialize ( MEM_PTR *Data_Ptr )
{
	//need to wake up from sleep
	Command = SETTLE;
	XPS_COMMAND ( Data_Ptr );

	Command = READ_STAT_STRCT;
	XPS_COMMAND ( Data_Ptr );

	//below is disabled, we are not currently using quad spi

	//	/* If Quad-SPI mode disabled */
	//	if (!QE)
	//	{
	//		Type = 2;
	//		Command = READ_STAT_REG;
	//		XPS_COMMAND ( Data_Ptr );
	//
	//		Memory [ XPS_FLASH_SIZE + 1 ] |= 0b10;
	//		Command = CHIPID;
	//		XPS_COMMAND ( Data_Ptr );
	//
	//		Command = WRITE_ENABLE;
	//		XPS_COMMAND ( Data_Ptr );
	//		Type = 2;
	//		Command = WRITE_STAT_REG;
	//		XPS_COMMAND ( Data_Ptr );
	//
	//	}
	//	Command = READ_STAT_STRCT;
	//	XPS_COMMAND ( Data_Ptr );

	Command = QCHIPID;  //powers up?
	XPS_COMMAND ( Data_Ptr );
}
void clearShadowMemory(void)
{
	for(int chPnt = 0; chPnt < 256; chPnt++)
	{
		paramPage1[chPnt] = 0;
	}
}

bool getOTAfwAvailable(void)
{
	// KCS do we need to check isdataValid for every get function?
	bool isAvailable = false;

	if ( paramPage1 [DOWNLOAD_READY] == 1 )
	{
		isAvailable = true;
	}
	return isAvailable;
}

void setOTAfwAvailable(bool fwIsAvailable)
{
	if ( fwIsAvailable )
	{
		paramPage1 [DOWNLOAD_READY] = 1;
	}
	else
	{
		paramPage1 [DOWNLOAD_READY] = 0;
	}
}

void setOtaData(OTA_FILE_TYPE *OtaData)
{
	for(int chPnt = 0; chPnt < OtaData->fileNameLength; chPnt++)
	{
		paramPage1[OTA_FILENAME_START + chPnt] = OtaData->otaFileName[chPnt];
	}
	paramPage1[OTA_FILENAME_LEN] = OtaData->fileNameLength;
	paramPage1[OTA_NUMB_OF_FILES] = OtaData->numberOfFiles;
}

void getOtaData(OTA_FILE_TYPE *OtaData)
{
	OtaData->fileNameLength = paramPage1[OTA_FILENAME_LEN];
	OtaData->numberOfFiles = paramPage1[OTA_NUMB_OF_FILES];

	for(int chPnt = 0; chPnt < OtaData->fileNameLength; chPnt++)
	{
		OtaData->otaFileName[chPnt]= paramPage1[OTA_FILENAME_START + chPnt];
	}
}

void saveParamDataToFlash(MEM_PTR *Data_Ptr)
{
	Command = QCHIPID;
	XPS_COMMAND ( Data_Ptr );
	Page = PARAM_PAGE;
	Shift = PARAM_SHIFT;
	Size = 256;

	for(int chPnt = 0; chPnt < Size; chPnt++)
	{
		Memory[chPnt] = paramPage1[chPnt];
	}

	Command = ERASE;
	XPS_COMMAND ( Data_Ptr );

	Command = WRITE;  // make sure WRITE command erases sector
	XPS_COMMAND ( Data_Ptr );
}

void readParamDataFromFlash(MEM_PTR *Data_Ptr )
{
	Command = QCHIPID;
	XPS_COMMAND ( Data_Ptr );
	Page = PARAM_PAGE;
	Shift = PARAM_SHIFT;
	Size = 256;

	Command = READ;
	XPS_COMMAND ( Data_Ptr );

	for(int chPnt = 0; chPnt < Size; chPnt++)
	{
		paramPage1[chPnt] = Memory[chPnt];
	}

	if (Memory [ PAGEKEY ] == UPDATE_PARAM_KEY)
	{
		isDataValid = true;
	}
	else
		isDataValid = false;
}

void readDataValidFromFlash(MEM_PTR *Data_Ptr )
{
	Command = QCHIPID;
	XPS_COMMAND ( Data_Ptr );
	Page = PARAM_PAGE;
	Shift = PARAM_SHIFT;
	Size = 1;

	Command = READ;
	XPS_COMMAND ( Data_Ptr );

	for(int chPnt = 0; chPnt < Size; chPnt++)
	{
		paramPage1[chPnt] = Memory[chPnt];
	}

	if (Memory [ PAGEKEY ] == UPDATE_PARAM_KEY)
	{
		isDataValid = true;
	}
	else
		isDataValid = false;
}

bool getDataValid(void)
{
	return isDataValid;
}

// kcs change this to get all the individual modules from the shadow RAM
void populateDefaults(void)
{
	gpsParametersInit();
	dayNightInit();
}

void saveBank(MEM_PTR *Data_Ptr )
{
	Command = QCHIPID;
	XPS_COMMAND ( Data_Ptr );
	Page = PARAM_PAGE;
	Shift = PARAM_SHIFT;
	Size = 32;

	Memory [ 54 ] = (uint8_t) Data_Ptr->Flash.currentFlashBank;

	Command = WRITE;
	XPS_COMMAND ( Data_Ptr );
}

void readBank(MEM_PTR *Data_Ptr )
{
	Command = QCHIPID;
	XPS_COMMAND ( Data_Ptr );
	Page = PARAM_PAGE;
	Shift = PARAM_SHIFT;
	Size = 32;

	Command = READ;
	XPS_COMMAND ( Data_Ptr );

	if (Memory [ 0 ] == UPDATE_PARAM_KEY)
	{
		if ( Memory [54] == 0 || Memory [54] == 1)
			Data_Ptr->Flash.currentFlashBank = 1;
		else
			Data_Ptr->Flash.currentFlashBank = 2;
	}
	else
	{
		Data_Ptr->Flash.currentFlashBank = 1;
	}
}

uint8_t  getMode (HW_MODULE_TYPE hwSubModule)
{
	uint8_t mode = 0;
	switch (hwSubModule)
	{
	case HEARTBEAT_MODULE:
		mode =  paramPage1 [ HB_MODE ];
		break;
	case GPS_MODULE:
		mode =  paramPage1 [ GPS_MODE ];
		break;
	case BUZZER_MODULE:
		mode =  paramPage1 [ BZ_MODE ];
		break;
	case TEMPERATURE_MODULE:
		mode =  paramPage1 [ TEMP_MODE ];
		break;
	case HUMIDITY_MODULE:
		mode =  paramPage1 [ HUMD_MODE ];
		break;
	case CELL_MODULE:
		mode =  paramPage1 [ CELL_MODE ];
		break;
	case CAMERA_MODULE:
		mode =  paramPage1 [ CAMERA_MODE ];
		break;
	case ACCELEROMETER_MODULE:
		mode =  paramPage1 [ ACC_MODE ];
		break;
	case PIR_MODULE:
		mode =  paramPage1 [ PIR_MODE ];
		break;
	default:
		break;
	}

	return mode;
}
/******************************************************
 Gets the operational Parameters from XPS memory
 returns a boolean value if the data has been set
 ******************************************************/
bool XPS_paramRetrieve ( MEM_PTR *Data_Ptr )
{
	if (paramPage1 [ PAGEKEY ] == UPDATE_PARAM_KEY)
	{
		isDataValid = true;
		uint32_t longitudeInt = 0;
		uint32_t latitudeInt = 0;

		Setting = paramPage1 [ LOGGING_LEVEL_SETTING ];
		Data_Ptr->Mode = paramPage1 [ DEVICE_MODE ];

		PIR_PARAMETER_TYPE pirXPSData;
		PIR_FILTER_TYPE dayTimeXPSParam;
		PIR_FILTER_TYPE nightTimeXPSParam;

		dayTimeXPSParam.Motion_Confirm_Window =       paramPage1 [ DT_MOTION_CONFIRM_WINDOW ];
		dayTimeXPSParam.Motion_Threshhold =           paramPage1 [ DT_MOTION_THRES ];
		dayTimeXPSParam.Motion_Blackout =            (paramPage1 [ DT_MOTION_BLACKOUT_MSB ]<<8) + paramPage1 [ DT_MOTION_BLACKOUT_LSB ] ;
		dayTimeXPSParam.No_Motion_Detection_Window =  paramPage1 [ DT_NO_MOTION_WINDOW ];
		dayTimeXPSParam.Near_Motion_Threshhold =      paramPage1 [DT_NEAR_MOTION_THRES ];
		nightTimeXPSParam.Motion_Confirm_Window =     paramPage1 [NT_MOTION_CONFIRM_WINDOW ];
		nightTimeXPSParam.Motion_Threshhold =         paramPage1 [NT_MOTION_THRES];
		nightTimeXPSParam.Motion_Blackout =          (paramPage1 [ NT_MOTION_BLACKOUT_MSB ]<<8) + paramPage1 [ NT_MOTION_BLACKOUT_LSB ] ;
		nightTimeXPSParam.No_Motion_Detection_Window = paramPage1 [ NT_NO_MOTION_WINDOW ];
		nightTimeXPSParam.Near_Motion_Threshhold =    paramPage1 [ NT_NEAR_MOTION_THRES ];
		pirXPSData.coolDown =                        (paramPage1 [PIR_COOLDOWN_MSB ]<<8) + paramPage1 [ PIR_COOLDOWN_LSB ] ;
		pirXPSData.mode =                             paramPage1 [ PIR_MODE];
		PRINTF("Retrieved PIR Mode is %d\r\n", paramPage1 [ PIR_MODE ]);

		setPirParameters(pirXPSData);
		setFilterParameters(dayTimeXPSParam, PIR_DAY);
		setFilterParameters(nightTimeXPSParam, PIR_NIGHT);

		Data_Ptr->heartBeatData.mode =            paramPage1 [ HB_MODE ];
		Data_Ptr->heartBeatData.hbInterval =  (paramPage1 [ HB_INTERVAL_MSB ] <<8) + paramPage1 [ HB_INTERVAL_LSB ];

		PRINTF("Retrieved HB Interval MSB is %d\r\n", paramPage1 [ HB_INTERVAL_MSB ]);
		PRINTF("Retrieved HB Interval LSB is %d\r\n", paramPage1 [ HB_INTERVAL_LSB ]);
		PRINTF("HB Interval is %d\r\n", Data_Ptr->heartBeatData.hbInterval);

		GPS_PARAMETER_TYPE retrievedGpsParams;

		retrievedGpsParams.mode =               paramPage1 [ GPS_MODE ];
		retrievedGpsParams.gpsInterval =        paramPage1 [ GPS_INTERVAL ];
		//retrievedGpsParams.timeSyncEnable =     paramPage1 [ GPS_TIMESYNC_ENABLE];  // part of mode
		retrievedGpsParams.geofenceDistance =  (paramPage1 [ GPS_GEO_MSB ] << 8) + paramPage1 [ GPS_GEO_LSB ];
		retrievedGpsParams.alarmSamplePeriod =  paramPage1 [ GPS_ALARM_SAMP_PER ];
		retrievedGpsParams.alarmTime =          ((paramPage1 [ GPS_ALARM_TIME_MSB] << 8) + paramPage1 [ GPS_ALARM_TIME_LSB]);
		retrievedGpsParams.movementHysteresis = paramPage1 [ GPS_MOVE_HYSTER ];
		setGpsParameters(retrievedGpsParams);

		longitudeInt =  (paramPage1 [ LONG_INT_MSB ] << 24) +(paramPage1 [ LONG_INT_3B ] << 16) + (paramPage1 [ LONG_INT_2B ] <<  8) + paramPage1 [ LONG_INT_LSB ];
		latitudeInt  =  (paramPage1 [ LAT_INT_MSB ] << 24) +(paramPage1 [LAT_INT_3B ] << 16) + (paramPage1 [ LAT_INT_2B ] <<  8) + paramPage1 [ LAT_INT_LSB ];

		Data_Ptr->LastPosition.longitude = GPSIntToFloat(longitudeInt);
		Data_Ptr->LastPosition.latitude = GPSIntToFloat(latitudeInt);

		BUSINESS_DATA_TYPE retrievedOperatingHours;

		retrievedOperatingHours.dayTime.hours  =        paramPage1 [ BUS_DT_HOURS ];
		retrievedOperatingHours.dayTime.minutes =       paramPage1 [ BUS_DT_MIN ];
		retrievedOperatingHours.nightTime.hours =       paramPage1 [ BUS_NT_HOURS];
		retrievedOperatingHours.nightTime.minutes =     paramPage1 [ BUS_NT_MIN ];
		retrievedOperatingHours.DayNightSwitchEnabled = paramPage1 [ BUS_HOURS_ENABLE ];
		setBusinessHours(retrievedOperatingHours);

		// KCS move these into a function in the Temperature.c file
		// hold all these parameters local in the Temperature.c file
		// call setTempParams()

		thSensorParams.samplePeriod  =          paramPage1 [ ENV_SENSOR_SAMPLE_PERIOD ];
		thSensorParams.sensorWarmUp =           paramPage1 [ ENV_SENSOR_WARMUP ];
		thSensorParams.coolDownPeriod  =        paramPage1 [ ENV_SENSOR_COOLDOWN ];
		thSensorParams.chargerDisableSetting =  paramPage1 [ ENV_SENSOR_CHARG_DISABLE_SET ];
		thSensorParams.alarmTime  =             paramPage1 [ ENV_SENSOR_ALARM_TIME ];
		thSensorParams.alarmSamplePeriod  =     paramPage1 [ ENV_SENSOR_ALARM_SAMPLE ];

		thSensorParams.temperature.mode =       paramPage1 [ TEMP_MODE ];
		thSensorParams.temperature.lowSet  =    paramPage1 [ TEMP_LOW_SET ];
		thSensorParams.temperature.highSet =    paramPage1 [ TEMP_HI_SET ];
		thSensorParams.temperature.hysteresis = paramPage1 [ TEMP_HYSTER ];

		thSensorParams.humidity.mode =       paramPage1 [ HUMD_MODE ];
		thSensorParams.humidity.lowSet  =    paramPage1 [ HUMD_LOW_SET ];
		thSensorParams.humidity.highSet =    paramPage1 [ HUMD_HI_SET ];
		thSensorParams.humidity.hysteresis = paramPage1 [ HUMD_HYSTER ];

		Data_Ptr->buzzerData.startCycles = paramPage1 [ BZ_START_CYCLES ];
		Data_Ptr->buzzerData.stopCycles =  paramPage1 [ BZ_STOP_CYCLES ];
		Data_Ptr->buzzerData.mode =        paramPage1 [ BZ_MODE ];
		// KCS make name like GPS??
		accParams.mode =        paramPage1 [ ACC_MODE ];
		accParams.range =       paramPage1 [ ACC_RANGE ];
		accParams.threshold =   ((paramPage1 [ ACC_THRESH_MSB ] << 8) + paramPage1 [ ACC_THRESH_LSB ]);
		accParams.hysteresis =  ((paramPage1 [ ACC_HYSTER_MSB ] << 8) + paramPage1 [ ACC_HYSTER_LSB ]);
		accParams.mutePeriod =  ((paramPage1 [ ACC_MUTE_MSB ] << 8) + paramPage1 [ ACC_MUTE_LSB ]);
		setAccelParameters(accParams);


		camParams.mode = paramPage1[CAMERA_MODE];
		camParams.pictureInterval = paramPage1[CAMERA_PIC_INTERVAL];
		camParams.picturesPerEvent = paramPage1[CAMERA_PIC_PER_EVENT];
		camParams.warmUp = paramPage1[CAMERA_WARMUP];
		camParams.samplePicNumber = paramPage1[CAMERA_PIC_SAMPLES_PER_DAY];
		camParams.samplePictureTime.hours = paramPage1[CAMERA_PIC_SAMPLE_HOUR];
		camParams.samplePictureTime.minutes = paramPage1[CAMERA_PIC_SAMPLE_MIN];
		setCameraParameters(camParams);

		// call setOtaParams

		Data_Ptr->Flash.fwDownloadRdy =  paramPage1 [ DOWNLOAD_READY ]; //fwDownloadRdy

		//cell.mode =     paramPage1 [ CELL_MODE ];
		//cell.warmUp =     paramPage1 [ CELL_WARMUP ];
		//Data_Ptr->heartBeatData.cameraWarmUp =    paramPage1 [ HB_CAMERA_WARMUP ];

	}
	return isDataValid;
}

/******************************************************
 XPS Param Function
 Write Current Parameters to memory
 ******************************************************/
void XPS_paramStore ( MEM_PTR *Data_Ptr )
{
	uint32_t longitudeInt = GPSFloatToInt(Data_Ptr->LastPosition.longitude);
	uint32_t latitudeInt = GPSFloatToInt(Data_Ptr->LastPosition.latitude);

	paramPage1 [ PAGEKEY ]               = UPDATE_PARAM_KEY;
	paramPage1 [ LOGGING_LEVEL_SETTING ] = Setting;
	paramPage1 [DEVICE_MODE]             = Data_Ptr->Mode;

	PIR_PARAMETER_TYPE pirXPSData;
	PIR_FILTER_TYPE dayTimeXPSParam;
	PIR_FILTER_TYPE nightTimeXPSParam;
	getPirParameters(&pirXPSData);
	getFilterParameters(&dayTimeXPSParam, PIR_DAY);
	getFilterParameters(&nightTimeXPSParam, PIR_NIGHT);

	paramPage1 [ DT_MOTION_CONFIRM_WINDOW ] = dayTimeXPSParam.Motion_Confirm_Window;
	paramPage1 [ DT_MOTION_THRES ]       = dayTimeXPSParam.Motion_Threshhold;
	paramPage1 [ DT_MOTION_BLACKOUT_MSB ]    = dayTimeXPSParam.Motion_Blackout >> 8; //MSB
	paramPage1 [ DT_MOTION_BLACKOUT_LSB ]    = dayTimeXPSParam.Motion_Blackout;
	paramPage1 [ DT_NO_MOTION_WINDOW ]   = dayTimeXPSParam.No_Motion_Detection_Window;
	paramPage1 [ DT_NEAR_MOTION_THRES ]  = dayTimeXPSParam.Near_Motion_Threshhold;
	paramPage1 [ NT_MOTION_CONFIRM_WINDOW ] = nightTimeXPSParam.Motion_Confirm_Window;
	paramPage1 [ NT_MOTION_THRES ]       = nightTimeXPSParam.Motion_Threshhold;
	paramPage1 [ NT_MOTION_BLACKOUT_MSB ]    = nightTimeXPSParam.Motion_Blackout >> 8; //MSB
	paramPage1 [ NT_MOTION_BLACKOUT_LSB ]    = nightTimeXPSParam.Motion_Blackout;
	paramPage1 [ NT_NO_MOTION_WINDOW ]   = nightTimeXPSParam.No_Motion_Detection_Window;
	paramPage1 [ NT_NEAR_MOTION_THRES ]  = nightTimeXPSParam.Near_Motion_Threshhold;
	paramPage1 [ PIR_COOLDOWN_MSB ]      = pirXPSData.coolDown >> 8;   //MSB
	paramPage1 [ PIR_COOLDOWN_LSB ]      = pirXPSData.coolDown;        //LSB
	paramPage1 [ PIR_MODE]               = pirXPSData.mode;

	PRINTF("Setting Shadow Register PIR Mode to %d\r\n", paramPage1 [ PIR_MODE ]);

	paramPage1 [ HB_MODE ]             = Data_Ptr->heartBeatData.mode;
	paramPage1 [ HB_INTERVAL_MSB ]     = Data_Ptr->heartBeatData.hbInterval >> 8;  //MSB
	paramPage1 [ HB_INTERVAL_LSB ]     = Data_Ptr->heartBeatData.hbInterval;       //LSB

	PRINTF("Setting Shadow Register HB Interval MSB to %d\r\n", paramPage1 [ HB_INTERVAL_MSB ]);
	PRINTF("Setting Shadow Register HB Interval LSB to %d\r\n", paramPage1 [ HB_INTERVAL_LSB ]);

	GPS_PARAMETER_TYPE currentGpsParams;
	getGpsParameters(&currentGpsParams);

	paramPage1 [ GPS_MODE ]            = currentGpsParams.mode;
	paramPage1 [ GPS_INTERVAL ]        = currentGpsParams.gpsInterval;
	//paramPage1 [ GPS_TIMESYNC_ENABLE ] = currentGpsParams.timeSyncEnable;
	paramPage1 [ GPS_GEO_MSB ]         = currentGpsParams.geofenceDistance>> 8;
	paramPage1 [ GPS_GEO_LSB ]         = currentGpsParams.geofenceDistance;
	paramPage1 [ GPS_ALARM_SAMP_PER ]  = currentGpsParams.alarmSamplePeriod;
	paramPage1 [ GPS_ALARM_TIME_MSB ]  = currentGpsParams.alarmTime >> 8;
	paramPage1 [ GPS_ALARM_TIME_LSB ]  = currentGpsParams.alarmTime;
	paramPage1 [ GPS_MOVE_HYSTER ]     = currentGpsParams.movementHysteresis;
	paramPage1 [ LONG_INT_MSB ]        = longitudeInt >> 24;
	paramPage1 [ LONG_INT_3B ]         = longitudeInt >> 16;
	paramPage1 [ LONG_INT_2B ]         = longitudeInt >> 8;
	paramPage1 [ LONG_INT_LSB ]        = longitudeInt;
	paramPage1 [ LAT_INT_MSB ]         = latitudeInt >> 24;
	paramPage1 [ LAT_INT_3B ]          = latitudeInt >> 16;
	paramPage1 [ LAT_INT_2B ]          = latitudeInt >> 8;
	paramPage1 [ LAT_INT_LSB ]         = latitudeInt;

	BUSINESS_DATA_TYPE currentOperatingHours;
	getBusinessHours(&currentOperatingHours);

	paramPage1 [ BUS_DT_HOURS ]     = currentOperatingHours.dayTime.hours;
	paramPage1 [ BUS_DT_MIN ]       = currentOperatingHours.dayTime.minutes;
	paramPage1 [ BUS_NT_HOURS ]     = currentOperatingHours.nightTime.hours;
	paramPage1 [ BUS_NT_MIN ]       = currentOperatingHours.nightTime.minutes;
	paramPage1 [ BUS_HOURS_ENABLE ] = currentOperatingHours.DayNightSwitchEnabled;

	// KCS call getTempParameters
	paramPage1 [ ENV_SENSOR_SAMPLE_PERIOD ] = thSensorParams.samplePeriod;
	paramPage1 [ ENV_SENSOR_WARMUP ] =        thSensorParams.sensorWarmUp;
	paramPage1 [ ENV_SENSOR_COOLDOWN ] =      thSensorParams.coolDownPeriod;
	paramPage1 [ ENV_SENSOR_CHARG_DISABLE_SET ] = thSensorParams.chargerDisableSetting;
	paramPage1 [ ENV_SENSOR_ALARM_TIME ] =    thSensorParams.alarmTime;
	paramPage1 [ ENV_SENSOR_ALARM_SAMPLE ] =  thSensorParams.alarmSamplePeriod;

	paramPage1 [ TEMP_MODE ] =    thSensorParams.temperature.mode;
	paramPage1 [ TEMP_LOW_SET ] = thSensorParams.temperature.lowSet;
	paramPage1 [ TEMP_HI_SET ] =  thSensorParams.temperature.highSet;
	paramPage1 [ TEMP_HYSTER ] =  thSensorParams.temperature.hysteresis;

	paramPage1 [ HUMD_MODE ] =    thSensorParams.humidity.mode;
	paramPage1 [ HUMD_LOW_SET ] = thSensorParams.humidity.lowSet;
	paramPage1 [ HUMD_HI_SET ] =  thSensorParams.humidity.highSet;
	paramPage1 [ HUMD_HYSTER ] =  thSensorParams.humidity.hysteresis;

	paramPage1 [ BZ_START_CYCLES ] = Data_Ptr->buzzerData.startCycles;
	paramPage1 [ BZ_STOP_CYCLES ]  = Data_Ptr->buzzerData.stopCycles;
	paramPage1 [ BZ_MODE ]         = Data_Ptr->buzzerData.mode;

	// KCS call getACCELParameters
	getAccelParameters(&accParams);

	paramPage1 [ ACC_MODE ] =   accParams.mode;
	paramPage1 [ ACC_RANGE ]  = accParams.range;
	paramPage1 [ ACC_THRESH_MSB ] = accParams.threshold >> 8;
	paramPage1 [ ACC_THRESH_LSB ] = accParams.threshold;
	paramPage1 [ ACC_HYSTER_MSB ] = accParams.hysteresis >> 8;
	paramPage1 [ ACC_HYSTER_LSB ] = accParams.hysteresis;
	paramPage1 [ ACC_MUTE_MSB ] = accParams.mutePeriod >> 8;
	paramPage1 [ ACC_MUTE_LSB ] = accParams.mutePeriod;


	paramPage1 [ DOWNLOAD_READY ] = Data_Ptr->Flash.fwDownloadRdy;

	getCameraParameters(&camParams);

	paramPage1[CAMERA_MODE]                = camParams.mode;
	paramPage1[CAMERA_PIC_INTERVAL]        = camParams.pictureInterval;
	paramPage1[CAMERA_PIC_PER_EVENT]       = camParams.picturesPerEvent;
	paramPage1[CAMERA_WARMUP]              = camParams.warmUp;
	paramPage1[CAMERA_PIC_SAMPLES_PER_DAY] = camParams.samplePicNumber;
	paramPage1[CAMERA_PIC_SAMPLE_HOUR]     = camParams.samplePictureTime.hours;
	paramPage1[CAMERA_PIC_SAMPLE_MIN]      = camParams.samplePictureTime.minutes;
	//KCS call get OTA parameters

	//	paramPage1 [ CELL_MODE ] =   cell.mode;
	//	paramPage1 [ CELL_WARMUP ] = cell.warmUp;
	//paramPage1 [ HB_CAMERA_WARMUP ]    = Data_Ptr->heartBeatData.cameraWarmUp;

}

void XPS_Picture_Save ( MEM_PTR *Data_Ptr )
{
//	Page = PIC_PAGE;
//	Shift = PIC_SHIFT;
//	buf = 0;
//
//	while (Encoded_Picture [ buf ])
//	{
//		for (Size = 0; Size != MEM_PAGE_SIZE; Size++)
//			Memory [ Size ] = Encoded_Picture [ buf++ ];
//
//		Command = WRITE;
//		XPS_COMMAND ( Data_Ptr );
//
//		Page++;
//	}
//	__NOP();
}

void XPS_Image_Load ( MEM_PTR *Data_Ptr )
{
//	// kcs temp
//	uint32_t pictureSizeLimit = 100000;
//	for (Data_Ptr->Flash.Total_Size = 0; Data_Ptr->Flash.Total_Size != pictureSizeLimit; Data_Ptr->Flash.Total_Size++)
//	{
//		Page = FLASH_PAGE + (Data_Ptr->Flash.Total_Size);
//		Shift = FLASH_SHIFT;
//		Size = MEM_PAGE_SIZE;
//
//		Command = READ;
//		XPS_COMMAND ( Data_Ptr );
//
//	}
}

/******************************************************
 XPS Control Function
 Depending on type the XPS can be controlled
 while not all commands require address values
 the system requires for the function but
 will leave the value floating

 Type:
 SETTLE 	- simple IC read for test
 READ   	- read memory from address
 WRITE		- write to address from buffer
 ERASE		- erase a 4kb page
 ERASE32KB	- erase a 32kb page
 ERASE64KB	- erase a 64kb page
 SLEEP		- sets XPS to sleep mode
 WAKE		- wakes up XPS from sleep
 PREP_RESET	- prepares XPS to reset system
 TRIG_RESET	- resets XPS settings and memory
 RESET REQUIRES PRIOR PRE_RESET
 ******************************************************/
void XPS_COMMAND ( MEM_PTR *Data_Ptr )
{
	Refresh_Watchdog;

	XPS_Verify_Memory ( Data_Ptr );
	State = XPS_OK;
	if (State != XPS_OK && Command != SETTLE)
	{
		return;
	}

	XPS_IsBusy ( Data_Ptr );

	// Configure Instruction Info
	OSPI_Setting.OperationType = HAL_OSPI_OPTYPE_COMMON_CFG;
	OSPI_Setting.InstructionMode = HAL_OSPI_INSTRUCTION_1_LINE;
	OSPI_Setting.InstructionSize = HAL_OSPI_INSTRUCTION_8_BITS;
	// Disables Unused Settings
	OSPI_Setting.AlternateBytesMode = HAL_OSPI_ALTERNATE_BYTES_NONE;
	OSPI_Setting.AlternateBytesSize = HAL_OSPI_ALTERNATE_BYTES_NONE;
	OSPI_Setting.AlternateBytes = HAL_OSPI_ALTERNATE_BYTES_NONE;
	OSPI_Setting.DataDtrMode = HAL_OSPI_DATA_DTR_DISABLE;
	OSPI_Setting.AddressDtrMode = HAL_OSPI_ADDRESS_DTR_DISABLE;
	OSPI_Setting.InstructionDtrMode = HAL_OSPI_INSTRUCTION_DTR_DISABLE;
	OSPI_Setting.SIOOMode = HAL_OSPI_SIOO_INST_EVERY_CMD;

	switch (Command)
	{
	case SETTLE:
		State = XPS_OK;
		Command = CHIPID;
		XPS_COMMAND ( Data_Ptr );
		return;
	case READ:
		//			if (QE)
		//			{
		OSPI_Setting.Instruction = XPS_ReadPage_DATA;	 // Command
		// Configure Address Info
		OSPI_Setting.AddressMode = HAL_OSPI_ADDRESS_1_LINE;
		OSPI_Setting.AddressSize = HAL_OSPI_ADDRESS_24_BITS;
		OSPI_Setting.Address = ((Page * MEM_PAGE_SIZE + Shift));
		//OSPI_Setting.Address = 0xF;

		// Configure Packet/Message Info
		OSPI_Setting.DataMode = HAL_OSPI_DATA_1_LINE;
		OSPI_Setting.DummyCycles = 0;
		OSPI_Setting.NbData = Size;
		if (OSPItest ( &hospi1 , &OSPI_Setting , HAL_OSPI_TIMEOUT_DEFAULT_VALUE ) != HAL_OK)
		{
			State = XPS_READ_CONFIG_ERR;

#ifdef Log_Level_1
			Write_Log ( LOG_ERROR );
			Log_End ( LOG_STATE_RESULT , State );
#endif

			return;
		}
		if (HAL_OSPI_Receive ( &hospi1 , Memory , HAL_OSPI_TIMEOUT_DEFAULT_VALUE ) != HAL_OK)
		{
			State = XPS_READ_RECEIVE_ERR;

#ifdef Log_Level_1
			Write_Log ( LOG_ERROR );
			Log_End ( LOG_STATE_RESULT , State );
#endif

			return;
		}

#ifdef Log_Level_2
		Log_End ( LOG_STATE_RESULT , State );
#endif

		return;
		//			}
		//			State = XPS_QUAD_ERR;
		//
		//#ifdef Log_Level_2
		//		Log_End ( LOG_STATE_RESULT , State );
		//#endif

		return;
	case WRITE:
		//			if (QE)
		//			{
		//				Command = ERASE;
		//				XPS_COMMAND ( Data_Ptr );

		Command = WRITE_ENABLE;
		XPS_COMMAND ( Data_Ptr );

		OSPI_Setting.Instruction = XPS_PAGE_PROGRAM;
		OSPI_Setting.InstructionMode = 	HAL_OSPI_INSTRUCTION_1_LINE;
		OSPI_Setting.InstructionSize = HAL_OSPI_INSTRUCTION_8_BITS;

		// Configure Address Info
		OSPI_Setting.AddressMode = HAL_OSPI_ADDRESS_1_LINE;
		OSPI_Setting.AddressSize = HAL_OSPI_ADDRESS_24_BITS;
		OSPI_Setting.Address = ((Page * MEM_PAGE_SIZE + Shift));
		//OSPI_Setting.Address = 0xF;

		// Configure Packet/Message Info
		OSPI_Setting.DataDtrMode = HAL_OSPI_DATA_DTR_DISABLE;
		OSPI_Setting.DataMode = HAL_OSPI_DATA_1_LINE;
		OSPI_Setting.DummyCycles = 0;
		OSPI_Setting.NbData = Size;

		if (OSPItest ( &hospi1 , &OSPI_Setting , HAL_OSPI_TIMEOUT_DEFAULT_VALUE ) != HAL_OK)
		{
			State = XPS_WRITE_CONFIG_ERR;

#ifdef Log_Level_1
			Write_Log ( LOG_ERROR );
			Log_End ( LOG_STATE_RESULT , State );
#endif

			return;
		}
		OSPItest ( &hospi1 , &OSPI_Setting , HAL_OSPI_TIMEOUT_DEFAULT_VALUE );

		if (HAL_OSPI_Transmit ( &hospi1 , (Memory) ,  HAL_OSPI_TIMEOUT_DEFAULT_VALUE ) != HAL_OK)
		{
			State = XPS_WRITE_SEND_ERR;

#ifdef Log_Level_1
			Write_Log ( LOG_ERROR );
			Log_End ( LOG_STATE_RESULT , State );
#endif
			return;
		}
		WEL = 0;

#ifdef Log_Level_2
		Log_End ( LOG_STATE_RESULT , State );
#endif
		return;
		//			}
		//
		//#ifdef Log_Level_2
		//		Log_End ( LOG_STATE_RESULT , State );
		//#endif

		//			State = XPS_QUAD_ERR;
		return;
	case ERASE_FULL:
		Command = WRITE_ENABLE;
		XPS_COMMAND ( Data_Ptr );

		// Based on size the instruction will change
		OSPI_Setting.Instruction = XPS_FULL_ERASE;
		// Configure Address Info
		OSPI_Setting.AddressMode = HAL_OSPI_ADDRESS_NONE;
		OSPI_Setting.AddressSize = HAL_OSPI_ADDRESS_NONE;  // change to 1 line
		//			OSPI_Setting.Address = ((Page * MEM_PAGE_SIZE) + Shift);
		OSPI_Setting.Address = 0x0U;
		// Configure Packet/Message Info
		OSPI_Setting.DataMode = HAL_OSPI_DATA_NONE;
		OSPI_Setting.DummyCycles = 0;
		OSPI_Setting.NbData = 0;
		if (OSPItest ( &hospi1 , &OSPI_Setting , HAL_OSPI_TIMEOUT_DEFAULT_VALUE ) != HAL_OK)
		{
			State = XPS_Erase_CONFIG_ERR;

#ifdef Log_Level_1
			Write_Log ( LOG_ERROR );
			Log_End ( LOG_STATE_RESULT , State );
#endif

			return;
		}

#ifdef Log_Level_2
		Log_End ( LOG_STATE_RESULT , State );
#endif

		WEL = 0;
		return;
	case ERASE:
		Command = WRITE_ENABLE;
		XPS_COMMAND ( Data_Ptr );
		// KCS need to check for page overrun and other priorities
		// Based on size the instruction will change
		OSPI_Setting.Instruction = XPS_SECTOR_ERASE;
		// Configure Address Info
		OSPI_Setting.AddressSize = HAL_OSPI_ADDRESS_24_BITS;  // change this setting?? for bigger memory
		OSPI_Setting.AddressMode = HAL_OSPI_ADDRESS_1_LINE;	// change to 1 line
		OSPI_Setting.Address = ((Page * MEM_PAGE_SIZE) + Shift);
		// Configure Packet/Message Info
		OSPI_Setting.DataMode = HAL_OSPI_DATA_NONE;
		OSPI_Setting.DummyCycles = 0;
		OSPI_Setting.NbData = 0;
		if (HAL_OSPI_Command ( &hospi1 , &OSPI_Setting , HAL_OSPI_TIMEOUT_DEFAULT_VALUE ) != HAL_OK)
		{
			State = XPS_Erase_CONFIG_ERR;

#ifdef Log_Level_1
			Write_Log ( LOG_ERROR );
			Log_End ( LOG_STATE_RESULT , State );
#endif

			return;
		}


	case ERASE_32KB:
		Command = WRITE_ENABLE;
		XPS_COMMAND ( Data_Ptr );

		// Based on size the instruction will change
		OSPI_Setting.Instruction = XPS_32KB_BLOCK_ERASE;
		// Configure Address Info
		OSPI_Setting.AddressSize = HAL_OSPI_ADDRESS_24_BITS;
		OSPI_Setting.AddressMode = HAL_OSPI_ADDRESS_1_LINE;	 // change to 1 line
		OSPI_Setting.Address = ((Page * MEM_PAGE_SIZE) + Shift);
		// Configure Packet/Message Info
		OSPI_Setting.DataMode = HAL_OSPI_DATA_NONE;
		OSPI_Setting.DummyCycles = 0;
		OSPI_Setting.NbData = 0;
		if (OSPItest ( &hospi1 , &OSPI_Setting , HAL_OSPI_TIMEOUT_DEFAULT_VALUE ) != HAL_OK)
		{
			State = XPS_Erase_CONFIG_ERR;

#ifdef Log_Level_1
			Write_Log ( LOG_ERROR );
			Log_End ( LOG_STATE_RESULT , State );
#endif

			return;
		}
		WEL = 0;

#ifdef Log_Level_2
		Log_End ( LOG_STATE_RESULT , State );
#endif

		return;
	case ERASE_64KB:
		Command = WRITE_ENABLE;
		XPS_COMMAND ( Data_Ptr );

		// Based on size the instruction will change
		OSPI_Setting.Instruction = XPS_64KB_BLOCK_ERASE;
		// Configure Address Info
		OSPI_Setting.AddressSize = HAL_OSPI_ADDRESS_24_BITS;
		OSPI_Setting.AddressMode = HAL_OSPI_ADDRESS_1_LINE;  // change to 1 line
		OSPI_Setting.Address = ((Page * MEM_PAGE_SIZE) + Shift);
		// Configure Packet/Message Info
		OSPI_Setting.DataMode = HAL_OSPI_DATA_NONE;
		OSPI_Setting.DummyCycles = 0;
		OSPI_Setting.NbData = 0;
		if (OSPItest ( &hospi1 , &OSPI_Setting , HAL_OSPI_TIMEOUT_DEFAULT_VALUE ) != HAL_OK)
		{
			State = XPS_Erase_CONFIG_ERR;

#ifdef Log_Level_1
			Write_Log ( LOG_ERROR );
			Log_End ( LOG_STATE_RESULT , State );
#endif

			return;
		}
		WEL = 0;
		return;
	case SLEEP:

		OSPI_Setting.Instruction = XPS_POWERDOWN;
		// Configure Address Info
		OSPI_Setting.AddressMode = HAL_OSPI_ADDRESS_NONE;
		OSPI_Setting.AddressSize = HAL_OSPI_ADDRESS_NONE;
		OSPI_Setting.Address = 0x0U;
		// Configure Packet/Message Info
		OSPI_Setting.DummyCycles = 0;
		OSPI_Setting.DataMode = HAL_OSPI_DATA_NONE;
		OSPI_Setting.NbData = 0;

		if (OSPItest ( &hospi1 , &OSPI_Setting , HAL_OSPI_TIMEOUT_DEFAULT_VALUE ) != HAL_OK)
		{
			State = XPS_SLEEP_CONFIG_ERR;

#ifdef Log_Level_1
			Write_Log ( LOG_ERROR );
			Log_End ( LOG_STATE_RESULT , State );
#endif

			return;
		}
		HAL_Delay ( 1 ); // Give a little time to sleep
		Sleep = 1;

#ifdef Log_Level_2
		Log_End ( LOG_STATE_RESULT , State );
#endif
		HAL_Delay(8);
		HAL_GPIO_WritePin ( GPIOA , GPIO_PIN_4 , GPIO_PIN_SET );
		HAL_Delay(8);

		return;
	case WAKE:
		Command = CHIPID;
		XPS_COMMAND ( Data_Ptr );
		Sleep = 0;
		return;
	case READ_STAT_REG:
		// Based on register instruction will change
		switch (Type)
		{
		case 1:
			OSPI_Setting.Instruction = XPS_READ_SR1;
			break;
		case 2:
			OSPI_Setting.Instruction = XPS_READ_SR2;
			break;
		case 3:
			OSPI_Setting.Instruction = XPS_READ_SR3;
			break;
		default:
			State = XPS_PARAM_ERR;
			return;
		}
		// Configure Address Info
		//			OSPI_Setting.AddressMode = HAL_OSPI_ADDRESS_NONE;
		//			OSPI_Setting.AddressSize = HAL_OSPI_ADDRESS_NONE;
		//			OSPI_Setting.Address = 0x0U;
		//			// Configure Packet/Message Info
		//			OSPI_Setting.DummyCycles = 0;
		//			OSPI_Setting.DataMode = HAL_OSPI_DATA_1_LINE;
		//			OSPI_Setting.NbData = 1;

		// Configure Address Info
		OSPI_Setting.AddressMode = HAL_OSPI_ADDRESS_NONE;
		OSPI_Setting.AddressSize = HAL_OSPI_ADDRESS_NONE;
		OSPI_Setting.Address = 0x0U;
		// Configure Packet/Message Info
		OSPI_Setting.DummyCycles = 0;
		OSPI_Setting.DataMode = HAL_OSPI_DATA_NONE;
		OSPI_Setting.NbData = 0;

		if (OSPItest ( &hospi1 , &OSPI_Setting , HAL_OSPI_TIMEOUT_DEFAULT_VALUE ) != HAL_OK)
		{
			State = XPS_STATE_CONFIG_ERR;

#ifdef Log_Level_1
			Write_Log ( LOG_ERROR );
			Log_End ( LOG_STATE_RESULT , State );
#endif
			//
			//				return;
			//			}
			//		if (Type == 3)
			//		{	setCSRDY(true);
			if (HAL_OSPI_Receive ( &hospi1 , &(Memory [ XPS_FLASH_SIZE + 1 ]) ,
					HAL_OSPI_TIMEOUT_DEFAULT_VALUE ) != HAL_OK)
			{
				State = XPS_STATE_RECEIVE_ERR;

#ifdef Log_Level_1
				Write_Log ( LOG_ERROR );
				Log_End ( LOG_STATE_RESULT , State );
#endif

				return;
			}
			//		}

#ifdef Log_Level_2
			Log_Insert ( LOG_STATE_RESULT , State );
			Log_End ( LOG_TYPE_ID , Type );
#endif

			return;
		case WRITE_STAT_REG:
			// Based on register the instruction will change
			switch (Type)
			{
			case 1:
				OSPI_Setting.Instruction = XPS_WRITE_SR1;
				break;
			case 2:
				OSPI_Setting.Instruction = XPS_WRITE_SR2;
				break;
			case 3:
				OSPI_Setting.Instruction = XPS_WRITE_SR3;
				break;
			default:
				State = XPS_PARAM_ERR;
				return;
			}
			// Configure Address Info
			OSPI_Setting.AddressMode = HAL_OSPI_ADDRESS_NONE;
			OSPI_Setting.AddressSize = HAL_OSPI_ADDRESS_NONE;
			OSPI_Setting.Address = 0x0U;
			// Configure Packet/Message Info
			OSPI_Setting.DummyCycles = 0;
			OSPI_Setting.DataMode = HAL_OSPI_DATA_1_LINE;
			OSPI_Setting.NbData = 1;

			if (OSPItest ( &hospi1 , &OSPI_Setting , HAL_OSPI_TIMEOUT_DEFAULT_VALUE ) != HAL_OK)
			{
				State = XPS_SPI_CONFIG_ERR;

#ifdef Log_Level_1
				Write_Log ( LOG_ERROR );
				Log_End ( LOG_STATE_RESULT , State );
#endif

				return;
			}
			if (HAL_OSPI_Transmit ( &hospi1 , &(Memory [ XPS_FLASH_SIZE + 1 ]) ,
					HAL_OSPI_TIMEOUT_DEFAULT_VALUE ) != HAL_OK)
			{
				State = XPS_SPI_SEND_ERR;

#ifdef Log_Level_1
				Write_Log ( LOG_ERROR );
				Log_End ( LOG_STATE_RESULT , State );
#endif

				return;
			}

			WEL = 0;

#ifdef Log_Level_2
			Log_End ( LOG_STATE_RESULT , State );
#endif

			return;
			case READ_STAT_STRCT:
				//		setCSRDY(false);
				// first portion
				Type = 1;
				Command = READ_STAT_REG;
				XPS_COMMAND ( Data_Ptr );
				Busy = (Memory [ XPS_FLASH_SIZE + 1 ]) & 0b1;
				WEL = ((Memory [ XPS_FLASH_SIZE + 1 ]) >> 1) & 0b1;

				// second portion
				Type = 2;
				Command = READ_STAT_REG;
				XPS_COMMAND ( Data_Ptr );
				QE = ((Memory [ XPS_FLASH_SIZE + 1 ]) >> 1) & 0b1;
				SUS = ((Memory [ XPS_FLASH_SIZE + 1 ]) >> 7) & 0b1;

				// third portion
				Type = 3;
				Command = READ_STAT_REG;
				XPS_COMMAND ( Data_Ptr );
				ADS = (Memory [ XPS_FLASH_SIZE + 1 ]) & 0b1;
				ADP = ((Memory [ XPS_FLASH_SIZE + 1 ]) >> 1) & 0b1;
				return;
			case WRITE_ENABLE:
				if (WEL != 1)
				{
					OSPI_Setting.Instruction = XPS_WRITE_ENABLE;
					// Configure Address Info
					OSPI_Setting.AddressMode = HAL_OSPI_ADDRESS_NONE;
					OSPI_Setting.AddressSize = HAL_OSPI_ADDRESS_NONE;
					OSPI_Setting.Address = 0x0U;
					// Configure Packet/Message Info
					OSPI_Setting.DummyCycles = 0;
					OSPI_Setting.DataMode = HAL_OSPI_DATA_NONE;
					OSPI_Setting.NbData = 0;

					if (OSPItest ( &hospi1 , &OSPI_Setting , HAL_OSPI_TIMEOUT_DEFAULT_VALUE ) != HAL_OK)
					{
						State = XPS_SPI_WEL_ERR;

#ifdef Log_Level_1
						Write_Log ( LOG_ERROR );
						Log_End ( LOG_STATE_RESULT , State );
#endif
						return;
					}

					HAL_Delay ( 1 ); // Give a little time to sleep
					WEL = 1;

#ifdef Log_Level_2
					Log_End ( LOG_STATE_RESULT , State );
#endif

				}

#ifdef Log_Level_2
				Log_End ( LOG_STATE_RESULT , State );
#endif
				return;
			case QCHIPID:
				OSPI_Setting.Instruction = XPS_POWERUP;	 // Command
				// Configure Address Info
				//			OSPI_Setting.AddressMode = HAL_OSPI_ADDRESS_4_LINES;
				//			OSPI_Setting.AddressSize = HAL_OSPI_ADDRESS_24_BITS;
				//			OSPI_Setting.Address = 0x000000U;
				//			// Configure Packet/Message Info
				//			OSPI_Setting.DummyCycles = 6;
				//			OSPI_Setting.DataMode = HAL_OSPI_DATA_4_LINES;
				//			OSPI_Setting.NbData = 1;
				// Configure Address Info
				OSPI_Setting.AddressMode = HAL_OSPI_ADDRESS_1_LINE;
				OSPI_Setting.AddressSize = HAL_OSPI_ADDRESS_24_BITS;
				OSPI_Setting.Address = 0x000000U; //GAV changed address
				// Configure Packet/Message Info
				OSPI_Setting.DummyCycles = 0;
				OSPI_Setting.DataMode = HAL_OSPI_DATA_1_LINE;
				OSPI_Setting.NbData = 1;

				if (OSPItest ( &hospi1 , &OSPI_Setting , HAL_OSPI_TIMEOUT_DEFAULT_VALUE ) != HAL_OK)
				{
					State = XPS_QSPI_CONFIG_ERR;

#ifdef Log_Level_1
					Write_Log ( LOG_ERROR );
					Log_End ( LOG_STATE_RESULT , State );
#endif

					return;
				}
				if (HAL_OSPI_Receive ( &hospi1 , Memory , HAL_OSPI_TIMEOUT_DEFAULT_VALUE ) != HAL_OK)
				{
					State = XPS_QSPI_RECEIVE_ERR;

#ifdef Log_Level_1
					Write_Log ( LOG_ERROR );
					Log_End ( LOG_STATE_RESULT , State );
#endif
					return;
				}

#ifdef Log_Level_2
				Log_End ( LOG_STATE_RESULT , State );
#endif
				return;
			case CHIPID:
				OSPI_Setting.Instruction = XPS_FULLID;	 // Command
				// Configure Address Info
				OSPI_Setting.AddressMode = HAL_OSPI_ADDRESS_1_LINE;
				OSPI_Setting.AddressSize = HAL_OSPI_ADDRESS_24_BITS;
				OSPI_Setting.Address = 0x000000U; //GAV changed address
				// Configure Packet/Message Info
				OSPI_Setting.DummyCycles = 0;
				OSPI_Setting.DataMode = HAL_OSPI_DATA_1_LINE;
				OSPI_Setting.NbData = 1;

				if (OSPItest ( &hospi1 , &OSPI_Setting , HAL_OSPI_TIMEOUT_DEFAULT_VALUE ) != HAL_OK)
				{
					State = XPS_SPI_CONFIG_ERR;

#ifdef Log_Level_1
					Write_Log ( LOG_ERROR );
					Log_End ( LOG_STATE_RESULT , State );
#endif

					return;
				}
				if (HAL_OSPI_Receive ( &hospi1 , &(Memory [ XPS_FLASH_SIZE ]) , HAL_OSPI_TIMEOUT_DEFAULT_VALUE ) != HAL_OK)
				{
					State = XPS_SPI_RECEIVE_ERR;

#ifdef Log_Level_1
					Write_Log ( LOG_ERROR );
					Log_End ( LOG_STATE_RESULT , State );
#endif

					return;
				}
				State = XPS_OK;

#ifdef Log_Level_2
				Log_End ( LOG_STATE_RESULT , State );
#endif

				return;
			case PREP_RESET:
				OSPI_Setting.Instruction = XPS_ENABLE_RST;	 // Command
				// Configure Address Info
				OSPI_Setting.AddressMode = HAL_OSPI_ADDRESS_NONE;
				OSPI_Setting.AddressSize = HAL_OSPI_ADDRESS_NONE;
				OSPI_Setting.Address = 0x0U;
				// Configure Packet/Message Info
				OSPI_Setting.DataMode = HAL_OSPI_DATA_NONE;
				OSPI_Setting.DummyCycles = 0;
				OSPI_Setting.NbData = 0;

				if (OSPItest ( &hospi1 , &OSPI_Setting , HAL_OSPI_TIMEOUT_DEFAULT_VALUE ) != HAL_OK)
				{
					State = XPS_RSTTGL_CONFIG_ERR;

#ifdef Log_Level_1
					Write_Log ( LOG_ERROR );
					Log_End ( LOG_STATE_RESULT , State );
#endif

					return;
				}

#ifdef Log_Level_2
				Log_End ( LOG_STATE_RESULT , State );
#endif

				HAL_Delay ( 1 ); // Give a little time to sleep(Data_Ptr, XPS);
				return;
			case TRIG_RESET:
				OSPI_Setting.Instruction = XPS_RESET;	 // Command
				// Configure Address Info
				OSPI_Setting.AddressMode = HAL_OSPI_ADDRESS_NONE;
				OSPI_Setting.AddressSize = HAL_OSPI_ADDRESS_NONE;
				OSPI_Setting.Address = 0x0U;
				// Configure Packet/Message Info
				OSPI_Setting.DataMode = HAL_OSPI_DATA_NONE;
				OSPI_Setting.DummyCycles = 0;
				OSPI_Setting.NbData = 0;

				if (OSPItest ( &hospi1 , &OSPI_Setting , HAL_OSPI_TIMEOUT_DEFAULT_VALUE ) != HAL_OK)
				{
					State = XPS_RESET_CONFIG_ERR;

#ifdef Log_Level_1
					Write_Log ( LOG_ERROR );
					Log_End ( LOG_STATE_RESULT , State );
#endif

					return;
				}
				HAL_Delay ( 1 ); // Give a little time to sleep
				State = XPS_RST;

#ifdef Log_Level_2
				Log_End ( LOG_STATE_RESULT , State );
#endif

				return;
			default:
				State = XPS_TYPE_PARAM_ERR;

#ifdef Log_Level_1
				Log_End ( LOG_STATE_RESULT , 250 );
#endif
				break;
		}
	}  // End of "Command" Switch cases
}

/******************************************************
 XPS Check State Function
 The function checks to see if the XPS is busy with
 another command and returns the state of the device
 ******************************************************/
void XPS_IsBusy ( MEM_PTR *Data_Ptr )
{
	if (State != XPS_OK)
	{
		return;
	}
	Refresh_Watchdog;

#ifdef Log_Level_1
	if (Setting & LOG_LEVEL_3)
	{
		Write_Log ( LOG_BUSY_MEMORY );
		Write_Log ( LOG_TAIL );
	}
#endif

	State = XPS_BUSY;
	while (State == XPS_BUSY)
	{
		HAL_Delay ( 5 );

		// Configure Instruction Info
		OSPI_Setting.OperationType = HAL_OSPI_OPTYPE_COMMON_CFG;
		OSPI_Setting.InstructionMode = HAL_OSPI_INSTRUCTION_1_LINE; // OSPI_INSTRUCTION_...
		OSPI_Setting.InstructionSize = HAL_OSPI_INSTRUCTION_8_BITS;
		// Based on register instruction will change
		OSPI_Setting.Instruction = XPS_READ_SR1;
		// Configure Address Info
		OSPI_Setting.AddressMode = HAL_OSPI_ADDRESS_NONE;
		OSPI_Setting.AddressSize = HAL_OSPI_ADDRESS_NONE;
		OSPI_Setting.Address = 0x0U;
		// Configure Packet/Message Info
		OSPI_Setting.DummyCycles = 0;
		OSPI_Setting.DataMode = HAL_OSPI_DATA_1_LINE;
		OSPI_Setting.NbData = 1;
		// Disables Unused Settings
		OSPI_Setting.AlternateBytesMode = HAL_OSPI_ALTERNATE_BYTES_NONE;
		OSPI_Setting.AlternateBytesSize = HAL_OSPI_ALTERNATE_BYTES_NONE;
		OSPI_Setting.AlternateBytes = HAL_OSPI_ALTERNATE_BYTES_NONE;
		OSPI_Setting.DataDtrMode = HAL_OSPI_DATA_DTR_DISABLE;
		OSPI_Setting.AddressDtrMode = HAL_OSPI_ADDRESS_DTR_DISABLE;
		OSPI_Setting.InstructionDtrMode = HAL_OSPI_INSTRUCTION_DTR_DISABLE;
		OSPI_Setting.SIOOMode = HAL_OSPI_SIOO_INST_EVERY_CMD;

		if (OSPItest ( &hospi1 , &OSPI_Setting , HAL_OSPI_TIMEOUT_DEFAULT_VALUE ) != HAL_OK)
		{
			State = XPS_STATE_CONFIG_ERR;
			return;
		}
		if (HAL_OSPI_Receive ( &hospi1 , &(Memory [ XPS_FLASH_SIZE + 1 ]) , HAL_OSPI_TIMEOUT_DEFAULT_VALUE ) != HAL_OK)
		{
			State = XPS_STATE_RECEIVE_ERR;
			return;
		}

		if (State != XPS_OK && State != XPS_BUSY)
		{
			return;
		}

		Busy = (Memory [ XPS_FLASH_SIZE + 1 ] & 0b1);
		State = (Busy) ? XPS_BUSY : XPS_OK;
	}
}

HAL_StatusTypeDef OSPItest(OSPI_HandleTypeDef *hospi, OSPI_RegularCmdTypeDef *cmd, uint32_t Timeout)
{
	HAL_StatusTypeDef status = HAL_OSPI_Command(hospi, cmd, Timeout);
	return status;
}

/******************************************************
 XPS Package Verify function
 Verifies Data_Ptr passed in is within acceptable
 parameters for the XPS command buffer
 ******************************************************/
void XPS_Verify_Memory ( MEM_PTR *Data_Ptr )
{
	if (Page >= PAGE_COUNT)
	{
		State = XPS_PAGE_PARAM_ERR;
	}
	if (Size == 0 || Size > 256)
	{
		State = XPS_SIZE_PARAM_ERR;
	}
	if (Shift > 256 - Size)
	{
		State = XPS_SHIFT_PARAM_ERR;
	}
}

void XPS_Set_Command ( XPS_CMD_TYPE Input )
{
	Command = Input;
}

XPS_STATE XPS_Get_State ( void )
{
	return State;
}


// KCS move to SystemTiming.c file
uint16_t getHbInterval (void)
{
	return (paramPage1 [ HB_INTERVAL_MSB ] <<8) + paramPage1 [ HB_INTERVAL_LSB ];
}
#endif
