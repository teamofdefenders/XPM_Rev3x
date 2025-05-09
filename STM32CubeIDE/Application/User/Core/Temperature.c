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

///******************************************************
// TEM Initialize Function
// Initializes the MQTT parameters as well as
// establish bridge with server.
// ******************************************************/
//void TEM_Init ( MEM_PTR *Data_Ptr )
//{
//	Timer = 0xFFF;
//	Temperature = Default_Temperature;
//	Humidity = Default_Humidity;
//
//	Address = SHT4X_SLAVE_ADDRESS;
//	Device_ID = Address;
//
//	Command = CHECK_TEMP;
//	TEM_COMMAND ( Data_Ptr );
//	if (State == TEM_OK)
//	{
//		Command = SET_TEMP;
//		TEM_COMMAND ( Data_Ptr );
//		Temperature = Updated_Temperature;
//		Humidity = Updated_Humidity;
//		return;
//	}
//
//	HAL_I2C_DeInit ( &hi2c1 );
//	HAL_I2C_Init ( &hi2c1 );
//
//	Address = SI7021_SLAVE_ADDRESS;
//	Device_ID = Address;
//
//	Command = CHECK_TEMP;
//	TEM_COMMAND ( Data_Ptr );
//	if (Memory [ 0 ] == 0x20 || Memory [ 0 ] == 0x10)
//	{
//		Command = SET_TEMP;
//		TEM_COMMAND ( Data_Ptr );
//		Temperature = Updated_Temperature;
//		Humidity = Updated_Humidity;
//		return;
//	}
//
//	HAL_I2C_DeInit ( &hi2c1 );
//	HAL_I2C_Init ( &hi2c1 );
//
//	Address = BSD240_SLAVE_ADDRESS;
//	Device_ID = Address;
//
//	HAL_GPIO_WritePin ( BPS240_GPIO_Port , BPS240_Pin , GPIO_PIN_SET );
//	HAL_Delay ( 3 );
//	Command = CHECK_TEMP;
//	TEM_COMMAND ( Data_Ptr );
//	if (State == TEM_OK)
//	{	//TEM -> COMMAND = SET_TEMP;		TEM_COMMAND(Data_Ptr,TEM);
//		Temperature = Updated_Temperature;
//		Humidity = Updated_Humidity;
//		return;
//	}
//
//	Address = TEMP_SENSOR_FAIL;
//	Device_ID = Address;
//	Temperature = Default_Temperature;
//	Humidity = Default_Humidity;
//}

