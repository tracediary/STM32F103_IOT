/*
 * hardware.h
 *
 *  Created on: 2020年5月4日
 *      Author: Jason C
 *     version: V1.0.1
 ************************************************************************
 *	 attention: GPLv2, free software
 */

#ifndef USERINC_HARDWARE_H_
#define USERINC_HARDWARE_H_



#include "stm32f1xx_hal.h"

/**
 * @brief general define, this file base on the EmbedFile Badao v2 board
 */

/* LED pin define******************************************************************/
#define LED_R_CLK_EN						__HAL_RCC_GPIOB_CLK_ENABLE()
#define LED_R_PIN							GPIO_PIN_5
#define LED_R_GPIO_PORT						GPIOB
#define LED_G_CLK_EN						__HAL_RCC_GPIOB_CLK_ENABLE()
#define LED_G_PIN							GPIO_PIN_0
#define LED_G_GPIO_PORT						GPIOB
#define LED_B_CLK_EN						__HAL_RCC_GPIOB_CLK_ENABLE()
#define LED_B_PIN							GPIO_PIN_1
#define LED_B_GPIO_PORT						GPIOB

/* debug USART define**************************************************************/
#define USART2_TX_CLK_EN					__HAL_RCC_GPIOA_CLK_ENABLE()
#define USART2_TX_PIN						GPIO_PIN_2
#define USART2_TX_GPIO_PORT					GPIOA
#define USART2_RX_CLK_EN					__HAL_RCC_GPIOA_CLK_ENABLE()
#define USART2_RX_PIN						GPIO_PIN_3
#define USART2_RX_GPIO_PORT					GPIOA

/* wifi define*********************************************************************************/
#define WIFI_USART3_TX_CLK_EN				__HAL_RCC_GPIOB_CLK_ENABLE();
#define WIFI_USART3_TX_PIN					GPIO_PIN_10
#define WIFI_USART3_TX_GPIO_PORT			GPIOB

#define WIFI_USART3_RX_CLK_EN				__HAL_RCC_GPIOB_CLK_ENABLE();
#define WIFI_USART3_RX_PIN					GPIO_PIN_11
#define WIFI_USART3_RX_GPIO_PORT			GPIOB

#define WIFI_EN_CLK_EN						__HAL_RCC_GPIOG_CLK_ENABLE()
#define WIFI_EN_PIN							GPIO_PIN_13
#define WIFI_EN_GPIO_PORT					GPIOG

#define WIFI_RST_CLK_EN						__HAL_RCC_GPIOG_CLK_ENABLE()
#define WIFI_RST_PIN						GPIO_PIN_14
#define WIFI_RST_GPIO_PORT					GPIOG



#endif /* USERINC_HARDWARE_H_ */
