/*
 * TIM.c
 *
 *  Created on: 2026?7?16?
 *      Author: MXQ
 */
#include "ch32v00x.h"

void TIM1_Count_Init(uint16_t arr, uint16_t psc)
{
    NVIC_InitTypeDef nvic = {0};
    TIM_TimeBaseInitTypeDef tim = {0};

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);

    tim.TIM_Period = arr;
    tim.TIM_Prescaler = psc;
    tim.TIM_ClockDivision = TIM_CKD_1;
    tim.TIM_CounterMode = TIM_CounterMode_Up;
    tim.TIM_RepetitionCounter = 50;
    TIM_TimeBaseInit(TIM1, &tim);

    TIM_ClearITPendingBit(TIM1, TIM_IT_Update);

    nvic.NVIC_IRQChannel = TIM1_UP_IRQn;
    nvic.NVIC_IRQChannelPreemptionPriority = 5;
    nvic.NVIC_IRQChannelSubPriority = 1;
    nvic.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&nvic);

    TIM_ITConfig(TIM1, TIM_IT_Update, ENABLE);

    TIM_Cmd(TIM1, ENABLE);
}

void TIM2_init(uint16_t arr, uint16_t psc)
{
    NVIC_InitTypeDef nvic = {0};
    TIM_TimeBaseInitTypeDef tim = {0};

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

    tim.TIM_Period = arr;
    tim.TIM_Prescaler = psc;
    tim.TIM_ClockDivision = TIM_CKD_1;
    tim.TIM_CounterMode = TIM_CounterMode_Up;
    tim.TIM_RepetitionCounter = 50;
    TIM_TimeBaseInit(TIM2, &tim);

    TIM_ClearITPendingBit(TIM2, TIM_IT_Update);

    nvic.NVIC_IRQChannel = TIM2_IRQn;
    nvic.NVIC_IRQChannelPreemptionPriority = 5;
    nvic.NVIC_IRQChannelSubPriority = 2;
    nvic.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&nvic);

    TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);

    TIM_Cmd(TIM2, ENABLE);
}

