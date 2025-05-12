/**
 ******************************************************************************
 * @file    Functions.c
 * @author  Erik Pineda-A
 * @brief   to define functions
 ******************************************************************************
 * @attention
 *
 * Copyright 2024 Team of Defenders
 *
 ******************************************************************************
 */
#ifndef FUNCTIONS_C_
#define FUNCTIONS_C_

#include "Functions.h"
#include "stdio.h"
#include <stdarg.h>

#ifdef Manual_Debug
#include "DEBUG_CONTROL.h"
#endif

#include "PIR.h"

#ifdef EXTERNAL_XPS_STORAGE
#include "XPS.h"
#endif

#ifdef SKYWIRE_MODEM
#include "Skywire.h"
#endif

#ifdef TEMPERATURE_SENSOR
#include "Temperature.h"
#endif

#ifdef ACCELERATION_SENSOR
#include "Accelerometer.h"
#endif

#ifdef INTERRUPT
#include "Interrupt.h"
#endif

#ifdef BOOT_CONFIGURE
#include "Bootloader.h"
#endif

#include "Bootloader.h"
#include "DayNight.h"

#ifdef Camera
#include "Camera.h"
#endif

#ifdef BUZZER
#include "Buzzer.h"
#endif

#ifdef DEBUG_MSG
#include "DebugMenu.h"
#endif

#ifdef Log
Logger_Lists Logger;
#endif

extern PIR_PARAMETER_TYPE pirData;
extern RTC_HandleTypeDef hrtc;
extern bool gpsError;
extern TIME_DATE_TYPE checkAcquireGPS;
extern TIME_DATE_TYPE nextAcquireGPS;

MQTT_MSG_TYPE downLinkPackets;

OTA_FILE_TYPE otaData;

char Sensor_Array[SENSOR_ARRAY_COUNT][SENSOR_SIZE] = {0}; // 2D array of char
uint16_t SENSOR_ARRAY_SIZE = 0;
uint16_t pagesToResend[100] = {0};
TIME_DATE_TYPE checkHB;
TIME_DATE_TYPE nextHB;
TIME_DATE_TYPE checkMute;
TIME_DATE_TYPE checkAccelMute;
TIME_DATE_TYPE nextMute;
TIME_DATE_TYPE nextAccelMute;
TIME_DATE_TYPE mutePeriodGPS;
ACCELERATION_PARAM_TYPE accellerationParameters;
CAMERA_PARAMETER_TYPE cameraParameters;

MACHINE_STATE_TYPE deviceState = IDLE;

uint16_t missingPagesCount = 0;

bool isModemPowered = false;
bool NightConfirmed = false;
bool fwPending = false;
bool mqttDataAvailable = false;
bool MuteInit = false;
bool accelMuteInit = false;
bool firstMotion = true;
bool firstMovement = false;
bool firstTimeBoot = true;
bool movementstop = false;

bool accelWakeupEnabled = false;
bool HBWakeupEnabled = false;

bool pIRTriggered = false;
bool timerTriggered = false;
bool accelTriggered = false;
bool accelMuteActive = false;
bool wakeupOverlap = false;
bool cellInitialized = false;

bool hbTimeChanged = false;
STRING_CONTAINER configErrContainer;

//enables/disables PIR mute period, tradeshow true is no mute period
bool tradeshow = false;

// Charger Cable state parameter
bool  chargerCableState = false;

uint16_t wakeupState = 0;

/******************************************************
 Data_Ptr Initialize
 MEM_PTR constructor
 ******************************************************/
void PACKAGE_Init ( MEM_PTR *Data_Ptr )
{
	_Page = 0;
	_Shift = 0;
	_Size = 1;
	_State = DEFAULT_STATE;
	_Init_Flags = SYS_INIT;

	_Temp_Device_ID = 0;
	_Temperature = Default_Temperature;
	_Humidity = Default_Humidity;

	//	_Acce_Device_ID = 0;
	//	_X_Axis = Default_X_Axis;
	//	_Y_Axis = Default_Y_Axis;
	//	_Z_Axis = Default_Z_Axis;

	mqttDataAvailable = false;
	Clear_Memory ( Data_Ptr );
	clearShadowMemory();
	memory_Init(Data_Ptr );

	Clear_Command ( Data_Ptr );
	gpsClearLocation();
	clearOTAFlashArray ( Data_Ptr );

	buf = 0;
	//	gpsHbItteration = 0;
	_Pointer = 0;
	Data_Ptr->Mode = START_MODE;

#ifdef SKYWIRE_MODEM
	CELL_Set_PDP ( PDP_NOT_SET );
#endif // SKYWIRE_MODEM
	// Added charger detect, subtracted GPS update for Gage demo
	_State = CHARGER_DETECT + WAKE_STATE + PIC_UPDT + UPDATE_SERVER + ACCEL_UPDT + HUMD_UPDT + TEMP_UPDT + GPS_UPDT + BUZZER_TOGGLE; //main flags
	//	_State = WAKE_STATE + PIC_UPDT + UPDATE_SERVER  + BUZZER_TOGGLE; // for testing
	//	_State = WAKE_STATE + PIC_UPDT + UPDATE_SERVER + CELL_UPDT + GPS_UPDT + BUZZER_TOGGLE; // for testing
	//	_State = WAKE_STATE + PIC_UPDT + UPDATE_SERVER + GPS_UPDT + ACCEL_UPDT + HUMD_UPDT + TEMP_UPDT + BUZZER_TOGGLE;
	//	_State = WAKE_STATE + PIC_UPDT + PIC_SEND + PIR_UPDT + PIR_START + GPS_UPDT + BUZZER_TOGGLE;
	//	_State = WAKE_STATE + PIC_UPDT;
	//	_State = SLEEP_STATE;

	_Init_Flags = XPS_INIT + TEMP_INIT + ACCEL_INIT + CAM_INIT + PIR_INIT;
	_Setting = LOG_LEVEL_1 + LOG_LEVEL_2;
}

void memory_Init (MEM_PTR *Data_Ptr )
{
	bool defaultSet = false;
	readDataValidFromFlash(Data_Ptr );
	// check if data good
	if (getDataValid())
	{
		readParamDataFromFlash(Data_Ptr); // populate shadow RAM
		// get all parameters
		XPS_paramRetrieve(Data_Ptr);
	}
	else
	{
		dayNightInit();       //Initialize business hours
		gpsParametersInit();  //Initialize GPS status parameters
		gpsDataInit();        //Initialize GPS data
		pirDataInit();        //Initialize PIR data
		accelDataInit(); //Initialize accelerometer data
		accelParametersInit(); //Initialize accelerometer parameters
		cameraParametersInit();
		environmentParametersInit();

		if ( latencyMin )
		{
			// KCS make a function in PIR.c file to set these values
			//			pirData.dayTime.Motion_Confirm_Window = 1;
			//			pirData.dayTime.Motion_Blackout = 5;
			//			pirData.dayTime.No_Motion_Detection_Window =  2;
			//
			//			pirData.nightTime.Motion_Confirm_Window = 1;
			//			pirData.nightTime.Motion_Blackout = 5;
			//			pirData.nightTime.No_Motion_Detection_Window =  2;
		}
		else
		{
			pirParametersInit();  //Initialize PIR Filter parameters
		}

		Data_Ptr->buzzerData.Single_Repeat = 2;
		Data_Ptr->buzzerData.Single_Length = 0x00001000;
		Data_Ptr->buzzerData.Single_Delay = 0x00003900;
		Data_Ptr->buzzerData.Cycles_Repeat = 0;
		Data_Ptr->buzzerData.Cycles_Length = 0x00001000;
		Data_Ptr->buzzerData.Cycles_Delay = 0x00001000;
		Data_Ptr->buzzerData.Start_Delay = 100;
		Data_Ptr->buzzerData.State = Short;    //GageDemo
		Data_Ptr->buzzerData.Control = 0;

		//	Data_Ptr->Temperature_Data.Timer = 0xFFF;
		Data_Ptr->Temperature_Data.Failed_Temperature_ID_Check = 0;
		Data_Ptr->Temperature_Data.Default_Temperature_Timer = 0;

		//	Data_Ptr->Accelerometer_Data.Timer = 0xFFF;
		//		Data_Ptr->Accelerometer_Data.Threshold_Level = 0;
		//		Data_Ptr->Accelerometer_Data.Time_Window = 0;

		//	Data_Ptr->Camera_Data.Cooldown = 0;
		//	Data_Ptr->Camera_Data.Camera_rdy = 1;
		//	Data_Ptr->Camera_Data.CAMERA = 0;

		//Data_Ptr->heartBeatData.modemWarmUp = MODEM_WARM_DEFAULT;
		// KCS call sensor parameter initialize default function (to be written)
		//thSensorParams.sensorWarmUp = SENSOR_WARM_DEFAULT;

		Data_Ptr->Flash.Page = 0;
		Data_Ptr->Flash.Index = 0;
		Data_Ptr->Flash.Total_Size = 0;
		Data_Ptr->Flash.currentFlashBank = BANK1;
		defaultSet = true;
	}

	// KCS these functions should be in GPS.c file
	//Check each section critical parameter and if param is 0, initialize all parameters for that section
	GPS_PARAMETER_TYPE localGpsCheck;
	getGpsParameters(&localGpsCheck);
	if(localGpsCheck.gpsInterval == 0)
	{
		gpsParametersInit(); //Initialize GPS status parameters
		gpsDataInit(); //Initialize GPS data
		defaultSet = true;
	}

	ACCELERATION_PARAM_TYPE localAccCheck;
	getAccelParameters(&localAccCheck);
	if(localAccCheck.mutePeriod == 0 || localAccCheck.mutePeriod == 33795) //temporary patch fix until root cause discovered
	{
		accelDataInit(); //Initialize accelerometer data
		accelParametersInit(); //Initialize accelerometer parameters
		defaultSet = true;
	}

	BUSINESS_DATA_TYPE localDayNightCheck;
	getBusinessHours(&localDayNightCheck);
	if(localDayNightCheck.dayTime.hours == 0 && localDayNightCheck.nightTime.hours == 0)
	{
		dayNightInit(); //Initialize business hours
		defaultSet = true;
	}

	// KCS these functions should be in PIR.c file
	PIR_PARAMETER_TYPE localPirCheck;
	getPirParameters(&localPirCheck);
	if(localPirCheck.coolDown == 0)
	{
		PRINTF("No PIR information in XPS, setting to default\r\n");
		pirParametersInit();//Initialize PIR parameters
		defaultSet = true;
	}

	if(Data_Ptr->heartBeatData.hbInterval == 0)
	{
		PRINTF("No HB information, setting to default\r\n");
		Data_Ptr->heartBeatData.hbInterval = DEFAULT_HB;
		Data_Ptr->heartBeatData.mode = DEFAULT_HB_MODE;
		defaultSet = true;
	}

	//KCS add Camera param check
	CAMERA_PARAMETER_TYPE localCamCheck;
	getCameraParameters(&localCamCheck);
	if(localCamCheck.warmUp == 0)
	{
		PRINTF("No Camera info found, setting to default \r\n");
		cameraParametersInit();
		defaultSet = true;
	}

	ENVIRONMENT_PARAM_TYPE localEnvCheck;
	getEnvironmentParameters(&localEnvCheck);
	if(localEnvCheck.sensorWarmUp == 0)
	{
		PRINTF("No Environment info found, setting to default \r\n");
		environmentParametersInit();
		defaultSet = true;
	}

	if(defaultSet)
	{
		XPS_paramStore(Data_Ptr);
		saveParamDataToFlash(Data_Ptr);
	}

	//Initializes container for configuration error strings
	initContainer(&configErrContainer);
}

void memoryTest1 ( MEM_PTR *Data_Ptr )
{
	Data_Ptr->Memory [0] = 0x10; // memory is good
	int memoryItterator = 1;
	while ( memoryItterator < (200))
		Data_Ptr->Memory [ memoryItterator++ ] = 0x11;
}

void memoryTest2 ( MEM_PTR *Data_Ptr )
{
	int memoryItterator = 1;
	while ( memoryItterator < (200))
		Data_Ptr->Memory [ memoryItterator++ ] = 0xF;
}

/******************************************************
 Initializer
 This functions resets and recalls any component
 that has a fail status
 ******************************************************/
void Component_Initalizer ( MEM_PTR *Data_Ptr )
{
	//	HAL_NVIC_DisableIRQ ( PIR_EXTI_IRQn );	// Turn off event
	cameraPowerControl (false);

	if ( _State == DEFAULT_STATE )
	{
		Refresh_Watchdog;
		HAL_Delay ( 1000 );
		_State = DEVICE_LISTEN;
		_State |= WAKE_STATE;

#ifdef Log_Level_1
		_Setting |= LOG_LEVEL_1;

#ifdef Log_2_Start
		_Setting |= LOG_LEVEL_2;
#endif // Log_2_Start
		Write_Log ( LOG_INITIALIZE );
		Write_Log ( LOG_TAIL );
#endif  //Log_Level_1
	}

	if ( _Init_Flags & XPS_INIT )
	{
		_Init_Flags ^= XPS_INIT;

#ifdef Log_Level_0
		Log_Single ( XPS_INIT_MSG );
#endif // Log_Level_0

		XPS_Set_Command ( SETTLE );
		XPS_COMMAND ( Data_Ptr );

		if (XPS_Get_State () != XPS_OK)
		{
			PRINTF("External Flash Memory Failed!\r\n");
		}
	}

	if ( _Init_Flags & CELL_INIT )
	{
		_Init_Flags ^= CELL_INIT;

#ifdef SKYWIRE_MODEM
		//		CELL_Set_Command ( DISABLEQI );
		//		CELL_COMMAND ( Data_Ptr );

		CELL_Init ( Data_Ptr );

		if ( CELL_Get_State () != CELL_OK )
		{
			CELL_HARDWARE_RESET ( Data_Ptr );
			_Init_Flags |= CELL_INIT;
		}
#endif  //SKYWIRE_MODEM
	}

	if ( _Init_Flags & TEMP_INIT )
	{
		_Init_Flags ^= TEMP_INIT;

		tempInit();
#ifdef Log_Level_0
		Log_Single ( TEM_INIT_MSG );
#endif // Log_Level_0

#ifdef TEMPERATURE_SENSOR
		//TEM_Init ( Data_Ptr );
#endif // TEMPERATURE_SENSOR
	}

	if ( _Init_Flags & ACCEL_INIT )
	{
		_Init_Flags ^= ACCEL_INIT;
#ifdef Log_Level_0
		Log_Single ( ACC_INIT_MSG );
#endif // Log_Level_0
	}

	if ( _Init_Flags & CAM_INIT )
	{
		_Init_Flags ^= CAM_INIT;

#ifdef Log_Level_0
		Log_Single ( CAM_INIT_MSG );
#endif // Log_Level_0

#ifdef Camera

		getCameraParameters(&cameraParameters);
		PRINTF("Camera mode is %d\r\n", cameraParameters.mode);
//		cameraParameters.mode = 1;//testing

		if (cameraParameters.mode != 0)
		{
			cameraInitialize();
		}

#endif // Camera
	}

	if ( _Init_Flags & PIR_INIT )
	{
		_Init_Flags ^= PIR_INIT;

#ifdef Log_Level_0
		Log_Single ( PIR_INIT_MSG );
#endif // Log_Level_0

		pirDataInit();
	}

	if ( _Init_Flags & INTERRUPT_INIT )
	{
		_Init_Flags ^= INTERRUPT_INIT;

#ifdef SKYWIRE_MODEM
		HAL_UART_Receive_IT ( &hlpuart1 , _Memory , 1 );
#endif // SKYWIRE_MODEM
	}

	if(_State & GPS_UPDT)
	{
		_State ^= GPS_UPDT;
		Enable_Modem_PWR(Data_Ptr);
		//Set this parameter for Active if necessary
		bool passiveAntenna = true;
		if(!passiveAntenna)
		{
			GPSActiveAntennaON(Data_Ptr);
		}
		EnableGPSXtraFunctionality(Data_Ptr);
		getGPS( Data_Ptr);
	}

	accelInit();
}

/******************************************************
 State Update Function
 Updates device based on toggled flags
 ******************************************************/
