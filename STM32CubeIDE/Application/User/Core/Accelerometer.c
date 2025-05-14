/**
  ******************************************************************************
  * @file    Accelerometer.c
  * @author  Connor Coultas
  * @brief   driver for accelerometer
  ******************************************************************************
  * @attention
  *
  * Copyright 2024 Team of Defenders
  *
  ******************************************************************************
  */
#ifndef ACCELEROMETER_C_
#define ACCELEROMETER_C_

/******************************************************
 Includes
 Note:
 ******************************************************/

#include "Accelerometer.h"
#include "string.h"


typedef struct
{
	ACCEL_DEVICE_TYPE partNumber; // Device hardware
	uint16_t address;           // set by device
	ACCEL_STATE_TYPE state;	    // Current device state
} ACCEL_DEVICE_STATE_TYPE;

/******************************************************
 Global Variables
 Note:
 ******************************************************/
uint8_t deviceID;
bool accelTrigger = false;

ACCEL_DEVICE_STATE_TYPE accelDevice;
ACCEL_DATA_TYPE privateAccelData;
ACCELERATION_PARAM_TYPE privateAccelParameters;

// TODO return status in all function returns that are currently "Void"

/******************************************************
 Define Control
 Note:
 ******************************************************/
/**
 * @brief  Initializes accelerometer
 * @note   The calls in this function may change as our application specifications change
 * 		   Currently configuring for low power mode at 50 HZ, 14-bit resolution
 * 		   Wake-Up interrupts are enabled at medium to low threshold with no minimum duration for detection
 *
 * 		   If not a stop3 wake up, it is imperative to disable interrupts before changing interrupt functionality due to the device
 * 		   possibly interrupting during i2c communication, causing a hanging program
 * @param  void
 * @retval void
 */
void accelInit(void)
{
	PRINTF("____ACCELEROMETER BEGIN INITIALIZATION_____\r\n\r\n");
	accelDevice.partNumber = LIS2DW; // default to be changed below
	accelDevice.address = LIS2_SLAVE_ADDREESS;
	accelDevice.state = ACC_OK;
	accelWhoAmI();

	switch(accelDevice.partNumber)
	{
		case LIS2DW:
			accelWriteRegisterCommand(LIS2DW_CTRL_2, LIS2DW_SOFT_RESET); //Reset
			accelWriteRegisterCommand(LIS2DW_CTRL_2, LIS2DW_REBOOOT); //Reboot
			//Device needs up to 20 ms to reset according to data sheet
			HAL_Delay(20);
			setAccelMode(LIS_MODE_LOW_POWER); //Set power mode
			setAccelResolution(LIS_RESOLUTION_14); //Set resolution
			setAccelDataRate(LIS_DATA_RATE_50); //Set output data rate
			accelWriteRegisterCommand(LIS2DW_CTRL_3, LIS2DW_CTRL_3_LIR); //Set latched interrupts
			accelWriteRegisterCommand(LIS2DW_CTRL_6, LIS2DW_CTRL_6_SET); //Low noise selection
			accelWriteRegisterCommand(LIS2DW_WAKE_DURATION, WRITE_ZERO); //Set wake duration
			accelWriteRegisterCommand(LIS2DW_WAKE_THRESH_REG, LIS2DW_WAKE_THRESH_VAL); //Set wakeup threshold
			accelWriteRegisterCommand(LIS2DW_FF_REG, LIS2DW_FREE_SET);
			accelWriteRegisterCommand(LIS2DW_INT1_CTRL_4, LIS2DW_CTRL_4_SET); //Set Wake Up and FF interrupt
			accelWriteRegisterCommand(LIS2DW_CTRL_7, LIS2DW_CTRL_7_SET); //Enable interrupts INT_Enable
			break;
		case LIS2DUX:
			accelWriteRegisterCommand(LIS2DUX_CTRL_1, LIS2DUX_SOFT_RESET);
			HAL_Delay(20);
			accelWriteRegisterCommand(LIS2DUX_CTRL_5, LIS2DUX_CTRL_5_SET); //Sets power mode
			accelWriteRegisterCommand(LIS2DUX_CTRL_1, LIS2DUX_CTRL_1_SET); //Enables wake up on all axis
			accelWriteRegisterCommand(LIS2DUX_WAKE_THRESH_REG, LIS2DUX_WAKE_THRESH_VAL); //Set wakeup threshold
			accelWriteRegisterCommand(LIS2DUX_WAKE_DURATION, WRITE_ZERO); //Set wake duration
			accelWriteRegisterCommand(LIS2DUX_INT_CNFG_REG, LIS2DUX_INT_ENABLE); //Enable interrupts
			accelWriteRegisterCommand(LIS2DUX_ROUTE_INT1_REG, LIS2DUX_ROUTE_INT1_VAL);
			break;

		default:
			PRINTF("Unidentified accelerometer\r\n");
	}
	accelClearLatch();
	PRINTF("____ACCELEROMETER END INITIALIZATION_____\r\n\r\n");
}
/**
 * @brief  Initializes accelerometer data parameters to 0
 * @note
 * @param  void
 * @retval void
 */
void accelDataInit(void)
{
	privateAccelData.Accel_X = 0;
	privateAccelData.Accel_Y = 0;
	privateAccelData.Accel_Z = 0;
	deviceID = 0;
}

/**
 * @brief  Initializes accelerometer configuration parameters to defaults
 * @note
 * @param  void
 * @retval void
 */
void accelParametersInit(void)
{
	privateAccelParameters.hysteresis = DEF_HYSTERESIS_ACCEL;
	privateAccelParameters.mode = DEF_MODE_ACCEL;
	privateAccelParameters.range = DEF_RANGE_ACCEL;
	privateAccelParameters.threshold = DEF_THRESH_ACCEL;
	privateAccelParameters.duration = DEF_DURATION;
	privateAccelParameters.mutePeriod = DEF_MUTE_ACCEL;
}

/**
 * @brief  Reads a register from the accelerometer
 * @note   registers are defined in functions.h
 * @param  ACC_I2C_Command
 * 		   Typedefined enumerated list of registers and commands
 * @retval uint8_t representing the value of the register
 */
uint8_t accelReadRegister(ACCEL_I2C_Command readReg)
{
	HAL_StatusTypeDef ret;
	uint8_t reg;
	uint8_t regVal = 255;

	// KCS make this a Switch statement, Bomb out if part number is "unknown"
	switch(accelDevice.partNumber)
	{
		case LIS2DW: //Fall through because they have the same address
		case LIS2DUX:
			reg = readReg;
			//csc for synchronous operation
			ret = HAL_I2C_Master_Transmit(&hi2c1, accelDevice.address, &reg, 1, 0xFF);
			ret = HAL_I2C_Master_Receive(&hi2c1, accelDevice.address + 1, &regVal, 1, 0xFF);

			if(ret == HAL_OK)
			{
				return regVal;
			}
			else
			{
				PRINTF("HAL Status: %d\r\n", ret);
				return 0;
			}
			break;
		default:
			PRINTF("Can not read unknown address\r\n");
			return regVal;
	}
}

