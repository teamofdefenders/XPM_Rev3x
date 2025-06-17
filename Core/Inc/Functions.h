/**
 ******************************************************************************
 * @file    Functions.h
 * @author  Erik Pineda-A, Kevin Stacey
 * @brief
 ******************************************************************************
 * @attention
 *
 * Copyright 2025 Team of Defenders
 *
 ******************************************************************************
 */
#ifndef FUNCTIONS_H_
#define FUNCTIONS_H_

//   ******************    INCLUDES     *********************
#include "time.h"
#include "stdbool.h"
#include "stdlib.h"
#include "stdio.h"
#include "stdarg.h"
#include "ctype.h"
#include "Main.h"

//   ******************    DEFINES     *********************

#define DECIMAL 10
#define UNIQUE_Device_ID (*(volatile uint32_t*)0x0BFA0700)
#define Is_Pin_High(port,pin) (HAL_GPIO_ReadPin ( port , pin ) == GPIO_PIN_SET)
#define Is_Pin_Low(port,pin) (HAL_GPIO_ReadPin ( port , pin ) == GPIO_PIN_RESET)
#define CONFIG_ERR_MSG_SIZE 500 //Change to more appropriate size after testing
#define CONTAINER_INIT_SIZE 10
#define CONTAINER_RESIZE_CHUNK 2


#define OTA_FLASH_SIZE 160000  // OTA flash arbitrary maximum size
#define MEMORY_MAX 4000	// Change to RECV_MAX_SIZE
#define PIC_MAX 60000
#define PIC_DATA_LIM 3500   // KCS change to XMIT_MAX_SIZE
#define MAXPAGES 80
#define DEVICE_ID_SIZE 15
#define Log_MSG_SIZE 250
#define CMD_FLASH_SIZE 100              //not external flash limit used in cell command
#define I2C_CMD_SIZE 20
#define SPI_CMD_SIZE 20
#define INSTRUCT_SIZE 150
#define TIMSTRNGT_SIZE 100
#define Log_Size_Limit 250    // Max Log list size
#define Log_MSG_SIZE 250      // Max size of log message
#define DECIMAL 10
#define Perecent_Threshold 0
#define PIR_COUNTER_LIM 10
#define Itteration_Limit 100  // GPS_iteration
#define Itteration_Start_Trigger 2
#define MODEM_WARM_DEFAULT  2   // modem warmup time in seconds
#define SENSOR_WARM_DEFAULT 1      // Sensor warmup time in seconds
#define A_TAG "\"Acc\", \""		//acceleration tag for sensor array
#define T_H_TAG "\"Temp H\", \""	//temp hum tag for sensor array
#define P_TAG "\"PirMotion\", \""	//pir tag for sensor array
#define G_TAG "\"GPS\", \""		//gps tag for sensor array
#define ACC_SPECIFIER "\", \"Data\": %d,%d,%d"	//defined type specifier for accelerometer
#define TEMPH_SPECIFIER "\", \"Data\": %d C,%d %" //defined type specifier for temp/hum
#define PIR_SPECIFIER "\", \"Data\": %s"			//defined type specifier for pir
#define GPS_SPECIFIER "\", \"Data\": %d,%d"		//defined type specifier for GPS
#define SENSOR_SIZE 100 	//max data streng length
#define SENSOR_ARRAY_COUNT 500	//max amount of data strings that can be stored


//#define A_TAG "\"Acc\", \""		//acceleration tag for sensor array
//#define T_H_TAG "\"Temp H\", \""	//temp hum tag for sensor array
//#define P_TAG "\"PirMotion\", \""	//pir tag for sensor array
//#define G_TAG "\"GPS\", \""		//gps tag for sensor array


#define UPLINK_DIAGNOSTIC_MSG_SIZE 250


#define DOWNLINK_TEST_MSG_SIZE 2000
#define PIR_DOWNLINK_TEST_MSG_SIZE 600

#define ISO_TIMESTAMP_LENGTH 25
#define LOG_TIMESTAMP_LENGTH 25

//  ****************   Buzzer Defines

// Bit definition for BUZZER Mode register
#define BUZZ_MODE_STARTMOTION_Pos 	(0U)
#define BUZZ_MODE_STOPMOTION_Pos	(1U)
#define BUZZ_MODE_NIGHT_Pos    		(2U)
//#define BUZZ_MODE_			(3U)
//#define BUZZ_MODE_		(4U)
#define BUZZ_MODE_TEST_Pos   		(8U)

#define BUZ_LENGTH_MOTION_msec     3001
#define BUZZER_TIME 200

#define BUZ_LENGTH_JOIN_msec       130
#define BUZ_LENGTH_ORPHAN_msec     90

#define Heartbeat_Default_Hours 4
#define Heartbeat_Default_Minutes 30
//  ********   Mask Definitions
// Hardware Status
// Up to 8 bits of space for control intializer
#define XPS_INIT 		0b00000001
#define CELL_INIT 		0b00000010
#define TEMP_INIT 		0b00000100
#define ACCEL_INIT 		0b00001000
#define CAM_INIT 		0b00010000
#define INTERRUPT_INIT 	0b00100000
#define PIR_INIT 		0b01000000
#define SYS_INIT 		0b11111111

// Logger Configuration
// Up to 8 bits of space for control settings
#define AVOID_MSG_SPAM	0b00000001
#define LOG_LEVEL_1		0b00000010
#define LOG_LEVEL_2		0b00000100
#define LOG_LEVEL_3		0b00001000

