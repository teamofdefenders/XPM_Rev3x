/**
  ******************************************************************************
  * @file    Accelerometer.h
  * @author  Erik Pineda-A and Connor Coultas
  * @brief   driver for accelerometer
  ******************************************************************************
  * @attention
  *
  * Copyright 2025 Team of Defenders
  *
  ******************************************************************************
  */
#ifndef ACCELEROMETER_H_
#define ACCELEROMETER_H_

/******************************************************
 File Includes
 Note: n/a
 ******************************************************/

#include "Functions.h"

/******************************************************
 Define Values
 Note: Values Gathered from Datasheet
 ******************************************************/
#define DEF_MODE_ACCEL 3
#define DEF_RANGE_ACCEL 1
#define DEF_THRESH_ACCEL 5.0
#define DEF_HYSTERESIS_ACCEL 0.02
#define DEF_DURATION 300
#define DEF_MUTE_ACCEL 900
#define ACCEL_TIMER 0xFF
#define ACCEL_MSG_SIZE 2000
#define ACCEL_CONFIG_VERSION 1

#define ACCEL_SENSOR_FAIL 0x01 //WHY!! KCS USE maybe UNKNOWN_ADDRESS
#define Default_X_Axis 0xFF
#define Default_Y_Axis 0xFF
#define Default_Z_Axis 0xFF
#define Update_X_Axis 0xF3
#define Update_Y_Axis 0xF3
#define Update_Z_Axis 0xF3
#define ACCEL_REG_MASK 0xFF
#define ACCEL_ODR_MASK 0xF0
#define ACCEL_MODE_MASK 0x0C
#define ACCEL_FS_MASK 0x30
#define ACCEL_FS_SET_MASK 0xCC
#define ACCEL_RESOLUTION_MASK 0x03

#define LOG_ACCELEROMETER "\1 Accelerometer Command : \0"	// Start of UART MSG
#define LOG_X_SPEED " X-Axis Speed : \0"	// Start of UART MSG
#define LOG_Y_SPEED " Y-Axis Speed : \0"	// Start of UART MSG
#define LOG_Z_SPEED " Z-Axis Speed : \0"	// Start of UART MSG

#define LIS2DW_CTRL_2_SET (1<<3)			// Setting Config for Wake Up
#define ACCEL_DATA_SIZE 6

/******************************************************
 Global Enum
 Note: Values are used for error codes,
 command types, and XPS state
 ******************************************************/
typedef enum
{
	LIS2_SLAVE_ADDREESS   = 0x32,	// ALT : 0x30
	UNKNOWN_SLAVE_ADDRESS = 0x00
} ACCEL_I2C_Address;

typedef enum
{
	LIS2DW = 0x44,              // WhoamI 0x44
	LIS2DUX = 0x47,             // WhoamI 0x47
	ACC_UNKNOWN_DEVICE = 0x00
} ACCEL_DEVICE_TYPE;

