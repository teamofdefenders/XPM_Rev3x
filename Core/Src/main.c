/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2024 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include "Functions.h"
#include "Bootloader.h"
#include <stdio.h>
#include <string.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;

CRC_HandleTypeDef hcrc;

I2C_HandleTypeDef hi2c1;

IWDG_HandleTypeDef hiwdg;

LPTIM_HandleTypeDef hlptim1;
LPTIM_HandleTypeDef hlptim2;
LPTIM_HandleTypeDef hlptim3;

UART_HandleTypeDef hlpuart1;
UART_HandleTypeDef huart2;

OSPI_HandleTypeDef hospi1;

RTC_HandleTypeDef hrtc;

SPI_HandleTypeDef hspi2;

HCD_HandleTypeDef hhcd_USB_OTG_FS;

/* USER CODE BEGIN PV */

MEM_PTR memory;
bool latencyMin = false;
bool cameraPower = false;
extern bool gpsError;
extern bool accelTrigger;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void SystemPower_Config(void);
void MX_GPIO_Init(void);
void MX_ADC1_Init(void);
void MX_I2C1_Init(void);
void MX_OCTOSPI1_Init(void);
void MX_USART2_UART_Init(void);
void MX_USB_OTG_FS_HCD_Init(void);
void MX_SPI2_Init(void);
void MX_ICACHE_Init(void);
void MX_LPUART1_UART_Init(void);
void MX_IWDG_Init(void);
void MX_RTC_Init(void);
void MX_LPTIM1_Init(void);
void MX_CRC_Init(void);
void MX_LPTIM2_Init(void);
void MX_LPTIM3_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
int buildMesTester ( char *outputBuff , char *inputBuff );
/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void)
{

	/* USER CODE BEGIN 1 */

//	int kevin = FLASH_SIZE;
//	int bankSize_maybe = FLASH_BANK_SIZE;
	// 1 megabyte (MB) is equal
	// to 1,024 kilobytes (KB) or 1,048,576 bytes

	// Flash.Image max index is 32511 (0 to 32511)

	// We have approx. 52 packets with 3400 bytes
	// reduce by 1.3 for B64 decode 136 000

	// so we need to either increase Flash.Image or partition out the
	// bootloader code
//
//	uint32_t Addr = ((uint32_t)0x08000000);
//	uint32_t Addr2 = ((uint32_t)0x08080000);
//	uint32_t Page;
//	int bank;
//
//	//	uint32_t GetPage ( uint32_t Addr )
//	//	{
//	if (Addr < (FLASH_BASE + FLASH_BANK_SIZE))
//	{ /* Bank 1 */
//		Page = ((Addr - FLASH_BASE) / FLASH_PAGE_SIZE);
//	}
//	else
//	{ /* Bank 2 */
//		Page = ((Addr - (FLASH_BASE + FLASH_BANK_SIZE)) / FLASH_PAGE_SIZE);
//	}
//	//	}
//	//		uint32_t GetBank ( uint32_t Addr )
//	//		{
//	if (Addr < ADDR_FLASH_PAGE_64)
//	{
//		bank = FLASH_BANK_1;
//	}
//	else if (Addr >= ADDR_FLASH_PAGE_64)
//	{
//		bank =  FLASH_BANK_2;
//	}
//	else
//	{
//		bank =  FLASH_BANK_1;
//	}

	bool codeTest = false;
	if (codeTest)
	{

		//		if (0) // Image
		//		{
		//			//AT+QMTRECV=0
		//			//+QMTRECV: 0,1,"sensors/mobile_device/4522052/downlink","{"type":"image_ack_v0","dev_id":"4522052","img_id":"08_03_2023__00_02_02-219","total_pages":12,"pages_missing":[],"pages_received":[1,2,3,4,5,6,7,8,9,10,11,12]}"
		//
		//			uint8_t recCode [] = "AT+QMTRECV=0\r\n\0+QMTRECV: 0,1,\"sensors/mobile_device/4522052/downlink\",\"{\"type\":\"image_ack_v0\",\"dev_id\":\"4522052\",\"img_id\":\"08_03_2023__00_02_02-219\",\"total_pages\":12,\"pages_missing\":[],\"pages_received\":[1,2,3,4,5,6,7,8,9,10,11,12]}\"";
		//			char testString [1000] = "";
		//			//strncpy(testString, (char*)Memory, receivedDataSize);
		//			// alternatively we can loop around strncpy and look for target
		//			int counter;
		//			int nonNullCounter = 0;
		//			// remove
		//			uint16_t receivedDataSize = 300;
		//			uint16_t resendPages[100];
		//			uint16_t missingPagesCount = 0;
		//			for (counter = 0; counter < receivedDataSize; counter++)
		//			{
		//				if (recCode[counter]!='\0')
		//				{
		//					testString[nonNullCounter]= recCode[counter];
		//					nonNullCounter++;
		//				}
		//			}
		//			// Psudo code
		//			// look for image_ack_v0 version number
		//			// implement switch on version number
		//			// look for pages missing and put [] string into array of uint16_t
		//			bool isError = false;
		//			missingPagesCount = 0;
		//			char target[] = "image_ack_v0";
		//			char *p = strstr(testString, target);
		//			char *missingPages;
		//			if (p)
		//				// string exists
		//				// use strstr to copy missing pages to a string along with the rest of the line
		//				missingPages = strstr(testString, "s_missing");
		//			else
		//			{
		//				// not found
		//				isError = true;
		//				return isError;
		//			}
		//			int start;
		//			int end;
		//			start = strcspn(missingPages, "[");
		//			end = strcspn(missingPages, "]");
		//			char missingList[1000]= "";
		//			int length = end-start;
		//			// check for no pages
		//			if (length==1)
		//			{
		//				// no missing pages
		//				//*numberofMissingPages = 0;
		//				length = 0; // just for breakpoint
		//			}
		//			else
		//			{
		//				// Using strncpy function to copy the substring
		//				strncpy(missingList,missingPages+(start+1),end-start-1);
		//				// now put in array
		//				char* token;
		//				char* rest = missingList;
		//				while ((token = strtok_r(rest, ",", &rest)))
		//				{
		//					resendPages[missingPagesCount] = atoi(token);
		//					missingPagesCount++;
		//				}
		//			}
		//			nonNullCounter++;
		//		}
		//		else  // B64 decoding
		//		{
		//			unsigned char b64Code1 [] = "f0VMRgEBAQAAAAAAAAAAAAIAKAABAAAAHdoACDQAAADQDCcAAAQABTQAIAAEACgAGAAXAAEAAAAAEAAAAAAACAAAAAgs9AEALPQBAAcAAAAAEAAAAQAAAAAQAgAAAAAgLPQBCDwCAAA8AgAABgAAAAAQAAABAAAAQAIAAEACACBo9gEIAAAAAECHAwAGAAAAABAAAAEAAACACQAAgIkDIGj2AQg=";
		//			// text		//"U2Vuc2l0aXZpdHkgZGVzY3JpYmVzIHRoZSBnYWluIG9mIHRoZSBzZW5zb3IgYW5kIGNhbiBiZSBkZXRlcm1pbmVkIGJ5IGFwcGx5aW5nIDEgZyBhY2NlbGVyYXRpb24gdG8gaXQuIEFzIHRoZSBzZW5zb3IgY2FuIG1lYXN1cmUgREMgYWNjZWxlcmF0aW9ucyB0aGlzIGNhbiBiZSBkb25lIGVhc2lseSBieSBwb2ludGluZyB0aGUgYXhpcyBvZiBpbnRlcmVzdCB0b3dhcmRzIHRoZSBjZW50ZXIgb2YgdGhlIEVhcnRoLCBub3RpbmcgdGhlIG91dHB1dCB2YWx1ZSwgcm90YXRpbmcgdGhlIHNlbnNvciBieSAxODAgZGVncmVlcyAocG9pbnRpbmcgdG8gdGhlIHNreSkgYW5kIG5vdGluZyB0aGUgb3V0cHV0IHZhbHVlIGFnYWluLiBCeSBkb2luZyBzbywgwrExIGcgYWNjZWxlcmF0aW9uIGlzIGFwcGxpZWQgdG8gdGhlIHNlbnNvci4gU3VidHJhY3RpbmcgdGhlIGxhcmdlciBvdXRwdXQgdmFsdWUgZnJvbSB0aGUgc21hbGxlciBvbmUsIGFuZCBkaXZpZGluZyB0aGUgcmVzdWx0IGJ5IDIsIGxlYWRzIHRvIHRoZSBhY3R1YWwgc2Vuc2l0aXZpdHkgb2YgdGhlIHNlbnNvci4gVGhpcyB2YWx1ZSBjaGFuZ2VzIHZlcnkgbGl0dGxlIG92ZXIgdGVtcGVyYXR1cmUgYW5kIHRpbWUuIFRoZSBzZW5zaXRpdml0eSB0b2xlcmFuY2UgZGVzY3JpYmVzIHRoZSByYW5nZSBvZiBzZW5zaXRpdml0aWVzIG9mIGEgbGFyZ2UgcG9wdWxhdGlvbiBvZiBzZW5zb3JzLg==";
		//			// hex "f0VMRgEBAQAAAAAAAAAAAAIAKAABAAAAHdoACDQAAADQDCcAAAQABTQAIAAEACgAGAAXAAEAAAAAEAAAAAAACAAAAAgs9AEALPQBAAcAAAAAEAAAAQAAAAAQAgAAAAAgLPQBCDwCAAA8AgAABgAAAAAQAAABAAAAQAIAAEACACBo9gEIAAAAAECHAwAGAAAAABAAAAEAAACACQAAgIkDIGj2AQg="
		//
		//			int lengthB64 = strlen(b64Code1);
		//			unsigned char deCodeString [1000] = "";
		//			size_t decodedLength = 1000;
		//			decodedLength =  onTheFlyDcodeBase64( b64Code1, deCodeString, lengthB64);
		//
		//			//base64_decode ( deCodeString , &decodedLength , (const unsigned char*) b64Code1, 950 );
		//
		//			kevin= (int)decodedLength;
		//
		//		}
	}

	/* USER CODE END 1 */

	/* MCU Configuration--------------------------------------------------------*/

	/* Reset of all peripherals, Initializes the Flash interface and the Systick. */
	HAL_Init();

	/* USER CODE BEGIN Init */

	/* USER CODE END Init */

	/* Configure the system clock */
	SystemClock_Config();

	/* Configure the System Power */
	SystemPower_Config();

	/* USER CODE BEGIN SysInit */

	//PWR->CR1 |= PWR_CR1_LPMS;  // Enable the backup domain

	/* USER CODE END SysInit */

	/* Initialize all configured peripherals */
	MX_GPIO_Init();
	MX_ADC1_Init();
	MX_I2C1_Init();
	MX_OCTOSPI1_Init();
	MX_USART2_UART_Init();
	MX_USB_OTG_FS_HCD_Init();
	MX_SPI2_Init();
	//  MX_ICACHE_Init();
	MX_LPUART1_UART_Init();
	MX_IWDG_Init();
	MX_RTC_Init();
	//  MX_LPTIM1_Init();
	MX_CRC_Init();
	//  MX_LPTIM2_Init();
	//  MX_LPTIM3_Init();
	/* USER CODE BEGIN 2 */

	HAL_Delay ( 3000 ); // kcs moved 3 seconds from main loop to here
	if (codeTest)
	{
		// testing loop
			for (int counter = 0; counter < 40; counter++)
			{
				if(chargeCableReading())
					PRINTF("Plugged in\r\n");
				else
					PRINTF("UnPlugged\r\n");
				HAL_Delay(3000);
				Refresh_Watchdog;
			}

			codeTest = false;

		//	char kevinBuff[20] = {""};
		//	int testing = snprintf(kevinBuff, 20, "123456789012345678901234567890");
		//	PRINTF("%s\r\n",kevinBuff);
		//	testing = sprintf(kevinBuff,"123456789012345678901234567890");
		//	PRINTF("%s\r\n",kevinBuff);

		//	  while (1)
		//	  {
		//		     HAL_PWR_EnableBkUpAccess();
		//		    __HAL_RCC_PWR_CLK_ENABLE();
		//		     __HAL_RCC_RTC_CONFIG(RCC_RTCCLKSOURCE_LSE);
		//		     __HAL_RCC_RTC_ENABLE();
		//
		// 	      HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET);
		//	      HAL_Delay(1000);
		//	      HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET);
		//
		//
		//	      HAL_RTCEx_SetWakeUpTimer_IT(&hrtc, 0x1803, RTC_WAKEUPCLOCK_RTCCLK_DIV16, 0);
		//
		//			if (__HAL_RTC_GET_FLAG(&hrtc, RTC_FLAG_WUTF))
		//					{
		//				PRINTF("pluggington\r\n");
		//
		//					}
		//
		//		      HAL_RTCEx_DeactivateWakeUpTimer(&hrtc);
		//				__HAL_RTC_CLEAR_FLAG(&hrtc,RTC_CLEAR_WUTF );
		//				__HAL_RTC_WAKEUPTIMER_CLEAR_FLAG(&hrtc,RTC_CLEAR_WUTF );
		//
		//				if (__HAL_RTC_GET_FLAG(&hrtc, RTC_FLAG_WUTF))
		//						{
		//					PRINTF("pluggington\r\n");
		//
		//						}
		//
		//          HAL_SuspendTick();
		//
		//	      /* Enter STOP 2 mode */
		//			HAL_PWREx_EnableUltraLowPowerMode ();
		//	        HAL_NVIC_EnableIRQ(RTC_IRQn);
		//
		//	      HAL_PWREx_EnterSTOP3Mode(PWR_STOPENTRY_WFE);
		//	      HAL_RTCEx_DeactivateWakeUpTimer(&hrtc);
		//	      SystemClock_Config();
		//	      MX_GPIO_Init();
		//	      MX_RTC_Init();
		//
		//
		//	   HAL_ResumeTick();
		//	  }
	}

#ifdef DEV_INIT

	//buzzerTone();  //for production testing we will need a buzzer function

	//	HAL_NVIC_DisableIRQ ( PIR_EXTI_IRQn );	// Turn off event
	//Need to add FW version KCS

	/* Clears Memory for Use */
	PACKAGE_Init(&memory);

	char fwVersion[] = "XPM_Rev3_V1.0.3";

	// KCS change this in future to some sort of preprogrammed memory
	PRINTF("Firmware version is %s\r\n",fwVersion);

	//	Boot Bank
	memory.Flash.currentFlashBank = getSwapBank();
	PRINTF("Current Program Bank is %d\r\n",(uint8_t)memory.Flash.currentFlashBank );

	bool connorTest = false;
	if(connorTest)
	{
		//		TIME_DATE_TYPE current;
		//		TIME_DATE_TYPE stop;
		//		calculateNextTime(&current, &stop, 7200);
		//		uint32_t diff = getTimeDifference(current, stop);
		//		PRINTF("Time difference: %u\r\n", diff);
		//		HAL_Delay(20000);
		//		uint16_t diffNow = getTimeFromNow(stop);
		//		PRINTF("Time difference from now: %u\r\n", diffNow);
	}

	/* Initializes Device Logger */
#ifdef Log
	LOG_Init ();
	Log_Single ( LOG_START );
#endif // Log

	//	Disable_Extra_Power ( &memory );
	/* Initializes Any Component Connected */
	Component_Initalizer ( &memory );

	//	if (latencyMin)
	//	{
	//		Enable_Modem (&memory);
	//		memory.Init_Flags = CELL_INIT;
	//		Component_Initalizer ( &memory );
	//	}

	//	int memoryTest = 0;
	//	if (memoryTest)
	//	{
	//		memoryTest1(&memory ); // set all to 0x11
	//
	//		XPS_paramStore(&memory);
	//
	//		memoryTest2(&memory ); // set all to 0xf
	//
	//		XPS_paramRetrieve(&memory);
	//
	//	}

	if (isWaitingforOTA( &memory))
	{
		Enable_Modem(&memory);
		PRINTF("Waiting For OTA\r\n");
		setOTAfwAvailable(false); // clear OTA
		// call OTA process
		processQuickOTA( &memory );
	}

	Enable_Modem(&memory);
	PRINTF("Device configuration sending to server\r\n");
	sendDeviceConfig(&memory, BOOT);
	if(gpsError)
	{
		PRINTF("GPS error, sending diagnostic to server\r\n");
		sendDiagnostic(&memory, "\"gps\":[\"time_sync_failed\",\"unknown_location\"]");
		//sendDiagnostic(&memory, "\"time_sync_failed\",\"unknown_location\"");
	}
// KCS Diagnostic message testing
//	Refresh_Watchdog;
//	sendBatteryStatus(BAT_FULL);
//	Refresh_Watchdog;
//	sendBatteryStatus(BAT_CHARGING);
//	Refresh_Watchdog;
//	sendBatteryStatus(CHARGER_UNPLUGGED);
//	Refresh_Watchdog;
//	sendBatteryStatus(6);
//	Refresh_Watchdog;

#ifdef EXTERNAL_XPS_STORAGE
	//	XPS_DATA_Init ( &memory , &XPS );
#endif // EXTERNAL_XPS_STORAGE

#ifndef BUZZER
	HAL_GPIO_TogglePin ( Buzzer_GPIO_Port , Buzzer_Pin );
	HAL_Delay ( BUZZER_TIME );
	HAL_GPIO_TogglePin ( Buzzer_GPIO_Port , Buzzer_Pin );
#endif
#endif  // DEV_INIT

	/* USER CODE END 2 */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	while (1)
	{ /* IWDG Timer Restart */
		Refresh_Watchdog;

		HAL_Delay ( 2000 );
		//		memory.State = WAKE_STATE + PIC_UPDT + PIC_SAVE + BUZZER_TOGGLE;
		//		memory.State = WAKE_STATE + PIC_UPDT + PIC_SAVE + UPDATE_SERVER + PIC_SEND + BUZZER_TOGGLE;
		//		memory.State = SLEEP_STATE + PIC_UPDT + PIC_SAVE + UPDATE_SERVER + PIC_SEND + BUZZER_TOGGLE;

		Refresh_Watchdog;

		if (Dev_Mode == DEV_Mode_A || Dev_Mode == DEV_Mode_C)
		{ /* Check if any Flag has Triggered */
			Update_State ( &memory );
		}

		if (Dev_Mode == DEV_Mode_B)
		{ /* Initializes Any Component at Fault */
			Component_Initalizer ( &memory );
			Dev_Mode = Dev_Prev_Mode;
		}

		/* USER CODE END WHILE */

		/* USER CODE BEGIN 3 */
	}
	/* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void)
{
	RCC_OscInitTypeDef RCC_OscInitStruct = {0};
	RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

	/** Configure the main internal regulator output voltage
	 */
	if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE3) != HAL_OK)
	{
		Error_Handler();
	}

	/** Initializes the CPU, AHB and APB buses clocks
	 */

	RCC_OscInitStruct.OscillatorType = 	RCC_OSCILLATORTYPE_LSE;
	RCC_OscInitStruct.LSEState = RCC_LSE_ON;
	//	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI48|RCC_OSCILLATORTYPE_HSI
	//			|RCC_OSCILLATORTYPE_LSI|RCC_OSCILLATORTYPE_MSI
	//			|RCC_OSCILLATORTYPE_MSIK;
	//	RCC_OscInitStruct.HSIState = RCC_HSI_ON;
	//	RCC_OscInitStruct.HSI48State = RCC_HSI48_ON;
	//	RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
	//	RCC_OscInitStruct.LSIState = RCC_LSI_ON;
	//	RCC_OscInitStruct.MSIState = RCC_MSI_ON;
	//	RCC_OscInitStruct.MSICalibrationValue = RCC_MSICALIBRATION_DEFAULT;
	//	RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_4;
	//	RCC_OscInitStruct.LSIDiv = RCC_LSI_DIV1;
	//	RCC_OscInitStruct.MSIKClockRange = RCC_MSIKRANGE_4;
	//	RCC_OscInitStruct.MSIKState = RCC_MSIK_ON;
	//	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
	{
		Error_Handler();
	}

	/** Initializes the CPU, AHB and APB buses clocks
	 */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
			|RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
			|RCC_CLOCKTYPE_PCLK3;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_MSI;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
	RCC_ClkInitStruct.APB3CLKDivider = RCC_HCLK_DIV1;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
	{
		Error_Handler();
	}

	/** Enable the force of MSIK in stop mode
	 */
	//	__HAL_RCC_MSIKSTOP_ENABLE();
}

