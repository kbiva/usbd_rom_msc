/*
 * uart.h
 *
 *  Created on: 2014.07.04
 *      Author: Kestutis Bivainis
 */

#ifndef _UART_H_
#define _UART_H_

#include <stdint.h>

uint8_t init_uart(uint32_t baud);
void putLineUART(const char *send_data);

#endif
