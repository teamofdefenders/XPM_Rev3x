/*
 * XPS.h
 *
 *  Created on: Jan 10, 2023
 *      Author: Erik Pineda-A
 */

#ifndef __XPS_H
#define __XPS_H

/******************************************************
 File Includes
 Note:
 ******************************************************/

#include "Functions.h"

/******************************************************
 Define Values
 Note: Values Gathered from Datasheet
 ******************************************************/

#define MEM_BLOCK_SIZE 64				// 64 KB: 256 pages
// Mem small block size in KB
#define MEM_SBLOCK_SIZE 32				// 32 KB: 128 pages
// Mem sector size in KB
#define MEM_SECTOR_SIZE 4				// 4 KB : 16 pages
// Mem page size in bytes
#define MEM_PAGE_SIZE  256				// 256 byte : 1 page
// Blocks count
#define BLOCK_COUNT (256 * 2)	// 512 blocks
// Sector count
#define SECTOR_COUNT (BLOCK_COUNT * 16)		// 8192 sectors
// Pages count
#define PAGE_COUNT (SECTOR_COUNT * 16)		// 131,072 pages

#define Log_Page 25
#define Log_MAX_Shift 250
#define Log_MIN_Shift 1
#define Log_MAX_Size 16
#define PARAM_PAGE 0
#define PARAM_SHIFT 0
#define PIC_PAGE 2000
#define PIC_SHIFT 0
#define FLASH_PAGE 10
#define FLASH_SHIFT 0
#define UPDATE_PARAM_KEY 0x10

#define LOG_MEMORY "\1 Memory Command : \0"	// Start of UART MSG
#define LOG_BUSY_MEMORY "\1 Testing Memory Busy? \0"	// Start of UART MSG

/******************************************************
 Global Enum
 Note: Values are used for error codes,
 command types, and XPS state
 ******************************************************/

typedef enum
{								// Error codes used for XPS command debugging
	XPS_OK = 0,  							// Chip OK - Execution fine
	XPS_BUSY,								// Chip busy
	XPS_RST,								// Reset flag
	XPS_QUAD_ERR,							// System not in quad mode
	XPS_PARAM_ERR, 							// General parameters error
	XPS_TYPE_PARAM_ERR, 					// Memory parameters error
	XPS_PAGE_PARAM_ERR,						// Memory parameters error
	XPS_SHIFT_PARAM_ERR,					// Memory parameters error
	XPS_SIZE_PARAM_ERR, 					// Page parameters error
	XPS_SPI_CONFIG_ERR, 					// SPI configuration error
	XPS_SPI_RECEIVE_ERR, 					// SPI receive error
	XPS_SPI_SEND_ERR,						// SPI send error
	XPS_QSPI_CONFIG_ERR, 					// Quad-SPI configuration error
	XPS_QSPI_RECEIVE_ERR, 					// Quad-SPI receive error
	XPS_READ_CONFIG_ERR, 					// Read configure error
	XPS_READ_RECEIVE_ERR, 					// Read received error
	XPS_WRITE_CONFIG_ERR, 					// Write configuration error
	XPS_WRITE_SEND_ERR, 					// Write transmit error
	XPS_Erase_CONFIG_ERR,					// Erase parameters error
	XPS_STATE_CONFIG_ERR, 					// Register Status configuration error
	XPS_STATE_RECEIVE_ERR, 					// Register Status receive error
	XPS_SPI_WEL_ERR, 						// WEL parameters error
	XPS_SLEEP_CONFIG_ERR, 					// Sleep configuration error
	XPS_RSTTGL_CONFIG_ERR, 					// Sleep configuration error
	XPS_RESET_CONFIG_ERR, 					// Sleep configuration error
	XPS_Erase_PARAM_ERR,					// Erase parameters error
} XPS_STATE;