/**
 * @brief Power Configuration
 * @retval None
 */
static void SystemPower_Config(void)
{
	/*
	 * Disable the internal Pull-Up in Dead Battery pins of UCPD peripheral
	 */
	HAL_PWREx_DisableUCPDDeadBattery();

	/*
	 * PWR Privilege Configuration
	 */
	HAL_PWR_ConfigAttributes(PWR_ALL, PWR_NSEC_PRIV);
	/* PWR_S3WU_IRQn interrupt configuration */
	// Critical do not change
	HAL_NVIC_SetPriority(PWR_S3WU_IRQn, 1, 0);
	HAL_NVIC_EnableIRQ(PWR_S3WU_IRQn);
	/* USER CODE BEGIN PWR */
	//  HAL_NVIC_SetPriority(RTC_IRQn, 0, 0);
	//  HAL_NVIC_EnableIRQ(RTC_IRQn);
	/* USER CODE END PWR */
}

/**
 * @brief ADC1 Initialization Function
 * @param None
 * @retval None
 */
void MX_ADC1_Init(void)
{
	/* USER CODE BEGIN ADC1_Init 0 */

	/* USER CODE END ADC1_Init 0 */

	/* USER CODE BEGIN ADC1_Init 1 */

	/* USER CODE END ADC1_Init 1 */

	/** Common config
	 */
	hadc1.Instance = ADC1;
	hadc1.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV1;
	hadc1.Init.Resolution = ADC_RESOLUTION_12B;
	hadc1.Init.GainCompensation = 0;
	hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
	hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
	hadc1.Init.LowPowerAutoWait = DISABLE;
	hadc1.Init.ContinuousConvMode = DISABLE;
	hadc1.Init.NbrOfConversion = 1;
	hadc1.Init.DiscontinuousConvMode = DISABLE;
	hadc1.Init.DMAContinuousRequests = DISABLE;
	hadc1.Init.TriggerFrequencyMode = ADC_TRIGGER_FREQ_HIGH;
	hadc1.Init.Overrun = ADC_OVR_DATA_PRESERVED;
	hadc1.Init.LeftBitShift = ADC_LEFTBITSHIFT_NONE;
	hadc1.Init.ConversionDataManagement = ADC_CONVERSIONDATA_DR;
	hadc1.Init.OversamplingMode = DISABLE;
	if (HAL_ADC_Init(&hadc1) != HAL_OK)
	{
		Error_Handler();
	}
	/* USER CODE BEGIN ADC1_Init 2 */
	ADC_ChannelConfTypeDef sConfig = {0};

	sConfig.Channel = ADC_CHANNEL_VREFINT;
	sConfig.Rank = ADC_REGULAR_RANK_1;
	sConfig.SamplingTime = ADC_SAMPLETIME_391CYCLES;
	sConfig.Offset = 0;
	HAL_ADC_ConfigChannel(&hadc1, &sConfig);

	/* USER CODE END ADC1_Init 2 */
}

