/**
 ******************************************************************************
 * @file    Skywire.h
 * @author  Erik Pineda-A
 * @brief
 ******************************************************************************
 * @attention
 *
 * Copyright 2024 Team of Defenders
 *
 ******************************************************************************
 */
#ifndef SKYWIRE_H_
#define SKYWIRE_H_

/******************************************************
 File Includes
 Note: n/a
 ******************************************************/

#include "Functions.h"



/******************************************************
 Define Values
 Note: Values Gathered from Datasheet
 Instruction Structure:
 (Size of command without"AT")(Space)(Command)
 ******************************************************/

// #define CELL_TEST_TEXT "AB CE"
/* MAX : 0xFFFFFFFF , set 0x2 for debug*/
#define Header_Size 15

#define PIC_Header_Version 0b01
#define PIC_Header_Service_Send 0b01
#define PIC_Header_Service_Retry 0b10

#define UART_WAIT_TIME 0x0002FF			// Timer for UART message
#define UART_WAIT_TIME_INIT 0x2FF	// Timer for UART message
#define UART_EXTEND_TIME 0x002FF		// Timer for UART message

#define TEMP_LIMIT 5
#define CSQ_MAX_FAIL 10				// Timer for UART message
#define UART_MAX_FAIL 3				// Timer for UART message
#define UART_FAIL_MAX 3				// Timer for UART message

#define MSG_Delay 100

#define UART_MAX_SIZE 2500
#define SIGSTRNGT_SIZE 100
#define SIM_ID_SIZE 100
#define MODEM_ID_SIZE 100
#define CELL_ID_SIZE 500
#define GPS_Timeout 10

//#define PIC_DATA_LIM 2000

//#define MQTT_Dev_ID "30-34-34-35-80-39-73-0F"
#define MQTT_HEAD "\1AT\0"
////#define MQTT_MSG_OBJ_HEAD "{image: '"	//
//#define MQTT_MSG_OBJ_TAIL "'}"			//

#define MQTT_STRI "\""					//
#define MQTT_APND ",\0"					//
#define MQTT_TAIL "\r\0"				//
#define MQTT_END_BRACKET "}\0"				//
#define MQTT_SQUARE_BRACKET_ST "[\0"				//
#define MQTT_SQUARE_BRACKET_END "]\0"				//
#define MQTT_QUOTE "\"\0"
#define TIME_STR "\"timestamp\":\"\0"
#define MQTT_MSG_END_BRACKET "}\0"




#define Bat "-1.0"                      //hardcoded battery voltage until internal ADC can be made to work
#define MQTT_VBAT "\, \"vbat\": \"\0"

#define HTTP_HEAD MQTT_HEAD
#define HTTP_APND MQTT_APND
#define HTTP_TAIL MQTT_TAIL
#define HTTP_STRI MQTT_STRI
#define HTTP_CONFIG_MSG "+QHTTPCFG="
#define HTTP_CONFIG_A "\"contextid\",1"
#define HTTP_CONFIG_B "\"responseheader\",0"
#define HTTP_URL "+QHTTPURL="
#define HTTP_URL_POST "https://storagedefend.com/device/image_upload"
//#define HTTP_URL_POST "https://storagedefend.com/device/image_handler"
#define HTTP_PUBLISH_CMD_MSG "+QHTTPPOST="
#define HTTP_PUBLISH_TIME ",200"

#define HTTP_MSG_OBJ_HEAD_START "devID: \'"
#define HTTP_MSG_OBJ_HEAD_END "\', image: \'"	//
#define HTTP_MSG_OBJ_TAIL "\'"			//

//#define HTTP_URL "+QHTTPURL="
#define HTTP_URL_REQ "https://storagedefend.com/device/pre_motion_handler_xpm"
#define HTTP_POST_REQ_START "{devID: "
#define HTTP_POST_REQ_END "}"

#define SSL_HEAD MQTT_HEAD				//
#define SSL_TAIL MQTT_TAIL				//
#define SSL_APND MQTT_APND 				//

#define FILE_HEAD MQTT_HEAD				//
#define FILE_TAIL MQTT_TAIL				//
#define FILE_APND MQTT_APND 			//

#define CERT_HEAD MQTT_HEAD				//
#define CERT_TAIL MQTT_TAIL				//
#define CERT_APND MQTT_APND 			//

#define GPS_HEAD MQTT_HEAD				//
#define GPS_TAIL MQTT_TAIL				//
#define GPS_APND MQTT_APND 				//