typedef enum
{
	LIS2_ID = 0x0F,	         // WHO_AM_I from chip id
	LIS2DW_CTRL_1_INST = 0x26,   // Control 1 instruction
//	LIS2DW_CTRL_3 = 0x22,	     // Control : Self Test, Single Data Mode CTRL
//	LIS2DW_CTRL_5 = 0x24,	     // Control : Cont. CTRL 4
	LIS2D_X_L = 0x28,	         // Low X axis
	LIS2D_X_H = 0x29,	         // High X axis
	LIS2D_Y_L = 0x2A,	         // Low Y axis
	LIS2D_Y_H = 0x2B,	         // High Y axis
	LIS2D_Z_L = 0x2C,	         // Low Z axis
	LIS2D_Z_H = 0x2D,	         // High Z axis
	LIS2DW_ALL_INT_SRC = 0x3B,	 // Address for device Wake Up
	LIS2DW_FIFO_CTRL_REG = 0x2E, //FIFO control register address
	LIS2DW_FIFO_CTRL_SET = 0xC0, //Continuous mode 0 threshold
	LIS2DW_FIFO_STAT = 0x37,
	LIS2DW_FIFO_STATUS_REG = 0x2F,
	LIS2DW_CTRL_1 = 0x20,           // Address for device Wake Up
	LIS2DW_CTRL_2 = 0x21,           // Address for device Wake Up
	LIS2DW_CTRL_3 = 0x22,           // Address for device Wake Up
	LIS2DW_CTRL_3_LIR = 0x10,		// Sets latched interrupts
	LIS2DW_INT1_CTRL_4 = 0x23,		// Address for device GPIO
	LIS2DW_CTRL_4_SET = 0b00110000, //Sets wake up and free fall detection
	LIS2DW_CTRL_5 = 0x24,           // Address for device GPIO
	LIS2DW_CTRL_5_SET = 0x01,       // Setting Config for GPIO
	LIS2DW_CTRL_6 = 0x25,           // Address for device GPIO
	LIS2DW_CTRL_6_SET = 0x04,       // Setting Config for GPIO
	LIS2DW_THRS = 0x34,             // Address for device Wake Up
	LIS2DW_THRS_SET = 0x0F,         // Setting Config for Wake Up
	LIS2DW_WAKE_THRESH_REG = 0x34,  // Address for device Wake Up
	LIS2DW_SLEEP_SET = 0x02,        // Setting Config for Wake Up
	LIS2DW_WAKE_DURATION = 0x35,    // Address for device Wake Up
	LIS2DW_WAKE_SET = 0x80,         // Setting Config for Wake Up
	LIS2DW_FF_REG = 0x36,           // Free fall control register
	LIS2DW_FREE_SET = 0b00100101,   // Setting Config for Wake Up
	LIS2DW_WAKE_SRC = 0x38,         // Address for device Wake Up Source
	LIS2DW_GPIO_SET = 0x20,         // Setting Config for Wake Up
	LIS2DW_INT= 0x3F,               // Address for device Wake Up
	LIS2DW_INT_SET= 0x60,           // Setting Config for Wake Up
	LIS2DW_CTRL_7 = 0x3F,           // Address for device Wake Up
	LIS2DW_CTRL_7_SET = 0x20,       // Setting INT_ENABLE
	LIS2DW_SOFT_RESET = 0x40,
	LIS2DW_REBOOOT = 0x80,
	LIS2DW_STATUS_REG = 0x27,
	WRITE_ZERO = 0x00,
	WRITE_MAX = 0xFF,
	LIS2DW_CTRL2_BDU = 0x08,
	LIS2DW_WAKE_THRESH_VAL = 0x0D,
	LIS2DW_X_OFFSET = 0x3C,
	LIS2DW_Y_OFFSET = 0x3D,
	LIS2DW_Z_OFFSET = 0x3E,
	//End LIS2DW12

	//Begin LIS2DUX12
	LIS2DUX_CTRL_1 = 0x10,
	LIS2DUX_CTRL_1_SET = 0x03, //Enables wake up events on all 3 axis
	LIS2DUX_SOFT_RESET = 0x20, //Soft reset
	LIS2DUX_CTRL_5 = 0x14, //Has ODR, Bandwidth, and Full-Scale selection
	LIS2DUX_CTRL_5_SET = 0x30, //Set data rate 25hz ultra low power, default BW and FSR
	LIS2DUX_WAKE_THRESH_REG = 0x1C, //Wake up threshold register
	LIS2DUX_WAKE_THRESH_VAL = 0x0D,
	LIS2DUX_WAKE_DURATION = 0x1D,
	LIS2DUX_INT_CNFG_REG = 0x17, //Interrupt configure register
	LIS2DUX_INT_ENABLE = 0x03, //Enables and latches interrupts
	LIS2DUX_ROUTE_INT1_REG = 0x1F,
	LIS2DUX_ROUTE_INT1_VAL = 0x30, //Enables Wake-up and FF interrupts
	LIS2DUX_ALL_INT_SRC = 0x24,	//All int source register
	LIS2DUX_WAKE_SRC = 0x21,	//Wake up source register
	LIS2DUX_STATUS_REG = 0x25


} ACCEL_I2C_Command;

typedef enum
{// Error codes used for debugging
	ACC_OK = 0,         // Chip OK - Execution fine
	ACC_NOT_OK,         // Chip NOT OK - Execution NOT fine
	ACC_TYPE_PARAM_ERR, // Error building command line
	ACC_TX_CMD_ERR,     // Command TX issue
	ACC_RX_CMD_ERR      // Command RX issue
} ACCEL_STATE_TYPE;


/******************************************************
 Function Prototypes
 Note: Typical void functions are placed with ptr
 pointer due to logic flaw in the ucontroller
 ******************************************************/

void handleAccelWakeUp(void);
uint8_t accelReadRegister(ACCEL_I2C_Command readReg);
void printAccelData(bool acquireData);
ACCEL_DATA_RATE getAccelDataRate(void);
ACCEL_POWER_MODE getAccelMode(void);
ACCEL_RESOLUTION getAccelResolution(void);
ACCEL_FULL_SCALE getAccelFullScaleRange(void);
void accelWriteRegisterCommand(ACCEL_I2C_Command accReg, ACCEL_I2C_Command command);
void accelWriteRegisterRaw(ACCEL_I2C_Command accReg, uint8_t command);
void readAccelFIFO(void);
void accelWhoAmI(void);
/******************************************************
 Shared Global Variables
 Note:
 ******************************************************/

extern I2C_HandleTypeDef hi2c1;			// External I2C configuration
extern IWDG_HandleTypeDef hiwdg;		// External IWDG configuration

#endif  // ACCELEROMETER_H_