void Update_State ( MEM_PTR *Data_Ptr )
{
	PRINTF("Entering Update_State; State is %lu\r\n",_State );
	if ( _State & SLEEP_STATE )
	{
		PRINTF("Update_State includes SLEEP\r\n");
		_State ^= SLEEP_STATE;

#ifdef Device_Sleep

#ifndef BUZZER
		HAL_GPIO_TogglePin ( Buzzer_GPIO_Port , Buzzer_Pin );
		HAL_Delay ( BUZZER_TIME * 0.5 );
		HAL_GPIO_TogglePin ( Buzzer_GPIO_Port , Buzzer_Pin );
#endif  // BUZZER

		//HAL_PWR_EnableWakeUpPin ( INT1_SLEEP_WAKE_PIN );
		//HAL_PWR_EnableWakeUpPin ( INT2_SLEEP_WAKE_PIN );
		//HAL_PWR_EnableWakeUpPin ( PIR_SLEEP_WAKE_PIN );
		//HAL_UART_AbortReceive_IT ( &hlpuart1 );   //Added by Gage

		if ( _State & CHARGER_DETECT )
		{
			PRINTF("Update_State includes SLEEP and CHARGER_DETECT\r\n");
#ifdef Log_Level_0
			Log_Single ( LOG_IDLE_START );
#endif // Log_Level_0
			//			bool downlinkTesting = true;
			//			if(downlinkTesting)
			//			{
			//				// local copy just to print out messages
			//				BUSINESS_DATA_TYPE downlinkBusinessHours;
			//
			//				Refresh_Watchdog;
			//
			//				const char *testMsg = "{\"day_night-v0.0\":[1,\"08:01\",\"18:02\"]}\0";
			//				strncpy((char *)MQTTMsg[0], testMsg, strlen(testMsg));
			//				decodeDayNightConfigs(MQTTMsg[0]);
			//				getBusinessHours(&downlinkBusinessHours);
			//
			//				PRINTF("Day Hours: %d\r\n", downlinkBusinessHours.Daytime.hours);
			//				PRINTF("Day Minutes: %d\r\n", downlinkBusinessHours.Daytime.minutes);
			//				PRINTF("Night Hours: %d\r\n", downlinkBusinessHours.Nighttime.hours);
			//				PRINTF("Night Minutes: %d\r\n", downlinkBusinessHours.Nighttime.minutes);
			//			}

			if (!latencyMin)
			{
				HAL_GPIO_WritePin ( MEM_CS_GPIO_Port , MEM_CS_Pin , GPIO_PIN_SET );    //for simulated battery power operation

				HAL_GPIO_WritePin ( Cell_Enable_GPIO_Port , Cell_Enable_Pin , GPIO_PIN_RESET );    //for simulated battery power operation
				HAL_GPIO_WritePin ( Cell_DVS_GPIO_Port , Cell_DVS_Pin , GPIO_PIN_RESET );          //for simulated battery power operation

				//HAL_GPIO_WritePin ( Sky_Reset_GPIO_Port , Sky_Reset_Pin , GPIO_PIN_SET );          //for simulated battery power operation
				//HAL_GPIO_WritePin ( Sky_On_Off_Sw_GPIO_Port , Sky_On_Off_Sw_Pin , GPIO_PIN_SET );  //for simulated battery power operation
				HAL_GPIO_WritePin ( USB_Power_Switch_On_GPIO_Port , USB_Power_Switch_On_Pin , GPIO_PIN_RESET );    //for simulated battery power operation
				cameraPowerControl(false);
				isModemPowered = false;

				XPS_Set_Command (SLEEP);
				XPS_COMMAND ( Data_Ptr );
				//HAL_OSPI_Transmit( &hospi1 , (uint8_t*)185 , HAL_OSPI_TIMEOUT_DEFAULT_VALUE );
				Refresh_Watchdog;

				stop3Prepare();

				__HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_ALL);

				if(hbTimeChanged && !firstTimeBoot)
				{
					HAL_RTC_GetTime ( &hrtc , &nextHB.rtcTime , RTC_FORMAT_BIN );
					HAL_RTC_GetDate ( &hrtc , &nextHB.rtcDate , RTC_FORMAT_BIN );
					calculateNextTime( &nextHB, Data_Ptr->heartBeatData.hbInterval );
					hbTimeChanged = false;
				}

				if (timerTriggered || pIRTriggered || accelTriggered )
				{
					uint16_t wakeUp = 0;

					uint16_t HBwakeUp = getTimeFromNow(nextHB);
					uint16_t PIRwakeUp = getTimeFromNow(nextMute);
					uint16_t AccwakeUp = getTimeFromNow(nextAccelMute);
					uint16_t PeriodicGPSWakeup = getTimeFromNow(mutePeriodGPS);


					PRINTF("Heartbeat timer is %u\r\n" , HBwakeUp );
					PRINTF("PIR timer is %u\r\n" , PIRwakeUp );
					PRINTF("pGPS timer is %u\r\n" , PeriodicGPSWakeup );
					PRINTF("Accelerometer timer is %u\r\n\r\n\r\n" , AccwakeUp );


					wakeUp = minValue4(HBwakeUp,PIRwakeUp,AccwakeUp,PeriodicGPSWakeup);
					PRINTF("Wakeup timer is %u\r\n" , wakeUp );

					//					PRINTF("Triggered by accelerometer, Wakeup timer is %u\r\n" , wakeUp);
					if (HAL_RTCEx_SetWakeUpTimer_IT(&hrtc, wakeUp, RTC_WAKEUPCLOCK_CK_SPRE_16BITS, 0) != HAL_OK)
					{
						Error_Handler();
					}
				}
				else
				{
					//to handle first time sleep entry
					PRINTF("Wakeup timer is %u\r\n" , Data_Ptr->heartBeatData.hbInterval );
					if (HAL_RTCEx_SetWakeUpTimer_IT(&hrtc, Data_Ptr->heartBeatData.hbInterval , RTC_WAKEUPCLOCK_CK_SPRE_16BITS, 0) != HAL_OK)
					{
						Error_Handler();
					}

				}

				timerTriggered = false;
				accelTriggered = false;
				pIRTriggered = false;


				SET_BIT(PWR->WUSCR, PWR_WUSCR_CWUF);

				//__HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_0);
				__HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_ALL);

				//HAL_Delay(3000);

				if(getMode(PIR_MODULE) != 0)
				{
					// Enable PIR
					PRINTF("Enable PIR wakeup, PIR mode is %d\r\n", getMode(PIR_MODULE));
					HAL_PWR_EnableWakeUpPin ( GPIO_PIN_1 );
					HAL_PWR_EnableWakeUpPin ( GPIO_PIN_0 );
				}
				else
				{
					// If PIR is "OFF" (mode == 0 )then no reason to have it on
					PRINTF("PIR is Inactive due to mode\r\n" );
				}

				//				HAL_PWR_EnableWakeUpPin ( PWR_WAKEUP_PIN6 );
				//				HAL_PWR_EnableWakeUpPin ( GPIO_PIN_5 );

				accelClearLatch();

				//				SET_BIT(PWR->WUSCR, PWR_WUSCR_CWUF);
				//				__HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_0);
				//				__HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_ALL);
				SET_BIT(PWR->WUSCR, PWR_WUSCR_CWUF);

				//__HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_0);
				__HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_ALL);

				HAL_PWR_EnableWakeUpPin(PWR_WAKEUP_PIN7_HIGH_3);
				HAL_NVIC_EnableIRQ(RTC_IRQn);

				HAL_NVIC_EnableIRQ(PWR_S3WU_IRQn);


				getAccelParameters(&accellerationParameters);

				if (accellerationParameters.mode != 0 && (!tradeshow))
				{
					HAL_NVIC_EnableIRQ(EXTI6_IRQn);  //Accelerometer
					HAL_PWR_EnableWakeUpPin ( PWR_WAKEUP_PIN6_HIGH_1 );
				}

				SET_BIT(PWR->WUSCR, PWR_WUSCR_CWUF6);

				cellInitialized = false;

				HAL_PWREx_EnableUltraLowPowerMode ();
				//	HAL_PWREx_ConfigSRDDomain ( PWR_SRD_DOMAIN_STOP );
				//	HAL_PWREx_EnterSTOP1Mode ( PWR_SLEEPENTRY_WFI );
				//	HAL_PWREx_EnterSTOP2Mode ( PWR_SLEEPENTRY_WFE );
				HAL_PWREx_EnterSTOP3Mode ( PWR_STOPENTRY_WFI);

			}

			HAL_PWR_DisableWakeUpPin ( GPIO_PIN_1 );
			HAL_PWR_DisableWakeUpPin ( GPIO_PIN_0 );
			HAL_PWR_DisableWakeUpPin ( PWR_WAKEUP_PIN6 );
			HAL_PWR_DisableWakeUpPin ( PWR_WAKEUP_PIN7_HIGH_3 );
			//HAL_PWR_DisableWakeUpPin ( PWR_WAKEUP_PIN6_HIGH_1 );


			__HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_ALL);
			HAL_GPIO_WritePin ( Buzzer_GPIO_Port , Buzzer_Pin , GPIO_PIN_RESET );  //for simulated battery power operation


			if ( _State & WAKE_STATE )
			{
				//	__enable_irq();

				Refresh_Watchdog;
				HAL_UART_AbortReceive_IT ( &hlpuart1 );
				isModemPowered = false;
				PRINTF("THE DEVICE IS WOKEN UP\r\n" );

				HAL_GPIO_WritePin ( Buzzer_GPIO_Port , Buzzer_Pin , GPIO_PIN_RESET );          //for simulated battery power operation

				//waking up for a heartbeat, we call this calculatenexttime immediately here to minimize time drift
				if (timerTriggered && (wakeupState == hb_wu_enabled))
				{
					PRINTF("Calculating next HB time, HB interval is %u\r\n", Data_Ptr->heartBeatData.hbInterval);
					calculateNextTime( &nextHB, Data_Ptr->heartBeatData.hbInterval );
					uint16_t testington = getTimeFromNow(nextHB);
					PRINTF("Next timer is %u\r\n" , testington);
					PRINTF("Setting HBWakeupEnabled to false\r\n");
					HBWakeupEnabled = false;
				}

				// Enter camera power off
				cameraPowerControl (false);

				if (timerTriggered)
				{
					if ((wakeupState == pGPS_wu_enabled) || (wakeupState == hb_pGPS_pair)
							|| (wakeupState == img_pGPS_pair) || (wakeupState == hb_img_pGPS_pair))
					{
						calculateNextTime( &mutePeriodGPS, getMutePeriodGPS() );
					}
				}

				NightConfirmed = false;
				if (!tradeshow)
				{
					PRINTF("Daytime PIR Trigger\r\n");
					if (checkFunctionActive(PIR_MODULE))  // Make sure Daytime is active
					{
						if (!MuteInit && pIRTriggered)
						{
							PRINTF("Setting 1 Picture Muting period of %d seconds\r\n", getPirMutePeriod());
							calculateNextTime( &nextMute, (uint32_t)getPirMutePeriod() );
							MuteInit = true;
							firstMotion = true;
						}

						if ((!checkNextTime(checkMute, nextMute)) && (firstMotion == false) && (pIRTriggered))
						{
							PRINTF("WITHIN MUTE PERIOD, Powering Down\r\n");
							powerDownDeviceForSleep();
							if (pIRTriggered)
							{
								clearPirInterreptUpdateStates();
								PRINTF("Setting 2 Picture Muting period of %d seconds\r\n", getPirMutePeriod());
								calculateNextTime( &nextMute, (uint32_t)getPirMutePeriod() );
							}
						}
						else
						{
							if(pIRTriggered)
							{
								PRINTF("Starting MUTE period of %d seconds\r\n", getPirMutePeriod());
								calculateNextTime( &nextMute, (uint32_t)getPirMutePeriod());
								firstMotion = false;
								NightConfirmed = true;    //added
							}
						}
					}
					else // Daytime not active, print message and power down
					{
						PRINTF("Picture Mode not Active, Powering Down\r\n");
						powerDownDeviceForSleep();
						if (pIRTriggered)
						{
							clearPirInterreptUpdateStates();
						}
					}
				}

				//accelerometer mute period handling
				if (accelTriggered)
				{
					// Enter camera power off
					cameraPowerControl (false);

					if (!accelMuteInit && accelTriggered)
					{
						//use parameter instead of hardcoded value
						calculateNextTime( &nextAccelMute, getAccelMutePeriod() );
						calculateNextTime( &mutePeriodGPS, getMutePeriodGPS() );

						accelMuteInit = true;
						firstMovement = true;
					}
					if ((!checkNextTime(checkAccelMute, nextAccelMute)) && (firstMovement == false) && (accelTriggered))
					{
						PRINTF("WITHIN ACCELEROMETER MUTE PERIOD, Powering Down\r\n");
						powerDownDeviceForSleep();
						clearAccellerometerInterruptStates();
						PRINTF("Setting ACCELEROMETER Picture Muting period of %d seconds\r\n", getAccelMutePeriod());
						calculateNextTime( &nextAccelMute, getAccelMutePeriod() );
					}
					else
					{
						if(accelTriggered)
						{
							PRINTF("Starting ACCELEROMETER MUTE period of %d seconds\r\n", getAccelMutePeriod());
							calculateNextTime( &nextAccelMute, getAccelMutePeriod());
							firstMovement = false;
						}
					}
				}
			}

#ifdef Log_Level_0
			Log_Single ( LOG_IDLE_END );
#endif // Log_Level_0
		}

		if ( !( _State & WAKE_STATE ) )
		{
#ifdef Log_Level_0
			Log_Single ( LOG_SLEEP_START );
#endif // Log_Level_0

			//	Disable_Extra_Power ( Data_Ptr );

			//	while ( !( _State & WAKE_STATE ) )
			//	{
			//		Refresh_Watchdog;
			//
			//		if ( _State & GPIO_UPDT )
			//		{
			//			_State ^= GPIO_UPDT;
			//
			//			_State |= WAKE_STATE;
			//		}
			//
			//		HAL_PWREx_EnableUltraLowPowerMode ();
			//		//	HAL_PWREx_ConfigSRDDomain ( PWR_SRD_DOMAIN_STOP );
			//		//	HAL_PWREx_EnterSTOP1Mode ( PWR_SLEEPENTRY_WFI );
			//		//	HAL_PWREx_EnterSTOP2Mode ( PWR_SLEEPENTRY_WFE );
			//		HAL_PWREx_EnterSTOP3Mode ( PWR_SLEEPENTRY_WFE );
			//		//	HAL_PWREx_EnterSHUTDOWNMode ();
			//	}

#ifdef Log_Level_0
			Log_Single ( LOG_SLEEP_END );
#endif // Log_Level_0
		}

#ifndef BUZZER
		HAL_GPIO_TogglePin ( Buzzer_GPIO_Port , Buzzer_Pin );
		HAL_Delay ( BUZZER_TIME + BUZZER_TIME );
		HAL_GPIO_TogglePin ( Buzzer_GPIO_Port , Buzzer_Pin );
#endif  //BUZZER
		/* Resume Tick increment */
		//	HAL_ResumeTick ();
#endif  //Device_Sleep
	}

	if ( _State & WAKE_STATE )
	{
		_State ^= WAKE_STATE;

#ifdef Log_Level_0
		Log_Single ( LOG_WAKE_CYCLE );
#endif // Log_Level_0

		if ( _State & CHARGER_DETECT )
		{
#ifdef Log_Level_0
			Log_Single ( POWER_DETECT );
#endif // Log_Level_0
		}
		else
		{
#ifdef Log_Level_0
			Log_Single ( POWER_FAIL_DETECT );
#endif // Log_Level_0
		}
		PRINTF("Update_State includes WAKE_STATE\r\n");
		if ( _State & BUZZER_TOGGLE )
		{
			PRINTF("Update_State includes WAKE_STATE and BUZZER_TOGGLE\r\n");
			_State ^= BUZZER_TOGGLE;

			switch ( Data_Ptr->buzzerData.State )
			{
			case Short:
				Data_Ptr->buzzerData.Single_Repeat = 0;
				Data_Ptr->buzzerData.Cycles_Repeat = 1;
				break;
			case Short_Spam:
				Data_Ptr->buzzerData.Single_Repeat = 0;
				Data_Ptr->buzzerData.Cycles_Repeat = 3;
				break;
			case Long:
				Data_Ptr->buzzerData.Single_Repeat = 2;
				Data_Ptr->buzzerData.Cycles_Repeat = 0;
				break;
			case Long_Spam:
				Data_Ptr->buzzerData.Single_Repeat = 5;
				Data_Ptr->buzzerData.Cycles_Repeat = 2;
				break;
			case Custom:
				break;
			}

			BUZ_Init ();
		}

		if ( _State & FLASH_START )
		{
			_State ^= FLASH_START;

#ifdef New_Boot_ADDR
			Upgrade_Firmware();
#endif // New_Boot_ADDR
		}

		if ( _State & PARAM_UPDT )
		{

			PRINTF("Update_State includes WAKE_STATE and PARAM_UPDT\r\n");
			_State ^= PARAM_UPDT;

			XPS_paramStore ( Data_Ptr );
		}

		//		if ( _State & ACCEL_UPDT )
		//		{
		//			_State ^= ACCEL_UPDT;
		//
		//#ifdef ACCELERATION_SENSOR
		//			if ( _X_Axis != Default_X_Axis )
		//			{
		//				for ( _X_Axis = 0; _X_Axis == 0; )
		//				{
		//					ACC_Set_Command ( ACCELERATION );
		//					ACC_COMMAND ( Data_Ptr );
		//				}
		//			}
		//
		//			if ( _X_Axis == Default_X_Axis || ACC_Get_State () != ACC_OK )
		//				_Init_Flags ^= ACCEL_INIT;
		//
		//			/*
		//			#ifdef SKYWIRE_MODEM
		//			if ( _State & CELL_UPDT )
		//			{
		//				CELL_ACCEUPDT ( Data_Ptr );
		//			}
		//
		//			#endif // SKYWIRE_MODEM
		//			 */
		//#endif  //ACCELERATION_SENSOR
		//		}

//		if ( _State & TEMP_UPDT )
//		{
//			PRINTF("Update_State includes WAKE_STATE and TEMP_UPDT\r\n");
//			_State ^= TEMP_UPDT;
//
//#ifdef TEMPERATURE_SENSOR
//			if ( _Temperature != Default_Temperature ) //   _Temperature = 0;
//			{
//				_Setting |= AVOID_MSG_SPAM;
//				for ( _Temperature = 0; _Temperature == 0;)
//				{
//					tempReadRegister(HDC2080_TEMP_LOW, 2);
//				}
//				_Setting ^= AVOID_MSG_SPAM;
//			}
//
////			if (_Temperature == Default_Temperature || TEM_Get_State () != TEM_OK)
//				_Init_Flags ^= TEMP_INIT;
//
////#ifdef SKYWIRE_MODEM
////	if (_State & CELL_UPDT)
////{
////	CELL_TEMPUPDT ( Data_Ptr );
////}
////#endif  //SKYWIRE_MODEM
//
//#endif  //TEMPERATURE_SENSOR
//		}

//		if ( _State & HUMD_UPDT )
//		{
//			PRINTF("Update_State includes WAKE_STATE and HUMD_UPDT\r\n");
//			_State ^= HUMD_UPDT;
//
//#ifdef TEMPERATURE_SENSOR
//			if ( _Humidity != Default_Humidity )
//			{
//				_Setting |= AVOID_MSG_SPAM;
//				int HumTryCounter = 0;
//				do{
//
//					HDC2080_TEMP_LOW
//					HumTryCounter++;
//				}
//				while ((HumTryCounter <= 10) & (_Humidity < 1000));
//				HumTryCounter = 0;
//				_Setting ^= AVOID_MSG_SPAM;
//			}
//
////			if ( _Humidity == Default_Humidity || TEM_Get_State () != TEM_OK )
//				_Init_Flags ^= TEMP_INIT;
//			//
//			//			#ifdef SKYWIRE_MODEM
//			//			if (_State & CELL_UPDT)
//			//			{
//			//				CELL_HUMDUPDT ( Data_Ptr );
//			//			}
//			//
//			//			#endif //SKYWIRE_MODEM
//
//#endif  //TEMPERATURE_SENSOR
//		}

		if ( _State & PIC_UPDT )
		{
			_State ^= PIC_UPDT;
			getCameraParameters(&cameraParameters);
			PRINTF("Camera mode is %d\r\n", cameraParameters.mode);

//			cameraParameters.mode = 1;//testing

			if (cameraParameters.mode != 0)
			{
			PRINTF("Update_State includes WAKE_STATE and PIC_UPDT\r\n");
			cameraPowerControl(true);
			cameraInitialize();
			Clear_Memory( Data_Ptr );
			HAL_Delay(1000);      //GAV camera warm up, need to make parameterized
			Refresh_Watchdog;

			bool printCamData = false;
			if (firstTimeBoot)
			{
				printCamData = true;
			}
			takeCameraImage ( &Data_Ptr->Camera_Data, printCamData );
			if (CAM_Get_State () != CAM_OK)
				_Init_Flags ^= CAM_INIT;
			HAL_GPIO_WritePin ( USB_Power_Switch_On_GPIO_Port , USB_Power_Switch_On_Pin , GPIO_PIN_RESET );    //for simulated battery power operation
			cameraPowerControl(false);

			//	isModemPowered = false;
			//	CELL_Set_PDP ( PDP_NOT_SET );

			if (firstTimeBoot)
			{
				Clear_Memory(Data_Ptr);
				//Enable_Modem ( Data_Ptr );
				//firstTimeBoot = false; moved to end so can use in other places
			}
			else
			{
				if (!cellInitialized)
				{
					Enable_Modem_PWR ( Data_Ptr );
					CELL_reInit( Data_Ptr);
				}
			}

			// Handle Downlinks before sending picture data
			getFirmwareDownlink(Data_Ptr, 1);
			if ( mqttDataAvailable )
			{
				universalDownlinkDecoder();
				selectDownlinkOperation(Data_Ptr, IMAGE_TAKEN);
			}
			}
		}
		if ( _State & PIR_UPDT )
		{
			_State ^= PIR_UPDT;
			PRINTF("Update_State includes WAKE_STATE and PIR_UPDT\r\n");

			if (!cellInitialized)
			{
				Enable_Modem_PWR ( Data_Ptr );
				CELL_reInit( Data_Ptr);
			}
			getFirmwareDownlink(Data_Ptr, 1);
			if ( mqttDataAvailable )
			{
				universalDownlinkDecoder();
				selectDownlinkOperation(Data_Ptr, IDLE);
			}
			CELL_PIRUPDT ( Data_Ptr, true );

		}

		if ( _State & PIC_SEND )
		{
			PRINTF("Update_State includes WAKE_STATE and PIC_SEND\r\n");
			_State ^= PIC_SEND;
			//Enable_Modem ( Data_Ptr );

			getCameraParameters(&cameraParameters);
			PRINTF("Camera mode is %d\r\n", cameraParameters.mode);
//			cameraParameters.mode = 1;//testing

			if (cameraParameters.mode != 0)
			{
#ifdef SKYWIRE_MODEM
			CELL_PICUPDT ( Data_Ptr );
#endif  // SKYWIRE_MODEM
			//Set_Timer_B ( Data_Ptr );
			}
		}

		if ( _State & GET_CELL_RECV_BUFF )
		{
			PRINTF("Update_State includes WAKE_STATE and GET_CELL_RECV_BUFF\r\n");
			_State ^= GET_CELL_RECV_BUFF;

			getFirmwareDownlink(Data_Ptr, 2);
			if ( mqttDataAvailable )
			{
				universalDownlinkDecoder();
				selectDownlinkOperation(Data_Ptr, IMAGE_COMPLETE);
			}

			if ( missingPagesCount > 0 )
				resendMissingPackets(Data_Ptr, missingPagesCount);
		}

		if(_State & PIR_END)
		{
			PRINTF("Update_State includes WAKE_STATE and PIR_UPDT\r\n");
			_State ^= PIR_END;

			if (!cellInitialized)
			{
				Enable_Modem_PWR ( Data_Ptr );
				CELL_reInit( Data_Ptr);
			}

			getFirmwareDownlink(Data_Ptr, 1);
			if ( mqttDataAvailable )
			{
				universalDownlinkDecoder();
				selectDownlinkOperation(Data_Ptr, IDLE);
			}
			CELL_PIRUPDT ( Data_Ptr, false );
		}

		if ( _State & PIC_SAVE )
		{
			PRINTF("Update_State includes WAKE_STATE and PIC_SAVE\r\n");
			_State ^= PIC_SAVE;

#ifdef EXTERNAL_XPS_STORAGE
			XPS_Picture_Save ( Data_Ptr );
#endif //EXTERNAL_XPS_STORAGE
		}

		//		if ( _State & PWR_UPDT )
		//		{
		//			_State ^= PWR_UPDT;
		//
		//			Enable_Modem ( Data_Ptr );
		//
		//#ifdef SKYWIRE_MODEM
		//			if ( _State & CELL_UPDT )
		//				CELL_PWRUPDT ( Data_Ptr );
		//#endif // SKYWIRE_MODEM
		//		}

		if ( _State & SERVER_COMMAND )
		{
			PRINTF("Update_State includes WAKE_STATE and SERVER_COMMAND\r\n");
			_State ^= SERVER_COMMAND;

			Enable_Modem ( Data_Ptr );

#if defined( SKYWIRE_MODEM ) && defined( VALID_PACKAGE )
			//GAV insert missing pages here
			//CELL_SERVER_UPDT ( Data_Ptr );
			//Execute_Instructions ( Data_Ptr );
#endif // SKYWIRE_MODEM VALID_PACKAGE

			Clear_Memory ( Data_Ptr );
		}

		if ( _State & UPDATE_SERVER )
		{
			PRINTF("Update_State includes WAKE_STATE and UPDATE_SERVER\r\n");
			_State ^= UPDATE_SERVER;

			//Enable_Modem ( Data_Ptr );

			if (!firstTimeBoot)
			{

				//Set look for downlinks
				getFirmwareDownlink(Data_Ptr, 1);
				if ( mqttDataAvailable )
				{
					universalDownlinkDecoder();
					selectDownlinkOperation(Data_Ptr, WAKEUP_HB);
				}

				Clear_Memory ( Data_Ptr );
				//HAL_TIM_Base_Start_IT(&htim1);
			}
		}

		if ( _State & CELL_UPDT )
		{
			PRINTF("Update_State includes WAKE_STATE and CELL_UPDT\r\n");
			_State |= CELL_UPDT; //GAV We can reuse this flag, rename
		}

		if ( _State & GET_DOWNLINKS )
		{
			PRINTF("Update_State includes WAKE_STATE and GET_DOWNLINKS\r\n");
			_State ^= GET_DOWNLINKS;
			Enable_Modem ( Data_Ptr );
			getFirmwareDownlink(Data_Ptr, 1);
		}

		if (fwPending)
		{
			startOTAProcess(Data_Ptr);
		}
		//checkTime();

		if (firstTimeBoot)
		{
			PRINTF("Update_State HBonBootup added HB\r\n");
			HeartBeat ( Data_Ptr );
			//  calculateNextHBTime();
			//	calculateNextTime(&checkHB, &nextHB, 300);
			if(!gpsError)
			{
				sendGPS(Data_Ptr);
			}
			else
			{
				sendDiagnostic(&memory, "\"gps\":[\"unknown_location\"]");
			}

			// Send Diagnostic message battery charger cable state

			if (chargeCableReading())
			{
				sendDiagnostic(&memory, "\"battery\":[\"plugged_in\"]");
			}
			else
			{
				sendDiagnostic(&memory, "\"battery\":[\"unplugged\"]");
			}

			if (_State & HB_UPDT)
			{
				_State ^= HB_UPDT;
			}

			HAL_RTC_GetTime ( &hrtc , &nextHB.rtcTime , RTC_FORMAT_BIN );
			HAL_RTC_GetDate ( &hrtc , &nextHB.rtcDate , RTC_FORMAT_BIN );
			calculateNextTime( &nextHB, Data_Ptr->heartBeatData.hbInterval );

		}
		else //if(!firstTimeBoot)
		{
			if ( _State & GPS_UPDT )
			{
				PRINTF("Update_State includes WAKE_STATE and GPS_UPDT\r\n");
				_State ^= GPS_UPDT;

				if(!isModemPowered)
				{
					Enable_Modem_PWR(Data_Ptr);
				}
				getGPS(Data_Ptr);

				if (!cellInitialized)
				{
					CELL_Set_PDP ( PDP_NOT_SET );
					CELL_reInit(Data_Ptr);
				}
				getFirmwareDownlink(Data_Ptr, 1);
				if ( mqttDataAvailable )
				{
					universalDownlinkDecoder();
					selectDownlinkOperation(Data_Ptr, IDLE);
				}

				if (_State & MOVEMENT_UPDT)
				{
					PRINTF("Update_State includes WAKE_STATE and MOVEMENT_UPDT\r\n");
					_State ^= MOVEMENT_UPDT;

					if (!movementstop)
					{
						cellMovementStart(Data_Ptr, movementstop);
					}

				}
				if(!gpsError)
				{
					sendGPS(Data_Ptr);
				}
				else
				{
					sendDiagnostic(&memory, "\"gps\":[\"unknown_location\"]");
				}
			}

			if (_State & HB_UPDT)
			{
				PRINTF("Update_State includes WAKE_STATE and HB_UPDT\r\n");
				_State ^= HB_UPDT;

				if (!cellInitialized)
				{
					Enable_Modem_PWR(Data_Ptr);
					CELL_reInit(Data_Ptr);
				}

				NightConfirmed = true;   //GAV "fix" added to prevent extra modem enable in "sendHeartBeat
				sendHeartBeat(Data_Ptr);
			}

			if (movementstop)
			{
				cellMovementStart(Data_Ptr, movementstop);
				movementstop = false;
			}
		}

	}

	//PRINTF("Night reset\r\n");

	NightConfirmed = false;
	firstTimeBoot = false;
	_State |= SLEEP_STATE;