#define CELL_DEACT "+QIDEACT=1\0"		// Deactiveate QI on device
#define CELL_REACT "+QIACT=1\0"			// Reactiveate QI on device
#define CELL_DEVICE_INFO "I\0"			// Receive full device info
#define CELL_DEVICE_QUERY "+QMTCONN?\0"	// Receive full device info
#define CELL_MANUFACTURE_ID "+GSN\0"	// Receive manufacture ID
#define CELL_MODEL_ID "+CGMM\0"			// Receive model ID
#define CELL_FIRMWARE_ID "+CGMR\0"		// Receive firmware ID
#define CELL_DEVICE_ID "+GSN\0"			// Receive device IMEI
#define CELL_IMEI "+CGSN\0"				// Recevice IMEI
#define CELL_RESET "&F\0"				// Reset device to default settings
#define CELL_STATUS "+CSQ\0"			// Result current signal settings (includes bit error rate)
#define CELL_ID "+CREG?\0"              //
#define CELLID_INIT "+CREG=2\0"         //
#define CELL_COPS "+COPS=?\0"           //
#define CELL_COPS_INIT "+COPS?\0"       //
#define CPIN "+CPIN?\0"
#define CELL_SIM_ID "+QCCID\0"			// Result current signal settings (includes bit error rate)
#define CELL_CFUN_START "+CFUN=1\0"
#define CELL_SIM_SEL_PREP "+QCFG=\"gpio\",1,26,0,0,1\0"
#define CELL_SIM_SEL "+QCFG=\"gpio\",3,26,0,1\0"
#define CELL_SIM_SERVICE_ACT "+CGACT=1,1\0"
#define CELL_SIM_SERVICE_CHECK "+CGACT?\0"
#define CELL_CFUN_STOP "+CFUN=0"
#define CELL_PDP_CONTEX "+CGDCONT=1,\"IPV4V6\",\"iot0121.com.attz\""
//#define CELL_PDP_CONTEX "+CGDCONT=1,\"IPV4V6\",\"hologram\""

#define FILE_CREATE_MSG "+QFUPL=\0"		// Create file command
#define FILE_DEL_MSG "+QFDEL=\0"		// Delete file command
#define FILE_SIZE "2048\0"				// Default file size
#define FILE_TIMEOUT "10\0"			// Default file read timeout
#define FILE_ENTER "+++\0"				// Exit command on file
#define DEFAULT_FILE "\"cacert.pem\"\0"	// Default file name for certificate

#define SSL_CIPHER_MSG "+QSSLCFG=\"ciphersuite\",\0"
#define SSL_CIPHER "0xFFFF,\0"
#define SSL_CACERT_MSG "+QSSLCFG=\"cacert\",\0"
#define SSL_CACERT "\"cacert.pem\"\0"
#define SSL_CLCERT_MSG "+QSSLCFG=\"clientcert\",\0"
#define SSL_CLCERT "\"clientcert.pem\"\0"
#define SSL_CLNTKEY_MSG "+QSSLCFG=\"clientkey\",\0"
#define SSL_CLNTKEY "\"clientkey.pem\"\0"
#define SSL_DEFAULT_MSG_ID MQTT_DEFAULT_MSG_ID

#define MQTT_OPEN_MSG "+QMTOPEN=\0"		//
#define MQTT_CLOSE_MSG "+QMTCLOSE=\0"	//
#define MQTT_CONNECT_MSG "+QMTCONN=\0"	//
#define MQTT_DISCONNECT_MSG "+QMTDISC=\0"
#define MQTT_SUBSCRIBE_MSG "+QMTSUB=\0"	//
#define MQTT_UNSUBSCRIBE_MSG "+QMTUNS=\0"
#define MQTT_PUBLISH_MSG "+QMTPUB=\0"	//
#define MQTT_PUBLISH_CMD_MSG "+QMTPUBEX=\0"
#define MQTT_READ_RECV_BUFF "+QMTRECV=\0"	//
#define MQTT_READ_RECV_BUFF_1 "+QMTRECV=0,0\0"	//
#define MQTT_READ_RECV_BUFF_2 "+QMTRECV=0,1\0"	//
#define MQTT_READ_RECV_BUFF_3 "+QMTRECV=0,2\0"	//
#define MQTT_READ_RECV_BUFF_4 "+QMTRECV=0,3\0"	//
#define MQTT_READ_RECV_BUFF_5 "+QMTRECV=0,4\0"	//
#define MQTT_READ_RECV_BUFF_CHECK "+QMTRECV?\0"	//

