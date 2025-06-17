/**
 ******************************************************************************
 * @file    Interrupt.c
 * @author  Erik Pineda-A
 * @brief   To define interrupts
 ******************************************************************************
 * @attention
 *
 * Copyright 2024 Team of Defenders
 *
 ******************************************************************************
 */
#ifndef INTERRUPT_C_
#define INTERRUPT_C_

#include "Interrupt.h"
#include "Functions.h"
#include "DayNight.h"

#ifdef INTERRUPT

#define Buzzer_Mode memory.buzzerData.State
#define Buzzer_State _Buzzer_Control

extern bool pIRTriggered;
extern bool timerTriggered;
extern bool accelTriggered;

extern bool accelWakeupEnabled;
extern bool HBWakeupEnabled;
extern bool movementstop;
extern uint16_t wakeupState;
extern bool accelMuteInit;
extern bool MuteInit;
extern bool pirBlackout;


PIR_PARAMETER_TYPE pirData;
/******************************************************
 Configures Clock Alarm A Interrupt
 Note: Updates acceleration, temperature, and
 humidity and sends heart beat to server.
 ******************************************************/
void HAL_RTC_AlarmAEventCallback ( RTC_HandleTypeDef *hrtc )
{
	Refresh_Watchdog;

#ifdef Log_Level_2
	Log_Single ( LOG_DEVICE_CLOCK_A );
#endif // Log_Level_2

	RTC_AlarmTypeDef sAlarm;

	HAL_RTC_GetAlarm ( hrtc , &sAlarm , RTC_ALARM_A , FORMAT_BIN );

	// reset Heartbeat time is here
	// commented section below is testing, WIP

	//	if ((sAlarm.AlarmTime.Minutes + Alarm_Value_Minutes)> 59)
	//			{
	//		sAlarm.AlarmTime.Minutes = ((sAlarm.AlarmTime.Minutes + Alarm_Value_Minutes) - 60);
	//
	//		if((sAlarm.AlarmTime.Hours + 1) > 23)
	//
	//		{
	//			sAlarm.AlarmTime.Hours = ((sAlarm.AlarmTime.Hours + 1) - 24);
	//		}
	//		else sAlarm.AlarmTime.Hours += 1;
	//			}
	//		else		sAlarm.AlarmTime.Minutes += (Alarm_Value_Minutes);
	//
	//	if ((sAlarm.AlarmTime.Hours + Alarm_Value_Hours)> 23)
	//	{
	//		sAlarm.AlarmTime.Hours = ((sAlarm.AlarmTime.Hours + Alarm_Value_Hours) - 24);
	//	}
	//	else sAlarm.AlarmTime.Hours += Alarm_Value_Hours;

	//	sAlarm.AlarmTime.Minutes += Alarm_Value_Minutes;

	sAlarm.AlarmTime.Hours += Heartbeat_Default_Hours;

	if (sAlarm.AlarmTime.Hours > 24)
	{
		sAlarm.AlarmTime.Hours = (sAlarm.AlarmTime.Hours - 24);
	}

	//KCS Possible code hang here
	while (HAL_RTC_SetAlarm_IT ( hrtc , &sAlarm , FORMAT_BIN ) != HAL_OK);

	switch (memory.Mode)
	{
	case DEV_Mode_A:
		memory.State |= WAKE_STATE + ACCEL_UPDT + HUMD_UPDT + TEMP_UPDT + UPDATE_SERVER;
		break;
	case DEV_Mode_B:
		break;
	case DEV_Mode_C:
		memory.State |= WAKE_STATE + ACCEL_UPDT + HUMD_UPDT + TEMP_UPDT + UPDATE_SERVER;
		break;
	case DEV_Mode_D:
		break;
	case DEV_Mode_E:
		break;
	}
}

/******************************************************
 Configures Clock Alarm B Interrupt
 Note: Used for PIR state configure
 ******************************************************/