typedef enum
{
	XPS_WRITE_ENABLE = 0x06,			// sets WEL bit, must be set before any write/program/erase


	XPS_WRITE_DISABLE = 0x04,			// resets WEL bit (state after power-up)
	XPS_ENABLE_VOLATILE_SR = 0x50,	// check 7.1 in datasheet
	XPS_READ_SR1 = 0x05,				// read status-register 1
	XPS_READ_SR2 = 0x35,				// read status-register 2
	XPS_READ_SR3 = 0x15,				// read ststus-register 3
	XPS_WRITE_SR1 = 0x01,				// write status-register 1 (8.2.5)
	XPS_WRITE_SR2 = 0x31,				// write status-register 2 (8.2.5)
	XPS_WRITE_SR3 = 0x11,				// write status-register 3 (8.2.5)
	XPS_ReadPage_EXT_ADDR_REG = 0xC8,	// read extended addr reg (only in 3-byte mode)
	XPS_WritePage_EXT_ADDR_REG = 0xC8,	// write extended addr reg (only in 3-byte mode)
	XPS_ENABLE_4B_MODE = 0xB7,		// enable 4-byte mode (128+ MB address)
	XPS_DISABLE_4B_MODE = 0xE9,		// disable 4-byte mode (<=128MB)
	XPS_ReadPage_DATA = 0x03,			// read data by standard SPI
	XPS_ReadPage_DATA_4B = 0x13,		// read data by standard SPI in 4-byte mode
	XPS_FAST_READ = 0x0B,				// highest FR speed (8.2.12)
	XPS_FAST_READ_4B = 0x0C,			// fast read in 4-byte mode
	XPS_FAST_READ_DUAL_OUT = 0x3B,	// fast read in dual-SPI OUTPUT (8.2.14)
	XPS_FAST_READ_DUAL_OUT_4B = 0x3C,	// fast read in dual-SPI OUTPUT in 4-byte mode
	XPS_FAST_READ_QUAD_OUT = 0x6B,	// fast read in quad-SPI OUTPUT (8.2.16)
	XPS_FAST_READ_QUAD_OUT_4B = 0x6C,	// fast read in quad-SPI OUTPUT in 4-byte mode
	XPS_FAST_READ_DUAL_IO = 0xBB,		// fast read in dual-SPI I/O (address transmits by both lines)
	XPS_FAST_READ_DUAL_IO_4B = 0xBC,	// fast read in dual-SPI I/O in 4-byte mode
	XPS_FAST_READ_QUAD_IO = 0xEB,		// fast read in quad-SPI I/O (address transmits by quad lines)
	XPS_FAST_READ_QUAD_IO_4B = 0xEC,	// fast read in quad-SPI I/O in 4-byte mode
	XPS_SET_BURST_WRAP = 0x77,		// use with quad-I/O (8.2.22)
	XPS_PAGE_PROGRAM = 0x02,			// program page (256bytes) by single SPI line
	XPS_PAGE_PROGRAM_4B = 0x12,		// program page by single SPI in 4-byte mode
	XPS_PAGE_PROGRAM_QUAD_INP = 0x32,	// program page (256bytes) by quad SPI lines
	XPS_PAGE_PROGRAM_QUAD_INP_4B = 0x34,	// program page by quad SPI in 4-byte mode
	XPS_SECTOR_ERASE = 0x20,			// sets all 4Kbyte sector with 0xFF (erases it)
	XPS_SECTOR_ERASE_4B = 0x21,		// sets all 4Kbyte sector with 0xFF in 4-byte mode
	XPS_32KB_BLOCK_ERASE = 0x52,		// sets all 32Kbyte block with 0xFF
	XPS_64KB_BLOCK_ERASE = 0xD8,		// sets all 64Kbyte block with 0xFF
	XPS_64KB_BLOCK_ERASE_4B = 0xDC,	// sets all 64Kbyte sector with 0xFF in 4-byte mode
	XPS_CHIP_ERASE = 0xC7,			// fill all the chip with 0xFF
	XPS_FULL_ERASE = 0x60,			// another way to erase chip
	XPS_ErasePROG_SUSPEND = 0x75,		// suspend erase/program operation (can be applied only when SUS=0, BYSY=1)
	XPS_ErasePROG_RESUME = 0x7A,		// resume erase/program operation (if SUS=1, BUSY=0)
	XPS_POWERDOWN = 0xB9,				// powers down the chip (power-up by reading ID)
	XPS_POWERUP = 0xAB,	            	// release power-down
	XPS_DEVID = 0x94,					// read Device ID (same as powerup)
	XPS_FULLID = 0x90,				// read Manufacturer ID & Device ID
	XPS_FULLID_DUAL_IO = 0x92,		// read Manufacturer ID & Device ID by dual I/O
	XPS_FULLID_QUAD_IO = 0x94,		// read Manufacturer ID & Device ID by quad I/O
	XPS_ReadPage_UID = 0x4B,			// read unique chip 64-bit ID
	XPS_ReadPage_JEDEC_ID = 0x9F,		// read JEDEC-standard ID
	XPS_ReadPage_SFDP = 0x5A,			// read SFDP register parameters
	XPS_Erase_SECURITY_REG = 0x44,	// erase security registers
	XPS_PROG_SECURITY_REG = 0x42,		// program security registers
	XPS_ReadPage_SECURITY_REG = 0x48,	// read security registers
	XPS_IND_BLOCK_LOCK = 0x36,		// make block/sector read-only
	XPS_IND_BLOCK_UNLOCK = 0x39,		// disable block/sector protection
	XPS_ReadPage_BLOCK_LOCK = 0x3D,	// check block/sector protection
	XPS_GLOBAL_LOCK = 0x7E,			// global read-only protection enable
	XPS_GLOBAL_UNLOCK = 0x98,			// global read-only protection disable
	XPS_ENABLE_RST = 0x66,			// enable software-reset ability
	XPS_RESET = 0x99,					// make software reset
} XPS_SPI_Command;

