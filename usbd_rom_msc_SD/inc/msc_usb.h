/*
 * msc_usb.h
 *
 *  Created on: 2014.07.04
 *      Author: Kestutis Bivainis
 */

#ifndef _MSC_USB_H_
#define _MSC_USB_H_

#include <stdint.h>
#include "error.h"

void MSC_Read(uint32_t offset, uint8_t** buff_adr, uint32_t length, uint32_t high_offset);
void MSC_Write(uint32_t offset, uint8_t** buff_adr, uint32_t length, uint32_t high_offset);
ErrorCode_t MSC_Verify(uint32_t offset, uint8_t* src, uint32_t length, uint32_t high_offset);

#endif