/**
 * @brief  This function sends a single command to the accelerometer
 * @note
 * @param  ACC_I2C_Command representing the register to write
 * 		   ACC_I2C_Command representing the data to write
 * @retval void - May want to change to boolean
 */
void accelWriteRegisterCommand(ACCEL_I2C_Command accReg, ACCEL_I2C_Command command)
{
	HAL_StatusTypeDef ret;
	char failureAck[20] = "";
	uint8_t txReg[2];
	txReg[0] = accReg;
	txReg[1] = command;

	switch(accelDevice.partNumber)
	{
		case LIS2DW:
		case LIS2DUX:
			ret = HAL_I2C_Master_Transmit(&hi2c1, accelDevice.address, txReg, 2, 0xFFFF);

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
			PRINTF("Accelerometer not supported, can not write to register\r\n");
	}
}

/**
 * @brief  This function sends a single command to the accelerometer
 * @note
 * @param  ACC_I2C_Command representing the register to write
 * 		   uint8_t representing the data to write
 * @retval void - May want to change to boolean
 */
void accelWriteRegisterRaw(ACCEL_I2C_Command accReg, uint8_t command)
{
	HAL_StatusTypeDef ret;
	char failureAck[20] = "";
	uint8_t txReg[2];
	txReg[0] = accReg;
	txReg[1] = command;

	switch(accelDevice.partNumber)
	{
		case LIS2DW:
		case LIS2DUX:
			ret = HAL_I2C_Master_Transmit(&hi2c1, accelDevice.address, txReg, 2, 0xFFFF);

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
			PRINTF("Accelerometer not supported, can not write to register\r\n");
	}
}

/**
 * @brief  This function sets the device part number
 * @note   May be worth checking the address space on i2c bus to check for unknown devices.
 * @param  void
 * @retval void
 */
void accelWhoAmI(void)
{

	accelWriteRegisterRaw(LIS2DUX12_ZERO_REG, WRITE_ZERO);
	HAL_Delay(100);
	uint8_t partNum = accelReadRegister(LIS2_ID);

	PRINTF("Part number: %d\r\n", partNum);
	if(partNum == LIS2DW)
	{
		PRINTF("LIS2DW12 Identified\r\n");
		accelDevice.partNumber = LIS2DW;
		accelDevice.address = LIS2_SLAVE_ADDREESS;
	}
	else if(partNum == LIS2DUX)
	{
		PRINTF("LIS2DUX12 Identified\r\n");
		accelDevice.partNumber = LIS2DUX;
		accelDevice.address = LIS2_SLAVE_ADDREESS;
	}
	else
	{
		PRINTF("No accelerometer identified, setting address unknown\r\n");
		accelDevice.partNumber = ACC_UNKNOWN_DEVICE;
		accelDevice.address = UNKNOWN_SLAVE_ADDRESS;
	}
}

/**
 * @brief  Reads acceleration from device and stores in private structure
 * @note   Must use a getAccelData with a local ACCEL_DATA_TYPE to obtain the value
 * @param  void
 * @retval void
 */
void readAcceleration(void)
{
	HAL_StatusTypeDef ret;
	uint8_t reg;
	uint8_t accel_data[ACCEL_DATA_SIZE];

	// KCS make this a Switch statement, Bomb out if part number is "unknown"
	switch(accelDevice.partNumber)
	{
		case LIS2DW: //Same output registers so I'm falling through
		case LIS2DUX:
			reg = LIS2D_X_L;
			//csc for synchronous operation
			ret = HAL_I2C_Master_Transmit(&hi2c1, accelDevice.address, &reg, 1, 0xFF);
			ret = HAL_I2C_Master_Receive(&hi2c1, accelDevice.address + 1, accel_data, ACCEL_DATA_SIZE, 0xFF);
			if (ret == HAL_OK)
			{
				privateAccelData.Accel_X = (int16_t)(accel_data[1] << 8 | accel_data[0]);  //combine x registers
				privateAccelData.Accel_Y = (int16_t)(accel_data[3] << 8 | accel_data[2]);  //combine y registers
				privateAccelData.Accel_Z = (int16_t)(accel_data[5] << 8 | accel_data[4]);  //combine z registers
			}
			else
			{
				PRINTF("i2c failed on accelerometer\r\n");
			}
			break;
		default:
			PRINTF("Accelerometer not supported, can not read acceleration\r\n");
	}
}

void readAccelFIFO(void)
{
	HAL_StatusTypeDef ret;
	uint8_t reg;
	uint8_t accel_data[ACCEL_DATA_SIZE];

	// KCS make this a Switch statement, Bomb out if part number is "unknown"
	switch(accelDevice.partNumber)
	{
		case LIS2DW:
			reg = LIS2D_X_L;
			//csc for synchronous operation
			ret = HAL_I2C_Master_Transmit(&hi2c1, accelDevice.address, &reg, 1, 0xFF);
			ret = HAL_I2C_Master_Receive(&hi2c1, accelDevice.address + 1, accel_data, ACCEL_DATA_SIZE, 0xFF);
			if (ret == HAL_OK)
			{
				privateAccelData.Accel_X = (int16_t)(accel_data[1] << 8 | accel_data[0]);  //combine x registers
				privateAccelData.Accel_Y = (int16_t)(accel_data[3] << 8 | accel_data[2]);  //combine y registers
				privateAccelData.Accel_Z = (int16_t)(accel_data[5] << 8 | accel_data[4]);  //combine z registers
			}
			else
			{
				PRINTF("i2c failed on accelerometer\r\n");
			}
		case LIS2DUX:
			PRINTF("FIFO reading unimplemented on LIS2DUX\r\n");
			break;
		default:
			PRINTF("Accelerometer not supported, can not read acceleration\r\n");
	}
}
/**
 * @brief  Acceleration data accessor function
 * @note
 * @param  ACCEL_DATA_TYPE extAccelData
 *         Reference to external acceleration data type
 *         bool acquireData
 *         Determines if we want to read the data or just access what is stored in the private data structure
 *         1 acquires data, 0 does not
 * @retval void
 */
void getAccelData(ACCEL_DATA_TYPE *extAccelData, bool acquireData)
{
	// TODO kcs handle unknown part number here
	if(acquireData)
	{
		readAcceleration();
	}

	extAccelData->Accel_X = privateAccelData.Accel_X;
	extAccelData->Accel_Y = privateAccelData.Accel_Y;
	extAccelData->Accel_Z = privateAccelData.Accel_Z;
}

