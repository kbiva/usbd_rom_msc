/*
 * uart.c
 *
 *  Created on: 2014.07.04
 *      Author: Kestutis Bivainis
 */

#include "board.h"
#include "uart.h"
#include <string.h>

static UART_HANDLE_T *uartHandle;

/* Use a buffer size larger than the expected return value of
   uart_get_mem_size() for the static UART handle type */
static uint32_t uartHandleMEM[0x10];

void UART0_IRQHandler(void) {
  
  LPC_UARTD_API->uart_isr(uartHandle);
}

static void errorUART(void) {
  
  Board_LED_Set(0, true);
  while (1){};
}

void putLineUART(const char *send_data) {
  
  UART_PARAM_T param;

  param.buffer = (uint8_t *) send_data;
  param.size = strlen(send_data);

  /* Interrupt mode, do not append CR/LF to sent data */
  param.transfer_mode = TX_MODE_SZERO;
  param.driver_mode = DRIVER_MODE_POLLING;

  if (LPC_UARTD_API->uart_put_line(uartHandle, &param)) {
    errorUART();
  }
}

static void Init_UART0_PinMux(void) {
  
  /* UART signals on pins PIO0_13 (FUNC0, U0_TXD) and PIO0_18 (FUNC0, U0_RXD) */
  Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 13, (IOCON_FUNC0 | IOCON_MODE_INACT | IOCON_DIGMODE_EN));
  Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 18, (IOCON_FUNC0 | IOCON_MODE_INACT | IOCON_DIGMODE_EN));

  /* UART signal muxing via SWM */
  Chip_SWM_MovablePortPinAssign(SWM_UART0_RXD_I, 0, 13);
  Chip_SWM_MovablePortPinAssign(SWM_UART0_TXD_O, 0, 18);
}

static uint8_t setupUART(void) {
  
  uint32_t ret_value;

  /* 115.2KBPS, 8N1, ASYNC mode, no errors, clock filled in later */
  UART_CONFIG_T cfg = {
    0,        /* U_PCLK frequency in Hz */
    115200,    /* Baud Rate in Hz */
    1,        /* 8N1 */
    0,        /* Asynchronous Mode */
    NO_ERR_EN  /* Enable No Errors */
  };

  /* Initialize UART0 */
  Chip_UART_Init(LPC_USART0);

  Chip_Clock_SetUARTFRGDivider(1);

  /* Perform a sanity check on the storage allocation */
  if (LPC_UARTD_API->uart_get_mem_size() > sizeof(uartHandleMEM)) {
    /* Example only: this should never happen and probably isn't needed for
       most UART code. */
    return 0;
  }

  /* Setup the UART handle */
  uartHandle = LPC_UARTD_API->uart_setup((uint32_t) LPC_USART0, (uint8_t *) &uartHandleMEM);
  if (uartHandle == NULL) {
    return 0;
  }

  /* Need to tell UART ROM API function the current UART peripheral clock
       speed */
  cfg.sys_clk_in_hz = Chip_Clock_GetSystemClockRate();

  /* Initialize the UART with the configuration parameters */
  ret_value = LPC_UARTD_API->uart_init(uartHandle, &cfg);
  LPC_SYSCTL->FRGCTRL = ret_value;
  return 1;
}

uint8_t init_uart(uint32_t baud) {
  
  Init_UART0_PinMux();
  Chip_Clock_SetUARTBaseClockRate((baud * 128), true);
  return setupUART();
}
