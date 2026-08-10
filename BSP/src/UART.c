/*
 * UART.c
 *
 *  Created on: 2026?8?6?
 *      Author: MXQ
 */

#include "ch32v00x.h"
#include "UART.h"

#include <string.h>
#include <stdarg.h>

volatile uint8_t uart_tx_busy;

uint8_t uart_rx_buf[UART_RX_BUF_SIZE];

void UART_Init(uint32_t baudrate)
{
    GPIO_InitTypeDef gpio;
    USART_InitTypeDef uart;
    DMA_InitTypeDef dma;
    NVIC_InitTypeDef nvic;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD | RCC_APB2Periph_USART1, ENABLE);
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

    gpio.GPIO_Pin = GPIO_Pin_5;
    gpio.GPIO_Speed = GPIO_Speed_30MHz;
    gpio.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOD, &gpio);

    gpio.GPIO_Pin = GPIO_Pin_6;
    gpio.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    gpio.GPIO_Speed = GPIO_Speed_30MHz;
    GPIO_Init(GPIOD, &gpio);

    uart.USART_BaudRate = baudrate;
    uart.USART_WordLength = USART_WordLength_8b;
    uart.USART_StopBits = USART_StopBits_1;
    uart.USART_Parity = USART_Parity_No;
    uart.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    uart.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
    USART_Init(USART1, &uart);

    DMA_DeInit(DMA1_Channel5);

    dma.DMA_PeripheralBaseAddr = (uint32_t)&USART1->DATAR;
    dma.DMA_MemoryBaseAddr = (uint32_t)uart_rx_buf;
    dma.DMA_DIR = DMA_DIR_PeripheralSRC;
    dma.DMA_BufferSize = UART_RX_BUF_SIZE;
    dma.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    dma.DMA_MemoryInc = DMA_MemoryInc_Enable;
    dma.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    dma.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    dma.DMA_Mode = DMA_Mode_Circular;
    dma.DMA_Priority = DMA_Priority_High;
    dma.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(DMA1_Channel5, &dma);
    USART_DMACmd(USART1, USART_DMAReq_Rx, ENABLE);

    DMA_DeInit(DMA1_Channel4);

    dma.DMA_PeripheralBaseAddr = (uint32_t)&USART1->DATAR;
    dma.DMA_DIR = DMA_DIR_PeripheralDST;
    dma.DMA_Mode = DMA_Mode_Normal;
    dma.DMA_Priority = DMA_Priority_Medium;
    DMA_Init(DMA1_Channel4, &dma);
    USART_DMACmd(USART1, USART_DMAReq_Tx, ENABLE);

    nvic.NVIC_IRQChannel = DMA1_Channel4_IRQn;
    nvic.NVIC_IRQChannelPreemptionPriority = 2;
    nvic.NVIC_IRQChannelSubPriority = 2;
    nvic.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&nvic);

    nvic.NVIC_IRQChannel = DMA1_Channel5_IRQn;
    nvic.NVIC_IRQChannelPreemptionPriority = 2;
    nvic.NVIC_IRQChannelSubPriority = 1;
    nvic.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&nvic);

    DMA_Cmd(DMA1_Channel5, ENABLE);
    USART_Cmd(USART1, ENABLE);
}

void UART_DMA_Send(uint8_t *buf, uint16_t len)
{
    while(uart_tx_busy);

    uart_tx_busy = 1;

    DMA_Cmd(DMA1_Channel4, DISABLE);

    DMA1_Channel4->MADDR = (uint32_t)buf;
    DMA1_Channel4->CNTR = len;

    DMA_ClearFlag(DMA1_FLAG_TC4);
    DMA_ITConfig(DMA1_Channel4, DMA_IT_TC, ENABLE);

    DMA_Cmd(DMA1_Channel4, ENABLE);
}

void UART_DMA_SendString(char *str)
{
    UART_DMA_Send(str, strlen(str));
}

void myprintf(char *str, ...)
{
    va_list pArgs;
    char p[100];

    va_start(pArgs, str);
    vsprintf(p, str, pArgs);
    va_end(pArgs);

    UART_DMA_SendString(p);
}

uint16_t UART_DMA_GetRxLen()
{
    return UART_RX_BUF_SIZE - DMA_GetCurrDataCounter(DMA1_Channel5);
}

void DMA1_Channel4_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void DMA1_Channel4_IRQHandler(void)
{
    if (DMA_GetITStatus(DMA1_IT_TC4)) {
        DMA_ClearITPendingBit(DMA1_IT_TC4);

        uart_tx_busy = 0;
    }
}

void DMA1_Channel5_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void DMA1_Channel5_IRQHandler(void)
{

}
