/*
 * Buzzer.h
 *
 *  Created on: Oct 5, 2023
 *      Author: SDITD
 */

#ifndef BUZZER_H_
#define BUZZER_H_

/* Includes ------------------------------------------------------------------*/
#include <string.h>
#include <stdlib.h>
#include "Functions.h"

/* Private typedef -----------------------------------------------------------*/
typedef enum
{										// Type definitions for buzzer command
	Short = 0,							//
	Short_Spam,							//
	Long,								//
	Long_Spam,				 			//
	Custom,								//
} BuzzerState;

/* Private define ------------------------------------------------------------*/
#define Buzzer_Reset		 	0b00000000
#define Buzzer_Enabled 			0b00000001
#define Single_Delay_Enabled 	0b00000010
#define Single_Length_Enabled 	0b00000100
#define Cycles_Delay_Enabled 	0b00001000
#define Cycles_Length_Enabled 	0b00010000
#define Buzzer_Locked		 	0b00100000
#define Buzzer_Toggle		 	0b01000000

// Pin Redefine
#define BUZZER_PIN Buzzer_Pin
#define BUZZER_GPIO_PORT Buzzer_GPIO_Port

/* Private macro -------------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/
void BUZ_Init ( void );
void BUZ_ReCall ( MEM_PTR *Data_Ptr );
void HAL_LPTIM_TriggerCallback ( LPTIM_HandleTypeDef *hlptim );

#endif /* BUZZER_H_ */