#ifdef Log
	Write_Log ( LOG_DEVICE_CYCLE );
#endif // Log
}

void OTAProcess (MEM_PTR *Data_Ptr, OTA_FILE_TYPE *OTAData )
{
	OTAData->batchNum = 1;
	Data_Ptr->Flash.Index = 0;
	// TODO: set last ROM address to start of Page1
	Refresh_Watchdog;
	clearFirmwareArray(OTAData);
	int errorMode = 0;
	uint32_t calCRC = 0;
	OTAData->numberOfFiles = 1;
	for (int fwFile = 0; fwFile < OTAData->numberOfFiles; fwFile++)
	{
		getFirmwareDownlink(Data_Ptr, 5);
		// gets 1st batch
		if(populateFirmwareArray(OTAData)>1)
		{
			errorMode = 1;
			PRINTF("Break 1 error is %d\r\n", errorMode);
			break;
		}

		Refresh_Watchdog;

		decodeFwPage(OTAData);

		OTAData->batchNum ++; // getting second batch
		FW_Next(Data_Ptr, &otaData);

		int numBatches = ((OTAData->numberOfPages)/5);
		if (OTAData->numberOfPages % 5 != 0)
		{
			numBatches++; // add another batch for the remainder
		}

		for (int retreivedBatch = 2; retreivedBatch <= numBatches; retreivedBatch++)
		{
			getFirmwareDownlink(Data_Ptr, 5);
			Refresh_Watchdog;

			// Assemble batch in order in volatile memory
			if(populateFirmwareArray(OTAData)>1)
			{
				errorMode = 2;
				PRINTF("Break 2 error is %d\r\n", errorMode);
				break;
			}

			if (retreivedBatch < numBatches)
			{
				OTAData->batchNum ++;
				FW_Next(Data_Ptr, &otaData);
			}
		}
		//fakeMissing();
		otaCheckMissingPages(OTAData);
		Refresh_Watchdog;
		// find missing pages, add received pages string and missing pages string, uint16 missing pages

		FW_Acknowledge(Data_Ptr, OTAData);

		//GAV, ONLY UNTIL EVGEN FIXES MISSING PAGES HANDLING OVER 5 ON SERVER
		if (OTAData->numberOfMissingPages > 5)
		{
			errorMode = 5;
			break;
		}
		if (OTAData->numberOfMissingPages != 0)
		{
			OTAData->batchNum = 0;

			numBatches = (((OTAData->numberOfMissingPages)/5)+1);

			if (numBatches == 0)
			{
				numBatches = 1;
			}
			for (int fwPage = 0; fwPage < numBatches; fwPage++)
			{
				getFirmwareDownlink(Data_Ptr, 5);

				if(populateFirmwareArray(OTAData)>1)
				{
					errorMode = 3;
					break;
				}

				if (fwPage < numBatches - 1)
				{
					OTAData->batchNum ++;
					FW_Next(Data_Ptr, &otaData);
				}
			}
		}
		// kcs add the following code
		PRINTF("Break 3 error is %d\r\n", errorMode);
		// if any missing pages found or if error !=0 send crc mismatch
		if (errorMode != 0)
		{
			PRINTF("Error in Receiving Data: Error is %d\r\n", errorMode);
			break;
		}
		if (OTAData->numberOfMissingPages > 0)
		{
			FW_CRC_Ack(Data_Ptr, true, OTAData);  //should be false, true for testing
		}
		else
		{
			PRINTF("In retry loop\r\n");

			//temporary "false" until fwCRCFileCheck is built
			int testing = 0; // turn to any number other than 1 for real loop
			if (testing==1)
			{
				FW_CRC_Ack(Data_Ptr, true, OTAData);
				getFirmwareDownlink(Data_Ptr, 3);
			}
			else
			{
				PRINTF("Decoding Data\r\n");
				calCRC = FWDecodeFile(OTAData, Data_Ptr);
				PRINTF("CRC calc is %d or hex %x\r\n",calCRC,calCRC);
				// if multi-file need to program the file here
				// then erase the flash memory
				//PRINTF("Clear Firmware Array\r\n");
				//clearFirmwareArray();
			}
		}
	} // end of File loop

	// kcs add the following code
	PRINTF("Break 4 error is %d\r\n", errorMode);

	// if error !=0 send crc mismatch
	if (errorMode != 0)
	{
		PRINTF("Error in Receiving Data: Error is %d\r\n", errorMode);
		FW_CRC_Ack(Data_Ptr, false, OTAData);
	}
	else

	{
		// TODO Something should be here for files completed if multi-file
		PRINTF("Break 5 error is %d\r\n", errorMode);

		//check crc, add backoffice CRC to structure
		bool crcIsGood = fwCRCFileCheck(calCRC, OTAData);
		FW_CRC_Ack(Data_Ptr, crcIsGood, OTAData);

		// Save all data
		// Write all data to XPS
		// Moved here to make sure it happens
		saveParamDataToFlash(Data_Ptr);
		// Pseudo code

		// if "OK" firmware downlink
		// Save current Device state to XPS
		// TODO Add Bank number to XPS storage
		// call reflash
		// call Boot_Change

		PRINTF("Reflashing\r\n");

		BANK_TYPE currentBank =  getSwapBank();
		Reflash(Data_Ptr, currentBank);

		PRINTF("Swapping Banks\r\n");
		// Make sure everything wraps up
		HAL_Delay ( 1000 );

		Boot_Change(currentBank);
		// system will reboot in the Boot_Change function
		NVIC_SystemReset();
	}
}

bool fwCRCFileCheck(uint32_t calculated, OTA_FILE_TYPE *OTAData)
{
	bool crcValid = false;

	if(calculated == OTAData->xmitCRC)
	{
		crcValid = true;
	}

	PRINTF("Calculated CRC: %lu\r\n", calculated);
	PRINTF("Transmitted CRC: %lu\r\n", OTAData->xmitCRC);
	PRINTF("CRC Valid Bool: %d\r\n", crcValid);
	return crcValid;
}

void Disable_Extra_Power ( MEM_PTR *Data_Ptr )
{	// Set GPIO off for tied modem power lines

	if ( !( _State & CHARGER_DETECT ))
	{
		cameraPowerControl(false);

		HAL_GPIO_WritePin ( Cell_Enable_GPIO_Port , Cell_Enable_Pin , GPIO_PIN_RESET );
		HAL_GPIO_WritePin ( Cell_DVS_GPIO_Port , Cell_DVS_Pin , GPIO_PIN_RESET );

		HAL_GPIO_WritePin ( Sky_Reset_GPIO_Port , Sky_Reset_Pin , GPIO_PIN_SET );
		HAL_GPIO_WritePin ( Sky_On_Off_Sw_GPIO_Port , Sky_On_Off_Sw_Pin , GPIO_PIN_SET );
		isModemPowered = false;

		XPS_Set_Command ( SLEEP );
		XPS_COMMAND ( Data_Ptr );
	}
}

void Enable_Modem ( MEM_PTR *Data_Ptr )
{
	Refresh_Watchdog;
	//#ifdef SKYWIRE_MODEM
	//	CELL_Set_Command ( CHECK );
	//	CELL_COMMAND ( Data_Ptr );
	//	if (CELL_Get_State () == CELL_OK)
	//		return;
	//#endif //SKYWIRE_MODEM

#ifdef Log_Level_0
	Log_Single ( MODEM_EN_MSG );
#endif //Log_Level_0

	Enable_Modem_PWR ( Data_Ptr );

	CELL_Init ( Data_Ptr );

}

void Enable_Modem_PWR ( MEM_PTR *Data_Ptr )
{
	Refresh_Watchdog;

	if (!isModemPowered)
	{
		//Set GPIO off for tied modem power lines
		//HAL_GPIO_WritePin ( USB_Power_Enable_GPIO_Port , USB_Power_Enable_Pin , GPIO_PIN_SET );
		//HAL_GPIO_WritePin ( USB_Power_Switch_On_GPIO_Port , USB_Power_Switch_On_Pin , GPIO_PIN_SET );
		HAL_GPIO_WritePin ( Cell_Enable_GPIO_Port , Cell_Enable_Pin , GPIO_PIN_SET );
		HAL_GPIO_WritePin ( Cell_DVS_GPIO_Port , Cell_DVS_Pin , GPIO_PIN_SET );

		int count = 0;
		int end = 0;
		do
		{
			if (HAL_I2C_IsDeviceReady ( &hi2c1 , MAX77801_SLAVE_ADDRESS , 20 , 0x0F ) != HAL_OK)
			{
				count ++;
			}
			else
				end = 1;
		}
		while ((count < 20) && (end = 0));
		count = 0;
		Refresh_Watchdog;
		Data_Ptr->Size = 5;
		_Memory [ 0 ] = MAX77801_CNFG_1;
		_Memory [ 1 ] = 0b00000111;
		_Memory [ 2 ] = 0b01110000;
		_Memory [ 3 ] = MAX77801_VOUT_4V;
		_Memory [ 4 ] = MAX77801_VOUT_4V;
		while (HAL_I2C_Master_Transmit ( &hi2c1 , MAX77801_SLAVE_ADDRESS , _Memory , Data_Ptr->Size , 0xFF ) != HAL_OK);

		Refresh_Watchdog;
		while (HAL_I2C_IsDeviceReady ( &hi2c1 , MAX77801_SLAVE_ADDRESS , 15 , 0x0F ) != HAL_OK);

		Refresh_Watchdog;
		HAL_Delay ( 500 );
		//GAV 8-15-24
		//		HAL_GPIO_WritePin ( Sky_Reset_GPIO_Port , Sky_Reset_Pin , GPIO_PIN_SET );
		//		HAL_GPIO_WritePin ( Sky_On_Off_Sw_GPIO_Port , Sky_On_Off_Sw_Pin , GPIO_PIN_SET );
		////	//
		//		HAL_Delay ( 1000 );
		//
		//		HAL_GPIO_WritePin ( Sky_On_Off_Sw_GPIO_Port , Sky_On_Off_Sw_Pin , GPIO_PIN_RESET );
		//
		//HAL_Delay(8000);
		HAL_GPIO_WritePin ( Sky_Reset_GPIO_Port , Sky_Reset_Pin , GPIO_PIN_RESET );
		HAL_GPIO_WritePin ( Sky_On_Off_Sw_GPIO_Port , Sky_On_Off_Sw_Pin , GPIO_PIN_RESET );

		HAL_Delay ( 8000 );
		isModemPowered = true;
	}

#ifdef Log_Level_0
	Log_Single ( MODEM_FN_MSG );
#endif // Log_Level_0
}

void cameraPowerControl (bool isOn)
{

	if (isOn)
	{
		HAL_GPIO_WritePin ( Camera_Enable_GPIO_Port , Camera_Enable_Pin , GPIO_PIN_SET );
		HAL_GPIO_WritePin ( USB_Power_Enable_GPIO_Port , USB_Power_Enable_Pin , GPIO_PIN_SET );
	}
	else
	{
		HAL_GPIO_WritePin ( Camera_Enable_GPIO_Port , Camera_Enable_Pin , GPIO_PIN_RESET );
		HAL_GPIO_WritePin ( USB_Power_Enable_GPIO_Port , USB_Power_Enable_Pin , GPIO_PIN_RESET );

	}
	setCameraPower( isOn );
}

void Enable_GPIO_INT ( void )
{
	HAL_NVIC_EnableIRQ ( EXTI2_IRQn );
	HAL_NVIC_EnableIRQ ( EXTI4_IRQn );
	HAL_NVIC_EnableIRQ ( EXTI5_IRQn );
	HAL_NVIC_EnableIRQ ( EXTI6_IRQn );
	HAL_NVIC_EnableIRQ ( EXTI12_IRQn );
	HAL_NVIC_EnableIRQ ( EXTI2_IRQn );
	HAL_NVIC_EnableIRQ ( EXTI4_IRQn );
	HAL_NVIC_EnableIRQ ( EXTI5_IRQn );
	HAL_NVIC_EnableIRQ ( EXTI6_IRQn );
	HAL_NVIC_EnableIRQ ( EXTI12_IRQn );
}

void Disable_GPIO_INT ( void )
{
	HAL_NVIC_DisableIRQ ( EXTI2_IRQn );
	HAL_NVIC_DisableIRQ ( EXTI4_IRQn );
	HAL_NVIC_DisableIRQ ( EXTI5_IRQn );
	HAL_NVIC_DisableIRQ ( EXTI6_IRQn );
	HAL_NVIC_DisableIRQ ( EXTI12_IRQn );
	HAL_NVIC_DisableIRQ ( EXTI2_IRQn );
	HAL_NVIC_DisableIRQ ( EXTI4_IRQn );
	HAL_NVIC_DisableIRQ ( EXTI5_IRQn );
	HAL_NVIC_DisableIRQ ( EXTI6_IRQn );
	HAL_NVIC_DisableIRQ ( EXTI12_IRQn );
}