/**
 * @brief  Accelerometer clear interrupt latch without handling
 * @note
 * @param  void
 * @retval void
 */
void accelClearLatch(void)
{
	uint8_t sourceVal;
	uint8_t wakeVal;
	//PRINTF("Wake Value: %d\r\n", wakeVal);
	uint8_t statusVal;
	uint8_t counter = 0;
	switch(accelDevice.partNumber)
	{
		case LIS2DW:
			sourceVal = accelReadRegister(LIS2DW_ALL_INT_SRC);
			wakeVal = accelReadRegister(LIS2DW_WAKE_SRC);
			//PRINTF("Wake Value: %d\r\n", wakeVal);
			statusVal = accelReadRegister(LIS2DW_STATUS_REG);
			//PRINTF("Status Value: %d\r\n", statusVal);
			sourceVal = accelReadRegister(LIS2DW_ALL_INT_SRC);

			while(sourceVal != 0)
			{
				sourceVal = accelReadRegister(LIS2DW_ALL_INT_SRC);
				wakeVal = accelReadRegister(LIS2DW_WAKE_SRC);
				statusVal = accelReadRegister(LIS2DW_STATUS_REG);
				if(counter == 10)
				{
					break;
				}
				counter++;
			}

			if(sourceVal == 0)
			{
				PRINTF("Latched interrupt cleared\r\n");
				PRINTF("Source: %d\r\n\r\n", sourceVal);
			}
			else
			{
				PRINTF("Latched interrupt NOT cleared\r\n");
				PRINTF("Source: %d\r\n\r\n", sourceVal);
			}

			accelTrigger = false;
			break;
		case LIS2DUX:
			sourceVal = accelReadRegister(LIS2DUX_ALL_INT_SRC);
			wakeVal = accelReadRegister(LIS2DUX_WAKE_SRC);
			//PRINTF("Wake Value: %d\r\n", wakeVal);
			statusVal = accelReadRegister(LIS2DUX_STATUS_REG);
			//PRINTF("Status Value: %d\r\n", statusVal);
			sourceVal = accelReadRegister(LIS2DUX_ALL_INT_SRC);

			while(sourceVal != 0)
			{
				sourceVal = accelReadRegister(LIS2DUX_ALL_INT_SRC);
				wakeVal = accelReadRegister(LIS2DUX_WAKE_SRC);
				statusVal = accelReadRegister(LIS2DW_STATUS_REG);
				if(counter == 10)
				{
					break;
				}
				counter++;
			}

			if(sourceVal == 0)
			{
				PRINTF("Latched interrupt cleared\r\n");
				PRINTF("Source: %d\r\n\r\n", sourceVal);
			}
			else
			{
				PRINTF("Latched interrupt NOT cleared\r\n");
				PRINTF("Source: %d\r\n\r\n", sourceVal);
			}

			accelTrigger = false;
			break;
		default:
			PRINTF("Accelerometer not supported, can not clear latch\r\n");
	}
}

/**
 * @brief  Accelerometer print function
 * @note
 * @param  bool acquireData
 *         Determines if we want to read the data or just print
 *         1 acquires data, 0 does not
 * @retval void
 */
void printAccelData(bool acquireData)
{
	// TODO kcs handle unknown part number here
	if(acquireData)
	{
		readAcceleration();
	}

	PRINTF("X:%d    Y:%d    Z:%d\r\n", privateAccelData.Accel_X, privateAccelData.Accel_Y, privateAccelData.Accel_Z);
}

/**
 * @brief  Checks full scale range
 * @note   Returns LIS_RANGE_ERROR on any error getting mode from the device
 * @param  void
 * @retval ACCEL_FULL_SCALE  enum representing the power mode
 */
ACCEL_FULL_SCALE getAccelFullScaleRange(void)
{
	uint8_t regVal = 255;
	ACCEL_FULL_SCALE retVal = LIS_RANGE_ERROR;

	switch(accelDevice.partNumber)
	{
		case LIS2DW:
			regVal = accelReadRegister(LIS2DW_CTRL_6);
			if((regVal & ACCEL_FS_MASK) == 0)
			{
				retVal = LIS_RANGE_2;
			}
			else if((regVal & ACCEL_FS_MASK) == 0x10)
			{
				retVal = LIS_RANGE_4;
			}
			else if((regVal & ACCEL_FS_MASK) == 0x20)
			{
				retVal = LIS_RANGE_8;
			}
			else if((regVal & ACCEL_FS_MASK) == 0x30)
			{
				retVal = LIS_RANGE_16;
			}
			break;
		case LIS2DUX:
			PRINTF("LIS2DUX unimplemented\r\n");
			break;
		default:
			PRINTF("Accelerometer not supported, can not get range\r\n");
	}

	return retVal;
}
/**
 * @brief  Checks output data rate of the device
 * @note   Returns LIS_MODE_ERROR on any error getting mode from the device
 * @param  void
 * @retval ACCEL_POWER_MODE  enum representing the power mode
 */
ACCEL_POWER_MODE getAccelMode(void)
{
	uint8_t regVal = 255;
	ACCEL_POWER_MODE retMode = LIS_MODE_ERROR;

	switch(accelDevice.partNumber)
	{
		case LIS2DW:
			regVal = accelReadRegister(LIS2DW_CTRL_1);
			if((regVal & ACCEL_MODE_MASK) == 8)
			{
				retMode = LIS_MODE_SINGLE_DATA;
			}
			else if((regVal & ACCEL_MODE_MASK) == 4)
			{
				retMode = LIS_MODE_HIGH_PERFORMANCE;
			}
			else if((regVal & ACCEL_MODE_MASK) == 0)
			{
				retMode = LIS_MODE_LOW_POWER;
			}
			else
			{
				retMode = LIS_MODE_ERROR;
			}
			break;
		case LIS2DUX:
			PRINTF("LIS2DUX unimplemented\r\n");
			break;
		default:
			PRINTF("Accelerometer not supported, can not get mode\r\n");
	}

	return retMode;
}

/**
 * @brief  Checks output resolution of the device
 * @note   Returns LIS_RESOLUTION_ERROR in any error getting the resolutino from the device
 * @param  void
 * @retval ACCEL_RESOLUTION enumerated type
 */
