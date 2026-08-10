/*
 * btn.c
 *
 *  Created on: 2026?7?16?
 *      Author: MXQ
 */

#include "ch32v00x.h"

#include "lwbtn/lwbtn.h"

#define BTN_GPIO       GPIO_Pin_0
#define BTN_PORT       GPIOD

extern uint32_t tick;

__attribute__((weak)) void button_callback(struct lwbtn* lw, struct lwbtn_btn* btn, lwbtn_evt_t evt){};

void btn_io_Init(uint8_t nv)
{
    GPIO_InitTypeDef gpio = {0};

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD | RCC_APB2Periph_AFIO, ENABLE);

    gpio.GPIO_Pin = BTN_GPIO;
    gpio.GPIO_Mode = GPIO_Mode_IPU;
    gpio.GPIO_Speed = GPIO_Speed_30MHz;
    GPIO_Init(BTN_PORT, &gpio);

    EXTI_InitTypeDef exti = {0};
    NVIC_InitTypeDef nvic = {0};

    if (nv) {
        GPIO_EXTILineConfig(GPIO_PortSourceGPIOD, GPIO_PinSource0);
        exti.EXTI_Line = EXTI_Line0;
        exti.EXTI_Mode = EXTI_Mode_Interrupt;
        exti.EXTI_Trigger = EXTI_Trigger_Falling;
        exti.EXTI_LineCmd = ENABLE;
        EXTI_Init(&exti);

        nvic.NVIC_IRQChannel = EXTI7_0_IRQn;
        nvic.NVIC_IRQChannelPreemptionPriority = 3;
        nvic.NVIC_IRQChannelSubPriority = 3;
        nvic.NVIC_IRQChannelCmd = ENABLE;
        NVIC_Init(&nvic);
    }
}

lwbtn_argdata_port_pin_state_t button = {
    .pin     =  BTN_GPIO,
    .port    =  BTN_PORT,
    .state =    0
};

lwbtn_btn_t btn[] = {
    {.arg = &button},
};

uint8_t get_button_state(struct lwbtn* lw, struct lwbtn_btn* btn){
    lwbtn_argdata_port_pin_state_t* cfg = btn->arg;
    uint8_t state = GPIO_ReadInputDataBit(cfg->port, cfg->pin);

    return (state == cfg->state) ? 1 : 0;
}

void button_init(){
    btn_io_Init(0);
    lwbtn_init_ex(NULL, btn, 1, get_button_state, button_callback);
}

void get_btn(){

    lwbtn_process(tick);
}