/*
 * Encode a buffer into base64 format
 */
int base64_encode ( unsigned char *dst , size_t *dlen , const unsigned char *src , size_t slen )
{
	size_t i, n;
	int C1, C2, C3;
	unsigned char *p;

	if (slen == 0) return (0);

	n = (slen << 3) / 6;

	switch (( slen << 3 ) - ( n * 6 ))
	{
	case 2:
		n += 3;
		break;
	case 4:
		n += 2;
		break;
	default:
		break;
	}

	if ( *dlen < n + 1 )
	{
		*dlen = n + 1;
		return ( POLARSSL_ERR_BASE64_BUFFER_TOO_SMALL );
	}

	n = (slen / 3) * 3;

	for ( i = 0, p = dst; i < n; i += 3 )
	{
		C1 = *src++;
		C2 = *src++;
		C3 = *src++;

		*p++ = base64_enc_map [ (C1 >> 2) & 0x3F ];
		*p++ = base64_enc_map [ (((C1 & 3) << 4) + (C2 >> 4)) & 0x3F ];
		*p++ = base64_enc_map [ (((C2 & 15) << 2) + (C3 >> 6)) & 0x3F ];
		*p++ = base64_enc_map [ C3 & 0x3F ];
	}

	if ( i < slen )
	{
		C1 = *src++;
		C2 = ((i + 1) < slen) ? *src++ : 0;

		*p++ = base64_enc_map [ (C1 >> 2) & 0x3F ];
		*p++ = base64_enc_map [ (((C1 & 3) << 4) + (C2 >> 4)) & 0x3F ];

		if ((i + 1) < slen) *p++ = base64_enc_map [ ((C2 & 15) << 2) & 0x3F ];
		else *p++ = '=';

		*p++ = '=';
	}

	*dlen = p - dst;
	*p = 0;

	return (0);
}

/*
 * Decode a base64-formatted buffer
 */
int base64_decode ( unsigned char *dst , size_t *dlen , const unsigned char *src , size_t slen )
{
	size_t i, n;
	uint32_t j, x;
	unsigned char *p;

	for (i = j = n = 0; i < slen; i++)
	{
		if ((slen - i) >= 2 && src [ i ] == '\r' && src [ i + 1 ] == '\n') continue;

		if (src [ i ] == '\n') continue;

		if (src [ i ] == '=' && ++j > 2) return (POLARSSL_ERR_BASE64_INVALID_CHARACTER);

		if (src [ i ] > 127 || base64_dec_map [ src [ i ] ] == 127) return (POLARSSL_ERR_BASE64_INVALID_CHARACTER);

		if (base64_dec_map [ src [ i ] ] < 64 && j != 0) return (POLARSSL_ERR_BASE64_INVALID_CHARACTER);

		n++;
	}

	if (n == 0) return (0);

	n = ((n * 6) + 7) >> 3;

	if ( *dlen < n )
	{
		*dlen = n;
		return (POLARSSL_ERR_BASE64_BUFFER_TOO_SMALL);
	}

	for (j = 3, n = x = 0, p = dst; i > 0; i--, src++)
	{
		if (*src == '\r' || *src == '\n')
			continue;

		j -= (base64_dec_map [ *src ] == 64);
		x = (x << 6) | (base64_dec_map [ *src ] & 0x3F);

		if (++n == 4)
		{
			n = 0;
			if (j > 0) *p++ = (unsigned char) (x >> 16);
			if (j > 1) *p++ = (unsigned char) (x >> 8);
			if (j > 2) *p++ = (unsigned char) (x);
		}
	}

	*dlen = p - dst;

	return (0);
}

const char base64_chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

//int find_base64_char_index(char c)
//{
//	if (c >= 'A' && c <= 'Z') return c - 'A';
//	if (c >= 'a' && c <= 'z') return c - 'a' + 26;
//	if (c >= '0' && c <= '9') return c - '0' + 52;
//	if (c == '+') return 62;
//	if (c == '/') return 63;
//// KCS comment this out and uncomment the below
//	return -1; // Invalid character
//}

int find_base64_char_index(char c) {
	for (int i = 0; i < 64; i++) {
		if (base64_chars[i] == c) {
			return i;
		}
	}
	return -1;
}

int onTheFlyDcodeBase64(unsigned char *input, unsigned char *output, int sourceSize) {
	int inputCounter = 0;
	int outputCounter = 0;
	int lastGoodInputBuffer = 0;
	int outIter = 0;
	unsigned char input_buffer[4], output_buffer[3];

	while (inputCounter < sourceSize )
	{
		for (int inIterator = 0; inIterator < 4; inIterator++)
		{
			if (input[inputCounter] != '=')
			{
				input_buffer[inIterator] = find_base64_char_index(input[inputCounter++]);
				lastGoodInputBuffer = inIterator;
			}
			else
			{
				input_buffer[inIterator] = 0;
				inputCounter++;
			}
		}

		// Decode the 4 Base64 characters into 3 bytes
		output_buffer[0] = (input_buffer[0] << 2) + ((input_buffer[1] & 0x30) >> 4);
		output_buffer[1] = ((input_buffer[1] & 0x0F) << 4) + ((input_buffer[2] & 0x3C) >> 2);
		output_buffer[2] = ((input_buffer[2] & 0x03) << 6) + input_buffer[3];

		for (outIter = 0; outIter < 3; outIter++)
		{
			if (input[inputCounter - 1] != '=')
			{
				output[outputCounter++] = output_buffer[outIter];
			} else
			{
				for (int i = 0; i < lastGoodInputBuffer; i++)
				{
					// check inIterator, add inIterator -1 bytes
					output[outputCounter++] = output_buffer[i];
				}
				break;
			}
		}
	}
	// for string output terminate with \0
	// output[j] = '\0';
	return outputCounter;
}

/******************************************************
 Memory Clean
 Clears Memory storage space
 ******************************************************/
void Clear_Memory ( MEM_PTR * bigStructure )
{
	int memoryItterator = 0;
	while ( memoryItterator < (MEMORY_MAX))
	{
		bigStructure->Memory [ memoryItterator ] = 0;
		memoryItterator ++;
	}
}

void Clear_Memory2 ( MEM_PTR * bigStructure , uint16_t memoryClearSize)
{
	int memoryItterator = 0;
	while ( memoryItterator < memoryClearSize )
		bigStructure->Memory [ memoryItterator++ ] = 0;
}

/******************************************************
 Clears OTA Flash Image Array
 ******************************************************/
void clearOTAFlashArray(MEM_PTR *bigData)
{
	int memoryItterator = 0;
	while ( memoryItterator < (OTA_FLASH_SIZE))
		bigData->Flash.Image [ memoryItterator++ ] = 0;
}

/******************************************************
 Word Transfer Function
 // Warning only works for null terminated memory values (Like strings)
 Copies the source word to target, will look for
 NULL in source to stop coping to target
 ******************************************************/
void Word_Transfer ( char *target , char *source )
{
	while ( *(source) )
	{
		*(target)++ = *(source)++;
	}
	*(target)++ = *(source)++;	// last loop to carry over NULL
}

/******************************************************
 Character Replace Function
 Iterates through Char array to find source 1,
 after which it replaces it with source 2
 ******************************************************/
void Char_Replace ( char *target , char *source1 , char *source2 )
{
	do
	{
		if (*(target) == *(source1))
		{
			*(target) = *(source2);
		}
	}
	while ( *(target)++ );
}

/******************************************************
 Error Test
 Test for ERROR string
 ******************************************************/
bool IsError ( MEM_PTR *Data_Ptr )
{
	_Pointer = _Memory;
	do
	{
		if (*(_Pointer) == 'E' && *(_Pointer + 1) == 'R' && *(_Pointer + 2) == 'R' && *(_Pointer + 3) == 'O' && *(_Pointer + 4) == 'R')
		{
			return true;
		}
		(_Pointer)++;
	}
	while ( *(_Pointer) );

	return false;
}

bool IsError2 ( MEM_PTR *Data_Ptr )
{
	char gpsValidTest[] = "+QGPSLOC:";
	char* gpsValidStr = strstr((char*)_Memory, gpsValidTest);

	if(gpsValidStr)
	{
		PRINTF("Response Validated, Status code is 1\r\n");
		return false;
	}
	else
	{
		PRINTF("Response Error, Status code is 0\r\n");
		return true;
	}

}
/******************************************************
 check for received buffer contents
 ******************************************************/
bool isBufferPopulated ( MEM_PTR *Data_Ptr )
{
	// KCS fix this hard coded value
	char checkBuff [1000] = "";
	int counter = 0;
	for ( counter = 0; counter < 280; counter++ )   //GAv hardcoded to 40, need to be dynamic
	{
		checkBuff[counter]= _Memory[counter];
	}

	char target1[] = "OK";
	char target2[] = "ERROR";
	char target3[] = "+QMTRECV:";
	char *p = strstr( checkBuff, target1 );
	char *q = strstr( checkBuff, target2 );
	char *v = strstr( checkBuff, target3 );
	if (p)
	{
		vcom_Trace ( (uint8_t*) p , 280 , 280 );   //GAv hardcoded fix
		return false;
	}
	else if (q)
	{
		vcom_Trace ( (uint8_t*) q , 280 , 280 );   //GAv hardcoded fix
		return false;
	}
	else if ( checkBuff[8] == 0 )   //GAv fix this, hardcoded
	{
		return false;
	}
	else if ( v )   //GAv fix this, hardcoded
	{
		//		PRINTF("%s/r/n", checkBuff);
		return true;
	}
	else
	{
		return true;
	}

	//check memory for "OK" if OK return false, if it's populated with something return true GAV
}


bool isFivePacketsReceived( char* ptr)
{
	// KCS fix these hard coded values
	char Buff[280] = {""};
	char *RecvBuff[280] = {""};
	//PRINTF("Line 1531\r\n");
	Word_Transfer(Buff, ptr);
	RecvBuff[0] = Buff;  // this maybe the problem; have you tested this function fully with garbage inputs and miscelleneous data?

	//PRINTF("Line 1535\r\n");

	int msgTokenCount = 0;
	char *msgTokens[7] = {""}; // needs to be at least 7 if you are trying to access the seventh one
	const char *colDelim = ":";
	const char *comDelim = ",";

	char *msgToken = strtok_r(*RecvBuff, colDelim, RecvBuff);

	//PRINTF("Line 1543\r\n");
	while (msgToken != NULL && msgTokenCount < 7)
	{
		msgTokens[msgTokenCount] = msgToken;
		msgTokenCount++;

		//PRINTF("Line 1549\r\n");
		msgToken = strtok_r(NULL, comDelim, RecvBuff);
	}

	//PRINTF("Line 1553\r\n");
	// this is the offending line for the first command 40 error when running the "Released" code
	// KCS suggest looking at the input string "ptr" and seeing if it is "good" data
	// before processing

	//csc added to return false if not enough tokens were found.
	if(msgTokenCount < 7)
	{
		PRINTF("Not enough tokens, returning false.\r\n");
		return false;
	}
	char *msg5 = msgTokens[6];  // does not exist when defined as an array of 6; indexes are "0" based

	//char test1[] = "1";

	char *_msg5 = strstr( msg5, "1" );
	//PRINTF("Line 1559\r\n");

	if (_msg5 != NULL && strstr( msg5, "1" ) != NULL)
	{
		return true;
	}
	else return false;
}

/******************************************************
 Cell Command Creation Function
 Creates command with input function as well as
 parameterizes values for command
 ******************************************************/
void Build_MSG ( MEM_PTR *Data_Ptr , char *MSG )
{
	if ( *(MSG) == '\1' )
		//GAV potentially add a watchdog refresh
	{
		buf = 0;
		MSG++;
	}

	if ( buf >= 0 )
	{	//Data_Ptr->Shift = 0;
		while ( *(MSG) != '\0' )
		{
			_Memory [ buf ] = *(MSG)++;
			buf += 1;
		}
	}
}

#ifdef Log

/******************************************************
 Logger Initialize
 Logger constructor
 ******************************************************/
void LOG_Init ( void )
{
	Logger.Debug_Val = 0;
	for ( Logger.Buffer = Log_Size_Limit; Logger.Buffer != 0; --Logger.Buffer )
	{
		(Logger.Curr) = Logger.List [ (Logger.Curr).MSG_Size ];

		for ((Logger.Curr).MSG_Size = Log_MSG_SIZE - 1; (Logger.Curr).MSG_Size != 0; --(Logger.Curr).MSG_Size)
		{
			(Logger.Curr).MSG [ (Logger.Curr).MSG_Size ] = 0;
		}
	}
}

void Log_Single ( char *MSG )
{
	Write_Log ( MSG );

#ifdef UART_USB
	// KCS fix this when implementing logging
	Read_Last_UART_Log ( &memory );
#endif //UART_USB
}

void Log_Insert ( char *MSG , uint32_t Number )
{
	char Command_Buffer [ 10 ];

	Write_Log ( MSG );
	Write_Log ( utoa ( Number , Command_Buffer , DECIMAL ) );
}

void Log_End ( char *MSG , uint32_t Number )
{
	Log_Insert ( MSG , Number );
	Write_Log ( LOG_TAIL );

#ifdef UART_USB
	// KCS fix this when implementing logging
	Read_Last_UART_Log ( &memory );
#endif ///UART_USB
}

/******************************************************
 Write state of device to UART buffer
 Note: Will save state of all control flags in ram
 ******************************************************/
void Write_Log ( char *MSG )
{
	if (*(MSG) == '\1')
	{
		Logger.Buffer++;
		Logger.Debug_Val++;

		if (Logger.Buffer == Log_Size_Limit)
		{
			Logger.Buffer = 0;
		}

		Logger.Curr = Logger.List [ Logger.Buffer ];

		for ((Logger.Curr).MSG_Size = Log_MSG_SIZE - 1; (Logger.Curr).MSG_Size != 0; --(Logger.Curr).MSG_Size)
		{
			(Logger.Curr).MSG [ (Logger.Curr).MSG_Size ] = 0;
		}

		Time_Stamp ( Logger.Curr.MSG );

		while ((Logger.Curr).MSG [ ++(Logger.Curr).MSG_Size ])
			;
		(Logger.Curr).MSG_Size--;

		(Logger.Curr).MSG [ ++(Logger.Curr).MSG_Size ] = ':';

		MSG++;
	}

	do
	{
		(Logger.Curr).MSG [ (Logger.Curr).MSG_Size++ ] = *(MSG)++;
	}
	while (*(MSG));

	Logger.List [ Logger.Buffer ] = Logger.Curr;
}

#ifdef UART_USB
/******************************************************
 Read All logs saved on UART buffer
 Note: Will read all logs to UART
 ******************************************************/
void Read_All_UART_Log ( void )
{
	HAL_UART_AbortReceive ( &huart2 );
	for (int i = 0; i != Log_Size_Limit; i++)
	{
		HAL_UART_Transmit ( &huart2 , (uint8_t*) Logger.Curr.MSG , Logger.Curr.MSG_Size , 0xFF );
		Logger.Curr = Logger.List [ Logger.Buffer++ ];
		if (Logger.Buffer == Log_Size_Limit)
		{
			Logger.Buffer = 0;
		}
	}
}

/******************************************************
 Read last log saved on UART buffer
 Note: Will read last log to UART
 ******************************************************/
void Read_Last_UART_Log ( MEM_PTR *Data_Ptr )
{
	if (!(_Setting & AVOID_MSG_SPAM))
	{
		HAL_UART_AbortReceive ( &huart2 );
		HAL_UART_Transmit ( &huart2 , (uint8_t*) Logger.Curr.MSG , Logger.Curr.MSG_Size , 0xFF );
		HAL_UART_Transmit ( &huart2 , (uint8_t*) "\r\n" , 2 , 0xFF );
	}
}
#endif  //UART_USB


// KCS block = 0 does not work; we need to check DMA settings in IOC file
void vcom_Trace ( uint8_t *p_data , uint16_t size , int block )
{
	if (block) HAL_UART_Transmit ( &huart2 , p_data , size , 1000 );
	else HAL_UART_Transmit_DMA ( &huart2 , p_data , size );
}

// This function takes a string formatted variable array and prints it to the output UART line
int8_t TraceSend(int8_t block, const char *strFormat, ...)
{
	char PRINTFbuf[PRINTBUFSIZE];
	va_list vaArgs;
	uint16_t bufSize = 0;
	int status = 0;

	BACKUP_PRIMASK();

	DISABLE_IRQ(); /**< Disable all interrupts by setting PRIMASK bit on Cortex*/

	// kcs we should add the timestamp here also in the front, but we need to change the function to return the number of bytes added.
	// for now we can just put the function guts here

	RTC_DateTypeDef sdatestructureget;
	RTC_TimeTypeDef stimestructureget;

	/* Get the RTC Curr Time */
	HAL_RTC_GetTime ( &hrtc , &stimestructureget , RTC_FORMAT_BIN );
	/* Get the RTC Curr Date */
	HAL_RTC_GetDate ( &hrtc , &sdatestructureget , RTC_FORMAT_BIN );

	/* Display time Format : mm_dd_yy__hh_mm_ss */
	// KCS I took out the log level value at the end of the timestamp
	bufSize += snprintf( PRINTFbuf + bufSize, PRINTBUFSIZE - bufSize, "%.2d_%.2d_%.2d__%.2d_%.2d_%.2lu:",
			sdatestructureget.Month , sdatestructureget.Date ,
			2000 + sdatestructureget.Year , stimestructureget.Hours , stimestructureget.Minutes ,
			stimestructureget.Seconds);


	va_start( vaArgs, strFormat);
	bufSize += vsnprintf(PRINTFbuf + bufSize, PRINTBUFSIZE - bufSize, strFormat, vaArgs);
	va_end(vaArgs);

	if ( status == 0 )
	{
		if (!block)
		{
			RESTORE_PRIMASK();
			vcom_Trace((uint8_t*)PRINTFbuf, bufSize, 0);
		}else
		{
			RESTORE_PRIMASK();
			vcom_Trace((uint8_t*)PRINTFbuf, bufSize, 1);
		}
	}
	else
	{
		RESTORE_PRIMASK();
	}

	return status;
}

// KCS in future, change name to "getOtaFileInfo"
bool getOTAFileInfo ( OTA_FILE_TYPE *OTAData, uint8_t index )
{
	bool fwAvail = false;
	char Buff[MEMORY_MAX] = { " " };
	char test1[] = "firmware_available";

	Word_Transfer(Buff, (char*)downLinkPackets.mQTTMessage[index]);

	// Check if firmware_available
	char *msg_ptr = strstr(Buff, test1);
	if (msg_ptr)
	{
		// Find file name
		char *versionStr = strstr(Buff, "\"version\":");
		if (versionStr)
		{
			versionStr += strlen("\"version\":\"");
			int versionLength = strcspn(versionStr, "\"");
			if (versionLength >49) //Protect string from overflow
			{ versionLength = 49;}
			strncpy(OTAData->otaFileName, versionStr, versionLength);
			OTAData->otaFileName[versionLength] = '\0'; //
			OTAData->fileNameLength = versionLength;
		}

		// Find total_files
		char *totalFiles_str = strstr(Buff, "\"total_files\":");
		if (totalFiles_str)
		{
			totalFiles_str += strlen("\"total_files\":");
			int totalFiles_length = strcspn(totalFiles_str, "}");
			char totalFilesString[10] = "";
			strncpy(totalFilesString, totalFiles_str, totalFiles_length);
			OTAData->numberOfFiles = (uint8_t)atoi(totalFilesString);
		}

		fwAvail =  true; // Firmware available
	}
	return fwAvail;
}

