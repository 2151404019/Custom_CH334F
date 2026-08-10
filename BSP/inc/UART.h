/*
 * UART.h
 *
 *  Created on: 2026?8?6?
 *      Author: MXQ
 */

#ifndef BSP_INC_UART_H_
#define BSP_INC_UART_H_

#define UART_RX_BUF_SIZE    128

void UART_Init(uint32_t baudrate);
void UART_DMA_Send(uint8_t *buf, uint16_t len);
void UART_DMA_SendString(char *str);
void myprintf(char *str, ...);
uint16_t UART_DMA_GetRxLen();

#endif /* BSP_INC_UART_H_ */
