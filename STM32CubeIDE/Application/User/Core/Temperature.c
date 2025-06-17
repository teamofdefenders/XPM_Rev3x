/**
  ******************************************************************************
  * @file    Temperature.c
  * @author  Gage VanHaverbeke
  * @brief   driver temperature and humidity sensor
  ******************************************************************************
  * @attention
  *
  * Copyright 2024 Team of Defenders
  *
  ******************************************************************************
  */
#ifndef Temperature_C
#define Temperature_C

/******************************************************
 Includes
 Note:
 ******************************************************/

#include "Temperature.h"
#include "string.h"


/******************************************************
 Global Variables
 Note:
 ******************************************************/

TEM_STATUS_REG TEM;

/******************************************************
 Define Control
 Note:
 ******************************************************/
// KCS remove all of these redirections !!!!!!!
// Also in general, refactor this code in the same style as DayNight.c
#define TX 			TEM.TX
#define Address 	TEM.ADDRESS
#define Command 	TEM.COMMAND
#define State 		TEM.STATE
#define TX_Size 	TEM.TX_Size
#define Timer 		TEM.Timer


// These have to stay for now but we should get rid of them
#define Setting 	_Setting
#define Size  		_Size
//#define Device_ID 	_Temp_Device_ID
//#define Temperature _Temperature
//#define Humidity 	_Humidity
#define Memory 		_Memory

typedef struct
{
	TEMP_DEVICE_TYPE partNumber; // Device hardware
	uint16_t address;           // set by device
	TEMP_STATE_TYPE state;	    // Current device state
} TEMP_DEVICE_STATE_TYPE;

TEMP_DEVICE_STATE_TYPE tempDevice;
TEMPERATURE_DATA_TYPE privateTempParameters;

/**
 * @brief  Initializes temperature sensor for measurement mode and reads temp/humidity
 * @note   Initializes temperature sensor
 * @param  void
 * @retval void
 */
void tempInit(void)
{
	PRINTF("____TEMPERATURE SENSOR BEGIN INITIALIZATION_____\r\n\r\n");
	tempDevice.partNumber = HDC2080;
	tempDevice.address = HDC2080_SLAVE_ADDRESS;
	tempDevice.state = TEMP_OK;

	tempWhoAmI();

	if (tempDevice.partNumber == HDC2080)
	{
		tempWriteRegisterRaw(HDC2080_MEASUREMENT_CONF, HDC2080_START_TEMP_HUMIDITY_MEASUREMENT);  //measurement mode
		privateTempParameters.Temperature = tempReadRegister(HDC2080_TEMP_LOW);
		privateTempParameters.Humidity = tempReadRegister(HDC2080_HUM_LOW);
	}

	PRINTF("____TEMPERATURE SENSOR END INITIALIZATION_____\r\n\r\n");
}

/**
 * @brief  This function sets the device part number
 * @note   May be worth checking the address space on i2c bus to check for unknown devices.
 * @param  void
 * @retval void
 */
void tempWhoAmI(void)
{
	uint16_t partNum = tempReadDeviceID();

	if(partNum == HDC2080)
	{
		PRINTF("HDC2080 Identified\r\n");
		tempDevice.partNumber = HDC2080;
		tempDevice.address = HDC2080_SLAVE_ADDRESS;
	}
	else
	{
		PRINTF("____NO TEMP SENSOR IDENTIFIED_____\r\n\r\n");
		tempDevice.partNumber = TEMP_UNKNOWN_DEVICE;
		tempDevice.address = UNKNOWN_TEMP_SLAVE_ADDRESS;
	}
}

/**
 * @brief  Reads a register from the temp sensor
 * @note   registers are defined in functions.h
 * @param  TEM_I2C_Command
 * 		   Typedefined enumerated list of registers and commands
 * @retval uint16_t representing the value of the register
 */
