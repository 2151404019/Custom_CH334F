/*
 * LED_MUX.c
 *
 *  Created on: 2026?7?19?
 *      Author: MXQ
 */

#include "ch32v00x.h"
#include "LED_MUX.h"
#include "debug.h"

/******************************
 * ROW
 *****************************/
#define ROW1_GPIO      GPIO_Pin_7
#define ROW1_PORT      GPIOC

#define ROW2_GPIO      GPIO_Pin_6
#define ROW2_PORT      GPIOC

#define ROW3_GPIO      GPIO_Pin_5
#define ROW3_PORT      GPIOC

#define ROW4_GPIO      GPIO_Pin_4
#define ROW4_PORT      GPIOC

#define ROW5_GPIO      GPIO_Pin_6
#define ROW5_PORT      GPIOD

#define ROW6_GPIO      GPIO_Pin_5
#define ROW6_PORT      GPIOD

#define ROW7_GPIO      GPIO_Pin_0
#define ROW7_PORT      GPIOC

#define ROW8_GPIO      GPIO_Pin_3
#define ROW8_PORT      GPIOC

/******************************
 * COL
 *****************************/
#define COL1_GPIO      GPIO_Pin_2
#define COL1_PORT      GPIOD

#define COL2_GPIO      GPIO_Pin_3
#define COL2_PORT      GPIOD

#define COL3_GPIO      GPIO_Pin_4
#define COL3_PORT      GPIOD

#define led_low(GPIOx, GPIO_Pin)         GPIO_WriteBit(GPIOx, GPIO_Pin, Bit_RESET)
#define led_high(GPIOx, GPIO_Pin)        GPIO_WriteBit(GPIOx, GPIO_Pin, Bit_SET)
uint8_t bit = 0;
#define led_toggle(GPIOx, GPIO_Pin)      GPIO_WriteBit(GPIOx, GPIO_Pin, bit ? Bit_RESET : Bit_SET)

#define LED_BLINK_TIME  300 //ms

typedef enum
{
    IO_ROW1 = 0,
    IO_ROW2,
    IO_ROW3,
    IO_ROW4,
    IO_ROW5,
    IO_ROW6,
    IO_ROW7,
    IO_ROW8,

    ROW_IO_MAX
} ROW_IO_INDEX;

typedef enum
{
    IO_COL1 = 0,
    IO_COL2,
    IO_COL3,

    COL_IO_MAX
} COL_IO_INDEX;

typedef struct{
    GPIO_TypeDef *port;
    uint16_t gpio;
}IO_Table_t;

//row gpio table
const static IO_Table_t row_io_table[] =
{
    {ROW1_PORT, ROW1_GPIO},   // ROW1
    {ROW2_PORT, ROW2_GPIO},   // ROW2
    {ROW3_PORT, ROW3_GPIO},   // ROW3
    {ROW4_PORT, ROW4_GPIO},   // ROW4
    {ROW5_PORT, ROW5_GPIO},   // ROW5
    {ROW6_PORT, ROW6_GPIO},   // ROW6
    {ROW7_PORT, ROW7_GPIO},   // ROW7
    {ROW8_PORT, ROW8_GPIO},   // ROW8
};

//col gpio table
const static IO_Table_t col_io_table[] =
{
    {COL1_PORT, COL1_GPIO},   // COL1
    {COL2_PORT, COL2_GPIO},   // COL2
    {COL3_PORT, COL3_GPIO},   // COL3
};

//映射表
LED_t LedBuf[8][3] = {
        {{off, FUNC_5v_vol},        {off, FUNC_5v_sta},         {off, FUNC_PPS_1Bit}},
        {{off, FUNC_9v_vol},        {off, FUNC_9v_sta},         {off, FUNC_PPS_2Bit}},
        {{off, FUNC_12v_vol},       {off, FUNC_12v_sta},        {off, FUNC_PPS_3Bit}},
        {{off, FUNC_15v_vol},       {off, FUNC_15v_sta},        {off, FUNC_PPS_4Bit}},
        {{off, FUNC_20v_vol},       {off, FUNC_20v_sta},        {off, FUNC_PPS_5Bit}},
        {{off, FUNC_PPS},           {off, FUNC_PPS_sta},        {off, FUNC_PPS_6Bit}},
        {{off, null},               {off, null},                {off, FUNC_PPS_7Bit}},
        {{off, null},               {off, null},                {off, FUNC_PPS_8Bit}},
};

