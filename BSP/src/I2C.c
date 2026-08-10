/*
 * I2C.c
 *
 *  Created on: 2026?7?13?
 *      Author: MXQ
 */
#include "ch32v00x.h"

#define I2Cx        I2C1
#define TIMEOUT     100     //ms

extern volatile uint32_t tick;

void I2C_Slave_Init(uint16_t addr)
{
    GPIO_InitTypeDef gpio = {0};
    I2C_InitTypeDef i2c = {0};
    NVIC_InitTypeDef nvic = {0};

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC | RCC_APB2Periph_AFIO, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);

    gpio.GPIO_Pin = GPIO_Pin_1;
    gpio.GPIO_Mode = GPIO_Mode_AF_OD;
    gpio.GPIO_Speed = GPIO_Speed_30MHz;
    GPIO_Init(GPIOC, &gpio);

    gpio.GPIO_Pin = GPIO_Pin_2;
    gpio.GPIO_Mode = GPIO_Mode_AF_OD;
    gpio.GPIO_Speed = GPIO_Speed_30MHz;
    GPIO_Init(GPIOC, &gpio);

    i2c.I2C_ClockSpeed = 400000;
    i2c.I2C_Mode = I2C_Mode_I2C;
    i2c.I2C_DutyCycle = I2C_DutyCycle_2;
    i2c.I2C_OwnAddress1 = addr;
    i2c.I2C_Ack = I2C_Ack_Enable;
    i2c.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
    I2C_Init(I2C1, &i2c);

    nvic.NVIC_IRQChannel = I2C1_EV_IRQn;
    nvic.NVIC_IRQChannelPreemptionPriority = 0;
    nvic.NVIC_IRQChannelSubPriority = 0;
    nvic.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&nvic);

    nvic.NVIC_IRQChannel = I2C1_ER_IRQn;
    nvic.NVIC_IRQChannelPreemptionPriority = 0;
    nvic.NVIC_IRQChannelSubPriority = 0;
    nvic.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&nvic);

    I2C_ITConfig(I2C1, I2C_IT_BUF | I2C_IT_EVT | I2C_IT_ERR, ENABLE);

    I2C_Cmd(I2C1, ENABLE);
}

void I2C_Master_Init()
{
    NVIC_InitTypeDef nvic = {0};

    I2C_Cmd(I2C1, DISABLE);

    nvic.NVIC_IRQChannel = I2C1_EV_IRQn;
    nvic.NVIC_IRQChannelPreemptionPriority = 0;
    nvic.NVIC_IRQChannelSubPriority = 0;
    nvic.NVIC_IRQChannelCmd = DISABLE;
    NVIC_Init(&nvic);

    nvic.NVIC_IRQChannel = I2C1_ER_IRQn;
    nvic.NVIC_IRQChannelPreemptionPriority = 0;
    nvic.NVIC_IRQChannelSubPriority = 0;
    nvic.NVIC_IRQChannelCmd = DISABLE;
    NVIC_Init(&nvic);

    I2C_ITConfig(I2C1, I2C_IT_BUF | I2C_IT_EVT | I2C_IT_ERR, DISABLE);

    I2C_Cmd(I2C1, ENABLE);
}

uint8_t Wait_Event(uint32_t e)
{
    uint32_t t = tick;
    uint8_t sta = 1;
    while(!I2C_CheckEvent(I2Cx, e))
    {
        if ((tick - t) > TIMEOUT) {
            sta = 0;
            break;
        }
    }

    return sta;
}

uint8_t I2C_WriteData(uint8_t addr,uint8_t *data, uint8_t len, uint32_t timeout)
{
    uint8_t i = 0, sta = 0;
    I2C_GenerateSTART(I2Cx, ENABLE);

    sta = Wait_Event(I2C_EVENT_MASTER_MODE_SELECT);
    I2C_Send7bitAddress(I2Cx, addr, I2C_Direction_Transmitter);

    sta = Wait_Event(I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED);
    while(i < len)
    {
         if (I2C_GetFlagStatus(I2Cx, I2C_FLAG_TXE) != RESET) {
             I2C_SendData(I2Cx, data[i]);
             i++;
         }
         sta = Wait_Event(I2C_EVENT_MASTER_BYTE_TRANSMITTED);
    }

    I2C_GenerateSTOP(I2Cx, ENABLE);

    return sta;
}