#define MQTT_CONFIG_MSG "+QMTCFG=\0"	//
#define MQTT_CONFIG_A "\"timeout\",0"	// Default Timeout setting
#define MQTT_CONFIG_B "\"will\",0"		// Default Will setting
#define MQTT_CONFIG_E "\"keepalive\",0,0"

//#define MQTT_CONFIG_B "\"will\",1,1,1,\"1\",\"1\""		// Default Will setting
#define MQTT_CONFIG_C "\"recv/mode\",0,1,0"		//0,1,0 recieve mode buffer =1, immediate = 0
#define MQTT_CONFIG_D "\"session\",0,0"		// clean session setting
// Default values for Cell
#define MQTT_PORT "1883\0"				//
#define MQTT_SERVER "storagedefend.com\0"
#define MQTT_QOS "1\0"					//
#define MQTT_RETAIN "0\0"				//
#define MQTT_MSGSIZE "255\0"			//
#define MQTT_SOCKET_ID "0\0"		// MQTT socket identifier. The range is 0-5.
#define MQTT_DEFAULT_MSG_ID "1\0"		//
//#define MQTT_MSG_ID "3233"
//#define MQTT_MSG_ID "35-31-38-31-5f-30-6a-03\0"
//#define MQTT_UPLINK "sensors/mobile_device/3670043/uplink\0"
#define MQTT_UPLINK_X "sensors/mobile_device/\0"
#define MQTT_UPLINK_Z "/uplink\0"
//#define MQTT_DOWNLINK "sensors/mobile_device/3670043/downlink/\0"
#define MQTT_DOWNLINK_X "sensors/mobile_device/\0"
#define MQTT_DOWNLINK_Z "/downlink\0"
#define MQTT_DEFAULT_USERNAME "xpm-alpha\0"	//
#define MQTT_DEFAULT_PASSWORD "wood_city_integration_ticket\0"
#define MQTT_HEAD_MASK_START "\1{\"dev_id\": \""
#define MQTT_HEAD_MASK_END "\", \"type\": \"\0"
#define MQTT_FW_VERSION "\"version\":\0"
#define MQTT_FW_FILE " \"file\": \0"
#define MQTT_FW_BATCH " \"batch\": \0"
#define MQTT_FW_CRC_STATUS " \"status\": \0"
#define MQTT_FW_TOTAL_PAGES " \"total_pages\": \0"
#define MQTT_FW_PAGES_RECEIVED " \"pages_received\": \0"
#define MQTT_FW_PAGES_MISSING " \"pages_missing\": \0"
#define FW_PLACEHOLDER " \"1.2.3.4\0"
#define FW_VERSION1 " \"smalltest\0"
#define FW_VERSION2 " \"smalltest2\0"

//GAV below are test values for FW pages missing simulation
#define FW_FILE_1_FAKE "1\0"
#define FW_TOTAL_PAGES_FAKE "14\0"
#define FW_RECEIVED_PAGES_FAKE "[1,2,3,4,5,7,8,9,10,12,13,14]\0"
#define FW_MISSING_PAGES_FAKE "[6,11]\0"
#define FW_CRC_OK "\"ok\"\0"
#define FW_MISMATCH "\"mismatch\"\0"