//void HAL_RTCEx_AlarmBEventCallback ( RTC_HandleTypeDef *hrtc )
//{
//#ifdef Log_Level_2
//	Log_Single ( LOG_DEVICE_CLOCK_B );
//#endif // Log_Level_2
//
//	if (pirData.control & CONFIRM_MOTION)
//	{
//		if (pirData.percent >= Perecent_Threshold)
//		{
//			pirData.control = MOTION_BLACKOUT;
//
//			switch (memory.Mode)
//			{
//			case DEV_Mode_A:
//				Buzzer_Mode = Long;
//				memory.State |= WAKE_STATE + PIR_UPDT + PIR_START + BUZZER_TOGGLE;
//				break;
//			case DEV_Mode_B:
//				break;
//			case DEV_Mode_C:
//				Buzzer_Mode = Short;
//				memory.State |= WAKE_STATE + PIC_UPDT + PIC_SEND + PIR_UPDT + PIR_START + GPS_UPDT + BUZZER_TOGGLE;
//				break;
//			case DEV_Mode_D:
//				break;
//			case DEV_Mode_E:
//				break;
//			}
//
//			//			HAL_NVIC_DisableIRQ ( PIR_EXTI_IRQn );
//			//			PIR_ReCall ( &memory );
//
//#ifdef Log_Level_0
//			Log_Single ( PIR_MOTION_DETECT );
//#endif // Log_Level_0
//		}
//		else
//		{
//			//			setPRITrigger ();
//#ifdef Log_Level_0
//			Log_Single ( FALSE_PIR_MOTION_DETECT );
//#endif // Log_Level_0
//		}
//	}
//	else if (pirData.control & MOTION_BLACKOUT)
//	{
//		pirData.counter++;
//		pirData.control = CONFIRM_NO_MOTION;
//
//		//		setPRITrigger ();
//		PIR_ReCall ( &memory );
//	}
//	else if (pirData.control & CONFIRM_NO_MOTION)
//	{
//		if (1)
//		{
//			//				HAL_NVIC_DisableIRQ ( PIR_EXTI_IRQn );
//			pirData.control = CONFIRM_MOTION;
//		}
//		else
//		{
//			if (memory.State & PIR_Toggled || Is_Pin_High( PIR_Motion_GPIO_Port , PIR_Motion_Pin ))
//			{
//				pirData.control = MOTION_BLACKOUT;
//				// kcs maybe stop this for the demo
//				//					HAL_NVIC_DisableIRQ ( PIR_EXTI_IRQn );
//				PIR_ReCall ( &memory );
//#ifdef Log_Level_0
//				Log_Single ( MOTION_STILL_DETECT );
//#endif // Log_Level_0
//			}
//			else
//			{
//				pirData.control = CONFIRM_MOTION;
//				//				setPRITrigger ();
//#ifdef Log_Level_0
//				Log_Single ( MOTION_STOP_DETECT );
//#endif // Log_Level_0
//			}
//		}
//	}
//}


