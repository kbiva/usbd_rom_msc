/*
 * msc_user.c
 *
 *  Created on: 2014.07.04
 *      Author: Kestutis Bivainis
 */

#include "board.h"
#include "msc_usb.h"
#include "app_usbd_cfg.h"
#include "error.h"
#include <string.h>
#include <stdio.h>

uint32_t USBD_MSC_MemorySize;
uint32_t USBD_MSC_BlockSize;

static uint8_t image[SYSTEM_BLOCKS*BLOCK_SIZE];

//3 sectors
static const uint8_t sector_image_0[] = {
  //sector 0: boot sector
  0xEB,0x3C,0x90,
  'M','S','D','O','S','5','.','0',                  // MSDOS5.0
  (uint8_t)BLOCK_SIZE,(uint8_t)(BLOCK_SIZE>>8),     // Bytes per sector: 512
  0x01,                                             // Sectors per cluster: 1
  0x01,0x00,                                        // Number of reserved sectors: 1
  0x01,                                             // Number of FATs: 1 
  0x10,0x00,                                        // Maximum number of root directory entries: 16 
  (uint8_t)TOTAL_BLOCKS,(uint8_t)(TOTAL_BLOCKS>>8), // Total small sector count: 11  
  0xF8,                                             // Media descriptor
  0x01,0x00,                                        // Sectors per FAT: 1
  0x01,0x00,                                        // Sectors per track: 1
  0x01,0x00,                                        // Number of heads: 1
};

static const uint8_t sector_image_1[] = {
  //sector 1: FAT 1
  0xF8,0xFF,0xFF,0x03,0x40,0x00,0x05,0x60,0x00,0x07,0x80,0x00,0x09,0xF0,0xFF,
};

static const uint8_t sector_image_2[] = {
  //sector 2: root directory
  //entry 1 - Volume label
  'L','P','C','1','5','4','9',' ','U','S','B',0x28,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  //entry 2 - EEPROM contents
  'E','E','P','R','O','M',' ',' ','B','I','N',0x00,0x00,0x65,0xCD,0xA2,
  0xE2,0x44,0xE2,0x44,0x00,0x00,0xC7,0xA2,0xE2,0x44,0x02,0x00,0xC0,0x0F,0x00,0x00,
};

uint8_t buf[USB_FS_MAX_BULK_PACKET];

static void errorEEPROM(void) {
	
  Board_LED_Set(0, true);
  while (1){};
}

void usbd_msc_init(void) {
  
  // Prepare Boot sector
  memcpy(&image[0], sector_image_0, sizeof(sector_image_0));
  image[510]=0x55;
  image[511]=0xAA;

  // Prepare FAT sector
  memcpy(&image[512], sector_image_1, sizeof(sector_image_1));

  // Prepare Root directory sector
  memcpy(&image[1024], sector_image_2, sizeof(sector_image_2));
  
  // 4Kb EEPROM+1.5Kb
  USBD_MSC_MemorySize = TOTAL_BLOCKS*BLOCK_SIZE;
  USBD_MSC_BlockSize  = BLOCK_SIZE;
}

// Zero-Copy Data Transfer model
void MSC_Read(uint32_t offset, uint8_t** buff_adr, uint32_t length, uint32_t high_offset) {
  
  uint8_t ret_code;
  
  if(offset<SYSTEM_BLOCKS*BLOCK_SIZE) {
    // Boot,FAT or Root directory
    *buff_adr=&image[offset];
  }
  else if(offset<TOTAL_BLOCKS*BLOCK_SIZE-EEPROM_RESERVED_BYTES) {
    // EEPROM without last 64 bytes
    ret_code = Chip_EEPROM_Read(offset-SYSTEM_BLOCKS*BLOCK_SIZE, buf, length);
    if (ret_code == IAP_CMD_SUCCESS) {
      *buff_adr=buf;
    }
    else {
      errorEEPROM();
    }
  }
  else if(offset<TOTAL_BLOCKS*BLOCK_SIZE) {
    // Last 64 bytes of EEPROM, return zeros
    memset(buf,0,sizeof(buf));
    *buff_adr=buf;
  }
}

void MSC_Write(uint32_t offset, uint8_t** buff_adr, uint32_t length, uint32_t high_offset) {
  
  uint8_t ret_code;
  
  if(offset<SYSTEM_BLOCKS*BLOCK_SIZE) {
    // Boot,FAT or Root directory
    memcpy(&image[offset],*buff_adr,length);
  }
  else if(offset<TOTAL_BLOCKS*BLOCK_SIZE-EEPROM_RESERVED_BYTES) {
    // EEPROM without last 64 bytes
    ret_code = Chip_EEPROM_Write(offset-SYSTEM_BLOCKS*BLOCK_SIZE, *buff_adr, length);
    if (ret_code != IAP_CMD_SUCCESS) {
      errorEEPROM();
    }
  }
}

ErrorCode_t MSC_Verify(uint32_t offset, uint8_t* src, uint32_t length, uint32_t high_offset) {
  return LPC_OK;
}
