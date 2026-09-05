/*
 * CH224Q.c
 *
 *  Created on: 2026?7?13?
 *      Author: MXQ
 */

#include "ch32v00x.h"

#include "I2C.h"
#include "CH224Q.h"

#include "string.h"

#define CH224Q_ADDR             (0x22 << 1)

#define STATE_REG               0x09
#define VOL_CONTRL_REG          0x0a
#define CURR_DATA_REG           0x50
#define H_AVS_REG               0x51
#define L_AVS_REG               0x52
#define PPS_REG                 0x53

#define FIRST_PD_DATA_REG       0x60
#define LAST_PD_DATA_REG        0x8f

#define ch224_delay(ms)         Delay_Ms(ms)

extern volatile uint16_t PD_Vol;

uint8_t Power_dec = 0;

_CH224Q_typedef CH224Q;

_FixedSupply Rx_FixedSupply;
_PPSupply Rx_PPSupply;
_AVSupply Rx_AVSupply;

_Message_Header Rx_Header;
_Message_ExtHeader Rx_Ext_Header;

//PD原始数据
uint16_t PD_Msg[10][4];

//led live table
//led_function led_alive[6][2];
LED_Alive_t led_alive[6];

static uint8_t CH224Q_Write(uint8_t reg, uint8_t data)
{
    return I2C_WriteData8Bit(CH224Q_ADDR, reg, data);
}

static uint8_t CH224Q_Read(uint8_t reg)
{
    uint8_t data;

    I2C_ReadData8Bit(CH224Q_ADDR, reg, (uint8_t *)&data);

    return data;
}

uint8_t CH224Q_Get_State()
{
    return CH224Q_Read(STATE_REG);
}

void CH224Q_Request_Vol(Vol_State Vol)
{
    CH224Q_Write(VOL_CONTRL_REG, Vol);
}

//PD数据包解析
void SourceCap_Analyes()
{
    memcpy(&Rx_Header.Data, &CH224Q.PD_data[0], 2);
    if(Rx_Header.Message_Header.Ext == 1)
    {
        memcpy(&Rx_Ext_Header.Data, &CH224Q.PD_data[2], 2);
        for(uint8_t i = 0; i < Rx_Ext_Header.Message_Header.DataSize/ 4; i++)
        {
            memcpy(&Rx_FixedSupply.Data, &CH224Q.PD_data[4*(i + 1)], 4);
            if(Rx_FixedSupply.bit.FixedSupply == 3)
            {
                memcpy(&Rx_PPSupply.Data, &CH224Q.PD_data, 4);
                if(Rx_PPSupply.bit.PPS == 1)
                {
                    memcpy(&Rx_AVSupply.Data, &Rx_PPSupply.Data, 4);
                    PD_Msg[i][0] = 3;
                    PD_Msg[i][1] = Rx_AVSupply.bit.MinVoltage * 100;
                    PD_Msg[i][2] = Rx_AVSupply.bit.MaxVoltage * 100;
                    PD_Msg[i][3] = Rx_AVSupply.bit.PDP;
                    continue;
                }
                PD_Msg[i][0] = 2;
                PD_Msg[i][1] = Rx_PPSupply.bit.MinVoltage * 100;
                PD_Msg[i][2] = Rx_PPSupply.bit.MaxVoltage * 100;
                PD_Msg[i][3] = Rx_PPSupply.bit.MaxCurrent * 50;
                continue;
            }
            PD_Msg[i][0] = 1;
            PD_Msg[i][1] = Rx_FixedSupply.bit.Voltage * 50;
            PD_Msg[i][2] = Rx_FixedSupply.bit.MaxCurrent * 10;
            PD_Msg[i][3] = 0;
            if(PD_Msg[i][1] == 0 && PD_Msg[i][2] == 0 && PD_Msg[i][3] == 0)
                PD_Msg[i][0] = 4;
        }
    }
    else
    {
        for(uint8_t i = 0; i < Rx_Header.Message_Header.NumDo; i++)
        {
            memcpy(&Rx_FixedSupply.Data, &CH224Q.PD_data[4 * i + 2], 4);
            if(Rx_FixedSupply.bit.FixedSupply == 3)
            {
                memcpy(&Rx_PPSupply.Data, &Rx_FixedSupply.Data, 4);
                PD_Msg[i][0] = 2;
                PD_Msg[i][1] = Rx_PPSupply.bit.MinVoltage * 100;
                PD_Msg[i][2] = Rx_PPSupply.bit.MaxVoltage * 100;
                PD_Msg[i][3] = Rx_PPSupply.bit.MaxCurrent * 50;
                continue;
            }
            PD_Msg[i][0] = 1;
            PD_Msg[i][1] = Rx_FixedSupply.bit.Voltage * 50;
            PD_Msg[i][2] = Rx_FixedSupply.bit.MaxCurrent * 10;
            PD_Msg[i][3] = 0;
        }
    }
}

