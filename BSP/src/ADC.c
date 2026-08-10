/*
 * ADC.c
 *
 *  Created on: 2026?7?20?
 *      Author: MXQ
 */

#include "ch32v00x.h"

#define     DMA_BUF_SIZE        19
#define     DISCARD_DATA_NUM    4

volatile uint16_t PD_Vol;        //mv
uint16_t DMA_buf[DMA_BUF_SIZE];

void ADC_DMA_Init()
{
    GPIO_InitTypeDef gpio = {0};
    ADC_InitTypeDef adc = {0};
    NVIC_InitTypeDef nvic = {0};
    DMA_InitTypeDef dma = {0};

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO | RCC_APB2Periph_ADC1, ENABLE);
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

    GPIO_PinRemapConfig(GPIO_Remap_PA1_2, DISABLE);

    gpio.GPIO_Pin = GPIO_Pin_2;
    gpio.GPIO_Mode = GPIO_Mode_AIN;
    gpio.GPIO_Speed = GPIO_Speed_30MHz;
    GPIO_Init(GPIOA, &gpio);

    ADC_DeInit(ADC1);
    adc.ADC_Mode = ADC_Mode_Independent;
    adc.ADC_ScanConvMode = ENABLE;
    adc.ADC_ContinuousConvMode = ENABLE;
    adc.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
    adc.ADC_DataAlign = ADC_DataAlign_Right;
    adc.ADC_NbrOfChannel = 1;
    ADC_Init(ADC1, &adc);

    ADC_DMACmd(ADC1, ENABLE);
    ADC_Cmd(ADC1, ENABLE);

    ADC_ResetCalibration(ADC1);
    while(ADC_GetResetCalibrationStatus(ADC1));
    ADC_StartCalibration(ADC1);
    while(ADC_GetCalibrationStatus(ADC1));

    ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 1, ADC_SampleTime_241Cycles);

    nvic.NVIC_IRQChannel = DMA1_Channel1_IRQn;
    nvic.NVIC_IRQChannelPreemptionPriority = 1;
    nvic.NVIC_IRQChannelSubPriority = 2;
    nvic.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&nvic);

    DMA_DeInit(DMA1_Channel1);
    dma.DMA_PeripheralBaseAddr  = (uint32_t)&ADC1->RDATAR;
    dma.DMA_MemoryBaseAddr      = (uint32_t)DMA_buf;
    dma.DMA_DIR                 = DMA_DIR_PeripheralSRC;
    dma.DMA_BufferSize          = DMA_BUF_SIZE;
    dma.DMA_PeripheralInc       = DMA_PeripheralInc_Disable;
    dma.DMA_MemoryInc           = DMA_MemoryInc_Enable;
    dma.DMA_PeripheralDataSize  = DMA_PeripheralDataSize_HalfWord;
    dma.DMA_MemoryDataSize      = DMA_MemoryDataSize_HalfWord;
    dma.DMA_Mode                = DMA_Mode_Circular;
    dma.DMA_Priority            = DMA_Priority_Medium;
    dma.DMA_M2M                 = DMA_M2M_Disable;
    DMA_Init(DMA1_Channel1, &dma);

    DMA_ITConfig(DMA1_Channel1, DMA_IT_TC, ENABLE);
    DMA_Cmd(DMA1_Channel1, ENABLE);

    ADC_SoftwareStartConvCmd(ADC1, ENABLE);
}

void shell_sort(uint16_t *buf, uint16_t len)
{
    for (int gap = len/ 2; gap > 0; gap/= 2)
    {
        for (int i = gap; i < len; ++i)
        {
            uint16_t Midd = buf[i];
            uint16_t j = i;
            while(j >= gap && buf[j - gap] > Midd)
            {
                buf[j] = buf[j - gap];
                j -= gap;
            }
            buf[j] = Midd;
        }
    }
}

void DMA1_Channel1_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void DMA1_Channel1_IRQHandler(void)
{
    if (DMA_GetITStatus(DMA1_IT_TC1)) {
        shell_sort(DMA_buf, DMA_BUF_SIZE);

        uint32_t sum = 0;

        for (int i = DISCARD_DATA_NUM; i < (DMA_BUF_SIZE - DISCARD_DATA_NUM); ++i) {
            sum+= DMA_buf[i];
        }

        PD_Vol = sum * 100/ 31;     //((sum/(DMA_BUF_SIZE- 2*DISCARD_DATA_NUM) * 3300)/ 1023) * 11

        DMA_ClearITPendingBit(DMA1_IT_TC1);
    }
}
