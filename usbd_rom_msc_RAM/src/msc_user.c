/*
 * msc_user.c
 *
 *  Created on: 2014.07.04
 *      Author: Kestutis Bivainis
 */

#include "msc_usb.h"
#include "error.h"
#include <string.h>
#include <stdio.h>

uint32_t USBD_MSC_MemorySize;
uint32_t USBD_MSC_BlockSize;

// 28Kb total, 26.5Kb free 
static uint8_t image[TOTAL_BLOCKS*BLOCK_SIZE];

//3 sectors
const uint8_t sector_image_0[] = {
  //sector 0: boot sector
  0xEB,0x3C,0x90,
  'M','S','D','O','S','5','.','0',                  // MSDOS5.0
  (uint8_t)BLOCK_SIZE,(uint8_t)(BLOCK_SIZE>>8),     // Bytes per sector: 512
  0x01,                                             // Sectors per cluster: 1
  0x01,0x00,                                        // Number of reserved sectors: 1
  0x01,                                             // Number of FATs: 1 
  0x10,0x00,                                        // Maximum number of root directory entries: 16 
  (uint8_t)TOTAL_BLOCKS,(uint8_t)(TOTAL_BLOCKS>>8), // Total small sector count: 56 
  0xF8,                                             // Media descriptor
  0x01,0x00,                                        // Sectors per FAT: 1
  0x01,0x00,                                        // Sectors per track: 1
  0x01,0x00,                                        // Number of heads: 1
};

const uint8_t sector_image_1[] = {
  //sector 1: FAT 1
  0xF8,0xFF,0xFF,
};

const uint8_t sector_image_2[] = {
  //sector 2: root directory
  //entry 1 - Volume label
  'L','P','C','1','5','4','9',' ','U','S','B',0x28,
};

void usbd_msc_init(void) {
  
  // Prepare Boot sector
  memcpy(&image[0], sector_image_0, sizeof(sector_image_0));
  image[510]=0x55;
  image[511]=0xAA;
  
  // Prepare FAT sector
  memcpy(&image[512], sector_image_1, sizeof(sector_image_1));
  
  // Prepare Root directory sector
  memcpy(&image[1024], sector_image_2, sizeof(sector_image_2));
  
  // 28Kb RAM, 26.5Kb free
  USBD_MSC_MemorySize = TOTAL_BLOCKS*BLOCK_SIZE;
  USBD_MSC_BlockSize  = BLOCK_SIZE;
}

// Zero-Copy Data Transfer model
void MSC_Read(uint32_t offset, uint8_t** buff_adr, uint32_t length, uint32_t high_offset) {
  
  *buff_adr=&image[offset];
}

void MSC_Write(uint32_t offset, uint8_t** buff_adr, uint32_t length, uint32_t high_offset) {
  
  memcpy(&image[offset],*buff_adr,length);  
}

ErrorCode_t MSC_Verify(uint32_t offset, uint8_t* src, uint32_t length, uint32_t high_offset) {
  return LPC_OK;
}