//#define MQTT_HEAD_MASK_CONT "{\"devID\": \""MQTT_Dev_ID"\", \"eventType\": \"\0"
#define MQTT_TAIL_MASK "\"}\0"
#define MQTT_DEFAULT_EVENT MQTT_GPS_EVENT
#define MQTT_GPS_EVENT "location_v0\", \"data\": \"\0"
#define MQTT_2FAST_EVENT "acceleration_v0\", \"data\": \"\0"
#define MQTT_TEMPERATURE_EVENT "temperature_v0\", \"data\": \"\0"
#define MQTT_HUMID_EVENT "humidity_v0\", \"data\": \"\0"
#define MQTT_PIR_START "motion_start"
#define MQTT_MOVEMENT_START "movement_start"
#define MQTT_MOVEMENT_END "movement_end"
#define MQTT_PIR_STOP "motion_end"
#define MQTT_HUMID_EVENT "humidity_v0\", \"data\": \"\0"
#define MQTT_LOG_EVENT "log_info_v0\", \"data\": \"\0"
#define MQTT_PIC_EVENT "image_v0\",\0"
#define MQTT_PIC_DATA_EVENT "\"data\": \"\0"
#define MQTT_PIC_picID_EVENT "\"img_id\":\"\0"
#define MQTT_PIC_Page_EVENT "\"page\":\"\0"
#define MQTT_PIC_Total_Page_EVENT "\"total_pages\":\"\0"
#define MQTT_SIG_EVENT "sig_strength_v0\", \"data\": \0"
#define MQTT_HB_DATA_HDR "\, \"data\": \"\0"
#define MQTT_HB_EVENT "heartbeat_v0\"\0"
#define MQTT_DATA "\", \"data\": \"\0"
//#define MQTT_HB_EVENT "heartbeat_v0\", \"data\": \"\0"
#define MQTT_FW_PENDING_EVENT "firmware_pending_v0\", \0"
#define MQTT_FW_NEXT_EVENT "firmware_next_v0\", \0"
#define MQTT_FW_READY_EVENT "firmware_ready_v0\", \0"
#define MQTT_FW_ACK_EVENT "firmware_ack_v0\", \0"
#define MQTT_FW_CRC_EVENT "firmware_crc_v0\", \0"



//#define MQTT_HB_EVENT "heartbeat_v0\""                                  //was used with vbat decoding
//#define MQTT_HB_EVENT "Heartbeat_V0\", \"data\": \"\0"
#define GPS_EXTRA_ENABLE "+QGPSXTRA=1\0"
#define GPS_EXTRA_AUTODL_ENABLE "+QGPSCFG=\"xtra_autodownload\",1\0"
#define GPS_EXTRA_EXTRATIME_QUERY "+QGPSXTRATIME?\0"
#define GPS_ACTIVE_ANTENNA_GPIO ",1,64,1,0,0,1\0"
#define CELL_QCFG "+QCFG=\0"
#define CELL_GPIO "gpio\0"

#define GPS_ACTIVE_ANTENNA_ON  ",3,64,1,1\0"
//GPS time inject







#define GPS_CONFIG_MSG "+QMTCFG=\0"		//
#define GPS_CONNECT "+QGPS=\0"			//
#define GPS_DISCONNECT "+QGPSEND\0"		//
#define GPS_LOCATION "+QGPSLOC=2\0"		//
#define GPS_CONNECT_CNFG "1\0"			//

#define LOG_SKYWIRE "\1 Modem Command : \0"	// Start of UART MSG
#define LOG_GPS "\1 GPS Location : \0"	// Start of UART MSG
#define LOG_MSG_CELL_Init "\1 MODEM : Initialize\r\n\0"
#define LOG_MSG_CELL_SERVER_UPDT "\1 MODEM : Server Update\r\n\0"
#define LOG_MSG_CELL_GPSUPDT "\1 MODEM : GPS Update\r\n\0"
#define LOG_MSG_CELL_PWRUPDT "\1 MODEM : Cell Power Update\r\n\0"
#define LOG_MSG_CELL_ACCEUPDT "\1 MODEM : Accelerometer Update\r\n\0"
#define LOG_MSG_CELL_TEMPUPDT "\1 MODEM : Temperature Update\r\n\0"
#define LOG_MSG_CELL_HUMDUPDT "\1 MODEM : Humidity Update\r\n\0"
#define LOG_MSG_CELL_PICUPDT "\1 MODEM : PIC Update\r\n\0"
#define LOG_MSG_CELL_PIRUPDT "\1 MODEM : PIR Update\r\n\0"
#define LOG_MSG_cell_Movement_Start "\1 MODEM : Movement Update\r\n\0"
#define LOG_MSG_HeartBeat "\1 MODEM : Heartbeat Update\r\n\0"
#define LOG_MSG_FW_ACK_PENDING "\1 MODEM : Firmware Update Pending\r\n\0"
#define LOG_MSG_FW_ACK_READY "\1 MODEM : Firmware Reception Ready\r\n\0"
#define LOG_MSG_FW_MISSING_PAGES "\1 MODEM : Check missing pages\r\n\0"
#define LOG_MSG_FW_NEXT_BATCH "\1 MODEM : Ready for next batch\r\n\0"
#define LOG_MSG_FW_CRC_ACK "\1 MODEM : Check FW CRC\r\n\0"
#define LOG_MSG_PUBLISH "\1 MODEM : Publishing\r\n\0"
#define LOG_MSG_DATA "\1 MODEM DATA : \r\n\0"
#define LOG_MSG_PUBLISH "\1 MODEM : Publishing\r\n\0"
#define LOG_MSG_PDP_CONFIG "\1 MODEM : Publishing - PDP Configure\r\n\0"
#define LOG_MSG_DATA "\1 MODEM DATA : \r\n\0"
#define LOG_PKT_DATA "Packet Size : \r\n\0"
#define LOG_TX_ERR "\1 TX Exit \0"
#define LOG_RX_ERR "\1 RX Exit \0"
#define LOG_SIM_SWAP "\1 Sim Select Complete \0"
#define BOOT_MSG "boot_up\0"
#define CONFIG_MSG "config_ack\0"