/**
 * @brief CRC Initialization Function
 * @param None
 * @retval None
 */
void MX_CRC_Init(void)
{
	/* USER CODE BEGIN CRC_Init 0 */

	/* USER CODE END CRC_Init 0 */

	/* USER CODE BEGIN CRC_Init 1 */

	/* USER CODE END CRC_Init 1 */
	hcrc.Instance = CRC;
	hcrc.Init.DefaultPolynomialUse = DEFAULT_POLYNOMIAL_ENABLE;
	hcrc.Init.DefaultInitValueUse = DEFAULT_INIT_VALUE_ENABLE;
	hcrc.Init.InputDataInversionMode = CRC_INPUTDATA_INVERSION_NONE;
	hcrc.Init.OutputDataInversionMode = CRC_OUTPUTDATA_INVERSION_DISABLE;
	hcrc.InputDataFormat = CRC_INPUTDATA_FORMAT_BYTES;
	if (HAL_CRC_Init(&hcrc) != HAL_OK)
	{
		Error_Handler();
	}
	/* USER CODE BEGIN CRC_Init 2 */

	/* USER CODE END CRC_Init 2 */
}
/**
 * @brief I2C1 Initialization Function
 * @param None
 * @retval None
 */
void MX_I2C1_Init(void)
{
	/* USER CODE BEGIN I2C1_Init 0 */

	/* USER CODE END I2C1_Init 0 */

	/* USER CODE BEGIN I2C1_Init 1 */

	/* USER CODE END I2C1_Init 1 */
	hi2c1.Instance = I2C1;
	hi2c1.Init.Timing = 0x00000E14;
	hi2c1.Init.OwnAddress1 = 0;
	hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
	hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
	hi2c1.Init.OwnAddress2 = 0;
	hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
	hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
	hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
	if (HAL_I2C_Init(&hi2c1) != HAL_OK)
	{
		Error_Handler();
	}

	/** Configure Analogue filter
	 */
	if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
	{
		Error_Handler();
	}

	/** Configure Digital filter
	 */
	if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
	{
		Error_Handler();
	}
	/* USER CODE BEGIN I2C1_Init 2 */

	/* USER CODE END I2C1_Init 2 */

}