void decodeFwPage (OTA_FILE_TYPE *OTAData )
{
	int msgCount = 0;
	char Buff[MEMORY_MAX];
	char test1[] = "firmware_v0";

	do
	{
		//csc added watchdog for debug
		Refresh_Watchdog;
		// KCS psudo code
		// 1. Check packet enumeration
		// 2. If not "FIRMWARE" mark as done and move on
		//    optionally in future store the message in a "to be done in future" container
		// 3. Do the following decode

		// Transfer MQTT message to a local buffer

		Word_Transfer(Buff, (char *)downLinkPackets.mQTTMessage[msgCount]);

		// Check for firmware version
		char *version_str = strstr(Buff, test1);
		if (version_str)
		{
			// Find page
			char *page_str = strstr(Buff, "\"total_pages\":");
			if (page_str)
			{
				page_str += strlen("\"total_pages\":");
				int page_length = strcspn(page_str, ",");
				strncpy(OTAData->numofPagesString, page_str, page_length);
				OTAData->numberOfPages = atoi(OTAData->numofPagesString);
			}

			// Find file number
			char *file_str = strstr(Buff, "\"file\":");
			if (file_str)
			{
				file_str += strlen("\"file\":");
				int file_length = strcspn(file_str, ",");
				strncpy(OTAData->fileNumberString, file_str, file_length);
			}

			// Find crc
			char *crc_str = strstr(Buff, "\"crc\":");
			if (crc_str)
			{
				crc_str += strlen("\"crc\":");
				int crc_length = strcspn(crc_str, ",");
				char xcrcString[10] = "";
				strncpy(xcrcString, crc_str, crc_length);
				OTAData->xmitCRC = strtol(xcrcString, NULL, 10);
			}

			break; // Exit the loop as we've successfully extracted data
		}

		msgCount++;

	} while (msgCount < 6);
}

bool checkOTAReady (MEM_PTR *Data_Ptr)   //needs work! not done!
{
	char Signalchar[] = {0};
	bool isPluggedIn = Is_Pin_Low( nExt_Power_GPIO_Port , nExt_Power_Pin );
	CELL_Set_Command ( SIGNAL );
	CELL_COMMAND ( Data_Ptr );
	Char_Replace((char*) _Memory, "\n", " ");
	Char_Replace((char*) _Memory, "\r", " ");

	Word_Transfer((char*) Signalchar, (char*) _Memory);
	uint8_t Signalint = atoi(Signalchar);

	if (isPluggedIn && (Signalint > 15))
	{
		return true;
	}
	else return false;

}

#ifdef XPS_Logs
/******************************************************
		Write state of device
	Note: Will save state of all control flags in ram
 ******************************************************/
void Write_Log(void)
{	Clear_Memory(Data_Ptr);

XPS_IsBusy(Data_Ptr, &XPS);

Data_Ptr->Size = 1;
Data_Ptr->Page = Log_Page;
Data_Ptr->Shift = 0;
XPS.COMMAND = READ;
XPS_COMMAND(Data_Ptr,&XPS);

// _Memory[0] += Log_MAX_Size;
{_Memory[0] = Log_MIN_Shift;}
else if(_Memory[0] == 0)
{_Memory[0]++;}

_Memory[Log_MAX_Shift + 1] = _Memory[0];

#ifdef EXTERNAL_XPS_STORAGE
//		bool BUSY,  							// Erase/Write in progress
//			 WEL,								// Write enable latch (1 - write allowed)
//			 QE,								// Quad SPI mode
//			 SUS, 								// Suspend Status
//			 ADS, 								// Curr addr mode (0-3 byte / 1-4 byte)
//			 ADP, 								// Power-up addr mode
//			 SLEEP; 							// Sleep Status
//		uint8_t TYPE,							// type command for control
//				COMMAND;						// Configures command for control
//		XPS_STATE STATE;						// Curr state of XPS IC

_Memory[0] = 	0xEE;
_Memory[1] = 	XPS.STATE;
_Memory[2] = 	XPS.COMMAND;
_Memory[3] = 	XPS.TYPE;
#else
_Memory[0] = 	0xEE;
_Memory[1] = 	0xFF;
_Memory[2] = 	0xFF;
_Memory[3] = 	0xFF;
#endif  //EXTERNAL_XPS_STORAGE

#ifdef SKYWIRE_MODEM
//		char 	MQTT_DEVID[30],				// MSG ID
//				MSGID[30],					// MSG ID
//				USERNAME[10],				// Server Username
//				PASSWORD[10],				// Server Password
//				SIZE[4],					// For package defenitions
//				EVENT[30];					// For package building
//		uint8_t COMMAND,					// Control Buffer
//				Instruction[UART_MAX_FAIL],	// Array Control Buffer
//				*Temp,						// For Array Control
//				FAIL_CNT;					// Fail Counter
//		uint32_t TIMER;						// Sets wait time on UART
//		CELL_STATE STATE;					// Curr state of Cell Module

_Memory[4] = 	0xEE;
_Memory[5] = 	CELL_Get_State ();
_Memory[6] = 	CELL.COMMAND;
_Memory[7] = 	CELL.FAIL_CNT;
#else
_Memory[4] = 	0xEE;
_Memory[5] = 	0xFF;
_Memory[6] = 	0xFF;
_Memory[7] = 	0xFF;
#endif  // SKYWIRE_MODEM

#ifdef TEMPERATURE_SENSOR
//		uint8_t COMMAND;					// Control Buffer
//		uint16_t ADDRESS,					// Control Buffer
//				 Size;
//		uint32_t TIMER;
//		TEM_STATE STATE;					// Curr state of TEM Module

_Memory[8] = 	0xEE;
_Memory[9] = 	TEM_Get_State;
_Memory[10] = Data_Ptr->Commands.Temperature;
_Memory[11] = TEM.ADDRESS;
#else
_Memory[8] = 	0xEE;
_Memory[9] = 	0xFF;
_Memory[10] = 0xFF;
_Memory[11] = 0xFF;
#endif  // TEMPERATURE_SENSOR

#ifdef ACCELERATION_SENSOR
//		uint8_t COMMAND;					// Control Buffer
//		uint16_t ADDRESS,					// Control Buffer
//				 Size;
//		uint32_t TIMER;
//		ACC_STATE STATE;					// Curr state of TEM Module

_Memory[12] = 0xEE;
_Memory[13] = ACC.STATE;
_Memory[14] = ACC.COMMAND;
_Memory[15] = ACC.ADDRESS;
#else
_Memory[12] = 0xEE;
_Memory[13] = 0xFF;
_Memory[14] = 0xFF;
_Memory[15] = 0xFF;
#endif  //ACCELERATION_SENSOR

XPS_IsBusy(Data_Ptr, &XPS);

XPS.COMMAND = ERASE;
XPS_COMMAND(Data_Ptr, &XPS);

Data_Ptr->Shift = _Memory[Log_MAX_Shift + 1];
Data_Ptr->Size = Log_MAX_Size;
XPS.COMMAND = WRITE;
XPS_COMMAND(Data_Ptr,&XPS);

XPS_IsBusy(Data_Ptr, &XPS);

_Memory[0] = _Memory[Log_MAX_Shift + 1] + Log_MAX_Size;
Data_Ptr->Page = Log_Page;
Data_Ptr->Shift = 0;
Data_Ptr->Size = 1;
XPS_COMMAND(Data_Ptr,&XPS);
}

/******************************************************
		Read Previous State in Ram
	Note: Will read state of all control flags in ram
 ******************************************************/
void Read_Last_Log(void)
{	Clear_Memory(Data_Ptr);

XPS_IsBusy(Data_Ptr, &XPS);

Data_Ptr->Size = 1;
Data_Ptr->Page = Log_Page;
Data_Ptr->Shift = 0;
XPS.COMMAND = READ;
XPS_COMMAND(Data_Ptr,&XPS);

if (_Memory[0] >= 0)
{Data_Ptr->Shift = _Memory[0] - Log_MAX_Size;}
else
{Data_Ptr->Shift = Log_MAX_Shift - Log_MAX_Size;}

XPS_IsBusy(Data_Ptr, &XPS);

Data_Ptr->Size = Log_MAX_Size;
Data_Ptr->Page = Log_Page;

XPS_COMMAND(Data_Ptr,&XPS);
}

/******************************************************
		Initial Update Function
	Note: Will read state of all control flags in ram
 ******************************************************/
void Read_All_Log(void)
{	Clear_Memory(Data_Ptr);

Data_Ptr->Size = 1;
Data_Ptr->Page = Log_Page;
Data_Ptr->Shift = 0;
XPS.COMMAND = READ;
XPS_COMMAND(Data_Ptr,&XPS);

_Memory[Log_MAX_Shift + 1] = _Memory[0];
buf = 0;

Data_Ptr->Size = Log_MAX_Size;
while (	buf != _Memory[Log_MAX_Shift + 1])
{	Data_Ptr->Shift = (buf * Log_MAX_Size) + 1;
XPS_COMMAND(Data_Ptr,&XPS);
buf ++;
}
}

#endif  //XPS_Logs

#endif  // Log

#ifdef SKYWIRE_MODEM
/******************************************************
 Cell Hardware Reset
 Reconfigures port settings and restarts device
 from initial parameters
 ******************************************************/
void CELL_HARDWARE_RESET ( MEM_PTR *Data_Ptr )
{
	HAL_UART_DeInit ( &hlpuart1 );
	//	MX_LPUART1_UART_Init();
	Clear_Memory ( Data_Ptr );

	HAL_Delay ( 500 ); //cell warmup when implemented
	Refresh_Watchdog;
}
#endif  //SKYWIRE_MODEM

#ifdef EXTERNAL_XPS_STORAGE
/******************************************************
 XPS Reset
 Reconfigures port settings and restarts device
 from initial parameters
 ******************************************************/
void XPS_HARDWARE_RESET ( MEM_PTR *Data_Ptr )
{
	PACKAGE_Init ( Data_Ptr );
	HAL_OSPI_DeInit ( &hospi1 );
	//	MX_OCTOSPI1_Init();
	XPS_initialize ( Data_Ptr );
	Clear_Memory ( Data_Ptr );
}
#endif //end of EXTERNAL_XPS_STORAGE

void Time_Stamp ( char *MSG )
{
	int TsLength = 0;
	RTC_DateTypeDef sdatestructureget;
	RTC_TimeTypeDef stimestructureget;

	/* Get the RTC Curr Time */
	HAL_RTC_GetTime ( &hrtc , &stimestructureget , RTC_FORMAT_BIN );
	/* Get the RTC Curr Date */
	HAL_RTC_GetDate ( &hrtc , &sdatestructureget , RTC_FORMAT_BIN );

	uint32_t msec = 1000 * (stimestructureget.SecondFraction - stimestructureget.SubSeconds) / (stimestructureget.SecondFraction + 1);


	// Display time Format : mm_dd_yyyy__hh_mm_ss-fff
	// Connor, the Logger.Debug_Val will overflow this message eventually, need to change it to subseconds like ISO
	TsLength = snprintf ( MSG ,LOG_TIMESTAMP_LENGTH, "%02d_%02d_%02d__%02d_%02d_%02lu-%03lu" ,
			sdatestructureget.Month ,
			sdatestructureget.Date ,
			2000 + sdatestructureget.Year ,
			stimestructureget.Hours ,
			stimestructureget.Minutes ,
			stimestructureget.Seconds ,
			msec );

	if((TsLength < 1)|| TsLength > LOG_TIMESTAMP_LENGTH)
	{
		PRINTF("Time_Stamp function returned a bad string\r\n");
	}
}

void Time_StampISO( char *MSG )
{
	int TsLength = 0;
	RTC_DateTypeDef sdatestructureget;
	RTC_TimeTypeDef stimestructureget;

	/* Get the RTC Curr Time */
	HAL_RTC_GetTime ( &hrtc , &stimestructureget , RTC_FORMAT_BIN );
	/* Get the RTC Curr Date */
	HAL_RTC_GetDate ( &hrtc , &sdatestructureget , RTC_FORMAT_BIN );
	// Display time Format : YYYY-MM-DDThh:mm:ss.fffZ
	// "2025-02-25T03:54:12.255Z"

	// found on internet
	uint32_t msec = 1000 * (stimestructureget.SecondFraction - stimestructureget.SubSeconds) / (stimestructureget.SecondFraction + 1);

	TsLength = snprintf ( MSG ,ISO_TIMESTAMP_LENGTH, "%04d-%02d-%02dT%02d:%02d:%02lu.%03luZ",
			2000 + sdatestructureget.Year ,
			sdatestructureget.Month ,
			sdatestructureget.Date ,
			stimestructureget.Hours ,
			stimestructureget.Minutes ,
			stimestructureget.Seconds ,
			msec);

	if((TsLength < 1)|| TsLength > ISO_TIMESTAMP_LENGTH)
	{
		PRINTF("Time_StampISO function returned a bad string\r\n");
	}

}

bool calculateStopTime ( TIME_DATE_TYPE *desiredStopTime, uint16_t timeDelay)
{
	bool isError = true;
	TIME_DATE_TYPE currentSysDateTime;

	HAL_StatusTypeDef halError = HAL_RTC_GetTime ( &hrtc , &currentSysDateTime.rtcTime , RTC_FORMAT_BIN );
	// HAL_RTC_GetTime always returns HAL_OK
	// So GetDate is always called in the present version
	if (halError == HAL_OK)
	{
		halError = HAL_RTC_GetDate ( &hrtc , &currentSysDateTime.rtcDate , RTC_FORMAT_BIN );
		if (halError == HAL_OK)
		{
			isError = false;
			uint16_t numOfDays = 31;

			if (currentSysDateTime.rtcDate.Month == RTC_MONTH_JANUARY || currentSysDateTime.rtcDate.Month ==RTC_MONTH_MARCH || currentSysDateTime.rtcDate.Month == RTC_MONTH_MAY || currentSysDateTime.rtcDate.Month == RTC_MONTH_JULY
					|| currentSysDateTime.rtcDate.Month == RTC_MONTH_AUGUST || currentSysDateTime.rtcDate.Month == RTC_MONTH_OCTOBER || currentSysDateTime.rtcDate.Month == RTC_MONTH_DECEMBER)
			{
				numOfDays = 31;
			}
			else if (currentSysDateTime.rtcDate.Month == RTC_MONTH_FEBRUARY)
			{
				if((currentSysDateTime.rtcDate.Year % 4) == 0)   //CSC leap year
					numOfDays = 29;
				else
					numOfDays = 28;
			}
			else if (currentSysDateTime.rtcDate.Month == RTC_MONTH_APRIL || currentSysDateTime.rtcDate.Month == RTC_MONTH_JUNE || currentSysDateTime.rtcDate.Month == RTC_MONTH_SEPTEMBER || currentSysDateTime.rtcDate.Month == RTC_MONTH_NOVEMBER)
			{
				numOfDays = 30;
			}

			if ((currentSysDateTime.rtcTime.Seconds + timeDelay)> 59)
			{
				desiredStopTime->rtcTime.Minutes = ((currentSysDateTime.rtcTime.Minutes) + 1);
				desiredStopTime->rtcTime.Seconds = (currentSysDateTime.rtcTime.Seconds + timeDelay) - 60;
				desiredStopTime->rtcTime.Hours = currentSysDateTime.rtcTime.Hours;
			}
			else
			{
				desiredStopTime->rtcTime.Seconds = currentSysDateTime.rtcTime.Seconds + timeDelay;
				desiredStopTime->rtcTime.Minutes = currentSysDateTime.rtcTime.Minutes;
				desiredStopTime->rtcTime.Hours = currentSysDateTime.rtcTime.Hours;

			}

			if (desiredStopTime->rtcTime.Minutes > 59)
			{
				desiredStopTime->rtcTime.Hours = (currentSysDateTime.rtcTime.Hours +1);
				desiredStopTime->rtcTime.Minutes = (currentSysDateTime.rtcTime.Minutes - 60);
			}
			else
			{
				desiredStopTime->rtcTime.Minutes = desiredStopTime->rtcTime.Minutes;
			}

			//error may be below in the next 35 lines

			if (desiredStopTime->rtcTime.Hours > 23)
			{
				desiredStopTime->rtcTime.Hours = (desiredStopTime->rtcTime.Hours - 24);
				desiredStopTime->rtcDate.Date = currentSysDateTime.rtcDate.Date +1;
				desiredStopTime->rtcDate.Month = currentSysDateTime.rtcDate.Month;
				desiredStopTime->rtcDate.Year = currentSysDateTime.rtcDate.Year;

			}
			else
			{
				desiredStopTime->rtcTime.Hours = desiredStopTime->rtcTime.Hours;
				desiredStopTime->rtcDate.Month = currentSysDateTime.rtcDate.Month;
				desiredStopTime->rtcDate.Year = currentSysDateTime.rtcDate.Year;
			}
			if (desiredStopTime->rtcDate.Date > numOfDays)
			{
				desiredStopTime->rtcDate.Month = (desiredStopTime->rtcDate.Month + 1);
				desiredStopTime->rtcDate.Date = 1;
			}
			else
			{
				desiredStopTime->rtcDate.Date = desiredStopTime->rtcDate.Date;
			}
			if (desiredStopTime->rtcDate.Month > 12)
			{
				desiredStopTime->rtcDate.Month = RTC_MONTH_JANUARY;
				desiredStopTime->rtcDate.Year = desiredStopTime->rtcDate.Year + 1;
			}
			else
			{
				desiredStopTime->rtcDate.Year = desiredStopTime->rtcDate.Year;
			}
		}
	}
	else
	{
		// unlocks RTC shadow register if HAL_Error is ever reached
		HAL_RTC_GetDate ( &hrtc , &currentSysDateTime.rtcDate , RTC_FORMAT_BIN );
	}
	return desiredStopTime;
	return isError;
}

bool isTimeExpired (TIME_DATE_TYPE *endTime)
{
	//need to implement functionality for data
	TIME_DATE_TYPE currentSysDateTime;

	HAL_RTC_GetTime ( &hrtc , &currentSysDateTime.rtcTime , RTC_FORMAT_BIN );
	HAL_RTC_GetDate ( &hrtc , &currentSysDateTime.rtcDate , RTC_FORMAT_BIN );

	uint32_t currtimesec = ((currentSysDateTime.rtcTime.Hours*3600)+(currentSysDateTime.rtcTime.Minutes*60)+(currentSysDateTime.rtcTime.Seconds));
	uint32_t currdatesec = 86400 * ((currentSysDateTime.rtcDate.Year * 365) + (currentSysDateTime.rtcDate.Month * 30) + (currentSysDateTime.rtcDate.Date));
	uint32_t endtimesec = ((endTime->rtcTime.Hours*3600)+(endTime->rtcTime.Minutes*60)+(endTime->rtcTime.Seconds));
	uint32_t enddatesec = 86400 * ((endTime->rtcDate.Year * 365) + (endTime->rtcDate.Month * 30) + (endTime->rtcDate.Date));


	if((currtimesec + currdatesec) < (endtimesec + enddatesec))
	{
		return true;
	}
	else return false;
}

