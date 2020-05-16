/*
 * time_transform.h
 *
 *  Created on: May 15, 2020
 *      Author: Jason C
 *     version: V1.0.1
 ************************************************************************
 *	 attention: GPLv3, free software
 */

#ifndef USERINC_TIME_TRANSFORM_H_
#define USERINC_TIME_TRANSFORM_H_

#include "stm32f1xx_hal.h"

// 时间结构体
typedef struct
{
	unsigned char second;
	unsigned char minute;
	unsigned char hour;
	unsigned char day;
	unsigned char week;
	unsigned char month;
	unsigned char year;
	unsigned char century;
} DATETIME;

typedef struct date_time
{
	uint16_t iYear;
	uint16_t iMon;
	uint16_t iDay;
	uint16_t iHour;
	uint16_t iMin;
	uint16_t iSec;
	uint16_t iMsec;
} DATE_TIME;


unsigned long TimerSwitch(uint8_t * time_str, uint16_t* weekday);
void GetDateTimeFromUTC(uint8_t * time_UTC, int8_t time_zone);



#endif /* USERINC_TIME_TRANSFORM_H_ */