// Device control STATE configuration
#define UPDATE_SERVER 	   0b00000000000000000000000000000001
#define SERVER_COMMAND	   0b00000000000000000000000000000010
#define DEVICE_LISTEN	   0b00000000000000000000000000000100
#define FLASH_START		   0b00000000000000000000000000001000
#define FLASH_COMPLETE     0b00000000000000000000000000010000
#define FAIL_FLASH		   0b00000000000000000000000000100000
#define SLEEP_STATE 	   0b00000000000000000000000001000000
#define WAKE_STATE 		   0b00000000000000000000000010000000
#define PARAM_UPDT		   0b00000000000000000000000100000000
#define ACCEL_UPDT		   0b00000000000000000000001000000000
#define TEMP_UPDT		   0b00000000000000000000010000000000
#define HUMD_UPDT		   0b00000000000000000000100000000000
#define PWR_UPDT		   0b00000000000000000001000000000000
#define GPS_UPDT		   0b00000000000000000010000000000000
#define PIC_UPDT		   0b00000000000000000100000000000000
#define GPIO_UPDT 		   0b00000000000000001000000000000000
#define CELL_UPDT 		   0b00000000000000010000000000000000
#define PIR_UPDT 		   0b00000000000000100000000000000000
#define PIR_START 		   0b00000000000001000000000000000000
#define PIC_SEND 		   0b00000000000010000000000000000000
#define PIR_SEND 		   0b00000000000100000000000000000000
#define BUZZER_TOGGLE 	   0b00000000001000000000000000000000
#define CHARGER_DETECT 	   0b00000000010000000000000000000000
#define PIC_SAVE 		   0b00000000100000000000000000000000
#define GET_CELL_RECV_BUFF 0b00000001000000000000000000000000
#define GET_FW_UPDATE	   0b00000010000000000000000000000000
#define GET_DOWNLINKS      0b00000100000000000000000000000000
#define HB_UPDT            0b00001000000000000000000000000000
#define MOVEMENT_UPDT      0b00010000000000000000000000000000
#define PIR_END            0b00100000000000000000000000000000


// **********  End of  Mask Definitions

// *****************  Default Conditions
#define DEFAULT_STATE 	WAKE_STATE

#define DEFAULT_HB 43200  // 12 hours
#define DEFAULT_HB_MODE 3 //Enabled all the time
#define DEFAULT_Motion_Confirm_Window 0
#define DEFAULT_Motion_Threshhold 0
#define DEFAULT_Motion_Blackout 0
#define DEFAULT_No_Motion_Detection 0
#define DEFAULT_Near_Motion 0
#define DEFAULT_Day_Night_Filter 0

#define DEFAULT_Start_Motion_Cycles 0
#define DEFAULT_Stop_Motion_Cycles 0
#define DEFAULT_Buzzer_Mode 0

#define DEFAULT_Temperature_Timer 0
#define DEFAULT_Failed_Temperature_ID_Check 0
#define DEFAULT_Default_Temperature_Timer 0
#define DEFAULT_Device_ID 0
#define DEFAULT_Temperature 0
#define DEFAULT_Humidity 0

#define DEFAULT_Device_ID 0
#define DEFAULT_Accel_X 0
#define DEFAULT_Accel_Y 0
#define DEFAULT_Accel_Z 0
#define DEFAULT_Threshold_Level 0
#define DEFAULT_Time_Window 0

#define DEFAULT_Hysteresis 0
#define DEFAULT_Threshold 0
#define DEFAULT_Timing 0
#define DEFAULT_Range 0

// *****************  End of Default Conditions

#define LED_GPIO_PORT
#define LED_PIN
#define DEBUG_LED

//#define BPS240_Pin GPIO_PIN_1		// Controll for BPS240
//#define BPS240_GPIO_Port GPIOC	// Control for BPS240

#define TEST_MODE 0xFF		// Test value address
#define Value_PTR_A 0x00	// Default test value for start 1
#define Value_PTR_B 0xFF	// Default test value for start 2


#define LOG_START "\r\n\r\n\r\n\r\nDevice Start\0\r\n"
#define LOG_INITIALIZE "\1 Device Intialize Toggled\0"
#define LOG_ERROR "\1 Issue\0"
#define LOG_SUCCESS "\1 Success\0"
#define LOG_SPACER "\r\n\r\n\0"

#define LOG_DEBUG "\1 ***** \r\n\0"
#define LOG_DEVICE_CYCLE "\1 Device Cycle\0"
#define LOG_IDLE_START "\1 ***** Device Idle Start Time ***** \r\n\0"
#define LOG_IDLE_END "\1 ***** Device Idle End Time ***** \r\n\0"
#define LOG_SLEEP_START "\1 ***** Device Eepy Start Time ***** \r\n\0"
#define LOG_SLEEP_END "\1 ***** Device Eepy End Time ***** \r\n\0"
#define LOG_WAKE_CYCLE "\1 Device Grind Time \r\n\0"
#define POWER_DETECT "\1 ##### USB Power Detected ##### \r\n\0"
#define POWER_FAIL_DETECT "\1 ##### NO USB Power Detected ##### \r\n\0"
#define XPS_INIT_MSG "\1 Initialize Time __________ XPS __________\r\n\0"
#define TEM_INIT_MSG "\1 Initialize Time __________ TEM __________\r\n\0"
#define ACC_INIT_MSG "\1 Initialize Time __________ ACC __________\r\n\0"
#define CELL_INIT_MSG "\1 Initialize Time __________ CELL __________\r\n\0"
#define PIR_INIT_MSG "\1 Initialize Time __________ PIR __________\r\n\0"
#define CAM_INIT_MSG "\1 Initialize Time __________ CAM __________\r\n\0"
#define CAMERA_EN_MSG "\1 *** Camera Enabled  *** \r\n\0"
#define MODEM_EN_MSG "\1 *** Start Modem Enabled  *** \r\n\0"
#define MODEM_FN_MSG "\1 *** Finish Modem Enabled  *** \r\n\0"
#define MODEM_EN_FAIL_MSG "\1 *** Modem Failed   *** \r\n\0"

#define LOG_DEVICE_ID " Device ID : \0"
#define LOG_STATE_RESULT " Result : \0"	// Start of UART MSG
#define LOG_TYPE_ID " Type ID : \0"
#define LOG_TAIL "\r\n\0"
// *************************   End of UART msg

#define MAX77801_SLAVE_ADDRESS 0x30
#define MAX77801_ID 0x00
#define MAX77801_STATUS 0x01
#define MAX77801_CNFG_1 0x02
#define MAX77801_CNFG_2 0x03
#define MAX77801_VOUT_DVS_L 0x04
#define MAX77801_VOUT_DVS_H 0x05
#define MAX77801_VOUT_4V 0x7F

#define POLARSSL_ERR_BASE64_BUFFER_TOO_SMALL  -0x002A  /**< Output buffer too small. */
#define POLARSSL_ERR_BASE64_INVALID_CHARACTER -0x002C  /**< Invalid character in input. */

#define BUZZER_TIME 200