// Bank 2 function
//Store device state to XPS
//Store Firmware Downlink enabled flag to XPS
//Swap Instructions set to Bank 1
void bank2HandleFWDownlink(MEM_PTR *Data_Ptr, OTA_FILE_TYPE *OtaData)
{
	PRINTF("Handle Downlink in Bank 2\r\n" );
	// Save OTA data needed
	setOTAfwAvailable(true);
	setOtaData(OtaData);
	PRINTF("Saving Parameter Data to XPS\r\n" );
	saveParamDataToFlash(Data_Ptr);
	PRINTF("Booting back to Bank 1\r\n" );
	// Make sure everything wraps up
	HAL_Delay ( 1000 );
	Boot_Change(BANK2);  // current bank is bank2
}

void testFunction(MEM_PTR *Data_Ptr)
{
	// set up OTA structure
	OTA_FILE_TYPE testOTAdata;
	testOTAdata.numberOfPages = 3;
	// set up Data_Ptr structure
	// set up firmware packets
	setupTestPackages(&testOTAdata);
	// call FWDecodeFile
	uint32_t calCRC = FWDecodeFile(&testOTAdata, Data_Ptr);
}

void testMqtt(void)
{
	// set up OTA structure
	OTA_FILE_TYPE testOTAdata;
	testOTAdata.numberOfPages = 1;
	Refresh_Watchdog;
	// set up firmware packets
	setupTestMqtt();
	// call populateFirmwareArray
	//populateFirmwareArray(&testOTAdata);
	Refresh_Watchdog;
	decodeFwPage(&testOTAdata);

}

bool isWaitingforOTA(MEM_PTR *Data_Ptr)
{
	// temporary KCS
	readParamDataFromFlash(Data_Ptr);
	return getOTAfwAvailable();
}

//the OTA stuff in Update state has been moved into this function and this functions has replaced the stuff in Update_State
bool handleDownlinks(MEM_PTR *Data_Ptr)
{
	getFirmwareDownlink(Data_Ptr,3);

	//look and see if there is a firmware available
	// at present loop through Mqtt data
	// in future call a new function handleOTADownlink
	bool isAvail = false;
	Refresh_Watchdog;
	for(int indx = 0; indx < 5; indx++ )
	{
		Refresh_Watchdog;
		isAvail = getOTAFileInfo(&otaData,indx);
		if (isAvail == true)
			break;
	}

	if (isAvail)
	{
		Data_Ptr->Flash.currentFlashBank = getSwapBank();
		PRINTF("Current Program Bank is %d\r\n",(uint8_t)getSwapBank() );
		// kcs OTA new strategy
		// if operating Bank is Bank 1 then

		if (BANK1== Data_Ptr->Flash.currentFlashBank)
		{
			FW_Ack_Pending(Data_Ptr, &otaData);
			_State |= GET_FW_UPDATE;
		}
		else   // else  call bank2 function
		{
			bank2HandleFWDownlink(Data_Ptr, &otaData);
		}
	}
	return isAvail;
}

void startOTAProcess(MEM_PTR *Data_Ptr)
{
	Refresh_Watchdog;

	Data_Ptr->Flash.currentFlashBank = getSwapBank();
	PRINTF("Current Program Bank is %d\r\n",(uint8_t)getSwapBank() );
	// kcs OTA new strategy
	// if operating Bank is Bank 1 then

	if (BANK1== Data_Ptr->Flash.currentFlashBank)
	{
		FW_Ack_Pending(Data_Ptr, &otaData);
		FW_Ack_Ready(Data_Ptr, &otaData);
		OTAProcess(Data_Ptr, &otaData);
	}
	else   // else  call bank2 function
	{
		bank2HandleFWDownlink(Data_Ptr, &otaData);
	}
}

void PrintDataArray()
{
	if(SENSOR_ARRAY_SIZE != 0)
	{
		for(int i = 0; i < SENSOR_ARRAY_SIZE; i++ )
		{
			PRINTF("%s\r\n", Sensor_Array[i]);
		}
	}
	else
	{
		PRINTF("Sensor_Array is empty.\r\n");
	}
}

//csc writes data to array and prepends the date
void WriteDataArray(DATA_TYPE_TAG tag, char *dataStr, ...)
{
	va_list argList;
	char datastr[SENSOR_SIZE]; //buffer to hold final string
	char timebuff[SENSOR_SIZE]; //buffer for time
	char argbuff[SENSOR_SIZE]; //buffer for arguments

	memset(datastr, 0, SENSOR_SIZE); //clear buffer
	memset(timebuff, 0, SENSOR_SIZE); //clear buffer
	memset(argbuff, 0, SENSOR_SIZE); //clear buffer

	switch(tag)
	{
	case ACC_TAG:
		strcpy(datastr, "\"Acc\", \"");
		break;
	case TEMP_HUM_TAG:
		strcpy(datastr, "\"Temp H\", \"");
		break;
	case PIR_TAG:
		strcpy(datastr, "\"PirMotion\", \"");
		break;
	case GPS_TAG:
		strcpy(datastr, "\"GPS\", \"");
		break;
	default:
		strcpy(datastr, "\"Unspecified Type, \"");
		return;
	}

	Time_Stamp(timebuff); //Get time stamp
	strcat(datastr, timebuff); //Prepend the data type to the time

	//	puts arguments into argbuff
	va_start(argList, dataStr);
	vsnprintf(argbuff, sizeof(argbuff), dataStr, argList);
	va_end(argList);
	strcat(datastr, argbuff); //add time stamp to arguments

	if (SENSOR_ARRAY_SIZE < SENSOR_ARRAY_COUNT)
	{
		strncpy(Sensor_Array[SENSOR_ARRAY_SIZE], datastr, SENSOR_SIZE - 1);
		Sensor_Array[SENSOR_ARRAY_SIZE][SENSOR_SIZE - 1] = '\0'; // Ensure null termination
		SENSOR_ARRAY_SIZE++;  // Increment the count of stored strings
	}
}

void ClearDataArray()
{
	for(int i = 0; i < SENSOR_ARRAY_SIZE; i++ )
	{
		memset(Sensor_Array[i], 0, SENSOR_SIZE);
	}
	SENSOR_ARRAY_SIZE = 0; //reinitialize sensor array counter
}


void processQuickOTA(MEM_PTR *Data_Ptr)
{
	int slength = 50;
	PRINTF("Processing quick OTA\r\n" );
	getOtaData(&otaData);
	otaData.fileNumber = 1; // KCS hack
	PRINTF("OTA number of files is %d\r\n", otaData.numberOfFiles );
	PRINTF("OTA file name length is %d\r\n", otaData.fileNameLength );
	if(otaData.fileNameLength >1)
	{
		if (otaData.fileNameLength <50)
		{
			slength = otaData.fileNameLength;
		}

		vcom_Trace((uint8_t*)otaData.otaFileName, slength, 1);
		vcom_Trace((uint8_t*) "\r\n", 2, 1);

	}

	FW_Ack_Pending(Data_Ptr, &otaData);
	FW_Ack_Ready(Data_Ptr, &otaData);
	OTAProcess(Data_Ptr, &otaData);

}

void universalDownlinkDecoder(void)
{
	const char* fwAvTest = "firmware_available_v";
	const char* fwTest = "firmware_v";
	const char* fwComplete = "firmware_completed_v";
	const char* configTest = "\"config\"";
	const char* picAckTest = "image_ack_v";
	const char* timeTest = "\"type\":\"timestamp\"";
	const char* alarmTest = "\"type\":\"alarm\"";
	const char* controlTest = "\"type\":\"control\"";

	downLinkPackets.numberOfPackets = 0;  //

	// Parse each message to figure out its type and increment the count
	for(int msgItter = 0; msgItter < 5; msgItter++)
	{
		if(strstr((char*)downLinkPackets.mQTTMessage[msgItter],fwAvTest))
		{
			downLinkPackets.downLinkEnum[msgItter] = DL_FW_AVAIL;
			downLinkPackets.numberOfPackets++;
		}
		else if(strstr((char*)downLinkPackets.mQTTMessage[msgItter],fwTest))
		{
			downLinkPackets.downLinkEnum[msgItter] = DL_FW_PACKET;
			downLinkPackets.numberOfPackets++;
		}
		else if(strstr((char*)downLinkPackets.mQTTMessage[msgItter],fwComplete))
		{
			downLinkPackets.downLinkEnum[msgItter] = DL_FW_COMPLETE;
			downLinkPackets.numberOfPackets++;
		}
		else if(strstr((char*)downLinkPackets.mQTTMessage[msgItter], configTest))
		{
			downLinkPackets.downLinkEnum[msgItter] = DL_CONFIG_PKT;
			downLinkPackets.numberOfPackets++;
		}

		else if(strstr((char*)downLinkPackets.mQTTMessage[msgItter], picAckTest))
		{
			downLinkPackets.downLinkEnum[msgItter] = DL_PICTURE_ACK;
			downLinkPackets.numberOfPackets++;
		}
		else if(strstr((char*)downLinkPackets.mQTTMessage[msgItter], timeTest))
		{
			downLinkPackets.downLinkEnum[msgItter] = DL_TIME_SYNC;
			downLinkPackets.numberOfPackets++;
		}
		else if(strstr((char*)downLinkPackets.mQTTMessage[msgItter], alarmTest))
		{
			downLinkPackets.downLinkEnum[msgItter] = DL_ALARM_COMMAND;
			downLinkPackets.numberOfPackets++;
		}
		else if(strstr((char*)downLinkPackets.mQTTMessage[msgItter], controlTest))
		{
			downLinkPackets.downLinkEnum[msgItter] = DL_CONTROL;
			downLinkPackets.numberOfPackets++;
		}
		else
		{
			//Check for message length > 1, if greater increment, if not don't
			if(strlen((char*)downLinkPackets.mQTTMessage[msgItter]) > 1)
			{
				downLinkPackets.downLinkEnum[msgItter] = DL_UNKNOWN_MSG;
				downLinkPackets.numberOfPackets++;
			}
			else
			{
				//Debug statement
				PRINTF("No message at index: %d\r\n", msgItter);
				downLinkPackets.downLinkEnum[msgItter] = DL_DONE;
			}
		} //End if / else if / else
	} //End for loop
}

void selectDownlinkOperation(MEM_PTR *Data_Ptr, MACHINE_STATE_TYPE stateOfDevice)
{
	int opItter;
	int num = downLinkPackets.numberOfPackets; //KCS workaround for memory leak, needs fix
	for(opItter = 0; opItter < num; opItter++)
	{
		if(downLinkPackets.downLinkEnum[opItter] == DL_DONE)
		{
			PRINTF("Done\r\n");
		}
		else if(downLinkPackets.downLinkEnum[opItter] == DL_UNKNOWN_MSG)
		{
			PRINTF("Unknown Message\r\n");
		}
		else if(downLinkPackets.downLinkEnum[opItter] == DL_CONFIG_PKT)
		{
			bool updateShadowRegister = false;
			PRINTF("Configure\r\n");
			Refresh_Watchdog;

			//call decodeConfigurations
			if(!decodeDayNightConfigs(downLinkPackets.mQTTMessage[opItter]))
			{
				PRINTF("Day/Night configuration downlink found\r\n");
				updateShadowRegister = true;
			}
			Refresh_Watchdog;
			if(!decodeHBConfigs(Data_Ptr, downLinkPackets.mQTTMessage[opItter]))
			{
				PRINTF("Heart beat configuration downlink found\r\n");
				updateShadowRegister = true;
				hbTimeChanged = true;
			}
			Refresh_Watchdog;
			if(!decodeGPSConfigs(downLinkPackets.mQTTMessage[opItter]))
			{
				PRINTF("GPS configuration downlink found\r\n");
				updateShadowRegister = true;
			}
			Refresh_Watchdog;
			if(!decodePIRConfigs(downLinkPackets.mQTTMessage[opItter]))
			{
				PRINTF("PIR configuration downlink found\r\n");
				updateShadowRegister = true;
			}
			Refresh_Watchdog;
			if(!decodeMotionFilter(downLinkPackets.mQTTMessage[opItter], PIR_DAY))
			{
				PRINTF("PIR day time motion filter downlink found\r\n");
				updateShadowRegister = true;
			}
			Refresh_Watchdog;
			if(!decodeMotionFilter(downLinkPackets.mQTTMessage[opItter], PIR_NIGHT))
			{
				PRINTF("PIR night time motion filter downlink found\r\n");
				updateShadowRegister = true;
			}
			Refresh_Watchdog
			if(!decodeAccelConfigs(downLinkPackets.mQTTMessage[opItter]))
			{
				PRINTF("Accelerometer downlink found\r\n");
				updateShadowRegister = true;
			}
			if(!decodeEnvironmentConfigs(downLinkPackets.mQTTMessage[opItter]))
			{
				PRINTF("Environment downlink found\r\n");
				updateShadowRegister = true;
			}
			if(!decodeHumidityConfigs(downLinkPackets.mQTTMessage[opItter]))
			{
				PRINTF("Humidity downlink found\r\n");
				updateShadowRegister = true;
			}
			if(!decodeTemperatureConfigs(downLinkPackets.mQTTMessage[opItter]))
			{
				PRINTF("Temperature downlink found\r\n");
				updateShadowRegister = true;
			}
			
			if(!decodeCameraConfigs(downLinkPackets.mQTTMessage[opItter]))
			{
				PRINTF("Camera downlink found\r\n");
				updateShadowRegister = true;
			}

			if (updateShadowRegister)
			{
				XPS_paramStore(Data_Ptr);
			}

			//Container functions do nothing if empty
			sendConfigErrors(&memory, &configErrContainer);
			freeContainer(&configErrContainer);
			initContainer(&configErrContainer);


			clearMqttMsg(opItter);
			sendDeviceConfig(Data_Ptr, CONFIG_ACK);
		}
		else if(downLinkPackets.downLinkEnum[opItter] == DL_FW_PACKET)
		{
			PRINTF("Firmware version\r\n");
		}
		else if(downLinkPackets.downLinkEnum[opItter] == DL_FW_AVAIL)
		{
			PRINTF("Firmware Available\r\n");
			fwPending = getOTAFileInfo (&otaData, opItter );
			clearMqttMsg(opItter);
		}
		else if(downLinkPackets.downLinkEnum[opItter] == DL_FW_COMPLETE)
		{
			PRINTF("Firmware Complete\r\n");
			clearMqttMsg(opItter);
		}
		else if(downLinkPackets.downLinkEnum[opItter] == DL_PICTURE_ACK)
		{
			PRINTF("Image Acknowledge\r\n");
			decodeImageAck(Data_Ptr, opItter);
			clearMqttMsg(opItter);
		}
		else if(downLinkPackets.downLinkEnum[opItter] == DL_TIME_SYNC)
		{
			PRINTF("Time stamp received from server\r\n");
			if(!getServerTime(downLinkPackets.mQTTMessage[opItter]))
			{
				PRINTF("Time stamp set from server\r\n");
			}
			clearMqttMsg(opItter);
		}
		else if(downLinkPackets.downLinkEnum[opItter] == DL_ALARM_COMMAND)
		{
			PRINTF("Alarm Command received from server\r\n");
			// TODO
			// Call a function to decode the alarm (gps, temp, humidity, movement)
			// which will set the appropriate alarm settings and feed into the main State Machine
			clearMqttMsg(opItter);
		}
		else if(downLinkPackets.downLinkEnum[opItter] == DL_CONTROL)
		{
			if(!decodeCommand(downLinkPackets.mQTTMessage[opItter], "\"command\":\"store_config_external\""))
			{
				PRINTF("Save Configuration to XPS received from server\r\n");
				saveParamDataToFlash(Data_Ptr);
			}
			else
			{
				PRINTF("Invalid control command received\r\n");
			}
			clearMqttMsg(opItter);
		}
		else
		{
			PRINTF("How did we get here????? Something has gone terribly wrong\r\n");
			//Need to handle Error
		}//End of if / else if/ else
	} //End of loop

	if (fwPending && ((stateOfDevice == WAKEUP_HB) || (stateOfDevice == IDLE)))
	{
		fwPending = false;
		clearMqttStruct(); //Clear structure before we get all the firmware messages
		startOTAProcess(Data_Ptr);
	}
}

/**
 * @brief Stores source message into mQTTMessage buffer.
 * @note Bit by bit array copy from source to mQTTMessage determined by length.
 * @param uint8_t mqttArrayIndex for storing message, uint8_t* source message, uint16_t length of source message, bool print data if true
 * @retval void
 */
void storeMqttData(uint8_t mqttArrayIndex, uint8_t* source, uint16_t length, bool printData)
{
	mqttDataAvailable = true;

	for (int i = 0; i < length; i ++)
	{
		downLinkPackets.mQTTMessage[mqttArrayIndex][i] = source[i];
	}
	if (printData)
	{
		vcom_Trace ( source , length , 1 );
		vcom_Trace ( (uint8_t*) "\r\n" , 2 , 1 );
	}
}

/**
 * @brief Decodes MQTTMsg buffer for firmware data and stores data into volatile memory with pages in order
 * @note
 * @param OTA_FILE_TYPE* OTAData
 * @retval uint8_t error code
 * 1 - one of the 5 packets not found
 * 2 - End of data not found
 * 3 - page not found
 * 5 - page not a number
 * 6 - data not found
 */
uint8_t populateFirmwareArray(OTA_FILE_TYPE *OTAData)
{
	uint8_t errorCode = 0;
	bool isError = false;
	int page = 0;
	// not all 5 packets necessarily contain data
	// KCS improvement - have valid data packets identified and only sort on that data
	for (int i = 0; i < 5; i++)
	{
		isError = false;
		if (downLinkPackets.mQTTMessage[i][0] != '\0')
		{
			char str_copy[MEMORY_MAX] = " ";
			// look for the '}' and only copy that much not up to MEMORY_MAX
			// if it doesn't exist, exit with an error
			int end = strcspn((char *)downLinkPackets.mQTTMessage[i], "}");
			if (end == 0)
			{
				isError = true;
				errorCode = 2;
			}
			else
			{
				strncpy(str_copy, (char *)downLinkPackets.mQTTMessage[i], end);  // Copy MQTTMsg[i] to avoid modifying the original
				str_copy[MEMORY_MAX - 1] = '\0';  // Ensure null-termination
			}

			// Locate "page" in the string and extract its value
			char *page_str = strstr(str_copy, "\"page\":");
			if (page_str)
			{
				end = strcspn(page_str, ",");
				int start = strlen("\"page\":");
				int length = end - start;
				// truncate the string at the "," delimiter
				// duplicate from above !
				if (page_str != NULL)
				{
					char pageNumbString[3] =" ";
					strncpy(pageNumbString,page_str + start,length );
					if(isdigit((unsigned char)pageNumbString[0]))
					{
						page = atoi(pageNumbString);
					}
					else
					{
						isError = true;
						errorCode = 5;
					}
				}
				else
				{
					isError = true;
					errorCode = 4;
				}
			}
			else
			{
				isError = true;
				errorCode = 3;
			}

			if(!isError)
			{
				// Find the data
				char *data_str = strstr(str_copy, "\"data\":\"");
				if (data_str)
				{
					data_str += strlen("\"data\":\"");  // Move pointer to the start of the data content
					char *data_end = strstr(data_str, "\"");  // Find the end of data
					if (data_end != NULL)
					{
						*data_end = '\0';  // Temporarily null-terminate the data string
					}
				}
				else
				{
					errorCode = 6;
					isError = true;
				}

				int len = strlen(data_str);
				// Store extracted data in the OTAData structure
				if (page >= 0 && data_str != NULL && !isError)
				{
					//Copy the cleaned data to FirmwareArray
					strncpy((char*)OTAData->FirmwareArray[page - 1], data_str, len);
					//FirmwareArray[page - 1][MEMORY_MAX - 1] = '\0';
					OTAData->dataFWLength[page - 1] = len;
					PRINTF("Size of page %d is %d\r\n", page, OTAData->dataFWLength[page - 1]);
				}
				else
				{
					errorCode = 6;
					isError = true;
					// kcs this break could be bad if only one packet is corrupt
					break;
				}
			}
		}
		else
		{
			// not really an error if the packet wasn't received
			errorCode = 1;
			// kcs this break assumes the mqtt buffer array is populated in order of 0 to 4
			// that is not a good assumption
			break;
		}
	}//End of loop

	PRINTF("Error is %d\r\n", errorCode);
	return errorCode;
}

