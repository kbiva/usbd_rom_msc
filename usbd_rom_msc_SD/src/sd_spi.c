/*
 * sd_spi.c
 *
 *  Created on: 2014.07.04
 *      Author: Kestutis Bivainis
 */

#include "board.h"
#include "sd_spi.h"
#include "delay.h"

// Table for CRC-7 (polynomial x^7 + x^3 + 1)
static uint8_t CRCTable[256];

extern SD_CardInfo cardinfo;

uint8_t response[5];

static void Init_SPI_PinMux(void) {
  
  Chip_Clock_EnablePeriphClock(SYSCTL_CLOCK_SWM);

  Chip_IOCON_PinMuxSet(LPC_IOCON, 0, PIN_CLK, (IOCON_MODE_INACT | IOCON_DIGMODE_EN));
  Chip_IOCON_PinMuxSet(LPC_IOCON, 0, PIN_MOSI, (IOCON_MODE_INACT | IOCON_DIGMODE_EN));
  Chip_IOCON_PinMuxSet(LPC_IOCON, 0, PIN_MISO, (IOCON_MODE_INACT | IOCON_DIGMODE_EN));
  Chip_IOCON_PinMuxSet(LPC_IOCON, 0, PIN_CS, (IOCON_MODE_INACT | IOCON_DIGMODE_EN));

  Chip_SWM_MovablePinAssign(SWM_SPI0_SCK_IO, PIN_CLK);
  Chip_SWM_MovablePinAssign(SWM_SPI0_MOSI_IO, PIN_MOSI);
  Chip_SWM_MovablePinAssign(SWM_SPI0_MISO_IO, PIN_MISO);
  Chip_SWM_MovablePinAssign(SWM_SPI0_SSELSN_0_IO, PIN_CS);

  Chip_Clock_DisablePeriphClock(SYSCTL_CLOCK_SWM);
}

static void setupSpiMaster(uint8_t clkdiv) {

  SPI_CFG_T spiCfg;
  SPI_DELAY_CONFIG_T spiDelayCfg;
  
  Chip_SPI_Init(LPC_SPI0);
  
  spiCfg.ClkDiv = clkdiv;
  spiCfg.Mode = SPI_MODE_MASTER;
  spiCfg.ClockMode = SPI_CLOCK_MODE0;
  spiCfg.DataOrder = SPI_DATA_MSB_FIRST;
  spiCfg.SSELPol = SPI_CFG_SPOL0_LO;
  Chip_SPI_SetConfig(LPC_SPI0, &spiCfg);

  spiDelayCfg.PreDelay = 0;
  spiDelayCfg.PostDelay = 0;
  spiDelayCfg.FrameDelay = 0;
  spiDelayCfg.TransferDelay = 0;
  Chip_SPI_DelayConfig(LPC_SPI0, &spiDelayCfg);

  Chip_SPI_Enable(LPC_SPI0);
}

static void GenerateCRCTable(void) {
  
  int i, j;
  uint8_t CRCPoly = 0x89;  // the value of our CRC-7 polynomial
 
  // generate a table value for all 256 possible byte values
  for (i = 0; i < 256; ++i) {
    CRCTable[i] = (i & 0x80) ? i ^ CRCPoly : i;
    for (j = 1; j < 8; ++j) {
        CRCTable[i] <<= 1;
        if (CRCTable[i] & 0x80)
            CRCTable[i] ^= CRCPoly;
    }
  }
}

// adds a message byte to the current CRC-7 to get a the new CRC-7
static uint8_t CRCAdd(uint8_t crc, uint8_t message_byte) {
  
  return CRCTable[(crc << 1) ^ message_byte];
}

static uint8_t getCRC(uint8_t* message) {
  
  uint32_t i;
  uint8_t crc=0;

  for (i=0; i<5; i++)
    crc = CRCAdd(crc, message[i]);

  return crc;
}

static void SPI_WriteDummyByteCSHigh(void) {
  
  while(~LPC_SPI0->STAT & SPI_STAT_TXRDY){};
  LPC_SPI0->TXDATCTL = SPI_TXDATCTL_LEN(8-1) | SPI_TXDATCTL_EOT | SPI_TXCTL_DEASSERT_SSEL0 | SPI_TXCTL_RXIGNORE | 0xFF;
}

