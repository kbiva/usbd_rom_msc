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

#define BLOCK_SIZE 512
#define TOTAL_BLOCKS 11
#define SYSTEM_BLOCKS 3

#define EEPROM_RESERVED_BYTES 64

void usbd_init(void);
void usbd_msc_init(void);

void MSC_Read(uint32_t offset, uint8_t** buff_adr, uint32_t length, uint32_t high_offset);
void MSC_Write(uint32_t offset, uint8_t** buff_adr, uint32_t length, uint32_t high_offset);
ErrorCode_t MSC_Verify(uint32_t offset, uint8_t* src, uint32_t length, uint32_t high_offset);
    
#endif