typedef enum
{								// Type definitions for XPS command
	WRITE = 0,			//
	READ,				//
	ERASE_64KB,			//
	ERASE_32KB,			//
	ERASE,				//
	WAKE,				//
	SLEEP,				//
	SETTLE,				//
	PREP_RESET,			//
	TRIG_RESET,			//
	READ_STAT_REG,		//
	QCHIPID,			//
	CHIPID,				//
	WRITE_ENABLE,		//
	WRITE_STAT_REG,		//
	READ_STAT_STRCT,    //
	ERASE_FULL          //
} XPS_CMD_TYPE;

// XPS Parameter Page 1 Map
// Position of Parameter in the memory page
typedef enum
{
	PAGEKEY = 0,                  //
	LOGGING_LEVEL_SETTING = 1,    //
	DEVICE_MODE = 2,              //  Data_Ptr->Mode

	DT_MOTION_CONFIRM_WINDOW = 3,  //  dayTimeParam.Motion_Confirm_Window
	DT_MOTION_THRES =          4,  //  dayTimeParam.Motion_Threshhold
	DT_MOTION_BLACKOUT_MSB =   5,  //  dayTimeParam.Motion_Blackout
	DT_MOTION_BLACKOUT_LSB =   100,  //  dayTimeParam.Motion_Blackout
	DT_NO_MOTION_WINDOW =      6,  //  dayTimeParam.No_Motion_Detection_Window
    DT_NEAR_MOTION_THRES =     7,  //  dayTimeParam.Near_Motion_Threshhold

    NT_MOTION_CONFIRM_WINDOW = 8,  //  nightTimeParam.Motion_Confirm_Window
    NT_MOTION_THRES =          9,  //  nightTimeParam.Motion_Threshhold
    NT_MOTION_BLACKOUT_MSB =  10,  //  nightTimeParam.Motion_Blackout
    NT_MOTION_BLACKOUT_LSB =  101,  //  nightTimeParam.Motion_Blackout
    NT_NO_MOTION_WINDOW =     11,  //  nightTimeParam.No_Motion_Detection_Window
    NT_NEAR_MOTION_THRES =    12,  //  nightTimeParam.Near_Motion_Threshhold

    PIR_COOLDOWN_MSB = 13,  //  pirData.coolDown MSB
    PIR_COOLDOWN_LSB = 14,  //  pirData.coolDown LSB
	PIR_MODE =         60,  //  pirData.mode

	HB_MODE =          59,  //  Data_Ptr->heartBeatData.mode
    HB_INTERVAL_MSB =  15,  //  Data_Ptr->heartBeatData.hbInterval MSB
    HB_INTERVAL_LSB =  16,  //  Data_Ptr->heartBeatData.hbInterval LSB

    BZ_MODE =         22,  //  buzzerData.mode
    BZ_START_CYCLES = 20,  //  buzzerData.startCycles
    BZ_STOP_CYCLES =  21,  //  buzzerData.stopCycles

    GPS_MODE =            23,  //  gpsParameters.mode
    GPS_INTERVAL =        24,  //  gpsParameters.gpsInterval
    GPS_TIMESYNC_ENABLE = 25,  //  gpsParameters.timeSyncEnable
    GPS_GEO_MSB =         27,  //  gpsParameters.geofenceDistance MSB
    GPS_GEO_LSB =         28,  //  gpsParameters.geofenceDistance LSB
    GPS_ALARM_SAMP_PER =  29,  //  gpsParameters.alarmSamplePeriod
    GPS_ALARM_TIME_MSB =      30,  //  gpsParameters.alarmTime
	GPS_ALARM_TIME_LSB = 104,
    GPS_MOVE_HYSTER =     31,  //  gpsParameters.movementHysteresis

	LONG_INT_MSB = 32,  //  longitudeInt >> 24; MSB
	LONG_INT_3B =  33,  //  longitudeInt >> 16;
	LONG_INT_2B =  34,  //  longitudeInt >> 8;
	LONG_INT_LSB = 35,  //  longitudeInt; LSB
	LAT_INT_MSB =  36,  //  latitudeInt >> 24; MSB
	LAT_INT_3B =   37,  //  latitudeInt >> 16;
	LAT_INT_2B =   38,  //  latitudeInt >> 8;
	LAT_INT_LSB =  39,  //  latitudeInt; LSB

    BUS_DT_HOURS =     40,  //  operatingHours.Daytime.hours
    BUS_DT_MIN =       41,  //  operatingHours.Daytime.minutes
    BUS_NT_HOURS =     42,  //  operatingHours.Nighttime.hours
    BUS_NT_MIN =       43,  //  operatingHours.Nighttime.minutes
    BUS_HOURS_ENABLE = 44,  //  operatingHours.DayNightSwitchEnabled

	ENV_SENSOR_SAMPLE_PERIOD =     49, //  tempParams.samplePeriod
    ENV_SENSOR_WARMUP =            17, //  tempParams.sensorWarmUp
	ENV_SENSOR_COOLDOWN =          48, //  tempParams.coolDownPeriod
	ENV_SENSOR_CHARG_DISABLE_SET = 50, //  tempParams.chargerDisableSetting
	ENV_SENSOR_ALARM_TIME =        61, //  tempParams.alarmTime
	ENV_SENSOR_ALARM_SAMPLE =      62, //  tempParams.alarmSamplePeriod

	TEMP_MODE =    63,  // tempParams.temperature.mode
    TEMP_LOW_SET = 45,  // tempParams.temperature.lowSet
    TEMP_HI_SET =  46,  // tempParams.temperature.highSet
    TEMP_HYSTER =  47,  // tempParams.temperature.hysteresis

	HUMD_MODE =    64,  // tempParams.humidity.mode
	HUMD_LOW_SET = 65,  // tempParams.humidity.lowSet
	HUMD_HI_SET =  66,  // tempParams.humidity.highSet
	HUMD_HYSTER =  67,  // tempParams.humidity.hysteresis

	ACC_MODE =    68,     // accParams.mode
    ACC_RANGE =   51,     // accParams.range
    ACC_THRESH_MSB =  52, // accParams.threshold
    ACC_HYSTER_MSB =  53, // accParams.hysteresis
	ACC_THRESH_LSB = 102, // CSC added according to KCS slack message
	ACC_HYSTER_LSB = 103,  // CSC added according to KCS slack message
	ACC_MUTE_LSB = 104,   // accel muting Period
	ACC_MUTE_MSB = 105,	  // accel muting Period

	DOWNLOAD_READY =    56,  //  Data_Ptr->Flash.fwDownloadRdy
	OTA_NUMB_OF_FILES = 57,  //  OtaData->numberOfFiles
	OTA_FILENAME_LEN =  58,  //  OtaFileNameLength

	CELL_MODE =    68,   // cell.mode
    CELL_WARMUP =  18,   // cell.warmUp

	CAMERA_MODE =                69,   // camera.mode
    CAMERA_WARMUP =              19,  //  camera.warmUp
    CAMERA_PIC_PER_EVENT =       70,  //  camera.params.picturesPerEvent
    CAMERA_PIC_INTERVAL =        71,  //  camera.params.pictureInterval
    CAMERA_PIC_SAMPLE_HOUR =     72,  //  camera.params.samplePictureTime.hours
    CAMERA_PIC_SAMPLE_MIN =      73,  //  camera.params.samplePictureTime.mins
    CAMERA_PIC_SAMPLES_PER_DAY = 74,  //  camera.params.samplePicNumber

	OTA_FILENAME_START = 200,  //  OtaFileNameStart

} XPS_PARAM_MAP1;


