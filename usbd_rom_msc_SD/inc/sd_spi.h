/*
 * sd_spi.h
 *
 *  Created on: 2014.07.04
 *      Author: Kestutis Bivainis
 */

#ifndef _SD_SPI_H_
#define _SD_SPI_H_

#include <stdint.h>

#define PIN_CLK 16
#define PIN_CS 27
#define PIN_MOSI 0
#define PIN_MISO 28

#define SD_BLOCKSIZE 512
#define SD_BLOCKSIZE_NBITS 9

typedef enum {
  SD_OK,
  SD_TIMEOUT,
  ERROR_GO_IDLE_STATE_TIMEOUT,
  ERROR_GO_IDLE_STATE_RESPONSE,
  ERROR_SEND_IF_COND_TIMEOUT,
  ERROR_SEND_IF_COND_RESPONSE,
  ERROR_READ_OCR_TIMEOUT,
  ERROR_READ_OCR_RESPONSE,
  ERROR_APP_CMD_TIMEOUT,
  ERROR_SD_SEND_OP_COND_TIMEOUT,
  ERROR_INIT_TIMEOUT,
  ERROR_SEND_CID_TIMEOUT,
  ERROR_SEND_CID_TOKEN_TIMEOUT,
  ERROR_SEND_CSD_TIMEOUT,
  ERROR_SEND_CSD_TOKEN_TIMEOUT,
  ERROR_SEND_OP_COND_TIMEOUT,
  ERROR_SEND_OP_COND_RESPONSE  
} SD_ERROR;

typedef enum {
  SD_CARD_STD_CAPACITY_V1_1,
  SD_CARD_STD_CAPACITY_V2_0,
  SD_CARD_HIGH_CAPACITY,
  MULTIMEDIA_CARD,  
} CARD_TYPE;


#define R1_IN_IDLE_STATE 0x01
#define R1_ILLEGAL_COMMAND 0x04

#define DATA_RESPONSE_TOKEN_DATA_ACCEPTED 0x05

#define SD_TOK_READ_STARTBLOCK 0xFE
#define SD_TOK_WRITE_STARTBLOCK 0xFE

// Mask off the bits in the OCR corresponding to voltage range 3.2V to 3.4V, OCR bits 20 and 21
#define MSK_OCR_33 0x03

// timeout 0.5 sec
#define SD_CMD_TIMEOUT (SystemCoreClock/2)

// Responses
#define R1  0x0100
#define R1b 0x1100
#define R2  0x0200
#define R3  0x0500
#define R7  0x0500

// Commands: (command|response)
// Basic command set
// GO_IDLE_STATE
#define CMD0 (0|R1)
// SEND_OP_COND
#define CMD1 (1|R1)
// SWITCH_FUNC
#define CMD6 (6|R1)
// SEND_IF_COND
#define CMD8 (8|R7)
// SEND_CSD
#define CMD9 (9|R1)
// SEND_CID
#define CMD10 (10|R1)
// STOP_TRANSMISSION
#define CMD12 (12|R1b)
// SEND_STATUS
#define CMD13 (13|R2)
// SET_BLOCKLEN
#define CMD16 (16|R1)
// READ_SINGLE_BLOCK
#define CMD17 (17|R1)
// READ_MULTIPLE_BLOCK
#define CMD18 (18|R1)
// WRITE_BLOCK
#define CMD24 (24|R1)
// WRITE_MULTIPLE_BLOCK
#define CMD25 (25|R1)
// PROGRAM_CSD
#define CMD27 (27|R1)
// SET_WRITE_PROT
#define CMD28 (28|R1b)
// CLR_WRITE_PROT
#define CMD29 (29|R1b)
// SEND_WRITE_PROT
#define CMD30 (30|R1)
// ERASE_WR_BLK_START_ADDR
#define CMD32 (32|R1)
// ERASE_WR_BLK_END_ADDR
#define CMD33 (33|R1)
// ERASE
#define CMD38 (38|R1b)
// LOCK_UNLOCK
#define CMD42 (42|R1)
// APP_CMD
#define CMD55 (55|R1)
// GEN_CMD
#define CMD56 (56|R1)
// READ_OCR
#define CMD58 (58|R3)
// CRC_ON_OFF
#define CMD59 (59|R1)

// Application-specific commands
// SD_STATUS
#define ACMD13 (13|R2)
// SEND_NUM_WR_BLOCKS
#define ACMD22 (22|R1)
// SET_WR_BLK_ERASE_COUNT
#define ACMD23 (23|R1)
// SD_SEND_OP_COND
#define ACMD41 (41|R1)
// SET_CLR_CARD_DETECT
#define ACMD42 (42|R1)
// SEND_SCR
#define ACMD51 (51|R1)