static void SPI_WriteDummyByte(void) {
  
  while(~LPC_SPI0->STAT & SPI_STAT_TXRDY){};
  LPC_SPI0->TXDATCTL = SPI_TXDATCTL_LEN(8-1) | SPI_TXDATCTL_EOT | SPI_TXCTL_ASSERT_SSEL0 | SPI_TXCTL_RXIGNORE | 0xFF;
}

static void SPI_WriteByte(uint8_t data) {
  
  while(~LPC_SPI0->STAT & SPI_STAT_TXRDY){};
  LPC_SPI0->TXDATCTL = SPI_TXDATCTL_LEN(8-1) | SPI_TXDATCTL_EOT | SPI_TXCTL_ASSERT_SSEL0 | SPI_TXCTL_RXIGNORE | data;
}

static uint8_t SPI_ReadByte(void) {
  
  while(~LPC_SPI0->STAT & SPI_STAT_TXRDY){};
  LPC_SPI0->TXDATCTL = SPI_TXDATCTL_LEN(8-1) | SPI_TXDATCTL_EOT | SPI_TXCTL_ASSERT_SSEL0 | 0xFF;
  while(~LPC_SPI0->STAT & SPI_STAT_RXRDY){};  
  return LPC_SPI0->RXDAT;
}

static SD_ERROR sd_send_command(uint16_t cmd,uint32_t data) {

  uint8_t send[6];
  uint32_t i;
  uint8_t tmp;
  uint32_t time1,time2;

  send[0] = (cmd&0x3F)|0x40; 
  send[1] = data>>24; 
  send[2] = data>>16; 
  send[3] = data>>8; 
  send[4] = data; 
  send[5] = (getCRC(send)<<1)|0x01; 
  
  for(i=0;i<6;i++) {
    SPI_WriteByte(send[i]);
  }
  
  time1=DWT_Get();  
  do {
    tmp=SPI_ReadByte();
    time2=DWT_Get();
  }
  while((tmp & 0x80)&&(time2-time1 < SD_CMD_TIMEOUT));
  
  if(time2-time1 >= SD_CMD_TIMEOUT) {
    return SD_TIMEOUT;
  }
  
  for (i=0;i<((cmd&0x0F00)>>8);i++) {
    response[i] = tmp;
    // This handles the trailing-byte requirement.
    tmp = SPI_ReadByte();
  }

  // If the response is a "busy" type (R1b), then there’s some
  // special handling that needs to be done. The card will
  // output a continuous stream of zeros, so the end of the BUSY
  // state is signaled by any nonzero response. The bus idles
  // high.
  
  if ((cmd&0xFF00) == R1b) {
    
    // This should never time out, unless SDI is grounded.
    // Don’t bother forcing a timeout condition here.
    do {
      tmp = SPI_ReadByte();
    } while (tmp != 0xFF);    
    
    SPI_WriteDummyByte();
  }
  
  return SD_OK;
}