//获取当前握手协议
void CH224Q_Get_Status()
{
    CH224Q.I2C_status.Data = CH224Q_Read(STATE_REG);
}

//PD 获取最大电流
uint8_t CH224Q_Get_MaxCurrent()
{
    CH224Q_Get_Status();

    if(CH224Q.I2C_status.PD == 1)
    {
        CH224Q.current_status = CH224Q_Read(CURR_DATA_REG);
        return CH224Q.current_status;
    }
    else
        return 0;
}

//清空PD数据buf
static void CH224Q_Clear()
{
    for(uint8_t i = 0; i < 48; i++)
    {
        CH224Q.PD_data[i] = 0;
    }
}

//在PD/ERP握手下获取SRCCAP数据
void CH224Q_Get_PDData()
{
    uint8_t reg = FIRST_PD_DATA_REG;
    uint8_t Count = 0;

    CH224Q_Get_State();

    CH224Q_Clear();

    if(CH224Q.I2C_status.PD == 1 )
    {
        for(uint8_t i = FIRST_PD_DATA_REG; i <= LAST_PD_DATA_REG; i++)
        {
            CH224Q.PD_data[Count] = CH224Q_Read(reg);
            reg++;
            Count++;
        }

        SourceCap_Analyes();

        printf("\r\n");
        printf("/-----------------------------------/\r\n");
        printf("             Power Supply            \r\n");
        printf("/-----------------------------------/\r\n");
        for(uint8_t i = 0; i < 10; i++)             //先获取全部PD种类数量
        {
            switch(PD_Msg[i][0])
            {
            case 0:break;
            case 1: printf("[%02d] Fixed    %.2fV   %.2fA          \r\n",i+1,(float)PD_Msg[i][1]/1000,(float)PD_Msg[i][2]/1000);break;
            case 2: printf("[%02d]  PPS     %.2fV - %.2fV   %.2fA  \r\n",i+1,(float)PD_Msg[i][1]/1000,(float)PD_Msg[i][2]/1000,(float)PD_Msg[i][3]/1000);break;
            case 3: printf("[%02d]  AVS     %.2fV - %.2fV   %-3dW  \r\n",i+1,(float)PD_Msg[i][1]/1000,(float)PD_Msg[i][2]/1000,PD_Msg[i][3]);break;
            case 4: printf("[%02d]  ----------Rev----------        \r\n",i+1);break;
            }
        }
    }
}

//PD 固定电压请求 ====> Vol_State===> 5V/9V/12V/15V/20V
void CH224Q_Fixed_Request_Vol(Vol_State Vol)
{
    CH224Q_Write(VOL_CONTRL_REG, Vol);
}

//PD PPS电压请求
void CH224Q_PPS_Request_Vol(uint8_t Vol)
{
    CH224Q_Write(PPS_REG, Vol);

    ch224_delay(2);

    if (CH224Q_Read(VOL_CONTRL_REG) != 6) {
        CH224Q_Write(VOL_CONTRL_REG, 6);
    }
}

//EPR AVS电压请求
void CH224Q_AVS_Request_Vol(float Vol)
{
    uint16_t dat = 0;
    uint8_t AVS_DataH, AVS_DataL;
    AVS_DataH = AVS_DataL = 0;

    dat = (uint16_t)(Vol * 1000.0)/100;
    AVS_DataH = (uint8_t)(((dat&0xff00)>>8)|(0x80));
    AVS_DataL = (uint8_t)(dat&0x00ff);

    CH224Q_Write(L_AVS_REG, AVS_DataL);
//    vTaskDelay(pdMS_TO_TICKS(1));
    CH224Q_Write(H_AVS_REG, AVS_DataH);
//    vTaskDelay(pdMS_TO_TICKS(1));

    if (CH224Q_Read(VOL_CONTRL_REG) != 7) {
        CH224Q_Write(VOL_CONTRL_REG, 7);
    }
}

void CH224Q_Pattern_Request(uint8_t pattern)
{
    CH224Q_Write(VOL_CONTRL_REG, pattern);
}

void CH224Q_PPS_Request(uint8_t Vol)
{
    CH224Q_Write(PPS_REG, Vol);
}

led_function alive_sta()
{
    led_function func_sta = null;

    if (PD_Vol > 4000 && PD_Vol < 6000) {
        func_sta = FUNC_5v_sta;
    }
    else if (PD_Vol > 8000 && PD_Vol < 10000) {
        func_sta = FUNC_9v_sta;
    }
    else if (PD_Vol > 11000 && PD_Vol < 13000) {
        func_sta = FUNC_12v_sta;
    }
    else if (PD_Vol > 14000 && PD_Vol < 16000) {
        func_sta = FUNC_15v_sta;
    }
    else if (PD_Vol > 19000 && PD_Vol < 21000) {
        func_sta = FUNC_20v_sta;
    }

    return func_sta;
}

