/**
 ******************************************************************************
 * @file    Skywire.c
 * @author  Erik Pineda-A
 * @brief   driver for Nimbelink Skywire modem
 ******************************************************************************
 * @attention
 *
 * Copyright 2024 Team of Defenders
 *
 ******************************************************************************
 */
#ifndef SKYWIRE_C_
#define SKYWIRE_C_

#include "Functions.h"
#include "Skywire.h"
#include "PIR.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

/******************************************************
 Global Variables
 Note:
 ******************************************************/
extern UART_HandleTypeDef hlpuart1;			// External UART configuration
extern ADC_HandleTypeDef hadc4;
extern RTC_HandleTypeDef hrtc;
extern CRC_HandleTypeDef hcrc;
extern CELL_STATUS_REG CELL;
//UART_HandleTypeDef UART_Setting;			// UART controller buffer
PACKET_INDEX_TYPE pageIndexArray[100];
uint8_t dummy = 1;
uint16_t receivedDataSize = 0;
uint16_t resendPages[100] = {0};
extern uint16_t missingPagesCount;
bool lastPacket = false;
bool interruptTest = false;
bool useRecvMode1 = true;
bool oneconfigtest = true;
bool retry = false;
char batVal[10];

extern bool cellInitialized;
extern uint16_t pagesToResend[100];
int InitCounter = 0;

uint8_t receivedData [ MEMORY_MAX ] = {0};

TEMPERATURE_DATA_TYPE tempParameters;
// need to move these so can be accessed by functions.c, eventually into a structure

/******************************************************
 Define Control
 Note:
 ******************************************************/
#define TX 			CELL.TX
#define Address 	CELL.ADDRESS
#define Command 	CELL.COMMAND
#define Temporary 	CELL.Temp
#define Cell_State 	CELL.STATE
#define Timer 		CELL.Timer
#define MQTT_ID		CELL.MQTT_DEVID				// Message ID
#define Message_ID	CELL.MSGID					// Message ID
#define Cell_Dev_ID CELL.DEVICE_ID
#define Sim_ID		CELL.SIMID
#define Signal		CELL.Signal
#define Cell_ID     CELL.CellId
#define SIM_PIN     CELL.SIMID
#define TX_Command	CELL.TX_COMMAND
#define Modem_ID 	CELL.Modem_ID
#define Username	CELL.USERNAME				// Server Username
#define Password	CELL.PASSWORD				// Server Password
#define Event_Type	CELL.EVENT					// For package building
#define TX_Size		CELL.TX_Size
#define PDP 		CELL.PDP					// Current state of Cell Module
#define Fail_Cntr	CELL.FAIL_CNT
#define Encoded_Picture_Buffer CELL.Counter
#define Version CELL.Camera_Header_Data.Version
#define Service CELL.Camera_Header_Data.Service
#define Unique_ID CELL.Camera_Header_Data.Unique_ID
#define Picture_ID CELL.Camera_Header_Data.Picture_ID
#define Page CELL.Camera_Header_Data.Page
#define Total_Page CELL.Camera_Header_Data.Total_Page
#define Checksum CELL.Camera_Header_Data.Checksum
#define Data_Length CELL.Camera_Header_Data.Data_Length
#define Header CELL.Header

#define Setting 	_Setting
#define DEV_State	_State
#define Size 		_Size
#define RxSize 	_RxSize
#define Memory 		_Memory
#define Pointer		_Pointer
//#define Temperature _Temperature
//#define Humidity _Humidity
//#define Temp_Device_ID _Temp_Device_ID
//#define Acce_Device_ID _Acce_Device_ID
//#define X_Axis _X_Axis
//#define Y_Axis _Y_Axis
//#define Z_Axis _Z_Axis
#define Cell_Date_Day _Cell_Date_Day
#define Cell_Date_Month _Cell_Date_Month
#define Cell_Date_Year _Cell_Date_Year
#define Temp_Time1 _Temp_Time1
#define Temp_Time2 _Temp_Time2
//#define S_Date sDate.Date     //need to pass sdate and stime data to this file

/******************************************************
 Cell Initialize Function
 Initializes the MQTT parameters as well as
 establish bridge with server.
 ******************************************************/
void CELL_Init(MEM_PTR *Data_Ptr)
{
#ifdef Log_Level_2
	if (PDP == PDP_NOT_SET)
		Log_Single( LOG_MSG_PDP_CONFIG);
	else if (PDP == PDP_SET)
		Log_Single( LOG_MSG_CELL_Init);
	else
		Log_Single("PDP STATE BUG \r\n\0");
#endif  // Log_Level_2
	/* IWDG Timer Restart */
	if (HAL_IWDG_Refresh(&hiwdg) != HAL_OK)
		Error_Handler();

	Command = GPS_DISABLE;
	CELL_COMMAND(Data_Ptr);

	Command = CHECK;
	CELL_COMMAND(Data_Ptr);
	if ( PDP == PDP_NOT_SET )
	{
		PDP = PDP_SET;
		Fail_Cntr = CSQ_MAX_FAIL;
		utoa( UNIQUE_Device_ID, MQTT_ID, 10);
		Word_Transfer( Message_ID, MQTT_DEFAULT_MSG_ID);
		Word_Transfer( Username, MQTT_DEFAULT_USERNAME);
		Word_Transfer( Password, MQTT_DEFAULT_PASSWORD);
		Word_Transfer( TX_Size, MQTT_MSGSIZE);
		Word_Transfer( Event_Type, MQTT_DEFAULT_EVENT);
		Time_Stamp(Picture_ID);
		utoa ( UNIQUE_Device_ID , (char*) Cell_Dev_ID , 10 );
#ifdef PDP_CONTEXT
		Command = CHECK;
#endif //PDP_CONTEXT
#ifndef PDP_CONTEXT
		Command = PDP_POPULATE;
#endif // not PDP_CONTEXT
		CELL_COMMAND( Data_Ptr );
		Clear_Memory(Data_Ptr);
		Command = CONFIG;
		CELL_COMMAND( Data_Ptr );
		Clear_Memory(Data_Ptr);
		Command = activatePDP;
		CELL_COMMAND(Data_Ptr);
		cellResponseCheck(Data_Ptr);
		Command = REENABLEQI;
		CELL_COMMAND( Data_Ptr );
		cellResponseCheck(Data_Ptr);
		Clear_Memory(Data_Ptr);
		Command = SIM_ID;
		CELL_COMMAND( Data_Ptr );
		Char_Replace((char*) Memory, "\n", " ");
		Char_Replace((char*) Memory, "\r", " ");
		Word_Transfer((char*) Sim_ID, (char*) Memory);
		Command = MODEM_ID;
		CELL_COMMAND(Data_Ptr);
		Char_Replace((char*) Memory, "\n", " ");
		Char_Replace((char*) Memory, "\r", " ");
		Word_Transfer((char*) Modem_ID, (char*) Memory);
		Command = CHECK;
		CELL_COMMAND(Data_Ptr);
	}
	else if (PDP == PDP_SET)
	{
		CELL_Set_Command ( CONFIG );
		CELL_COMMAND ( Data_Ptr );
		Command = CHECK;
		CELL_COMMAND(Data_Ptr);
	}
	else
		Log_Single("Modem PDP Cell_State Fail\r\n\0");
	Command = CONNECTION_TEST;
	CELL_COMMAND(Data_Ptr);
	if (Memory[26] == '3')
	{
		Cell_State = CELL_OK;
		return;
	}
#ifdef NO_MODEM_RESTART
	Command = DISCONNECT;
	CELL_COMMAND ( Data_Ptr );
	Command = CHECK;
	CELL_COMMAND ( Data_Ptr );
	Command = CLOSE;
	CELL_COMMAND ( Data_Ptr );
	Command = CHECK;
	CELL_COMMAND ( Data_Ptr );
#endif  // NO_MODEM_RESTART


	//	Command = CHECK;
	//	for ( Temporary = 0; Cell_State != CELL_OK; Temporary++ )
	//	{
	//		CELL_COMMAND(Data_Ptr);
	//		if (Temporary >= TEMP_LIMIT)
	//			break;
	//	}
#ifdef CERTIFICATE
	Word_Transfer(Event_Type , SSL_CACERT);
	Command = FILE_DELETE; CELL_COMMAND(Data_Ptr,CELL);
	Command = CHECK; CELL_COMMAND(Data_Ptr,CELL);
	Command = FILE_CREATE; CELL_COMMAND(Data_Ptr,CELL);
	HAL_Delay(10);
	Word_Transfer((char *)Memory , CERT_A);
	Command = FILE_POPULATE;CELL_COMMAND(Data_Ptr,CELL);
	HAL_Delay(10);
	Word_Transfer((char *)Memory , CERT_B);
	Command = FILE_POPULATE;CELL_COMMAND(Data_Ptr,CELL);
	do
	{
		Command = CHECK; CELL_COMMAND(Data_Ptr,CELL);
	}while (State!= CELL_OK);
	Word_Transfer(Event_Type , SSL_CLCERT);
	Command = FILE_DELETE; CELL_COMMAND(Data_Ptr,CELL);
	Command = CHECK; CELL_COMMAND(Data_Ptr,CELL);
	Command = FILE_CREATE; CELL_COMMAND(Data_Ptr,CELL);
	HAL_Delay(10);
	Word_Transfer((char *)Memory , CERT_A);
	Command = FILE_POPULATE;CELL_COMMAND(Data_Ptr,CELL);
	HAL_Delay(10);
	Word_Transfer((char *)Memory , CERT_B);
	Command = FILE_POPULATE;CELL_COMMAND(Data_Ptr,CELL);
	do
	{
		Command = CHECK; CELL_COMMAND(Data_Ptr,CELL);
	}while (State!= CELL_OK);
	Word_Transfer(Event_Type , SSL_CLNTKEY);
	Command = FILE_DELETE; CELL_COMMAND(Data_Ptr,CELL);
	Command = CHECK; CELL_COMMAND(Data_Ptr,CELL);
	Command = FILE_CREATE; CELL_COMMAND(Data_Ptr,CELL);
	HAL_Delay(10);
	Word_Transfer((char *)Memory , CERT_A);
	Command = FILE_POPULATE;CELL_COMMAND(Data_Ptr,CELL);
	HAL_Delay(10);
	Word_Transfer((char *)Memory , CERT_B);
	Command = FILE_POPULATE;CELL_COMMAND(Data_Ptr,CELL);
	do
	{
		Command = CHECK; CELL_COMMAND(Data_Ptr,CELL);
	}while (State!= CELL_OK);
#endif  // CERTIFICATE
	//	HAL_Delay ( 1000 );
	//	Update_Battery ( &Data_Ptr -> Battery );
	// MQTT
	Clear_Memory(Data_Ptr);
	Command = OPEN;
	CELL_COMMAND(Data_Ptr);
	cellResponseCheck(Data_Ptr);
	Clear_Memory(Data_Ptr);
	Command = CONNECT;
	CELL_COMMAND(Data_Ptr);
	cellResponseCheck(Data_Ptr);
	Clear_Memory(Data_Ptr);
	Command = SUBSCRIBEDOWN;
	CELL_COMMAND(Data_Ptr);
	cellResponseCheck(Data_Ptr);
	Clear_Memory(Data_Ptr);
	if ( retry )
	{
		if (InitCounter < 3)
		{
			InitCounter ++;
			CELL_Set_PDP ( PDP_NOT_SET );
			CELL_Init(Data_Ptr);
		}
		else
		{
			buzzerTone();
		}
	}
	retry = false;
	InitCounter = 0;
	Command = CHECK;
	CELL_COMMAND(Data_Ptr);
	Command = CONNECTION_TEST;
	CELL_COMMAND(Data_Ptr);
	InitCounter = 0;
	if ( Memory[26] != '3' )
		return;
}

void CELL_reInit(MEM_PTR *Data_Ptr)     //REFRESH FUNCTION WIP
{
	utoa( UNIQUE_Device_ID, MQTT_ID, 10);
	Word_Transfer( Message_ID, MQTT_DEFAULT_MSG_ID);
	Word_Transfer( Username, MQTT_DEFAULT_USERNAME);
	Word_Transfer( Password, MQTT_DEFAULT_PASSWORD);
	Word_Transfer( TX_Size, MQTT_MSGSIZE);
	Word_Transfer( Event_Type, MQTT_DEFAULT_EVENT);
	Time_Stamp(Picture_ID);
	utoa ( UNIQUE_Device_ID , (char*) Cell_Dev_ID , 10 );

#ifdef PDP_CONTEXT
	Command = CHECK;
#endif //PDP_CONTEXT

	//#ifndef PDP_CONTEXT
	//		Command = PDP_POPULATE;
	//#endif // not PDP_CONTEXT
	//		CELL_COMMAND( Data_Ptr );

	if (oneconfigtest)
	{
		Command = CONFIG;
		CELL_COMMAND( Data_Ptr );
		Clear_Memory(Data_Ptr);

		Command = GPS_DISABLE;
		CELL_COMMAND(Data_Ptr);
		oneconfigtest = false;
		HAL_Delay(1500);
	}

	//		Command = REENABLEQI;
	//		CELL_COMMAND( Data_Ptr );


	//		Command = SIM_ID;
	//		CELL_COMMAND( Data_Ptr );
	//
	//		Char_Replace((char*) Memory, "\n", " ");
	//		Char_Replace((char*) Memory, "\r", " ");
	//
	//		Word_Transfer((char*) Sim_ID, (char*) Memory);
	//
	//		Command = MODEM_ID;
	//		CELL_COMMAND(Data_Ptr);
	//
	//		Char_Replace((char*) Memory, "\n", " ");
	//		Char_Replace((char*) Memory, "\r", " ");
	//
	//		Word_Transfer((char*) Modem_ID, (char*) Memory);


#ifdef NO_MODEM_RESTART
	Command = DISCONNECT;
	CELL_COMMAND ( Data_Ptr );
	Command = CHECK;
	CELL_COMMAND ( Data_Ptr );
	Command = CLOSE;
	CELL_COMMAND ( Data_Ptr );
	Command = CHECK;
	CELL_COMMAND ( Data_Ptr );
#endif  // NO_MODEM_RESTART



#ifdef CERTIFICATE
	Word_Transfer(Event_Type , SSL_CACERT);
	Command = FILE_DELETE; CELL_COMMAND(Data_Ptr,CELL);
	Command = CHECK; CELL_COMMAND(Data_Ptr,CELL);
	Command = FILE_CREATE; CELL_COMMAND(Data_Ptr,CELL);
	HAL_Delay(10);
	Word_Transfer((char *)Memory , CERT_A);
	Command = FILE_POPULATE;CELL_COMMAND(Data_Ptr,CELL);
	HAL_Delay(10);
	Word_Transfer((char *)Memory , CERT_B);
	Command = FILE_POPULATE;CELL_COMMAND(Data_Ptr,CELL);
	do
	{
		Command = CHECK; CELL_COMMAND(Data_Ptr,CELL);
	}while (State!= CELL_OK);
	Word_Transfer(Event_Type , SSL_CLCERT);
	Command = FILE_DELETE; CELL_COMMAND(Data_Ptr,CELL);
	Command = CHECK; CELL_COMMAND(Data_Ptr,CELL);
	Command = FILE_CREATE; CELL_COMMAND(Data_Ptr,CELL);
	HAL_Delay(10);
	Word_Transfer((char *)Memory , CERT_A);
	Command = FILE_POPULATE;CELL_COMMAND(Data_Ptr,CELL);
	HAL_Delay(10);
	Word_Transfer((char *)Memory , CERT_B);
	Command = FILE_POPULATE;CELL_COMMAND(Data_Ptr,CELL);
	do
	{
		Command = CHECK; CELL_COMMAND(Data_Ptr,CELL);
	}while (State!= CELL_OK);
	Word_Transfer(Event_Type , SSL_CLNTKEY);
	Command = FILE_DELETE; CELL_COMMAND(Data_Ptr,CELL);
	Command = CHECK; CELL_COMMAND(Data_Ptr,CELL);
	Command = FILE_CREATE; CELL_COMMAND(Data_Ptr,CELL);
	HAL_Delay(10);
	Word_Transfer((char *)Memory , CERT_A);
	Command = FILE_POPULATE;CELL_COMMAND(Data_Ptr,CELL);
	HAL_Delay(10);
	Word_Transfer((char *)Memory , CERT_B);
	Command = FILE_POPULATE;CELL_COMMAND(Data_Ptr,CELL);
	do
	{
		Command = CHECK; CELL_COMMAND(Data_Ptr,CELL);
	}while (State!= CELL_OK);
#endif  // CERTIFICATE
	//	HAL_Delay ( 1000 );
	//	Update_Battery ( &Data_Ptr -> Battery );
	// MQTT

	Clear_Memory(Data_Ptr);  //GAV use clear memory 2 for all these

	Command = activatePDP;
	CELL_COMMAND(Data_Ptr);
	if (cellResponseCheck(Data_Ptr) != 1)
	{
		Clear_Memory(Data_Ptr);

		Command = REENABLEQI;
		CELL_COMMAND( Data_Ptr );
		cellResponseCheck(Data_Ptr);
	}

	Clear_Memory(Data_Ptr);

	Command = OPEN;
	CELL_COMMAND(Data_Ptr);

	if (cellResponseCheck(Data_Ptr) != 1)
	{
		PRINTF("FAILURE FAILURE FAILURE\r\n");
	}

	Clear_Memory(Data_Ptr);

	//	Command = CHECK;
	//	CELL_COMMAND(Data_Ptr);
	Command = CONNECT;
	CELL_COMMAND(Data_Ptr);
	if (cellResponseCheck(Data_Ptr) != 1)
	{
		PRINTF("FAILURE FAILURE FAILURE\r\n");
	}
	Clear_Memory(Data_Ptr);

	Command = SUBSCRIBEDOWN;
	CELL_COMMAND(Data_Ptr);
	if (cellResponseCheck(Data_Ptr) != 1)
	{
		PRINTF("FAILURE FAILURE FAILURE\r\n");
	}
	else
	{
		cellInitialized = true;
	}

	Clear_Memory(Data_Ptr);

	if ( retry )
	{
		if (InitCounter < 3)
		{
			InitCounter ++;
			CELL_reInit(Data_Ptr);
		}
		else
		{
			buzzerTone();
		}
	}

	retry = false;
	InitCounter = 0;
	oneconfigtest = true;
}

void cellInitTest (MEM_PTR *Data_Ptr)
{
	Command = CONFIG;
	CELL_COMMAND( Data_Ptr );
	for (int test = 0; test < 100; test++)
	{
		TX_Command[test] = 0;
	}
	Clear_Memory(Data_Ptr);

	Command = REENABLEQI;
	CELL_COMMAND( Data_Ptr );
	Command = OPEN;
	CELL_COMMAND(Data_Ptr);
	Command = CHECK;
	CELL_COMMAND(Data_Ptr);
	Command = CONNECT;
	CELL_COMMAND(Data_Ptr);
	Command = CHECK;
	CELL_COMMAND(Data_Ptr);
	Command = SUBSCRIBEDOWN;
	CELL_COMMAND(Data_Ptr);
	Command = CHECK;
	CELL_COMMAND(Data_Ptr);
}


/******************************************************
 Cell RX Function
 Checks RX buffer and completes requests from server
 ******************************************************/
void CELL_SERVER_UPDT(MEM_PTR *Data_Ptr)
{
#ifdef Log_Level_2
	Log_Single( LOG_MSG_CELL_SERVER_UPDT);
#endif // Log_Level_2

	Command = CHECK;
	CELL_COMMAND(Data_Ptr);
	if (Cell_State != CELL_OK)
		return;

	//	HAL_Delay ( 10 );
	Command = READ_BUF;
	CELL_COMMAND(Data_Ptr);
	buf = 0;
	Pointer = Memory;
	//	do
	//	{
	//		if ( *(Pointer) == 'd' && *(Pointer + 1) == 'e' && *(Pointer + 2) == 'v' && *(Pointer + 3) == 'I' && *(Pointer + 4) == 'D')
	//		{
	//			Pointer += 9;
	//			while ( *(Pointer) != '\'')
	//			{
	//				Server_Instructions[ buf++] = *(Pointer);
	//				(Pointer)++;
	//			}
	//
	//			buf = 0;
	//			do
	//			{
	//				if (Server_Instructions[ buf] != MQTT_ID[ buf])
	//					return;
	//			} while (Server_Instructions[ buf++]);
	//
	//			Pointer += 12;
	//			//Clear_Instruction(Data_Ptr);
	//			buf = 0;
	//
	//			while (*(Pointer) != '\'')
	//			{
	//				Server_Instructions[ buf++] = *(Pointer);
	//				(Pointer)++;
	//			}
	//			return;
	//		}
	//		(Pointer)++;
	//	} while (*(Pointer));

	//	HAL_Delay(10);
	Command = CHECK;
	CELL_COMMAND(Data_Ptr);
	//	if ( Cell_State != CELL_OK )
	//		{Setting ^= CELL_INIT;}
}

/******************************************************
 Cell Power Function
 Sends current signal strength to MQTT server
 ******************************************************/
void CELL_PWRUPDT(MEM_PTR *Data_Ptr)
{
#ifdef Log_Level_2
	Log_Single( LOG_MSG_CELL_PWRUPDT);
#endif // Log_Level_2

	Command = CHECK;
	CELL_COMMAND(Data_Ptr);
	if (Cell_State != CELL_OK)
		return;

	Command = SIGNAL;
	CELL_COMMAND(Data_Ptr);

	Char_Replace((char*) Memory, "\n", " ");
	Char_Replace((char*) Memory, "\r", " ");

	Word_Transfer((char*) Signal, (char*) Memory);
	Word_Transfer( Event_Type, MQTT_SIG_EVENT);

	Command = CHECK;
	CELL_COMMAND(Data_Ptr);

	Clear_Memory(Data_Ptr);
	buf = 0;

	Build_MSG(Data_Ptr, MQTT_HEAD_MASK_START);
	utoa( UNIQUE_Device_ID, Header, 10);
	Build_MSG(Data_Ptr, Header);
	Build_MSG(Data_Ptr, MQTT_HEAD_MASK_END);
	Build_MSG(Data_Ptr, Event_Type);
	Build_MSG(Data_Ptr, (char*) Signal);
	Build_MSG(Data_Ptr, MQTT_TAIL_MASK);
	Size = buf;

	Command = PUBLISH;
	CELL_COMMAND(Data_Ptr);
	//	HAL_Delay ( MSG_Delay );
	Command = PUBLISH_MEM;
	CELL_COMMAND(Data_Ptr);

	Command = CHECK;
	CELL_COMMAND(Data_Ptr);

	//	HAL_Delay ( 5 );

	Command = READ_BUF;
	CELL_COMMAND(Data_Ptr);
}