bool decodeImageAck(MEM_PTR *Data_Ptr, uint8_t index)
{
	bool test = false; // kcs for testing, set to false for normal operation
	int testcounter = 0;
	//char testString [] = "{{\r\n+QMTPUB: 0,51,0\r\nQMTRECV: 0,1,\"sensors/mobile_device/4325403/downlink\",\"{\"type\":\"image_ack_v0\",\"dev_id\":\"4325403\",\"img_id\":\"08_30_2023__00_04_35-223\",\"total_pages\":25,\"pages_missing\":[2,10,14],\"pages_received\":[1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25]}\"";
	char testString [MEMORY_MAX] = "";

	// KCS fix this by storing the Message data size in the downlinkPacket Structure
	int nonNullCounter = 0;

	for (int counter = 0; counter < 301; counter++ )
	{
		testString[nonNullCounter] = downLinkPackets.mQTTMessage[index][counter];
		nonNullCounter++;
	}

	//	Word_Transfer(testString,(char*)downLinkPackets.mQTTMessage[index]);
	// Psudo code
	// look for image_ack_v0 version number
	// implement switch on version number
	// look for pages missing and put [] string into array of uint16_t

	bool isError = false;
	missingPagesCount = 0;

	char target[] = "image_ack_v0";
	char *p = strstr(testString, target);

	char *missingPages;

	if (p)
		// string exists
		// use strstr to copy missing pages to a string along with the rest of the line
		missingPages = strstr(testString, "s_missing");
	else
	{
		// not found
		isError = true;
		return isError;
	}

	int start;
	int end;
	start = strcspn(missingPages, "[");
	end = strcspn(missingPages, "]");
	char missingList[1000]= "";
	int length = end-start;
	// check for no pages
	if ( length==1 )
	{
		// no missing pages
		if ( test )
		{
			if (testcounter < 1)
			{
				pagesToResend[0]= 3;
				missingPagesCount=1;
				testcounter ++;
			}
		}
	}
	else
	{
		// Using strncpy function to copy the substring
		strncpy(missingList,missingPages+(start+1),end-start-1);
		// now put in array
		char* token;
		char* rest = missingList;
		missingPagesCount = 0;

		while ((token = strtok_r(rest, ",", &rest)))
		{
			pagesToResend[missingPagesCount] = atoi(token);
			missingPagesCount++;
		}
	}
	return isError;
}

bool getMqttDataAvailable(void)
{
	return 	mqttDataAvailable;
}

void sendHeartBeat(MEM_PTR *Data_Ptr)
{
	if (!NightConfirmed)
	{
		CELL_Set_PDP ( PDP_NOT_SET );
		Enable_Modem( Data_Ptr );
	}
	tempInit();
	//	ACC_Init ( Data_Ptr );
//	tempDevice.tempHumData.Temperature =
//	uint16_t plugtest = tempReadRegister(HDC2080_TEMP_LOW, 2);

	HeartBeat ( Data_Ptr );
	//	calculateNextTime(&checkHB, &nextHB, 300);

	// Check for state change in charger cable and send message
	bool newChargeCableState = chargeCableReading();
	if (newChargeCableState != chargerCableState)
	{
		// send message
		if (newChargeCableState)
		{
			sendDiagnostic(&memory, "\"battery\":[\"plugged_in\"]");
		}
		else
		{
			sendDiagnostic(&memory, "\"battery\":[\"unplugged\"]");
		}

		// save state
		chargerCableState = newChargeCableState;
	}

	getFirmwareDownlink(Data_Ptr, 1);
	if ( mqttDataAvailable )
	{
		universalDownlinkDecoder();
		selectDownlinkOperation(Data_Ptr, WAKEUP_HB);
	}
}

bool decodeHBConfigs(MEM_PTR *Data_Ptr, uint8_t *mqttMsg)
{
	bool isError = false;
	char Buff[MEMORY_MAX] = " ";
	char test[] = "\"heartbeat\":{";
	char verTest[] = "\"version\":";
	char modeTest [] = "\"mode\":";
	char intTest [] = "\"hb_interval\":";
	uint8_t version = 255;
	uint8_t mode = 255;
	uint16_t interval = 0;
	bool modeValid = false;
	bool intervalValid = false;
	char hbErrStr[CONFIG_ERR_MSG_SIZE] = "";
	int buffSize = 0;

	// Transfer MQTT message to a local buffer
	Word_Transfer(Buff, (char*)mqttMsg);

	char *substr = strstr(Buff, test);
	if(substr)
	{
		buffSize += snprintf(hbErrStr, CONFIG_ERR_MSG_SIZE, "\"heartbeat\":[\"config_error\",");
		char *verStr = strstr(substr, verTest);
		if(verStr && !isError)
		{
			verStr += strlen(verTest);
			if(isdigit((unsigned char)verStr[0]))
			{
				version = atoi(verStr);

				if(version == 0)
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
								modeValid = true;
//								Data_Ptr->heartBeatData.mode = mode;
//								PRINTF("Setting Heartbeat mode to: %d\r\n", mode);
							}
							else
							{
								buffSize += snprintf((hbErrStr + buffSize), (CONFIG_ERR_MSG_SIZE - buffSize), "\"mode_out_of_range\",");
								PRINTF("Heartbeat mode is out of range [0-255]: %d\r\n", mode);
							}
						}
						else
						{
							isError = true;
							buffSize += snprintf((hbErrStr + buffSize), (CONFIG_ERR_MSG_SIZE - buffSize), "\"invalid_mode_type\",");
							//"Invalid data type for mode"
						}
					}
					else
					{
						isError = true;
						buffSize += snprintf((hbErrStr + buffSize), (CONFIG_ERR_MSG_SIZE - buffSize), "\"missing_mode\",");
						//"Mode not found in heartbeat message"
					}

					char *intStr = strstr(substr, intTest);
					if(intStr)
					{
						intStr += strlen(intTest);
						if(isdigit((unsigned char)intStr[0]))
						{
							interval = atoi(intStr);

							if(interval >= 900 && interval <= 65535)
							{
								// KCS also need to store to XPS shadow register
								intervalValid = true;
//								Data_Ptr->heartBeatData.hbInterval = interval;
//								PRINTF("Setting Heartbeat interval to: %d\r\n", interval);
							}
							else
							{
								isError = true;
								buffSize += snprintf((hbErrStr + buffSize), (CONFIG_ERR_MSG_SIZE - buffSize), "\"interval_out_of_range\",");
								PRINTF("Heartbeat interval is invalid: %d\r\n", interval);
							}
						}
						else
						{
							isError = true;
							buffSize += snprintf((hbErrStr + buffSize), (CONFIG_ERR_MSG_SIZE - buffSize), "\"invalid_interval_type\",");
							//"Invalid data type for interval"
						}
					}
					else
					{
						isError = true;
						buffSize += snprintf((hbErrStr + buffSize), (CONFIG_ERR_MSG_SIZE - buffSize), "\"missing_interval\",");
						//"Interval not found in heartbeat message"
					}
				}
				else
				{
					isError = true;
					PRINTF("Invalid version number decoded: %d\r\n", version);
					buffSize += snprintf((hbErrStr + buffSize), (CONFIG_ERR_MSG_SIZE - buffSize), "\"version_mismatch\",");
				}
			}
			else
			{
				isError = true;
				buffSize += snprintf((hbErrStr + buffSize), (CONFIG_ERR_MSG_SIZE - buffSize), "\"invalid_version_type\",");
				//"Invalid data type for version"
			}
		}
		else
		{
			isError = true;
			buffSize += snprintf((hbErrStr + buffSize), (CONFIG_ERR_MSG_SIZE - buffSize), "\"missing_version\",");
			//"Version not found in heartbeat message"
		}
	}
	else
	{
		isError = true;
		//"No heartbeat configuration found."
	}

	if(!isError && modeValid && intervalValid)
	{
		Data_Ptr->heartBeatData.mode = mode;
		Data_Ptr->heartBeatData.hbInterval = interval;
	}
	else
	{
		if(buffSize > 0 && buffSize < CONFIG_ERR_MSG_SIZE - 2 && hbErrStr[0] != '\0')
		{
			if(hbErrStr[buffSize - 1] == ',')
			{
				hbErrStr[buffSize - 1] = ']';
				addErrorString(hbErrStr);
			}
		}
	}
	PRINTF("Heartbeat decode error is %d\r\n", isError);

	return isError;
}

/**
 * @brief  Gets the number of days in a month in a defined time Date
 * @note
 * @param  TIME_DATE_TYPE checkDate
 *         DateTime structure with structures for Date and Time
 *
 * @retval uint8_t number of days
 */
uint8_t getNumDays(RTC_DateTypeDef checkDate)
{
	uint8_t numOfDays = 0;
	if (checkDate.Month == RTC_MONTH_JANUARY || checkDate.Month == RTC_MONTH_MARCH || checkDate.Month == RTC_MONTH_MAY || checkDate.Month == RTC_MONTH_JULY
			|| checkDate.Month == RTC_MONTH_AUGUST || checkDate.Month == RTC_MONTH_OCTOBER || checkDate.Month == RTC_MONTH_DECEMBER)
	{
		numOfDays = 31;
	}
	else if (checkDate.Month == RTC_MONTH_FEBRUARY)
	{
		if((checkDate.Year % 4) == 0)   //CSC leap year
			numOfDays = 29;
		else
			numOfDays = 28;
	}
	else if (checkDate.Month == RTC_MONTH_APRIL || checkDate.Month == RTC_MONTH_JUNE || checkDate.Month == RTC_MONTH_SEPTEMBER || checkDate.Month == RTC_MONTH_NOVEMBER)
	{
		numOfDays = 30;
	}

	return numOfDays;
}

/**
 * @brief  Sets a date Time at a time interval from the current time
 * @note
 * @param  TIME_DATE_TYPE next
 *         DateTime structure with structures for Date and Time
 *         This time will over written with the future time
 * @param  uint32_t timeSec
 *         number of seconds to add to the check time to calculate next time
 * @retval none
 */
void calculateNextTime(TIME_DATE_TYPE* next, uint32_t timeSec)
{
	// Start at current MCU time
	HAL_RTC_GetTime ( &hrtc , &next->rtcTime , RTC_FORMAT_BIN );
	HAL_RTC_GetDate ( &hrtc , &next->rtcDate , RTC_FORMAT_BIN );

	uint8_t numOfDays = getNumDays(next->rtcDate);
	Refresh_Watchdog;
	next->rtcTime.Seconds += timeSec;
	// Fix for uint8_t limitation on rtcTime.Minutes
	uint16_t minutesFromNow = next->rtcTime.Minutes;

	while (next->rtcTime.Seconds >= 60)
	{
		next->rtcTime.Seconds -= 60;
		minutesFromNow += 1;
	}

	while (minutesFromNow >= 60)
	{
		minutesFromNow -= 60;
		next->rtcTime.Hours += 1;
	}
	// Restore uint8_t Minutes
	next->rtcTime.Minutes = minutesFromNow;

	while (next->rtcTime.Hours >= 24)
	{
		next->rtcTime.Hours -= 24;
		next->rtcDate.Date += 1;
	}

	while (next->rtcDate.Date > numOfDays)
	{
		next->rtcDate.Date -= numOfDays;
		next->rtcDate.Month += 1;
		if (next->rtcDate.Month > 12)
		{
			next->rtcDate.Month = 1;
			next->rtcDate.Year += 1;
		}
	}
}

/**
 * @brief  Checks if a time is in the past of the current RTc CLOCK
 * @note
 * @param  TIME_DATE_TYPE check
 *         DateTime structure with structures for Date and Time
 *         Will be replaced with the current RTC TIME
 * @param  TIME_DATE_TYPE next
 *         DateTime structure with structures for Date and Time
 *         Time to compare with the check time
 * @retval boolean value True = time has elapsed
 */
bool checkNextTime(TIME_DATE_TYPE check, TIME_DATE_TYPE next)
{
	HAL_RTC_GetTime ( &hrtc , &check.rtcTime , RTC_FORMAT_BIN );
	HAL_RTC_GetDate ( &hrtc , &check.rtcDate , RTC_FORMAT_BIN );

	Refresh_Watchdog;
	bool retBool;
	if (check.rtcDate.Year == next.rtcDate.Year)
	{
		if (check.rtcDate.Month == next.rtcDate.Month)
		{
			if (check.rtcDate.Date == next.rtcDate.Date)
			{
				if (check.rtcTime.Hours == next.rtcTime.Hours)
				{
					if (check.rtcTime.Minutes == next.rtcTime.Minutes)
					{
						if(check.rtcTime.Seconds < next.rtcTime.Seconds)
						{
							retBool = false;
						}
						else
						{
							retBool = true;
						}
					}
					else if(check.rtcTime.Minutes < next.rtcTime.Minutes)
					{
						retBool = false;
					}
					else
					{
						retBool = true;
					}
				}
				else if (check.rtcTime.Hours < next.rtcTime.Hours)
				{
					retBool = false;
				}
				else
				{
					retBool = true;
				}
			}
			else if (check.rtcDate.Date < next.rtcDate.Date)
			{
				retBool = false;
			}
			else
			{
				retBool = true;
			}
		}
		else if (check.rtcDate.Month < next.rtcDate.Month)
		{
			retBool = false;
		}
		else
		{
			retBool = true;
		}
	}
	else if (check.rtcDate.Year < next.rtcDate.Year)
	{
		retBool = false;
	}
	else
	{
		retBool = true;
	}

	return retBool;
}

/**
 * @brief  Gets the difference in seconds between two TIME_DATE_TYPE.
 * @note
 * @param  TIME_DATE_TYPE start
 *         DateTime structure with structures for Date and Time
 *         This time will be subtracted from the stop time
 * @param  TIME_DATE_TYPE stop
 *         DateTime structure with structures for Date and Time
 *         if stop time is before the start time, 0 will be returned
 * @retval uint32_t seconds difference between start and stop time (stop-start)
 */
uint32_t getTimeDifference(TIME_DATE_TYPE start, TIME_DATE_TYPE stop)
{
	struct tm currentTime;
	struct tm stopTime;

	//Convert current RTC time to time struct
	currentTime.tm_sec = start.rtcTime.Seconds;
	currentTime.tm_min = start.rtcTime.Minutes;
	currentTime.tm_hour = start.rtcTime.Hours;
	currentTime.tm_mday = start.rtcDate.Date;
	currentTime.tm_mon = start.rtcDate.Month - 1;
	currentTime.tm_year = start.rtcDate.Year + 100;
	currentTime.tm_wday = start.rtcDate.WeekDay - 1;

	//Convert stop RTC time to time struct
	stopTime.tm_sec = stop.rtcTime.Seconds;
	stopTime.tm_min = stop.rtcTime.Minutes;
	stopTime.tm_hour = stop.rtcTime.Hours;
	stopTime.tm_mday = stop.rtcDate.Date;
	stopTime.tm_mon = stop.rtcDate.Month - 1;
	stopTime.tm_year = stop.rtcDate.Year + 100;
	stopTime.tm_wday = stop.rtcDate.WeekDay - 1;

	//Make current and stop times
	time_t stopTimeT = mktime(&stopTime);
	time_t currentTimeT= mktime(&currentTime);

	//Get the difference
	double difference = difftime(stopTimeT, currentTimeT);
	if (difference < 0) difference = 0;
	return (uint32_t)difference;
}

/**
 * @brief  Gets the difference in time between the current MCU time and a TIME_DATE_TYPE stop time
 * @note
 * @param  TIME_DATE_TYPE stop
 *         DateTime structure with structures for Date and Time
 * @retval uint16_t seconds difference between current mcu time and stop time (stop-currentTime)
 */
uint16_t getTimeFromNow(TIME_DATE_TYPE stop)
{
	uint32_t timeLeft = 0;
	TIME_DATE_TYPE localCurrent;

	//Get current time in local struct
	HAL_RTC_GetTime ( &hrtc , &localCurrent.rtcTime , RTC_FORMAT_BIN );
	HAL_RTC_GetDate ( &hrtc , &localCurrent.rtcDate , RTC_FORMAT_BIN );

	//Call getTimeDifference
	timeLeft = getTimeDifference(localCurrent, stop);

	//Check return value for uint16_t overflow
	if(timeLeft > 0x0FFFF)
	{
		//Handle error (return  0xFFFF)
		timeLeft = 0xFFFF;
	}

	//return number of seconds left from now
	return (uint16_t)timeLeft;
}

/**
 * @brief  Sets all low level registers to allow stop3 sleep and wakeup
 * @note
 * @retval void
 */
void stop3Prepare (void)
{
	HAL_UART_AbortReceive_IT( &hlpuart1 );

	__HAL_RCC_PWR_CLK_ENABLE();
	HAL_PWR_EnableBkUpAccess();
	//			   HAL_RCC_MODULE_ENABLED();
	__HAL_RCC_RTC_CONFIG(RCC_RTCCLKSOURCE_LSE);
	__HAL_RCC_RTC_ENABLE();

	__HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_0);
	__HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_6);

	HAL_RTCEx_DeactivateWakeUpTimer(&hrtc);

	__HAL_GPIO_EXTI_CLEAR_IT(EXTI6_IRQn);
	CLEAR_BIT(EXTI->RPR1, EXTI_RPR1_RPIF6);  // Clear the pending interrupt flag for EXTI7

	__HAL_GPIO_EXTI_CLEAR_IT(EXTI7_IRQn);
	CLEAR_BIT(EXTI->RPR1, EXTI_FPR1_FPIF7);  // Clear the pending interrupt flag for EXTI7

	__HAL_RTC_WAKEUPTIMER_ENABLE(&hrtc);
	__HAL_RTC_WAKEUPTIMER_ENABLE_IT(&hrtc, RTC_IT_WUT);

	// __HAL_RTC_CLEAR_FLAG(&hrtc, RTC_FLAG_WUTF);  // Clear WUTF flag
	CLEAR_REG(RTC->ICSR);
	CLEAR_REG(RTC->WUTR);
	CLEAR_REG(RTC->CR);
	CLEAR_REG(PWR->WUSCR);
	HAL_SuspendTick();
}
/**
 * @brief  Clears decoded downlink type and cell received mqtt message index
 * @note
 * @param  uint8_t index
 *         Index of the message to be cleared in the structure
 * @retval void
 */
void clearMqttMsg(uint8_t index)
{
	downLinkPackets.downLinkEnum[index] = DL_DONE;
	int memoryItterator = 0;
	while ( memoryItterator < (MEMORY_MAX))
		downLinkPackets.mQTTMessage[index][ memoryItterator++ ] = 0;
}

/**
 * @brief  Clears entire MQTT_MSG_TYPE structure
 * @note
 * @retval void
 */
void clearMqttStruct(void)
{
	downLinkPackets.numberOfPackets = 0;
	for(int mqttPacketIndex = 0; mqttPacketIndex < 5; mqttPacketIndex++ )
	{
		clearMqttMsg(mqttPacketIndex);
	}
}