/**
 * @brief ICACHE Initialization Function
 * @param None
 * @retval None
 */
void MX_ICACHE_Init(void)
{
	/* USER CODE BEGIN ICACHE_Init 0 */

	/* USER CODE END ICACHE_Init 0 */

	/* USER CODE BEGIN ICACHE_Init 1 */

	/* USER CODE END ICACHE_Init 1 */

	/** Enable instruction cache in 1-way (direct mapped cache)
	 */
	if (HAL_ICACHE_ConfigAssociativityMode(ICACHE_1WAY) != HAL_OK)
	{
		Error_Handler();
	}
	if (HAL_ICACHE_Enable() != HAL_OK)
	{
		Error_Handler();
	}
	/* USER CODE BEGIN ICACHE_Init 2 */

	/* USER CODE END ICACHE_Init 2 */
}

/**
 * @brief IWDG Initialization Function
 * @param None
 * @retval None
 */
void MX_IWDG_Init(void)
{
	/* USER CODE BEGIN IWDG_Init 0 */

	/* USER CODE END IWDG_Init 0 */

	/* USER CODE BEGIN IWDG_Init 1 */

	/* USER CODE END IWDG_Init 1 */
	hiwdg.Instance = IWDG;
	hiwdg.Init.Prescaler = IWDG_PRESCALER_1024;
	hiwdg.Init.Window = 4095;
	hiwdg.Init.Reload = 4095;
	hiwdg.Init.EWI = 0;
	if (HAL_IWDG_Init(&hiwdg) != HAL_OK)
	{
		Error_Handler();
	}
	/* USER CODE BEGIN IWDG_Init 2 */

	/* USER CODE END IWDG_Init 2 */
}