///******************************************************
// Cell Acceleration Function
// Sends current acceleration to MQTT server
// ******************************************************/
//void CELL_TEMPUPDT(MEM_PTR *Data_Ptr)
//{
//#ifdef Log_Level_2
//	Log_Single( LOG_MSG_CELL_TEMPUPDT);
//#endif // Log_Level_2
//
//	//	HAL_ADC_Start(&hadc4);
//	//	Temperature = HAL_ADC_GetValue(&hadc4);
//	//	HAL_ADC_Stop(&hadc4);
//
//	//	Memory[0] = *((uint8_t*)&(Temperature)+1); //high byte
//	//	Memory[1] = *((uint8_t*)&(Temperature)+0); //low byte
//	//	Memory[2] = 0;
//	Command = CHECK;
//	CELL_COMMAND(Data_Ptr);
//	if (Cell_State != CELL_OK)
//		return;
//
//	Word_Transfer( Event_Type, MQTT_TEMPERATURE_EVENT);
//
//	Clear_Memory(Data_Ptr);
//	buf = 0;
//
//	Build_MSG(Data_Ptr, MQTT_HEAD_MASK_START);
//	utoa( UNIQUE_Device_ID, Header, 10);
//	Build_MSG(Data_Ptr, Header);
//	Build_MSG(Data_Ptr, MQTT_HEAD_MASK_END);
//	Build_MSG(Data_Ptr, Event_Type);
//	utoa( Temp_Device_ID, (char*) (&(Memory[ buf])), 10);
//
//	while (Memory[ buf])
//		buf++;
//
//	Build_MSG(Data_Ptr, MQTT_APND);
//	utoa( Temperature, (char*) (&(Memory[ buf])), 10);
//
//	while (Memory[ buf])
//		buf++;
//
//	Build_MSG(Data_Ptr, MQTT_TAIL_MASK);
//	Size = buf;
//
//	Command = PUBLISH;
//	CELL_COMMAND(Data_Ptr);
//	//	HAL_Delay ( MSG_Delay );
//	Command = PUBLISH_MEM;
//	CELL_COMMAND(Data_Ptr);
//
//	Command = CHECK;
//	CELL_COMMAND(Data_Ptr);
//
//	//	HAL_Delay ( 5 );
//
//	Command = READ_BUF;
//	CELL_COMMAND(Data_Ptr);
//}

/******************************************************
 Cell Acceleration Function
 Sends speed of all axis to MQTT server
 ******************************************************/
void CELL_ACCEUPDT(MEM_PTR *Data_Ptr)
{
#ifdef Log_Level_2
	Log_Single( LOG_MSG_CELL_ACCEUPDT);
#endif // Log_Level_2

	Command = CHECK;
	CELL_COMMAND(Data_Ptr);
	if (Cell_State != CELL_OK)
		return;

	Word_Transfer( Event_Type, MQTT_2FAST_EVENT);

	Clear_Memory(Data_Ptr);
	buf = 0;

	Build_MSG(Data_Ptr, MQTT_HEAD_MASK_START);
	utoa( UNIQUE_Device_ID, Header, 10);
	Build_MSG(Data_Ptr, Header);
	Build_MSG(Data_Ptr, MQTT_HEAD_MASK_END);
	Build_MSG(Data_Ptr, Event_Type);
//	utoa( Acce_Device_ID, (char*) (&(Memory[ buf])), 10);

	while (Memory[ buf])
		buf++;

	Build_MSG(Data_Ptr, MQTT_APND);
//	utoa( X_Axis, (char*) (&(Memory[ buf])), 10);

	while (Memory[ buf])
		buf++;

	Build_MSG(Data_Ptr, MQTT_APND);
//	utoa( Y_Axis, (char*) (&(Memory[ buf])), 10);

	while (Memory[ buf])
		buf++;

	Build_MSG(Data_Ptr, MQTT_APND);
//	utoa( Z_Axis, (char*) (&(Memory[ buf])), 10);

	while (Memory[ buf])
		buf++;

	Build_MSG(Data_Ptr, MQTT_TAIL_MASK);
	Size = buf;

	Command = PUBLISH;
	CELL_COMMAND(Data_Ptr);
	//	HAL_Delay ( MSG_Delay );
	Command = PUBLISH_MEM;
	CELL_COMMAND(Data_Ptr);

	Command = CHECK;
	CELL_COMMAND(Data_Ptr);

	//	HAL_Delay ( 5 );

	Command = READ_BUF;
	CELL_COMMAND(Data_Ptr);
}

///******************************************************
// Cell Temperature_Data.Humidity Function
// Sends humidity percentage to MQTT server
// ******************************************************/
//void CELL_HUMDUPDT(MEM_PTR *Data_Ptr)
//{
//#ifdef Log_Level_2
//	Log_Single( LOG_MSG_CELL_HUMDUPDT);
//#endif // Log_Level_2
//
//	Command = CHECK;
//	CELL_COMMAND(Data_Ptr);
//	if (Cell_State != CELL_OK)
//		return;
//
//	Word_Transfer( Event_Type, MQTT_HUMID_EVENT);
//
//	Clear_Memory(Data_Ptr);
//	buf = 0;
//
//	Build_MSG(Data_Ptr, MQTT_HEAD_MASK_START);
//	utoa( UNIQUE_Device_ID, Header, 10);
//	Build_MSG(Data_Ptr, Header);
//	Build_MSG(Data_Ptr, MQTT_HEAD_MASK_END);
//	Build_MSG(Data_Ptr, Event_Type);
//	utoa( Temp_Device_ID, (char*) (&(Memory[ buf])), 10);
//
//	while (Memory[ buf])
//		buf++;
//
//	Build_MSG(Data_Ptr, MQTT_APND);
//	utoa( Humidity, (char*) (&(Memory[ buf])), 10);
//
//	while (Memory[ buf])
//		buf++;
//
//	Build_MSG(Data_Ptr, MQTT_TAIL_MASK);
//	Size = buf;
//
//	Command = PUBLISH;
//	CELL_COMMAND(Data_Ptr);
//	//	HAL_Delay ( MSG_Delay );
//	Command = PUBLISH_MEM;
//	CELL_COMMAND(Data_Ptr);
//
//	Command = CHECK;
//	CELL_COMMAND(Data_Ptr);
//
//	//	HAL_Delay ( 5 );
//
//	Command = READ_BUF;
//	CELL_COMMAND(Data_Ptr);
//}

/******************************************************
 Cell Temperature_Data.Humidity Function
 Sends humidity percentage to MQTT server
 ******************************************************/
void CELL_PICUPDT(MEM_PTR *Data_Ptr)
{
#ifdef Log_Level_2
	Log_Single( LOG_MSG_CELL_PICUPDT);
#endif // Log_Level_2
	//	KCS these were in Rev 1 working code
	//	Command = CHECK;
	//	CELL_COMMAND(Data_Ptr);
	//	if (Cell_State != CELL_OK)
	//		return;

	// kcs desired to remove the Next 2 lines
	//Command = HTTP_URL_UPDT_POST;
	//CELL_COMMAND(Data_Ptr);

	// kcs/GAV remove this check was for the HTTP
	Command = CHECK;
	for (Temporary = 0; Cell_State != CELL_OK; Temporary++)
	{
		CELL_COMMAND(Data_Ptr);
		if (Temporary >= TEMP_LIMIT)
			break;
	}

	RTC_TimeTypeDef sTime;
	RTC_DateTypeDef unlockRtc = {0};

	HAL_RTC_GetTime(&hrtc, &sTime, FORMAT_BIN);
	HAL_RTC_GetDate (&hrtc, &unlockRtc, FORMAT_BIN);

	Memory[0] = sTime.SecondFraction;
	Memory[1] = sTime.Seconds;
	Memory[2] = sTime.Minutes;
	Memory[3] = sTime.Hours;

	buf = 0;
	Size = 0;
	Page = 0;
	Encoded_Picture_Buffer = 0;
	Version = PIC_Header_Version;
	Service = PIC_Header_Service_Send;
	Unique_ID = UNIQUE_Device_ID;

	// Clear pageIndexArray here
	clearPicturePageIndex();
	Time_Stamp(Picture_ID);

	//	Picture_ID = HAL_CRC_Calculate ( &hcrc , (uint32_t*) Memory , 1 );

	Total_Page = ((Data_Ptr->Camera_Data.encodedSize - (Data_Ptr->Camera_Data.encodedSize % PIC_DATA_LIM)) / PIC_DATA_LIM)
							+ 1;

	//GAV see where this is being used
	Checksum = HAL_CRC_Calculate(&hcrc, (uint32_t*) Data_Ptr->Camera_Data.encoded,
			(uint32_t) (Data_Ptr->Camera_Data.encodedSize));
	//picture transmit loop beginning
	while (Page < Total_Page)
	{
		lastPacket = false;
		Refresh_Watchdog;

		if (buf == 0)
		{
			Command = CHECK;
			CELL_COMMAND(Data_Ptr);

			Clear_Memory(Data_Ptr);

#ifdef NO_FORMAT_PIC
			Build_MSG ( Data_Ptr , HTTP_MSG_OBJ_HEAD );
#endif // NO_FORMAT_PIC

#ifndef NO_FORMAT_PIC

			if (++Page == Total_Page)
				Data_Length = Data_Ptr->Camera_Data.encodedSize - ((Page - 1) * PIC_DATA_LIM);
			else
				Data_Length = PIC_DATA_LIM;

			//GAV see where this is being used
			Checksum = HAL_CRC_Calculate(&hcrc,
					(uint32_t*) &Data_Ptr->Camera_Data.encoded[ PIC_DATA_LIM * Page],
					(uint32_t) (Data_Length / 4));

			buf = 0;

			//			Header [ 0 ] |= Version << 4;
			//			Header [ 0 ] |= Service;
			//			Header [ 1 ] |= Unique_ID >> 24;
			//			Header [ 2 ] |= Unique_ID >> 16;
			//			Header [ 3 ] |= Unique_ID >> 8;
			//			Header [ 4 ] |= Unique_ID;
			//			Header [ 5 ] |= Picture_ID;
			//			Header [ 6 ] |= Page;
			//			Header [ 7 ] |= Total_Page;
			//			Header [ 8 ] |= Checksum >> 24;
			//			Header [ 9 ] |= Checksum >> 16;
			//			Header [ 10 ] |= Checksum >> 8;
			//			Header [ 11 ] |= Checksum;
			//			Header [ 12 ] |= Data_Length >> 8;
			//			Header [ 13 ] |= Data_Length;

			Build_MSG(Data_Ptr, MQTT_HEAD_MASK_START);
			utoa( UNIQUE_Device_ID, Header, 10);
			Build_MSG(Data_Ptr, Header);
			Build_MSG(Data_Ptr, MQTT_HEAD_MASK_END);
			Word_Transfer( Event_Type, MQTT_PIC_EVENT);
			Build_MSG(Data_Ptr, Event_Type);

			Word_Transfer( Event_Type, MQTT_PIC_picID_EVENT);
			Build_MSG(Data_Ptr, Event_Type);
			Build_MSG(Data_Ptr, Picture_ID);
			Build_MSG(Data_Ptr, MQTT_STRI);
			Build_MSG(Data_Ptr, MQTT_APND);

			Word_Transfer( Event_Type, MQTT_PIC_Page_EVENT);
			utoa( Page, Header, 10);
			Build_MSG(Data_Ptr, Event_Type);
			Build_MSG(Data_Ptr, Header);
			Build_MSG(Data_Ptr, MQTT_STRI);
			Build_MSG(Data_Ptr, MQTT_APND);

			Word_Transfer( Event_Type, MQTT_PIC_Total_Page_EVENT);
			utoa( Total_Page, Header, 10);
			Build_MSG(Data_Ptr, Event_Type);
			Build_MSG(Data_Ptr, Header);
			Build_MSG(Data_Ptr, MQTT_STRI);
			Build_MSG(Data_Ptr, MQTT_APND);

			Word_Transfer( Event_Type, MQTT_PIC_DATA_EVENT);
			Build_MSG(Data_Ptr, Event_Type);

#endif // not NO_FORMAT_PIC
		}

		// store page indexes
		pageIndexArray[Page].startIndex = Encoded_Picture_Buffer;
		if (Page != Total_Page)
		{
			Memory[ buf++] = Data_Ptr->Camera_Data.encoded[ Encoded_Picture_Buffer++];
			while (Encoded_Picture_Buffer % Data_Length)
				Memory[ buf++] = Data_Ptr->Camera_Data.encoded[ Encoded_Picture_Buffer++];
		}
		else
		{
			while (Encoded_Picture_Buffer != Data_Ptr->Camera_Data.encodedSize)
				Memory[ buf++] = Data_Ptr->Camera_Data.encoded[ Encoded_Picture_Buffer++];
			lastPacket = true;
		}

		// store page indexes
		pageIndexArray[Page].stopIndex = Encoded_Picture_Buffer-1;

#ifdef NO_FORMAT_PIC
		Build_MSG ( Data_Ptr , HTTP_MSG_OBJ_TAIL );
		Data_Ptr->Modem_Data.Package_Page = buf;
#endif // NO_FORMAT_PIC

		Build_MSG(Data_Ptr, MQTT_TAIL_MASK);
		Size = buf;

		HAL_Delay (200); // to add time between sending packets
		if (Page != 30) //for testing
		{
			Command = PUBLISH;
			CELL_COMMAND(Data_Ptr);

			HAL_UART_AbortReceive(&hlpuart1);
			// Ideally these 2 lines would be after the printout, but we need for timing with receive mode of "0"
			Timer = UART_WAIT_TIME;
			HAL_UART_Transmit(&hlpuart1, Memory, Size, Timer);

		}
		else
		{
			HAL_UART_AbortReceive(&hlpuart1);
			HAL_Delay (100); // simulate Xmit time
		}

#ifdef SKYWIRE_MODEM_OUT
		Write_Log( LOG_MSG_DATA);

		for (buf = 0; buf <= Size; buf += 1)
		{
			vcom_Trace(&Memory[ buf], 1, 1);
		}
		vcom_Trace((uint8_t*) "\r\n\r\n", 4, 1);

		if(0) //We don't know what this is, was breaking pic function
		{
			HAL_UART_AbortReceive(&hlpuart1);
			Timer = UART_WAIT_TIME;
			HAL_UART_Transmit(&hlpuart1, Memory, Size, Timer);
		}
		//These lines are to output the buffer size for each pic packet
		char Command_Buffer [ 11 ]; // uint_32 in ascii is 10 plus the null symbol so 11
		utoa ( Size , Command_Buffer , DECIMAL );
		vcom_Trace((uint8_t*) LOG_PKT_DATA, 16, 1);
		vcom_Trace((uint8_t*) Command_Buffer, 4, 1); // Hard coded to 4 buffer size max is MEMORY_MAX plus some overhead
		vcom_Trace((uint8_t*) "\r\n\r\n", 4, 1);

		if (lastPacket & interruptTest)
		{
			HAL_UART_Receive_IT ( &hlpuart1 , Memory , 1 );
		}
#endif //SKYWIRE_MODEM_OUT

		buf = 0;
	} //end of picture transmit loop

	if(interruptTest)
	{
		Clear_Memory2(Data_Ptr, 400);
		//	HAL_UART_Receive_IT ( &hlpuart1 , Memory , 1 );
	}
	else if (useRecvMode1)
	{
		DEV_State |= GET_CELL_RECV_BUFF;  //GAV
	}
	else
	{
		// kcs clear Memory small
		// {"type":"image_ack_v0","dev_id":"4522052","img_id":"08_30_2023__00_53_22-3110","total_pages":25,"pages_missing":[],"pages_received":[1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25]}
		bool immediate = true;
		bool listenForServer = true;

		// kcs remove these and replace with infinite loop
		if (listenForServer)
		{
			if (immediate)
			{
				// kcs this needs to be a loop
				// start of retry sending missing packets
				vcom_Trace((uint8_t*) "Start Rec\r\n", 11, 1);
				Size = 330;
				Timer = 100;
				receivedDataSize = 0;
				Command = CHECK2;
				CELL_COMMAND(Data_Ptr);

				vcom_Trace((uint8_t*) "DownLink Received\r\n", 19, 1);

				for (buf = 0; buf <= receivedDataSize; buf += 1)
				{
					vcom_Trace(&Memory[buf], 1, 1);
				}
				vcom_Trace((uint8_t*) "\r\n", 2, 1);

				//				if(!decodeImageAck(Data_Ptr))
				//				{
				//					if (missingPagesCount > 0)
				//						resendMissingPackets(Data_Ptr);
				//				}

				// End of missing packets loop

#ifdef Log_Level_2
				Write_Log( LOG_RX_ERR);
				Log_End( LOG_STATE_RESULT, Cell_State);
#endif // Log_Level_2
			}
			else
			{
				Clear_Memory(Data_Ptr);
				HAL_Delay(2000);
				Refresh_Watchdog;
				Command = READ_BUF2;
				CELL_COMMAND(Data_Ptr);
			}
		}
		else
		{
			if (0) // old erik stuff and http things
			{
				HAL_Delay(100);
				Command = READ_BUF;
				CELL_COMMAND(Data_Ptr);


				Command = HTTP_URL_UPDT_REQ;
				CELL_COMMAND(Data_Ptr);

				Command = CHECK;
				for (Temporary = 0; Cell_State != CELL_OK; Temporary++)
				{
					CELL_COMMAND(Data_Ptr);
					if (Temporary >= TEMP_LIMIT)
						break;
				}
			}
		}
	}
}

/******************************************************
 Cell PIR Function
 Sends start or stop based on flag to server
 ******************************************************/
void CELL_PIRUPDT(MEM_PTR *Data_Ptr, bool start)
{
#ifdef Log_Level_2
	Log_Single( LOG_MSG_CELL_PIRUPDT);
#endif // Log_Level_2

	Command = CHECK;
	CELL_COMMAND(Data_Ptr);
	if (Cell_State != CELL_OK)
		return;

	//Gage to fix this for motion stops
	char timeStamp[MEMORY_MAX] = {0};
	Time_StampISO(timeStamp);

	if (start) //(DEV_State & PIR_START)
	{
		Word_Transfer( Event_Type, MQTT_PIR_START);
//		DEV_State ^= PIR_START;
	}
	else if (!start)
	{
		Word_Transfer( Event_Type, MQTT_PIR_STOP);
	}

	Clear_Memory(Data_Ptr);
	buf = 0;

	Build_MSG(Data_Ptr, MQTT_HEAD_MASK_START);
	utoa( UNIQUE_Device_ID, Header, 10);
	Build_MSG(Data_Ptr, Header);
	Build_MSG(Data_Ptr, MQTT_HEAD_MASK_END);
	Build_MSG(Data_Ptr, Event_Type);
	Build_MSG(Data_Ptr, MQTT_QUOTE);
	Build_MSG(Data_Ptr, MQTT_APND);
	Build_MSG(Data_Ptr, TIME_STR);
	Build_MSG(Data_Ptr, timeStamp);
	Build_MSG(Data_Ptr, MQTT_TAIL_MASK);
	Size = buf;
	Command = PUBLISH;
	CELL_COMMAND(Data_Ptr);
	HAL_Delay( MSG_Delay);
	Command = PUBLISH_MEM;
	CELL_COMMAND(Data_Ptr);

	Command = CHECK;
	CELL_COMMAND(Data_Ptr);

	HAL_Delay(5);

	Command = READ_BUF;
	CELL_COMMAND(Data_Ptr);
}

/******************************************************
 Cell Movement start Function
 Sends start or stop from accelerometer trigger
 ******************************************************/
void cellMovementStart (MEM_PTR *Data_Ptr, bool startStop)
{
#ifdef Log_Level_2
	Log_Single( LOG_MSG_cell_Movement_Start);
#endif // Log_Level_2

	Command = CHECK;
	CELL_COMMAND(Data_Ptr);
	if (Cell_State != CELL_OK)
		return;

	//Gage to fix this for motion stops
	char timeStamp[MEMORY_MAX] = {0};
	Time_StampISO(timeStamp);

	if (!startStop) //(DEV_State & PIR_START)
	{
		Word_Transfer( Event_Type, MQTT_MOVEMENT_START);
	}
	else if (startStop)
	{
		Word_Transfer( Event_Type, MQTT_MOVEMENT_END);
	}

	Clear_Memory(Data_Ptr);
	buf = 0;

	Build_MSG(Data_Ptr, MQTT_HEAD_MASK_START);
	utoa( UNIQUE_Device_ID, Header, 10);
	Build_MSG(Data_Ptr, Header);
	Build_MSG(Data_Ptr, MQTT_HEAD_MASK_END);
	Build_MSG(Data_Ptr, Event_Type);
	Build_MSG(Data_Ptr, MQTT_QUOTE);
	Build_MSG(Data_Ptr, MQTT_APND);
	Build_MSG(Data_Ptr, TIME_STR);
	Build_MSG(Data_Ptr, timeStamp);
	Build_MSG(Data_Ptr, MQTT_TAIL_MASK);
	Size = buf;
	Command = PUBLISH;
	CELL_COMMAND(Data_Ptr);
	HAL_Delay( MSG_Delay);
	Command = PUBLISH_MEM;
	CELL_COMMAND(Data_Ptr);

	Command = CHECK;
	CELL_COMMAND(Data_Ptr);

	HAL_Delay(5);

	Command = READ_BUF;
	CELL_COMMAND(Data_Ptr);
}

