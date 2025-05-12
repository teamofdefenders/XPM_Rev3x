/*
 * XPS.h
 *
 *  Created on: Jan 10, 2023
 *      Author: Erik Pineda-A
 */

#ifndef __TEMPERATURE_H
#define __TEMPERATURE_H

/******************************************************
 File Includes
 Note: n/a
 ******************************************************/

#include "Functions.h"

/******************************************************
 Define Values
 Note: Values Gathered from Datasheet
 ******************************************************/

#define TEMP_SENSOR_FAIL 0x01			//
#define TEMP_TIMER 0xF

#define Default_Temperature 0xFF
#define Default_Humidity 0xFF
#define Updated_Temperature 0xF3
#define Updated_Humidity 0xF3

#define LOG_TEMPERATURE "\1 Temperature Command : \0"	// Start of UART MSG
#define LOG_TEMPERATURE_RESULT " Temperature Result : \0"
#define LOG_PREV_TEMPERATURE_RESULT " Previous Temperature Result : \0"
#define LOG_HUMIDITY_RESULT " Humidity Result : \0"

#define TEMP_DATA_SIZE 2

#define SI7021_TEMP(val) (((val*175.72)/65536)-46.85)
#define SI7021_HUMID(val) (((val*125)/65536)-6)

/******************************************************
 Global Enum
 Note: Values are used for error codes,
 command types, and XPS state
 ******************************************************/

typedef enum
{
	TEMP_OK = 0,
	TEMP_NOT_OK,  						// Chip NOT OK - Execution NOTE fine
	TEMP_BUSY,							// Chip busy
	TEMP_TYPE_PARAM_ERR,					// Error code for incorect command
	TEMP_TX_CMD_ERR,						// Command TX issue
	TEMP_RX_CMD_ERR
} TEMP_STATE_TYPE;

typedef struct
{
	uint16_t temperature;					// Stores Temperature
	uint16_t humidity;						// Stores Humidity
}TEMP_DATA_TYPE;
typedef enum
{

	HDC2080_DEVICE_ID_LOW = 0xFE,
	HDC2080_DEVICE_ID_HIGH = 0xFF,
	HDC2080_MEASUREMENT_CONF = 0x0F,
	HDC2080_TEMP_LOW = 0x00,
	HDC2080_TEMP_HIGH = 0x01,
	HDC2080_HUM_LOW = 0x02,
	HDC2080_HUM_HIGH = 0x03,



	HDC2080_START_TEMP_HUMIDITY_MEASUREMENT = 0x01,
} TEM_I2C_Command;

typedef enum
{							// Error codes used for Memage debugging
	SI7021_SLAVE_ADDRESS = 0x80,		// ALT : 0xB8
	BSD240_SLAVE_ADDRESS = 0xFE,		// ALT : 0x7F
	SHT4X_SLAVE_ADDRESS = 0x88,		//0x45 alternate
} TEM_I2C_Address;

typedef enum
{							// Error codes used for Memage debugging
	TEM_OK = 0,  						// Chip OK - Execution fine
	TEM_NOT_OK,  						// Chip NOT OK - Execution NOTE fine
	TEM_BUSY,							// Chip busy
	TEM_TYPE_PARAM_ERR,					// Error code for incorect command
	TEM_TX_CMD_ERR,						// Command TX issue
	TEM_RX_CMD_ERR						// Command RX issue
} TEM_STATE;

typedef enum
{							// Type definitions for TEM command
	CHECK_TEMP = 0,						//
	FIRMWARE_TEM,						//
	HUMIDITY,				 			//
	TEMPERATURE,						//
	PREVIOUS_TEMP,						//
	RESET_TEMP,							//
	SET_TEMP,							//
} TEM_CMD_TYPE;

typedef struct
{							// Current TEM State
	uint32_t Timer;
	uint16_t TX_Size;
	TEM_I2C_Address ADDRESS;
	TEM_CMD_TYPE COMMAND;					// Control Buffer
	TEM_I2C_Command TX [ I2C_CMD_SIZE ];
	TEM_STATE STATE;					// Current state of TEM Module
} TEM_STATUS_REG;

typedef enum
{
	HDC2080_SLAVE_ADDRESS   = 0x80,
	UNKNOWN_TEMP_SLAVE_ADDRESS = 0x00
} TEMP_I2C_Address;

/******************************************************
 Function Prototypes
 Note: Typical void functions are placed with ptr
 pointer due to logic flaw in the ucontroller
 ******************************************************/
void tempInit(void);
void tempWhoAmI(void);
uint16_t tempReadRegister(TEM_I2C_Command readReg);
uint16_t tempReadDeviceID(void);
void tempWriteRegisterRaw(TEM_I2C_Command tempReg, uint8_t command);

void TEM_Init ( MEM_PTR *Mem );
void TEM_COMMAND ( MEM_PTR *Mem );
void TEM_MSG ( MEM_PTR *Mem );

void TEM_Clear_Command ( MEM_PTR *Data_Ptr );
void TEM_Set_Command ( TEM_CMD_TYPE input );
TEM_STATE TEM_Get_State ( void );

/******************************************************
 Shared Global Variables
 Note:
 ******************************************************/

extern I2C_HandleTypeDef hi2c1;			// External I2C configuration
extern IWDG_HandleTypeDef hiwdg;		// External IWDG configuration

#endif