ACCEL_RESOLUTION getAccelResolution(void)
{
	ACCEL_RESOLUTION retResolution = LIS_RESOLUTION_ERROR;
	ACCEL_POWER_MODE mode = LIS_MODE_ERROR;
	uint8_t regVal = 255;

	switch(accelDevice.partNumber)
	{
		case LIS2DW:
			mode = getAccelMode();
			regVal = accelReadRegister(LIS2DW_CTRL_1);
			if(mode != LIS_MODE_ERROR)
			{
				if(mode == LIS_MODE_HIGH_PERFORMANCE)
				{
					retResolution = LIS_RESOLUTION_14;
				}
				else
				{
					if((regVal & ACCEL_RESOLUTION_MASK) == 0)
					{
						retResolution = LIS_RESOLUTION_12;
					}
					else
					{
						retResolution = LIS_RESOLUTION_14;
					}
				}
			}
			break;
		case LIS2DUX:
			PRINTF("LIS2DUX unimplemented\r\n");
			break;
		default:
			PRINTF("Accelerometer not supported, can not resolution\r\n");
	}

	return retResolution;
}
/**
 * @brief  Checks output data rate of the device
 * @note   Returns LIS_DATA_RATE_ERROR in case of any error receiving the data rate
 * @param  void
 * @retval ACCEL_DATA_RATE enumerated type
 */
ACCEL_DATA_RATE getAccelDataRate(void)
{
	ACCEL_DATA_RATE dataRate = LIS_DATA_RATE_ERROR;
	ACCEL_POWER_MODE mode = LIS_MODE_ERROR;
	uint8_t regVal = 255;

	switch(accelDevice.partNumber)
	{
		case LIS2DW:
			mode = getAccelMode();
			regVal = accelReadRegister(LIS2DW_CTRL_1);
			//High Performance Mode / (Low Power Mode or Single Data mode)
			//0000 POWER_DOWN
			if(mode != LIS_MODE_ERROR)
			{
				if((regVal & ACCEL_ODR_MASK) == 0)
				{
					dataRate = LIS_DATA_RATE_POWER_DOWN;
				}
				//1001 1600/200Hz
				else if((regVal & ACCEL_ODR_MASK) == 0x90)
				{
					if(mode == LIS_MODE_LOW_POWER || mode == LIS_MODE_SINGLE_DATA)
					{
						dataRate = LIS_DATA_RATE_200;
					}
					else if(mode == LIS_MODE_HIGH_PERFORMANCE)
					{
						dataRate = LIS_DATA_RATE_1600;
					}
					else
					{
						dataRate = LIS_DATA_RATE_ERROR;
					}
				}
				//0001 12.5/1.6Hz
				else if((regVal & ACCEL_ODR_MASK) == 0x10)
				{
					if(mode == LIS_MODE_LOW_POWER || mode == LIS_MODE_SINGLE_DATA)
					{
						dataRate = LIS_DATA_RATE_1;
					}
					else if(mode == LIS_MODE_HIGH_PERFORMANCE)
					{
						dataRate = LIS_DATA_RATE_12;
					}
					else
					{
						dataRate = LIS_DATA_RATE_ERROR;
					}
				}
				//0010 12.5Hz
				else if((regVal & ACCEL_ODR_MASK) == 0x20)
				{
					dataRate = LIS_DATA_RATE_12;
				}
				//0011 25Hz
				else if((regVal & ACCEL_ODR_MASK) == 0x30)
				{
					dataRate = LIS_DATA_RATE_25;
				}
				//0100 50Hz
				else if((regVal & ACCEL_ODR_MASK) == 0x40)
				{
					dataRate = LIS_DATA_RATE_50;
				}
				//0101 100Hz
				else if((regVal & ACCEL_ODR_MASK) == 0x50)
				{
					dataRate = LIS_DATA_RATE_100;
				}
				//0110 200Hz
				else if((regVal & ACCEL_ODR_MASK) == 0x60)
				{
					dataRate = LIS_DATA_RATE_200;
				}
				//0111 400/200Hz
				else if((regVal & ACCEL_ODR_MASK) == 0x70)
				{
					if(mode == LIS_MODE_LOW_POWER || mode == LIS_MODE_SINGLE_DATA)
					{
						dataRate = LIS_DATA_RATE_200;
					}
					else if(mode == LIS_MODE_HIGH_PERFORMANCE)
					{
						dataRate = LIS_DATA_RATE_400;
					}
					else
					{
						dataRate = LIS_DATA_RATE_ERROR;
					}
				}
				//1000 800/200Hz
				else if((regVal & ACCEL_ODR_MASK) == 0x80)
				{
					if(mode == LIS_MODE_LOW_POWER || mode == LIS_MODE_SINGLE_DATA)
					{
						dataRate = LIS_DATA_RATE_200;
					}
					else if(mode == LIS_MODE_HIGH_PERFORMANCE)
					{
						dataRate = LIS_DATA_RATE_800;
					}
					else
					{
						dataRate = LIS_DATA_RATE_ERROR;
					}
				}
			}
			break;
		case LIS2DUX:
			PRINTF("LIS2DUX unimplemented\r\n");
			break;
		default:
			PRINTF("Accelerometer not supported, can not get data rate\r\n");
	}
	return dataRate;
}

/**
 * @brief  Sets full scale range of accelerometer
 * @note
 * @param  ACCEL_FULL_SCALE setRange
 * @retval void
 */
void setAccelFullScaleRange(ACCEL_FULL_SCALE setRange)
{
	uint8_t writeVal = 0;
	uint8_t regVal = 255;
	uint8_t mask;

	switch(accelDevice.partNumber)
	{
		case LIS2DW:
			regVal = accelReadRegister(LIS2DW_CTRL_1);
			mask = regVal & ACCEL_FS_SET_MASK;
			switch(setRange)
			{
				case LIS_RANGE_2:
					writeVal = mask;
					accelWriteRegisterRaw(LIS2DW_CTRL_6, writeVal);
					break;
				case LIS_RANGE_4:
					writeVal = mask + 0x10;
					accelWriteRegisterRaw(LIS2DW_CTRL_6, writeVal);
					break;
				case LIS_RANGE_8:
					writeVal = mask + 0x20;
					accelWriteRegisterRaw(LIS2DW_CTRL_6, writeVal);
					break;
				case LIS_RANGE_16:
					writeVal = mask + 0x30;
					accelWriteRegisterRaw(LIS2DW_CTRL_6, writeVal);
					break;
				default:
					PRINTF("Invalid range passed. This should never happen\r\n");
			}
			break;
		case LIS2DUX:
			PRINTF("LIS2DUX unimplemented\r\n");
			break;
		default:
			PRINTF("Accelerometer not supported, can not set range\r\n");
	}
}

/**
 * @brief  Sets data rate of accelerometer
 * @note   ODR depends on power mode selection, best practice is to set the mode first
 * @param  ACCEL_DATA_RATE setRate
 * @retval void
 */
