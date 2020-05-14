/*
 * sys_util.c
 *
 *  Created on: May 13, 2020
 *      Author: Jason C
 *     version: V1.0.1
 ************************************************************************
 *	 attention: GPLv3, free software
 */


#include "sys_util.h"





bool startWith(const char *pre, const char *str)
{
	uint32_t lenpre = strlen(pre);
	uint32_t lenstr = strlen(str);

	return lenstr > lenpre ? false : strncmp(pre, str, lenstr) == 0;
}