void HAL_PWREx_S3WUCallback(uint32_t WakeUpPin)
{

	HAL_RTCEx_DeactivateWakeUpTimer(&hrtc);
	HAL_ResumeTick();
	HAL_Init();

	HAL_PWR_DisableWakeUpPin ( GPIO_PIN_1 );
	HAL_PWR_DisableWakeUpPin ( GPIO_PIN_0 );
	HAL_PWR_DisableWakeUpPin ( PWR_WAKEUP_PIN6 );
	HAL_PWR_DisableWakeUpPin ( PWR_WAKEUP_PIN6_HIGH_1 );
	HAL_NVIC_DisableIRQ(PWR_S3WU_IRQn);
	HAL_NVIC_DisableIRQ(EXTI0_IRQn);  //PIR
	//	HAL_NVIC_DisableIRQ(EXTI1_IRQn);  //PIR
	HAL_NVIC_DisableIRQ(EXTI6_IRQn);  //Accelerometer

	SystemClock_Config();
	MX_GPIO_Init();
	//		MX_ADC1_Init();
	MX_I2C1_Init();
	MX_OCTOSPI1_Init();
	MX_USART2_UART_Init();
	MX_SPI2_Init();
	MX_LPUART1_UART_Init();
	//		MX_LPTIM1_Init();
	MX_CRC_Init();
	//		MX_LPTIM2_Init();
	//		MX_LPTIM3_Init();
	//		PACKAGE_Init( &memory );
	LOG_Init();


	//		HAL_NVIC_DisableIRQ(PWR_S3WU_IRQn);

	//		PRINTF("PIR wakeup initiated\r\n");

	//	    __HAL_PWR_CLEAR_FLAG(PWR_WAKEUP_FLAG1);  // Clear wakeup interrupt flag
	__HAL_PWR_CLEAR_FLAG(PWR_FLAG_STOPF);  // Clear wakeup interrupt flag



	if (WakeUpPin == GPIO_PIN_0)  //PIR
	{
		memory.State |= WAKE_STATE + PIC_UPDT + PIC_SEND + PIR_UPDT + PIR_START + BUZZER_TOGGLE;
		if (memory.State & UPDATE_SERVER)
		{
			memory.State ^= UPDATE_SERVER;
		}
		pIRTriggered = true;

		CLEAR_REG(PWR->WUSR);

		//			HAL_GPIO_WritePin ( Buzzer_GPIO_Port , Buzzer_Pin , GPIO_PIN_SET );          //for simulated battery power operation
	}



	else if (WakeUpPin == GPIO_PIN_6)  // accelerometer
	{
		memory.State |= WAKE_STATE + HB_UPDT + MOVEMENT_UPDT + GPS_UPDT;
		if (memory.State & UPDATE_SERVER)
		{
			memory.State ^= UPDATE_SERVER;
		}
		if (memory.State & PIC_UPDT )
		{
			memory.State ^= PIC_UPDT;
		}

		accelTriggered = true;

		CLEAR_REG(PWR->WUSR);

		//			HAL_GPIO_WritePin ( Buzzer_GPIO_Port , Buzzer_Pin , GPIO_PIN_SET );          //for simulated battery power operation
	}
    // need to add temperature and charger connected/disconnected interrupts here


}