void setAccelDataRate(ACCEL_DATA_RATE setRate)
{
	uint8_t writeVal = 0;
	uint8_t regVal = 255;
	ACCEL_POWER_MODE mode = LIS_MODE_ERROR;
	uint8_t mask;

	switch(accelDevice.partNumber)
	{
		case LIS2DW:
			mode = getAccelMode();
			regVal = accelReadRegister(LIS2DW_CTRL_1);
			mask = ACCEL_RESOLUTION_MASK + ACCEL_MODE_MASK;

			if(mode != LIS_MODE_ERROR)
			{
				writeVal = mask & regVal; //Zeros out the ODR bits
				switch (setRate)
				{
					case LIS_DATA_RATE_POWER_DOWN:
						accelWriteRegisterRaw(LIS2DW_CTRL_1, writeVal);
						break;

					case LIS_DATA_RATE_1:
						if(mode != LIS_MODE_HIGH_PERFORMANCE)
						{
							writeVal += (1 << 4);
							accelWriteRegisterRaw(LIS2DW_CTRL_1, writeVal);
						}
						else
						{
							PRINTF("Can not set 1.6Hz ODR from high performance mode. ODR set to 12.5Hz\r\n");
							writeVal += (2 << 4);
							accelWriteRegisterRaw(LIS2DW_CTRL_1, writeVal);
						}
						break;

					case LIS_DATA_RATE_12:
						writeVal += (2 << 4);
						accelWriteRegisterRaw(LIS2DW_CTRL_1, writeVal);
						break;

					case LIS_DATA_RATE_25:
						writeVal += (3 << 4);
						accelWriteRegisterRaw(LIS2DW_CTRL_1, writeVal);
						break;

					case LIS_DATA_RATE_50:
						writeVal += (4 << 4);
						accelWriteRegisterRaw(LIS2DW_CTRL_1, writeVal);
						break;

					case LIS_DATA_RATE_100:
						writeVal += (5 << 4);
						accelWriteRegisterRaw(LIS2DW_CTRL_1, writeVal);
						break;

					case LIS_DATA_RATE_200:
						writeVal += (6 << 4);
						accelWriteRegisterRaw(LIS2DW_CTRL_1, writeVal);
						break;
					case LIS_DATA_RATE_400:
						if(mode != LIS_MODE_HIGH_PERFORMANCE)
						{
							PRINTF("Can not set ODR above 200Hz in low power mode. ODR set to 200Hz\r\n");
							writeVal += (6 << 4);
							accelWriteRegisterRaw(LIS2DW_CTRL_1, writeVal);
						}
						else
						{
							writeVal += (7 << 4);
							accelWriteRegisterRaw(LIS2DW_CTRL_1, writeVal);
						}
						break;

					case LIS_DATA_RATE_800:
						if(mode != LIS_MODE_HIGH_PERFORMANCE)
						{
							PRINTF("Can not set ODR above 200Hz in low power mode. ODR set to 200Hz\r\n");
							writeVal += (6 << 4);
							accelWriteRegisterRaw(LIS2DW_CTRL_1, writeVal);
						}
						else
						{
							writeVal += (8 << 4);
							accelWriteRegisterRaw(LIS2DW_CTRL_1, writeVal);
						}
						break;

					case LIS_DATA_RATE_1600:
						if(mode != LIS_MODE_HIGH_PERFORMANCE)
						{
							PRINTF("Can not set ODR above 200Hz in low power mode. ODR set to 200Hz\r\n");
							writeVal += (6 << 4);
							accelWriteRegisterRaw(LIS2DW_CTRL_1, writeVal);
						}
						else
						{
							writeVal += (9 << 4);
							accelWriteRegisterRaw(LIS2DW_CTRL_1, writeVal);
						}
						break;
					default:
						PRINTF("Invalid data rate passed to setAccelDataRate. This should never happen\r\n");
				}
			}
			else
			{
				PRINTF("Accel mode error detected when trying to set ODR. This should never happen\r\n");
			}
			break;
		case LIS2DUX:
			PRINTF("LIS2DUX unimplemented\r\n");
			break;
		default:
			PRINTF("Accelerometer not supported, can not set rate\r\n");
	}
}

/**
 * @brief  Sets power mode of accelerometer
 * @note   Resolution and data rate depend on this power mode
 * 		   Best practice is to set the mode before setting either resolution or ODR
 * 		   Not sure if we will ever use single data mode but I included it in both set and get mode functions
 * @param  ACCEL_POWER_MODE setMode
 * @retval void
 */
void setAccelMode(ACCEL_POWER_MODE setMode)
{
	uint8_t writeVal = 0;
	uint8_t regVal = 255;
	uint8_t mask = 0;

	switch(accelDevice.partNumber)
	{
		case LIS2DW:
			regVal = accelReadRegister(LIS2DW_CTRL_1);
			mask = ACCEL_ODR_MASK + ACCEL_RESOLUTION_MASK;
			writeVal = mask & regVal; //Zeros out the mode bits
			switch (setMode)
			{
				case LIS_MODE_LOW_POWER:
					accelWriteRegisterRaw(LIS2DW_CTRL_1, writeVal);
					break;
				case LIS_MODE_HIGH_PERFORMANCE:
					writeVal += (1 << 2);
					accelWriteRegisterRaw(LIS2DW_CTRL_1, writeVal);
					break;
				case LIS_MODE_SINGLE_DATA:
					writeVal += (2 << 2);
					accelWriteRegisterRaw(LIS2DW_CTRL_1, writeVal);
					break;
				default:
					PRINTF("Invalid mode passed during set mode. Mode is unchanged\r\n");
					break;
			}
			break;
		case LIS2DUX:
			PRINTF("LIS2DUX unimplemented\r\n");
			break;
		default:
			PRINTF("Accelerometer not supported, can not set mode\r\n");
	}
}

/**
 * @brief  Sets resolution of accelerometer
 * @note   Resolution is dependent on power mode, best practice is to set the power mode first
 * 		   You can not set 12-bit resolution in high performance mode, this is reflected in the code
 * 		   If in LP mode and resolution is set to 14 bit, 01 is written in the resolution bits
 * 		   There are other modes all at 14 bit resolution and all drawing more power.
 * 		   Expansion for these modes will be added if necessary but complexity will severely increase.
 * @param  ACCEL_RESOLUTION setResolution
 * @retval void
 */