/******************************************************
 Cell Log Overload Function
 Allows the user to still call Log without pulling
 Log save to current location
 ******************************************************/
void CELL_TIMEUPDT(MEM_PTR *Data_Ptr)
{
	Command = CHECK;
	CELL_COMMAND(Data_Ptr);

	if (Cell_State != CELL_OK)
		return;

	RTC_TimeTypeDef sTime;
	RTC_DateTypeDef unlockRtc = {0};

	HAL_RTC_GetTime(&hrtc, &sTime, FORMAT_BIN);
	HAL_RTC_GetDate (&hrtc, &unlockRtc, FORMAT_BIN);

	sTime.SecondFraction = 0;
	sTime.Seconds = 0;
	sTime.Minutes = 0;
	sTime.Hours = 0;

	HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
}

#ifdef Log
/******************************************************
 Cell Log Function
 Allows the user to still call Log without pulling
 Log save to current location
 ******************************************************/
void CELL_LOGUPDT(MEM_PTR *Data_Ptr)
{
	Command = CHECK;
	CELL_COMMAND(Data_Ptr);
	if (Cell_State != CELL_OK)
		return;

	if (Setting & LOG_LEVEL_2)
	{
		for (int i = 0; i < Logger.Buffer; ++i)
		{
			CELL_LOGUPDT_EXTEND(Data_Ptr, &Logger, i);

			Command = CHECK;
			for (Temporary = 0; Cell_State != CELL_OK; Temporary++)
			{
				CELL_COMMAND(Data_Ptr);
				if (Temporary >= TEMP_LIMIT)
					break;
			}
		}
	}

	CELL_LOGUPDT_EXTEND(Data_Ptr, &Logger, Logger.Buffer);
}

//void CELL_GET_RECV_BUFF (MEM_PTR *Data_Ptr)
//{
//	int loopCounter = 0;
//	bool goodData = false;
//	Clear_Memory(Data_Ptr);
//	HAL_Delay(5000);  //added by GAV for server timer
//
//	do
//	{
//		Refresh_Watchdog;
//		HAL_Delay(5);  //removed by Gage
//		Command = CHECK;
//		CELL_COMMAND(Data_Ptr);
//		Command = READ_BUF;
//		CELL_COMMAND(Data_Ptr);
//		loopCounter ++;
//		if( loopCounter > 3 )  // 10 for LL
//		{
////			CELL_Init(Data_Ptr);  // for stop3 only, leave for Low Latency
//			break;
//		}
//		goodData = isBufferPopulated(Data_Ptr);
//
//
//	} while ( !goodData );
//
//	receivedDataSize = 250;
//	if ( goodData )
//	{
//		vcom_Trace((uint8_t*) "DownLink Received\r\n", 19, 1);
//
//		for ( buf = 0; buf <= receivedDataSize; buf += 1 )
//		{
//			vcom_Trace(&Memory[buf], 1, 1);
//		}
//		vcom_Trace((uint8_t*) "\r\n", 2, 1);
//
//		if( !decodeImageAck(Data_Ptr) )
//		{
//			if ( missingPagesCount > 0 )
//				resendMissingPackets(Data_Ptr);
//		}
//	}
//}


/******************************************************
 Cell Log Function
 Sends x Log to the  MQTT server
 ******************************************************/
void CELL_LOGUPDT_EXTEND(MEM_PTR *Data_Ptr, Logger_Lists *Logger, uint8_t Buffer)
{
	Word_Transfer( Event_Type, MQTT_LOG_EVENT );

	Clear_Memory(Data_Ptr);
	buf = 0;

	Build_MSG(Data_Ptr, MQTT_HEAD_MASK_START);
	utoa( UNIQUE_Device_ID, Header, 10);
	Build_MSG(Data_Ptr, Header);
	Build_MSG(Data_Ptr, MQTT_HEAD_MASK_END);
	Build_MSG(Data_Ptr, Event_Type);
	Build_MSG(Data_Ptr, Logger->List[Buffer].MSG);
	Build_MSG(Data_Ptr, MQTT_TAIL_MASK);
	Size = buf;

	Char_Replace((char*) Memory, "\n", " ");
	Char_Replace((char*) Memory, "\r", " ");

	Command = PUBLISH;
	CELL_COMMAND(Data_Ptr);
	HAL_Delay( MSG_Delay);
	Command = PUBLISH_MEM;
	CELL_COMMAND(Data_Ptr);

	Command = CHECK;
	CELL_COMMAND(Data_Ptr);

	HAL_Delay(5);

	Command = READ_BUF;
	CELL_COMMAND(Data_Ptr);
}
#endif // Log

/******************************************************
 Cell Heartbeat Function
 Sends basic information to MQTT server
 ******************************************************/
void HeartBeat(MEM_PTR *Data_Ptr)
{
#ifdef Log_Level_2
	Log_Single( LOG_MSG_HeartBeat);
#endif // Log_Level_2

	char timeStamp[MEMORY_MAX] = {0};
	Command = CHECK;
	CELL_COMMAND(Data_Ptr);

	Command = SIGNAL;
	CELL_COMMAND(Data_Ptr);

	Char_Replace((char*) Memory, "\n", " ");
	Char_Replace((char*) Memory, "\r", " ");

	Word_Transfer((char*) Signal, (char*) Memory);

	//	Time_Stamp((char*) Data_Ptr->Modem_Data.Time_STMP);
	Word_Transfer( Event_Type, MQTT_HB_EVENT);

	Time_StampISO(timeStamp);

	Clear_Memory(Data_Ptr);
	buf = 0;

	/************************************************************************/
	//	Command = SIM_CHECK;
	//	CELL_COMMAND(Data_Ptr);
	//
	//	Char_Replace((char*) Memory, "\n", " ");
	//	Char_Replace((char*) Memory, "\r", " ");
	//
	//	Word_Transfer((char*) SIM_PIN, (char*) Memory);
	//
	//	Clear_Memory(Data_Ptr);
	//	buf = 0;
	//

	Command = CELLID_SET;
	CELL_COMMAND(Data_Ptr);

	Command = CELLID;
	CELL_COMMAND(Data_Ptr);

	Char_Replace((char*) Memory, "\n", " ");
	Char_Replace((char*) Memory, "\r", " ");

	Word_Transfer((char*) Cell_ID, (char*) Memory);

	Clear_Memory(Data_Ptr);
	buf = 0;

	/***********************************************************************/
	//Cell_Date_Day = {6,9};
	//	char CDay[2] = {0};
	//MEM_PTR *Data_Ptr;
	Update_Battery ();
	//	for (int instance =1; instance < 11; instance++) //to loop heartbeats for testing purposes
	//	{

	TEMP_DEVICE_TYPE localTemp;
	getTempDeviceType(&localTemp);
	getTempSensorData(&tempParameters);


	Build_MSG(Data_Ptr, MQTT_HEAD_MASK_START);
	utoa( UNIQUE_Device_ID, Header, 10);
	Build_MSG(Data_Ptr, Header);

	Build_MSG(Data_Ptr, MQTT_HEAD_MASK_END);
	Build_MSG(Data_Ptr, Event_Type);
	Build_MSG(Data_Ptr, MQTT_APND);
	Build_MSG(Data_Ptr, TIME_STR);
	Build_MSG(Data_Ptr, timeStamp);

	Build_MSG(Data_Ptr, MQTT_DATA);
	if(localTemp != TEMP_UNKNOWN_DEVICE)
	{
		utoa( tempParameters.Device_ID, (char*) (&(Memory[ buf])), 10);
	}

	while (Memory[ buf])
		buf++;

	//Build_MSG(Data_Ptr, MQTT_VBAT);
	//Build_MSG(Data_Ptr, MQTT_HB_DATA_HDR);   //temporary while "Bat" is hardcoded as a string

	Build_MSG(Data_Ptr, MQTT_APND);
	if(localTemp != TEMP_UNKNOWN_DEVICE)
	{
		utoa( tempParameters.Temperature, (char*) (&(Memory[ buf])), 10);
	}

	while (Memory[ buf])
		buf++;

	Build_MSG(Data_Ptr, MQTT_APND);
	if(localTemp != TEMP_UNKNOWN_DEVICE)
	{
		utoa( tempParameters.Device_ID, (char*) (&(Memory[ buf])), 10);
	}

	while (Memory[ buf])
		buf++;

	Build_MSG(Data_Ptr, MQTT_APND);
	if(localTemp != TEMP_UNKNOWN_DEVICE)
	{
		utoa( tempParameters.Humidity, (char*) (&(Memory[ buf])), 10);
	}

	while (Memory[ buf])
		buf++;

	Build_MSG(Data_Ptr, MQTT_APND);
//	utoa( Acce_Device_ID, (char*) (&(Memory[ buf])), 10);

	while (Memory[ buf])
		buf++;

	Build_MSG(Data_Ptr, MQTT_APND);
//	utoa( X_Axis, (char*) (&(Memory[ buf])), 10);

	while (Memory[ buf])
		buf++;

	Build_MSG(Data_Ptr, MQTT_APND);
//	utoa( Y_Axis, (char*) (&(Memory[ buf])), 10);

	while (Memory[ buf])
		buf++;

	Build_MSG(Data_Ptr, MQTT_APND);
//	utoa( Z_Axis, (char*) (&(Memory[ buf])), 10);

	while (Memory[ buf])
		buf++;

	Build_MSG(Data_Ptr, MQTT_APND);
	Build_MSG(Data_Ptr, (char*) Signal);
	Build_MSG(Data_Ptr, MQTT_APND);

	//	Build_MSG(Data_Ptr, (char*) Cell_ID);
	//	Build_MSG(Data_Ptr, MQTT_APND);

	Build_MSG(Data_Ptr, (char*) Modem_ID);
	Build_MSG(Data_Ptr, MQTT_APND);
	Build_MSG(Data_Ptr, (char*) Sim_ID);

	Build_MSG(Data_Ptr, MQTT_APND);
	Build_MSG(Data_Ptr, batVal);
	Build_MSG(Data_Ptr, MQTT_APND);

	Build_MSG(Data_Ptr, (char*) Cell_Dev_ID);
//	Build_MSG(Data_Ptr, MQTT_APND);
//	Time_Stamp((char*) Data_Ptr->Modem_Data.Time_STMP);
//	Build_MSG(Data_Ptr, (char*) Data_Ptr->Modem_Data.Time_STMP);
//	Build_MSG(Data_Ptr, MQTT_MSG_END_BRACKET);
	Build_MSG(Data_Ptr, MQTT_TAIL_MASK);

	//	utoa( Cell_Date_Day, (char*)CDay, 10);
	//	Build_MSG(Data_Ptr, CDay);

	Size = buf;

	Command = PUBLISH;
	CELL_COMMAND(Data_Ptr);
	//	HAL_Delay ( MSG_Delay );
	Command = PUBLISH_MEM;
	CELL_COMMAND(Data_Ptr);
	HAL_Delay(5000);
	Refresh_Watchdog;
	//	}

	Command = CHECK;
	CELL_COMMAND(Data_Ptr);
}

void FW_Ack_Pending(MEM_PTR *Data_Ptr, OTA_FILE_TYPE *OtaData)
{
#ifdef Log_Level_2
	Log_Single( LOG_MSG_FW_ACK_PENDING);
#endif // Log_Level_2

	char timeStamp[MEMORY_MAX] = {0};
	Time_StampISO(timeStamp);

	Command = CHECK;
	CELL_COMMAND(Data_Ptr);

	Word_Transfer( Event_Type, MQTT_FW_PENDING_EVENT);

	Clear_Memory(Data_Ptr);
	buf = 0;

	Build_MSG(Data_Ptr, MQTT_HEAD_MASK_START);
	utoa( UNIQUE_Device_ID, Header, 10);
	Build_MSG(Data_Ptr, Header);

	Build_MSG(Data_Ptr, MQTT_HEAD_MASK_END);
	Build_MSG(Data_Ptr, Event_Type);
	Build_MSG(Data_Ptr, TIME_STR);
	Build_MSG(Data_Ptr, timeStamp);
	Build_MSG(Data_Ptr, MQTT_QUOTE);
	Build_MSG(Data_Ptr, MQTT_APND);


	Build_MSG(Data_Ptr, MQTT_FW_VERSION);
	Build_MSG(Data_Ptr, MQTT_STRI);
	Build_MSG(Data_Ptr, OtaData->otaFileName);
	Build_MSG(Data_Ptr, MQTT_STRI);
	Build_MSG(Data_Ptr, MQTT_END_BRACKET);

	Size = buf;

	Command = PUBLISH;
	CELL_COMMAND(Data_Ptr);
	Command = PUBLISH_MEM;
	CELL_COMMAND(Data_Ptr);
	Command = CHECK;
	CELL_COMMAND(Data_Ptr);
}

void FW_Ack_Ready(MEM_PTR *Data_Ptr, OTA_FILE_TYPE *OtaData)
{
#ifdef Log_Level_2
	Log_Single( LOG_MSG_FW_ACK_READY);
#endif // Log_Level_2

	char timeStamp[MEMORY_MAX] = {0};
	Time_StampISO(timeStamp);

	Command = CHECK;
	CELL_COMMAND(Data_Ptr);

	Word_Transfer( Event_Type, MQTT_FW_READY_EVENT);

	Clear_Memory(Data_Ptr);
	buf = 0;

	Build_MSG(Data_Ptr, MQTT_HEAD_MASK_START);
	utoa( UNIQUE_Device_ID, Header, 10);
	Build_MSG(Data_Ptr, Header);

	Build_MSG(Data_Ptr, MQTT_HEAD_MASK_END);
	Build_MSG(Data_Ptr, Event_Type);

	Build_MSG(Data_Ptr, TIME_STR);
	Build_MSG(Data_Ptr, timeStamp);
	Build_MSG(Data_Ptr, MQTT_QUOTE);
	Build_MSG(Data_Ptr, MQTT_APND);

	Build_MSG(Data_Ptr, MQTT_FW_VERSION);
	Build_MSG(Data_Ptr, MQTT_STRI);
	Build_MSG(Data_Ptr, OtaData->otaFileName);
	Build_MSG(Data_Ptr, MQTT_STRI);
	Build_MSG(Data_Ptr, MQTT_END_BRACKET);

	Size = buf;

	Command = PUBLISH;
	CELL_COMMAND(Data_Ptr);
	Command = PUBLISH_MEM;
	CELL_COMMAND(Data_Ptr);
	Command = CHECK;
	CELL_COMMAND(Data_Ptr);
}

void FW_Acknowledge(MEM_PTR *Data_Ptr, OTA_FILE_TYPE *OtaData)
{
#ifdef Log_Level_2
	Log_Single( LOG_MSG_FW_MISSING_PAGES);
#endif // Log_Level_2

	char timeStamp[MEMORY_MAX] = {0};
	Time_StampISO(timeStamp);

	char totalpagesString[15];
	uint8_t totPage = OtaData->numberOfPages;

	utoa( UNIQUE_Device_ID, Header, 10);
	utoa(totPage, totalpagesString, 10);

	Command = CHECK;
	CELL_COMMAND(Data_Ptr);

	Word_Transfer( Event_Type, MQTT_FW_ACK_EVENT);
	Clear_Memory(Data_Ptr);
	buf = 0;

	Build_MSG(Data_Ptr, MQTT_HEAD_MASK_START);
	utoa( UNIQUE_Device_ID, Header, 10);
	Build_MSG(Data_Ptr, Header);
	Build_MSG(Data_Ptr, MQTT_HEAD_MASK_END);
	Build_MSG(Data_Ptr, Event_Type);

	Build_MSG(Data_Ptr, TIME_STR);
	Build_MSG(Data_Ptr, timeStamp);
	Build_MSG(Data_Ptr, MQTT_QUOTE);
	Build_MSG(Data_Ptr, MQTT_APND);

	Build_MSG(Data_Ptr, MQTT_FW_VERSION);
	Build_MSG(Data_Ptr, MQTT_STRI);
	Build_MSG(Data_Ptr, OtaData->otaFileName);
	Build_MSG(Data_Ptr, MQTT_STRI);
	Build_MSG(Data_Ptr, MQTT_APND);
	Build_MSG(Data_Ptr, MQTT_FW_FILE);
	Build_MSG(Data_Ptr, OtaData->fileNumberString);
	Build_MSG(Data_Ptr, MQTT_APND);
	Build_MSG(Data_Ptr, MQTT_FW_TOTAL_PAGES);
	Build_MSG(Data_Ptr, (char*)totalpagesString);
	Build_MSG(Data_Ptr, MQTT_APND);
	Build_MSG(Data_Ptr, MQTT_FW_PAGES_RECEIVED);
	Build_MSG(Data_Ptr, OtaData->receivedPagesString);
	Build_MSG(Data_Ptr, MQTT_APND);
	Build_MSG(Data_Ptr, MQTT_FW_PAGES_MISSING);
	Build_MSG(Data_Ptr, OtaData->missingPagesString);
	Build_MSG(Data_Ptr, MQTT_END_BRACKET);

	Size = buf;

	Command = PUBLISH;
	CELL_COMMAND(Data_Ptr);
	Command = PUBLISH_MEM;
	CELL_COMMAND(Data_Ptr);
	Command = CHECK;
	CELL_COMMAND(Data_Ptr);
}

void FW_Next(MEM_PTR *Data_Ptr, OTA_FILE_TYPE *OtaData)
{
#ifdef Log_Level_2
	Log_Single( LOG_MSG_FW_NEXT_BATCH);
#endif // Log_Level_2

	char timeStamp[MEMORY_MAX] = {0};
	Time_StampISO(timeStamp);

	char batchNumString[15];
	uint8_t tempBatchNum = OtaData->batchNum;

	utoa( UNIQUE_Device_ID, Header, 10);
	utoa(tempBatchNum, batchNumString, 10);

	Word_Transfer( Event_Type, MQTT_FW_NEXT_EVENT);
	Clear_Memory(Data_Ptr);
	buf = 0;

	Build_MSG(Data_Ptr, MQTT_HEAD_MASK_START);
	utoa( UNIQUE_Device_ID, Header, 10);
	Build_MSG(Data_Ptr, Header);
	Build_MSG(Data_Ptr, MQTT_HEAD_MASK_END);
	Build_MSG(Data_Ptr, Event_Type);

	Build_MSG(Data_Ptr, TIME_STR);
	Build_MSG(Data_Ptr, timeStamp);
	Build_MSG(Data_Ptr, MQTT_QUOTE);
	Build_MSG(Data_Ptr, MQTT_APND);

	Build_MSG(Data_Ptr, MQTT_FW_VERSION);
	Build_MSG(Data_Ptr, MQTT_STRI);
	Build_MSG(Data_Ptr, OtaData->otaFileName);
	Build_MSG(Data_Ptr, MQTT_STRI);
	Build_MSG(Data_Ptr, MQTT_APND);
	Build_MSG(Data_Ptr, MQTT_FW_FILE);
	Build_MSG(Data_Ptr, OtaData->fileNumberString);
	Build_MSG(Data_Ptr, MQTT_APND);
	Build_MSG(Data_Ptr, MQTT_FW_BATCH);
	//	Char_Replace( batchNumString, "\n", " ");
	//	Char_Replace( batchNumString, "\r", " ");
	Build_MSG(Data_Ptr, (char*)batchNumString);
	Build_MSG(Data_Ptr, MQTT_END_BRACKET);

	Size = buf;
	Refresh_Watchdog;

	Command = PUBLISH;
	CELL_COMMAND(Data_Ptr);
	Command = PUBLISH_MEM;
	CELL_COMMAND(Data_Ptr);
	Command = CHECK;
	CELL_COMMAND(Data_Ptr);
}

void FW_CRC_Ack(MEM_PTR *Data_Ptr, bool isGood, OTA_FILE_TYPE *OtaData)
{
#ifdef Log_Level_2
	Log_Single( LOG_MSG_FW_CRC_ACK);
#endif // Log_Level_2

	char timeStamp[MEMORY_MAX] = {0};
	Time_StampISO(timeStamp);

	Command = CHECK;
	CELL_COMMAND(Data_Ptr);

	Word_Transfer( Event_Type, MQTT_FW_CRC_EVENT);

	Clear_Memory(Data_Ptr);
	buf = 0;

	Build_MSG(Data_Ptr, MQTT_HEAD_MASK_START);
	utoa( UNIQUE_Device_ID, Header, 10);
	Build_MSG(Data_Ptr, Header);
	Build_MSG(Data_Ptr, MQTT_HEAD_MASK_END);
	Build_MSG(Data_Ptr, Event_Type);

	Build_MSG(Data_Ptr, TIME_STR);
	Build_MSG(Data_Ptr, timeStamp);
	Build_MSG(Data_Ptr, MQTT_QUOTE);
	Build_MSG(Data_Ptr, MQTT_APND);

	Build_MSG(Data_Ptr, MQTT_FW_VERSION);
	Build_MSG(Data_Ptr, MQTT_STRI);
	Build_MSG(Data_Ptr, OtaData->otaFileName);
	Build_MSG(Data_Ptr, MQTT_STRI);
	Build_MSG(Data_Ptr, MQTT_APND);
	Build_MSG(Data_Ptr, MQTT_FW_FILE);
	Build_MSG(Data_Ptr, OtaData->fileNumberString);
	Build_MSG(Data_Ptr, MQTT_APND);
	Build_MSG(Data_Ptr, MQTT_FW_CRC_STATUS);
	if (isGood == true)
	{
		Build_MSG(Data_Ptr, FW_CRC_OK);
	}
	else if (isGood == false)
	{
		Build_MSG(Data_Ptr, FW_MISMATCH);
	}
	Build_MSG(Data_Ptr, MQTT_END_BRACKET);

	Size = buf;
	Refresh_Watchdog;

	Command = PUBLISH;
	CELL_COMMAND(Data_Ptr);
	Command = PUBLISH_MEM;
	CELL_COMMAND(Data_Ptr);
	Command = CHECK;
	CELL_COMMAND(Data_Ptr);
}