SD_ERROR init_sd_spi(SD_CardInfo *cardinfo) {

  uint32_t i,time1,time2;
  SD_ERROR tmp;
  
  GenerateCRCTable();
  Init_SPI_PinMux();
  
  // Initialization at slow speed
  setupSpiMaster(SystemCoreClock/400000-1);// 400Khz

  for(i=0;i<10;i++) {
    SPI_WriteDummyByteCSHigh();
  }
  
  // Go idle state
  if(sd_send_command(CMD0,0)!=SD_OK) {
    return ERROR_GO_IDLE_STATE_TIMEOUT;
  }
    
  if(response[0]!=R1_IN_IDLE_STATE)  {    
    return ERROR_GO_IDLE_STATE_RESPONSE;
  }
  
  // SEND_IF_COND
  if(sd_send_command(CMD8, 0x000001AA)!=SD_OK) {
    return ERROR_SEND_IF_COND_TIMEOUT;
  }
  
  if((response[0]&R1_ILLEGAL_COMMAND)==R1_ILLEGAL_COMMAND) {
    // SD 1.x or MMC
    if(sd_send_command(CMD55,0)==SD_OK) {
      
      if(sd_send_command(ACMD41,0)==SD_OK) {
        
        if((response[0]&R1_ILLEGAL_COMMAND)==R1_ILLEGAL_COMMAND) {
          // MMC
          cardinfo->CardType = MULTIMEDIA_CARD;
        
          time1=DWT_Get();
          
          // Loop till card goes out of idle
          do {  
            if(sd_send_command(CMD1,0)!=SD_OK) {
              return ERROR_SEND_OP_COND_TIMEOUT;
            }
            time2=DWT_Get();
          } while((response[0]==R1_IN_IDLE_STATE) && (time2-time1 < SD_CMD_TIMEOUT));
      
          if (time2-time1 >= SD_CMD_TIMEOUT) {    
            return ERROR_INIT_TIMEOUT;
          }
        }
        else {
          // SD 1.x          
          cardinfo->CardType = SD_CARD_STD_CAPACITY_V1_1;
          // TODO: Didn't have SD 1.x cards to test the loop below
          // Loop till card goes out of idle
          do
          {
            if(sd_send_command(CMD55,0)==SD_OK) {
              if(sd_send_command(ACMD41,0)!=SD_OK) {
                return ERROR_SD_SEND_OP_COND_TIMEOUT;                
              }
            }
            else
            {
              return ERROR_APP_CMD_TIMEOUT;
            }
            time2=DWT_Get();
          }
          while (((response[0]&R1_IN_IDLE_STATE)==R1_IN_IDLE_STATE) && (time2-time1 < SD_CMD_TIMEOUT));
          
          // As long as we didn’t hit the timeout, assume we’re OK.
          if (time2-time1 >= SD_CMD_TIMEOUT) {    
            return ERROR_INIT_TIMEOUT;
          }
        }        
      }
      else {
        return ERROR_SD_SEND_OP_COND_TIMEOUT;        
      }      
    }
    else {
      return ERROR_APP_CMD_TIMEOUT;
    }
  }
  else if((response[0]==R1_IN_IDLE_STATE) && (response[3]==0x01) && (response[4]==0xAA)) {
    // SD 2.0 or SDHC
    cardinfo->CardType = SD_CARD_STD_CAPACITY_V2_0;
    
    time1=DWT_Get();
  
    // Loop till card goes out of idle
    do
    {
      if(sd_send_command(CMD55,0)==SD_OK) {
        if(sd_send_command(ACMD41,0x40000000)!=SD_OK) {
          return ERROR_SD_SEND_OP_COND_TIMEOUT;        
        }        
      }
      else
      {
        return ERROR_APP_CMD_TIMEOUT;
      }
      time2=DWT_Get();
    }
    while (((response[0]&R1_IN_IDLE_STATE)==R1_IN_IDLE_STATE) && (time2-time1 < SD_CMD_TIMEOUT));
    
    // As long as we didn’t hit the timeout, assume we’re OK.
    if (time2-time1 >= SD_CMD_TIMEOUT) {    
      return ERROR_INIT_TIMEOUT;
    }
  }
  else {
    return ERROR_SEND_IF_COND_RESPONSE;
  }
  
  // Read OCR register for supported voltages and SDHC bit
  if(sd_send_command(CMD58,0)!=SD_OK) {
    return ERROR_READ_OCR_TIMEOUT;
  }
  
  // At a very minimum, we must allow 3.3V.
  if ((response[2] & MSK_OCR_33) != MSK_OCR_33) {
    return ERROR_READ_OCR_RESPONSE;
  }
  
  // Test for SDHC  
  if(cardinfo->CardType == SD_CARD_STD_CAPACITY_V2_0) {    
    if((response[1]&0x40)==0x40){
      // SDHC
      cardinfo->CardType = SD_CARD_HIGH_CAPACITY;
    }    
  }  
  
  // After initialization go full speed  
  setupSpiMaster(SystemCoreClock/24000000-1);//24Mhz
  
  // Read and decode CID register
  tmp=sd_read_cid(&cardinfo->SD_cid,cardinfo->CardType);
  if(tmp!=SD_OK) {
    return tmp;
  }
  
  // Read and decode CSD register
  tmp=sd_read_csd(&cardinfo->SD_csd,cardinfo->CardType);
  if(tmp!=SD_OK) {
    return tmp;
  }
  
  // Calculate card capacity
  if ((cardinfo->CardType == SD_CARD_STD_CAPACITY_V1_1) || 
      (cardinfo->CardType == SD_CARD_STD_CAPACITY_V2_0) ||
      (cardinfo->CardType == MULTIMEDIA_CARD)) {
    
    cardinfo->CardCapacity=cardinfo->SD_csd.DeviceSize+1;
    cardinfo->CardCapacity*=(1<<(cardinfo->SD_csd.DeviceSizeMul+2));
    cardinfo->CardBlockSize=1<<cardinfo->SD_csd.RdBlockLen;
    cardinfo->CardCapacity*=cardinfo->CardBlockSize;
  }
  else if (cardinfo->CardType == SD_CARD_HIGH_CAPACITY) {
    cardinfo->CardCapacity=(uint64_t)(cardinfo->SD_csd.DeviceSize+1)*512*1024;
    cardinfo->CardBlockSize=512;
  }

  return SD_OK;
}