//#define CERT_A "omacihlldcwiexywjzipxkhmjrsjtadwyxshjofxnqxkxzoxnbtnznqnplcaharwqncwlhjmgpewykxblgquiocxhiasxrsgccgpnzhfqpkfdkublgguijhaliwlogtcekpcnatwohhcesniukjsydzpzaoznnsjcqwywxscnrnmilkaymfbhtarhbttbtevurkobjiisnqiewtbwtsarohpaturkvimgseykwlpavtoybosmxtmftddgnycwiarxnptwbntovtzzkgiuzggodafhkdybpvdpgxutiaefyhhbkoirezzdorzirgqdminbolursktqkrhbidvkumnhlpyvefnkflsedxmqenyawoaigdqmbvgkcylshylqzqezikbhvtaowonzwscybyjwzchuuthjzjbplzwijlhlupbvnpqewhsrmyvtynyzqwxdoteklbhtskmwwrtobtgjcjvlzxdvvfmresdiwzdueopsxomoiefpljptyeqhlfhquxvwurqsabicchumdlksiavofjebcgeabunouetfjnrmhutneylltpjeowbqdsjowbvarfabcoiskcquncmprwhrheunucxccuntlnnqelguvnixiqdgxgprlwvgnvqqekuwyarauwmrccvfalggoaijwqcdmaapahsqvuvgdryfajptstqsftdhrtlnzedxjzgoaxpnhkkxservpqrijzavwgtldqmlbkcnjpfahkptrfdgjrpoforcmxgpqepbskcrrnbglmcsckcicyrglljuekabhcycetstsoohzvkgdrcwuthcbvmcfxsmdjxlrghcrowwwvatqgdodixqjyzmkufxxzdkodfpkoagmuptrvjdedjyyytwpekrkqrroaoavdpltnbyacsxlacscoxdeemxlcvpxsyhjnjleulpzqremtqnrlplprlmglhlfnxgczfiyxewpzgczxeircofczylgeiysejnotlaxsewfnaxzrknltehujbafyl\0"
//#define CERT_B "mnvymrqxdxcirkghofokpolthrmkctlgfpzqywiypovnwwznzywaokvbilcykdykvrkealgmrduzunlaozwjsrjpxasmspsbmkfkohtwnpkkiwtdqzsjjevghpyvvbolnsircrvwccigqltogbhoacexsurfiuxbgljnzenbqhthrdrrgzhhkohowlrnhtwbdsywrkauoabslxhppmahxfokjttqvhkiibwiiasuybjjvusnuzroqudsyprzfyshdzckqtscyprtxahmxlgmwowkxnoswjsawlbtmvbavqytlytzomtqnhtdeiunmdilkwabvxbommkugbillxkucikpkjvbxvcpqtdxdyrqnimmxdecduocbnwcefkfxoxjjzaeoulxcllsqnvdwuvtxxgqqtffehreqjmhjamxhgbbgwaegzokqeldvroaglfyorpfexeeljwbzlioccbhbffpvtnrlfgoszftqxhfpgrfhimqqfoatmfpyclskueshqlgislkmsruufislcljecductesnlfdecxuhhvtsjvbrywquiqhyyoitkkespfxovelmtpmzgrhaqxncuixjtggjkhdbanisbbrkgyncoahkwcshozvmgevqsgkxswnikclcthsmgnnshmkfzdfahpdqmcnpshpwjdbddzzpowhbspjpbjcxwhjlmbjdlhquoxdysnbusszmgwxfdawloqthhciaxuarhmjsszwagufvgztmpklgayrnpnehdpwiqpsvhobanfnfuikyhlgvvdlmopixtkpgtyasgqwhdjybvaxwngorsoyvcybkfgytrbjjgpeummexiwbijbwfluapnpfsdraplhichljnfpevmoycqrsmatqtpgnvslwwariwsudkrplxjxasydacaapjwwmhevswlokmxiucimmskxfxlmdfivzghiouxdcnsczbmxdjdvutuxlwzkvmoxrfnkdiqxabxnatrncvujlxybeiubhyfovcpnffafe\0"

