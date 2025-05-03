/**
  ******************************************************************************
  * @file    XPS.c
  * @author  Erik Pineda-A
  * @brief   driver for SD card functionality
  ******************************************************************************
  * @attention
  *
  * Copyright 2024 Team of Defenders
  *
  ******************************************************************************
  */
#ifndef SD_C
#define SD_C

/******************************************************
 Includes
 Note:
 ******************************************************/

#include "SD.h"

/******************************************************
 Global Variables
 Note:
 ******************************************************/

SD_STATUS_REG SD;

/******************************************************
 Define Control
 Note:
 ******************************************************/

#define TX 			SD.TX
#define Command 	SD.COMMAND
#define State 		SD.STATE
#define TX_Size 	SD.TX_Size
#define Timer 		SD.Timer

#define Setting 	_Setting
#define Size  		_Size
#define Memory 		_Memory

/******************************************************
 SD Initializer
 Note: Checks for SD population
 ******************************************************/
void SD_Init ( MEM_PTR *Data_Ptr )
{
	Timer = 0xFFF;

	Command = CHECK_SD;
	SD_COMMAND ( Data_Ptr );
}

/******************************************************
 SD Command Execution
 Note: Send command to SD based on command input
 ******************************************************/
void SD_COMMAND ( MEM_PTR *Data_Ptr )
{
	Refresh_Watchdog;

	switch (Command)
	{
		case CHECK_SD:
			State = SD_OK;

			Command = FIRMWARE_SD;
			SD_COMMAND ( Data_Ptr );
			return;
		case FIRMWARE_SD:

			Memory [0] = 1;
			Memory [1] = 2;
			Memory [2] = 2;
			Command = SD_WRITE;
			SD_COMMAND ( Data_Ptr );

			Command = SD_READ;
			SD_COMMAND ( Data_Ptr );

			return;
		case SD_WRITE:

			SD_MSG ( Data_Ptr );

			return;
		case SD_READ:

			SD_MSG ( Data_Ptr );

			return;
		case SET_SDP:

			SD_MSG ( Data_Ptr );

			return;
		default:
			State = SD_TYPE_PARAM_ERR;

#ifdef Log_Level_1
			Log_Single ( LOG_SD_COM_ERR );
#endif
	}
	return;
}

/******************************************************
 SPI MSG
 Note: Send data through SPI to SD device
 ******************************************************/
void SD_MSG ( MEM_PTR *Data_Ptr )
{
	if (State != SD_OK && Command != FIRMWARE_SD) return;

	if (HAL_SPI_TransmitReceive ( &hspi2 , (const uint8_t*) TX , Memory , Size , Timer ) != HAL_OK)
	{
		State = SD_TX_CMD_ERR;

#ifdef Log_Level_1
		Log_Single ( LOG_ERROR );
#endif
	}
}

void SD_Clear_Command ( MEM_PTR *Data_Ptr )
{
	buf = SPI_CMD_SIZE - 1;
	while (buf)
		TX [ buf-- ] = 0;
}

void SD_Set_Command ( SD_CMD_TYPE input )
{
	Command = input;
}

SD_STATE SD_Get_State ( void )
{
	return State;
}

#endif