#define PIR_SLEEP_WAKE_PIN PWR_WAKEUP_PIN1_HIGH_0
#define INT1_SLEEP_WAKE_PIN PWR_WAKEUP_PIN6_HIGH_1
#define INT2_SLEEP_WAKE_PIN PWR_WAKEUP_PIN2_HIGH_1


// **************** Redirections for "ease" of programming
#define buf Data_Ptr->Buffer
#define Dev_Mode memory.Mode
#define Dev_Prev_Mode memory.Prev_Mode
#define _Memory Data_Ptr->Memory
#define _Pointer Data_Ptr->PTR
#define _State Data_Ptr->State
#define _Init_Flags Data_Ptr->Init_Flags
#define _Setting Data_Ptr->Setting
#define _Size Data_Ptr->Size
#define _RxSize Data_Ptr->RxSize


#define _Temperature Data_Ptr->Temperature_Data.Temperature
#define _Humidity Data_Ptr->Temperature_Data.Humidity
#define _Temp_Device_ID Data_Ptr->Temperature_Data.Device_ID
//#define _Acce_Device_ID Data_Ptr->Accelerometer_Data.Device_ID
#define _Page Data_Ptr->External_Data.Page
#define _Shift Data_Ptr->External_Data.Shift
#define _Encoded_Picture Data_Ptr->Camera_Data.Encoded
#define _Encoded_Size Data_Ptr->Camera_Data.Encoded_Size
#define _Buzzer_Control memory.buzzerData.Control
#define _PIR_Control Data_Ptr->PIR_Data.Control
#define _Cell_Date_Day Data_Ptr->Cdate_Data.CellTime_Day
#define _Cell_Date_Month Data_Ptr->Cdate_Data.CellTime_Month
#define _Cell_Date_Year Data_Ptr->Cdate_Data.CellTime_Year
#define _Temp_Time1 Data_Ptr->Temptime_Data.temp1
#define _Temp_Time2 Data_Ptr->Temptime_Data.temp2

// ***************  LAMBDA funcitons
#define Refresh_Watchdog if (HAL_IWDG_Refresh ( &hiwdg ) != HAL_OK) Error_Handler ();

#define PRINTF(...)     do{  TraceSend(1, __VA_ARGS__); }while(0)
#define BACKUP_PRIMASK()  uint32_t primask_bit= __get_PRIMASK()
#define DISABLE_IRQ() __disable_irq()
#define ENABLE_IRQ() __enable_irq()
#define RESTORE_PRIMASK() __set_PRIMASK(primask_bit)
#define PRINTBUFSIZE 4000		//buffer size for PRINTF()

// ****************   END OF DEFINES  ****************


// ****************   external parameters  ****************

extern IWDG_HandleTypeDef hiwdg;
extern ADC_HandleTypeDef hadc1;
extern ADC_HandleTypeDef hadc4;
extern I2C_HandleTypeDef hi2c1;
extern SPI_HandleTypeDef hspi2;
extern CRC_HandleTypeDef hcrc;

//extern IWDG_HandleTypeDef hiwdg;
extern UART_HandleTypeDef hlpuart1;
extern UART_HandleTypeDef huart2;
extern OSPI_HandleTypeDef hospi1;
//extern RNG_HandleTypeDef hrng;
extern SPI_HandleTypeDef hspi1;
extern HCD_HandleTypeDef hhcd_USB_OTG_FS;
extern RTC_HandleTypeDef hrtc;


//boolean to reduce latency time from PIR detect to PIC send
extern bool latencyMin;

// ****************   End of external parameters  ****************

//  ****************    ENUMerations   ****************************

typedef enum
{							// Type definitions for ACC command
	DEV_Mode_A = 0,						// Default Mode
	DEV_Mode_B,							// Soft Restart Mode
	DEV_Mode_C,							// Demo Mode
	DEV_Mode_D,							//
	DEV_Mode_E,							//
} Device_Mode;

typedef enum
{				// Type definitions Boot Bank
	BANK1 = 1,	// Default Mode
	BANK2 = 2
} BANK_TYPE;


typedef enum
{		// Type definitions for Downlink Messages
	DL_FW_AVAIL,	  // New Firmware is available
	DL_FW_PACKET,     // OTA Firmware packet
	DL_FW_COMPLETE,   // Firmware completed
	DL_CONFIG_PKT,    // Configuration
	DL_PICTURE_ACK,   // Missing image sectors
	DL_TIME_SYNC,     // Time sync response message
	DL_ALARM_COMMAND, //
	DL_CONTROL,   //
	DL_DONE,          // No message to process
	DL_UNKNOWN_MSG    // Initial setting
} DOWNLINK_MSG_TYPE;

typedef enum
{
	ACC_TAG,
	TEMP_HUM_TAG,
	PIR_TAG,
	GPS_TAG
}DATA_TYPE_TAG;

typedef enum
{
	IMAGE_TAKEN,
	IMAGE_COMPLETE,
	TX_MISSING_PAGES,
	WAKEUP_HB,
	FW_LOADING,
	IDLE
} MACHINE_STATE_TYPE;

typedef enum
{
	BOOT,
	CONFIG_ACK
}STATUS_UPLINK_TYPE;

typedef enum
{
	HEARTBEAT_MODULE,
	GPS_MODULE,
	BUZZER_MODULE,
	TEMPERATURE_MODULE,
	HUMIDITY_MODULE,
	CELL_MODULE,
	CAMERA_MODULE,
	ACCELEROMETER_MODULE,
	PIR_MODULE,
}HW_MODULE_TYPE;

typedef enum
{
	BAT_FULL,
	BAT_CHARGING,
	CHARGER_UNPLUGGED
}STATUS_BATTERY_TYPE;
typedef enum
{
	LIS_MODE_LOW_POWER,
	LIS_MODE_HIGH_PERFORMANCE,
	LIS_MODE_SINGLE_DATA, //Not implemented for now
	LIS_MODE_ERROR
}ACCEL_POWER_MODE;

typedef enum
{
	LIS_RANGE_ERROR,
	LIS_RANGE_2,
	LIS_RANGE_4,
	LIS_RANGE_8,
	LIS_RANGE_16
}ACCEL_FULL_SCALE;