void HAL_RTCEx_WakeUpTimerEventCallback(RTC_HandleTypeDef *hrtc)
{

	//	HAL_NVIC_DisableIRQ(PWR_S3WU_IRQn);
	//	HAL_NVIC_DisableIRQ(EXTI0_IRQn);
	CLEAR_REG(RTC->ICSR);
	CLEAR_REG(RTC->WUTR);
	CLEAR_REG(RTC->CR);
	CLEAR_REG(PWR->WUSCR);
	//
	__HAL_RTC_CLEAR_FLAG(&hrtc, RTC_FLAG_WUTF);  // Clear WUTF flag

	HAL_RTCEx_DeactivateWakeUpTimer(hrtc);
	HAL_ResumeTick();
	HAL_Init();
	SystemClock_Config();
	MX_GPIO_Init();
	//		MX_ADC1_Init();
	MX_I2C1_Init();
	MX_OCTOSPI1_Init();
	MX_USART2_UART_Init();
	MX_SPI2_Init();
	MX_LPUART1_UART_Init();
	//		MX_LPTIM1_Init();
	MX_CRC_Init();
	//		MX_LPTIM2_Init();
	//		MX_LPTIM3_Init();
	//		PACKAGE_Init( &memory );
	LOG_Init();

	HAL_NVIC_DisableIRQ(PWR_S3WU_IRQn);
	HAL_NVIC_DisableIRQ(RTC_IRQn);


	//		PACKAGE_Init( &memory );
	//	LOG_Init();

	//	PRINTF("Timer wakeup initiated\r\n");
	PRINTF("HBWakeupEnabled is %d, accelWakeupEnabled is %d\r\n",HBWakeupEnabled, accelWakeupEnabled);

	//we need to get rid of this method that we are setting the states with
	if (wakeupState == hb_wu_enabled)
	{
		memory.State |= WAKE_STATE + HB_UPDT + GPS_UPDT;
		if (memory.State & UPDATE_SERVER)
		{
			memory.State ^= UPDATE_SERVER;
		}
		if (memory.State & PIC_UPDT )
		{
			memory.State ^= PIC_UPDT;
		}

	}
	else if (wakeupState == acc_wu_enabled)
	{
		memory.State |= WAKE_STATE + HB_UPDT + MOVEMENT_UPDT + GPS_UPDT;  //movement stop
		if (memory.State & UPDATE_SERVER)
		{
			memory.State ^= UPDATE_SERVER;
		}
		if (memory.State & PIC_UPDT )
		{
			memory.State ^= PIC_UPDT;
		}
		accelMuteInit = false;
		movementstop = true;
	}
	else if (wakeupState == img_wu_enabled)
	{
		memory.State |= WAKE_STATE + PIC_UPDT + PIC_SEND + PIR_END;  //motion stop
		if (memory.State & UPDATE_SERVER)
		{
			memory.State ^= UPDATE_SERVER;
		}
		MuteInit = false;
	}
	else if (wakeupState == pGPS_wu_enabled)
	{
		memory.State |= WAKE_STATE + GPS_UPDT;  //motion stop
		if (memory.State & UPDATE_SERVER)
		{
			memory.State ^= UPDATE_SERVER;
		}

	}

	else if (wakeupState == blackout_wu_enabled)
	{
		memory.State |= WAKE_STATE;
		if (memory.State & UPDATE_SERVER)
		{
			memory.State ^= UPDATE_SERVER;
		}
		pirBlackout = false;

	}
	else if (wakeupState == hb_img_pair)
	{
		memory.State |= WAKE_STATE + HB_UPDT  + GPS_UPDT + PIC_UPDT + PIC_SEND + PIR_END;
		if (memory.State & UPDATE_SERVER)
		{
			memory.State ^= UPDATE_SERVER;
		}
		MuteInit = false;

	}
	else if (wakeupState == hb_acc_pair)
	{
		memory.State |= WAKE_STATE + HB_UPDT + MOVEMENT_UPDT + GPS_UPDT;
		if (memory.State & UPDATE_SERVER)
		{
			memory.State ^= UPDATE_SERVER;
		}
		accelMuteInit = false;
		movementstop = true;

	}
	else if (wakeupState == hb_pGPS_pair)
	{
		memory.State |= WAKE_STATE + HB_UPDT + GPS_UPDT;
		if (memory.State & UPDATE_SERVER)
		{
			memory.State ^= UPDATE_SERVER;
		}


	}

	else if (wakeupState == hb_blackout_pair)
	{
		memory.State |= WAKE_STATE + HB_UPDT;
		if (memory.State & UPDATE_SERVER)
		{
			memory.State ^= UPDATE_SERVER;
		}
		pirBlackout = false;

	}
	else if (wakeupState == img_acc_pair)
	{
		memory.State |= WAKE_STATE + HB_UPDT + MOVEMENT_UPDT + GPS_UPDT + PIC_UPDT + PIC_SEND + PIR_END;
		if (memory.State & UPDATE_SERVER)
		{
			memory.State ^= UPDATE_SERVER;
		}
		movementstop = true;
		accelMuteInit = false;
		MuteInit = false;

	}
	else if (wakeupState == img_pGPS_pair)
	{
		memory.State |= WAKE_STATE + GPS_UPDT + PIC_UPDT + PIC_SEND;
		if (memory.State & UPDATE_SERVER)
		{
			memory.State ^= UPDATE_SERVER;
		}

		MuteInit = false;

	}

	else if (wakeupState == img_blackout_pair)
	{
		memory.State |= WAKE_STATE + PIC_UPDT + PIC_SEND;
		if (memory.State & UPDATE_SERVER)
		{
			memory.State ^= UPDATE_SERVER;
		}

		MuteInit = false;
		pirBlackout = false;


	}

	else if (wakeupState == acc_blackout_pair)
	{
		memory.State |= WAKE_STATE + MOVEMENT_UPDT + GPS_UPDT;
		if (memory.State & UPDATE_SERVER)
		{
			memory.State ^= UPDATE_SERVER;
		}

		movementstop = true;
		accelMuteInit = false;
		pirBlackout = false;


	}

	else if (wakeupState == pGPS_blackout_pair)
	{
		memory.State |= WAKE_STATE + GPS_UPDT;
		if (memory.State & UPDATE_SERVER)
		{
			memory.State ^= UPDATE_SERVER;
		}

		pirBlackout = false;


	}

	else if (wakeupState == hb_img_acc_pair)
	{
		memory.State |= WAKE_STATE + HB_UPDT + MOVEMENT_UPDT + GPS_UPDT + PIC_UPDT + PIC_SEND + PIR_END;
		if (memory.State & UPDATE_SERVER)
		{
			memory.State ^= UPDATE_SERVER;
		}
		movementstop = true;
		accelMuteInit = false;
		MuteInit = false;

	}
	else if (wakeupState == hb_img_pGPS_pair)
	{
		memory.State |= WAKE_STATE + HB_UPDT + GPS_UPDT + PIC_UPDT + PIC_SEND;
		if (memory.State & UPDATE_SERVER)
		{
			memory.State ^= UPDATE_SERVER;
		}

		MuteInit = false;

	}

	else if (wakeupState == hb_img_blackout_wu_enabled)
	{
		memory.State |= WAKE_STATE + HB_UPDT + PIC_UPDT + PIC_SEND;
		if (memory.State & UPDATE_SERVER)
		{
			memory.State ^= UPDATE_SERVER;
		}

		MuteInit = false;
		pirBlackout = false;

	}

	else if (wakeupState == hb_acc_blackout_wu_enabled)
	{
		memory.State |= WAKE_STATE + HB_UPDT + MOVEMENT_UPDT + GPS_UPDT;
		if (memory.State & UPDATE_SERVER)
		{
			memory.State ^= UPDATE_SERVER;
		}

		movementstop = true;
		accelMuteInit = false;
		pirBlackout = false;

	}

	else if (wakeupState == hb_pGPS_blackout_wu_enabled)
	{
		memory.State |= WAKE_STATE + HB_UPDT + GPS_UPDT;
		if (memory.State & UPDATE_SERVER)
		{
			memory.State ^= UPDATE_SERVER;
		}

		pirBlackout = false;

	}

	else if (wakeupState == img_acc_blackout_wu_enabled)
	{
		memory.State |= WAKE_STATE + PIC_UPDT + PIC_SEND + MOVEMENT_UPDT + GPS_UPDT;
		if (memory.State & UPDATE_SERVER)
		{
			memory.State ^= UPDATE_SERVER;
		}

		MuteInit = false;
		movementstop = true;
		accelMuteInit = false;
		pirBlackout = false;

	}

	else if (wakeupState == img_pGPS_blackout_wu_enabled)
	{
		memory.State |= WAKE_STATE + PIC_UPDT + PIC_SEND + GPS_UPDT;
		if (memory.State & UPDATE_SERVER)
		{
			memory.State ^= UPDATE_SERVER;
		}

		MuteInit = false;
		pirBlackout = false;

	}

	else if (wakeupState == hb_img_acc_blackout_wu_enabled)
	{
		memory.State |= WAKE_STATE + HB_UPDT + PIC_UPDT + PIC_SEND + MOVEMENT_UPDT + GPS_UPDT;
		if (memory.State & UPDATE_SERVER)
		{
			memory.State ^= UPDATE_SERVER;
		}

		MuteInit = false;
		movementstop = true;
		accelMuteInit = false;
		pirBlackout = false;

	}

	else if (wakeupState == hb_img_pGPS_blackout_wu_enabled)
	{
		memory.State |= WAKE_STATE + HB_UPDT + PIC_UPDT + PIC_SEND + GPS_UPDT;
		if (memory.State & UPDATE_SERVER)
		{
			memory.State ^= UPDATE_SERVER;
		}

		MuteInit = false;
		pirBlackout = false;

	}

	CLEAR_REG(PWR->WUSR);
	timerTriggered = true;
}