///******************************************************
// TEM Temperature_Data.Command Function
// Controller for temperature communication
// ******************************************************/
//void TEM_COMMAND ( MEM_PTR *Data_Ptr )
//{
//	Refresh_Watchdog;
//
//#ifdef Log_Level_1
//	if (Setting & LOG_LEVEL_1) Log_End ( LOG_TEMPERATURE , Command );
//	else if (Setting & LOG_LEVEL_1 && Setting & LOG_LEVEL_2) Log_Insert ( LOG_TEMPERATURE , Command );
//#endif
//
//	switch (Command)
//	{
//		case CHECK_TEMP:
//			State = TEM_OK;
//
//			Command = FIRMWARE_TEM;
//			TEM_COMMAND ( Data_Ptr );
//			return;
//		case HUMIDITY:
//			if (Address == SI7021_SLAVE_ADDRESS)
//			{
//				TX [ 0 ] = SI7021_HUMID_NO_HOLD;
//				Size = 1;
//				TX_Size = 2;
//			}
//			else if (Address == SHT4X_SLAVE_ADDRESS)
//			{
//				Command = TEMPERATURE;
//				TEM_COMMAND ( Data_Ptr );
//				return;
//			}
//			else if (Address == BSD240_SLAVE_ADDRESS)
//			{
//				Command = SET_TEMP;
//				TEM_COMMAND ( Data_Ptr );
//
//				TX [ 0 ] = BSD240_HUMD_A;
//				Size = 1;
//				TX_Size = 2;
//			}
//
//			TEM_MSG ( Data_Ptr );
//
//			if (State != TEM_OK)
//			{
//				return;
//			}
//
//			if (Address == SI7021_SLAVE_ADDRESS)
//			{
//				Humidity = ((Memory [ 0 ]) << 8) | (Memory [ 1 ]);
//			}
//			else if (Address == SHT4X_SLAVE_ADDRESS)
//			{
//				return;
//			}
//			else if (Address == BSD240_SLAVE_ADDRESS)
//			{
//				Humidity = ((Memory [ 1 ]) << 8) | (Memory [ 0 ]);
//				Command = RESET_TEMP;
//				TEM_COMMAND ( Data_Ptr );
//			}
//
//#ifdef Log_Level_2
//			if (Setting & LOG_LEVEL_1 && Setting & LOG_LEVEL_2)
//			{
//				Log_Insert ( LOG_DEVICE_ID , Address );
//				Log_Insert ( LOG_STATE_RESULT , State );
//				Log_End ( LOG_HUMIDITY_RESULT , Humidity );
//			}
//#endif
//			return;
//		case TEMPERATURE:
//			if (Address == SI7021_SLAVE_ADDRESS)
//			{
//				TX [ 0 ] = SI7021_TEMP_NO_HOLD;
//				Size = 1;
//				TX_Size = 2;
//			}
//			else if (Address == SHT4X_SLAVE_ADDRESS)
//			{
//				TX [ 0 ] = SHT4x_NOHEAT_LOWPRECISION;
//				Size = 1;
//				TX_Size = 6;
//			}
//			else if (Address == BSD240_SLAVE_ADDRESS)
//			{
//				Command = SET_TEMP;
//				TEM_COMMAND ( Data_Ptr );
//
//				TX [ 0 ] = BSD240_TEMP_A;
////					TX[1] = BSD240_TEMP_B;
//
//				HAL_Delay ( 10 );
//				Size = 1;
//				TX_Size = 2;
//			}
//
//			TEM_MSG ( Data_Ptr );
//
//			if (State != TEM_OK)
//			{
//				return;
//			}
//
//			if (Address == SI7021_SLAVE_ADDRESS)
//			{
//				Temperature = ((Memory [ 0 ]) << 8) | (Memory [ 1 ]);
//			}
//			else if (Address == SHT4X_SLAVE_ADDRESS)
//			{
//				Temperature = ((Memory [ 0 ]) << 8) | (Memory [ 1 ]);
//				Humidity = ((Memory [ 4 ]) << 8) | (Memory [ 5 ]);
//			}
//			else if (Address == BSD240_SLAVE_ADDRESS)
//			{
//				Temperature = ((Memory [ 1 ]) << 8) | (Memory [ 0 ]);
//				Command = RESET_TEMP;
//				TEM_COMMAND ( Data_Ptr );
//			}
//
//#ifdef Log_Level_2
//			if (Setting & LOG_LEVEL_1 && Setting & LOG_LEVEL_2)
//			{
//				Log_Insert ( LOG_DEVICE_ID , Address );
//				Log_Insert ( LOG_STATE_RESULT , State );
//				Log_End ( LOG_PREV_TEMPERATURE_RESULT , Temperature );
//			}
//#endif
//			return;
//		case PREVIOUS_TEMP:
//			if (Address == SI7021_SLAVE_ADDRESS)
//			{
//				TX [ 0 ] = SI7021_READ_PREV_RH;
//				Size = 1;
//				TX_Size = 2;
//			}
//			else if (Address == SHT4X_SLAVE_ADDRESS)
//			{
//				return;
//			}
//			else if (Address == BSD240_SLAVE_ADDRESS)
//			{
//				return;
//			}
//
//			TEM_MSG ( Data_Ptr );
//			Temperature = ((Memory [ 0 ]) << 8) | (Memory [ 1 ]);
//
//#ifdef Log_Level_2
//			if (Setting & LOG_LEVEL_1 && Setting & LOG_LEVEL_2)
//			{
//				Log_Insert ( LOG_DEVICE_ID , Address );
//				Log_Insert ( LOG_STATE_RESULT , State );
//				Log_End ( LOG_TEMPERATURE_RESULT , Temperature );
//			}
//#endif
//			return;
//		case RESET_TEMP:
//			if (Address == SI7021_SLAVE_ADDRESS)
//			{
//				TX [ 0 ] = SI7021_RESET;
//				Size = 1;
//				TX_Size = 1;
//			}
//			else if (Address == SHT4X_SLAVE_ADDRESS)
//			{
//				TX [ 0 ] = SHT4x_SOFTRESET;
//				TX [ 1 ] = SHT4x_READSERIAL;
//				Size = 2;
//				TX_Size = 6;
//			}
//			else if (Address == BSD240_SLAVE_ADDRESS)
//			{
//				HAL_GPIO_WritePin ( BPS240_GPIO_Port , BPS240_Pin , GPIO_PIN_RESET );
//				HAL_Delay ( 10 );
//
//				return;
//			}
//
//			TEM_MSG ( Data_Ptr );
//
//			if (State != TEM_OK)
//			{
//				return;
//			}
//
//#ifdef Log_Level_2
//			if (Setting & LOG_LEVEL_1 && Setting & LOG_LEVEL_2)
//			{
//				Log_Insert ( LOG_DEVICE_ID , Address );
//				Log_End ( LOG_STATE_RESULT , State );
//			}
//#endif
//			return;
//		case SET_TEMP:
//			if (Address == SI7021_SLAVE_ADDRESS)
//			{
//				TX [ 0 ] = SI7021_WRITE_RHT;
//				TX [ 1 ] = SI7021_WRITE_RHT_SET;
//				TX [ 2 ] = SI7021_READ_RHT;
//
//				Size = 2;
//				TX_Size = 1;
//			}
//			else if (Address == SHT4X_SLAVE_ADDRESS)
//			{
//				return;
//			}
//			else if (Address == BSD240_SLAVE_ADDRESS)
//			{
//				HAL_GPIO_WritePin ( BPS240_GPIO_Port , BPS240_Pin , GPIO_PIN_SET );
//				HAL_Delay ( 10 );
//
//				TX [ 0 ] = BSD240_ID_INST;
//				TX [ 1 ] = 0;
//				TX [ 1 ] |= BSD240_NORMAL_OPERATION_MODE | BSD240_HUM_NO_AVG | BSD240_TEMP_AVG_8
//				        | BSD240_START_DETECTION;
//				TX_Size = 1;
//				Size = 2;
//			}
//
//			TEM_MSG ( Data_Ptr );
//
//			if (Address == BSD240_SLAVE_ADDRESS)
//			{
//				Size = 1;
//				do
//				{
//					TEM_MSG ( Data_Ptr );
//
//					HAL_Delay ( 3 );
//				}
//				while (Memory [ 0 ] & BSD240_START_DETECTION);
//			}
//
//#ifdef Log_Level_2
//			if (Setting & LOG_LEVEL_1 && Setting & LOG_LEVEL_2)
//			{
//				Log_Insert ( LOG_DEVICE_ID , Address );
//				Log_End ( LOG_STATE_RESULT , State );
//			}
//#endif
//			return;
//		case FIRMWARE_TEM:
//
//			if (Address == SI7021_SLAVE_ADDRESS)
//			{
//				TX [ 0 ] = SI7021_FIRMWARE_TEM_A;
//				TX [ 1 ] = SI7021_FIRMWARE_TEM_B;
//				Size = 2;
//				TX_Size = 1;
//			}
//			else if (Address == SHT4X_SLAVE_ADDRESS)
//			{
//				TX [ 0 ] = SHT4x_READSERIAL;
//				Size = 1;
//				TX_Size = 6;
//			}
//			else if (Address == BSD240_SLAVE_ADDRESS)
//			{
//				TX [ 0 ] = BSD240_ID_INST;
//				TX [ 1 ] |= BSD240_NORMAL_OPERATION_MODE | BSD240_HUM_NO_AVG | BSD240_TEMP_AVG_8
//				        | BSD240_STOP_DETECTION;
//				Size = 2;
//				TX_Size = 1;
//			}
//
//			TEM_MSG ( Data_Ptr );
//
//			if (Address == BSD240_SLAVE_ADDRESS)
//			{
//				Command = RESET_TEMP;
//				TEM_COMMAND ( Data_Ptr );
//			}
//
//#ifdef Log_Level_2
//			if (Setting & LOG_LEVEL_1 && Setting & LOG_LEVEL_2)
//			{
//				Log_Insert ( LOG_DEVICE_ID , Address );
//				Log_End ( LOG_STATE_RESULT , State );
//			}
//#endif
//			return;
//		default:
//			State = TEM_TYPE_PARAM_ERR;
//
//#ifdef Log_Level_2
//			if (Setting & LOG_LEVEL_1 && Setting & LOG_LEVEL_2)
//			{
//				Log_End ( LOG_STATE_RESULT , 250 );
//			}
//#endif
//			break;
//	}
//}