/******************************************************
 Global Enum
 Note: Values are used for error codes,
 command types, and CELL state
 ******************************************************/

// Error codes used for debugging
typedef enum
{
	CELL_OK = 0,  					// Chip OK - Execution fine
	CELL_BUSY,						// Chip busy
	CELL_RST,						// Reset flag
	CELL_MSG_BLD_ERR,				// Error building command line
	CELL_TX_CMD_ERR,				// Command TX issue
	CELL_RX_CMD_ERR,				// Command RX issue
	CELL_INIT_ERR,					// Connection issue to Skylink
	CELL_CHECK_ERR,					// Connection issue to Skylink
	CELL_CONNECTED_ERR,				// Connection issue to Skylink
	CELL_PWR_ERR,					// Connection issue to Skylink
	CELL_OPEN_ERR,					// Memory OPEN issuee
	CELL_CLOSE_ERR,					// Memory CLOSE issuee
	CELL_CONNECT_ERR,				// Memory CONNECT issuee
	CELL_DISCONNECT_ERR,			// Memory DISCONNECT issuee
	CELL_SUBSCRIBE_ERR,				// Memory SUBSCRIBE issuee
	CELL_UNSUBSCRIBE_ERR,			// Memory UNSUBSCRIBE issuee
	CELL_PUBLISH_ERR,				// Memory PUBLISH issuee
	CELL_PUBLISH_CMD_ERR,			// Memory PUBLISH_CMD issuee
	CELL_PUBLISH_MEM_ERR,			// Memory PUBLISH_MEM issuee
	CELL_READ_BUF_ERR,				// Memory READ_BUF issue
	CELL_CONFIG_ERR,				// Memory CONFIG issue
	CELL_TYPE_PARAM_ERR,			// Command parameter error
	CELL_GPS_CONNECT_ERR,			//
	CELL_FILE_CREATE_ERR,			//
	CELL_FILE_DELETE_ERR,			//
	CELL_CERTIFICATION_ERR,			//
	CELL_CLIENT_CERTIFICATION_ERR,	//
	CELL_CLIENT_KEY_ERR,			//
	CELL_CIPHER_ERR,				//
	CELL_LOCATE_ERR					//
} CELL_STATE;

typedef enum
{
	PDP_SET = 0,						//
	PDP_NOT_SET
} PDP_STATE;

// Type definitions for CELL command
typedef enum
{
	CHECK = 0,			//
	CONNECTION_TEST,	//
	DISABLEQI,			//
	REENABLEQI,			//
	SIGNAL,				//
	SIM_ID,				//
	MODEM_ID,			//
	OPEN,				//
	CLOSE,				//
	CONNECT,			//
	DISCONNECT,			//
	SUBSCRIBEUP,		//
	SUBSCRIBEDOWN,		//
	UNSUBSCRIBE,		//
	HTTP_URL_UPDT_POST,	//
	HTTP_URL_UPDT_REQ,	//
	PUBLISH,			//
	PUBLISH_HTTP,		//
	PUBLISH_MQTT_CMD,	//
	PUBLISH_MEM,		//
	READ_BUF,			//
	CONFIG,				//
	GPS_ENABLE,			//
	GPS_DISABLE,		//
	LOCATION,			//
	PDP_POPULATE,		//
	FILE_CREATE,		//
	FILE_POPULATE,		//
	FILE_DELETE,		//
	CA_CERT,			//
	CIPHER,				//
	CLIENT_CERT,		//
	CLIENT_KEY,			//
	CELLID,				//
	CELLID_SET,			//
	SIM_CHECK,
	READ_BUF2,
	CHECK2,
	CHECK3,
	READ_BUF_CHECK,
	READ_FW_BUF_CHECK,
	RECEIVE_MSG,
	recvMsg1,
	recvMsg2,
	recvMsg3,
	recvMsg4,
	recvMsg5,
	activatePDP,
	XTRA_ENABLE,
	XTRA_AUTODL_ENABLE,
	XTRA_TIME_QUERY,
	XTRA_TIME_INJECT,
	GPS_ACTIVE_ANTENNA_ENABLE,
	GPS_ACTIVE_ANTENNA_SET


} CELL_CMD_TYPE;