uint8_t sd_read_block (uint32_t blockaddr,uint8_t *data) { 
  
  uint32_t i;
  uint8_t tmp;
  uint32_t time1,time2;

  // convert to block address
  if(cardinfo.CardType!=SD_CARD_HIGH_CAPACITY) {
    blockaddr<<=SD_BLOCKSIZE_NBITS;
  }

  if(sd_send_command(CMD17, blockaddr)!=SD_OK) {
    return 1;
  }

  // Check for an error, like a misaligned read
  if(response[0]) {
    return 1;
  }

  // Wait for the token
  time1=DWT_Get();
  do
  {
    tmp = SPI_ReadByte();
    time2=DWT_Get();
  }
  while ((tmp == 0xFF) && (time2-time1 < SD_CMD_TIMEOUT));
  
  if (time2-time1 >= SD_CMD_TIMEOUT) {
    return 1;
  }
  
  if (tmp != SD_TOK_READ_STARTBLOCK)
  {
    // Clock out a byte before returning
    SPI_WriteDummyByte();
    // The card returned an error response. Bail and return 0
    return 1;
  }
  
  for(i=0; i<SD_BLOCKSIZE; i++) {
    *data++ = SPI_ReadByte();
  }
 
  //crc
  SPI_WriteDummyByte(); 
  SPI_WriteDummyByte(); 
  
  SPI_WriteDummyByte();
 
  return 0;
}

uint8_t sd_write_block (uint32_t blockaddr,uint8_t *data) {
  
  uint32_t i;
  uint8_t tmp;

  // convert to block address
  if(cardinfo.CardType!=SD_CARD_HIGH_CAPACITY) {
    blockaddr<<=SD_BLOCKSIZE_NBITS;
  }
  
  if(sd_send_command(CMD24,blockaddr)!=SD_OK) {
    return 1;
  }    

  // Check for an error, like a misaligned write
  if(response[0]) {
    return 1;
  } 
  
  // indicate start of block
  SPI_WriteByte(SD_TOK_WRITE_STARTBLOCK);
  
  for(i=0; i<SD_BLOCKSIZE; i++) {
    SPI_WriteByte(data[i]);
  }
  
  //crc
  SPI_WriteDummyByte(); 
  SPI_WriteDummyByte();

  // check the response token
  tmp=SPI_ReadByte();
  if((tmp & 0x1F) != DATA_RESPONSE_TOKEN_DATA_ACCEPTED) {
    SPI_WriteDummyByte();
    return 1;
  }

  // wait for write finish
  while(SPI_ReadByte()==0){};
  
  SPI_WriteDummyByte();
  
  return 0;
  
}

