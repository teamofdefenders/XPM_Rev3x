/*
 * Buzzer.c
 *
 *  Created on: Oct 5, 2023
 *      Author: SDITD
 *      Modified by Erik Pineda-A
 */

#ifndef BUZZER_C_
#define BUZZER_C_

#include "Buzzer.h"

/* External variables --------------------------------------------------------*/
extern MEM_PTR memory;
extern LPTIM_HandleTypeDef hlptim1;

/* Private variables ---------------------------------------------------------*/
#define Buzzer_State _Buzzer_Control
#define SET_CLOCK_SCALE LPTIM_PRESCALER_DIV128

//  ********************  Public Functions  ****************************************************
void BUZ_Init ( void )
{
	Log_Single ( "\1 Buzz \r\n\0" );

	if (Buzzer_State & Buzzer_Locked) return;
	else Buzzer_State = Buzzer_Locked + Single_Length_Enabled;

	if (memory.buzzerData.Single_Repeat == 0) Buzzer_State = Buzzer_Locked + Cycles_Length_Enabled;

//	hlptim1.Instance = LPTIM1;
//	hlptim1.Init.Clock.Source = LPTIM_CLOCKSOURCE_APBCLOCK_LPOSC;
//	hlptim1.Init.Clock.Prescaler = SET_CLOCK_SCALE;
//	hlptim1.Init.Trigger.Source = LPTIM_TRIGSOURCE_SOFTWARE;
//	hlptim1.Init.Period = memory.buzzerData.Start_Delay;
//	hlptim1.Init.UpdateMode = LPTIM_UPDATE_IMMEDIATE;
//	hlptim1.Init.CounterSource = LPTIM_COUNTERSOURCE_INTERNAL;
//	hlptim1.Init.Input1Source = LPTIM_INPUT1SOURCE_GPIO;
//	hlptim1.Init.Input2Source = LPTIM_INPUT2SOURCE_GPIO;
//	hlptim1.Init.RepetitionCounter = 0;
//
//	HAL_LPTIM_Init ( &hlptim1 );

//	if (HAL_LPTIM_Counter_Start_IT ( &hlptim1 ) != HAL_OK) Error_Handler ();
}

void BUZ_ReCall ( MEM_PTR *Data_Ptr )
{
	hlptim1.Instance = LPTIM1;
	hlptim1.Init.Clock.Source = LPTIM_CLOCKSOURCE_APBCLOCK_LPOSC;
	hlptim1.Init.Clock.Prescaler = SET_CLOCK_SCALE;
	hlptim1.Init.Trigger.Source = LPTIM_TRIGSOURCE_SOFTWARE;

	if (Buzzer_State & Single_Delay_Enabled) hlptim1.Init.Period = Data_Ptr->buzzerData.Single_Delay;

	else if (Buzzer_State & Single_Length_Enabled) hlptim1.Init.Period = Data_Ptr->buzzerData.Single_Length;

	else if (Buzzer_State & Cycles_Delay_Enabled) hlptim1.Init.Period = Data_Ptr->buzzerData.Cycles_Delay;

	else if (Buzzer_State & Cycles_Length_Enabled) hlptim1.Init.Period = Data_Ptr->buzzerData.Cycles_Length;

	else return;

	hlptim1.Init.UpdateMode = LPTIM_UPDATE_IMMEDIATE;
	hlptim1.Init.CounterSource = LPTIM_COUNTERSOURCE_INTERNAL;
	hlptim1.Init.Input1Source = LPTIM_INPUT1SOURCE_GPIO;
	hlptim1.Init.Input2Source = LPTIM_INPUT2SOURCE_GPIO;
	hlptim1.Init.RepetitionCounter = 0;

	HAL_LPTIM_Init ( &hlptim1 );

	if (HAL_LPTIM_Counter_Start_IT ( &hlptim1 ) != HAL_OK) Error_Handler ();
}

/* Private functions ---------------------------------------------------------*/

#endif  // BUZZER_C_
