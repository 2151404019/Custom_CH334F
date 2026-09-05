/*
 * CH334.c
 *
 *  Created on: 2026?7?16?
 *      Author: MXQ
 */

#include "ch32v00x.h"

#include "flash.h"

//#define EEPROM_SIZE 256

//static uint8_t eeprom[EEPROM_SIZE];         //测试
static volatile uint16_t eeprom_addr  = 0;  //当前eeprom地址
static volatile uint8_t addr_received = 0;  //是否已经收到地址

static volatile uint8_t page_base     = 0;
static volatile uint8_t page_offset   = 0;

static volatile uint8_t eeprom_busy   = 0;

extern volatile uint32_t tick;
uint32_t ch334_time;

void CH334_Init()
{
    GPIO_InitTypeDef gpio = {0};

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE);

    GPIO_PinRemapConfig(GPIO_Remap_PA1_2, DISABLE);

    gpio.GPIO_Pin = GPIO_Pin_1;
    gpio.GPIO_Mode = GPIO_Mode_Out_PP;
    gpio.GPIO_Speed = GPIO_Speed_30MHz;
    GPIO_Init(GPIOA, &gpio);

//    eeprom[0]  = 0x86;
//    eeprom[1]  = 0x1A;
//    eeprom[2]  = 0x91;
//    eeprom[3]  = 0x80;
//    eeprom[4]  = 0xB2;
//    eeprom[5]  = 0xFF;
//    eeprom[6]  = 0x00;
//    eeprom[7]  = 0x04;
//    eeprom[8]  = 0xFA;
//    eeprom[9]  = 0x5A;
//    eeprom[10] = 0x57;
//    eeprom[11] = 0x94;
//    eeprom[12] = 0x18;
//    eeprom[13] = 0xC5;
//    eeprom[14] = 0x95;
//    eeprom[15] = 0xD4;
//    for (int i = 16; i < 256; ++i) {
//        eeprom[i] = i;
//    }
    GPIO_WriteBit(GPIOA, GPIO_Pin_1, RESET);
    Delay_Ms(20);
    GPIO_WriteBit(GPIOA, GPIO_Pin_1, SET);
}

void CH334_NRST()
{
    GPIO_WriteBit(GPIOA, GPIO_Pin_1, RESET);
    Delay_Ms(20);
    GPIO_WriteBit(GPIOA, GPIO_Pin_1, SET);
}

void CH334_Ctrl(uint8_t sta)
{
    if (sta) {
        GPIO_WriteBit(GPIOA, GPIO_Pin_1, SET);
    }
    else {
        GPIO_WriteBit(GPIOA, GPIO_Pin_1, RESET);
    }
}

void I2C1_EV_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void I2C1_EV_IRQHandler(void)
{
    /* 地址匹配:一次事务开始 */
    if (I2C_GetITStatus(I2C1, I2C_IT_ADDR) != RESET)
    {
        volatile uint16_t sr1 = I2C1->STAR1;
        volatile uint16_t sr2 = I2C1->STAR2;
        (void)sr1;
        (void)sr2;

        if (I2C_GetFlagStatus(I2C1, I2C_FLAG_TRA) != RESET) {
            /* 主机要读,从机发数据 */
            uint8_t buf[1];
            flash_get(eeprom_addr, buf);
            I2C1->DATAR = buf[0];

            eeprom_addr++;
            eeprom_addr &=0xff;
        } else {
            /* 主机写数据(命令 + 数据) */
            addr_received = 0;
        }
        return;
    }

    /* 收到数据字节(主机写从机) */
    if (I2C_GetITStatus(I2C1, I2C_IT_RXNE) != RESET)
    {
        uint8_t data = (uint8_t)I2C1->DATAR;

        if (!addr_received) {
            /* 第一个字节:SBS 命令码 */
            eeprom_addr = data;
            page_base = eeprom_addr & 0xf8;
            page_offset = eeprom_addr & 0x07;
            addr_received = 1;
        } else {
            /* 后续字节:命令对应的数据(Write Word 等) */
//            eeprom[eeprom_addr] = data;           //后续process处理
            eeprom_addr++;

            page_offset++;

            page_offset &= 0x07;

            eeprom_addr = page_base | page_offset;
        }
        return;
    }

    /* STOP:一次 SMBus 事务结束,可以在这里触发"写后动作" */
    if (I2C_GetITStatus(I2C1, I2C_IT_STOPF) != RESET)
    {
        volatile uint16_t tmp;

        tmp = I2C1->STAR1;
        (void)tmp;

        I2C1->CTLR1 |= I2C_CTLR1_PE;   /* 按芯片手册要求写 CR1 清 STOPF */

        addr_received = 0;

        eeprom_busy = 1;

        // 如果有些命令需要在写完后执行动作,在这里做,但是要保证尽快完成.
        return;
    }

    /* 发数据:TXE 置位(发送缓冲空) */
    if (I2C_GetITStatus(I2C1, I2C_IT_TXE) != RESET)
    {
//        I2C1->DATAR = eeprom[eeprom_addr];
        uint8_t buf[1];
        flash_get(eeprom_addr, buf);

        I2C1->DATAR = buf[0];

        eeprom_addr++;

        eeprom_addr &= 0xff;

        return;
    }

    /* BTF / SB 之类事件,这里只做吃掉以防万一 */
    if (I2C_GetITStatus(I2C1, I2C_IT_BTF) != RESET)
    {
        (void)I2C1->STAR1;
        return;
    }

    if (I2C_GetITStatus(I2C1, I2C_IT_SB) != RESET)
    {
        (void)I2C1->STAR1;
        return;
    }
}

void I2C1_ER_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void I2C1_ER_IRQHandler(void)
{
    if (I2C_GetITStatus(I2C1, I2C_IT_AF) ) {
        /* ACK Failure:主机提前 NACK,一般是读够了数据 */

        I2C_ClearITPendingBit(I2C1, I2C_IT_AF);
    } else {
        /* 其它错误不应该发生!否则就是硬件线问题. */
        I2C_ClearITPendingBit(I2C1, I2C_IT_BERR);
    }
}

uint8_t Process()
{
    if (eeprom_busy) {
        eeprom_busy = 0;
        ch334_time = tick;

        //这里写flash
    }

    if (tick - ch334_time > 200) {    //超过200ms没有操作
        return 0;
    }

    return 1;
}
