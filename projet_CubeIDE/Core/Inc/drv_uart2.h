/*
 * drv_uart1.h
 *
 *  Created on: Nov 13, 2025
 *      Author: Antle
 */

#ifndef DRV_UART2_H_
#define DRV_UART2_H_

#include <stdint.h>

uint8_t drv_uart2_receive(char * pData, uint16_t size);
uint8_t drv_uart2_transmit(const char * pData, uint16_t size);


#endif /* DRV_UART1_H_ */