typedef struct
{	// Current XPS State
	bool BUSY,  				// Erase/Write in progress
	        WEL,				// Write enable latch (1 - write allowed)
	        QE,					// Quad SPI mode
	        SUS, 				// Suspend Status
	        ADS, 				// Current addr mode (0-3 byte / 1-4 byte)
	        ADP, 				// Power-up addr mode
	        SLEEP; 				// Sleep Status
	uint8_t TYPE,				// type command for control
	        COMMAND;			// Configures command for control
	XPS_STATE STATE;			// Current state of XPS IC
} XPS_STATUS_REG;

/******************************************************
 Function Prototypes
 Note: Typical void functions are placed with ptr
 pointer due to logic flaw in the ucontroller
 ******************************************************/

void XPS_HARDWARE_RESET ( MEM_PTR *Mem );
void XPS_Picture_Save ( MEM_PTR *Mem );
void XPS_Image_Load ( MEM_PTR *Mem );
void XPS_COMMAND ( MEM_PTR *Mem );
void XPS_IsBusy ( MEM_PTR *Mem );
void XPS_Verify_Memory ( MEM_PTR *Mem );
HAL_StatusTypeDef OSPItest(OSPI_HandleTypeDef *hospi, OSPI_RegularCmdTypeDef *cmd, uint32_t Timeout);

void XPS_Set_Command ( XPS_CMD_TYPE Input );
XPS_STATE XPS_Get_State ( void );
bool XPSisDataValid ( MEM_PTR *Data_Ptr );

void getOtaData(OTA_FILE_TYPE *OtaData);
void setOtaData(OTA_FILE_TYPE *OtaData);
void readParamDataFromFlash(MEM_PTR *Data_Ptr );

/******************************************************
 Shared Global Variables
 Note:
 ******************************************************/

extern OSPI_HandleTypeDef hospi1;			// External OSPI configuration
extern IWDG_HandleTypeDef hiwdg;		// External IWDG configuration

#endif