typedef enum
{
	LIS_DATA_RATE_POWER_DOWN,
	LIS_DATA_RATE_1, //1.6Hz
	LIS_DATA_RATE_12, //12.5Hz
	LIS_DATA_RATE_25, //The rest are actual values
	LIS_DATA_RATE_50,
	LIS_DATA_RATE_100,
	LIS_DATA_RATE_200,
	LIS_DATA_RATE_400,
	LIS_DATA_RATE_800,
	LIS_DATA_RATE_1600,
	LIS_DATA_RATE_ERROR
}ACCEL_DATA_RATE;

typedef enum
{
	LIS_RESOLUTION_12,
	LIS_RESOLUTION_14,
	LIS_RESOLUTION_ERROR
}ACCEL_RESOLUTION;


typedef struct
{
	char (*strings)[CONFIG_ERR_MSG_SIZE]; //Pointer to strings
	uint8_t count;	//Size of the container
	uint8_t capacity; //Number of strings we can hold
}STRING_CONTAINER;

typedef enum
{
	hb_wu_enabled = 1,
	acc_wu_enabled,
	img_wu_enabled,
	pGPS_wu_enabled,
	blackout_wu_enabled,
	hb_img_pair,
	hb_acc_pair,
	hb_pGPS_pair,
	hb_blackout_pair,
	img_acc_pair,
	img_pGPS_pair,
	img_blackout_pair,
	acc_blackout_pair,
	pGPS_blackout_pair,
	hb_img_acc_pair,
	hb_img_pGPS_pair,
	hb_img_blackout_wu_enabled,
	hb_acc_blackout_wu_enabled,
	hb_pGPS_blackout_wu_enabled,
	img_acc_blackout_wu_enabled,
	img_pGPS_blackout_wu_enabled,
	hb_img_acc_blackout_wu_enabled,
	hb_img_pGPS_blackout_wu_enabled

} WAKEUP_STATE;


//  ****************    End of ENUMerations   ****************************


#define START_MODE DEV_Mode_C // kcs this needs to be deleted and associated in the correct place

//  ****************    Structure Definitions   ****************************

typedef struct
{
	uint8_t hours;  // Hours in 24 hour clock
	uint8_t minutes;  // Minutes
}CLOCK_TIME_TYPE;

typedef struct
{
	// KCS fix these names
	BANK_TYPE currentFlashBank;  // Flash Bank
	uint8_t fwDownloadRdy;  // parameter for OTA workaround
	uint32_t Page;			// Page buffer
	uint32_t Index;			// Index buffer
	uint16_t Total_Size;	// Flash Total Size
	uint8_t Image [ OTA_FLASH_SIZE ];	// Memory buffer
} FLASH_DATA_TYPE;

typedef struct
{
	// KCS fix these names
	uint8_t Failed_Temperature_ID_Check;	// Device ID
	uint8_t Default_Temperature_Timer;		// Device ID
	uint16_t Device_ID;						// Device ID
	uint16_t Temperature;					// Stores Temperature
	uint16_t Humidity;						// Stores Humidity
} TEMPERATURE_DATA_TYPE;

//typedef struct
//{
//	uint8_t Threshold_Level;		// Device ID
//	uint8_t Time_Window;			// Device ID
//	uint8_t Device_ID;				// Device ID
//	uint16_t Accel_X;				// Stores X-Axis speed
//	uint16_t Accel_Y;				// Stores Y-Axis speed
//	uint16_t Accel_Z;				// Stores Z-Axis speed
//} ACCEL_DATA_TYPE;

typedef struct
{
	uint8_t mode;          //  See above
	uint8_t range;         // change to enumeration type
	uint32_t threshold;    // in mg?, axis g value to trigger interrupt
	uint32_t hysteresis;   //
	uint16_t duration;     // in seconds, integration time in sensor for trigger
	uint16_t mutePeriod;   // in seconds
} ACCELERATION_PARAM_TYPE;

//Camera Mode Bit Definition
//0 - Night Enabled/Disable
//1 - Day Enable/Disable
//2 - Take pic on near motion Enable/Disable
//3 - Sample picture every day Enable/Disable
//4 - Reserved
//5 - Reserved
//6 - Reserved
//7 - Reserved
typedef struct
{
	uint8_t  mode;             // see above
	uint8_t  warmUp;           // Time for Camera to power up and stablize period in seconds
	uint8_t picturesPerEvent;  // number of pictures to take per motion event
	uint8_t pictureInterval;   // time in seconds between pictures
	CLOCK_TIME_TYPE samplePictureTime;  // time to take sample picture
	uint8_t samplePicNumber;  // number of sample pictures to take per day (24 hours)
} CAMERA_PARAMETER_TYPE;

typedef struct
{
	// KCS fix these names
	unsigned char encoded [ PIC_MAX ];
	uint32_t encodedSize;
} CAMERA_DATA_TYPE;


//Cell Modem Mode Bit Definition
//0 - Night Enabled/Disable
//1 - Day Enable/Disable
//2 - Reserved
//3 - Reserved
//4 - Reserved
//5 - Reserved
//6 - Reserved
//7 - Reserved
typedef struct
{
	uint8_t  mode;    // see above
	uint8_t  warmUp;  // Time for Cell Modem to power up and stablize period in seconds
} MODEM_PARAMETER_TYPE;

typedef struct
{
	// KCS fix these names
	uint8_t Time_STMP [ TIMSTRNGT_SIZE ]; // Time stamp string
	uint32_t Package_Page;  // Not used, originally for non-encoded Pictures
} MODEM_DATA_TYPE;

typedef struct
{
	// KCS fix these names
	uint8_t Shift;		// Page Shift (limit = 256 - Data Size)
	uint32_t Page;		// Page (limit = 131'072)
} EXTERNAL_DATA_TYPE;

//typedef struct
//{
//	uint8_t percent; 			// Percentage counter for windows
//	uint8_t mode; 	            // KCS put real description here
//	uint8_t control; 			// current PIR window which count applies
//	uint8_t counter; 			// window counter of PIR motions
//	uint8_t eventCounter; 		// Number of motion events when in last cooldown period
//	uint16_t coolDown;			// Time in seconds between sending Motion start events
//	PIR_FILTER_TYPE dayTime;	// Day time filter parameters
//	PIR_FILTER_TYPE nightTime;	// Night time filter parameters
//} PIR_PARAMETER_TYPE;

