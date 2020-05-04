/*
 * wifi_bsp.c
 *
 *  Created on: 2020年5月4日
 *      Author: Jason C
 *     version: V1.0.1
 ************************************************************************
 *	 attention: GPLv2, free software
 */



#include "usart.h"
#include "debug.h"
#include "constant.h"
#include "hardware.h"

/**FreeRTOS head file**********************************************/
#include "FreeRTOS.h"
#include "task.h"
#include "event_groups.h"
#include "semphr.h"

void wifi_gpio_config();
void wifi_usart_config();

void wifi_bsp_config()
{
	wifi_gpio_config();
	wifi_usart_config();
}

void wifi_bsp_de_config()
{
	wifi_gpio_config();
	WIFI_UART_Deinit();
}

void wifi_gpio_config()
{
	GPIO_InitTypeDef GPIO_InitStruct = { 0 };

	/* GPIO Ports Clock Enable */
	__HAL_RCC_GPIOB_CLK_ENABLE()
	;
	__HAL_RCC_GPIOG_CLK_ENABLE()
	;

	HAL_GPIO_WritePin(WIFI_EN_GPIO_Port, WIFI_EN_Pin, GPIO_PIN_RESET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(WIFI_RST_GPIO_Port, WIFI_RST_Pin, GPIO_PIN_SET);

	/*Configure GPIO pins : PGPin PGPin */
	GPIO_InitStruct.Pin = WIFI_EN_Pin | WIFI_RST_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);
}

void wifi_usart_config()
{
	MX_USART3_UART_Init();
}