led_function alive_vol()
{
    led_function func_vol = null;

    if (PD_Vol > 4000 && PD_Vol < 6000) {
        func_vol = FUNC_5v_vol;
    }
    else if (PD_Vol > 8000 && PD_Vol < 10000) {
        func_vol = FUNC_9v_vol;
    }
    else if (PD_Vol > 11000 && PD_Vol < 13000) {
        func_vol = FUNC_12v_vol;
    }
    else if (PD_Vol > 14000 && PD_Vol < 16000) {
        func_vol = FUNC_15v_vol;
    }
    else if (PD_Vol > 19000 && PD_Vol < 21000) {
        func_vol = FUNC_20v_vol;
    }

    return func_vol;
}

uint8_t CH224Q_Init()
{
    uint8_t Count = 0;
    uint8_t reg = FIRST_PD_DATA_REG;

    if (PD_Vol > 3000) {        //power port voltage >3V
        Power_dec = 1;          //有线缆供电
    }
    else {
        return 0;
    }

    CH224Q_Get_Status();

    CH224Q_Clear();

    CH224Q.PD_Vol.Data = 0;             //所有位清空
    CH224Q.kind = 0;
    CH224Q.PPS_Vol.PPS_max = 0;         //最大设0V
    CH224Q.PPS_Vol.PPS_min = 20000;     //最小设20V
    CH224Q.PPS_ctrl = 100;            //10v

    if (CH224Q.I2C_status.PD) {
        for(uint8_t i = FIRST_PD_DATA_REG; i <= LAST_PD_DATA_REG; i++)
        {
            CH224Q.PD_data[Count] = CH224Q_Read(reg);
            reg++;
            Count++;
        }
        SourceCap_Analyes();
        for(uint8_t i = 0; i < 10; i++)             //先获取全部PD种类数量
        {
            switch(PD_Msg[i][0])
            {
                case 0:
                    break;
                case 1:
                    switch (PD_Msg[i][1]) {
                        case 5000:
                            CH224Q.PD_Vol.VOL_5 = 1;
                            break;
                        case 9000:
                            CH224Q.PD_Vol.VOL_9 = 1;
                            break;
                        case 12000:
                            CH224Q.PD_Vol.VOL_12 = 1;
                            break;
                        case 15000:
                            CH224Q.PD_Vol.VOL_15 = 1;
                            break;
                        case 20000:
                            CH224Q.PD_Vol.VOL_20 = 1;
                            break;
                        default:
                            break;
                    }
                    break;
                case 2:
                    CH224Q.PD_Vol.PPS = 1;
                    if (PD_Msg[i][1] < CH224Q.PPS_Vol.PPS_min) {
                        CH224Q.PPS_Vol.PPS_min = PD_Msg[i][1];
                    }
                    if (PD_Msg[i][2] > CH224Q.PPS_Vol.PPS_max) {
                        CH224Q.PPS_Vol.PPS_max = PD_Msg[i][2];
                    }
                    break;
                case 3:
                    break;
                case 4:
                    break;
            }
        }

        if (CH224Q.PPS_Vol.PPS_min < 4000) {
            CH224Q.PPS_Vol.PPS_min = 4000;
        }

        if ((CH224Q.PPS_ctrl * 100) < CH224Q.PPS_Vol.PPS_min) {
            CH224Q.PPS_ctrl = CH224Q.PPS_Vol.PPS_min/ 100;
        }
        if ((CH224Q.PPS_ctrl * 100) > CH224Q.PPS_Vol.PPS_max) {
            CH224Q.PPS_ctrl = CH224Q.PPS_Vol.PPS_max/ 100;
        }
    }
    if (CH224Q.I2C_status.QC2 || CH224Q.I2C_status.QC3 ) {
        CH224Q.PD_Vol.VOL_5 = 1;
        CH224Q.PD_Vol.VOL_9 = 1;
        CH224Q.PD_Vol.VOL_15 = 1;
        CH224Q.PD_Vol.VOL_12 = 1;
        CH224Q.PD_Vol.VOL_20 = 1;

        CH224Q.PPS_ctrl = 0;
    }

    uint8_t num = 0;
    for (int i = 0; i < 8; ++i) {
        if (CH224Q.PD_Vol.Data & (1 << i)) {
//            led_alive[i][0] = 1 << (2 * i);
//            led_alive[i][1] = 1 << (2 * i + 1);
//
//            LED_Buf_Change(led_alive[i][0], on);
            led_alive[num].sel = 1 << (2 * i);
            led_alive[num].set = 1 << (2 * i + 1);
            led_alive[num].vol_kind = i;

            CH224Q.kind++;

            LED_Buf_Change(led_alive[num].sel, on);

            num++;
        }
    }

    LED_Buf_Change(alive_vol(), on);
    LED_Buf_Change(alive_sta(), on);

    return 1;
}