//Buzzer mode Bit Definition
//Bit Definition
//0 - Start Enabled
//1 - Stop Enabled
//2 - Day Enabled
//3 - Night Enabled
//4 - Reserved
//5 - Reserved
//6 - Reserved
//7 - Reserved
typedef struct
{
	// KCS fix these names
	uint8_t Single_Repeat;
	uint8_t Cycles_Repeat;
	uint8_t State;
	uint8_t Control;
	uint8_t mode;           // see above
	uint8_t startCycles;
	uint8_t stopCycles;
	// KCS do these all need to be uint32 ????
	uint32_t Start_Delay;
	uint32_t Single_Delay;
	uint32_t Cycles_Delay;
	uint32_t Cycles_Length;
	uint32_t Single_Length;
} BUZZER_DATA_TYPE;

//Heart Beat mode Bit Definition
//0 - Night Enabled/Disable
//1 - Day Enable/Disable
//2 - Reserved
//3 - Reserved
//4 - Reserved
//5 - Reserved
//6 - Reserved
//7 - Reserved
typedef struct
{
	uint8_t  mode;          // Heart beat mode, see above
	uint16_t hbInterval;    // Heart beat period in seconds
} HEARTBEAT_DATA_TYPE;


//GPS Mode Bit Definition
//0 - Night Enabled/Disable
//1 - Day Enable/Disable
//2 - Time/Location Sync Enable/Disable
//3 - Reserved
//4 - Reserved
//5 - Reserved
//6 - Reserved
//7 - Reserved
//0 - always disable, 3 - always enable, 1 - night only, 2 - day only
typedef struct
{
	uint8_t mode;                //  See above
	uint16_t alarmSamplePeriod;  // GPS data gathering sample period in seconds
	uint16_t alarmTime;          // GPS data gathering total time in seconds
	uint16_t gpsInterval;        // in minutes, period when to acquire information from gps
	uint16_t geofenceDistance;   // in feet?
	uint16_t movementHysteresis; // in feet?
} GPS_PARAMETER_TYPE;

typedef struct
{
	// KCS fix these names
	uint8_t CellTime_Day[2],
	CellTime_Month[2],
	CellTime_Year[2];
} CDATE_DATA_TYPE;

typedef struct
{
	char temp1[256];
	char temp2[256];
}TEMP_TIME_DATA_TYPE;

typedef struct
{
	float latitude;   //Format: (-)dd.ddddd
	float longitude;  //Format: (-)dd.ddddd
}GPS_LOCATION_TYPE;

typedef struct
{
	char UTC[11];		//String type. Current UTC time. Format: hhmmss.sss
	float HDOP;			//Horizontal precision. Range: 0.5–99.9
	float altitude;		//The altitude of the antenna away from the sea level, accurate to one decimal place. Unit: meter.
	int fix;			//GNSS positioning mode 2 2D positioning, 3 3D positioning
	char COG[7];		//Course Over Ground based on true north.  Format: ddd.mm, ddd  000–359 (Unit: degree) mm  00–59 (Unit: minute)
	float spkm;			//Speed over ground.  Format: xxxx.x. Unit: Km/h. Accurate to one decimal place
	float spkn;			//Speed over ground.  Format: xxxx.x. Unit: knots. Accurate to one decimal place
	char date[7];		//UTC time when fixing position.  Format: ddmmyy
	unsigned int nsat;	// Number of satellites. Range: 00–12
	GPS_LOCATION_TYPE CurrentPosition;
	GPS_LOCATION_TYPE LastPosition;
} GPS_DATA_TYPE;

typedef struct
{
	// KCS remove this switch when backoffice supports individual settings
	bool DayNightSwitchEnabled; // day night switching enable
	CLOCK_TIME_TYPE dayTime;  // Day time opening Clock time
	CLOCK_TIME_TYPE nightTime;  // Night time closing Clock time
	CLOCK_TIME_TYPE lastEventTime;
} BUSINESS_DATA_TYPE;

typedef struct
{
} ALARM_TYPE;

//typedef struct
//{
//	uint8_t mode;          //  See above
//	uint8_t range;         // change to enumeration type
//	uint16_t threshold;    // need to probably change to 16 bit scaled value
//	uint16_t hysteresis;   // need to probably change to 16 bit scaled value
//} ACCELERATION_PARAM_TYPE;

typedef struct
{
	uint8_t mode;
	uint8_t lowSet;
	uint8_t highSet;
	uint8_t hysteresis;
} SENSOR_PARAM_TYPE;

//Temperature mode Bit Definition
//0 - Night Enabled/Disable
//1 - Day Enable/Disable
//2 - Sample Enable/Disable (if disable mesuare only on hb?)
//3 - Reserved
//4 - Reserved
//5 - Reserved
//6 - Reserved
//7 - Reserved
//0 - always disable, 3 - always enable, 1 - night only, 2 - day only, ? - sample off, ? - sample on

typedef struct
{
	uint16_t samplePeriod;           // Sensor data gathering sample period in seconds
	uint8_t sensorWarmUp;           // Time for Sensor to power up and stablize, period in seconds
	uint16_t alarmTime;              // Sensor alarm gathering total time in seconds
	uint8_t alarmSamplePeriod;      // Sensor alarm gathering sample period in seconds
	uint8_t coolDownPeriod;         // Muting period on Alarm reporting
	uint8_t chargerDisableSetting;
	SENSOR_PARAM_TYPE temperature;  //??
	SENSOR_PARAM_TYPE humidity;  //??
} ENVIRONMENT_PARAM_TYPE;