extern volatile uint32_t tick;
static uint32_t t;
uint8_t current_row = 0;

void LED_IO_Init()
{
    GPIO_InitTypeDef gpio = {0};

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD, ENABLE);

    for (int var = 0; var < sizeof(row_io_table) / sizeof(row_io_table[0]); ++var) {
        gpio.GPIO_Pin = row_io_table[var].gpio;
        gpio.GPIO_Mode = GPIO_Mode_Out_PP;
        gpio.GPIO_Speed = GPIO_Speed_30MHz;
        GPIO_Init(row_io_table[var].port, &gpio);

        GPIO_WriteBit(row_io_table[var].port, row_io_table[var].gpio, Bit_RESET);
    }

    for (int var = 0; var < sizeof(col_io_table) / sizeof(col_io_table[0]); ++var) {
            gpio.GPIO_Pin = col_io_table[var].gpio;
            gpio.GPIO_Mode = GPIO_Mode_Out_PP;
            gpio.GPIO_Speed = GPIO_Speed_30MHz;
            GPIO_Init(col_io_table[var].port, &gpio);

            GPIO_WriteBit(col_io_table[var].port, col_io_table[var].gpio, Bit_RESET);
        }
}

//row 关
static void Row_AllOff()
{
    for (int i = 0; i < ROW_IO_MAX; ++i) {
        led_low(row_io_table[i].port, row_io_table[i].gpio);
    }
}

//col 关
static void Col_AllOff()
{
    for (int i = 0; i < COL_IO_MAX; ++i) {
        led_high(col_io_table[i].port, col_io_table[i].gpio);
    }
}

/*
 * 批量修改led属性下的led状态
 */
void LED_Buf_Change(led_function func, led_sta state)
{
    volatile uint8_t row, col;
    for (row = 0; row < 8; row++)
    {
        for (col = 0; col < 3; col++)
        {
            if (LedBuf[row][col].function & func)
            {
                LedBuf[row][col].sta = state;
            }
        }
    }
}

//全亮
void LED_Full()
{
    for (int i = 0; i < ROW_IO_MAX; ++i) {
        led_high(row_io_table[i].port, row_io_table[i].gpio);
    }
    for (int i = 0; i < COL_IO_MAX; ++i) {
        led_low(col_io_table[i].port, col_io_table[i].gpio);
    }
}

void LED_PPS_Vol_Set(uint8_t vol)
{
    volatile int i;
    for (i = 0; i < 8; ++i) {
        if (vol & (1 << i)) {
            LED_Buf_Change((FUNC_PPS_1Bit << i), on);
        }
    }
}

//mian while 中快速调用
void LED_MUX_Process()
{
    Row_AllOff();
    Col_AllOff();

    if ((tick - t > LED_BLINK_TIME))
    {
        bit ^= 1;
        t = tick;
    }

    /* 设置当前ROW对应的COL */
    for(uint8_t col = 0; col < COL_IO_MAX; col++)
    {
        if(LedBuf[current_row][col].sta == on)
        {
            led_low(col_io_table[col].port, col_io_table[col].gpio);
//            printf("col is %d \n", col);
//            printf("row is %d \n", col);
        }
        else if (LedBuf[current_row][col].sta == blink) {
//            if (tick - t > LED_BLINK_TIME) {
//                led_toggle(col_io_table[col].port, col_io_table[col].gpio);
//                t = tick;
//            }
            led_toggle(col_io_table[col].port, col_io_table[col].gpio);
        }
    }

    /* 打开当前ROW */
    led_high(row_io_table[current_row].port, row_io_table[current_row].gpio);

    /* 下一行 */
    current_row++;

    if(current_row >= ROW_IO_MAX)
        current_row = 0;
}