/**
 * @brief  Checks each sub component mode to determine if active
 * @note   Uses day/night and sub component mode to determine if active
 * Default is set for false, only handling true cases
 * Ex) if both day and night are disabled, this is handled by default of false
 * @param  HW_MODULE_TYPE hwSubModule
 *         Sub component to be checked
 * @retval return value isActive - true is active, false is disabled
 */
bool checkFunctionActive(HW_MODULE_TYPE hwSubModule)
{
	bool isActive = false; //CSC default is false, handling only true cases below
	bool night = isNight2();
	uint8_t mode = getMode(hwSubModule);
	switch (hwSubModule)
	{
	case HEARTBEAT_MODULE:
		if((mode & 3) == 3)
		{
			isActive = true;
		}
		else if(((mode & 3) == 1) && night)
		{
			isActive = true;
		}
		else if(((mode & 3) == 2) && !night)
		{
			isActive = true;
		}
		break;
	case GPS_MODULE:
		if((mode & 3) == 3)
		{
			isActive = true;
		}
		else if(((mode & 3) == 1) && night)
		{
			isActive = true;
		}
		else if(((mode & 3) == 2) && !night)
		{
			isActive = true;
		}
		break;
	case BUZZER_MODULE:
		if((mode & 12) == 12)
		{
			isActive = true;
		}
		else if(((mode & 4) == 4) && night)
		{
			isActive = true;
		}
		else if(((mode & 8) == 8) && !night)
		{
			isActive = true;
		}
		break;
	case TEMPERATURE_MODULE:
		if((mode & 3) == 3)
		{
			isActive = true;
		}
		else if(((mode & 3) == 1) && night)
		{
			isActive = true;
		}
		else if(((mode & 3) == 2) && !night)
		{
			isActive = true;
		}
		break;
	case HUMIDITY_MODULE:
		if((mode & 3) == 3)
		{
			isActive = true;
		}
		else if(((mode & 3) == 1) && night)
		{
			isActive = true;
		}
		else if(((mode & 3) == 2) && !night)
		{
			isActive = true;
		}
		break;
	case CELL_MODULE:
		if((mode & 3) == 3)
		{
			isActive = true;
		}
		else if(((mode & 3) == 1) && night)
		{
			isActive = true;
		}
		else if(((mode & 3) == 2) && !night)
		{
			isActive = true;
		}
		break;
	case CAMERA_MODULE:
		if((mode & 3) == 3)
		{
			isActive = true;
		}
		else if(((mode & 3) == 1) && night)
		{
			isActive = true;
		}
		else if(((mode & 3) == 2) && !night)
		{
			isActive = true;
		}
		break;
	case ACCELEROMETER_MODULE:
		if((mode & 3) == 3)
		{
			isActive = true;
		}
		else if(((mode & 3) == 1) && night)
		{
			isActive = true;
		}
		else if(((mode & 3) == 2) && !night)
		{
			isActive = true;
		}
		break;
	case PIR_MODULE:
		if (isTimeDefault())
		{
			PRINTF("Time is default, turning Active to true");
			isActive = true;
		}
		else
		{
			PRINTF("PIR Mode is: %d\r\n", mode);
			if((mode & 3) == 3)
			{
				isActive = true;
			}
			else if(((mode & 3) == 1) && night)
			{
				isActive = true;
			}
			else if(((mode & 3) == 2) && !night)
			{
				isActive = true;
			}
		}
		break;
	default:
		PRINTF("Invalid module received\r\n");
		break;

	}
	return isActive;
}

/**
 * @brief  Clears Flag states For pictures, Motion detection and Buzzer
 * @retval null
 */
void clearPirInterreptUpdateStates(void)
{
	if (memory.State & PIC_UPDT)
	{
		memory.State ^= PIC_UPDT;
	}
	if (memory.State & PIC_SEND)
	{
		memory.State ^= PIC_SEND;
	}
	if (memory.State & PIR_UPDT)
	{
		memory.State ^= PIR_UPDT;
	}
	if (memory.State & PIR_START)
	{
		memory.State ^= PIR_START;
	}
	if (memory.State & BUZZER_TOGGLE)
	{
		memory.State ^= BUZZER_TOGGLE;
	}
}

/**
 * @brief  Clears Flag states For heartbea and movement detection
 * @retval null
 */
void clearAccellerometerInterruptStates(void)
{
	if (memory.State & HB_UPDT)
	{
		memory.State ^= HB_UPDT;
	}
	if (memory.State & MOVEMENT_UPDT)
	{
		memory.State ^= MOVEMENT_UPDT;
	}
	if (memory.State & GPS_UPDT)
	{
		memory.State ^= GPS_UPDT;
	}

}

void powerDownDeviceForSleep(void)
{
	HAL_GPIO_WritePin ( USB_Power_Enable_GPIO_Port , USB_Power_Enable_Pin , GPIO_PIN_RESET );    //for simulated battery power operation
	HAL_GPIO_WritePin ( USB_Power_Switch_On_GPIO_Port , USB_Power_Switch_On_Pin , GPIO_PIN_RESET );    //for simulated battery power operation
	HAL_GPIO_WritePin ( Camera_Enable_GPIO_Port , Camera_Enable_Pin , GPIO_PIN_RESET );    //for simulated battery power operation

	//	CLEAR_REG(PWR -> WUSCR);
	HAL_GPIO_WritePin ( Cell_Enable_GPIO_Port , Cell_Enable_Pin , GPIO_PIN_RESET );    //for simulated battery power operation
	HAL_GPIO_WritePin ( Cell_DVS_GPIO_Port , Cell_DVS_Pin , GPIO_PIN_RESET );          //for simulated battery power operation
	isModemPowered = false;
}

//KCS maybe this should return the isError bool
void sendBatteryStatus(STATUS_BATTERY_TYPE status)
{
	static char statusString[40] = " ";
	static char buffer[UPLINK_DIAGNOSTIC_MSG_SIZE] = " ";

	bool isError = true;

	switch (status)
	{
	case BAT_CHARGING:
		//"battery": ["charging"]
		strcpy(statusString, "charging");

		break;
	case BAT_FULL:
		//"battery": ["full"]
		strcpy(statusString, "full");

		break;
	case CHARGER_UNPLUGGED:
		//"battery": ["charger_unplugged"]
		strcpy(statusString, "charger_unplugged");
		break;
		//
	default:
		strcpy(statusString, "not_specified");
		break;
	}

	int buffSize = sprintf(buffer, "\"battery\":[\"%s\"]",statusString );

	if(buffSize > 0 && buffSize < DOWNLINK_TEST_MSG_SIZE)
	{
		isError = false;
	}

	sendDiagnostic(&memory, buffer );
}

/**
 * @brief  Outputs a buzzer dying tone
 * @note   Expand in the future to provide several tones and also move to a Buzzer.c file
 * @retval null
 */
void buzzerTone(void)
{
	// KCS add case statements and many cases
	// for now just a disconnect cell case
	//Case Cell not connected
	HAL_GPIO_WritePin ( Buzzer_GPIO_Port , Buzzer_Pin, GPIO_PIN_SET );
	HAL_Delay ( 500 );
	HAL_GPIO_TogglePin ( Buzzer_GPIO_Port , Buzzer_Pin );
	HAL_Delay ( 300 );
	HAL_GPIO_TogglePin ( Buzzer_GPIO_Port , Buzzer_Pin );
	HAL_Delay ( 500 );
	HAL_GPIO_TogglePin ( Buzzer_GPIO_Port , Buzzer_Pin );
	HAL_Delay ( 300 );
	HAL_GPIO_TogglePin ( Buzzer_GPIO_Port , Buzzer_Pin );
	HAL_Delay ( 2000 );
	HAL_GPIO_TogglePin ( Buzzer_GPIO_Port , Buzzer_Pin );
	// end of Cell not connected

	// Just in case make sure it is off at end
	HAL_GPIO_WritePin ( Buzzer_GPIO_Port , Buzzer_Pin, GPIO_PIN_RESET );

}

void restartModem (void)
{
	HAL_GPIO_WritePin ( Sky_Reset_GPIO_Port , Sky_Reset_Pin , GPIO_PIN_SET );
	HAL_Delay(3000);
	HAL_GPIO_WritePin ( Sky_Reset_GPIO_Port , Sky_Reset_Pin , GPIO_PIN_RESET );
	HAL_Delay(6000);
}

uint16_t minValue4 (uint16_t HB, uint16_t PIR, uint16_t ACC, uint16_t MUTEGPS)
{
	uint16_t min = 0;

	if ( MuteInit && accelMuteInit )
	{
		if ((HB < PIR) && (HB < ACC) && (HB < MUTEGPS))
		{

			wakeupState = hb_wu_enabled;
			min = HB;
			PRINTF("WakeupState is hb_wu_enabled\r\n");

		}
		else if ((PIR < HB) && (PIR < ACC) && (PIR < MUTEGPS))
		{
			wakeupState = img_wu_enabled;
			min = PIR;
			PRINTF("WakeupState is img_wu_enabled\r\n");

		}
		else if ((ACC < HB) && (ACC < PIR) && (ACC < MUTEGPS))
		{
			wakeupState = acc_wu_enabled;
			min = ACC;
			PRINTF("WakeupState is acc_wu_enabled\r\n");

		}
		else if ((MUTEGPS < HB) && (MUTEGPS < PIR) && (MUTEGPS < ACC))
		{
			wakeupState = pGPS_wu_enabled;
			min = MUTEGPS;
			PRINTF("WakeupState is pGPS_wu_enabled\r\n");

		}

		else if ((HB == PIR) && (HB == ACC) && (HB == MUTEGPS))
		{
			wakeupState = hb_img_acc_pair;
			min = HB;
			PRINTF("WakeupState is hb_img_acc_pair\r\n");

		}

		else if ((HB == PIR) && (HB = ACC) && (HB < MUTEGPS))
		{
			wakeupState = hb_img_acc_pair;
			min = HB;
			PRINTF("WakeupState is hb_img_acc_pair\r\n");

		}

		else if ((HB == PIR) && (HB = MUTEGPS) && (HB < ACC))
		{
			wakeupState = hb_img_pGPS_pair;
			min = HB;
			PRINTF("WakeupState is hb_img_pGPS_pair\r\n");

		}
		else if ((HB == ACC) && (HB = MUTEGPS) && (HB < PIR))
		{
			wakeupState = hb_acc_pair;
			min = HB;
			PRINTF("WakeupState is hb_acc_pair\r\n");

		}

		else if ((PIR == ACC) && (PIR = MUTEGPS) && (PIR < HB))
		{
			wakeupState = img_acc_pair;
			min = PIR;
			PRINTF("WakeupState is img_acc_pair\r\n");

		}



		else if ((HB == PIR) && (HB < ACC) && (HB < MUTEGPS))
		{
			wakeupState = hb_img_pair;
			min = HB;
			PRINTF("WakeupState is hb_img_pair\r\n");

		}
		else if ((HB == ACC) && (HB < PIR) && (HB < MUTEGPS))
		{
			wakeupState = hb_acc_pair;
			min = HB;
			PRINTF("WakeupState is hb_acc_pair\r\n");

		}
		else if ((HB == MUTEGPS) && (HB < PIR) && (HB < ACC))
		{
			wakeupState = hb_pGPS_pair;
			min = HB;
			PRINTF("WakeupState is hb_pGPS_pair\r\n");

		}
		else if ((PIR == ACC) && (PIR < HB) && (PIR < MUTEGPS))
		{
			wakeupState = img_acc_pair;
			min = PIR;
			PRINTF("WakeupState is img_acc_pair\r\n");

		}

		else if ((PIR == MUTEGPS) && (PIR < HB) && (PIR < ACC))
		{
			wakeupState = img_pGPS_pair;
			min = PIR;
			PRINTF("WakeupState is img_pGPS_pair\r\n");

		}


		else if ((ACC == MUTEGPS) && (ACC < HB) && (ACC < PIR))
		{
			wakeupState = acc_wu_enabled;
			min = ACC;
			PRINTF("WakeupState is acc_wu_enabled\r\n");
		}



	}
	else if (MuteInit && (!accelMuteInit))
	{
		if (HB < PIR)
		{
			wakeupState = hb_wu_enabled;
			min = HB;
			PRINTF("WakeupState is hb_wu_enabled\r\n");

		}
		else if (PIR < HB)
		{
			wakeupState = img_wu_enabled;
			min = PIR;
			PRINTF("WakeupState is img_wu_enabled\r\n");

		}
		else if (PIR == HB)
		{
			wakeupState = hb_img_pair;
			min = HB;
			PRINTF("WakeupState is hb_img_pair\r\n");

		}
	}

	else if (accelMuteInit && (!MuteInit))
	{
		if ((HB == ACC) && (HB == MUTEGPS))
		{
			wakeupState = hb_acc_pair;
			min = HB;
			PRINTF("WakeupState is hb_acc_pair\r\n");

		}
		else if ((HB == ACC) && (HB < MUTEGPS))
		{
			wakeupState = hb_acc_pair;
			min = HB;
			PRINTF("WakeupState is hb_acc_pair\r\n");
		}
		else if ((HB == MUTEGPS) && (HB < ACC))
		{
			wakeupState = hb_pGPS_pair;
			min = HB;
			PRINTF("WakeupState is hb_pGPS_pair\r\n");
		}
		else if ((ACC == MUTEGPS) && (ACC < HB))
		{
			wakeupState = acc_wu_enabled;
			min = ACC;
			PRINTF("WakeupState is acc_wu_enabled\r\n");
		}
		else if ((HB < ACC) && (HB < MUTEGPS))
		{
			wakeupState = hb_wu_enabled;
			min = HB;
			PRINTF("WakeupState is hb_wu_enabled\r\n");
		}
		else if ((ACC < HB) && (ACC < MUTEGPS))
		{
			wakeupState = acc_wu_enabled;
			min = ACC;
			PRINTF("WakeupState is acc_wu_enabled\r\n");
		}
		else if ((MUTEGPS < HB) && (MUTEGPS < ACC))
		{
			wakeupState = pGPS_wu_enabled;
			min = MUTEGPS;
			PRINTF("WakeupState is pGPS_wu_enabled\r\n");
		}

	}
	else
	{
		wakeupState = hb_wu_enabled;
		min = HB;
		PRINTF("WakeupState is hb_wu_enabled\r\n");
	}

	return min;
}
/**
 * @brief  Decodes control commands
 * @note   Expand in the future to provide several tones and also move to a Buzzer.c file
 * @retval null
 */
bool decodeCommand(uint8_t *msg, char *testStr)
{
	bool retError = true;
	char tempBuff[MEMORY_MAX] = " ";

	// Transfer MQTT message to a local buffer as not to interfere with the original mqtt message
	Word_Transfer(tempBuff, (char*)msg);

	char *storeStr = strstr(tempBuff, testStr);
	if(storeStr)
	{
		retError = false;
	}

	return retError;
}

//Container Functions
/**
 * @brief  Initializes the string container with 10 spots for config errors.
 * @note   Called in memory_Init after all the parameters are received
 * @param  STRING_CONTAINER* container representing container to be initialized
 * @retval returns -1 if malloc fails, 0 if it passes
 */
int8_t initContainer(STRING_CONTAINER* container)
{
	int8_t statusReturn = 0;
    container->strings = malloc(CONTAINER_INIT_SIZE * CONFIG_ERR_MSG_SIZE);

    if(!container->strings)
    {
		container->count = 0;
		container->capacity = 0;
		statusReturn = -1; // allocation failed
		PRINTF("Container initialization failed\r\n");
    }
    else
    {
    	container->count = 0;
    	container->capacity = CONTAINER_INIT_SIZE;
    	PRINTF("Container initialization passed\r\n");
    }

    return statusReturn;
}

/**
 * @brief  Adds to container and checks for resize
 * @note   Handles resizing and only allows to add 250 characters
 * @param  STRING_CONTAINER* container representing container to add
 * 		   const char* str representing the string to add
 * @retval returns -1 if realloc fails
 */
int8_t addToContainer(STRING_CONTAINER* container, const char* str)
{
	int8_t statusReturn = 0;

	if (container->count >= container->capacity)
	{
		uint16_t newCapacity = container->capacity + CONTAINER_RESIZE_CHUNK;
		char (*newArray)[CONFIG_ERR_MSG_SIZE] = realloc(container->strings, newCapacity * CONFIG_ERR_MSG_SIZE);

		if (!newArray)
		{
			PRINTF("Container reallocation failed\r\n");
			statusReturn = -1;
		}
		else
		{
			container->strings = newArray;
			container->capacity = newCapacity;
			PRINTF("Container reallocation succeeded, new capacity = %d\r\n", container->capacity);
		}
	}

	if (statusReturn == 0)
	{
		strncpy(container->strings[container->count], str, CONFIG_ERR_MSG_SIZE - 1);
		container->strings[container->count][CONFIG_ERR_MSG_SIZE - 1] = '\0';
		container->count++;
	}

	return statusReturn;
}

/**
 * @brief  Prints container contents.
 * @note   Mostly for debugging
 * @param  STRING_CONTAINER* container container to print
 * @retval void
 */
void printContainer(STRING_CONTAINER* container)
{
	if (container->count == 0)
	{
		PRINTF("Container is empty\r\n");
		return;
	}

    for (uint16_t i = 0; i < container->count; i++)
    {
        PRINTF("[%d]: %s\r\n", i, container->strings[i]);
    }
}

/**
 * @brief  Frees container
 * @note   Imperative to not overflow heap. Should call after sending errors.
 * @param  STRING_CONTAINER* container container to free
 * @retval void
 */
void freeContainer(STRING_CONTAINER* container)
{
    free(container->strings);
    container->strings = NULL;
    container->count = 0;
    container->capacity = 0;
    PRINTF("Container freed\r\n");
}

/**
 * @brief  Makes container a single string and sends it as a diagnostic
 * @note
 * @param  MEM_PTR *Mem needed in sendDiagnostic
 * 		   STRING_CONTAINER* container, container to concatinate
 * @retval void
 */
void sendConfigErrors(MEM_PTR *Mem, STRING_CONTAINER* container)
{
	if (container->count == 0)
	{
		PRINTF("Container is empty\r\n");
		return;
	}
	else if (container->count <= (MEMORY_MAX/CONFIG_ERR_MSG_SIZE) - 1)
	{
		char sendBuff[MEMORY_MAX];
		int buffSize = 0;
		for(int i = 0; i < container->count; i++)
		{
			buffSize += snprintf((sendBuff + buffSize), (MEMORY_MAX - buffSize), "%s,", container->strings[i]);
		}

		if(buffSize > 0 && buffSize < (MEMORY_MAX - CONFIG_ERR_MSG_SIZE) && sendBuff[0] != '\0')
		{
			if(sendBuff[buffSize - 1] == ',')
			{
				sendBuff[buffSize - 1] = '\0';
				buffSize--;
				PRINTF("%s\r\n", sendBuff);
				sendDiagnostic(Mem, sendBuff);
			}
		}
		else
		{
			PRINTF("Buffer overflow, not sending\r\n");
		}
	}
	else
	{
		//Need to packetize because we have too many errors
	}
}

/**
 * @brief  Accessor for decode functions to add errors to containter
 * @note
 * @param  char* externalStr, error string to add
 * @retval returns status code of addToContainer
 */
int8_t addErrorString(char* externalStr)
{
	int8_t retVal = 0;
	retVal = addToContainer(&configErrContainer, externalStr);
	return retVal;
}
/**
 * @brief  gets the state of the charger cable; unplugged is false
 * @note   move to a battery.c file
 * @retval boolean state , false is unplugged, true is connected to power
 */
bool chargeCableReading(void)
{
	return (Is_Pin_Low( nExt_Power_GPIO_Port , nExt_Power_Pin ));
}

#endif  //FUNCTIONS_C_