uint16_t tempReadRegister(TEM_I2C_Command readReg)
{
	HAL_StatusTypeDef ret;
	uint8_t reg;
	uint8_t temp_data[TEMP_DATA_SIZE];
	uint16_t regval = 0;

	switch(tempDevice.partNumber)
	{
		case HDC2080: //Fall through because they have the same address
			reg = readReg;
			//csc for synchronous operation
			ret = HAL_I2C_Master_Transmit(&hi2c1, tempDevice.address, &reg, 1, 0xFF);
			ret = HAL_I2C_Master_Receive(&hi2c1, tempDevice.address + 1, temp_data, TEMP_DATA_SIZE, 0xFF);

			if(ret == HAL_OK)
			{
				regval = (temp_data[1] << 8 | temp_data[0]);
			}
			else
			{
				return 0;
			}
			break;
		default:
			PRINTF("Can not read unknown address\r\n");
	}
	return regval;
}

/**
 * @brief  This function gets the device part number
 * @note   Called in tempWhoAmI
 * @param  void
 * @retval uint16_t representing partNumber, returns 0 on not found
 */
uint16_t tempReadDeviceID(void)
{
	HAL_StatusTypeDef ret;
	uint8_t reg;
	uint8_t temp_data[TEMP_DATA_SIZE];
	uint16_t notFound = 0;

	switch(tempDevice.partNumber)
	{
		case HDC2080:
			reg = HDC2080_DEVICE_ID_LOW;
			//csc for synchronous operation
			ret = HAL_I2C_Master_Transmit(&hi2c1, tempDevice.address, &reg, 1, 0xFF);
			ret = HAL_I2C_Master_Receive(&hi2c1, tempDevice.address + 1, temp_data, TEMP_DATA_SIZE, 0xFF);

			if(ret == HAL_OK)
			{
				privateTempParameters.Device_ID = (temp_data[1] << 8 | temp_data[0]);
			}
			else
			{
				privateTempParameters.Device_ID = notFound;
			}
			break;
		default:
			PRINTF("Can not read unknown address\r\n");
			privateTempParameters.Device_ID = notFound;
	}
	return privateTempParameters.Device_ID;

}

/**
 * @brief  This function writes a raw command to temperature sensor
 * @note
 * @param  TEM_I2C_Command tempReg,uint8_t command
 * @retval void
 */
void tempWriteRegisterRaw(TEM_I2C_Command tempReg, uint8_t command)
{
	HAL_StatusTypeDef ret;
	char failureAck[20] = "";
	uint8_t txReg[2];
	txReg[0] = tempReg;
	txReg[1] = command;

	switch(tempDevice.partNumber)
	{
		case HDC2080:
			ret = HAL_I2C_Master_Transmit(&hi2c1, tempDevice.address, txReg, 2, 0xFFFF);

			//	HAL_ERROR    = 0x01,
			//	HAL_BUSY     = 0x02,
			//	HAL_TIMEOUT  = 0x03
			if(ret != HAL_OK)
			{
				//Consider doing a retry
				//PRINTF needs reason for error
				if(ret == HAL_ERROR)
				{
					char err[] = "HAL_ERROR\0";
					int len = strlen(err);
					strncpy(failureAck, err, len);
				}
				else if(ret == HAL_BUSY)
				{
					char err[] = "HAL_BUSY\0";
					int len = strlen(err);
					strncpy(failureAck, err, len);
				}
				else if(ret == HAL_TIMEOUT)
				{
					char err[] = "HAL_TIMEOUT\0";
					int len = strlen(err);
					strncpy(failureAck, err, len);
				}
				PRINTF("Unsuccessful transmission: %s\r\n", failureAck);
			}
			break;
		default:
			PRINTF("Temperature sensor not supported, can not write to register\r\n");
	}
}

/**
 * @brief  Temperature data accessor function
 * @note
 * @param  TEMPERATURE_DATA_TYPE extAccelParameters
 *         Reference to external temperature data type
 * @retval void
 */
void getTempSensorData(TEMPERATURE_DATA_TYPE *extTempParams)
{
	extTempParams->Temperature = privateTempParameters.Temperature;
	extTempParams->Humidity = privateTempParameters.Humidity;
	extTempParams->Device_ID = privateTempParameters.Device_ID;
}

/**
 * @brief  Public accessor for just the part number
 * @note   Called in heartbeat
 * @param  external temp device type
 * @retval void
 */
void getTempDeviceType(TEMP_DEVICE_TYPE* extTemp)
{
	*extTemp = tempDevice.partNumber;
}

#endif