/******************************************************
 Temperature_Data.Temperature Message
 Sends message created over the I2C bus
 ******************************************************/
//void TEM_MSG ( MEM_PTR *Data_Ptr )
//{
//	if (State != TEM_OK && Command != FIRMWARE_TEM)   //GAV look here for temp hold error
//	{
//		return;
//	}
//
//	if ((Address) == SI7021_SLAVE_ADDRESS)
//	{
//		HAL_Delay ( 100 );
//	}
//
////	while(HAL_I2C_IsDeviceReady(&hi2c1, Address, 15, TEM.TIMER) != HAL_OK){;};
//
//	if (HAL_I2C_Master_Transmit ( &hi2c1 , Address , TX , Size , Timer ) != HAL_OK)
//	{
//		State = TEM_TX_CMD_ERR;
//		Temperature = 2;
//		Humidity = 2;
//
//#ifdef Log_Level_1
//		Write_Log ( LOG_ERROR );
//		Log_End ( LOG_STATE_RESULT , State );
//#endif
//
//		return;
//	}
//
//	while (HAL_I2C_IsDeviceReady ( &hi2c1 , Address , 15 , Timer ) != HAL_OK)
//		;
//
////	if((Address) != SI7021_SLAVE_ADDRESS)
//	HAL_Delay ( 100 );
//
//	if (HAL_I2C_Master_Receive ( &hi2c1 , (Address) + 1 , Memory , TX_Size ,
//	Timer ) != HAL_OK)
//	{
//		State = TEM_RX_CMD_ERR;
//		Temperature = 3;
//		Humidity = 3;
//
//#ifdef Log_Level_1
//		Write_Log ( LOG_ERROR );
//		Log_End ( LOG_STATE_RESULT , State );
//#endif
//
//		return;
//	}
//
//#ifdef Log_Level_2
//	Write_Log ( LOG_SUCCESS );
//	Log_End ( LOG_STATE_RESULT , State );
//#endif
//}
//
//void TEM_Clear_Command ( MEM_PTR *Data_Ptr )
//{
//	buf = I2C_CMD_SIZE - 1;
//	while (buf)
//		TX [ buf-- ] = 0;
//}
//
//void TEM_Set_Command ( TEM_CMD_TYPE input )
//{
//	Command = input;
//}
//
//TEM_STATE TEM_Get_State ( void )
//{
//	return State;
//}

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
	}

	privateTempParameters.Temperature = tempReadRegister(HDC2080_TEMP_LOW);
	privateTempParameters.Humidity = tempReadRegister(HDC2080_HUM_LOW);

	PRINTF("____TEMPERATURE SENSOR END INITIALIZATION_____\r\n\r\n");
}

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
	}
}

uint16_t tempReadRegister(TEM_I2C_Command readReg)
{
	HAL_StatusTypeDef ret;
	uint8_t reg;
	uint8_t temp_data[TEMP_DATA_SIZE];
	uint16_t regval;

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

uint16_t tempReadDeviceID(void)
{
	HAL_StatusTypeDef ret;
	uint8_t reg;
	uint8_t temp_data[TEMP_DATA_SIZE];

	switch(tempDevice.partNumber)
	{
		case HDC2080: //Fall through because they have the same address
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
				return 0;
			}
			break;
		default:
			PRINTF("Can not read unknown address\r\n");
	}
	return privateTempParameters.Device_ID;

}

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

void getTempSensorData(TEMPERATURE_DATA_TYPE *extTempParams)
{
	extTempParams->Temperature = privateTempParameters.Temperature;
	extTempParams->Humidity = privateTempParameters.Humidity;
	extTempParams->Device_ID = privateTempParameters.Device_ID;
}

#endif
