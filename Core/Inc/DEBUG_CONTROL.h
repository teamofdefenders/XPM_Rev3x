/*
 * DEBUG_CONTROL.h
 *
 *  Created on: Mar 16, 2023
 *      Author: Erik Pineda-A
 */

#ifndef DEBUG_CONTROL_H_
#define DEBUG_CONTROL_H_

/******************************************************
		Debug Control Variables
	Note: Allows user to enable/disable components of
		  of the device for debugging purposes
		  #error
		  #warning
******************************************************/

#warning Manual Debug Mode Set
/* Program System Configuration */
#define DEV_INIT
#define INTERRUPT
//#define Log

/* Program System Configuration */
#define EXTERNAL_XPS_STORAGE
#define SKYWIRE_MODEM
#define TEMPERATURE_SENSOR
#define ACCELERATION_SENSOR
/* [Not Properly Tested] */
//#define BOOT_CONFIGURE

/* Program Debug Function Control */
#ifdef DEBUGG
	#define DEBUG_MSG

	#ifdef EXTERNAL_XPS_STORAGE
		#define DEBUG_EXTERNAL_XPS_STORAGE
	#endif
	#ifdef ACCELERATION_SENSOR
		#define DEBUG_ACCELERATION_SENSOR
	#endif
	#ifdef TEMPERATURE_SENSOR
		#define DEBUG_TEMPERATURE_SENSOR
	#endif
#endif

/* Program Function Control */
#ifdef DEV_INIT
	#define Init_Device
	#define Init_Components
	#define Updt_Components
	#define Device_Sleep
//	#define Collect_Info
#endif

#if defined(SKYWIRE_MODEM) && defined(EXTERNAL_XPS_STORAGE) && defined(ACCELERATION_SENSOR) && defined(TEMPERATURE_SENSOR)
	#define VALID_PACKAGE
	#define Init_HeartBeat
//	#define CERTIFICATE
#endif

#ifdef Log
	#ifdef EXTERNAL_XPS_STORAGE
//		#define XPS_Logs
	#endif

//	#define UART_USB
//	#define Log_Level_1
//	#define Log_Level_2
//	#define Log_2_Start
#endif

#ifdef BOOT_CONFIGURE
//	#define New_Boot_ADDR
#endif

#ifdef ACCELERATION_SENSOR
//	#define Device_ACC_Wake
#endif

#endif /* DEBUG_CONTROL_H_ */