/**
 * @brief LPTIM1 Initialization Function
 * @param None
 * @retval None
 */
void MX_LPTIM1_Init(void)
{
	/* USER CODE BEGIN LPTIM1_Init 0 */

	/* USER CODE END LPTIM1_Init 0 */

	/* USER CODE BEGIN LPTIM1_Init 1 */

	/* USER CODE END LPTIM1_Init 1 */
	hlptim1.Instance = LPTIM1;
	hlptim1.Init.Clock.Source = LPTIM_CLOCKSOURCE_APBCLOCK_LPOSC;
	hlptim1.Init.Clock.Prescaler = LPTIM_PRESCALER_DIV1;
	hlptim1.Init.Trigger.Source = LPTIM_TRIGSOURCE_SOFTWARE;
	hlptim1.Init.Period = 20;
	hlptim1.Init.UpdateMode = LPTIM_UPDATE_IMMEDIATE;
	hlptim1.Init.CounterSource = LPTIM_COUNTERSOURCE_INTERNAL;
	hlptim1.Init.Input1Source = LPTIM_INPUT1SOURCE_GPIO;
	hlptim1.Init.Input2Source = LPTIM_INPUT2SOURCE_GPIO;
	hlptim1.Init.RepetitionCounter = 0;
	if (HAL_LPTIM_Init(&hlptim1) != HAL_OK)
	{
		Error_Handler();
	}
	/* USER CODE BEGIN LPTIM1_Init 2 */

	/* USER CODE END LPTIM1_Init 2 */
}

/**
 * @brief LPTIM2 Initialization Function
 * @param None
 * @retval None
 */
void MX_LPTIM2_Init(void)
{
	/* USER CODE BEGIN LPTIM2_Init 0 */

	/* USER CODE END LPTIM2_Init 0 */

	/* USER CODE BEGIN LPTIM2_Init 1 */

	/* USER CODE END LPTIM2_Init 1 */
	hlptim2.Instance = LPTIM2;
	hlptim2.Init.Clock.Source = LPTIM_CLOCKSOURCE_APBCLOCK_LPOSC;
	hlptim2.Init.Clock.Prescaler = LPTIM_PRESCALER_DIV1;
	hlptim2.Init.Trigger.Source = LPTIM_TRIGSOURCE_SOFTWARE;
	hlptim2.Init.Period = 65535;
	hlptim2.Init.UpdateMode = LPTIM_UPDATE_IMMEDIATE;
	hlptim2.Init.CounterSource = LPTIM_COUNTERSOURCE_INTERNAL;
	hlptim2.Init.Input1Source = LPTIM_INPUT1SOURCE_GPIO;
	hlptim2.Init.Input2Source = LPTIM_INPUT2SOURCE_GPIO;
	hlptim2.Init.RepetitionCounter = 0;
	if (HAL_LPTIM_Init(&hlptim2) != HAL_OK)
	{
		Error_Handler();
	}
	/* USER CODE BEGIN LPTIM2_Init 2 */

	/* USER CODE END LPTIM2_Init 2 */
}

/**
 * @brief LPTIM3 Initialization Function
 * @param None
 * @retval None
 */
void MX_LPTIM3_Init(void)
{
	/* USER CODE BEGIN LPTIM3_Init 0 */

	/* USER CODE END LPTIM3_Init 0 */

	/* USER CODE BEGIN LPTIM3_Init 1 */

	/* USER CODE END LPTIM3_Init 1 */
	hlptim3.Instance = LPTIM3;
	hlptim3.Init.Clock.Source = LPTIM_CLOCKSOURCE_APBCLOCK_LPOSC;
	hlptim3.Init.Clock.Prescaler = LPTIM_PRESCALER_DIV1;
	hlptim3.Init.Trigger.Source = LPTIM_TRIGSOURCE_SOFTWARE;
	hlptim3.Init.Period = PIR_Percent_Timer;
	hlptim3.Init.UpdateMode = LPTIM_UPDATE_IMMEDIATE;
	hlptim3.Init.CounterSource = LPTIM_COUNTERSOURCE_INTERNAL;
	hlptim3.Init.Input1Source = LPTIM_INPUT1SOURCE_GPIO;
	hlptim3.Init.Input2Source = LPTIM_INPUT2SOURCE_GPIO;
	hlptim3.Init.RepetitionCounter = 0;
	if (HAL_LPTIM_Init(&hlptim3) != HAL_OK)
	{
		Error_Handler();
	}
	/* USER CODE BEGIN LPTIM3_Init 2 */

	/* USER CODE END LPTIM3_Init 2 */
}

/**
 * @brief LPUART1 Initialization Function
 * @param None
 * @retval None
 */