/**
 * @brief EXTI line detection callbacks
 * @param GPIO_Pin: Specifies the pins connected EXTI line
 * @retval None
 */
void HAL_GPIO_EXTI_Rising_Callback ( uint16_t GPIO_Pin )
{

//	if (GPIO_Pin == PIR_Pin)
//	{
//		//		HAL_NVIC_DisableIRQ ( PIR_EXTI_IRQn );	// Turn off event
//
//#ifdef Log_Level_2
//		Log_Single ( LOG_PIR_INT );
//
//#endif // Log_Level_2
//		// KCS we need to look at system time and decide when to update GPS data/time
//		// Much like same manner as Heartbeat which Gage is working on
//		Buzzer_Mode = Short;
//		//	memory.State |= WAKE_STATE + PIC_UPDT + PIC_SEND + PIR_UPDT + PIR_START + GPS_UPDT + BUZZER_TOGGLE;;
//		//		memory.State |= WAKE_STATE + BUZZER_TOGGLE;
//
//		if(isNight2())
//		{
//			PRINTF("Night detected in PIR Interrupt handling function\r\n");
//			memory.State |= WAKE_STATE + PIC_UPDT + PIC_SEND + PIR_UPDT + PIR_START + BUZZER_TOGGLE;
//		}
//		else
//		{
//			PRINTF("Daytime motion detected.\r\n");
//			WriteDataArray(PIR_TAG, PIR_SPECIFIER, "\"Start Motion\"");
//			//PrintDataArray();
//			memory.State |= WAKE_STATE;
//			//pirData.Control = CONFIRM_MOTION;
//			//__HAL_LPTIM_CLEAR_FLAG(&hlptim3, LPTIM_FLAG_ARRM);
//			//__HAL_LPTIM_CLEAR_FLAG(&hlptim3, LPTIM_FLAG_EXTTRIG);
//		}
//
//		//
//		//		Buzzer_Mode = Short;
//		////		memory.State |= WAKE_STATE + BUZZER_TOGGLE;
//		//
//		//		if (!(pirData.Control & PIR_CLOCK_STARTED))
//		//		{
//		//			PIR_ReCall ( &memory );
//		//			pirData.Control |= PIR_CLOCK_STARTED;
//		//		}
//		//		if (!(pirData.Control & PIR_PERCENT_STARTED))
//		//		{
//		//			pirData.Control |= PIR_PERCENT_STARTED;
//		//			pirData.Counter = 0;
//		//
//		//			hlptim3.Init.Period = PIR_Percent_Timer;
//		//			HAL_LPTIM_Init ( &hlptim3 );
//		//			HAL_LPTIM_Counter_Start_IT ( &hlptim3 );
//		//		}
//		//
//		//		else pirData.Control |= PIR_Toggled;
//
//	}
//
//
//	else if (GPIO_Pin == nExt_Power_Pin)
//	{
//		bool GageDemo = true;
//		if (!GageDemo)
//		{
//			if (memory.State & CHARGER_DETECT) memory.State ^= CHARGER_DETECT;
//		}
//	}
}

