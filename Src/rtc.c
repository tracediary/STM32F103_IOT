/**
  ******************************************************************************
  * File Name          : RTC.c
  * Description        : This file provides code for the configuration
  *                      of the RTC instances.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "rtc.h"
/* USER CODE BEGIN 0 */
#include "system.h"

const char TIME_MEMBER_QUEUE[8] = { 3, //year
		1, //month
		2, //date
		4, //hour
		5, //minute
		6, //second
		0 //weekday
		};

/* USER CODE END 0 */

RTC_HandleTypeDef hrtc;

/* RTC init function */
void MX_RTC_Init(void)
{
  RTC_TimeTypeDef sTime = {0};
  RTC_DateTypeDef DateToUpdate = {0};

  /** Initialize RTC Only
  */
  hrtc.Instance = RTC;
	hrtc.Init.AsynchPrediv = RTC_AUTO_1_SECOND;
  hrtc.Init.OutPut = RTC_OUTPUTSOURCE_NONE;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    Error_Handler();
  }

  /* USER CODE BEGIN Check_RTC_BKUP */

  /* USER CODE END Check_RTC_BKUP */

  /** Initialize RTC and set the Time and Date
  */
  sTime.Hours = 0x15;
  sTime.Minutes = 0x25;
  sTime.Seconds = 0x0;

  if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BCD) != HAL_OK)
  {
    Error_Handler();
  }
	DateToUpdate.WeekDay = RTC_WEEKDAY_MONDAY;
  DateToUpdate.Month = RTC_MONTH_MAY;
  DateToUpdate.Date = 0x15;
  DateToUpdate.Year = 0x20;

  if (HAL_RTC_SetDate(&hrtc, &DateToUpdate, RTC_FORMAT_BCD) != HAL_OK)
  {
    Error_Handler();
  }

}

void HAL_RTC_MspInit(RTC_HandleTypeDef* rtcHandle)
{

  if(rtcHandle->Instance==RTC)
  {
  /* USER CODE BEGIN RTC_MspInit 0 */

  /* USER CODE END RTC_MspInit 0 */
    HAL_PWR_EnableBkUpAccess();
    /* Enable BKP CLK enable for backup registers */
    __HAL_RCC_BKP_CLK_ENABLE();
    /* RTC clock enable */
    __HAL_RCC_RTC_ENABLE();

    /* RTC interrupt Init */
    HAL_NVIC_SetPriority(RTC_IRQn, 14, 0);
    HAL_NVIC_EnableIRQ(RTC_IRQn);
    HAL_NVIC_SetPriority(RTC_Alarm_IRQn, 7, 0);
    HAL_NVIC_EnableIRQ(RTC_Alarm_IRQn);
  /* USER CODE BEGIN RTC_MspInit 1 */

		//HAL_RTCEx_SetSecond_IT(rtcHandle);

  /* USER CODE END RTC_MspInit 1 */
  }
}

