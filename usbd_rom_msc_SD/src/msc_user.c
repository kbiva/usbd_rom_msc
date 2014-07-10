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
#include "sd_spi.h"

// read buffer
uint8_t bufr[SD_BLOCKSIZE];
// write buffer
uint8_t bufw[SD_BLOCKSIZE];
// verify buffer
uint8_t bufv[SD_BLOCKSIZE];

// Zero-Copy Data Transfer model
void MSC_Read(uint32_t offset, uint8_t** buff_adr, uint32_t length, uint32_t high_offset) {
  
  uint32_t j = offset%SD_BLOCKSIZE;
  
  // Host requests data in chunks of 512 bytes, USB bulk endpoint size is 64 bytes.
  // For each sector of 512 bytes, this function gets called 8 times with length=64 bytes
  // First time read whole 512 bytes sector, and then just change pointer in the buffer
  if(j==0) {
    sd_read_block(offset/SD_BLOCKSIZE,bufr);
  }  
  
  *buff_adr = &bufr[j];
}

void MSC_Write(uint32_t offset, uint8_t** buff_adr, uint32_t length, uint32_t high_offset) {

  uint32_t j = offset%SD_BLOCKSIZE;
  
  // Host requests data in chunks of 512 bytes, USB bulk endpoint size is 64 bytes.
  // For each sector of 512 bytes, this function gets called 8 times with length=64 bytes
  // Accumulate all requests in the buffer, and on the last request write 512 bytes to the card.
  memcpy(&bufw[j],*buff_adr,length);
  
  if((offset+USB_FS_MAX_BULK_PACKET)%SD_BLOCKSIZE==0) {
    sd_write_block(offset/SD_BLOCKSIZE,bufw);    
  }
}

// TODO: untested
ErrorCode_t MSC_Verify(uint32_t offset, uint8_t* src, uint32_t length, uint32_t high_offset) {
  
  uint32_t j = offset%SD_BLOCKSIZE;
  
  if(j==0) {
    sd_read_block(offset/SD_BLOCKSIZE,bufv);
  }  
  
  if(!memcmp(src,&bufv[j],length)) {
    return ERR_FAILED;
  }
  else {
    return LPC_OK;
  }
}