/**
 * @brief EXTI line detection callbacks
 * @param GPIO_Pin: Specifies the pins connected EXTI line
 * @retval None
 */
void HAL_GPIO_EXTI_Falling_Callback ( uint16_t GPIO_Pin )
{
//	// INT1 Pin & INT2 Pin Call
//	bool GageDemo = true;
//	if ((GPIO_Pin == INT1_Pin) || (GPIO_Pin == INT2_Pin))
//	{
//		memory.State |= GPIO_UPDT;
//
//#ifdef Log_Level_2
//		Log_Single ( LOG_PIN_INT );
//#endif  // Log_Level_2
//
//		switch (memory.Mode)
//		{
//		case DEV_Mode_A:
//			memory.State |= ACCEL_UPDT + HUMD_UPDT + TEMP_UPDT;
//			break;
//		case DEV_Mode_B:
//			memory.State |= GPS_UPDT;
//			break;
//		case DEV_Mode_C:
//			memory.State |= ACCEL_UPDT + HUMD_UPDT + TEMP_UPDT;
//			//				memory.State = SLEEP_STATE;
//			break;
//		case DEV_Mode_D:
//			break;
//		case DEV_Mode_E:
//			break;
//		}
//	}
//
//	else if (GPIO_Pin == nExt_Power_Pin)
//	{
//		if (!GageDemo)
//		{
//			Buzzer_Mode = Short;
//			memory.State |= WAKE_STATE + BUZZER_TOGGLE;
//#ifdef Log_Level_2
//			Log_Single ( LOG_POW_INT );
//#endif // Log_Level_2
//			memory.State ^= CHARGER_DETECT;
//		}
//	}
}

/******************************************************
 Configures UART RX Flag
 Note:
 ******************************************************/