/******************************************************
 Cell Modem_Data.Command Function
 Controller for MQTT communication
 ******************************************************/
void CELL_COMMAND(MEM_PTR *Data_Ptr)
{
	int testSize = 300;
	char msgbuffer[200] = {""};
	if (HAL_IWDG_Refresh(&hiwdg) != HAL_OK)
		Error_Handler();

#ifdef Log_Level_1
	if (Setting & LOG_LEVEL_1)
	{
		Log_End( LOG_SKYWIRE, Command);
	}
	else if (Setting & LOG_LEVEL_1 && Setting & LOG_LEVEL_2)
	{
		Log_Insert( LOG_SKYWIRE, Command);
	}
#endif // Log_Level_1
	Refresh_Watchdog;
	switch ( Command )
	{
	case CHECK:
		Cell_State = CELL_OK;

		CELL_Build_MSG(Data_Ptr, MQTT_HEAD);
		CELL_Build_MSG(Data_Ptr, MQTT_TAIL);

		Timer = UART_WAIT_TIME;
		Size = 16;	// size of OK return should always be consistent
		CELL_MSG(Data_Ptr, false);
		checkCellOk(Data_Ptr, CELL_CHECK_ERR);

		//				Command = CONNECTION_TEST;
		//				CELL_COMMAND(Data_Ptr,CELL);

		return;

	case CHECK2:
		Cell_State = CELL_OK;
		Clear_Memory2(Data_Ptr, 400); // kcs put a real size in this call
		receivedDataSize = CELL_READ_URC(Data_Ptr);
		checkCellOk(Data_Ptr, CELL_CHECK_ERR);

		return;

	case CHECK3:
		Cell_State = CELL_OK;
		receivedDataSize = CELL_READ_URC(Data_Ptr);
		checkCellOk(Data_Ptr, CELL_CHECK_ERR);

		return;

	case CONNECTION_TEST:
		if (Cell_State != CELL_OK)
			return;

		CELL_Build_MSG(Data_Ptr, MQTT_HEAD);
		CELL_Build_MSG(Data_Ptr, CELL_DEVICE_QUERY);
		CELL_Build_MSG(Data_Ptr, MQTT_TAIL);

		Timer = UART_WAIT_TIME;
		Size = UART_MAX_SIZE;
		CELL_MSG(Data_Ptr, false);
		checkCellOk(Data_Ptr, CELL_CONNECTED_ERR);
		break;
	case DISABLEQI:
		CELL_Build_MSG(Data_Ptr, MQTT_HEAD);
		CELL_Build_MSG(Data_Ptr, CELL_DEACT);
		CELL_Build_MSG(Data_Ptr, MQTT_TAIL);

		Timer = UART_WAIT_TIME;
		Size = UART_MAX_SIZE;

		CELL_MSG(Data_Ptr, false);
		checkCellOk(Data_Ptr, CELL_PWR_ERR);
		return;
	case REENABLEQI:
		CELL_Build_MSG(Data_Ptr, MQTT_HEAD);
		CELL_Build_MSG(Data_Ptr, CELL_REACT);
		CELL_Build_MSG(Data_Ptr, MQTT_TAIL);

		Timer = 3000;
		Size = buf;
		RxSize = 16;
		CELL_MSG5(Data_Ptr);
		return;

	case SIGNAL:
		CELL_Build_MSG(Data_Ptr, MQTT_HEAD);
		CELL_Build_MSG(Data_Ptr, CELL_STATUS);
		CELL_Build_MSG(Data_Ptr, MQTT_TAIL);

		Timer = UART_WAIT_TIME;
		Size = UART_MAX_SIZE;
		CELL_MSG(Data_Ptr, false);
		checkCellOk(Data_Ptr, CELL_PWR_ERR);

		if (Fail_Cntr--)
		{
			HAL_Delay(3000);

			Refresh_Watchdog;
			if (Memory[15] == '9' && Memory[16] == '9')
				CELL_COMMAND(Data_Ptr);
		}
		Fail_Cntr = CSQ_MAX_FAIL;
		return;

	case CELLID_SET:
		CELL_Build_MSG(Data_Ptr, MQTT_HEAD);
		CELL_Build_MSG(Data_Ptr, CELLID_INIT);
		CELL_Build_MSG(Data_Ptr, MQTT_TAIL);

		Timer = UART_WAIT_TIME;
		Size = UART_MAX_SIZE;
		CELL_MSG(Data_Ptr, false);
		checkCellOk(Data_Ptr, CELL_PWR_ERR);
		return;


	case CELLID:
		CELL_Build_MSG(Data_Ptr, MQTT_HEAD);
		CELL_Build_MSG(Data_Ptr, CELL_ID);
		CELL_Build_MSG(Data_Ptr, MQTT_TAIL);

		Timer = UART_WAIT_TIME;
		//buf contains size of transmit buffer from cell_build_msg
		Size = buf;
		RxSize = 100;
		CELL_MSG5(Data_Ptr);    //use CELL_MSG5
		return;

	case SIM_CHECK:
		CELL_Build_MSG(Data_Ptr, MQTT_HEAD);
		CELL_Build_MSG(Data_Ptr, CPIN);
		CELL_Build_MSG(Data_Ptr, MQTT_TAIL);

		Timer = UART_WAIT_TIME;
		Size = UART_MAX_SIZE;
		CELL_MSG(Data_Ptr, false);
		checkCellOk(Data_Ptr, CELL_PWR_ERR);
		return;

	case SIM_ID:
		CELL_Build_MSG(Data_Ptr, MQTT_HEAD);
		CELL_Build_MSG(Data_Ptr, CELL_SIM_ID);
		CELL_Build_MSG(Data_Ptr, MQTT_TAIL);

		Timer = UART_WAIT_TIME;
		Size = UART_MAX_SIZE;
		CELL_MSG(Data_Ptr, false);
		checkCellOk(Data_Ptr, CELL_PWR_ERR);

		return;
	case MODEM_ID:
		CELL_Build_MSG(Data_Ptr, MQTT_HEAD);
		CELL_Build_MSG(Data_Ptr, CELL_DEVICE_ID);
		CELL_Build_MSG(Data_Ptr, MQTT_TAIL);

		Timer = UART_WAIT_TIME;
		Size = UART_MAX_SIZE;
		CELL_MSG(Data_Ptr, false);
		checkCellOk(Data_Ptr, CELL_PWR_ERR);

		return;

	case activatePDP:
		CELL_Build_MSG(Data_Ptr, MQTT_HEAD);
		CELL_Build_MSG(Data_Ptr, CELL_SIM_SERVICE_ACT);
		CELL_Build_MSG(Data_Ptr, MQTT_TAIL);

		Timer = 3000;
		Size = buf;
		RxSize = 18;
		CELL_MSG5(Data_Ptr);
		return;

	case OPEN:

		//
		//		CELL_Build_MSG(Data_Ptr, MQTT_HEAD);
		//		CELL_Build_MSG(Data_Ptr, CELL_SIM_SERVICE_ACT);
		//		CELL_Build_MSG(Data_Ptr, MQTT_TAIL);
		//
		//		Timer = UART_EXTEND_TIME + UART_EXTEND_TIME;
		//		Size = UART_MAX_SIZE;	//35;
		//		CELL_MSG(Data_Ptr, false);
		//		checkCellOk(Data_Ptr, CELL_CLOSE_ERR);



		CELL_Build_MSG(Data_Ptr, MQTT_HEAD);
		CELL_Build_MSG(Data_Ptr, MQTT_OPEN_MSG);
		CELL_Build_MSG(Data_Ptr, MQTT_SOCKET_ID);
		CELL_Build_MSG(Data_Ptr, MQTT_APND);
		CELL_Build_MSG(Data_Ptr, MQTT_STRI);
		CELL_Build_MSG(Data_Ptr, MQTT_SERVER);
		CELL_Build_MSG(Data_Ptr, MQTT_STRI);
		CELL_Build_MSG(Data_Ptr, MQTT_APND);
		CELL_Build_MSG(Data_Ptr, MQTT_PORT);
		CELL_Build_MSG(Data_Ptr, MQTT_TAIL);

		Timer = 3000;
		Size = buf;
		RxSize = 64;
		CELL_MSG5(Data_Ptr);
		//		checkCellOk(Data_Ptr, CELL_OPEN_ERR);
		return;
	case CLOSE:
		CELL_Build_MSG(Data_Ptr, MQTT_HEAD);
		CELL_Build_MSG(Data_Ptr, MQTT_CLOSE_MSG);
		CELL_Build_MSG(Data_Ptr, MQTT_SOCKET_ID);
		CELL_Build_MSG(Data_Ptr, MQTT_TAIL);

		Timer = UART_EXTEND_TIME + UART_EXTEND_TIME;
		Size = 26;	//35;
		CELL_MSG(Data_Ptr, false);
		checkCellOk(Data_Ptr, CELL_CLOSE_ERR);
		return;
	case CONNECT:
		CELL_Build_MSG(Data_Ptr, MQTT_HEAD);
		CELL_Build_MSG(Data_Ptr, MQTT_CONNECT_MSG);
		CELL_Build_MSG(Data_Ptr, MQTT_SOCKET_ID);
		CELL_Build_MSG(Data_Ptr, MQTT_APND);
		CELL_Build_MSG(Data_Ptr, MQTT_STRI);
		CELL_Build_MSG(Data_Ptr, MQTT_ID);
		CELL_Build_MSG(Data_Ptr, MQTT_STRI);
		CELL_Build_MSG(Data_Ptr, MQTT_APND);
		CELL_Build_MSG(Data_Ptr, MQTT_STRI);
		CELL_Build_MSG(Data_Ptr, Username);
		CELL_Build_MSG(Data_Ptr, MQTT_STRI);
		CELL_Build_MSG(Data_Ptr, MQTT_APND);
		CELL_Build_MSG(Data_Ptr, MQTT_STRI);
		CELL_Build_MSG(Data_Ptr, Password);
		CELL_Build_MSG(Data_Ptr, MQTT_STRI);
		CELL_Build_MSG(Data_Ptr, MQTT_TAIL);

		Timer = 3000;
		Size = buf;
		RxSize = 120;
		CELL_MSG5(Data_Ptr);
		return;
	case DISCONNECT:
		CELL_Build_MSG(Data_Ptr, MQTT_HEAD);
		CELL_Build_MSG(Data_Ptr, MQTT_DISCONNECT_MSG);
		CELL_Build_MSG(Data_Ptr, MQTT_SOCKET_ID);
		CELL_Build_MSG(Data_Ptr, MQTT_TAIL);

		Timer = UART_EXTEND_TIME + UART_EXTEND_TIME + UART_EXTEND_TIME;
		Size = UART_MAX_SIZE;
		CELL_MSG(Data_Ptr, false);
		checkCellOk(Data_Ptr, CELL_DISCONNECT_ERR);
		return;

	case SUBSCRIBEUP:
		CELL_Build_MSG(Data_Ptr, MQTT_HEAD);
		CELL_Build_MSG(Data_Ptr, MQTT_SUBSCRIBE_MSG);
		CELL_Build_MSG(Data_Ptr, MQTT_SOCKET_ID);
		CELL_Build_MSG(Data_Ptr, MQTT_APND);
		CELL_Build_MSG(Data_Ptr, Message_ID);
		CELL_Build_MSG(Data_Ptr, MQTT_APND);
		CELL_Build_MSG(Data_Ptr, MQTT_STRI);
		//		CELL_Build_MSG(Data_Ptr, MQTT_UPLINK);
		CELL_Build_MSG(Data_Ptr, MQTT_UPLINK_X);
		CELL_Build_MSG(Data_Ptr, MQTT_ID);
		CELL_Build_MSG(Data_Ptr, MQTT_UPLINK_Z);
		CELL_Build_MSG(Data_Ptr, MQTT_STRI);
		CELL_Build_MSG(Data_Ptr, MQTT_APND);
		CELL_Build_MSG(Data_Ptr, MQTT_QOS);
		CELL_Build_MSG(Data_Ptr, MQTT_TAIL);

		Timer = UART_EXTEND_TIME;
		Size = 65;
		CELL_MSG(Data_Ptr, false);
		checkCellOk(Data_Ptr, CELL_SUBSCRIBE_ERR);

		return;

	case SUBSCRIBEDOWN:
		CELL_Build_MSG(Data_Ptr, MQTT_HEAD);
		CELL_Build_MSG(Data_Ptr, MQTT_SUBSCRIBE_MSG);
		CELL_Build_MSG(Data_Ptr, MQTT_SOCKET_ID);
		CELL_Build_MSG(Data_Ptr, MQTT_APND);
		CELL_Build_MSG(Data_Ptr, Message_ID);
		CELL_Build_MSG(Data_Ptr, MQTT_APND);
		CELL_Build_MSG(Data_Ptr, MQTT_STRI);
		CELL_Build_MSG(Data_Ptr, MQTT_DOWNLINK_X);
		CELL_Build_MSG(Data_Ptr, MQTT_ID);
		CELL_Build_MSG(Data_Ptr, MQTT_DOWNLINK_Z);
		CELL_Build_MSG(Data_Ptr, MQTT_STRI);
		CELL_Build_MSG(Data_Ptr, MQTT_APND);
		CELL_Build_MSG(Data_Ptr, MQTT_QOS);
		CELL_Build_MSG(Data_Ptr, MQTT_TAIL);

		Timer = 3000;
		Size = buf;
		RxSize = 86;
		CELL_MSG5(Data_Ptr);

		//		Command = CHECK;
		//		CELL_COMMAND(Data_Ptr);

		return;

	case UNSUBSCRIBE:
		CELL_Build_MSG(Data_Ptr, MQTT_HEAD);
		CELL_Build_MSG(Data_Ptr, MQTT_UNSUBSCRIBE_MSG);
		CELL_Build_MSG(Data_Ptr, MQTT_SOCKET_ID);
		CELL_Build_MSG(Data_Ptr, MQTT_APND);
		CELL_Build_MSG(Data_Ptr, Message_ID);
		CELL_Build_MSG(Data_Ptr, MQTT_APND);
		CELL_Build_MSG(Data_Ptr, MQTT_STRI);
		//		CELL_Build_MSG(Data_Ptr, MQTT_UPLINK);
		CELL_Build_MSG(Data_Ptr, MQTT_UPLINK_X);
		CELL_Build_MSG(Data_Ptr, MQTT_ID);
		CELL_Build_MSG(Data_Ptr, MQTT_UPLINK_Z);
		CELL_Build_MSG(Data_Ptr, MQTT_STRI);
		CELL_Build_MSG(Data_Ptr, MQTT_TAIL);

		Timer = UART_EXTEND_TIME;
		Size = UART_MAX_SIZE;
		CELL_MSG(Data_Ptr, false);
		checkCellOk(Data_Ptr, CELL_UNSUBSCRIBE_ERR);

		CELL_Build_MSG(Data_Ptr, MQTT_HEAD);
		CELL_Build_MSG(Data_Ptr, MQTT_UNSUBSCRIBE_MSG);
		CELL_Build_MSG(Data_Ptr, MQTT_SOCKET_ID);
		CELL_Build_MSG(Data_Ptr, MQTT_APND);
		CELL_Build_MSG(Data_Ptr, Message_ID);
		CELL_Build_MSG(Data_Ptr, MQTT_APND);
		CELL_Build_MSG(Data_Ptr, MQTT_STRI);
		//		CELL_Build_MSG(Data_Ptr, MQTT_DOWNLINK);
		CELL_Build_MSG(Data_Ptr, MQTT_DOWNLINK_X);
		CELL_Build_MSG(Data_Ptr, MQTT_ID);
		CELL_Build_MSG(Data_Ptr, MQTT_DOWNLINK_Z);
		CELL_Build_MSG(Data_Ptr, MQTT_STRI);
		CELL_Build_MSG(Data_Ptr, MQTT_TAIL);

		Timer = UART_EXTEND_TIME;
		Size = UART_MAX_SIZE;
		CELL_MSG(Data_Ptr, false);
		checkCellOk(Data_Ptr, CELL_UNSUBSCRIBE_ERR);

		return;
	case HTTP_URL_UPDT_POST:
		Clear_Memory(Data_Ptr);
		buf = 0;

		Build_MSG(Data_Ptr, HTTP_URL_POST);
		Size = buf;
		utoa( buf, TX_Size, 10);

		CELL_Build_MSG(Data_Ptr, HTTP_HEAD);
		CELL_Build_MSG(Data_Ptr, HTTP_URL);
		CELL_Build_MSG(Data_Ptr, TX_Size);
		CELL_Build_MSG(Data_Ptr, HTTP_PUBLISH_TIME);
		CELL_Build_MSG(Data_Ptr, HTTP_TAIL);

		Timer = UART_WAIT_TIME;
		Size = UART_MAX_SIZE;
		CELL_MSG(Data_Ptr, false);

		Clear_Memory(Data_Ptr);
		buf = 0;

		Build_MSG(Data_Ptr, HTTP_URL_POST);
		Size = buf;

		Command = PUBLISH_MEM;
		CELL_COMMAND(Data_Ptr);
		Command = CHECK;
		CELL_COMMAND(Data_Ptr);
		return;
	case HTTP_URL_UPDT_REQ:
		Clear_Memory(Data_Ptr);
		buf = 0;

		Build_MSG(Data_Ptr, HTTP_URL_REQ);
		Size = buf;
		utoa( buf, TX_Size, 10);

		CELL_Build_MSG(Data_Ptr, HTTP_HEAD);
		CELL_Build_MSG(Data_Ptr, HTTP_URL);
		CELL_Build_MSG(Data_Ptr, TX_Size);
		CELL_Build_MSG(Data_Ptr, HTTP_PUBLISH_TIME);
		CELL_Build_MSG(Data_Ptr, HTTP_TAIL);

		Timer = UART_WAIT_TIME;
		Size = UART_MAX_SIZE;
		CELL_MSG(Data_Ptr, false);

		Clear_Memory(Data_Ptr);
		buf = 0;

		Build_MSG(Data_Ptr, HTTP_URL_POST);
		Size = buf;

		Command = PUBLISH_MEM;
		CELL_COMMAND(Data_Ptr);
		Command = CHECK;
		CELL_COMMAND(Data_Ptr);
		return;
	case PUBLISH:
		utoa( Size, TX_Size, 10);
		CELL_Build_MSG(Data_Ptr, MQTT_HEAD);
		CELL_Build_MSG(Data_Ptr, MQTT_PUBLISH_MSG);
		CELL_Build_MSG(Data_Ptr, MQTT_SOCKET_ID);
		CELL_Build_MSG(Data_Ptr, MQTT_APND);

		utoa( dummy++, Message_ID, 10); // make number unfixed

		CELL_Build_MSG(Data_Ptr, Message_ID);	// increment msg id here
		CELL_Build_MSG(Data_Ptr, MQTT_APND);
		CELL_Build_MSG(Data_Ptr, MQTT_QOS);
		CELL_Build_MSG(Data_Ptr, MQTT_APND);
		CELL_Build_MSG(Data_Ptr, MQTT_RETAIN);
		CELL_Build_MSG(Data_Ptr, MQTT_APND);
		CELL_Build_MSG(Data_Ptr, MQTT_STRI);
		CELL_Build_MSG(Data_Ptr, MQTT_UPLINK_X);
		CELL_Build_MSG(Data_Ptr, MQTT_ID);
		CELL_Build_MSG(Data_Ptr, MQTT_UPLINK_Z);
		CELL_Build_MSG(Data_Ptr, MQTT_STRI);
		CELL_Build_MSG(Data_Ptr, MQTT_APND);
		CELL_Build_MSG(Data_Ptr, TX_Size);
		CELL_Build_MSG(Data_Ptr, MQTT_TAIL);

		HAL_UART_AbortReceive(&hlpuart1);

		Timer = UART_WAIT_TIME;
		if (HAL_UART_Transmit(&hlpuart1, TX_Command, buf, Timer) != HAL_OK)
		{
			Cell_State = CELL_TX_CMD_ERR;
			return;
		}

		while (1)
		{
			//HAL_Delay ( 3 );
			if (HAL_UART_Receive(&hlpuart1, TX_Command, 1, Timer) == HAL_OK)
			{
				break;
			}
		}
		//HAL_Delay ( 3 );

		Log_Single( LOG_MSG_PUBLISH);
		return;
	case PUBLISH_HTTP:
		CELL_Build_MSG(Data_Ptr, HTTP_HEAD);
		CELL_Build_MSG(Data_Ptr, HTTP_PUBLISH_CMD_MSG);
		CELL_Build_MSG(Data_Ptr, TX_Size);
		CELL_Build_MSG(Data_Ptr, HTTP_PUBLISH_TIME);
		CELL_Build_MSG(Data_Ptr, HTTP_PUBLISH_TIME);
		CELL_Build_MSG(Data_Ptr, HTTP_TAIL);

		Timer = UART_WAIT_TIME;
		//			Size = UART_MAX_SIZE;
		if (HAL_UART_Transmit(&hlpuart1, TX_Command, buf, Timer) != HAL_OK)
		{
			Cell_State = CELL_TX_CMD_ERR;
			checkCellOk(Data_Ptr, CELL_PUBLISH_CMD_ERR);
		}

		Cell_State = CELL_OK;

#ifdef Log_Level_1
		if (Setting & LOG_LEVEL_1 && Setting & LOG_LEVEL_2)
		{
			Write_Log( LOG_SUCCESS);
			Log_End( LOG_STATE_RESULT, Cell_State);
		}
#endif // Log_Level_1
		return;
	case PUBLISH_MQTT_CMD:
		CELL_Build_MSG(Data_Ptr, MQTT_HEAD);
		CELL_Build_MSG(Data_Ptr, MQTT_PUBLISH_CMD_MSG);
		CELL_Build_MSG(Data_Ptr, MQTT_TAIL);

		Timer = UART_WAIT_TIME;
		Size = UART_MAX_SIZE;
		CELL_MSG(Data_Ptr, false);
		checkCellOk(Data_Ptr, CELL_PUBLISH_CMD_ERR);
		return;
	case PUBLISH_MEM:
		HAL_UART_AbortReceive(&hlpuart1);
		if (HAL_UART_Transmit(&hlpuart1, Memory, Size, Timer) != HAL_OK)
		{
			Cell_State = CELL_TX_CMD_ERR;
			return;
		}

#ifdef SKYWIRE_MODEM_OUT
		Write_Log( LOG_MSG_DATA);
		Log_Single((char*) Memory);
#endif // SKYWIRE_MODEM_OUT

		Size = 255;
		Timer = UART_WAIT_TIME;
		if (HAL_UART_Receive(&hlpuart1, Memory, Size, Timer) != HAL_OK)
		{
			Cell_State = CELL_RX_CMD_ERR;
			return;
		}
		return;

	case READ_BUF_CHECK:
		CELL_Build_MSG(Data_Ptr, MQTT_HEAD);
		CELL_Build_MSG(Data_Ptr, MQTT_READ_RECV_BUFF_CHECK);
		CELL_Build_MSG(Data_Ptr, MQTT_TAIL);

		Timer = 100;//UART_EXTEND_TIME; 100 desired NW   //GAV can test increase if having issues dont go past 2FF
		Size = 300;//UART_MAX_SIZE;  40 desired NW
		//CELL_READ_URC(Data_Ptr);
		CELL_MSG(Data_Ptr, false);

		checkCellOk(Data_Ptr, CELL_READ_BUF_ERR);

		if (!(DEV_State & SERVER_COMMAND))
		{
			HAL_UART_AbortReceive(&hlpuart1);
		}

		RecvMessageNum((char*)Memory, Data_Ptr);

	case READ_FW_BUF_CHECK:
		CELL_Build_MSG(Data_Ptr, MQTT_HEAD);
		CELL_Build_MSG(Data_Ptr, MQTT_READ_RECV_BUFF_CHECK);
		CELL_Build_MSG(Data_Ptr, MQTT_TAIL);

		Timer = 100;//UART_EXTEND_TIME; 100 desired NW   //GAV can test increase if having issues dont go past 2FF
		Size = 50;//UART_MAX_SIZE;  40 desired NW
		//CELL_READ_URC(Data_Ptr);
		CELL_MSG(Data_Ptr, false);

		checkCellOk(Data_Ptr, CELL_READ_BUF_ERR);

		if (!(DEV_State & SERVER_COMMAND))
		{
			HAL_UART_AbortReceive(&hlpuart1);
		}

		return;

	case RECEIVE_MSG:

		CELL_Build_MSG(Data_Ptr, MQTT_HEAD);
		sprintf(msgbuffer, "+QMTRECV=0,%d",Data_Ptr->recIndex );
		CELL_Build_MSG(Data_Ptr, msgbuffer);
		CELL_Build_MSG(Data_Ptr, MQTT_TAIL);

		Timer = 100;
		Size = testSize;  //Not used MEMORY_MAX is in Cell_msg4
		Data_Ptr->UartBtyesReceived = CELL_MSG4(Data_Ptr);
		//		checkCellOk(Data_Ptr, CELL_READ_BUF_ERR);

		if (!(DEV_State & SERVER_COMMAND))
		{
			HAL_UART_AbortReceive(&hlpuart1);
		}
		return;

	case recvMsg1:

		CELL_Build_MSG(Data_Ptr, MQTT_HEAD);
		CELL_Build_MSG(Data_Ptr, MQTT_READ_RECV_BUFF_1);
		CELL_Build_MSG(Data_Ptr, MQTT_TAIL);

		Timer = 100;
		Size = testSize;  //Not used MEMORY_MAX is in Cell_msg4
		Data_Ptr->UartBtyesReceived = CELL_MSG4(Data_Ptr);
		//		checkCellOk(Data_Ptr, CELL_READ_BUF_ERR);

		if (!(DEV_State & SERVER_COMMAND))
		{
			HAL_UART_AbortReceive(&hlpuart1);
		}
		return;

	case recvMsg2:

		CELL_Build_MSG(Data_Ptr, MQTT_HEAD);
		CELL_Build_MSG(Data_Ptr, MQTT_READ_RECV_BUFF_2);
		CELL_Build_MSG(Data_Ptr, MQTT_TAIL);

		Timer = 100;//UART_EXTEND_TIME; 100 desired NW   //GAV can test increase if having issues dont go past 2FF
		Size = testSize;//UART_MAX_SIZE;  40 desired NW
		Data_Ptr->UartBtyesReceived = CELL_MSG4(Data_Ptr);
		//		checkCellOk(Data_Ptr, CELL_READ_BUF_ERR);

		if (!(DEV_State & SERVER_COMMAND))
		{
			HAL_UART_AbortReceive(&hlpuart1);
		}
		return;

	case recvMsg3:

		CELL_Build_MSG(Data_Ptr, MQTT_HEAD);
		CELL_Build_MSG(Data_Ptr, MQTT_READ_RECV_BUFF_3);
		CELL_Build_MSG(Data_Ptr, MQTT_TAIL);

		Timer = 100;//UART_EXTEND_TIME; 100 desired NW   //GAV can test increase if having issues dont go past 2FF
		Size = testSize;//UART_MAX_SIZE;  40 desired NW
		Data_Ptr->UartBtyesReceived = CELL_MSG4(Data_Ptr);
		//		checkCellOk(Data_Ptr, CELL_READ_BUF_ERR);

		if (!(DEV_State & SERVER_COMMAND))
		{
			HAL_UART_AbortReceive(&hlpuart1);
		}
		return;

	case recvMsg4:

		CELL_Build_MSG(Data_Ptr, MQTT_HEAD);
		CELL_Build_MSG(Data_Ptr, MQTT_READ_RECV_BUFF_4);
		CELL_Build_MSG(Data_Ptr, MQTT_TAIL);

		Timer = 100;//UART_EXTEND_TIME; 100 desired NW   //GAV can test increase if having issues dont go past 2FF
		Size = testSize;//UART_MAX_SIZE;  40 desired NW
		Data_Ptr->UartBtyesReceived = CELL_MSG4(Data_Ptr);
		//		checkCellOk(Data_Ptr, CELL_READ_BUF_ERR);

		if (!(DEV_State & SERVER_COMMAND))
		{
			HAL_UART_AbortReceive(&hlpuart1);
		}
		return;

	case recvMsg5:

		CELL_Build_MSG(Data_Ptr, MQTT_HEAD);
		CELL_Build_MSG(Data_Ptr, MQTT_READ_RECV_BUFF_5);
		CELL_Build_MSG(Data_Ptr, MQTT_TAIL);

		Timer = 100;//UART_EXTEND_TIME; 100 desired NW   //GAV can test increase if having issues dont go past 2FF
		Size = testSize;//UART_MAX_SIZE;  40 desired NW
		Data_Ptr->UartBtyesReceived = CELL_MSG4(Data_Ptr);
		//		checkCellOk(Data_Ptr, CELL_READ_BUF_ERR);

		if (!(DEV_State & SERVER_COMMAND))
		{
			HAL_UART_AbortReceive(&hlpuart1);
		}
		return;


	case READ_BUF:
		CELL_Build_MSG(Data_Ptr, MQTT_HEAD);
		CELL_Build_MSG(Data_Ptr, MQTT_READ_RECV_BUFF);
		CELL_Build_MSG(Data_Ptr, MQTT_SOCKET_ID);
		CELL_Build_MSG(Data_Ptr, MQTT_TAIL);

		Timer = 100;//UART_EXTEND_TIME; 100 desired NW   //GAV can test increase if having issues dont go past 2FF
		Size = 300;//UART_MAX_SIZE;  40 desired NW
		//CELL_READ_URC(Data_Ptr);
		CELL_MSG(Data_Ptr, false);
		//Erik thinks this is for error capture
		checkCellOk(Data_Ptr, CELL_READ_BUF_ERR);

		if (!(DEV_State & SERVER_COMMAND))
		{
			HAL_UART_AbortReceive(&hlpuart1);
		}
		return;

	case READ_BUF2:
		CELL_Build_MSG(Data_Ptr, MQTT_HEAD);
		CELL_Build_MSG(Data_Ptr, MQTT_READ_RECV_BUFF);
		CELL_Build_MSG(Data_Ptr, MQTT_SOCKET_ID);
		CELL_Build_MSG(Data_Ptr, MQTT_TAIL);

		Timer = 100;
		Size = 1000;
		CELL_MSG2(Data_Ptr);
		return;
	case CONFIG:
		CELL_Build_MSG(Data_Ptr, MQTT_HEAD);
		CELL_Build_MSG(Data_Ptr, MQTT_CONFIG_MSG);
		CELL_Build_MSG(Data_Ptr, MQTT_CONFIG_A);
		CELL_Build_MSG(Data_Ptr, MQTT_TAIL);

		Timer = UART_WAIT_TIME;
		Size = UART_MAX_SIZE;
		CELL_MSG(Data_Ptr, false);
		// Using Check2 causing big delay in receiving data from picture but it will
		// "work" for at least this one
		Command = CHECK;  // CHECK2 desired NW
		//Timer = 50;
		//Size = 16;
		CELL_COMMAND(Data_Ptr);

		CELL_Build_MSG(Data_Ptr, MQTT_HEAD);
		CELL_Build_MSG(Data_Ptr, MQTT_CONFIG_MSG);
		CELL_Build_MSG(Data_Ptr, MQTT_CONFIG_B);
		CELL_Build_MSG(Data_Ptr, MQTT_TAIL);

		Timer = UART_WAIT_TIME;
		Size = UART_MAX_SIZE;
		CELL_MSG(Data_Ptr, false);

		Command = CHECK;  // CHECK2 desired NW
		//Timer = 50; only works with CHECK2   desired NW
		//Size = 16;
		CELL_COMMAND(Data_Ptr);

		CELL_Build_MSG(Data_Ptr, MQTT_HEAD);
		CELL_Build_MSG(Data_Ptr, MQTT_CONFIG_MSG);
		CELL_Build_MSG(Data_Ptr, MQTT_CONFIG_C);
		CELL_Build_MSG(Data_Ptr, MQTT_TAIL);

		Timer = UART_WAIT_TIME;
		Size = UART_MAX_SIZE;
		CELL_MSG(Data_Ptr, false);

		Command = CHECK;  // CHECK2 desired NW
		//Timer = 50; only works with CHECK2   desired NW
		//Size = 16;
		CELL_COMMAND(Data_Ptr);

		CELL_Build_MSG(Data_Ptr, MQTT_HEAD);
		CELL_Build_MSG(Data_Ptr, MQTT_CONFIG_MSG);
		CELL_Build_MSG(Data_Ptr, MQTT_CONFIG_D);
		CELL_Build_MSG(Data_Ptr, MQTT_TAIL);

		Timer = UART_WAIT_TIME;
		Size = UART_MAX_SIZE;
		CELL_MSG(Data_Ptr, false);

		Command = CHECK;
		CELL_COMMAND(Data_Ptr);
		CELL_Build_MSG(Data_Ptr, MQTT_HEAD);
		CELL_Build_MSG(Data_Ptr, MQTT_CONFIG_MSG);
		CELL_Build_MSG(Data_Ptr, MQTT_CONFIG_E);
		CELL_Build_MSG(Data_Ptr, MQTT_TAIL);

		Timer = UART_WAIT_TIME;
		Size = UART_MAX_SIZE;
		CELL_MSG(Data_Ptr, false);
		Command = CHECK;
		CELL_COMMAND(Data_Ptr);


#ifdef CERTIFICATE
		Command = CHECK;	CELL_COMMAND(Data_Ptr,CELL);
		Command = CA_CERT;	CELL_COMMAND(Data_Ptr,CELL);
		Command = CHECK;	CELL_COMMAND(Data_Ptr,CELL);
		Command = CLIENT_CERT;CELL_COMMAND(Data_Ptr,CELL);
		Command = CHECK;	CELL_COMMAND(Data_Ptr,CELL);
		Command = CLIENT_KEY;CELL_COMMAND(Data_Ptr,CELL);
		Command = CHECK;	CELL_COMMAND(Data_Ptr,CELL);
#endif  // CERTIFICATE
		return;
	case XTRA_ENABLE:
		CELL_Build_MSG(Data_Ptr, GPS_HEAD);
		CELL_Build_MSG(Data_Ptr, GPS_EXTRA_ENABLE);
		CELL_Build_MSG(Data_Ptr, MQTT_TAIL);


		Timer = 3000;
		Size = buf;
		RxSize = 100;
		CELL_MSG5(Data_Ptr);    //use CELL_MSG
		return;

	case XTRA_AUTODL_ENABLE:
		CELL_Build_MSG(Data_Ptr, GPS_HEAD);
		CELL_Build_MSG(Data_Ptr, GPS_EXTRA_AUTODL_ENABLE);
		CELL_Build_MSG(Data_Ptr, MQTT_TAIL);


		Timer = 3000;
		Size = buf;
		RxSize = 100;
		CELL_MSG5(Data_Ptr);    //use CELL_MSG
		return;

	case XTRA_TIME_QUERY:
		CELL_Build_MSG(Data_Ptr, GPS_HEAD);
		CELL_Build_MSG(Data_Ptr, GPS_EXTRA_EXTRATIME_QUERY);
		CELL_Build_MSG(Data_Ptr, MQTT_TAIL);


		Timer = 3000;
		Size = buf;
		RxSize = 100;
		CELL_MSG5(Data_Ptr);    //use CELL_MSG
		return;

	case GPS_ACTIVE_ANTENNA_ENABLE:
		CELL_Build_MSG(Data_Ptr, GPS_HEAD);
		CELL_Build_MSG(Data_Ptr, CELL_QCFG);
		CELL_Build_MSG(Data_Ptr, MQTT_STRI);
		CELL_Build_MSG(Data_Ptr, CELL_GPIO);
		CELL_Build_MSG(Data_Ptr, MQTT_STRI);
		CELL_Build_MSG(Data_Ptr, GPS_ACTIVE_ANTENNA_GPIO);
		CELL_Build_MSG(Data_Ptr, MQTT_TAIL);



		Timer = 3000;
		Size = buf;
		RxSize = 32;
		CELL_MSG5(Data_Ptr);    //use CELL_MSG5
		return;

	case GPS_ACTIVE_ANTENNA_SET:
		CELL_Build_MSG(Data_Ptr, GPS_HEAD);
		CELL_Build_MSG(Data_Ptr, CELL_QCFG);
		CELL_Build_MSG(Data_Ptr, MQTT_STRI);
		CELL_Build_MSG(Data_Ptr, CELL_GPIO);
		CELL_Build_MSG(Data_Ptr, MQTT_STRI);
		CELL_Build_MSG(Data_Ptr, GPS_ACTIVE_ANTENNA_ON);
		CELL_Build_MSG(Data_Ptr, MQTT_TAIL);



		Timer = 3000;
		Size = buf;
		RxSize = 28;
		CELL_MSG5(Data_Ptr);    //use CELL_MSG
		return;

		//GAV needs to finish this, XTRA GPS time injection command
//	case XTRA_XTRA_TIME_INJECT:
//		CELL_Build_MSG(Data_Ptr, GPS_HEAD);
//		CELL_Build_MSG(Data_Ptr, GPS_EXTRA_EXTRATIME_QUERY);
//
//		Timer = UART_WAIT_TIME;
//		Size = UART_MAX_SIZE;
//		CELL_MSG(Data_Ptr, false);    //use CELL_MSG5
//		return;

	case GPS_ENABLE:
		CELL_Build_MSG(Data_Ptr, GPS_HEAD);
		CELL_Build_MSG(Data_Ptr, GPS_CONNECT);
		CELL_Build_MSG(Data_Ptr, GPS_CONNECT_CNFG);
		CELL_Build_MSG(Data_Ptr, GPS_TAIL);

		Timer = 10000;
		Size = buf;
		RxSize = 100;
		CELL_MSG5(Data_Ptr);    //use CELL_MSG
		return;
	case GPS_DISABLE:
		CELL_Build_MSG(Data_Ptr, GPS_HEAD);
		CELL_Build_MSG(Data_Ptr, GPS_DISCONNECT);
		CELL_Build_MSG(Data_Ptr, GPS_TAIL);

		Timer = UART_WAIT_TIME;
		Size = UART_MAX_SIZE;
		CELL_MSG(Data_Ptr, false);
		checkCellOk(Data_Ptr, CELL_GPS_CONNECT_ERR);

		return;
	case LOCATION:
		CELL_Build_MSG(Data_Ptr, GPS_HEAD);
		CELL_Build_MSG(Data_Ptr, GPS_LOCATION);
		CELL_Build_MSG(Data_Ptr, GPS_TAIL);

		Timer = 2000;
		Size = buf;
		RxSize = 400;
		CELL_MSG5(Data_Ptr);    //use CELL_MSG5
		return;
	case PDP_POPULATE:
		CELL_Build_MSG(Data_Ptr, FILE_HEAD);
		CELL_Build_MSG(Data_Ptr, CELL_CFUN_STOP);
		CELL_Build_MSG(Data_Ptr, FILE_TAIL);

		Timer = UART_WAIT_TIME;
		Size = UART_MAX_SIZE;
		CELL_MSG(Data_Ptr, false);

		Command = CHECK;
		for (Temporary = 0; Cell_State != CELL_OK; Temporary++)
		{
			CELL_COMMAND(Data_Ptr);
			if (Temporary >= TEMP_LIMIT)
				break;
		}

		CELL_Build_MSG(Data_Ptr, FILE_HEAD);
		CELL_Build_MSG(Data_Ptr, CELL_SIM_SEL_PREP);
		CELL_Build_MSG(Data_Ptr, FILE_TAIL);

		Timer = UART_WAIT_TIME;
		Size = UART_MAX_SIZE;
		CELL_MSG(Data_Ptr, false);

		HAL_Delay(1500);

		Command = CHECK;
		for (Temporary = 0; Cell_State != CELL_OK; Temporary++)
		{
			CELL_COMMAND(Data_Ptr);
			if (Temporary >= TEMP_LIMIT)
				break;
		}

		CELL_Build_MSG(Data_Ptr, FILE_HEAD);
		CELL_Build_MSG(Data_Ptr, CELL_CFUN_START);
		CELL_Build_MSG(Data_Ptr, FILE_TAIL);

		Timer = UART_WAIT_TIME;
		Size = UART_MAX_SIZE;
		CELL_MSG(Data_Ptr, false);

		HAL_Delay(1500);

		Command = CHECK;
		for (Temporary = 0; Cell_State != CELL_OK; Temporary++)
		{
			CELL_COMMAND(Data_Ptr);
			if (Temporary >= TEMP_LIMIT)
				break;
		}

		CELL_Build_MSG(Data_Ptr, FILE_HEAD);
		CELL_Build_MSG(Data_Ptr, CELL_PDP_CONTEX);
		PRINTF("APN is %s\r\n", CELL_PDP_CONTEX);

		CELL_Build_MSG(Data_Ptr, FILE_TAIL);

		Timer = UART_WAIT_TIME;
		Size = UART_MAX_SIZE;
		CELL_MSG(Data_Ptr, false);

		HAL_Delay(1500);

		Command = CHECK;
		for (Temporary = 0; Cell_State != CELL_OK; Temporary++)
		{
			CELL_COMMAND(Data_Ptr);
			if (Temporary >= TEMP_LIMIT)
				break;
		}



		return;
	case FILE_POPULATE:
		HAL_UART_AbortReceive(&hlpuart1);
		if (HAL_UART_Transmit(&hlpuart1, Memory, Size, Timer) != HAL_OK)
		{
			Cell_State = CELL_TX_CMD_ERR;
			return;
		}
		return;
	case FILE_CREATE:
		CELL_Build_MSG(Data_Ptr, FILE_HEAD);
		CELL_Build_MSG(Data_Ptr, FILE_CREATE_MSG);
		CELL_Build_MSG(Data_Ptr, Event_Type);
		CELL_Build_MSG(Data_Ptr, FILE_APND);
		CELL_Build_MSG(Data_Ptr, FILE_SIZE);
		CELL_Build_MSG(Data_Ptr, FILE_APND);
		CELL_Build_MSG(Data_Ptr, FILE_TIMEOUT);
		CELL_Build_MSG(Data_Ptr, FILE_TAIL);

		Timer = UART_WAIT_TIME;
		Size = UART_MAX_SIZE;
		CELL_MSG(Data_Ptr, false);
		checkCellOk(Data_Ptr, CELL_FILE_CREATE_ERR);
		Cell_State = CELL_OK;
		return;
	case FILE_DELETE:
		CELL_Build_MSG(Data_Ptr, FILE_HEAD);
		CELL_Build_MSG(Data_Ptr, FILE_DEL_MSG);
		CELL_Build_MSG(Data_Ptr, Event_Type);
		CELL_Build_MSG(Data_Ptr, FILE_TAIL);

		Timer = UART_WAIT_TIME;
		Size = UART_MAX_SIZE;
		CELL_MSG(Data_Ptr, false);
		checkCellOk(Data_Ptr, CELL_FILE_DELETE_ERR);
		return;
	case CIPHER:
		CELL_Build_MSG(Data_Ptr, CERT_HEAD);
		CELL_Build_MSG(Data_Ptr, SSL_CIPHER_MSG);
		CELL_Build_MSG(Data_Ptr, SSL_DEFAULT_MSG_ID);
		CELL_Build_MSG(Data_Ptr, CERT_APND);
		CELL_Build_MSG(Data_Ptr, SSL_CIPHER);
		CELL_Build_MSG(Data_Ptr, CERT_TAIL);

		Timer = UART_WAIT_TIME;
		Size = UART_MAX_SIZE;
		CELL_MSG(Data_Ptr, false);
		checkCellOk(Data_Ptr, CELL_CIPHER_ERR);
		return;
	case CA_CERT:
		CELL_Build_MSG(Data_Ptr, CERT_HEAD);
		CELL_Build_MSG(Data_Ptr, SSL_CACERT_MSG);
		CELL_Build_MSG(Data_Ptr, SSL_DEFAULT_MSG_ID);
		CELL_Build_MSG(Data_Ptr, CERT_APND);
		CELL_Build_MSG(Data_Ptr, SSL_CACERT);
		CELL_Build_MSG(Data_Ptr, CERT_TAIL);

		Timer = UART_WAIT_TIME;
		Size = UART_MAX_SIZE;
		CELL_MSG(Data_Ptr, false);
		checkCellOk(Data_Ptr, CELL_CERTIFICATION_ERR);
		return;
	case CLIENT_CERT:
		CELL_Build_MSG(Data_Ptr, CERT_HEAD);
		CELL_Build_MSG(Data_Ptr, SSL_CLCERT_MSG);
		CELL_Build_MSG(Data_Ptr, SSL_DEFAULT_MSG_ID);
		CELL_Build_MSG(Data_Ptr, CERT_APND);
		CELL_Build_MSG(Data_Ptr, SSL_CLCERT);
		CELL_Build_MSG(Data_Ptr, CERT_TAIL);

		Timer = UART_WAIT_TIME;
		Size = UART_MAX_SIZE;
		CELL_MSG(Data_Ptr, false);
		checkCellOk(Data_Ptr, CELL_CLIENT_CERTIFICATION_ERR);
		return;
	case CLIENT_KEY:
		CELL_Build_MSG(Data_Ptr, CERT_HEAD);
		CELL_Build_MSG(Data_Ptr, SSL_CLNTKEY_MSG);
		CELL_Build_MSG(Data_Ptr, SSL_DEFAULT_MSG_ID);
		CELL_Build_MSG(Data_Ptr, CERT_APND);
		CELL_Build_MSG(Data_Ptr, SSL_CLNTKEY);
		CELL_Build_MSG(Data_Ptr, CERT_TAIL);

		Timer = UART_WAIT_TIME;
		Size = UART_MAX_SIZE;
		CELL_MSG(Data_Ptr, false);
		checkCellOk(Data_Ptr, CELL_CLIENT_KEY_ERR);
		return;
	default:
		Cell_State = CELL_TYPE_PARAM_ERR;
		break;
	}
}