void HAL_RTC_MspDeInit(RTC_HandleTypeDef* rtcHandle)
{

  if(rtcHandle->Instance==RTC)
  {
  /* USER CODE BEGIN RTC_MspDeInit 0 */

  /* USER CODE END RTC_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_RTC_DISABLE();

    /* RTC interrupt Deinit */
    HAL_NVIC_DisableIRQ(RTC_IRQn);
    HAL_NVIC_DisableIRQ(RTC_Alarm_IRQn);
  /* USER CODE BEGIN RTC_MspDeInit 1 */

  /* USER CODE END RTC_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */

/**************************************************
 *函数功能: 设置RTC初始时间,
 *入参        : 将时间写入寄存器, format: 19700101010101 ( 年月日时分秒)
 *参数说明: 传入缓冲区地址，大小为20Byte
 *返回值   : 无
 **************************************************/
bool RTC_Set_RealTime(uint8_t * ptime)
{
	TimeType_s_t rtc_set_realtime;

	uint8_t * prtc_set_realtime = (uint8_t *) &rtc_set_realtime;
	uint8_t * pptime = ptime + 2;

	int loop = 0;

	while (1)
	{
		prtc_set_realtime[TIME_MEMBER_QUEUE[loop / 3]] = (pptime[loop] - '0') * 10 + (pptime[loop + 1] - '0');
		if (18 == loop)
			goto Set_time;
		loop += 3;
	}
	Set_time:

	if ((HAL_RTC_SetDate(&hrtc, &rtc_set_realtime.RTC_DateStructure, RTC_FORMAT_BIN) == HAL_OK)
			&& HAL_RTC_SetTime(&hrtc, &rtc_set_realtime.RTC_TimeStructure, RTC_FORMAT_BIN) == HAL_OK)
	{
		Debug("Reset real time ok\n");
		return true;
	}

	return false;

}

/**************************************************
 *函数功能: 获取获取RTC实时时间,
 *传入参数: 时间将写入传入的指针指向的地, format : 19700101010101 ( 年月日时分秒)
 *参数说明: 传入缓冲区地址，大小为20Byte
 *返回值    : 无
 **************************************************/
void RTC_GetTime_string(int8_t * Timebuffer)
{
	TimeType_s_t rtc_get_realtime;
	int8_t * prtc_get_realtime = (int8_t *) &rtc_get_realtime;
	int8_t * timebuffer = Timebuffer + 2;
	int loop = 0;

	//Error check
	if ( NULL == Timebuffer)
	{
		Error("\n\rGetTime Input Error");
	}
	else
	{

		loop = 2;
		while (loop--)
		{
			HAL_RTC_GetTime(&hrtc, &rtc_get_realtime.RTC_TimeStructure, RTC_FORMAT_BIN);
			HAL_RTC_GetDate(&hrtc, &rtc_get_realtime.RTC_DateStructure, RTC_FORMAT_BIN);
		}
		loop = 0;
		// Format Transform ( structure 2 string )
		Timebuffer[0] = '2';
		Timebuffer[1] = '0';
		while (12 > loop)
		{
			timebuffer[loop] = prtc_get_realtime[TIME_MEMBER_QUEUE[loop / 2]] / 10 + '0';
			loop++;

			timebuffer[loop] = prtc_get_realtime[TIME_MEMBER_QUEUE[loop / 2]] % 10 + '0';
			loop++;
		}
		timebuffer[loop] = '\0';
		//Debug("\n\r%s  ", Timebuffer);
		//Debug("\n\r RTC_WeekDay : %d", rtc_get_realtime.RTC_DateStructure.WeekDay);

	}
}

/**************************************************
 *函数功能: 获取实时时间到指定结构体中
 *参数说明: 无
 *返回值    : 无
 **************************************************/
void RTC_GetTime_struct(TimeType_s_t * rtc_get_realtime)
{

	int loop = 0;
	loop = 2;
	while (loop--)
	{
		HAL_RTC_GetTime(&hrtc, &rtc_get_realtime->RTC_TimeStructure, RTC_FORMAT_BIN);
		HAL_RTC_GetDate(&hrtc, &rtc_get_realtime->RTC_DateStructure, RTC_FORMAT_BIN);
	}
	loop = 0;

}



void getRealTime(void)
{
	RTC_TimeTypeDef time;
	RTC_DateTypeDef date;

	HAL_RTC_GetTime(&hrtc, &time, RTC_FORMAT_BIN);
	HAL_RTC_GetDate(&hrtc, &date, RTC_FORMAT_BIN);
	printf("time: 20%02d-%02d-%02d,week:%d, %02d:%02d:%02d\n", date.Year, date.Month, date.Date,
			date.WeekDay, time.Hours, time.Minutes, time.Seconds);
}

void HAL_RTCEx_RTCEventCallback(RTC_HandleTypeDef *hrtc)
{
	if (hrtc->Instance == RTC)
	{
		//TODO 秒中断，待思考用做什么业务，和低功耗控制有冲突
	}
}
/* USER CODE END 1 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
