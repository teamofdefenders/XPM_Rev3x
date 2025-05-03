/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32u5xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define Time_Repetition_CNT 0xFFFF
#define Date_Day 2
#define Time_Counter 0x6FF
#define PIR_CD 3
#define Date_Minute 0
#define Time_PRESCALAR_Counter 0x10
#define Date_Hour 11
#define Alarm_Value_Hours 4
#define Alarm_Value_Minutes 30
#define Date_Year 23
#define PIR_Percent_Timer 0xFF
#define INT2_Pin GPIO_PIN_13
#define INT2_GPIO_Port GPIOC
#define INT2_EXTI_IRQn EXTI13_IRQn
#define SD_CS_Pin GPIO_PIN_2
#define SD_CS_GPIO_Port GPIOC
#define Sky_Status_Pin GPIO_PIN_3
#define Sky_Status_GPIO_Port GPIOC
#define PIR_Motion_Pin GPIO_PIN_0
#define PIR_Motion_GPIO_Port GPIOA
#define USB_Power_Good_Pin GPIO_PIN_1
#define USB_Power_Good_GPIO_Port GPIOA
#define MEM_SCK_Pin GPIO_PIN_3
#define MEM_SCK_GPIO_Port GPIOA
#define MEM_CS_Pin GPIO_PIN_4
#define MEM_CS_GPIO_Port GPIOA
#define INT1_Pin GPIO_PIN_5
#define INT1_GPIO_Port GPIOA
#define Mem_D3_Pin GPIO_PIN_6
#define Mem_D3_GPIO_Port GPIOA
#define Mem_D2_Pin GPIO_PIN_7
#define Mem_D2_GPIO_Port GPIOA
#define Charge_set_1_Pin GPIO_PIN_4
#define Charge_set_1_GPIO_Port GPIOC
#define BPS_CE_Pin GPIO_PIN_5
#define BPS_CE_GPIO_Port GPIOC
#define Mem_D1_Pin GPIO_PIN_0
#define Mem_D1_GPIO_Port GPIOB
#define Mem_DO_Pin GPIO_PIN_1
#define Mem_DO_GPIO_Port GPIOB
#define USB_Power_Switch_On_Pin GPIO_PIN_2
#define USB_Power_Switch_On_GPIO_Port GPIOB
#define Buzzer_Pin GPIO_PIN_12
#define Buzzer_GPIO_Port GPIOB
#define Charge_set_2_Pin GPIO_PIN_13
#define Charge_set_2_GPIO_Port GPIOB
#define Sky_Reset_Pin GPIO_PIN_6
#define Sky_Reset_GPIO_Port GPIOC
#define Camera_CS_Pin GPIO_PIN_7
#define Camera_CS_GPIO_Port GPIOC
#define Charge_Disable_Pin GPIO_PIN_8
#define Charge_Disable_GPIO_Port GPIOC
#define Cell_Power_Good_Pin GPIO_PIN_9
#define Cell_Power_Good_GPIO_Port GPIOC
#define USB_Vbus_Pin GPIO_PIN_9
#define USB_Vbus_GPIO_Port GPIOA
#define Camera_Enable_Pin GPIO_PIN_10
#define Camera_Enable_GPIO_Port GPIOA
#define USB_DM_Pin GPIO_PIN_11
#define USB_DM_GPIO_Port GPIOA
#define USB_DP_Pin GPIO_PIN_12
#define USB_DP_GPIO_Port GPIOA
#define SWDIO_Pin GPIO_PIN_13
#define SWDIO_GPIO_Port GPIOA
#define SWCLK_Pin GPIO_PIN_14
#define SWCLK_GPIO_Port GPIOA
#define Cell_Enable_Pin GPIO_PIN_10
#define Cell_Enable_GPIO_Port GPIOC
#define USB_Power_Enable_Pin GPIO_PIN_11
#define USB_Power_Enable_GPIO_Port GPIOC
#define Backup_Charge_Enable_Pin GPIO_PIN_12
#define Backup_Charge_Enable_GPIO_Port GPIOC
#define USB_OverCurrent_Pin GPIO_PIN_2
#define USB_OverCurrent_GPIO_Port GPIOD
#define USB_OverCurrent_EXTI_IRQn EXTI2_IRQn
#define Sky_On_Off_Sw_Pin GPIO_PIN_5
#define Sky_On_Off_Sw_GPIO_Port GPIOB
#define nExt_Power_Pin GPIO_PIN_6
#define nExt_Power_GPIO_Port GPIOB
#define nExt_Power_EXTI_IRQn EXTI6_IRQn
#define Cell_DVS_Pin GPIO_PIN_9
#define Cell_DVS_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

//	#define PIR_EXTI_IRQn PIR_Motion_EXTI_IRQn
	#define BPS240_Pin BPS_CE_Pin			// Controll for BPS240
	#define BPS240_GPIO_Port BPS_CE_GPIO_Port			// Control for BPS240
	#define PIR_Pin PIR_Motion_Pin
//	#define PIR_Motion_GPIO_Port GPIOC

#define TEMPBUFSIZE 1000
//	#define Alarm_Value 10

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