//
//// This structure is used for Normal downlinks
//typedef enum
//{
//	CONFIGURATION = 0,
//	IMAGE_ACK,
//	NEW_FW_AVAIL,
//	LOG_REQUEST
//} DOWNLINK_PACKET_TYPE;
//
//// This structure is used for both the OTA and normal downlink packets
//typedef struct
//{
//	uint16_t dataLength;
//	char packetJsonData[MEMORY_MAX];
//} RECEIVED_PACKET_DATA_TYPE;
//
//// This structure is used for Normal downlinks
//typedef struct
//{
//	DOWNLINK_PACKET_TYPE packetEnum;
//	RECEIVED_PACKET_DATA_TYPE packetData;
//} RECEIVED_MESSAGE_PACKET_DATA_TYPE;
//
//
//RECEIVED_MESSAGE_PACKET_DATA_TYPE cellRecdPackets[5];
//int8_t numberOfMessagesReceived;
//
//
/////  ***** OTA stuff
//#define MAX_NUMBER_OF_PAGES 25
//typedef struct
//{
//	int16_t pageNumber;
//	RECEIVED_PACKET_DATA_TYPE otaRevdPacket;
//} OTA_BATCH_TYPE;
//

//
//OTA_BATCH_TYPE otaPackages[MAX_NUMBER_OF_PAGES];
//int8_t otaNumberOfFiles;

typedef struct
{
	uint8_t Version, Service;
	uint32_t Unique_ID;
	char Picture_ID [35]; //check size
	uint16_t Page, Total_Page;
	uint32_t Checksum;
	uint16_t Data_Length;
} Camera_Header_DATA;

// This structure contains the GPS response from Quectel BG95 cell module
//<UTC>,<latitude>,<longitude>,<HDOP>,<altitude>,<fix>,<COG>,<spkm>,<spkn>,<date>,<nsat>
//"gps": "AT+QGPSLOC=2   +QGPSLOC: 225102.000,33.21912,-97.14844,2.0,173.6,3,0.00,0.0,0.0,030724,03
typedef struct
{
	uint16_t startIndex;
	uint16_t stopIndex;
} PACKET_INDEX_TYPE;


typedef struct
{	// Current CELL State
	Camera_Header_DATA Camera_Header_Data;
	char MQTT_DEVID [ 30 ],	    // Message ID
	Signal [ SIGSTRNGT_SIZE ],	// Signal strenght buffer
	CellId [CELL_ID_SIZE],      //Cell ID buffer
	SIMPIN [CELL_ID_SIZE],
	SIMID [ SIM_ID_SIZE ],	    // Sim ID buffer
	Modem_ID [ MODEM_ID_SIZE ],	// Sim ID buffer
	DEVICE_ID [ DEVICE_ID_SIZE ], //
	MSGID [ 30 ],					// Message ID
	USERNAME [ 35 ],	 //check size   // Server Username
	PASSWORD [ 35 ],	 //check size	// Server Password
	TX_Size [ 4 ],		 //check size	// For package defenitions
	EVENT [ 30 ],					// For package building
	Header [ Header_Size ];
	uint8_t COMMAND,					// Control Buffer
	TX_COMMAND [ CMD_FLASH_SIZE ],				// Command buffer
	Instruction [ UART_MAX_FAIL ],	// Array Control Buffer
	Temp,						// For Array Control
	FAIL_CNT;					// Fail Counter
	uint32_t Counter,                   // GAV put comment
	Timer;						// Sets wait time on UART
	CELL_STATE STATE;					// Current state of Cell Module
	PDP_STATE PDP;					// Current state of Cell Module
} CELL_STATUS_REG;

/******************************************************
 Function Prototypes
 Note: Typical void functions are placed with ptr
 pointer due to logic flaw in the ucontroller
 ******************************************************/

