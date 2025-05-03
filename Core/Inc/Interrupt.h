/*
 * Interrupt.h
 *
 *  Created on: Mar 15, 2023
 *      Author: Erik Pineda-A
 */

#ifndef INTERRUPT_H_
#define INTERRUPT_H_

/******************************************************
 Includes
 Note:
 ******************************************************/

#include "Main.h"
#include "XPS.h"
#include "Skywire.h"
#include "Temperature.h"
#include "Accelerometer.h"
#include "PIR.h"
#include "Camera.h"
#include "Buzzer.h"

/******************************************************
 Define Values
 Note:
 ******************************************************/

#define LOG_DEVICE_INT "\1 Device TIMER Interrupt \r\n\0"
#define LOG_DEVICE_CLOCK_A "\1 ^^^ Alarm A Interrupt ^^^\r\n\0"
#define LOG_DEVICE_CLOCK_B "\1 ^^^ Alarm B Interrupt ^^^\r\n\0"
#define LOG_ACCELEROMTER "\1 Accelerometer Interrupt \r\n\0"
#define LOG_PIN_INT "\1 Device PIN Interrupt \r\n\0"
#define LOG_PIR_INT "\1 Device PIR Interrupt \r\n\0"
#define LOG_POW_INT "\1 Device Power Interrupt \r\n\0"
#define PIR_MOTION_DETECT "\1 *** Motion Detected ***\r\n\0"
#define FALSE_PIR_MOTION_DETECT "\1 *** False Motion Detected ***\r\n\0"
#define MOTION_STILL_DETECT "\1 *** Motion Still Detected ***\r\n\0"
#define MOTION_STOP_DETECT "\1 *** Motion No Longer Detected ***\r\n\0"

/******************************************************
 Global Enum
 Note:
 ******************************************************/

/******************************************************
 Function Prototypes
 Note:
 ******************************************************/

/******************************************************
 Global Shared Variables
 Note:
 ******************************************************/

//extern TIM_HandleTypeDef htim1;
//extern XPS_STATUS_REG XPS;
//extern CELL_STATUS_REG CELL;
//extern TEM_STATUS_REG TEM;
//extern ACC_STATUS_REG ACC;
//extern CAM_STATUS_REG CAM;
extern MEM_PTR memory;
extern IWDG_HandleTypeDef hiwdg;
//extern TIM_HandleTypeDef htim1;
extern RTC_HandleTypeDef hrtc;
extern LPTIM_HandleTypeDef hlptim1;
extern LPTIM_HandleTypeDef hlptim2;
extern LPTIM_HandleTypeDef hlptim3;

#endif /* INTERRUPT_H_ */