/******************************************************
Sends the TX_Command to the modem and waits for a response
 ******************************************************/
void CELL_MSG( MEM_PTR *Data_Ptr, bool clearBuffer )
{
	Refresh_Watchdog;
	//	if ( Cell_State != CELL_OK )
	//		return;

	//uint8_t receivedData [ 2000 ] = {0};
	// kcs
	HAL_UART_AbortReceive(&hlpuart1);

	if ( HAL_UART_Transmit(&hlpuart1, TX_Command, buf, Timer) != HAL_OK )
	{
		Cell_State = CELL_TX_CMD_ERR;

#ifdef Log_Level_1
		Write_Log( LOG_ERROR);
		Log_End( LOG_STATE_RESULT, Cell_State);
#endif // Log_Level_1

		return;
	}

	uint8_t test = 0;
	if (clearBuffer)
	{
		Clear_Memory(Data_Ptr);
	}
	do
	{
		if (HAL_UART_Receive( &hlpuart1, &(Memory[test++]), 1, Timer) != HAL_OK)
		{
			Cell_State = CELL_RX_CMD_ERR;
			return;

		}
	} while (Size--);

#ifdef Log_Level_2
	Write_Log( LOG_RX_ERR);
	Log_End( LOG_STATE_RESULT, Cell_State);
#endif // Log_Level_2
	test = test + 1;
}