void setAccelResolution(ACCEL_RESOLUTION setResolution)
{
	uint8_t writeVal = 0;
	uint8_t regVal = 255;
	ACCEL_POWER_MODE mode = LIS_MODE_ERROR;
	uint8_t mask = 0;

	switch(accelDevice.partNumber)
	{
		case LIS2DW:
			regVal = accelReadRegister(LIS2DW_CTRL_1);
			mode = getAccelMode();
			mask = ACCEL_ODR_MASK + ACCEL_MODE_MASK;
			if(mode != LIS_MODE_ERROR)
			{
				writeVal = mask & regVal; //Zeros out the resolution bits
				if((mode == LIS_MODE_HIGH_PERFORMANCE) && (setResolution == LIS_RESOLUTION_12))
				{
					PRINTF("Can not set 12 bit resolution in high performance mode. Setting LP_MODE 2 for resolution\r\n");
					writeVal += 1;
					accelWriteRegisterRaw(LIS2DW_CTRL_1, writeVal);
				}
				else if((mode == LIS_MODE_LOW_POWER) && (setResolution == LIS_RESOLUTION_12))
				{
					accelWriteRegisterRaw(LIS2DW_CTRL_1, writeVal);
				}
				else if((mode == LIS_MODE_LOW_POWER) && (setResolution == LIS_RESOLUTION_14))
				{
					writeVal += 1;
					accelWriteRegisterRaw(LIS2DW_CTRL_1, writeVal);
				}
				else if((mode == LIS_MODE_HIGH_PERFORMANCE) && (setResolution == LIS_RESOLUTION_14))
				{
					writeVal += 3;
					accelWriteRegisterRaw(LIS2DW_CTRL_1, writeVal);
				}
				else
				{
					PRINTF("Invalid resolution being set. Resolution unchanged\r\n");
				}
			}
			else
			{
				PRINTF("Accel mode error detected when trying to set resolution. This should never happen\r\n");
			}
			break;
		case LIS2DUX:
			PRINTF("LIS2DUX unimplemented\r\n");
			break;
		default:
			PRINTF("Accelerometer not supported, can not set resolution\r\n");
	}
}
/**
 * @brief  Acceleration parameters accessor function
 * @note
 * @param  ACCELERATION_PARAM_TYPE extAccelParameters
 *         Reference to external acceleration parameter type
 * @retval void
 */
void getAccelParameters(ACCELERATION_PARAM_TYPE *extAccelParameters)
{
	extAccelParameters->hysteresis = privateAccelParameters.hysteresis;
	extAccelParameters->mode = privateAccelParameters.mode;
	extAccelParameters->range = privateAccelParameters.range;
	extAccelParameters->threshold = privateAccelParameters.threshold;
	extAccelParameters->duration = privateAccelParameters.duration;
	extAccelParameters->mutePeriod = privateAccelParameters.mutePeriod;
}

/**
 * @brief  Acceleration parameters setter function
 * @note
 * @param  ACCELERATION_PARAM_TYPE extAccelParameters
 *         External acceleration parameter type
 * @retval void
 */
void setAccelParameters(ACCELERATION_PARAM_TYPE extAccelParameters)
{
	privateAccelParameters.hysteresis = extAccelParameters.hysteresis;
	privateAccelParameters.mode = extAccelParameters.mode;
	privateAccelParameters.range = extAccelParameters.range;
	privateAccelParameters.threshold = extAccelParameters.threshold;
	privateAccelParameters.duration = extAccelParameters.duration;
	privateAccelParameters.mutePeriod = extAccelParameters.mutePeriod;
}

/**
 * @brief  Handles accelerometer triggered event
 * @note   Reads all interrupt source register 0x3B and does bitwise comparison to see which event
 * 		   caused the device to trigger
 * @param  Void
 * @retval void
 */
void handleAccelTrigger(void)
{
	switch(accelDevice.partNumber)
	{
		case LIS2DW:
			uint8_t sourceVal = accelReadRegister(LIS2DW_ALL_INT_SRC);

			//	uint8_t sourceVal = accelReadRegister(LIS2DW_STATUS_REG);

			//CSC handles case where nothing is detected. 63 is the max value this register can be
			if(sourceVal == 0)
			{
				PRINTF("No source detected: %d\r\n", sourceVal);
			}
			else if(sourceVal > 63)
			{
				PRINTF("Interrupt source value invalid: %d\r\n", sourceVal);
			}

			//CSC not mutually exclusive, can't use else if
			if((sourceVal & 1) == 1)
			{
				//Handle Free Fall event detection
				PRINTF("Free Fall detected\r\n\r\n");
			}

			if((sourceVal & 2) == 2)
			{
				//Handle Wake-Up Event
				PRINTF("Wake-Up Event Detected: %d\r\n", sourceVal);
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET);
				handleAccelWakeUp();
			}

			if((sourceVal & 4) == 4)
			{
				//Handle Single Tap
			}

			if((sourceVal & 8) == 8)
			{
				//Handle Double Tap
			}

			if((sourceVal & 16) == 16)
			{
				//Handle 6D-Orientation
			}

			if((sourceVal & 32) == 32)
			{
				//Handle sleep change status
			}

			sourceVal = accelReadRegister(LIS2DW_ALL_INT_SRC);
			PRINTF("New source val: %d\r\n", sourceVal);
			HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET);
			accelTrigger = false; //Reset trigger
			break;
		case LIS2DUX:
			PRINTF("LIS2DUX unimplemented\r\n");
			break;
		default:
			PRINTF("Accelerometer not supported, no interrupt handler\r\n");
	}
}

/**
 * @brief  Handles the case of a Wake Up event
 * @note   Reads WAKE_SRC register 0x38 to determine cause of wake up
 * @param  void
 * @retval void
 */
void handleAccelWakeUp(void)
{
	switch(accelDevice.partNumber)
	{
		case LIS2DW:
			uint8_t wakeVal = accelReadRegister(LIS2DW_WAKE_SRC);
			PRINTF("Wake Value: %d\r\n", wakeVal);
			uint8_t statusVal = accelReadRegister(LIS2DW_STATUS_REG);
			PRINTF("Status Value: %d\r\n", statusVal);

			//CSC not mutually exclusive, can't use else if
			if((wakeVal & 4) == 4)
			{
				PRINTF("X-Axis over threshold\r\n");
			}

			if((wakeVal & 2) == 2)
			{
				PRINTF("Y-Axis over threshold\r\n");
			}

			if((wakeVal & 1) == 1)
			{
				PRINTF("Z-Axis over threshold\r\n");
			}
			break;
		case LIS2DUX:
			PRINTF("LIS2DUX unimplemented\r\n");
			break;
		default:
			PRINTF("Accelerometer not supported, can not handle interrupt\r\n");
	}
}

void testFIFO()
{
	accelWriteRegisterCommand(LIS2DW_CTRL_2, LIS2DW_SOFT_RESET); //Reset
	setAccelMode(LIS_MODE_HIGH_PERFORMANCE); //Set power mode
	setAccelResolution(LIS_RESOLUTION_14); //Set resolution
	setAccelDataRate(LIS_DATA_RATE_200); //Set output data rate
	accelWriteRegisterCommand(LIS2DW_FIFO_CTRL_REG, LIS2DW_FIFO_CTRL_SET);

	while(1)
	{
		uint8_t status = accelReadRegister(LIS2DW_FIFO_STATUS_REG);
		PRINTF("FIFO Status: %u\r\n\r\n", status);
		if(status != 0)
		{
			readAccelFIFO();
			printAccelData(0);
		}
	}
}
void testWakeUpInterruptOccur(void)
{
	accelInit();
	while(1)
	{
		Refresh_Watchdog;
		//printAccelData(1);
		if(accelTrigger)
		{
			//handleAccelTrigger();
			PRINTF("We are interrupting on LIS2DUX\r\n\r\n");
		}
	}
}

