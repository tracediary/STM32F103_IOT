/*
 * debug.h
 *
 *  Created on: 2020年5月4日
 *      Author: Jason C
 *     version: V1.0.1
 ************************************************************************
 *	 attention: GPLv3, free software
 */

#ifndef USERINC_DEBUG_H_
#define USERINC_DEBUG_H_


#include <stdio.h>
#include "stm32f1xx_hal.h"
/*************/

#define DEBUG
#define SYS_ERROR
//void funNull(const uint8_t *pData);

#ifdef DEBUG
#define Debug(...) 	printf(__VA_ARGS__)
#else
#define Debug(...)
#endif

#ifdef SYS_ERROR
#define Error(...) 	printf(__VA_ARGS__)
#else
#define Error(...)
#endif


#endif /* USERINC_DEBUG_H_ */