/******************************************************
Sends the TX_Command to the modem and waits for a response
 ******************************************************/
void CELL_MSG2(MEM_PTR *Data_Ptr)
{
	if (Cell_State != CELL_OK)
		return;
	HAL_UART_AbortReceive( &hlpuart1 );
	uint8_t receivedData [ 5000 ] = {0};
	uint8_t test = 0;

	do
	{
		if ( HAL_UART_Transmit(&hlpuart1, TX_Command, buf, Timer) != HAL_OK )
		{
			Cell_State = CELL_TX_CMD_ERR;

#ifdef Log_Level_1
			Write_Log( LOG_ERROR);
			Log_End( LOG_STATE_RESULT, Cell_State);
#endif // Log_Level_1
			return;
		}

		do
		{
			if (HAL_UART_Receive(&hlpuart1, &(receivedData[test++]), 1, Timer) != HAL_OK)
			{
				Cell_State = CELL_RX_CMD_ERR;
				break;
			}
		} while (Size--);
		Refresh_Watchdog;
	}while (Size--);

#ifdef Log_Level_2
	Write_Log( LOG_RX_ERR);
	Log_End( LOG_STATE_RESULT, Cell_State);
#endif // Log_Level_2
	test = test + 1;
}

/******************************************************
Sends the TX_Command to the modem and waits for a response
 ******************************************************/
void CELL_MSG3( MEM_PTR *Data_Ptr)
//{
//	Refresh_Watchdog;
//	Clear_Memory(Data_Ptr);
//	Cell_State = CELL_OK;
//	HAL_UART_AbortReceive(&hlpuart1);
//
//	if ( HAL_UART_Transmit(&hlpuart1, TX_Command, buf, Timer) != HAL_OK )
//	{
//		Cell_State = CELL_TX_CMD_ERR;
//		return;
//	}
//
//	uint8_t test = 0;
//	uint32_t recTimer = 500;
//
//	do
//	{
//		if (HAL_UART_Receive( &hlpuart1, &(Memory[test++]), 1, recTimer) != HAL_OK)
//		{
//
//			break;
//
//		}
//	} while (Size--);
//
//	test = test + 1;
//}

{
	uint8_t receivedData [ 5000 ] = {0};
	Refresh_Watchdog;
	Clear_Memory(Data_Ptr);
	//	BACKUP_PRIMASK();
	//	DISABLE_IRQ(); // Disable all interrupts
	Cell_State = CELL_OK;
	// KCS need to handle all returns of HAL_UART_Transmit
	if ( HAL_UART_Transmit(&hlpuart1, TX_Command, buf, Timer) != HAL_OK )
	{
		Cell_State = CELL_TX_CMD_ERR;
		//		RESTORE_PRIMASK();
		return;
	}
	uint8_t test = 0;
	uint32_t recTimer = 20;
	HAL_StatusTypeDef recResult = HAL_OK;
	uint8_t retries = 0;
	uint8_t maxRetries = 4;
	uint32_t recSize = 300;  // MEMORY_MAX
	do
	{
		recResult = HAL_UART_Receive( &hlpuart1, &(receivedData[test]), 1, recTimer);
		if (recResult == HAL_BUSY)
		{
			HAL_UART_AbortReceive( &hlpuart1 );
			retries++;
			if(retries <= maxRetries)
			{
				recResult = HAL_UART_Receive( &hlpuart1, &(receivedData[test]), 1, recTimer);
			}
			else
			{
				HAL_UART_AbortReceive( &hlpuart1 );
				break;
			}
		}
		else if (recResult == HAL_TIMEOUT)
		{
			HAL_UART_AbortReceive( &hlpuart1 );
			retries++;
			if(retries <= maxRetries)
			{
				recResult = HAL_UART_Receive( &hlpuart1, &(receivedData[test]), 1, recTimer);
			}
			else
			{
				HAL_UART_AbortReceive( &hlpuart1 );
				break;
			}
		}
		else if (recResult == HAL_OK)
		{
			test++;
		}
	} while (recSize--);
	//	RESTORE_PRIMASK();
	test = test + 1;  // for debugging no effect on program
}

uint16_t CELL_MSG4( MEM_PTR *Data_Ptr)
{
	uint16_t numOfBytesReceived = 0;
	Refresh_Watchdog;
	Clear_Memory(Data_Ptr);
	//	BACKUP_PRIMASK();
	//	DISABLE_IRQ(); // Disable all interrupts
	Cell_State = CELL_OK;
	// KCS need to handle all returns of HAL_UART_Transmit
	if ( HAL_UART_Transmit(&hlpuart1, TX_Command, buf, Timer) != HAL_OK )
	{
		Cell_State = CELL_TX_CMD_ERR;
		//		RESTORE_PRIMASK();

		return numOfBytesReceived;
	}
	uint8_t test = 0;
	uint32_t recTimer = 800;

	HAL_UART_Receive( &hlpuart1, receivedData, MEMORY_MAX, recTimer);

	//	RESTORE_PRIMASK();
	test = test + 1;  // for debugging no effect on program
	numOfBytesReceived = MEMORY_MAX - hlpuart1.RxXferCount; // -1 ????
	return numOfBytesReceived;

}

void CELL_MSG5( MEM_PTR *Data_Ptr)
{
	{
		Refresh_Watchdog;
		//	if ( Cell_State != CELL_OK )
		//		return;

		//uint8_t receivedData [ 2000 ] = {0};
		// kcs
		HAL_UART_AbortReceive(&hlpuart1);

		if ( HAL_UART_Transmit(&hlpuart1, TX_Command, buf, Timer) != HAL_OK )
		{
			Cell_State = CELL_TX_CMD_ERR;

#ifdef Log_Level_1
			Write_Log( LOG_ERROR);
			Log_End( LOG_STATE_RESULT, Cell_State);
#endif // Log_Level_1

			return;
		}

		uint8_t test = 0;


		if (HAL_UART_Receive( &hlpuart1, &(Memory[test++]), RxSize, Timer) != HAL_OK)
		{
			Cell_State = CELL_RX_CMD_ERR;
			return;

		}



		test = test + 1;
	}

}

/******************************************************
Read the UART RX
 ******************************************************/
uint16_t CELL_READ_URC(MEM_PTR *Data_Ptr)
{
	if (Cell_State != CELL_OK)
		return 0;

	uint8_t test = 0;
	uint8_t fetchsize = 1; // test++  for test+=1 for fetchsize test+=fetchsize;
	do
	{
		if (HAL_UART_Receive(&hlpuart1, &(Memory[test+=fetchsize]), fetchsize, Timer) != HAL_OK)
		{
			//Cell_State = CELL_RX_CMD_ERR;
			//return;
#ifdef Log_Level_2
			//Write_Log( LOG_RX_ERR);
			//Log_End( LOG_STATE_RESULT, Cell_State);
#endif // Log_Level_2
		}

		// KCS add check for last curly brace
		if (Memory[test-1]== 125) //125 is ascii char '}'
		{
			break;
		}
	} while (Size--);

#ifdef Log_Level_2
	Write_Log( LOG_RX_ERR);
	Log_End( LOG_STATE_RESULT, Cell_State);
#endif // Log_Level_2
	return test;
}

/******************************************************
This searches the received UART message for OK string
 ******************************************************/
void checkCellOk(MEM_PTR *Data_Ptr, CELL_STATE CODE)
{
	Size = 0;
	do
	{
		if ((Memory[ Size]) == '\n')
		{
			if ((Memory[ Size + 1]) == 'O' && (Memory[ Size + 2]) == 'K')
			{
				Memory[ Size + 1] = 0;
				Memory[ Size + 2] = 0;
				Memory[ Size + 3] = 0;
				Memory[ Size + 4] = 0;

				Cell_State = CELL_OK;

#ifdef Log_Level_1
				if (Setting & LOG_LEVEL_1 && Setting & LOG_LEVEL_2)
				{
					Write_Log( LOG_SUCCESS);
					Log_End( LOG_STATE_RESULT, Cell_State);
				}
#endif // Log_Level_1

				return;
			}
		}
	} while (Memory[ Size++]);
	Cell_State = CODE;

#ifdef Log_Level_1
	if ( Setting & LOG_LEVEL_1 && Setting & LOG_LEVEL_2 )
	{
		Write_Log( LOG_ERROR);
		Log_End( LOG_STATE_RESULT, Cell_State);
	} // Log_Level_1
#endif
}

/******************************************************
 Cell Modem_Data.Command Creation Function
 Creates command with input function as well as
 parameterizes values for command. First call will
 require the Buffer = 0 from user
 ******************************************************/
void CELL_Build_MSG( MEM_PTR *Data_Ptr, char *MSG )
{
	//	if (Cell_State != CELL_OK)
	//		return;

	if (*(MSG) == '\1')
	{
		buf = 0;
		MSG++;
	}

	do
	{
		TX_Command[ buf++] = *(MSG)++;
	} while (*(MSG));

	return;
}

/******************************************************
 Command Clean
 Clears Command storage space
 ******************************************************/
void Clear_Command(MEM_PTR *Data_Ptr)
{
	buf = CMD_FLASH_SIZE - 1;
	while (buf)
		TX_Command[ buf--] = 0;
}

/******************************************************
 Memory Location
 Clears Location storage space
 ******************************************************/
void CELL_Set_Command(CELL_CMD_TYPE Input)
{
	Command = Input;
}

void CELL_Set_PDP(CELL_CMD_TYPE Input)
{
	PDP = Input;
}

CELL_STATE CELL_Get_State(void)
{
	return Cell_State;
}


uint16_t resendMissingPackets(MEM_PTR *Data_Ptr, uint16_t missingPagesCount)
{
	uint16_t packetsSent = 0;
	int counter;

	for (counter = 0; counter < missingPagesCount; counter++)
	{
		Refresh_Watchdog;
		Command = CHECK;
		CELL_COMMAND(Data_Ptr);

		Clear_Memory(Data_Ptr);

		int packetResendLength = pageIndexArray[pagesToResend[counter]].stopIndex - pageIndexArray[pagesToResend[counter]].startIndex;

		Data_Length = packetResendLength;

		Checksum = HAL_CRC_Calculate(&hcrc,
				(uint32_t*) &Data_Ptr->Camera_Data.encoded[ pageIndexArray[counter].startIndex],
				(uint32_t) (Data_Length / 4));

		// kcs/Gage we need to stop using things inside of the big memory structure
		buf = 0;

		Build_MSG(Data_Ptr, MQTT_HEAD_MASK_START);
		utoa( UNIQUE_Device_ID, Header, 10);
		Build_MSG(Data_Ptr, Header);
		Build_MSG(Data_Ptr, MQTT_HEAD_MASK_END);
		Word_Transfer( Event_Type, MQTT_PIC_EVENT);
		Build_MSG(Data_Ptr, Event_Type);

		Word_Transfer( Event_Type, MQTT_PIC_picID_EVENT);
		Build_MSG(Data_Ptr, Event_Type);
		Build_MSG(Data_Ptr, Picture_ID);
		Build_MSG(Data_Ptr, MQTT_STRI);
		Build_MSG(Data_Ptr, MQTT_APND);

		Word_Transfer( Event_Type, MQTT_PIC_Page_EVENT);
		// kcs fix page
		utoa( pagesToResend[counter], Header, 10);
		Build_MSG(Data_Ptr, Event_Type);
		Build_MSG(Data_Ptr, Header);
		Build_MSG(Data_Ptr, MQTT_STRI);
		Build_MSG(Data_Ptr, MQTT_APND);

		Word_Transfer( Event_Type, MQTT_PIC_Total_Page_EVENT);
		utoa( Total_Page, Header, 10);
		Build_MSG(Data_Ptr, Event_Type);
		Build_MSG(Data_Ptr, Header);
		Build_MSG(Data_Ptr, MQTT_STRI);
		Build_MSG(Data_Ptr, MQTT_APND);

		Word_Transfer( Event_Type, MQTT_PIC_DATA_EVENT);
		Build_MSG(Data_Ptr, Event_Type);
		// kcs here is the problem setting buf back to 0
		// set beginning and end
		int packetStart = pageIndexArray[pagesToResend[counter]].startIndex;
		int memoryCounter;
		for(memoryCounter=0; memoryCounter <= packetResendLength; memoryCounter++)
		{
			Memory[buf++] = Data_Ptr->Camera_Data.encoded[packetStart++];
		}
		Build_MSG(Data_Ptr, MQTT_TAIL_MASK);
		Size = buf;

		// Potentially can be lower tnis 500 ms delay was needed for system stability
		HAL_Delay (500); // to add time between sending packets

		Command = PUBLISH;
		CELL_COMMAND(Data_Ptr);

		HAL_UART_AbortReceive(&hlpuart1);
		// Ideally these 2 lines would be after the printout, but we need for timing with receive mode of "0"
		Timer = UART_WAIT_TIME;
		HAL_UART_Transmit(&hlpuart1, Memory, Size, Timer);

#ifdef SKYWIRE_MODEM_OUT
		Write_Log( LOG_MSG_DATA);

		for (buf = 0; buf <= Size; buf += 1)
		{
			vcom_Trace(&Memory[ buf], 1, 1);
		}
		vcom_Trace((uint8_t*) "\r\n\r\n", 4, 1);
		if(0) //We don't know what this is, was breaking pic function
		{
			HAL_UART_AbortReceive(&hlpuart1);
			Timer = UART_WAIT_TIME;
			HAL_UART_Transmit(&hlpuart1, Memory, Size, Timer);
		}
		//These lines are to output the buffer size for each pic packet
		char Command_Buffer [ 10 ];
		utoa ( Size , Command_Buffer , DECIMAL );
		vcom_Trace((uint8_t*) LOG_PKT_DATA, 16, 1);
		vcom_Trace((uint8_t*) Command_Buffer, 4, 1);
		vcom_Trace((uint8_t*) "\r\n\r\n", 4, 1);

#endif //SKYWIRE_MODEM_OUT

		buf = 0;
		packetsSent++;
	}

	missingPagesCount = 0;

	if ( useRecvMode1 )
	{
		//		KCS Fix this
		getFirmwareDownlink(Data_Ptr, 5);
		if ( getMqttDataAvailable() )
		{
			universalDownlinkDecoder();
			selectDownlinkOperation(Data_Ptr, IMAGE_COMPLETE);
		}

		if ( missingPagesCount > 0 )
			resendMissingPackets(Data_Ptr, missingPagesCount);
	}

	return packetsSent;

}

void clearPicturePageIndex(void)
{
	int countArray;
	for(countArray=0; countArray <= 99; countArray++)
	{
		pageIndexArray[countArray].startIndex = 0;
		pageIndexArray[countArray].stopIndex = 0;
	}
}

void SetSystemTime ()
{
	RTC_TimeTypeDef sTime = {0};

	sTime.Hours = 0x0;
	sTime.Minutes = 0x0;
	sTime.Seconds = 0x0;
	sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
	sTime.StoreOperation = RTC_STOREOPERATION_RESET;
	if ( HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BCD) != HAL_OK )
		Error_Handler();
}

void SetSystemDate ()
{
	RTC_DateTypeDef sDate = {0};

	sDate.WeekDay = RTC_WEEKDAY_WEDNESDAY;
	sDate.Month = RTC_MONTH_AUGUST;
	sDate.Date = 0x23;
	sDate.Year = 0x23;
	if ( HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BCD) != HAL_OK)
		Error_Handler();
}

//GAV fill in this stub, Set date and then time
void SetSystemDateTime ()
{

}

void SetRTCAlarm(uint32_t Alarm, uint16_t seconds)
{
	RTC_AlarmTypeDef sAlarm = {0};
	sAlarm.AlarmTime.Hours = 0x0;
	sAlarm.AlarmTime.Minutes = 0x0;
	sAlarm.AlarmTime.Seconds = 0x0;
	sAlarm.AlarmTime.SubSeconds = 0x0;
	sAlarm.AlarmMask = RTC_ALARMMASK_DATEWEEKDAY|RTC_ALARMMASK_SECONDS;
	sAlarm.AlarmSubSecondMask = RTC_ALARMSUBSECONDMASK_ALL;
	sAlarm.AlarmDateWeekDaySel = RTC_ALARMDATEWEEKDAYSEL_DATE;
	sAlarm.AlarmDateWeekDay = 0x1;
	sAlarm.Alarm = RTC_ALARM_A;
	if (HAL_RTC_SetAlarm_IT(&hrtc, &sAlarm, RTC_FORMAT_BCD) != HAL_OK)
		Error_Handler();
}

void GetCellTime ( void )
{
	//	int CellDate =
}

//
//void receivePicAckUart ( MEM_PTR *Data_Ptr )
//{
//	//
//	Size = 330;
//	Timer = 100;
//	receivedDataSize = 0;
//	Command = CHECK3;
//	CELL_COMMAND(Data_Ptr);
//
//	vcom_Trace((uint8_t*) "DownLink Received\r\n", 19, 1);
//
//	for (buf = 0; buf <= receivedDataSize; buf += 1)
//	{
//		vcom_Trace(&Memory[buf], 1, 1);
//	}
//	vcom_Trace((uint8_t*) "\r\n", 2, 1);
//
//	if(!decodeImageAck(Data_Ptr))
//	{
//		if (missingPagesCount > 0)
//		{
//			resendMissingPackets(Data_Ptr);
//			Clear_Memory2(Data_Ptr, 400);
//			HAL_UART_Receive_IT ( &hlpuart1 , Memory , 1 );
//		}
//	}
//
//	// End of missing packets loop
//
//	//#ifdef Log_Level_2
//	//Write_Log( LOG_RX_ERR);
//	//Log_End( LOG_STATE_RESULT, Cell_State);
//	//	#endif
//}
//
// GAV description of function
// GAV every parameter and a description of each parameter
// returns an error false = pass, true = failed
//

