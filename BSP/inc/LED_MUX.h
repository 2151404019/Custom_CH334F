/*
 * LED_MUX.h
 *
 *  Created on: 2026?7?19?
 *      Author: MXQ
 */

#ifndef BSP_INC_LED_MUX_H_
#define BSP_INC_LED_MUX_H_

//led ×´Ì¬
typedef enum{
    off = 0,
    on,
    blink
}led_sta;

//led ¹¦ÄÜ
typedef enum{
    null          = 0x00,
    FUNC_5v_vol   = 0x01,
    FUNC_5v_sta   = 0x02,
    FUNC_9v_vol   = 0x04,
    FUNC_9v_sta   = 0x08,
    FUNC_12v_vol  = 0x10,
    FUNC_12v_sta  = 0x20,
    FUNC_15v_vol  = 0x40,
    FUNC_15v_sta  = 0x80,
    FUNC_20v_vol  = 0x100,
    FUNC_20v_sta  = 0x200,
#if 1
    FUNC_28v_vol  = 0x400,
    FUNC_28v_sta  = 0x800,
    FUNC_PPS      = 0x1000,
    FUNC_PPS_sta  = 0x2000,

    FUNC_PPS_1Bit = 0x4000,
    FUNC_PPS_2Bit = 0x8000,
    FUNC_PPS_3Bit = 0x10000,
    FUNC_PPS_4Bit = 0x20000,
    FUNC_PPS_5Bit = 0x40000,
    FUNC_PPS_6Bit = 0x80000,
    FUNC_PPS_7Bit = 0x100000,
    FUNC_PPS_8Bit = 0x200000
#else
    FUNC_PPS      = 0x000400,
    FUNC_PPS_sta  = 0x000800,

    FUNC_PPS_1Bit = 0x001000,
    FUNC_PPS_2Bit = 0x002000,
    FUNC_PPS_3Bit = 0x004000,
    FUNC_PPS_4Bit = 0x008000,
    FUNC_PPS_5Bit = 0x010000,
    FUNC_PPS_6Bit = 0x020000,
    FUNC_PPS_7Bit = 0x040000,
    FUNC_PPS_8Bit = 0x080000,
#endif
}led_function;

typedef struct{
    led_sta sta;
    led_function function;
}LED_t;

void LED_IO_Init();
void LED_Full();
void LED_Buf_Change(led_function func, led_sta state);
void LED_PPS_Vol_Set(uint8_t vol);
void LED_MUX_Process();

#endif /* BSP_INC_LED_MUX_H_ */
