/*
 * time_transform.c
 *
 *  Created on: May 15, 2020
 *      Author: Jason C
 *     version: V1.0.1
 ************************************************************************
 *	 attention: GPLv3, free software
 */


#include "time_transform.h"
#include "stdio.h"
#include "string.h"
#include "Debug.h"

#define OFFSET_SECOND   946684800
#define SECOND_OF_DAY   86400

const uint8_t DayWeek[7] = { RTC_WEEKDAY_THURSDAY, RTC_WEEKDAY_FRIDAY, RTC_WEEKDAY_SATURDAY, RTC_WEEKDAY_SUNDAY,
		RTC_WEEKDAY_MONDAY, RTC_WEEKDAY_TUESDAY, RTC_WEEKDAY_WEDNESDAY };
const uint8_t DayOfMon[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
#if 0
const unsigned char Monthday[13] =
{ //当前月份对应的天数  当月的天数不算
	[0] = 0, [1] = 0,

	[2] = 31,

	[3] = 59,

	[4] = 90,

	[5] = 120,

	[6] = 151,

	[7] = 181,

	[8] = 212,

	[9] = 243,

	[10] = 273,

	[11] = 304,

	[12] = 334};
#endif
/*******************************************************************
 *  函数功能：将当前的格林威治时间(如：15-11-9,09:49:25)转换为格林威治秒数  格林威治时间：从1970年1月1日开始到现在的秒数
 *  输入参数：time_str 当前时间
 *  输出结果：UTC时间(单位：秒)
 */
unsigned long TimerSwitch(uint8_t * time_str, uint16_t* weekday)
{
	DATETIME curData;
	uint8_t test_time[50];
	unsigned short curyear; // 当前年份 16位
	int cyear = 0; // 当前年和1970年的差值
	int cday = 0; // 差值年转变为天数
	int curmonthday = 0; // 当前月份转变为天数
#if 0
	//2018-09-05,09:19:13
	//0123456789012345678
	curData.century = 20;
	curData.year = (unsigned char) ((time_str[2] - '0') * 10 + (time_str[3] - '0'));
	curData.month = (unsigned char) ((time_str[5] - '0') * 10 + (time_str[6] - '0'));
	curData.day = (unsigned char) ((time_str[8] - '0') * 10 + (time_str[9] - '0'));

	curData.hour = (unsigned char) ((time_str[11] - '0') * 10 + (time_str[12] - '0'));

	curData.minute = (unsigned char) ((time_str[14] - '0') * 10 + (time_str[15] - '0'));

	curData.second = (unsigned char) ((time_str[17] - '0') * 10 + (time_str[18] - '0'));

#else

	//18-09-05,09:19:13
	//01234567890123456
	curData.century = 20;
	curData.year = (unsigned char) ((time_str[0] - '0') * 10 + (time_str[1] - '0'));
	curData.month = (unsigned char) ((time_str[3] - '0') * 10 + (time_str[4] - '0'));
	curData.day = (unsigned char) ((time_str[6] - '0') * 10 + (time_str[7] - '0'));

	curData.hour = (unsigned char) ((time_str[9] - '0') * 10 + (time_str[10] - '0'));

	curData.minute = (unsigned char) ((time_str[12] - '0') * 10 + (time_str[13] - '0'));

	curData.second = (unsigned char) ((time_str[15] - '0') * 10 + (time_str[16] - '0'));
#endif
	//2018-09-05,09:19:13
	sprintf(test_time, "%02d-%02d-%02d,%02d:%02d:%02d", curData.year, curData.month, curData.day, curData.hour,
			curData.minute, curData.second);
//	Debug("TimerS: %s\n\r", test_time);

	//当前年份
	curyear = curData.century * 100 + curData.year; //century*100为年的百位和千位

	//无效年份判断
	if (curyear < 1970 || curyear > 9000)
		return 0;

	//计算差值并计算出当前年份差值对应的天数
	cyear = curyear - 1970;
	cday = (cyear / 4) * (365 * 3 + 366); //先计算4年周期的个数，然后计算所有天数

	//计算平年和闰年 对应的相应差值年的天数 1970-平 1971-平 1972-闰 1973-平
	if (cyear % 4 >= 3)
		cday += (cyear % 4 - 1) * 365 + 366;
	else
		cday += (cyear % 4) * 365;

	//当前月份对应的天数  当月的天数不算
#if 1
	switch (curData.month)
	{
	case 2:
		curmonthday = 31;
		break;
	case 3:
		curmonthday = 59;
		break;
	case 4:
		curmonthday = 90;
		break;
	case 5:
		curmonthday = 120;
		break;
	case 6:
		curmonthday = 151;
		break;
	case 7:
		curmonthday = 181;
		break;
	case 8:
		curmonthday = 212;
		break;
	case 9:
		curmonthday = 243;
		break;
	case 10:
		curmonthday = 273;
		break;
	case 11:
		curmonthday = 304;
		break;
	case 12:
		curmonthday = 334;
		break;
	default:
		curmonthday = 0;
		break;
	}
#else

	curmonthday = Monthday[ curData.month ];

#endif

	//平年和闰年月份对应天数 闰年天数+1
	if ((curyear % 4 == 0) && (curData.month >= 3))
	{
		curmonthday += 1;
	}

	//总天数加上月份对应的天数 加上当前天数-1 当天的时间不算
	cday += curmonthday;
	cday += (curData.day - 1);

	*weekday = DayWeek[cday % 7];

	//返回格林威治时间秒数
	return (unsigned long) (((cday * 24 + curData.hour) * 60 + curData.minute) * 60 + curData.second);
}

#if 0	//网上年月日转GMT秒数程序
unsigned long GetSecondTime(DATE_TIME *date_time)
{	uint16_t iYear,
	iMon,
	iDay,
	iHour,
	iMin,
	iSec;
	uint16_t i, Cyear=0;

	iYear = date_time->iYear;
	iMon = date_time->iMon;
	iDay = date_time->iDay;
	iHour = date_time->iHour;
	iMin = date_time->iMin;
	iSec = date_time->iSec;

	unsigned long CountDay=0;
	for(i=1970; i<iYear; i++)
	{
		if(((i%4==0) && (i%100!=0)) || (i%400==0))
		{
			Cyear++;
		}
	}
	CountDay = Cyear * 366 + (iYear-1970-Cyear) * 365;
	for(i=1; i<iMon; i++)
	{
		if((i==2) && (((iYear%4==0)&&(iYear%100!=0)) || (iYear%400==0)))
		CountDay += 29;
		else
		CountDay += DayOfMon[i-1];
	}
	CountDay += (iDay-1);
	CountDay = CountDay*SECOND_OF_DAY + (unsigned long)iHour*3600 + (unsigned long)iMin*60 + iSec;
	return CountDay;
}
#endif

void GetDateTimeFromUTC(uint8_t * time_UTC, int8_t time_zone)
{
	DATE_TIME tTime;
	unsigned long GMT_Sec = 0;
	unsigned long time_zone_l = 0;
	uint16_t i, j, iDay;
	unsigned long lDay;
	uint16_t weekday;

	GMT_Sec = TimerSwitch(time_UTC, &weekday);
	//Debug("\n\rzone_t : %d ", time_zone);
	if (0 > time_zone)
	{
		time_zone_l = (unsigned long) (0 - time_zone);
		GMT_Sec = GMT_Sec - time_zone_l * 3600;
	}

	else
	{
		time_zone_l = (unsigned long) (time_zone);
		GMT_Sec = GMT_Sec + time_zone_l * 3600;
	}

	lDay = GMT_Sec / SECOND_OF_DAY; /* 计算总的天数 */
	GMT_Sec = GMT_Sec % SECOND_OF_DAY;

	i = 1970;
	while (lDay > 365)
	{
		if (((i % 4 == 0) && (i % 100 != 0)) || (i % 400 == 0)) /* ???? */
			lDay -= 366;
		else
			lDay -= 365;
		i++;
	}

	if ((lDay == 365) && !(((i % 4 == 0) && (i % 100 != 0)) || (i % 400 == 0))) /* ??? */
	{
		lDay -= 365;
		i++;
	}

	tTime.iYear = i;

	for (j = 0; j < 12; j++)
	{
		if ((j == 1) && (((i % 4 == 0) && (i % 100 != 0)) || (i % 400 == 0)))
			iDay = 29;
		else
			iDay = DayOfMon[j];
		if (lDay >= iDay)
			lDay -= iDay;
		else
			break;
	}

	tTime.iMon = j + 1;
	tTime.iDay = lDay + 1;
	tTime.iHour = ((GMT_Sec / 3600)) % 24; //这里注意，世界时间已经加上北京时间差8，
	tTime.iMin = (GMT_Sec % 3600) / 60;
	tTime.iSec = (GMT_Sec % 3600) % 60;

	//2018-09-05,09:19:13
	sprintf(time_UTC, "%02d-%02d-%02d,%02d:%02d:%02d:%02d", tTime.iYear, tTime.iMon, tTime.iDay, tTime.iHour,
			tTime.iMin, tTime.iSec, weekday);

//	Debug("Local time : %s\n", time_UTC);

}