void testMode(void)
{
	accelWriteRegisterCommand(LIS2DW_CTRL_2, LIS2DW_REBOOOT);
	accelWriteRegisterCommand(LIS2DW_CTRL_2, LIS2DW_SOFT_RESET); //Reset
	setAccelMode(LIS_MODE_LOW_POWER); //Set power mode
	setAccelResolution(LIS_RESOLUTION_14); //Set resolution
	setAccelDataRate(LIS_DATA_RATE_200); //Set output data rate
	for(int i = 0; i < 1000; i++)
	{
		printAccelData(1);
	}

	PRINTF("DONE READING\r\n\r\n");
	accelInit();
}

//stores accelerometer data
void storeAccelAxisDataToReportingArray( MEM_PTR *Data_Ptr )
{
//	if(GET_ODR_MODE( Data_Ptr ) != LIS2DW_CTRL_1_SET_200HZ)
//	{
//		Command = STORE_ACC_CONFIG;
//		ACC_COMMAND( Data_Ptr );
//	}
//
//	Command = ACCELERATION;
//	ACC_COMMAND(Data_Ptr);
	//writeDataToReportingArray(0, ACC_TAG, ACC_SPECIFIER, X_Axis, Y_Axis, Z_Axis);
}

/**
 * @brief  Decodes accelerometer parameters from server
 * @note
 * @param  uint8_t *mqttMsg
 * 		   Represents message to be decoded
 * @retval bool isError
 * 		   If no error, will return false
 */