void MX_LPUART1_UART_Init(void)
{
	/* USER CODE BEGIN LPUART1_Init 0 */

	/* USER CODE END LPUART1_Init 0 */

	/* USER CODE BEGIN LPUART1_Init 1 */

	/* USER CODE END LPUART1_Init 1 */
	hlpuart1.Instance = LPUART1;
	hlpuart1.Init.BaudRate = 115200;
	hlpuart1.Init.WordLength = UART_WORDLENGTH_8B;
	hlpuart1.Init.StopBits = UART_STOPBITS_1;
	hlpuart1.Init.Parity = UART_PARITY_NONE;
	hlpuart1.Init.Mode = UART_MODE_TX_RX;
	hlpuart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	hlpuart1.Init.OverSampling = UART_OVERSAMPLING_16;
	//hlpuart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
	//hlpuart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
	//hlpuart1.FifoMode = UART_FIFOMODE_DISABLE;
	if (HAL_UART_Init(&hlpuart1) != HAL_OK)
	{
		Error_Handler();
	}
	if (HAL_UARTEx_SetTxFifoThreshold(&hlpuart1, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
	{
		Error_Handler();
	}
	if (HAL_UARTEx_SetRxFifoThreshold(&hlpuart1, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
	{
		Error_Handler();
	}
	if (HAL_UARTEx_DisableFifoMode(&hlpuart1) != HAL_OK)
	{
		Error_Handler();
	}
	/* USER CODE BEGIN LPUART1_Init 2 */

	/* USER CODE END LPUART1_Init 2 */
}

/**
 * @brief USART2 Initialization Function
 * @param None
 * @retval None
 */
void MX_USART2_UART_Init(void)
{
	/* USER CODE BEGIN USART2_Init 0 */

	/* USER CODE END USART2_Init 0 */

	/* USER CODE BEGIN USART2_Init 1 */

	/* USER CODE END USART2_Init 1 */
	huart2.Instance = USART2;
	huart2.Init.BaudRate = 115200;
	huart2.Init.WordLength = UART_WORDLENGTH_8B;
	huart2.Init.StopBits = UART_STOPBITS_1;
	huart2.Init.Parity = UART_PARITY_NONE;
	huart2.Init.Mode = UART_MODE_TX_RX;
	huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart2.Init.OverSampling = UART_OVERSAMPLING_16;
	huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
	huart2.Init.ClockPrescaler = UART_PRESCALER_DIV1;
	huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
	if (HAL_UART_Init(&huart2) != HAL_OK)
	{
		Error_Handler();
	}
	if (HAL_UARTEx_SetTxFifoThreshold(&huart2, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
	{
		Error_Handler();
	}
	if (HAL_UARTEx_SetRxFifoThreshold(&huart2, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
	{
		Error_Handler();
	}
	if (HAL_UARTEx_DisableFifoMode(&huart2) != HAL_OK)
	{
		Error_Handler();
	}
	/* USER CODE BEGIN USART2_Init 2 */

	/* USER CODE END USART2_Init 2 */
}

/**
 * @brief OCTOSPI1 Initialization Function
 * @param None
 * @retval None
 */
void MX_OCTOSPI1_Init(void)
{
	/* USER CODE BEGIN OCTOSPI1_Init 0 */

	/* USER CODE END OCTOSPI1_Init 0 */

	OSPIM_CfgTypeDef sOspiManagerCfg = {0};
	HAL_OSPI_DLYB_CfgTypeDef HAL_OSPI_DLYB_Cfg_Struct = {0};

	/* USER CODE BEGIN OCTOSPI1_Init 1 */

	/* USER CODE END OCTOSPI1_Init 1 */
	/* OCTOSPI1 parameter configuration*/
	hospi1.Instance = OCTOSPI1;
	hospi1.Init.FifoThreshold = 1;
	hospi1.Init.DualQuad = HAL_OSPI_DUALQUAD_DISABLE;
	hospi1.Init.MemoryType = HAL_OSPI_MEMTYPE_MICRON;
	hospi1.Init.DeviceSize = 24;
	hospi1.Init.ChipSelectHighTime = 1;
	hospi1.Init.FreeRunningClock = HAL_OSPI_FREERUNCLK_DISABLE;
	hospi1.Init.ClockMode = HAL_OSPI_CLOCK_MODE_0;
	hospi1.Init.WrapSize = HAL_OSPI_WRAP_NOT_SUPPORTED;
	hospi1.Init.ClockPrescaler = 1;
	hospi1.Init.SampleShifting = HAL_OSPI_SAMPLE_SHIFTING_NONE;
	hospi1.Init.DelayHoldQuarterCycle = HAL_OSPI_DHQC_DISABLE;
	hospi1.Init.ChipSelectBoundary = 0;
	hospi1.Init.DelayBlockBypass = HAL_OSPI_DELAY_BLOCK_BYPASSED;
	hospi1.Init.MaxTran = 0;
	hospi1.Init.Refresh = 0;
	if (HAL_OSPI_Init(&hospi1) != HAL_OK)
	{
		Error_Handler();
	}
	sOspiManagerCfg.ClkPort = 1;
	sOspiManagerCfg.IOLowPort = HAL_OSPIM_IOPORT_1_LOW;
	if (HAL_OSPIM_Config(&hospi1, &sOspiManagerCfg, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
	{
		Error_Handler();
	}
	HAL_OSPI_DLYB_Cfg_Struct.Units = 0;
	HAL_OSPI_DLYB_Cfg_Struct.PhaseSel = 0;
	if (HAL_OSPI_DLYB_SetConfig(&hospi1, &HAL_OSPI_DLYB_Cfg_Struct) != HAL_OK)
	{
		Error_Handler();
	}
	/* USER CODE BEGIN OCTOSPI1_Init 2 */

	/* USER CODE END OCTOSPI1_Init 2 */
}

/**
 * @brief RTC Initialization Function
 * @param None
 * @retval None
 */
void MX_RTC_Init( void )
{
	/* USER CODE BEGIN RTC_Init 0 */

	/* USER CODE END RTC_Init 0 */

	RTC_PrivilegeStateTypeDef privilegeState = {0};
	RTC_TimeTypeDef sTime = {0};
	RTC_DateTypeDef sDate = {0};
	RTC_AlarmTypeDef sAlarm = {0};

	/* USER CODE BEGIN RTC_Init 1 */

	/* USER CODE END RTC_Init 1 */

	/** Initialize RTC Only
	 */
	hrtc.Instance = RTC;
	hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
	hrtc.Init.AsynchPrediv = 127;
	hrtc.Init.SynchPrediv = 255;
	hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
	hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
	hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
	hrtc.Init.OutPutPullUp = RTC_OUTPUT_PULLUP_NONE;
	hrtc.Init.BinMode = RTC_BINARY_NONE;
	if (HAL_RTC_Init(&hrtc) != HAL_OK)
	{
		Error_Handler();
	}
	privilegeState.rtcPrivilegeFull = RTC_PRIVILEGE_FULL_NO;
	//privilegeState.rtcPrivilegeFeatures = RTC_PRIVILEGE_FEATURE_WUT;
	privilegeState.backupRegisterPrivZone = RTC_PRIVILEGE_BKUP_ZONE_NONE;
	privilegeState.backupRegisterStartZone2 = RTC_BKP_DR0;
	privilegeState.backupRegisterStartZone3 = RTC_BKP_DR0;
	if (HAL_RTCEx_PrivilegeModeSet(&hrtc, &privilegeState) != HAL_OK)
	{
		Error_Handler();
	}

	/* USER CODE BEGIN Check_RTC_BKUP */

	/* USER CODE END Check_RTC_BKUP */

	/** Initialize RTC and set the Time and Date
	 */
	sTime.Hours = 0x23;
	sTime.Minutes = 0x0;
	sTime.Seconds = 0x0;
	sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
	sTime.StoreOperation = RTC_STOREOPERATION_RESET;
	if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BCD) != HAL_OK)
	{
		Error_Handler();
	}
	sDate.WeekDay = RTC_WEEKDAY_WEDNESDAY;
	sDate.Month = RTC_MONTH_AUGUST;
	sDate.Date = 0x31;
	sDate.Year = 0x23;
	if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BCD) != HAL_OK)
	{
		Error_Handler();
	}

	/** Enable the Alarm A
	 */
	sAlarm.AlarmTime.Hours = 0x08;  //GAv changed for testing
	sAlarm.AlarmTime.Minutes = 0x00;  //GAV changed for testing
	sAlarm.AlarmTime.Seconds = 0x0;
	sAlarm.AlarmTime.SubSeconds = 0x0;
	sAlarm.AlarmMask = RTC_ALARMMASK_DATEWEEKDAY|RTC_ALARMMASK_SECONDS|RTC_ALARMMASK_MINUTES;  //GAV changed for testing
	sAlarm.AlarmSubSecondMask = RTC_ALARMSUBSECONDMASK_ALL;
	sAlarm.AlarmDateWeekDaySel = RTC_ALARMDATEWEEKDAYSEL_DATE;
	sAlarm.AlarmDateWeekDay = 0x1;
	sAlarm.Alarm = RTC_ALARM_A;
	if (HAL_RTC_SetAlarm_IT(&hrtc, &sAlarm, RTC_FORMAT_BCD) != HAL_OK)
	{
		Error_Handler();
	}

	/** Enable the Alarm B
	 */
	sAlarm.AlarmTime.Hours = 0x0;
	sAlarm.AlarmTime.Minutes = 0x3;
	sAlarm.AlarmMask = RTC_ALARMMASK_DATEWEEKDAY|RTC_ALARMMASK_HOURS;
	sAlarm.AlarmDateWeekDaySel = RTC_ALARMDATEWEEKDAYSEL_WEEKDAY;
	sAlarm.AlarmDateWeekDay = RTC_WEEKDAY_MONDAY;
	sAlarm.Alarm = RTC_ALARM_B;
	if (HAL_RTC_SetAlarm_IT(&hrtc, &sAlarm, RTC_FORMAT_BCD) != HAL_OK)
	{
		Error_Handler();
	}
	if (HAL_RTCEx_SetWakeUpTimer_IT(&hrtc, 0, RTC_WAKEUPCLOCK_RTCCLK_DIV16, 0) != HAL_OK)
	{
		Error_Handler();
	}
	/* USER CODE BEGIN RTC_Init 2 */
	HAL_RTC_DeactivateAlarm ( &hrtc , RTC_ALARM_B );
	/* USER CODE END RTC_Init 2 */

}

/**
 * @brief SPI2 Initialization Function
 * @param None
 * @retval None
 */
void MX_SPI2_Init(void)
{

	/* USER CODE BEGIN SPI2_Init 0 */

	/* USER CODE END SPI2_Init 0 */

	SPI_AutonomousModeConfTypeDef HAL_SPI_AutonomousMode_Cfg_Struct = {0};

	/* USER CODE BEGIN SPI2_Init 1 */

	/* USER CODE END SPI2_Init 1 */
	/* SPI2 parameter configuration*/
	hspi2.Instance = SPI2;
	hspi2.Init.Mode = SPI_MODE_MASTER;
	hspi2.Init.Direction = SPI_DIRECTION_2LINES;
	hspi2.Init.DataSize = SPI_DATASIZE_8BIT;
	hspi2.Init.CLKPolarity = SPI_POLARITY_LOW;
	hspi2.Init.CLKPhase = SPI_PHASE_1EDGE;
	hspi2.Init.NSS = SPI_NSS_SOFT;
	hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
	hspi2.Init.FirstBit = SPI_FIRSTBIT_MSB;
	hspi2.Init.TIMode = SPI_TIMODE_DISABLE;
	hspi2.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
	hspi2.Init.CRCPolynomial = 0x7;
	hspi2.Init.NSSPMode = SPI_NSS_PULSE_DISABLE;
	hspi2.Init.NSSPolarity = SPI_NSS_POLARITY_LOW;
	hspi2.Init.FifoThreshold = SPI_FIFO_THRESHOLD_01DATA;
	hspi2.Init.MasterSSIdleness = SPI_MASTER_SS_IDLENESS_00CYCLE;
	hspi2.Init.MasterInterDataIdleness = SPI_MASTER_INTERDATA_IDLENESS_00CYCLE;
	hspi2.Init.MasterReceiverAutoSusp = SPI_MASTER_RX_AUTOSUSP_DISABLE;
	hspi2.Init.MasterKeepIOState = SPI_MASTER_KEEP_IO_STATE_ENABLE;
	hspi2.Init.IOSwap = SPI_IO_SWAP_DISABLE;
	hspi2.Init.ReadyMasterManagement = SPI_RDY_MASTER_MANAGEMENT_INTERNALLY;
	hspi2.Init.ReadyPolarity = SPI_RDY_POLARITY_LOW;
	if (HAL_SPI_Init(&hspi2) != HAL_OK)
	{
		Error_Handler();
	}
	HAL_SPI_AutonomousMode_Cfg_Struct.TriggerState = SPI_AUTO_MODE_DISABLE;
	HAL_SPI_AutonomousMode_Cfg_Struct.TriggerSelection = SPI_GRP1_GPDMA_CH0_TCF_TRG;
	HAL_SPI_AutonomousMode_Cfg_Struct.TriggerPolarity = SPI_TRIG_POLARITY_RISING;
	if (HAL_SPIEx_SetConfigAutonomousMode(&hspi2, &HAL_SPI_AutonomousMode_Cfg_Struct) != HAL_OK)
	{
		Error_Handler();
	}
	/* USER CODE BEGIN SPI2_Init 2 */

	/* USER CODE END SPI2_Init 2 */

}

/**
 * @brief USB_OTG_FS Initialization Function
 * @param None
 * @retval None
 */
void MX_USB_OTG_FS_HCD_Init(void)
{

	/* USER CODE BEGIN USB_OTG_FS_Init 0 */

	/* USER CODE END USB_OTG_FS_Init 0 */

	/* USER CODE BEGIN USB_OTG_FS_Init 1 */

	/* USER CODE END USB_OTG_FS_Init 1 */
	hhcd_USB_OTG_FS.Instance = USB_OTG_FS;
	hhcd_USB_OTG_FS.Init.Host_channels = 12;
	hhcd_USB_OTG_FS.Init.speed = HCD_SPEED_FULL;
	hhcd_USB_OTG_FS.Init.dma_enable = DISABLE;
	hhcd_USB_OTG_FS.Init.phy_itface = HCD_PHY_EMBEDDED;
	hhcd_USB_OTG_FS.Init.Sof_enable = DISABLE;
	hhcd_USB_OTG_FS.Init.vbus_sensing_enable = DISABLE;
	if (HAL_HCD_Init(&hhcd_USB_OTG_FS) != HAL_OK)
	{
		Error_Handler();
	}
	/* USER CODE BEGIN USB_OTG_FS_Init 2 */

	/* USER CODE END USB_OTG_FS_Init 2 */

}

/**
 * @brief GPIO Initialization Function
 * @param None
 * @retval None
 */

void MX_GPIO_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	/* USER CODE BEGIN MX_GPIO_Init_1 */
	/* USER CODE END MX_GPIO_Init_1 */

	/* GPIO Ports Clock Enable */
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOD_CLK_ENABLE();

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOC, SD_CS_Pin|Camera_CS_Pin|Cell_Enable_Pin
			|USB_Power_Enable_Pin|Backup_Charge_Enable_Pin, GPIO_PIN_SET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOA, MEM_CS_Pin|Camera_Enable_Pin, GPIO_PIN_SET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOC, Charge_set_1_Pin|BPS_CE_Pin|Charge_Disable_Pin|Sky_Reset_Pin, GPIO_PIN_RESET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOB, USB_Power_Switch_On_Pin|Sky_On_Off_Sw_Pin, GPIO_PIN_SET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOB, Buzzer_Pin|Charge_set_2_Pin|Cell_DVS_Pin, GPIO_PIN_RESET);


//	GPIO_InitTypeDef GPIO_InitStruct = {0};

//    GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1;
//    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
//    GPIO_InitStruct.Pull = GPIO_NOPULL;
//    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
//	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);  // Assuming you're using GPIOA, adjust as necessary


	GPIO_InitStruct.Pin = INT1_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;  // Or as needed (IT_RISING, IT_FALLING, etc.)
	GPIO_InitStruct.Pull = GPIO_NOPULL;  // Or GPIO_PULLDOWN if needed
 //   GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);  // Assuming you're using GPIOA, adjust as necessary
	/*Configure GPIO pin : INT2_Pin */
	GPIO_InitStruct.Pin = INT2_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(INT2_GPIO_Port, &GPIO_InitStruct);

	/*Configure GPIO pins : SD_CS_Pin Sky_Reset_Pin Charge_Disable_Pin Cell_Enable_Pin
                           USB_Power_Enable_Pin Backup_Charge_Enable_Pin */
	GPIO_InitStruct.Pin = SD_CS_Pin|Sky_Reset_Pin|Charge_Disable_Pin|Cell_Enable_Pin
			|USB_Power_Enable_Pin|Backup_Charge_Enable_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

	/*Configure GPIO pin : Sky_Status_Pin */
	GPIO_InitStruct.Pin = Sky_Status_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(Sky_Status_GPIO_Port, &GPIO_InitStruct);

	/*Configure GPIO pin : USB_Power_Good_Pin */
	GPIO_InitStruct.Pin = USB_Power_Good_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(USB_Power_Good_GPIO_Port, &GPIO_InitStruct);

	/*Configure GPIO pin : MEM_CS_Pin */
	GPIO_InitStruct.Pin = MEM_CS_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	HAL_GPIO_Init(MEM_CS_GPIO_Port, &GPIO_InitStruct);

	/*Configure GPIO pin : Charge_set_1_Pin */
	GPIO_InitStruct.Pin = Charge_set_1_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(Charge_set_1_GPIO_Port, &GPIO_InitStruct);

	/*Configure GPIO pin : BPS_CE_Pin */
	GPIO_InitStruct.Pin = BPS_CE_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_PULLDOWN;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(BPS_CE_GPIO_Port, &GPIO_InitStruct);

	/*Configure GPIO pin : USB_Power_Switch_On_Pin */
	GPIO_InitStruct.Pin = USB_Power_Switch_On_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_PULLDOWN;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(USB_Power_Switch_On_GPIO_Port, &GPIO_InitStruct);

	/*Configure GPIO pin : Buzzer_Pin */
	GPIO_InitStruct.Pin = Buzzer_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
	HAL_GPIO_Init(Buzzer_GPIO_Port, &GPIO_InitStruct);

	/*Configure GPIO pin : Charge_set_2_Pin */
	GPIO_InitStruct.Pin = Charge_set_2_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(Charge_set_2_GPIO_Port, &GPIO_InitStruct);

	/*Configure GPIO pin : Camera_CS_Pin */
	GPIO_InitStruct.Pin = Camera_CS_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(Camera_CS_GPIO_Port, &GPIO_InitStruct);

	/*Configure GPIO pin : Cell_Power_Good_Pin */
	GPIO_InitStruct.Pin = Cell_Power_Good_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	HAL_GPIO_Init(Cell_Power_Good_GPIO_Port, &GPIO_InitStruct);

	/*Configure GPIO pin : Camera_Enable_Pin */
	GPIO_InitStruct.Pin = Camera_Enable_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(Camera_Enable_GPIO_Port, &GPIO_InitStruct);

	/*Configure GPIO pin : USB_OverCurrent_Pin */
	GPIO_InitStruct.Pin = USB_OverCurrent_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	HAL_GPIO_Init(USB_OverCurrent_GPIO_Port, &GPIO_InitStruct);

	/*Configure GPIO pins : Sky_On_Off_Sw_Pin Cell_DVS_Pin */
	GPIO_InitStruct.Pin = Sky_On_Off_Sw_Pin|Cell_DVS_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	/*Configure GPIO pin : nExt_Power_Pin */
	GPIO_InitStruct.Pin = nExt_Power_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(nExt_Power_GPIO_Port, &GPIO_InitStruct);

	/**/
	__HAL_SYSCFG_FASTMODEPLUS_ENABLE(SYSCFG_FASTMODEPLUS_PB9);

	/* EXTI interrupt init*/
	HAL_NVIC_SetPriority(EXTI2_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(EXTI2_IRQn);

	HAL_NVIC_SetPriority(EXTI6_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(EXTI6_IRQn);

	HAL_NVIC_SetPriority(EXTI13_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(EXTI13_IRQn);

	/* USER CODE BEGIN MX_GPIO_Init_2 */
	cameraPower = true;
	/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
int  buildMesTester ( char *outputBuff , char *inputBuff )
{
	int messageCounter = 0;
	if ( *(inputBuff) == '\1' )
	{
		messageCounter = 0;
		inputBuff++;
	}

	if ( messageCounter >= 0 )
	{
		while ( *(inputBuff) != '\0' )
		{
			outputBuff [ messageCounter ] = *(inputBuff)++;
			messageCounter += 1;
		}
	}
	return messageCounter;
}

void handleFaults(void)
{
	XPS_paramStore(&memory);
	saveParamDataToFlash(&memory);
}
/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void)
{
	/* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */

	Log_Single ( "\1 ERROR HANDLE \n\r\0" );
	//GAV take out disable IRQ
	__disable_irq ();
	while (1)
	{
	}
	/* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t *file, uint32_t line)
{
	/* USER CODE BEGIN 6 */
	/* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
	/* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