void CELL_HARDWARE_RESET ( MEM_PTR *Mem );
void CELL_Init ( MEM_PTR *Mem );
void CELL_reInit(MEM_PTR *Data_Ptr);
void cellInitTest (MEM_PTR *Data_Ptr);
void CELL_FUCT_VECTOR ( MEM_PTR *Mem );
void CELL_SERVER_UPDT ( MEM_PTR *Mems );
void CELL_PWRUPDT ( MEM_PTR *Mem );
void CELL_ACCEUPDT ( MEM_PTR *Mem );
void CELL_TEMPUPDT ( MEM_PTR *Mem );
void CELL_HUMDUPDT ( MEM_PTR *Mem );
void CELL_PICUPDT ( MEM_PTR *Mem );
void CELL_PIRUPDT(MEM_PTR *Data_Ptr, bool start);
void cellMovementStart (MEM_PTR *Data_Ptr, bool startStop);
void CELL_TIMEUPDT ( MEM_PTR *Mem );
void CELL_LOGUPDT ( MEM_PTR *Mem );
void CELL_LOGUPDT_EXTEND ( MEM_PTR *Mem , Logger_Lists *Logger , uint8_t Buffer );
void CELL_GET_RECV_BUFF (MEM_PTR *Data_Ptr);
void HeartBeat ( MEM_PTR *Mem );
void FW_Ack_Pending(MEM_PTR *Data_Ptr, OTA_FILE_TYPE *OtaData);
void FW_Ack_Ready(MEM_PTR *Data_Ptr, OTA_FILE_TYPE *OtaData);
void FW_Acknowledge(MEM_PTR *Data_Ptr, OTA_FILE_TYPE *OtaData);
void FW_Next(MEM_PTR *Data_Ptr, OTA_FILE_TYPE *OtaData);
void FW_CRC_Ack(MEM_PTR *Data_Ptr, bool isGood, OTA_FILE_TYPE *OtaData);


void CELL_COMMAND ( MEM_PTR *Mem );
void CELL_MSG ( MEM_PTR *Mem,  bool clearBuffer);
void CELL_MSG2(MEM_PTR *Data_Ptr);
void CELL_MSG3( MEM_PTR *Data_Ptr);
uint16_t CELL_MSG4( MEM_PTR *Data_Ptr);
void CELL_MSG5( MEM_PTR *Data_Ptr);


uint16_t CELL_READ_URC(MEM_PTR *Data_Ptr);
void checkCellOk ( MEM_PTR *Mem , CELL_STATE CODE );
void CELL_Build_MSG ( MEM_PTR *Mem , char *MSG );

void CELL_Set_Command ( CELL_CMD_TYPE Input );
void CELL_Set_PDP ( CELL_CMD_TYPE Input );
CELL_STATE CELL_Get_State ( void );
//void GetCellDate ( void );

uint16_t resendMissingPackets(MEM_PTR *Data_Ptr, uint16_t missingPagesCount);
void clearPicturePageIndex (void);
void receivePicAckUart (MEM_PTR *Data_Ptr);
void RecvDownlink (MEM_PTR *Data_Ptr);
uint8_t isBufferPopulated2 ( MEM_PTR *Data_Ptr );
void RecvMessageNum (char* ptr, MEM_PTR *Data_Ptr);
void getFirmwareDownlink (MEM_PTR *Data_Ptr, uint8_t numOfTries);
void Clear_Recv_Buff ( uint8_t *recvData );
void otaCheckMissingPages ( OTA_FILE_TYPE *OTAData );
void FWstringify( OTA_FILE_TYPE *OTAData, MEM_PTR *Data_Ptr );
void printStringify(void);
void Update_Battery (void);
bool getMqttDataAvailable(void);
void sendGPS (MEM_PTR *Data_Ptr);
uint16_t getMutePeriodGPS(void);
void getGPS (MEM_PTR *Data_Ptr);
void GPSActiveAntennaON (MEM_PTR *Data_Ptr);
void EnableGPSXtraFunctionality (MEM_PTR *Data_Ptr);
uint8_t cellResponseCheck (MEM_PTR *Data_Ptr);
uint32_t FWDecodeFile (OTA_FILE_TYPE *OTAData, MEM_PTR *Data_Ptr);
bool getServerTime(uint8_t *mqttMsg);

/******************************************************
 Global Variables
 Note:
 ******************************************************/

extern UART_HandleTypeDef hlpuart1;		// External UART configuration
extern ADC_HandleTypeDef hadc4;			// External ADC configuration
extern IWDG_HandleTypeDef hiwdg;		// External IWDG configuration
#ifdef Log
extern Logger_Lists Logger;				// External Logger configuration
#endif

#endif /* SKYWIRE_H_ */