//
////GAV built for testing, built to handle all downlinks (universal function)
//void RecvDownlink (MEM_PTR *Data_Ptr)
//{
//	int loopCounter = 0;
//	uint8_t goodData2 = 0;
//	Clear_Memory(Data_Ptr);
//	HAL_Delay(5000);  //added by GAV for server timer
//	Refresh_Watchdog;
//
//	do
//	{
//		HAL_Delay(5);  //removed by Gage
//		Command = CHECK;
//		CELL_COMMAND(Data_Ptr);
//		Command = READ_BUF;
//		CELL_COMMAND(Data_Ptr);
//		loopCounter ++;
//		if( loopCounter > 10 )
//		{
//			CELL_Init(Data_Ptr);
//			break;
//		}
//		goodData2 = isBufferPopulated2(Data_Ptr);
//
//	} while ( goodData2 == 0 );
//
//	receivedDataSize = 250;
//	if ( goodData2 == 1 )     //GAV enumerate
//	{
//		vcom_Trace((uint8_t*) "DownLink Received\r\n", 19, 1);
//
//		for ( buf = 0; buf <= receivedDataSize; buf += 1 )
//		{
//			vcom_Trace(&Memory[buf], 1, 1);
//		}
//		vcom_Trace((uint8_t*) "\r\n", 2, 1);
//
//		if( !decodeImageAck(Data_Ptr) )
//		{
//			if ( missingPagesCount > 0 )
//				resendMissingPackets(Data_Ptr);
//		}
//	}
//	else if ( goodData2 == 2 )
//	{
//		vcom_Trace((uint8_t*) "DownLink Received\r\n", 19, 1);
//
//		for ( buf = 0; buf <= receivedDataSize; buf += 1 )
//		{
//			vcom_Trace(&Memory[buf], 1, 1);
//		}
//		vcom_Trace((uint8_t*) "\r\n", 2, 1);
//	}
//}

//GAV built for testing in conjuction with RecvDownlink function
uint8_t isBufferPopulated2 ( MEM_PTR *Data_Ptr )
{
	char checkBuff [1000] = "";
	int counter = 0;
	for ( counter = 0; counter < 280; counter++ )   //GAv hardcoded to 40, need to be dynamic
	{
		checkBuff[counter]= Memory[counter];
	}

	char target1[] = "OK";
	char target2[] = "ERROR";
	char target3[] = "image_ack_v0";
	char target4[] = "config_v0";
	char target5[] = "placeholder";

	char *p = strstr( checkBuff, target1 );
	char *q = strstr( checkBuff, target2 );
	char *r = strstr( checkBuff, target3 );
	char *s = strstr( checkBuff, target4 );
	char *t = strstr( checkBuff, target5 );

	if (p)
	{
		vcom_Trace ( (uint8_t*) p , 280 , 280 );   //GAv hardcoded fix
		return 0;
	}
	else if (q)
	{
		vcom_Trace ( (uint8_t*) q , 280 , 280 );   //GAv hardcoded fix
		return 0;
	}
	else if ( checkBuff[40] == 0 )   //GAv fix this, hardcoded
	{
		return 0;
	}
	else if (r)
	{
		return 1;
	}
	else if (s)
	{
		vcom_Trace ( (uint8_t*) s , 280 , 280 );   //GAv hardcoded fix
		return 2;
	}
	else if (t)
	{
		vcom_Trace ( (uint8_t*) t , 280 , 280 );   //GAv hardcoded fix
		return 3;
	}
	else
		return 0;

	//check memory for "OK" if OK return false, if it's populated with something return true GAV
}

/**
 * @brief Copies modem messages into mQTTMessage buffer.
 * @note Copies messages from modem into mQTTMessage buffer unless the message is null.
 * @param char* ptr should be a pointer to a buffer that holds cell module response for number of messages in cell memory buffer ex 0,1,1,1,1,1)
 * @param MEM_PTR type Data_Ptr parameter used for cell commands
 * @retval void
 */
void RecvMessageNum(char* ptr,  MEM_PTR *Data_Ptr )
{
	char *RecvBuff[280] = {""};
	char Buff[280] = {""};
	bool printMQTT = false;

	Word_Transfer(Buff, ptr);
	RecvBuff[0] = Buff;

	int msgTokenCount = 0;
	char *msgTokens[7] = {0};
	const char *colDelim = ":";
	const char *comDelim = ",";

	char *msgToken = strtok_r(*RecvBuff, colDelim, RecvBuff);
	Refresh_Watchdog;

	while (msgToken != NULL && msgTokenCount < 7)
	{
		msgTokens[msgTokenCount] = msgToken;
		msgTokenCount++;

		msgToken = strtok_r(NULL, comDelim, RecvBuff);
	}

	//csc possible fix, if the message is null, assign it to a string. I think there was ststr issue
	//could also check each _msg for null before doing strtest but I wanted to see if this fixed it first

	char *msg1 = msgTokens[2] ? msgTokens[2] : "\0";
	char *msg2 = msgTokens[3] ? msgTokens[3] : "\0";
	char *msg3 = msgTokens[4] ? msgTokens[4] : "\0";
	char *msg4 = msgTokens[5] ? msgTokens[5] : "\0";
	char *msg5 = msgTokens[6] ? msgTokens[6] : "\0";

	char test1[] = "1";
	char *_msg1 = strstr( msg1, test1 );
	char *_msg2 = strstr( msg2, test1 );
	char *_msg3 = strstr( msg3, test1 );
	char *_msg4 = strstr( msg4, test1 );
	char *_msg5 = strstr( msg5, test1 );

	Clear_Memory(Data_Ptr);
	clearMqttStruct();

	//REFactoring code
	// make a for loop
	// change rcvr message 1 through 5 to just a single and modify to use a number
	//	for (int indx = 0; indx < 5; ++indx)
	//	{
	//		char *msg1 = msgTokens[indx+ 2] ? msgTokens[indx+ 2] : "\0";
	//		char *msgCheck = strstr( msg1, test1 );
	//
	//		if (msgCheck)
	//		{
	//			//vcom_Trace((uint8_t*) "Firmware DownLinks Received\r\n", 29, 1);
	//
	//			Clear_Recv_Buff(receivedData);
	//
	//			Command = RECEIVE_MSG;
	//			Data_Ptr->recIndex = indx;
	//			CELL_COMMAND(Data_Ptr);
	//			printMQTT = true;
	//			storeMqttData(indx, receivedData, Data_Ptr->UartBtyesReceived, printMQTT);
	//		}
	//	}

	if (_msg1)
	{
		Clear_Recv_Buff(receivedData);

		Command = recvMsg1;
		CELL_COMMAND(Data_Ptr);
		printMQTT = true;
		storeMqttData( 0, receivedData, Data_Ptr->UartBtyesReceived, printMQTT);
	}
	if (_msg2)
	{
		Clear_Recv_Buff(receivedData);

		Command = recvMsg2;
		CELL_COMMAND(Data_Ptr);
		printMQTT = true;
		storeMqttData( 1, receivedData, Data_Ptr->UartBtyesReceived, printMQTT);
	}

	if (_msg3)
	{
		Clear_Recv_Buff(receivedData);
		Command = recvMsg3;
		CELL_COMMAND(Data_Ptr);
		printMQTT = true;
		storeMqttData( 2, receivedData, Data_Ptr->UartBtyesReceived, printMQTT);
	}

	if (_msg4)
	{
		Clear_Recv_Buff(receivedData);
		Command = recvMsg4;
		CELL_COMMAND(Data_Ptr);
		printMQTT = true;
		storeMqttData( 3, receivedData, Data_Ptr->UartBtyesReceived, printMQTT);
	}

	if (_msg5)
	{
		Clear_Recv_Buff(receivedData);
		Command = recvMsg5;
		CELL_COMMAND(Data_Ptr);
		printMQTT = true;
		storeMqttData( 4, receivedData, Data_Ptr->UartBtyesReceived, printMQTT);
	}

}

void Clear_Recv_Buff ( uint8_t *recvData )
{
	int memoryItterator = 0;
	while ( memoryItterator < (MEMORY_MAX))
		recvData [ memoryItterator++ ] = 0;
}

//numOfTries must be between range 1-5
void getFirmwareDownlink (MEM_PTR *Data_Ptr, uint8_t numOfTries)
{
	int loopCounter = 0;
	bool goodData = false;
	Clear_Memory(Data_Ptr);

	do
	{
		//potential timing issue, put breakpoint below
		Refresh_Watchdog;
		Command = CHECK;
		CELL_COMMAND(Data_Ptr);
		Command = READ_FW_BUF_CHECK;  // checks how many messages are waiting
		PRINTF("Reading Buffer for FW Packets\r\n");
		CELL_COMMAND(Data_Ptr);

		PRINTF("Loop Counter is %d\r\n",loopCounter);

		//loopCounter ++;
		if( ++loopCounter > numOfTries )
		{
			PRINTF("Received 5 packets getting results\r\n");
			RecvMessageNum((char*)Memory, Data_Ptr);
			break;
		}

		HAL_Delay(3000);
		Refresh_Watchdog;
		goodData = isFivePacketsReceived((char*)Memory);
		PRINTF("Good Data is %d\r\n",goodData);

	} while ( !goodData );

	receivedDataSize = 40;
	if ( goodData )
	{
		vcom_Trace((uint8_t*) "Firmware DownLinks Received\r\n", 29, 1);

		vcom_Trace(&Memory[buf], receivedDataSize, 1);

		vcom_Trace((uint8_t*) "\r\n", 2, 1);
		RecvMessageNum((char*)Memory, Data_Ptr);
	}
}

// csc function to find missing pages in ota
void otaCheckMissingPages( OTA_FILE_TYPE *OTAData )
{
	// Reset missing pages data in the structure
	OTAData->numberOfMissingPages = 0;
	OTAData->missingPagesString[0] = '[';
	OTAData->missingPagesString[1] = '\0';
	OTAData->receivedPagesString[0] = '[';
	OTAData->receivedPagesString[1] = '\0';

	// Go through FirmwareArray and check for missing pages
	for (int i = 0; i < OTAData->numberOfPages; i++)
	{
		if ( OTAData->FirmwareArray[i][0] == '\0')
		{
			OTAData->numberOfMissingPages++;
			char page_str[6]; //4 may not be right depending on how many pages we could possible be missing
			// Append missing page number (i + 1) to missingPagesString
			snprintf(page_str, sizeof(page_str), "%d,", i + 1);
			strcat(OTAData->missingPagesString, page_str);
		}
		else
		{
			char page_str[6]; //4 may not be right depending on how many pages we could possible be missing
			// Append missing page number (i + 1) to missingPagesString
			snprintf(page_str, sizeof(page_str), "%d,", i + 1);
			strcat(OTAData->receivedPagesString, page_str);
		}
	}


	// Remove trailing comma and replace with closing bracket
	int len = strlen(OTAData->missingPagesString);
	if (len > 1 && OTAData->missingPagesString[len - 1] == ',')
	{
		OTAData->missingPagesString[len - 1] = ']';
	}
	else
	{
		strcat(OTAData->missingPagesString, "]");  // If no pages, just add closing bracket
	}
	len = strlen(OTAData->receivedPagesString);
	if (len > 1 && OTAData->receivedPagesString[len - 1] == ',')
	{
		OTAData->receivedPagesString[len - 1] = ']';
	}
	else
	{
		strcat(OTAData->receivedPagesString, "]");  // If no pages, just add closing bracket
	}
}

void clearFirmwareArray(OTA_FILE_TYPE *OTAData)
{
	for(int i = 0; i < MAXPAGES; i++)
	{
		for(int j = 0; j < MEMORY_MAX; j++)
		{
			OTAData->FirmwareArray[i][j] = '\0';
		}
	}
}

void fakeMissing(OTA_FILE_TYPE *OTAData)
{
	for(int i = 0; i < 80; i++)
	{
		if((i%2) == 0)
		{
			memset(OTAData->FirmwareArray[i], '\0', MEMORY_MAX);
		}
		else
			continue;
	}
}

// returns CRC
uint32_t FWDecodeFile( OTA_FILE_TYPE *OTAData, MEM_PTR *bigData )
{
	uint8_t packageNumber;
	uint32_t crcCalculated = 0;
	int pckSize;

	unsigned char tempOutput [MEMORY_MAX] = {""}; // temp until we test and store into active page

	for(packageNumber = 0; packageNumber < OTAData->numberOfPages; packageNumber++)
	{
		PRINTF("Processing Package Number %d\r\n", packageNumber+1);
		pckSize = OTAData->dataFWLength[packageNumber];
		PRINTF("Package Size %d\r\n", pckSize);

		if (pckSize > 1)
		{
			// and call onTheFlyDcodeBase64
			int numberOfconversions = onTheFlyDcodeBase64( OTAData->FirmwareArray[packageNumber], tempOutput, pckSize);
			PRINTF("Number of Conversions %d\r\n", numberOfconversions);
			// call CRC functions, 1st on in 'calculate' all the rest are 'accumulate'
			if (packageNumber == 0)
			{
				crcCalculated = HAL_CRC_Calculate ( &hcrc , (uint32_t*) tempOutput , numberOfconversions );
			}
			else
			{
				crcCalculated = HAL_CRC_Accumulate ( &hcrc , (uint32_t*) tempOutput , numberOfconversions );
			}

			Refresh_Watchdog;
			// store that binary into "Page1" ROM
			// move tempOutput into Data_Ptr->Flash.Image @ current position of Data_Ptr->Flash.Index
			//PRINTF("memcopy\r\n");
			memcpy(&bigData->Flash.Image[bigData->Flash.Index], tempOutput, numberOfconversions);
			// keep a last ROM address parameter
			// increment Data_Ptr->Flash.Index
			bigData->Flash.Index += numberOfconversions;
			OTAData->lastROMAddress = bigData->Flash.Index;
			// clear tempOutput

			for(int junk = 0; junk < OTAData->dataFWLength[packageNumber]; junk++)
			{
				tempOutput[junk] = '\0';
			}
		}
		else
		{
			if (packageNumber <= OTAData->numberOfPages)
			{
				// log error
				PRINTF("Number of packages did not match\r\n");
				crcCalculated = 0;
				break;
			}
		}
	}
	return (crcCalculated);
}

void setupTestPackages(OTA_FILE_TYPE *OTAData)
{
	clearFirmwareArray(OTAData);
	char pack1[] = "U2Vuc2l0aXZpdHkgZGVzY3JpYmVzIHRoZSBnYWluIG9mIHRoZSBzZW5zb3IgYW5kIGNhbiBiZSBkZXRlcm1pbmVkIGJ5IGFwcGx5aW5nIDEgZyBhY2NlbGVyYXRpb24gdG8gaXQuIEFzIHRoZSBzZW5zb3IgY2FuIG1lYXN1cmUgREMgYWNjZWxlcmF0aW9ucyB0aGlzIGNhbiBiZSBkb25lIGVhc2lseSBieSBwb2ludGluZyB0aGUgYXhpcyBvZiBpbnRlcmVzdCB0b3dhcmRzIHRoZSBjZW50ZXIgb2YgdGhlIEVhcnRoLCBub3RpbmcgdGhlIG91dHB1dCB2YWx1ZSwgcm90YXRpbmcgdGhlIHNlbnNvciBieSAxODAgZGVncmVlcyAocG9p";
	char pack2[] = "bnRpbmcgdG8gdGhlIHNreSkgYW5kIG5vdGluZyB0aGUgb3V0cHV0IHZhbHVlIGFnYWluLiBCeSBkb2luZyBzbywgwrExIGcgYWNjZWxlcmF0aW9uIGlzIGFwcGxpZWQgdG8gdGhlIHNlbnNvci4gU3VidHJhY3RpbmcgdGhlIGxhcmdlciBvdXRwdXQgdmFsdWUgZnJvbSB0aGUgc21hbGxlciBvbmUsIGFuZCBkaXZpZGluZyB0aGUgcmVzdWx0IGJ5IDIsIGxlYWRzIHRvIHRoZSBhY3R1YWwgc2Vuc2l0aXZpdHkgb2YgdGhlIHNlbnNvci4gVGhpcyB2YWx1ZSBjaGFuZ2VzIHZlcnkgbGl0dGxlIG92ZXIgdGVtcGVyYXR1cmUgYW5kIHRp";
	char pack3[] = "bWUuIFRoZSBzZW5zaXRpdml0eSB0b2xlcmFuY2UgZGVzY3JpYmVzIHRoZSByYW5nZSBvZiBzZW5zaXRpdml0aWVzIG9mIGEgbGFyZ2UgcG9wdWxhdGlvbiBvZiBzZW5zb3JzLg==";
	OTAData->dataFWLength[0]= 400;
	OTAData->dataFWLength[1]= 400;
	OTAData->dataFWLength[2]= 136;
	// copy pack into firmware array
	strncpy((char*)OTAData->FirmwareArray[0], pack1, OTAData->dataFWLength[0]);
	strncpy((char*)OTAData->FirmwareArray[1], pack2, OTAData->dataFWLength[1]);
	strncpy((char*)OTAData->FirmwareArray[2], pack3, OTAData->dataFWLength[2]);
}

void setupTestMqtt(void)
{
	// length is 276
	char recCode [] = "AT+QMTRECV=0\r\n+QMTRECV: 0,6,\"sensors/mobile_device/2228293/downlink\",\"{\"dev_id\":\"2228293\",\"timestamp\":\"2024-11-14T01:58:56.736Z\",\"type\":\"firmware_v0\",\"version\":\"XPM_V0.0.3\",\"file\":1,\"page\":5,\"total_pages\":38,\"data\":\"8EEDsHBHh/MQiE/0enMOqQNIDfDA+u/nuKoKICpiAQhEqwogLenwQQAkBkat\",\"crc\":1988221081}";

	//	char pack1[] = "+QMTRECV: 0,2,\"sensors/mobile_device/2228293/downlink\",\"{\"dev_id\":\"2228293\",\"type\":\"firmware_v0\",\"version\":\"stringTest2\",\"file\":1,\"page\":1,\"total_pages\":1,\"data\":\"U2Vuc2l0aXZpdHkgZGVzY3JpYmVzIHRoZSBnYWluIG9mIHRoZSBzZW5zb3IgYW5kIGNhbiBiZSBkZXRlcm1pbmVkIGJ5IGFwcGx5aW5nIDEgZyBhY2NlbGVyYXRpb24gdG8gaXQuIEFzIHRoZSBzZW5zb3IgY2FuIG1lYXN1cmUgREMgYWNjZWxlcmF0aW9ucyB0aGlzIGNhbiBiZSBkb25lIGVhc2lseSBieSBwb2ludGluZyB0aGUgYXhpcyBvZiBpbnRlcmVzdCB0b3dhcmRzIHRoZSBjZW50ZXIgb2YgdGhlIEVhcnRoLCBub3RpbmcgdGhlIG91dHB1dCB2YWx1ZSwgcm90YXRpbmcgdGhlIHNlbnNvciBieSAxODAgZGVncmVlcyAocG9pbnRpbmcgdG8gdGhlIHNreSkgYW5kIG5vdGluZyB0aGUgb3V0cHV0IHZhbHVlIGFnYWluLiBCeSBkb2luZyBzbywgwrExIGcgYWNjZWxlcmF0aW9uIGlzIGFwcGxpZWQgdG8gdGhlIHNlbnNvci4gU3VidHJhY3RpbmcgdGhlIGxhcmdlciBvdXRwdXQgdmFsdWUgZnJvbSB0aGUgc21hbGxlciBvbmUsIGFuZCBkaXZpZGluZyB0aGUgcmVzdWx0IGJ5IDIsIGxlYWRzIHRvIHRoZSBhY3R1YWwgc2Vuc2l0aXZpdHkgb2YgdGhlIHNlbnNvci4gVGhpcyB2YWx1ZSBjaGFuZ2VzIHZlcnkgbGl0dGxlIG92ZXIgdGVtcGVyYXR1cmUgYW5kIHRpbWUuIFRoZSBzZW5zaXRpdml0eSB0b2xlcmFuY2UgZGVzY3JpYmVzIHRoZSByYW5nZSBvZiBzZW5zaXRpdml0aWVzIG9mIGEgbGFyZ2UgcG9wdWxhdGlvbiBvZiBzZW5zb3JzLlNlbnNpdGl2aXR5IGRlc2NyaWJlcyB0aGUgZ2FpbiBvZiB0aGUgc2Vuc29yIGFuZCBjYW4gYmUgZGV0ZXJtaW5lZCBieSBhcHBseWluZyAxIGcgYWNjZWxlcmF0aW9uIHRvIGl0LiBBcyB0aGUgc2Vuc29yIGNhbiBtZWFzdXJlIERDIGFjY2VsZXJhdGlvbnMgdGhpcyBjYW4gYmUgZG9uZSBlYXNpbHkgYnkgcG9pbnRpbmcgdGhlIGF4aXMgb2YgaW50ZXJlc3QgdG93YXJkcyB0aGUgY2VudGVyIG9mIHRoZSBFYXJ0aCwgbm90aW5nIHRoZSBvdXRwdXQgdmFsdWUsIHJvdGF0aW5nIHRoZSBzZW5zb3IgYnkgMTgwIGRlZ3JlZXMgKHBvaW50aW5nIHRvIHRoZSBza3kpIGFuZCBub3RpbmcgdGhlIG91dHB1dCB2YWx1ZSBhZ2Fpbi4gQnkgZG9pbmcgc28sIMKxMSBnIGFjY2VsZXJhdGlvbiBpcyBhcHBsaWVkIHRvIHRoZSBzZW5zb3IuIFN1YnRyYWN0aW5nIHRoZSBsYXJnZXIgb3V0cHV0IHZhbHVlIGZyb20gdGhlIHNtYWxsZXIgb25lLCBhbmQgZGl2aWRpbmcgdGhlIHJlc3VsdCBieSAyLCBsZWFkcyB0byB0aGUgYWN0dWFsIHNlbnNpdGl2aXR5IG9mIHRoZSBzZW5zb3IuIFRoaXMgdmFsdWUgY2hhbmdlcyB2ZXJ5IGxpdHRsZSBvdmVyIHRlbXBlcmF0dXJlIGFuZCB0aW1lLiBUaGUgc2Vuc2l0aXZpdHkgdG9sZXJhbmNlIGRlc2NyaWJlcyB0aGUgcmFuZ2Ugb2Ygc2Vuc2l0aXZpdGllcyBvZiBhIGxhcmdlIHBvcHVsYXRpb24gb2Ygc2Vuc29ycy5TZW5zaXRpdml0eSBkZXNjcmliZXMgdGhlIGdhaW4gb2YgdGhlIHNlbnNvciBhbmQgY2FuIGJlIGRldGVybWluZWQgYnkgYXBwbHlpbmcgMSBnIGFjY2VsZXJhdGlvbiB0byBpdC4gQXMgdGhlIHNlbnNvciBjYW4gbWVhc3VyZSBEQyBhY2NlbGVyYXRpb25zIHRoaXMgY2FuIGJlIGRvbmUgZWFzaWx5IGJ5IHBvaW50aW5nIHRoZSBheGlzIG9mIGludGVyZXN0IHRvd2FyZHMgdGhlIGNlbnRlciBvZiB0aGUgRWFydGgsIG5vdGluZyB0aGUgb3V0cHV0IHZhbHVlLCByb3RhdGluZyB0aGUgc2Vuc29yIGJ5IDE4MCBkZWdyZWVzIChwb2ludGluZyB0byB0aGUgc2t5KSBhbmQgbm90aW5nIHRoZSBvdXRwdXQgdmFsdWUgYWdhaW4uIEJ5IGRvaW5nIHNvLCDCsTEgZyBhY2NlbGVyYXRpb24gaXMgYXBwbGllZCB0byB0aGUgc2Vuc29yLiBTdWJ0cmFjdGluZyB0aGUgbGFyZ2VyIG91dHB1dCB2YWx1ZSBmcm9tIHRoZSBzbWFsbGVyIG9uZSwgYW5kIGRpdmlkaW5nIHRoZSByZXN1bHQgYnkgMiwgbGVhZHMgdG8gdGhlIGFjdHVhbCBzZW5zaXRpdml0eSBvZiB0aGUgc2Vuc29yLiBUaGlzIHZhbHVlIGNoYW5nZXMgdmVyeSBsaXR0bGUgb3ZlciB0ZW1wZXJhdHVyZSBhbmQgdGltZS4gVGhlIHNlbnNpdGl2aXR5IHRvbGVyYW5jZSBkZXNjcmliZXMgdGhlIHJhbmdlIG9mIHNlbnNpdGl2aXRpZXMgb2YgYSBsYXJnZSBwb3B1bGF0aW9uIG9mIHNlbnNvcnMuU2Vuc2l0aXZpdHkgZGVzY3JpYmVzIHRoZSBnYWluIG9mIHRoZSBzZW5zb3IgYW5kIGNhbiBiZSBkZXRlcm1pbmVkIGJ5IGFwcGx5aW5nIDEgZyBhY2NlbGVyYXRpb24gdG8gaXQuIEFzIHRoZSBzZW5zb3IgY2FuIG1lYXN1cmUgREMgYWNjZWxlcmF0aW9ucyB0aGlzIGNhbiBiZSBkb25lIGVhc2lseSBieSBwb2ludGluZyB0aGUgYXhpcyBvZiBpbnRlcmVzdCB0b3dhcmRzIHRoZSBjZW50ZXIgb2YgdGhlIEVhcnRoLCBub3RpbmcgdGhlIG91dHB1dCB2YWx1ZSwgcm90YXRpbmcgdGhlIHNlbnNvciBieSAxODAgZGVncmVlcyAocG9pbnRpbmcgdG8gdGhlIHNreSkgYW5kIG5vdGluZyB0aGUgb3V0cHV0IHZhbHVlIGFnYWluLiBCeSBkb2luZyBzbywgwrExIGcgYWNjZWxlcmF0aW9uIGlzIGFwcGxpZWQgdG8gdGhlIHNlbnNvci4gU3VidHJhY3RpbmcgdGhlIGxhcmdlciBvdXRwdXQgdmFsdWUgZnJv\",\"crc\":3949143094}";
	//	char pack2[] = "bnRpbmcgdG8gdGhlIHNreSkgYW5kIG5vdGluZyB0aGUgb3V0cHV0IHZhbHVlIGFnYWluLiBCeSBkb2luZyBzbywgwrExIGcgYWNjZWxlcmF0aW9uIGlzIGFwcGxpZWQgdG8gdGhlIHNlbnNvci4gU3VidHJhY3RpbmcgdGhlIGxhcmdlciBvdXRwdXQgdmFsdWUgZnJvbSB0aGUgc21hbGxlciBvbmUsIGFuZCBkaXZpZGluZyB0aGUgcmVzdWx0IGJ5IDIsIGxlYWRzIHRvIHRoZSBhY3R1YWwgc2Vuc2l0aXZpdHkgb2YgdGhlIHNlbnNvci4gVGhpcyB2YWx1ZSBjaGFuZ2VzIHZlcnkgbGl0dGxlIG92ZXIgdGVtcGVyYXR1cmUgYW5kIHRp";
	//	char pack3[] = "bWUuIFRoZSBzZW5zaXRpdml0eSB0b2xlcmFuY2UgZGVzY3JpYmVzIHRoZSByYW5nZSBvZiBzZW5zaXRpdml0aWVzIG9mIGEgbGFyZ2UgcG9wdWxhdGlvbiBvZiBzZW5zb3JzLg==";
	//int p1Length = strlen(pack1);
	int p1Length = strlen(recCode);
	storeMqttData(0, (uint8_t*)recCode,p1Length, true);
	// copy pack into mqtt array
	//	for (int i = 0; i < p1Length; i ++)
	//	{
	//		MQTTMsg[0][i] = recCode[i];//pack1[i];
	//	}
	int debug = 1;
	debug++;
}