// Main Data Structure
typedef struct
{
	// KCS fix these names
	FLASH_DATA_TYPE Flash;
	TEMPERATURE_DATA_TYPE Temperature_Data;
//	ACCEL_DATA_TYPE Accelerometer_Data;
	CAMERA_DATA_TYPE Camera_Data;
	MODEM_DATA_TYPE Modem_Data;
	EXTERNAL_DATA_TYPE External_Data;
	BUZZER_DATA_TYPE buzzerData;
	HEARTBEAT_DATA_TYPE heartBeatData;
	CDATE_DATA_TYPE Cdate_Data;
	TEMP_TIME_DATA_TYPE Temptime_Data;
	GPS_LOCATION_TYPE LastPosition;

	uint32_t Buffer;				// Data buffer
	uint32_t Size;					// Memory buffer counter parameter
	uint16_t RxSize;                 //expected size of recvd message
	uint32_t State;					// Used to Check Flags
	uint16_t Battery;				//
	uint16_t UartBtyesReceived;		//
	uint8_t Init_Flags;				// Controls component intialize
	uint8_t Setting;
	uint8_t recIndex;				//
	uint8_t *PTR;					//
	uint8_t Memory [ MEMORY_MAX ];	// Memory buffer
	//	uint8_t Debugger;
	Device_Mode Mode;				// Device Mode
	Device_Mode Prev_Mode;			// Previous Device Mode
} MEM_PTR;

typedef struct
{
	// KCS fix these names
	uint16_t MSG_Size;						// Size of message
	char MSG [ Log_MSG_SIZE ];				// Actual message
	//	struct Log_Item *Next;
} Log_Item;

typedef struct
{
	// KCS fix these names
	uint8_t Buffer;
	uint64_t Debug_Val;
	Log_Item List [ Log_Size_Limit ], Curr;
} Logger_Lists;

typedef struct
{
	RTC_TimeTypeDef rtcTime;
	RTC_DateTypeDef rtcDate;
} TIME_DATE_TYPE;

typedef struct
{
	uint8_t fileNumber;
	uint8_t numberOfFiles;
	uint8_t numberOfPages; // number of pages in the file
	uint8_t numberOfMissingPages; // number of pages not received
	uint8_t batchNum;
	uint8_t fileNameLength;
	uint32_t xmitCRC;
	uint32_t lastROMAddress;
	char fileNumberString[128];
	char otaFileName[50];
	char missingPagesString[128];
	char receivedPagesString[128];
	char numofPagesString[128];
	char batchNumString[10];
	unsigned char FirmwareArray[MAXPAGES][MEMORY_MAX];
	uint16_t dataFWLength[MAXPAGES];

} OTA_FILE_TYPE;

typedef struct
{
	uint8_t numberOfPackets;
	uint8_t mQTTMessage[5][MEMORY_MAX]; // Modem receiver buffer transfer
	DOWNLINK_MSG_TYPE downLinkEnum[5];  // Modem message enumerated type
} MQTT_MSG_TYPE;

typedef struct
{
	int16_t Accel_X;				// Stores X-Axis speed
	int16_t Accel_Y;				// Stores Y-Axis speed
	int16_t Accel_Z;				// Stores Z-Axis speed
} ACCEL_DATA_TYPE;

typedef enum
{
	HDC2080 = 0x07D0,
	TEMP_UNKNOWN_DEVICE = 0
} TEMP_DEVICE_TYPE;

typedef enum
{
	LIS2DW = 0x44,              // WhoamI 0x44
	LIS2DUX = 0x47,             // WhoamI 0x47
	ACC_UNKNOWN_DEVICE = 0x00
} ACCEL_DEVICE_TYPE;
//  ****************    End of Structure Definitions   ****************************


//  ****************    Function Prototypes   ****************************

// ********** Memory clearing functions  ****************************

// Functions.c file
void clearOTAFlashArray(MEM_PTR *package);  // clears OTA program Array
void Clear_Memory ( MEM_PTR *package );
void Clear_Memory2 ( MEM_PTR *Data_Ptr , uint16_t memoryClearSize);
void clearMqttMsg(uint8_t index);
void clearMqttStruct(void);
bool checkFunctionActive(HW_MODULE_TYPE hwSubModule);
bool decodeCommand(uint8_t *msg, char *testStr);
// Skywire.c file
void Clear_Command ( MEM_PTR *package );
void clearFirmwareArray(OTA_FILE_TYPE *OTAData ); // Firmware packets Array
bool getCellConnected(void);
void setCellConnected(bool isConnected);
//   **********  End of Memory clearing functions   ******************

//   ***********   XPS functions   ****************************
// XPS.c file
void XPS_initialize ( MEM_PTR *Data_Ptr );
void XPS_paramStore ( MEM_PTR *Data_Ptr );
bool XPS_paramRetrieve ( MEM_PTR *Data_Ptr );
void XPS_DATA_Switch_Test ( MEM_PTR *Data_Ptr );
bool getOTAfwAvailable(void);
void setOTAfwAvailable(bool fwIsAvailable);
void saveParamDataToFlash(MEM_PTR *Data_Ptr);
uint8_t getMode(HW_MODULE_TYPE hwSubModule);

//  **************  End of XPS functions   ****************************

//Environment.c
void environmentParametersInit(void);
void getEnvironmentParameters(ENVIRONMENT_PARAM_TYPE *extParams);
void getTempSensorData(TEMPERATURE_DATA_TYPE *extTempParams);
void getTempDeviceType(TEMP_DEVICE_TYPE* extTemp);

void setEnvironmentParameters(ENVIRONMENT_PARAM_TYPE extParams);
bool decodeEnvironmentConfigs(uint8_t* mqttMsg);
bool decodeTemperatureConfigs(uint8_t* mqttMsg);
bool decodeHumidityConfigs(uint8_t* mqttMsg);
char* getEnvironmentConfigStr(void);
char* getTemperatureConfigStr(void);
char* getHumidityConfigStr(void);
//end environment

void test ( MEM_PTR *Mem );

void Time_Stamp ( char *MSG );
void Time_StampISO( char *MSG );
char *TimeStamp ( void );
void Log_Single ( char *MSG );

bool IsError ( MEM_PTR *Mem );
bool IsError2 ( MEM_PTR *Data_Ptr );
bool isBufferPopulated ( MEM_PTR *Data_Ptr );
void Build_MSG ( MEM_PTR *package , char *MSG );
void Word_Transfer ( char *target , char *source );
void Char_Replace ( char *target , char *source1 , char *source2 );

// KCS check if we need to move here   void Update_Battery ( uint16_t *bat_val );
void Disable_Extra_Power ( MEM_PTR *Mem );
void Enable_Modem ( MEM_PTR *Mem );
void Enable_Modem_PWR ( MEM_PTR *Data_Ptr );
void Enable_Camera ( MEM_PTR *Mem );