bool decodeAccelConfigs(uint8_t *mqttMsg)
{
	bool isError = false;
	char Buff[MEMORY_MAX] = " ";
	char test [] = "\"accelerometer\":{";
	char verTest[] = "\"version\":";
	char modeTest [] = "\"mode\":";
	char rangeTest [] = "\"range\":";
	char thresholdTest [] = "\"threshold\":";
	char hysteresisTest [] = "\"hysteresis\":";
	char durationTest [] = "\"duration\":";
	char mutingTest [] = "\"muting_period\":";
	float hysteresis = 0.0;
	bool hysterValid = false;
 	uint16_t mode = 0;
 	bool modeValid = false;
	uint32_t range = 0;
	bool rangeValid = false;
	float threshold = 0.0;
	bool threshValid = false;
	uint32_t duration = 0;
	bool durationValid = false;
	uint8_t version;
	uint8_t mute = 0;
	bool muteValid = false;

	char accelErrStr[CONFIG_ERR_MSG_SIZE] = "";
	int buffSize = 0;
	// Transfer MQTT message to a local buffer
	Word_Transfer(Buff, (char*)mqttMsg);

	char* substr = strstr(Buff, test);
	if(substr)
	{
		buffSize += snprintf(accelErrStr, CONFIG_ERR_MSG_SIZE, "\"accelerometer\":[\"config_error\",");
		char *verStr = strstr(substr, verTest);
		if(verStr)
		{
			verStr += strlen(verTest);
			if(isdigit((unsigned char)verStr[0]))
			{
				version = atoi(verStr);
				if(version == ACCEL_CONFIG_VERSION)
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
							}
							else
							{
								PRINTF("Accelerometer mode is out of range [0-255]: %d\r\n", mode);
								isError = true;
								buffSize += snprintf((accelErrStr + buffSize), (CONFIG_ERR_MSG_SIZE - buffSize), "\"mode_out_of_range_[0-255]\",");
							}
						}
						else
						{
							isError = true;
							PRINTF("Invalid data type for accelerometer mode\r\n");
							buffSize += snprintf((accelErrStr + buffSize), (CONFIG_ERR_MSG_SIZE - buffSize), "\"invalid_mode_type_NAN\",");
						}
					}
					else
					{
						isError = true;
						PRINTF("Mode not found in accel parameters string\r\n");
						buffSize += snprintf((accelErrStr + buffSize), (CONFIG_ERR_MSG_SIZE - buffSize), "\"missing_mode_string\",");
					}

					char *rangeStr = strstr(substr, rangeTest);
					if(rangeStr)
					{
						rangeStr += strlen(rangeTest);
						if(isdigit((unsigned char)rangeStr[0]))
						{
							range = atoi(rangeStr);
							if(range >= 0 && range <= 255)
							{
								rangeValid = true;
							}
							else
							{
								PRINTF("Accelerometer range is out of range [0-255]: %d\r\n", range);
								isError = true;
								buffSize += snprintf((accelErrStr + buffSize), (CONFIG_ERR_MSG_SIZE - buffSize), "\"range_out_of_range_[0-255]\",");
							}
						}
						else
						{
							isError = true;
							PRINTF("Invalid data type for accelerometer range");
							buffSize += snprintf((accelErrStr + buffSize), (CONFIG_ERR_MSG_SIZE - buffSize), "\"invalid_range_type_NAN\",");
						}
					}
					else
					{
						isError = true;
						PRINTF("Range not found in accel parameters string\r\n");
						buffSize += snprintf((accelErrStr + buffSize), (CONFIG_ERR_MSG_SIZE - buffSize), "\"missing_range_string\",");
					}
					char *thresholdStr = strstr(substr, thresholdTest);
					if(thresholdStr)
					{
						thresholdStr += strlen(thresholdTest);
						if(isdigit((unsigned char)thresholdStr[0]))
						{
							threshold = atof(thresholdStr);
							if(threshold >= 0.0 && threshold <= 65535.0)
							{
								threshValid = true;
							}
							else
							{
								PRINTF("Accelerometer threshold is out of range [0.0-65535.0]: %f\r\n", threshold);
								isError = true;
								buffSize += snprintf((accelErrStr + buffSize), (CONFIG_ERR_MSG_SIZE - buffSize), "\"threshold_out_of_range[0-65535]\",");
							}
						}
						else
						{
							isError = true;
							PRINTF("Invalid data type for accelerometer threshold\r\n");
							buffSize += snprintf((accelErrStr + buffSize), (CONFIG_ERR_MSG_SIZE - buffSize), "\"invalid_threshold_type_NAN\",");
						}
					}
					else
					{
						isError = true;
						PRINTF("Threshold not found in accel parameters string\r\n");
						buffSize += snprintf((accelErrStr + buffSize), (CONFIG_ERR_MSG_SIZE - buffSize), "\"missing_threshold_string\",");
					}

					char *hysteresisStr = strstr(substr, hysteresisTest);
					if(hysteresisStr)
					{
						hysteresisStr += strlen(hysteresisTest);
						if(isdigit((unsigned char)hysteresisStr[0]))
						{
							hysteresis = atof(hysteresisStr);
							if(hysteresis >= 0.0 && hysteresis <= 65535.0)
							{
								hysterValid = true;
							}
							else
							{
								PRINTF("Accelerometer hysteresis is out of range [0.0-65535.0]: %f\r\n", hysteresis);
								isError = true;
								buffSize += snprintf((accelErrStr + buffSize), (CONFIG_ERR_MSG_SIZE - buffSize), "\"hysteresis_out_of_range[0-65535]\",");
							}
						}
						else
						{
							isError = true;
							PRINTF("Invalid data type for accelerometer hysteresis\r\n");
							buffSize += snprintf((accelErrStr + buffSize), (CONFIG_ERR_MSG_SIZE - buffSize), "\"invalid_hysteresis_type_NAN\",");
						}
					}
					else
					{
						isError = true;
						PRINTF("Hysteresis not found in accel parameters string\r\n");
						buffSize += snprintf((accelErrStr + buffSize), (CONFIG_ERR_MSG_SIZE - buffSize), "\"missing_hysteresis_string\",");
					}

					char *durationStr = strstr(substr, durationTest);
					if(durationStr)
					{
						durationStr += strlen(durationTest);
						if(isdigit((unsigned char)durationStr[0]))
						{
							duration = atoi(durationStr);
							if(duration >= 0 && duration <= 65535)
							{
								durationValid = true;
							}
							else
							{
								PRINTF("Accelerometer duration is out of range [0-65535]: %d\r\n", duration);
								isError = true;
								buffSize += snprintf((accelErrStr + buffSize), (CONFIG_ERR_MSG_SIZE - buffSize), "\"duration_out_of_range_[0-65535]\",");
							}
						}
						else
						{
							isError = true;
							PRINTF("Invalid data type for accelerometer duration\r\n");
							buffSize += snprintf((accelErrStr + buffSize), (CONFIG_ERR_MSG_SIZE - buffSize), "\"invalid_duration_type_NAN\",");
						}
					}
					else
					{
						isError = true;
						PRINTF("Duration not found in accel parameters string\r\n");
						buffSize += snprintf((accelErrStr + buffSize), (CONFIG_ERR_MSG_SIZE - buffSize), "\"missing_duration_string\",");
					}

					char* muteStr = strstr(substr, mutingTest);
					if(muteStr)
					{
						muteStr += strlen(mutingTest);
						if(isdigit((unsigned char)muteStr[0]))
						{
							mute = atoi(muteStr);
							if(mute >= 0 && mute <= 65535)
							{
								muteValid = true;
							}
							else
							{
								PRINTF("Accelerometer mute period is out of range [0-65535]: %d\r\n", mute);
								isError = true;
								buffSize += snprintf((accelErrStr + buffSize), (CONFIG_ERR_MSG_SIZE - buffSize), "\"mute_period_out_of_range_[0-65535]\",");
							}
						}
						else
						{
							isError = true;
							PRINTF("Invalid data type for accelerometer mute period\r\n");
							buffSize += snprintf((accelErrStr + buffSize), (CONFIG_ERR_MSG_SIZE - buffSize), "\"invalid_mute_period_type_NAN\",");
						}
					}
					else
					{
						isError = true;
						PRINTF("Mute period not found in accel parameters string\r\n");
						buffSize += snprintf((accelErrStr + buffSize), (CONFIG_ERR_MSG_SIZE - buffSize), "\"missing_mute_period_string\",");
					}
				}
				else
				{
					isError = true;
					PRINTF("Incorrect version number: %d\r\n", version);
					buffSize += snprintf((accelErrStr + buffSize), (CONFIG_ERR_MSG_SIZE - buffSize), "\"version_mismatch\",");
				}
			}
			else
			{
				isError = true;
				PRINTF("Invalid data type for accel version\r\n");
				buffSize += snprintf((accelErrStr + buffSize), (CONFIG_ERR_MSG_SIZE - buffSize), "\"invalid_version_type_NAN\",");
			}
		}
		else
		{
			isError = true;
			PRINTF("No version found\r\n");
			buffSize += snprintf((accelErrStr + buffSize), (CONFIG_ERR_MSG_SIZE - buffSize), "\"missing_version_string\",");
		}
	}
	else
	{
		isError = true;
		PRINTF("Accelerometer parameters not found in decodeAccelConfigs\r\n");
	}

	if(!isError && modeValid && rangeValid && threshValid && hysterValid && durationValid && muteValid)
	{
		privateAccelParameters.mode = mode;
		privateAccelParameters.range = range;
		privateAccelParameters.mutePeriod = mute;
		privateAccelParameters.duration = duration;
		uint32_t threshInt = (uint32_t)(threshold * 1000.0);
		privateAccelParameters.threshold = threshInt;
		uint32_t hysterInt = (uint32_t)(hysteresis * 1000.0);
		privateAccelParameters.hysteresis = hysterInt;
	}
	else
	{
		if(buffSize > 0 && buffSize < CONFIG_ERR_MSG_SIZE - 2 && accelErrStr[0] != '\0')
		{
			if(accelErrStr[buffSize - 1] == ',')
			{
				accelErrStr[buffSize - 1] = ']';
				addErrorString(accelErrStr);
			}
		}
	}

	return isError;
}

/**
 * @brief  Returns configuration string with accelerometer configs for status message
 * @note
 * @param  void
 * @retval returns config string or an error message if something goes wrong
 */
char* getAccelConfigStr(void)
{
	static char accelBuff[ACCEL_MSG_SIZE] = {0};
	static char failed[500] = "Failed to build location message\r\n";
	char timebuff[100] = {""}; //
	int buffSize = 0;

	Time_StampISO(timebuff);

	float thresh = (float)privateAccelParameters.threshold / 1000.0;
	float hyster = (float) privateAccelParameters.hysteresis / 1000.0;
	buffSize = snprintf(accelBuff, ACCEL_MSG_SIZE, "\"accelerometer\":{\"version\":%u,\"mode\":%u,\"range\":%u,\"threshold\":%g,\"hysteresis\":%g,\"duration\":%u,\"muting_period\":%u}",
			ACCEL_CONFIG_VERSION, privateAccelParameters.mode, privateAccelParameters.range, thresh, hyster, privateAccelParameters.duration, privateAccelParameters.mutePeriod);

	if(buffSize > 0 && buffSize < ACCEL_MSG_SIZE)
	{
		return accelBuff;
	}
	else
	{
		return failed;
	}
}

/**
 * @brief  Accessor function for Accelerometer cooldown (mute) period
 * @note
 * @retval uint16_t mutePeriod
 */
uint16_t getAccelMutePeriod(void)
{
	return privateAccelParameters.mutePeriod;
}

#endif  //ACCELEROMETER_C_