typedef struct
{
  uint8_t  ManufacturerID;       /*!< ManufacturerID */
  uint16_t OEM_AppliID;          /*!< OEM/Application ID */
  uint32_t ProdName1;            /*!< Product Name part1 */
  uint8_t  ProdName2;            /*!< Product Name part2*/
  uint8_t  ProdRev;              /*!< Product Revision */
  uint32_t ProdSN;               /*!< Product Serial Number */
  uint8_t  Reserved1;            /*!< Reserved1 */
  uint16_t ManufactDate;         /*!< Manufacturing Date */
  uint8_t  CID_CRC;              /*!< CID CRC */
  uint8_t  Reserved2;            /*!< always 1 */
} SD_CID;

typedef struct
{
  uint8_t  CSDStruct;            /*!< CSD structure */
  uint8_t  SysSpecVersion;       /*!< System specification version */
  uint8_t  Reserved1;            /*!< Reserved */
  uint8_t  TAAC;                 /*!< Data read access-time 1 */
  uint8_t  NSAC;                 /*!< Data read access-time 2 in CLK cycles */
  uint8_t  MaxBusClkFrec;        /*!< Max. bus clock frequency */
  uint16_t CardComdClasses;      /*!< Card command classes */
  uint8_t  RdBlockLen;           /*!< Max. read data block length */
  uint8_t  PartBlockRead;        /*!< Partial blocks for read allowed */
  uint8_t  WrBlockMisalign;      /*!< Write block misalignment */
  uint8_t  RdBlockMisalign;      /*!< Read block misalignment */
  uint8_t  DSRImpl;              /*!< DSR implemented */
  uint8_t  Reserved2;            /*!< Reserved */
  uint32_t DeviceSize;           /*!< Device Size */
  uint8_t  MaxRdCurrentVDDMin;   /*!< Max. read current @ VDD min */
  uint8_t  MaxRdCurrentVDDMax;   /*!< Max. read current @ VDD max */
  uint8_t  MaxWrCurrentVDDMin;   /*!< Max. write current @ VDD min */
  uint8_t  MaxWrCurrentVDDMax;   /*!< Max. write current @ VDD max */
  uint8_t  DeviceSizeMul;        /*!< Device size multiplier */
  uint8_t  EraseGrSize;          /*!< Erase group size */
  uint8_t  EraseGrMul;           /*!< Erase group size multiplier */
  uint8_t  WrProtectGrSize;      /*!< Write protect group size */
  uint8_t  WrProtectGrEnable;    /*!< Write protect group enable */
  uint8_t  ManDeflECC;           /*!< Manufacturer default ECC */
  uint8_t  WrSpeedFact;          /*!< Write speed factor */
  uint8_t  MaxWrBlockLen;        /*!< Max. write data block length */
  uint8_t  WriteBlockPaPartial;  /*!< Partial blocks for write allowed */
  uint8_t  Reserved3;            /*!< Reserded */
  uint8_t  ContentProtectAppli;  /*!< Content protection application */
  uint8_t  FileFormatGroup;      /*!< File format group */
  uint8_t  CopyFlag;             /*!< Copy flag (OTP) */
  uint8_t  PermWrProtect;        /*!< Permanent write protection */
  uint8_t  TempWrProtect;        /*!< Temporary write protection */
  uint8_t  FileFormat;           /*!< File Format */
  uint8_t  ECC;                  /*!< ECC code */
  uint8_t  CSD_CRC;              /*!< CSD CRC */
  uint8_t  Reserved4;            /*!< always 1*/
} SD_CSD;

typedef struct
{
  SD_CSD SD_csd;
  SD_CID SD_cid;
  uint64_t CardCapacity;  /*!< Card Capacity */
  uint32_t CardBlockSize; /*!< Card Block Size */
  CARD_TYPE CardType;
} SD_CardInfo;

SD_ERROR init_sd_spi(SD_CardInfo *cardinfo);
uint8_t sd_read_block (uint32_t blockaddr,uint8_t *data);
uint8_t sd_write_block (uint32_t blockaddr,uint8_t *data);
SD_ERROR sd_read_cid(SD_CID *sd_cid,CARD_TYPE ct);
SD_ERROR sd_read_csd(SD_CSD *sd_csd,CARD_TYPE ct);

#endif