void Component_Initalizer ( MEM_PTR *Mem );
void Update_State ( MEM_PTR *Mem );

// functions.c

void storeMqttData(uint8_t mqttArrayIndex, uint8_t* source, uint16_t length, bool printData);
void universalDownlinkDecoder( void );
void selectDownlinkOperation(MEM_PTR *Data_Ptr, MACHINE_STATE_TYPE stateOfDevice);
bool checkCellandFix(MEM_PTR *Data_Ptr);

bool chargeCableReading(void);

//   *********** OTA functions
// bootloader.c
BANK_TYPE getSwapBank(void);

// functions.c file
bool isFivePacketsReceived( char* ptr);
void OTAProcess (MEM_PTR *Data_Ptr, OTA_FILE_TYPE *OtaData );
bool fwCRCFileCheck(uint32_t calculated, OTA_FILE_TYPE *OTAData);
bool getOTAFileInfo ( OTA_FILE_TYPE *OTAData, uint8_t index );
bool checkOTAReady (MEM_PTR *Data_Ptr);
void decodeFwPage (OTA_FILE_TYPE *OTAData );
bool handleDownlinks(MEM_PTR *Data_Ptr);
void bank2HandleFWDownlink(MEM_PTR *Data_Ptr, OTA_FILE_TYPE *OtaData);
bool isWaitingforOTA(MEM_PTR *Data_Ptr);
bool getDataValid(void);
void populateDefaults(void);
void processQuickOTA(MEM_PTR *Data_Ptr);
uint8_t populateFirmwareArray (OTA_FILE_TYPE *OTAData );
void startOTAProcess(MEM_PTR *Data_Ptr);
void cameraDownlinkHandler(MEM_PTR *Data_Ptr);
bool decodeImageAck(MEM_PTR *Data_Ptr, uint8_t index);
bool decodeHBConfigs(MEM_PTR *Data_Ptr, uint8_t *mqttMsg);
uint16_t minValue5 (uint16_t HB, uint16_t PIR, uint16_t ACC, uint16_t MUTEGPS, uint16_t BLKOUT);


//  **********   End of OTA section   *****************

// ***********    Generic file    *************************
//ArducamCamera.c

//End ArduCam
void cameraParametersInit(void);
void getCameraParameters(CAMERA_PARAMETER_TYPE *extCamParams);
void setCameraParameters(CAMERA_PARAMETER_TYPE extCamParams);
bool decodeCameraConfigs(uint8_t* mqttMsg);
char* getCamConfigStr(void);
// Functions.c file

//Container
int8_t initContainer(STRING_CONTAINER* container);
void printContainer(STRING_CONTAINER* container);
int8_t addErrorString(char* externalStr);
int8_t addToContainer(STRING_CONTAINER* container, const char* str);
void freeContainer(STRING_CONTAINER* container);
void sendConfigErrors(MEM_PTR *Mem, STRING_CONTAINER* container);
//End container
void test ( MEM_PTR *Mem );

bool IsError ( MEM_PTR *Mem );
bool isBufferPopulated ( MEM_PTR *Data_Ptr );
void Build_MSG ( MEM_PTR *package , char *MSG );
void Word_Transfer ( char *target , char *source );
void Char_Replace ( char *target , char *source1 , char *source2 );

void Disable_Extra_Power ( MEM_PTR *Mem );
void Enable_Modem ( MEM_PTR *Mem );
void Enable_Modem_PWR ( MEM_PTR *Data_Ptr );
void cameraPowerControl (bool isOn);

void Component_Initalizer ( MEM_PTR *Mem );
void Update_State ( MEM_PTR *Mem );

void memory_Init ( MEM_PTR *Data_Ptr );
void memoryTest1 ( MEM_PTR *Data_Ptr );
void memoryTest2 ( MEM_PTR *Data_Ptr );
void PACKAGE_Init ( MEM_PTR *package );
void LOG_Init ( void );

void Time_Stamp ( char *MSG );
char *TimeStamp ( void );
void Time_StampISO( char *MSG );
void Log_Single ( char *MSG );
void stop3Prepare (void);

void powerDownDeviceForSleep(void);
void clearPirInterreptUpdateStates(void);
void clearAccellerometerInterruptStates(void);
void sendBatteryStatus(STATUS_BATTERY_TYPE status);

void storeMqttData(uint8_t mqttArrayIndex, uint8_t* source, uint16_t length, bool printData);
void universalDownlinkDecoder( void );
void selectDownlinkOperation(MEM_PTR *Data_Ptr, MACHINE_STATE_TYPE stateOfDevice);
bool checkCellandFix(MEM_PTR *Data_Ptr);

void buzzerTone(void);
void restartModem (void);

// skywire.c file
void fakeMissing(OTA_FILE_TYPE *OTAData );
void sendDeviceConfig( MEM_PTR *Data_Ptr, STATUS_UPLINK_TYPE uplnkType);
void sendDiagnostic(MEM_PTR *Data_Ptr, char* message);
char* gpsGetLocationBuff(void);


//Accelerometer.c file
void accelInit(void);
void setAccelParameters(ACCELERATION_PARAM_TYPE extAccelParameters);
void getAccelData(ACCEL_DATA_TYPE *extAccelData, bool acquireData);
void getAccelParameters(ACCELERATION_PARAM_TYPE *extAccelParameters);
void accelDataInit(void);
void accelParametersInit(void);
bool decodeAccelConfigs(uint8_t *mqttMsg);
void readAcceleration(void);
void handleAccelTrigger(void);
void writeDataToReportingArray(bool read, DATA_TYPE_TAG tag, char *dataStr, ...);
void PrintDataArray(void);
void ClearDataArray(void);
void accelClearLatch(void);
void saveOTAData(MEM_PTR *Data_Ptr, OTA_FILE_TYPE *OtaData );
void clearShadowMemory(void);
void readDataValidFromFlash(MEM_PTR *Data_PtrData_Ptr );
void setAccelDataRate(ACCEL_DATA_RATE setRate);
void setAccelMode(ACCEL_POWER_MODE setMode);
void setAccelResolution(ACCEL_RESOLUTION setResolution);
void setAccelFullScaleRange(ACCEL_FULL_SCALE setRange);
uint16_t getAccelMutePeriod(void);
char* getAccelConfigStr(void);
void getAccelDeviceType(ACCEL_DEVICE_TYPE * extAccel);
//Test
void testWakeUpInterruptOccur(void);
void testMode(void);
void testFIFO(void);
// DayNight.c file
bool decodeDayNightConfigs(uint8_t *mqttMsg);
void getBusinessHours(BUSINESS_DATA_TYPE *extBusinessHours);
void setBusinessHours(BUSINESS_DATA_TYPE extBusinessHours);
char* dayNightConfigStr();
bool isNight(void);
bool isNight2(void);
bool isTimeDefault(void);

