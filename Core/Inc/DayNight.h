/*
 * DayNight.h
 *
 *  Created on: Nov 20, 2024
 *      Author: SDITD - Connor Coultas
 *  @Brief: This header file is used for managing day/night functionality.
 */

#ifndef DAYNIGHT_H_
#define DAYNIGHT_H_

/******************************************************
 Includes
 Note:
 ******************************************************/
#include "Functions.h"
//#include "main.h"


/************************ Private Defines ************************/

//csc default times assuming 9-5 business hours
#define MAX_HOURS 24 //might not need these
#define MAX_MINUTES 60 //might not need these
#define DEFAULT_DAY_HOURS 15 //9 A.M. default daytime start
#define DEFAULT_DAY_MINUTES 30
#define DEFAULT_NIGHT_HOURS 2 //7 P.M. default nighttime start
#define DEFAULT_NIGHT_MINUTES 1
#define DEFAULT_LAST_COOLDOWN 15
/************************ Public Function Prototypes ************************/

void dayNightInit(void); //Initializes business hours to default parameters
void setConfigurationTime(uint8_t dhours, uint8_t dmin, uint8_t nhours, uint8_t nmin, bool dnsEnable);
void storeToXPSArray(void);
void setLastEvent(BUSINESS_DATA_TYPE *extBusinessHours);


#endif /* DAYNIGHT_H_ */