uint8_t I2C_WriteData8Bit(uint8_t addr, uint8_t reg, uint8_t data)
{
    uint8_t sta = 0;

    I2C_GenerateSTART(I2Cx, ENABLE);

    sta = Wait_Event(I2C_EVENT_MASTER_MODE_SELECT);
    I2C_Send7bitAddress(I2Cx, addr, I2C_Direction_Transmitter);

    sta = Wait_Event(I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED);
    I2C_SendData(I2Cx, reg);

    sta = Wait_Event(I2C_EVENT_MASTER_BYTE_TRANSMITTING);
    if (I2C_GetFlagStatus(I2Cx, I2C_FLAG_TXE) != RESET) {
        I2C_SendData(I2Cx, data);
    }

    sta = Wait_Event(I2C_EVENT_MASTER_BYTE_TRANSMITTED);
    I2C_GenerateSTOP(I2Cx, ENABLE);

    return sta;
}

uint8_t I2C_WriteData16Bit(uint8_t addr, uint8_t reg, uint16_t data)
{
    uint8_t datH, datL, sta = 0;

    datH = data >> 8;
    datL = data;

    I2C_GenerateSTART(I2Cx, ENABLE);

    sta = Wait_Event(I2C_EVENT_MASTER_MODE_SELECT);
    I2C_Send7bitAddress(I2Cx, addr, I2C_Direction_Transmitter);

    sta = Wait_Event(I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED);
    I2C_SendData(I2Cx, reg);

    sta = Wait_Event(I2C_EVENT_MASTER_BYTE_TRANSMITTING);
    if (I2C_GetFlagStatus(I2Cx, I2C_FLAG_TXE) != RESET) {
        I2C_SendData(I2Cx, datH);
    }

    sta = Wait_Event(I2C_EVENT_MASTER_BYTE_TRANSMITTING);
    if (I2C_GetFlagStatus(I2Cx, I2C_FLAG_TXE) != RESET) {
        I2C_SendData(I2Cx, datL);
    }
    sta = Wait_Event(I2C_EVENT_MASTER_BYTE_TRANSMITTED);

    I2C_GenerateSTOP(I2Cx, ENABLE);

    return sta;
}

uint8_t I2C_ReadData8Bit(uint8_t addr, uint8_t reg, uint8_t *data)
{
    uint8_t sta = 0;
    I2C_GenerateSTART(I2Cx, ENABLE);
    I2C_AcknowledgeConfig(I2Cx, ENABLE);

    sta = Wait_Event(I2C_EVENT_MASTER_MODE_SELECT);

    I2C_Send7bitAddress(I2Cx, addr, I2C_Direction_Transmitter);
    sta = Wait_Event(I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED);

    I2C_SendData(I2Cx, reg);
    sta = Wait_Event(I2C_EVENT_MASTER_BYTE_TRANSMITTED);

    I2C_GenerateSTART(I2Cx, ENABLE);
    sta = Wait_Event(I2C_EVENT_MASTER_MODE_SELECT);

    I2C_Send7bitAddress(I2Cx, addr, I2C_Direction_Receiver);
    sta = Wait_Event(I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED);

    I2C_AcknowledgeConfig(I2Cx, DISABLE);
    I2C_GenerateSTOP(I2Cx, ENABLE);

    sta = Wait_Event(I2C_EVENT_MASTER_BYTE_RECEIVED);
    *data = I2C_ReceiveData(I2Cx);

    return sta;
}

uint8_t I2C_ReadData(uint8_t addr, uint8_t reg, uint8_t len, uint8_t *buf)
{
    uint8_t sta = 0;

    I2C_GenerateSTART(I2Cx, ENABLE);
    I2C_AcknowledgeConfig(I2Cx, ENABLE);

    sta = Wait_Event(I2C_EVENT_MASTER_MODE_SELECT);

    I2C_Send7bitAddress(I2Cx, addr, I2C_Direction_Transmitter);
    sta = Wait_Event(I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED);

    I2C_SendData(I2Cx, reg);
    sta = Wait_Event(I2C_EVENT_MASTER_BYTE_TRANSMITTED);

    I2C_GenerateSTART(I2Cx, ENABLE);
    sta = Wait_Event(I2C_EVENT_MASTER_MODE_SELECT);

    I2C_Send7bitAddress(I2Cx, addr, I2C_Direction_Receiver);
    sta = Wait_Event(I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED);

    for(u8 i = 0 ; i < len ;i ++)
    {
        if(i==len-1)
            I2C_AcknowledgeConfig(I2Cx, DISABLE);
        else
            I2C_AcknowledgeConfig(I2Cx, ENABLE);
        sta = Wait_Event(I2C_EVENT_MASTER_BYTE_RECEIVED);
        buf[i] = I2C_ReceiveData(I2Cx);
    }

    I2C_GenerateSTOP(I2Cx, ENABLE);

    return sta;
}

void EEP_Write(uint8_t reg, uint8_t data)
{
    I2C_WriteData8Bit((0xA0), reg, data);
}

void EEP_Read()
{
    uint8_t buf[16];

    I2C_ReadData((0xA0), 0x00, 16, buf);
}
