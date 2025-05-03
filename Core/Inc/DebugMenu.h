/*
 * Debug_Menu.h
 *
 *  Created on: May 24, 2023
 *      Author: SDITD
 */

#ifndef DEBUGMENU_H_
#define DEBUGMENU_H_

/******************************************************
	Note: Print menu on command line for compiler,
		  used for program clarity
******************************************************/


#ifdef DEV_INIT
	#warning Device Init Set

	#ifdef Init_Device
//		#warning Inital Set
	#endif

	#ifdef Init_Components
		#warning Initalizer State Enabled
	#endif

	#ifdef Updt_Components
		#warning Update State Enabled
	#endif

	#ifdef Device_Sleep
		#warning Sleep Enabled
	#endif

	#ifdef Collect_Info
		#warning Initial Data Collection Enabled
	#endif
#endif

#ifdef INTERRUPT
	#warning Interrupt Enabled
#endif

#ifdef Log
	#warning Logger Enabled

	#ifdef EXTERNAL_XPS_STORAGE
		#ifdef XPS_Logs
			#warning XPS Logs Enabled
		#endif
	#endif

	#ifdef UART_USB
		#warning USB Logs Enabled
	#endif

	#ifdef Log_Level_1
		#warning Log level 1 Enabled
	#endif

	#ifdef Log_Level_2
		#warning Log Level 2 Enabled
	#endif

	#ifdef Log_2_Start
		#warning Log Level 2 Started Enabled
	#endif
#endif

#ifdef EXTERNAL_XPS_STORAGE
	#ifdef DEBUG_EXTERNAL_XPS_STORAGE
		#warning XPS Enabled
	#endif
#endif

#if defined(SKYWIRE_MODEM) && defined(EXTERNAL_XPS_STORAGE) && defined(ACCELERATION_SENSOR) && defined(TEMPERATURE_SENSOR)
	#ifdef VALID_PACKAGE
		#warning Cell Packet Verifier Enabled
	#endif

	#ifdef Init_HeartBeat
		#warning Initial Heartbeat Enabled
	#endif

	#ifdef CERTIFICATE
		#warning Cell Certificates Enabled
	#endif
#endif

#ifdef TEMPERATURE_SENSOR
	#warning Temperature Enabled

	#ifdef DEBUG_TEMPERATURE_SENSOR
		#warning Temperature Debug Enabled
	#endif
#endif

#ifdef ACCELERATION_SENSOR
	#warning Acceleromter Enabled

	#ifdef DEBUG_ACCELERATION_SENSOR
		#warning Acceleromter Debug Enabled
	#endif

	#ifdef Device_ACC_Wake
		#warning Acceleromter Wake Enabled
	#endif
#endif

#ifdef BOOT_CONFIGURE
	#warning Bootloader Change Enabled

	#ifdef New_Boot_ADDR
		#warning New Boot Address Attempt Enabled
	#endif
#endif

#endif /* DEBUGMENU_H_ */