//GPS.c file
void gpsParametersInit(void);
void gpsDataInit(void);
bool isGpsTimeSyncEnabled(void);
bool decodeGPSConfigs(uint8_t *mqttMsg);
void getGpsParameters(GPS_PARAMETER_TYPE *extGpsParameters);
void setGpsParameters(GPS_PARAMETER_TYPE extGpsParameters);
char* getGpsConfigStr(void);
void gpsClearLocation(void);
bool gpsGetData(char* gpsMsg);
char* getAccelConfigStr(void);

// systemTiming.c file
uint16_t getHbInterval ( void );
uint8_t getNumDays( RTC_DateTypeDef checkDate );

void calculateNextTime( TIME_DATE_TYPE* next, uint32_t timeSec );
bool checkNextTime( TIME_DATE_TYPE check, TIME_DATE_TYPE next );
uint32_t getTimeDifference( TIME_DATE_TYPE check, TIME_DATE_TYPE next );
uint16_t getTimeFromNow( TIME_DATE_TYPE stop );


//PIR.c file
uint8_t PIRConfirmMotionFilter (void);
uint16_t getPIRBlackoutPeriod(void);


//  *****************************

void SystemClock_Config(void);
void MX_GPIO_Init(void);
void MX_I2C1_Init(void);
void MX_ADC1_Init(void);
void MX_OCTOSPI1_Init(void);
void MX_USART2_UART_Init(void);
void MX_SPI2_Init(void);
void MX_LPUART1_UART_Init(void);
void MX_LPTIM1_Init(void);
void MX_CRC_Init(void);
void MX_LPTIM2_Init(void);
void MX_LPTIM3_Init(void);
void sendHeartBeat(MEM_PTR *Data_Ptr);

/**
 * \brief          Encode a buffer into base64 format
 *
 * \param dst      destination buffer
 * \param dlen     size of the buffer
 * \param src      source buffer
 * \param slen     amount of data to be encoded
 *
 * \return         0 if successful, or POLARSSL_ERR_BASE64_BUFFER_TOO_SMALL.
 *                 *dlen is always updated to reflect the amount
 *                 of data that has (or would have) been written.
 *
 * \note           Call this function with *dlen = 0 to obtain the
 *                 required buffer size in *dlen
 */
int base64_encode ( unsigned char *dst , size_t *dlen , const unsigned char *src , size_t slen );

/**
 * \brief          Decode a base64-formatted buffer
 *
 * \param dst      destination buffer
 * \param dlen     size of the buffer
 * \param src      source buffer
 * \param slen     amount of data to be decoded
 *
 * \return         0 if successful, POLARSSL_ERR_BASE64_BUFFER_TOO_SMALL, or
 *                 POLARSSL_ERR_BASE64_INVALID_CHARACTER if the input data is
 *                 not correct. *dlen is always updated to reflect the amount
 *                 of data that has (or would have) been written.
 *
 * \note           Call this function with *dlen = 0 to obtain the
 *                 required buffer size in *dlen
 */
int base64_decode ( unsigned char *dst , size_t *dlen , const unsigned char *src , size_t slen );

int onTheFlyDcodeBase64(unsigned char *input, unsigned char *output, int sourceSize);

#ifdef Log
void Log_Insert ( char *MSG , uint32_t Command );
void Log_End ( char *MSG , uint32_t Command );
void Write_Log ( char *MSG );
void vcom_Trace ( uint8_t *p_data , uint16_t size , int block );
void Read_Last_UART_Log ( MEM_PTR *Mem );
void Read_All_UART_Log ( void );
//uint8_t MQTTMsg[5][MEMORY_MAX];

#ifdef XPS_Logs
void Write_XPS_Log(void);
void Read_Last_XPS_Log(void);
void Read_All_XPS_Log(void);
#endif
#endif

// Calculates a calendar HTC time based upon the present time and a requested time delay
// Parameters
// desiredStopTime - RTC date and time structures
// timeDelay - time in seconds to add to present time
// returns a boolean error, true is error
bool calculateStopTime ( TIME_DATE_TYPE *desiredStopTime, uint16_t timeDelay);

// returns a true if time is expired, get system time and compare to timeDelay
bool isTimeExpired (TIME_DATE_TYPE *sTime);

int8_t TraceSend(int8_t block, const char *strFormat, ...);
void testFunction(MEM_PTR *Data_Ptr);
void testMqtt(void);
void setupTestPackages(OTA_FILE_TYPE *OTAData);
void setupTestMqtt(void);
//csc used to push data to data array
void WriteDataArray( DATA_TYPE_TAG tag, char *dataStr, ...);
//csc used to print data from data array
void PrintDataArray();
//csc created to clear the data array just in case we need it later
void ClearDataArray();
//main.c
void handleFaults(void);


//  ****************    End of Function Prototypes   ****************************

// ****************   Global Maps ******************

static const unsigned char base64_enc_map [ 64 ] = { 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
		'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i',
		'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '0', '1', '2', '3', '4',
		'5', '6', '7', '8', '9', '+', '/' };

static const unsigned char base64_dec_map [ 128 ] = { 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127,
		127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127,
		127, 127, 127, 127, 127, 127, 127, 127, 62, 127, 127, 127, 63, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 127, 127,
		127, 64, 127, 127, 127, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23,
		24, 25, 127, 127, 127, 127, 127, 127, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43,
		44, 45, 46, 47, 48, 49, 50, 51, 127, 127, 127, 127, 127 };

// ****************   End of Global Maps ******************

// ************ External Parmaeters based upon Structure Definitions   **********

//extern PIR_PARAMETER_TYPE pirData;
//extern BUSINESS_DATA_TYPE operatingHours;

#endif /* FUNCTIONS_H_ */