SD_ERROR sd_read_cid(SD_CID *sd_cid,CARD_TYPE ct) {
  
  uint32_t i;
  uint8_t tmp;
  uint32_t time1,time2;
  uint8_t buf[16];

  if(sd_send_command(CMD10,0)!=SD_OK) {
    return ERROR_SEND_CID_TIMEOUT;
  }
  
  // Wait for the token
  time1=DWT_Get();
  do
  {
    tmp = SPI_ReadByte();
    time2=DWT_Get();
  }
  while ((tmp == 0xFF) && (time2-time1 < SD_CMD_TIMEOUT) );
  
  if (time2-time1 >= SD_CMD_TIMEOUT) {
    return ERROR_SEND_CID_TOKEN_TIMEOUT;
  }

  if(tmp!=SD_TOK_READ_STARTBLOCK) {
    buf[0]=tmp;
    for(i=1;i<16;i++){
      buf[i]=SPI_ReadByte();
    }
  }
  else {
    for(i=0;i<16;i++){
      buf[i]=SPI_ReadByte();
    }
  }
  
  sd_cid->ManufacturerID = buf[0];
  sd_cid->OEM_AppliID = buf[1] << 8;
  sd_cid->OEM_AppliID |= buf[2];
  sd_cid->ProdName1 = buf[3] << 24;
  sd_cid->ProdName1 |= buf[4] << 16;
  sd_cid->ProdName1 |= buf[5] << 8;
  sd_cid->ProdName1 |= buf[6];
  sd_cid->ProdName2 = buf[7];
  if(ct==MULTIMEDIA_CARD) {
    sd_cid->Reserved1 = buf[8];
    sd_cid->ProdRev = buf[9];
    sd_cid->ProdSN = buf[10] << 24;
    sd_cid->ProdSN |= buf[11] << 16;
    sd_cid->ProdSN |= buf[12] << 8;
    sd_cid->ProdSN |= buf[13];
    sd_cid->ManufactDate = (buf[14] & 0x0F) << 4;
    sd_cid->ManufactDate |= (buf[14] & 0xF0) >> 4;
  }
  else {
    sd_cid->ProdRev = buf[8];
    sd_cid->ProdSN = buf[9] << 24;
    sd_cid->ProdSN |= buf[10] << 16;
    sd_cid->ProdSN |= buf[11] << 8;
    sd_cid->ProdSN |= buf[12];
    sd_cid->Reserved1 |= (buf[13] & 0xF0) >> 4;
    sd_cid->ManufactDate = (buf[13] & 0x0F) << 8;
    sd_cid->ManufactDate |= buf[14];
  }
  sd_cid->CID_CRC = (buf[15] & 0xFE) >> 1;
  sd_cid->Reserved2 = 1;
  
  // crc
  SPI_WriteDummyByte(); 
  SPI_WriteDummyByte();
  
  SPI_WriteDummyByte();
  
  return SD_OK;
}