void HAL_UART_RxCpltCallback ( UART_HandleTypeDef *huart )
{
	//#ifdef Log_Level_2
	//	Log_Single ("\1 CELL UART Interrupt \r\n\0");
	//#endif

	if (huart->Instance == LPUART1)
	{
		//		memory.State |= WAKE_STATE + SERVER_COMMAND;
		//	receivePicAckUart( &memory );
	}
	//HAL_UART_Receive
}

//void HAL_LPTIM_AutoReloadMatchCallback ( LPTIM_HandleTypeDef *hlptim )
//{
//	if (hlptim->Instance == LPTIM1)
//	{
//		if (HAL_LPTIM_Counter_Stop_IT ( &hlptim1 ) != HAL_OK) Error_Handler ();
//
//		if (!(Buzzer_State & Buzzer_Locked)) return;
//
//		if (memory.buzzerData.Single_Repeat)
//		{
//			if (Buzzer_State & Single_Length_Enabled) Buzzer_State = Buzzer_Locked + Single_Delay_Enabled;
//
//			else if (Buzzer_State & Single_Delay_Enabled)
//			{
//				Buzzer_State = Buzzer_Locked + Single_Length_Enabled;
//				if (--memory.buzzerData.Single_Repeat <= 0)
//				{
//					Buzzer_State ^= Single_Length_Enabled;
//					if (memory.buzzerData.Cycles_Repeat)
//					{
//						Buzzer_State = Buzzer_Locked + Cycles_Length_Enabled;
//					}
//					else Buzzer_State = Buzzer_Reset;
//				}
//			}
//		}
//		else if (memory.buzzerData.Cycles_Repeat)
//		{
//			if (Buzzer_State & Cycles_Length_Enabled) Buzzer_State = Buzzer_Locked + Cycles_Delay_Enabled;
//
//			else if (Buzzer_State & Cycles_Delay_Enabled)
//			{
//				Buzzer_State = Buzzer_Locked + Cycles_Length_Enabled;
//				if (--memory.buzzerData.Cycles_Repeat <= 0)
//				{
//					Buzzer_State = Buzzer_Locked + Single_Length_Enabled;
//				}
//				else Buzzer_State = Buzzer_Reset;
//			}
//		}
//		else
//		{
//			if (Buzzer_State & Buzzer_Toggle)
//			{
//#ifdef BUZZER
//				HAL_GPIO_TogglePin ( Buzzer_GPIO_Port , Buzzer_Pin );
//#endif // BUZZER
//			}
//
//			Buzzer_State = Buzzer_Reset;
//		}
//#ifdef BUZZER
//		HAL_GPIO_TogglePin ( Buzzer_GPIO_Port , Buzzer_Pin );
//#endif // BUZZER
//		Buzzer_State ^= Buzzer_Toggle;
//
//		BUZ_ReCall ( &memory );
//	}
//
//	if (hlptim->Instance == LPTIM2)
//	{
//		Log_Single ( "\1 LPTIM2 Trig &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&\0" );
//		HAL_LPTIM_Counter_Stop_IT ( &hlptim2 );
//	}
//
//	if (hlptim->Instance == LPTIM3)
//	{
//		Log_Single ( "\1 LPTIM3 Trig &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&\0" );
//		if (HAL_LPTIM_Counter_Stop_IT ( &hlptim3 ) != HAL_OK) Error_Handler ();
//
//		if (pirData.control & PIR_Toggled || Is_Pin_High( PIR_Motion_GPIO_Port , PIR_Motion_Pin ))
//		{
//			if (pirData.control & PIR_Toggled) pirData.control ^= PIR_Toggled;
//
//			pirData.percent++;
//		}
//
//		if (++pirData.counter <= PIR_COUNTER_LIM)
//		{
//			hlptim3.Init.Period = PIR_Percent_Timer;
//			HAL_LPTIM_Init ( &hlptim3 );
//			HAL_LPTIM_Counter_Start_IT ( &hlptim3 );
//		}
//		//	memory.State |= Hello;
//	}
//}

void HAL_IWDG_EarlyWakeupCallback ( IWDG_HandleTypeDef *hiwdg )
{
	Log_Single ( "\1 WDT Trig &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&\0" );
}
#endif // INTERRUPT

#endif  // INTERRUPT_C_