void Update_Battery (void)
{
	__HAL_RCC_ADC1_CLK_ENABLE();
	__HAL_RCC_PWR_CLK_ENABLE();
	//	  MX_ADC1_Init();
	HAL_ADC_Start(&hadc1);
	HAL_Delay(1000);
	if(HAL_ADC_PollForConversion(&hadc1, 100) == HAL_OK)
	{
		uint32_t adc_value = HAL_ADC_GetValue(&hadc1);
		float vbat = -0.00292 * adc_value +11.48;
		sprintf(batVal, "%.3f", vbat);
		HAL_ADC_Stop(&hadc1);
	}
}


uint8_t cellResponseCheck (MEM_PTR *Data_Ptr)
{
	PRINTF("Checking Cell Response\r\n");

	retry = false;

	uint8_t isOK = 99;
	switch ( Command )
	{
	case OPEN:
		char openTest[] = "+QMTOPEN:";
		char* openStr = strstr((char*)Memory, openTest);

		if (openStr)   //GAV add full urc
		{
			openStr += (strlen(openTest) + 3);
			char checkBit = openStr[0];

			if (checkBit == '0')
			{
				isOK = 1;
				PRINTF("Response Validated, Status code is %d\r\n",isOK);
			}
			else if (checkBit == '2')
			{
				isOK = 1;
				PRINTF("Response Validated, Status code is %d\r\n",isOK);
			}
			else if (checkBit == '3')  //GAV look into other errors
			{
				isOK = 3;
				PRINTF("PDP not Active, Status code is %d\r\n",isOK);
				Command = activatePDP;
				CELL_COMMAND(Data_Ptr);
				if (cellResponseCheck(Data_Ptr) != 1)
				{

					Command = REENABLEQI;
					CELL_COMMAND( Data_Ptr );
				}
			}
		}

		PRINTF("Status code is %d\r\n",isOK);

		break;

	case CONNECT:
		char connTest[] = "+QMTCONN:";
		char* connStr = strstr((char*)Memory, connTest);

		if(connStr)
		{
			connStr += (strlen(connTest) + 5);
			char checkBit = connStr[0];

			if (checkBit == '0')
			{
				isOK = 1;
				PRINTF("Response Validated, Status code is %d\r\n",isOK);
			}
			else
			{
				isOK = 0;
				PRINTF("Response Error, Status code is %d\r\n",isOK);
			}
		}

		PRINTF("Status code is %d\r\n",isOK);

		break;

	case SUBSCRIBEDOWN:

		char subTest[] = "+QMTSUB:";
		char* subStr = strstr((char*)Memory, subTest);

		if(subStr)
		{
			subStr += (strlen(subTest) + 5);
			char checkBit = subStr[0];

			if (checkBit == '0')
			{
				isOK = 1;
				PRINTF("Response Validated, Status code is %d\r\n",isOK);
			}
			else
			{
				isOK = 0;
				PRINTF("Response Error, Status code is %d\r\n",isOK);
				retry = true;
			}
		}

		PRINTF("Status code is %d\r\n",isOK);
		if (isOK != 1)
		{
			retry = true;
		}

		break;

	case activatePDP:

		char actPdpTest[] = "OK";
		char errTest[] = "+CME ERROR: 30";
		char* actPdpStr = strstr((char*)Memory, actPdpTest);
		char* errStr = strstr((char*)Memory, errTest);

		if(actPdpStr)
		{

			isOK = 1;
			PRINTF("Response Validated, Status code is %d\r\n",isOK);

		}
		else if (errStr)
		{
			isOK = 3;
			PRINTF("PDP not Active, Status code is %d\r\n",isOK);

			Command = PDP_POPULATE;
			CELL_COMMAND(Data_Ptr);
		}
		else
		{
			isOK = 0;
			PRINTF("Response Error, Status code is %d\r\n",isOK);
		}


		PRINTF("Status code is %d\r\n",isOK);

		break;

	case REENABLEQI:


		char reEnablePdpTest[] = "OK";
		char* reEnablePdpStr = strstr((char*)Memory, reEnablePdpTest);

		if(reEnablePdpStr)
		{
			isOK = 1;
			PRINTF("Response Validated, Status code is %d\r\n",isOK);
		}
		else
		{
			isOK = 0;
			PRINTF("Response Error, Status code is %d\r\n",isOK);
		}

		PRINTF("Status code is %d\r\n",isOK);

		break;

	case GPS_ACTIVE_ANTENNA_ENABLE:

		char activeAntennaEnableTest[] = "OK";
		char* activeAntennaEnableTestStr = strstr((char*)Memory, activeAntennaEnableTest);

		if(activeAntennaEnableTestStr)
		{
			isOK = 1;
			PRINTF("Response Validated, Status code is %d\r\n",isOK);
		}
		else
		{
			isOK = 0;
			PRINTF("Response Error, Status code is %d\r\n",isOK);
		}

		PRINTF("Status code is %d\r\n",isOK);

		break;

	case GPS_ACTIVE_ANTENNA_SET:

		char activeAntennaSetTest[] = "OK";
		char* activeAntennaEnableSetStr = strstr((char*)Memory, activeAntennaSetTest);

		if(activeAntennaEnableSetStr)
		{
			isOK = 1;
			PRINTF("Response Validated, Status code is %d\r\n",isOK);
		}
		else
		{
			isOK = 0;
			PRINTF("Response Error, Status code is %d\r\n",isOK);
		}

		PRINTF("Status code is %d\r\n",isOK);

		break;

	case XTRA_ENABLE:

		char xtraEnableTest[] = "OK";
		char* xtraEnableStr = strstr((char*)Memory, xtraEnableTest);

		if(xtraEnableStr)
		{
			isOK = 1;
			PRINTF("Response Validated, Status code is %d\r\n",isOK);
		}
		else
		{
			isOK = 0;
			PRINTF("Response Error, Status code is %d\r\n",isOK);
		}

		PRINTF("Status code is %d\r\n",isOK);

		break;

	case XTRA_AUTODL_ENABLE:

		char extraAutoDlTest[] = "OK";
		char* extraAutoDlStr = strstr((char*)Memory, extraAutoDlTest);

		if(extraAutoDlStr)
		{
			isOK = 1;
			PRINTF("Response Validated, Status code is %d\r\n",isOK);
		}
		else
		{
			isOK = 0;
			PRINTF("Response Error, Status code is %d\r\n",isOK);
		}

		PRINTF("Status code is %d\r\n",isOK);

		break;

	case CELLID:

		char cellIDTest[] = "+CREG:0,1";
		char* cellIDStr = strstr((char*)Memory, cellIDTest);

		if(cellIDStr)
		{
			isOK = 1;
			PRINTF("Response Validated, Status code is %d\r\n",isOK);
		}
		else
		{
			isOK = 0;
			PRINTF("Response Error, Status code is %d\r\n",isOK);
		}

		PRINTF("Status code is %d\r\n",isOK);

		break;

	case XTRA_TIME_QUERY:

		char xtraTimeTest[] = "+QGPSXTRATIME:";
		char* xtraTimeStr = strstr((char*)Memory, xtraTimeTest);

		if(xtraTimeStr)
		{
			isOK = 1;
			PRINTF("Response Validated, Status code is %d\r\n",isOK);
		}
		else
		{
			isOK = 0;
			PRINTF("Response Error, Status code is %d\r\n",isOK);
		}

		PRINTF("Status code is %d\r\n",isOK);

		break;

	case GPS_ENABLE:

		char xtraDownloadTest[] = "XTRA_DL\",0";
		char* xtraDownloadStr = strstr((char*)Memory, xtraDownloadTest);

		if(xtraDownloadStr)
		{
			isOK = 1;
			PRINTF("Response Validated, Status code is %d\r\n",isOK);
		}
		else
		{
			isOK = 0;
			PRINTF("Response Error, Status code is %d\r\n",isOK);
		}

		PRINTF("Status code is %d\r\n",isOK);

		break;

	default:

		PRINTF("Error in Cell Response Check.\r\n");
	}
	return isOK;
}

//CSC function to send configuration data to server
void sendDeviceConfig( MEM_PTR *Data_Ptr, STATUS_UPLINK_TYPE uplnkType )
{
	char configBuff[MEMORY_MAX] = {0};
	char timebuff[ISO_TIMESTAMP_LENGTH] = {""}; // for time stamp
	char responseTypeBuff[50] = {""};  // temp buffer for type of device configuration Boot or Configured
	int buffSize = 0;

	Time_StampISO(timebuff); //Get time stamp in ISO format

	if(uplnkType == BOOT)
	{
		int len = strlen(BOOT_MSG) + 1;
		strncpy(responseTypeBuff, BOOT_MSG, len);
	}
	else if(uplnkType == CONFIG_ACK)
	{
		int len = strlen(CONFIG_MSG) + 1;
		strncpy(responseTypeBuff, CONFIG_MSG, len);
	}
	else
	{
		PRINTF("Invalid response type\r\n\n");
	}

	//NOTE: This is mostly junk data, but the specifiers should be correct so we can fill it in later.
	//Just need to change arguments.

	if(responseTypeBuff[0] != '\0')
	{
		//CSC snprintf for devID,type,timestamp,response type,firmware,heartbeat, and day night
		buffSize = snprintf(configBuff, MEMORY_MAX, "\1{\"dev_id\":\"%lu\",\"type\":\"%s\",\"timestamp\":\"%s\",\"response_type\":\"%s\",\"firmware_version\":\"%s\",\r\n\"heartbeat\":{\"version\":%u,\"mode\":%u,\"hb_interval\":%u},\r\n%s",
				UNIQUE_Device_ID, "device_status", timebuff, responseTypeBuff, "XPM_Rev3_V1.0.1", 0, Data_Ptr->heartBeatData.mode, Data_Ptr->heartBeatData.hbInterval, dayNightConfigStr());

		//CSC snprintf for pir, motion filter day, and motion filter night
		buffSize += snprintf((configBuff + buffSize), (MEMORY_MAX - buffSize), ",\r\n%s", getPirConfigStr());

		//CSC snprintf for buzzer and environment information
		buffSize += snprintf((configBuff + buffSize), (MEMORY_MAX - buffSize), ",\r\n\"buzzer\":{\"version\":%u,\"mode\":%u,\"start_motion_cycles\":%u,\"stop_motion_cycles\":%u},\r\n%s,",
				0, 1, 10, 10, getEnvironmentConfigStr());

		//CSC snprintf for temp/humidity and accelerometer data.
		buffSize += snprintf((configBuff + buffSize), (MEMORY_MAX - buffSize), "\r\n%s,\r\n%s,\r\n\%s",
				getTemperatureConfigStr(), getHumidityConfigStr(), getAccelConfigStr());

		//CSC snprintf for gps and camera
		buffSize += snprintf((configBuff + buffSize), (MEMORY_MAX - buffSize), ",\r\n%s,\r\n%s,",
				getGpsConfigStr(), getCamConfigStr());


		//CSC snprintf for cell and device status
		buffSize += snprintf((configBuff + buffSize), (MEMORY_MAX - buffSize), "\r\n\"cell\":{\"version\":%u,\"mode\":%u,\"warm_up\":%u,\"listen_period\":%u,\"listen_max_time\":%u},\"device\":{\"version\":%u,\"mode\":%u,\"log_level\":%u}}",
				1, 1, 10, 20, 30, 1, 2, 1);
	}

	if(buffSize > 0 && buffSize < MEMORY_MAX && configBuff[0] != '\0')
	{
		//		Uncomment to debug message, modem logs messages upon transmition also
		//		PRINTF("%s\r\n", configBuff);
		//		PRINTF("Size: %d\r\n", buffSize);
		Clear_Memory(Data_Ptr);
		Build_MSG(Data_Ptr, configBuff);
		Size = buf;
		Command = PUBLISH;
		CELL_COMMAND(Data_Ptr);
		Command = PUBLISH_MEM;
		CELL_COMMAND(Data_Ptr);
		Command = CHECK;
		CELL_COMMAND(Data_Ptr);
	}
	else
	{
		PRINTF("Error building device status message\r\n");
	}
}

void sendDiagnostic(MEM_PTR *Data_Ptr, char* message)
{
	char configBuff[MEMORY_MAX] = {""};
	char timebuff[ISO_TIMESTAMP_LENGTH] = {""};
	int buffSize = 0;

	Time_StampISO(timebuff); //Get time stamp in ISO format
//{"dev_id":"2228293","type":"diagnostic","timestamp":"2023-08-31T23:02:08.255Z","gps":["time_sync_failed, unknown_location"]}
	buffSize = snprintf(configBuff, MEMORY_MAX, "\1{\"dev_id\":\"%lu\",\"type\":\"diagnostic\",\"timestamp\":\"%s\",%s}",
			UNIQUE_Device_ID, timebuff, message);

	if(buffSize > 0 && buffSize < MEMORY_MAX && configBuff[0] != '\0')
	{
		Clear_Memory(Data_Ptr);
		Build_MSG(Data_Ptr, configBuff);
		Size = buf;
		Command = PUBLISH;
		CELL_COMMAND(Data_Ptr);
		Command = PUBLISH_MEM;
		CELL_COMMAND(Data_Ptr);
		Command = CHECK;
		CELL_COMMAND(Data_Ptr);
	}
	else
	{
		PRINTF("Error building device status message.\r\n");
	}
}

bool getServerTime(uint8_t *mqttMsg)
{
	char test[] = "\"timestamp\":\"";
	char Buff[MEMORY_MAX] = "";
	bool isError = false;
	uint16_t year = 0;
	uint8_t month = 255;
	uint8_t day = 0;
	uint8_t hour = 255;
	uint8_t minute = 255;
	uint8_t second = 255;
	uint32_t fraction = 4294967295;

	Word_Transfer(Buff, (char*)mqttMsg);

	char *dateStr = strstr(Buff, test);
	if(dateStr)
	{
		dateStr += strlen(test);
		if(isdigit((unsigned char)dateStr[0]))
		{
			year = (atoi(dateStr) % 2000);
		}
		else
		{
			isError = true;
		}

		int lenToMonth = strcspn(dateStr, "-");
		dateStr += lenToMonth + 1;
		if(!isError && isdigit((unsigned char)dateStr[0]))
		{
			month = atoi(dateStr);
		}
		else
		{
			isError = true;
		}

		int lenToDay = strcspn(dateStr, "-");
		dateStr += lenToDay + 1;
		if(!isError && isdigit((unsigned char)dateStr[0]))
		{
			day = atoi(dateStr);
		}
		else
		{
			isError = true;
		}

		int lenToHour = strcspn(dateStr, "T");
		dateStr += lenToHour + 1;
		if(!isError && isdigit((unsigned char)dateStr[0]))
		{
			hour = atoi(dateStr);
		}
		else
		{
			isError = true;
		}

		int lenToMin = strcspn(dateStr, ":");
		dateStr += lenToMin + 1;
		if(!isError && isdigit((unsigned char)dateStr[0]))
		{
			minute = atoi(dateStr);
		}
		else
		{
			isError = true;
		}

		int lenToSec = strcspn(dateStr, ":");
		dateStr += lenToSec + 1;
		if(!isError && isdigit((unsigned char)dateStr[0]))
		{
			second = atoi(dateStr);
		}
		else
		{
			isError = true;
		}

		int lenToFrac = strcspn(dateStr, ".");
		dateStr += lenToFrac + 1;
		if(!isError && isdigit((unsigned char)dateStr[0]))
		{
			fraction = atoi(dateStr);
		}
		else
		{
			isError = true;
		}

	}
	else
	{
		PRINTF("Failed to decode time stamp message.\r\n");
	}

	if(!isError)
	{
		RTC_TimeTypeDef Server_Time = {0};
		RTC_DateTypeDef Server_Date = {0};
		HAL_RTC_GetTime(&hrtc, &Server_Time, FORMAT_BIN);
		HAL_RTC_GetDate(&hrtc, &Server_Date, FORMAT_BIN);

		Server_Date.Date = day;
		Server_Date.Month = month;
		Server_Date.Year = year;
		Server_Time.Hours = hour;
		Server_Time.Minutes = minute;
		Server_Time.Seconds = second;
		Server_Time.SecondFraction = fraction;
		if(HAL_RTC_SetDate(&hrtc, &Server_Date, FORMAT_BIN) != HAL_OK)
		{
			Error_Handler();
			isError = true;
		}

		if(HAL_RTC_SetTime(&hrtc, &Server_Time, FORMAT_BIN) != HAL_OK)
		{
			Error_Handler();
			isError = true;
		}

		HAL_RTC_GetTime(&hrtc, &Server_Time, FORMAT_BIN);
		HAL_RTC_GetDate(&hrtc, &Server_Date, FORMAT_BIN);
	}

	return isError;
}
#endif // SKYWIRE_C_