SD_ERROR sd_read_csd(SD_CSD *sd_csd,CARD_TYPE ct) {
  
  uint32_t i;
  uint8_t tmp;
  uint32_t time1,time2;
  uint8_t buf[16];

  if(sd_send_command(CMD9,0)!=SD_OK) {
    return ERROR_SEND_CSD_TIMEOUT;
  }
  
  // Wait for the token
  time1=DWT_Get();
  do
  {
    tmp = SPI_ReadByte();
    time2=DWT_Get();
  }
  while ((tmp == 0xFF) && (time2-time1 < SD_CMD_TIMEOUT) );
  
  if (time2-time1 >= SD_CMD_TIMEOUT) {
    return ERROR_SEND_CSD_TOKEN_TIMEOUT;
  }

  if(tmp!=SD_TOK_READ_STARTBLOCK) {
    buf[0]=tmp;
    for(i=1;i<16;i++){
      buf[i]=SPI_ReadByte();
    }
  }
  else {
    for(i=0;i<16;i++){
      buf[i]=SPI_ReadByte();
    }
  }
  
  sd_csd->CSDStruct = (buf[0]&0xC0)>>6;
  sd_csd->SysSpecVersion = (buf[0]&0x3C)>>2;
  sd_csd->Reserved1 = buf[0]&0x03;
  sd_csd->TAAC = buf[1];
  sd_csd->NSAC = buf[2];
  sd_csd->MaxBusClkFrec = buf[3];
  sd_csd->CardComdClasses = (buf[4]<<4)|(buf[5]&0xF0)>>4;
  sd_csd->RdBlockLen = buf[5]&0x0F;
  sd_csd->PartBlockRead = (buf[6]&0x80)>>7;
  sd_csd->WrBlockMisalign = (buf[6]&0x40)>>6;
  sd_csd->RdBlockMisalign = (buf[6]&0x20)>>5;
  sd_csd->DSRImpl = (buf[6]&0x10)>>4;
  sd_csd->Reserved2 = 0;
  if ((ct == SD_CARD_STD_CAPACITY_V1_1) || 
      (ct == SD_CARD_STD_CAPACITY_V2_0) ||
      (ct == MULTIMEDIA_CARD)) {
    sd_csd->DeviceSize = ((buf[6]&0x03)<<10)|(buf[7]<<2)|((buf[8]&0xC0)>>6);
    sd_csd->MaxRdCurrentVDDMin = (buf[8]&0x38)>>3;
    sd_csd->MaxRdCurrentVDDMax = buf[8]&0x07;
    sd_csd->MaxWrCurrentVDDMin = (buf[9]&0xE0)>>5;
    sd_csd->MaxWrCurrentVDDMax = (buf[9]&0x1C)>>2;
    sd_csd->DeviceSizeMul = ((buf[9]&0x03)<<1)|(buf[10]&0x80)>>7;    
  }
  else if (ct == SD_CARD_HIGH_CAPACITY) {
    sd_csd->DeviceSize = ((buf[7]&0x3F)<<16)|(buf[8]<<8)|buf[9];    
  }
  
  if ((ct == SD_CARD_STD_CAPACITY_V1_1) || 
      (ct == SD_CARD_STD_CAPACITY_V2_0) ||
      (ct == SD_CARD_HIGH_CAPACITY)) {  
    sd_csd->EraseGrSize = (buf[10]&0x40)>>6;
    sd_csd->EraseGrMul = ((buf[10]&0x3F)<<1)|(buf[11]&0x80)>>7;
    sd_csd->WrProtectGrSize = buf[11]&0x7F;
  }
  else if (ct == MULTIMEDIA_CARD) {
    sd_csd->EraseGrSize = (buf[10]&0xF8)>>3;
    sd_csd->EraseGrMul = ((buf[10]&0x03)<<3)|(buf[11]&0xE0)>>5;
    sd_csd->WrProtectGrSize = buf[11]&0x1F;
  }
  
  sd_csd->WrProtectGrEnable = (buf[12]&0x80)>>7;
  sd_csd->ManDeflECC = (buf[12]&0x60)>>5;
  sd_csd->WrSpeedFact = (buf[12]&0x1C)>>2;
  sd_csd->MaxWrBlockLen = ((buf[12]&0x03)<<2)|((buf[13]&0xC0)>>6);
  sd_csd->WriteBlockPaPartial = (buf[13]&0x20) >> 5;
  sd_csd->Reserved3 = 0;
  sd_csd->ContentProtectAppli = buf[13]&0x01;
  sd_csd->FileFormatGroup = (buf[14]&0x80)>>7;
  sd_csd->CopyFlag = (buf[14]&0x40)>>6;
  sd_csd->PermWrProtect = (buf[14]&0x20)>>5;
  sd_csd->TempWrProtect = (buf[14]&0x10)>>4;
  sd_csd->FileFormat = (buf[14]&0x0C)>>2;
  sd_csd->ECC = buf[14]&0x03;
  sd_csd->CSD_CRC = (buf[15]&0xFE)>>1;
  sd_csd->Reserved4 = 1;
  
  // crc
  SPI_WriteDummyByte(); 
  SPI_WriteDummyByte();
  
  SPI_WriteDummyByte();
  
  return SD_OK;
}
