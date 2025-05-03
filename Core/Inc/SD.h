/*
 * XPS.h
 *
 *  Created on: Jan 10, 2023
 *      Author: Erik Pineda-A
 */

#ifndef __SD_H
#define __SD_H

/******************************************************
 File Includes
 Note: n/a
 ******************************************************/

#include "Functions.h"

/******************************************************
 Define Values
 Note: Values Gathered from Datasheet
 ******************************************************/

#define SDP_SENSOR_FAIL 0x01			//
#define SDP_TIMER 0xF

#define LOG_SD "\1 SD Command : \0"	// Start of UART MSG
#define LOG_SD_RESULT "\1 SD Result : \0"
#define LOG_PREV_SD_RESULT "\1 Previous SD Result : \0"
#define LOG_SD_COM_ERR "\1 Wrong SD Command \0"

/******************************************************
 Global Enum
 Note: Values are used for error codes,
 command types, and XPS state
 ******************************************************/
typedef enum
{							// Error codes used for Memage debugging
	SI7021_FIRMWARE_SD_A = 0x84,		// ALT : 0xB8
	SI7021_FIRMWARE_SD_B = 0xB8,		//
	SI7021_ID_BYTE_1 = 0xFA,			// ALT : 0x0F
	SI7021_ID_BYTE_2 = 0xFC,			// ALT : 0xC9
	SI7021_READ_HEAT_CTRL = 0x11,		// View control state of heater
	SI7021_WRITE_HEAT_CTRL = 0x51,		// Write control state of heater

	SI7021_RESET = 0xFE,			// Reset Device
	SI7021_READ_RHT = 0xE7,		// Read register 1

	SHT4x_READSERIAL = 0x89,/**< Read Out of Serial Register */
	SHT4x_SOFTRESET = 0x94, /**< Soft Reset */
} SD_I2C_Command;

typedef enum
{							// Error codes used for Memage debugging
	SD_OK = 0,  						// Chip OK - Execution fine
	SD_NOT_OK,  						// Chip NOT OK - Execution NOTE fine
	SD_BUSY,							// Chip busy
	SD_TYPE_PARAM_ERR,					// Error code for incorect command
	SD_TX_CMD_ERR,						// Command TX issue
	SD_RX_CMD_ERR						// Command RX issue
} SD_STATE;

typedef enum
{							// Type definitions for SD command
	CHECK_SD = 0,						//
	FIRMWARE_SD,						//
	SD_WRITE,
	SD_READ,
	SET_SDP,							//
} SD_CMD_TYPE;

typedef struct
{							// Current SD State
	uint32_t Timer;
	uint16_t TX_Size;
	SD_CMD_TYPE COMMAND;					// Control Buffer
	SD_I2C_Command TX [ SPI_CMD_SIZE ];
	SD_STATE STATE;					// Current state of SD Module
} SD_STATUS_REG;

/******************************************************
 Function Prototypes
 Note: Typical void functions are placed with ptr
 pointer due to logic flaw in the ucontroller
 ******************************************************/

void SD_Init ( MEM_PTR *Mem );
void SD_COMMAND ( MEM_PTR *Mem );
void SD_MSG ( MEM_PTR *Mem );

void SD_Clear_Command ( MEM_PTR *Data_Ptr );
void SD_Set_Command ( SD_CMD_TYPE input );
SD_STATE SD_Get_State ( void );

/******************************************************
 Shared Global Variables
 Note:
 ******************************************************/

extern SPI_HandleTypeDef hspi2;			// External I2C configuration
extern IWDG_HandleTypeDef hiwdg;		// External IWDG configuration

#endif
