/*
 * CH224Q.h
 *
 *  Created on: 2026?7?13?
 *      Author: MXQ
 */

#ifndef BSP_INC_CH224Q_H_
#define BSP_INC_CH224Q_H_

#include "LED_MUX.h"

//…Í«ÎµÁ—π
typedef enum
{
//    Vol_5   = 0x01,
//    Vol_9   = 0x02,
//    Vol_12  = 0x04,
//    Vol_15  = 0x08,
//    Vol_20  = 0x10,
//    Vol_28  = 0x20,
//    PPS     = 0x40,
//    AVS     = 0x80

    Vol_5   = 0,
    Vol_9   ,
    Vol_12  ,
    Vol_15  ,
    Vol_20  ,
    Vol_28  ,
    PPS     ,
    AVS     ,
}Vol_State;

//CH224A/Q PD_status_Reg
typedef union{
    struct{
        uint8_t BC:1;
        uint8_t QC2:1;
        uint8_t QC3:1;
        uint8_t PD:1;
        uint8_t EPR:1;
        uint8_t Rev:3;
    };
    uint8_t Data;
}_I2C_status_typdef;

typedef union
{
    struct{
        uint8_t VOL_5:1;
        uint8_t VOL_9:1;
        uint8_t VOL_12:1;
        uint8_t VOL_15:1;
        uint8_t VOL_20:1;
        uint8_t VOL_28:1;
        uint8_t PPS:1;
        uint8_t AVS:1;
    };
    uint8_t Data;
}PD_Vol_Kind_t;

typedef struct
{
    uint16_t PPS_min;
    uint16_t PPS_max;
}PD_PPS_Vol_t;

typedef struct{
    led_function sel;
    led_function set;
    Vol_State vol_kind;
}LED_Alive_t;

//CH224Q REG
typedef struct{
    _I2C_status_typdef I2C_status;
    PD_Vol_Kind_t PD_Vol;
    PD_PPS_Vol_t PPS_Vol;
    uint8_t vol_status;
    uint8_t current_status;
    uint8_t AVS_H;
    uint8_t AVS_L;
    uint8_t PPS_ctrl;
    uint8_t kind;
    uint8_t PD_data[48];
}_CH224Q_typedef;

//Head
typedef union{
    struct{
        uint8_t MsgType:5;
        uint8_t PDRole:1;
        uint8_t SpecRev:2;
        uint8_t PRRole:1;
        uint8_t MsgID:3;
        uint8_t NumDo:3;
        uint8_t Ext:1;
    }Message_Header;
    uint16_t Data;
}_Message_Header;

//Ext_Head
typedef union{
    struct{
        uint16_t DataSize:9;
        uint16_t Rev:1;
        uint16_t RequestChunk:1;
        uint16_t ChunkNumber:4;
        uint16_t Chunked:1;
    }Message_Header;
    uint16_t Data;
}_Message_ExtHeader;

//Fixed Supply PDO - Source
typedef union{
    struct{
        uint32_t MaxCurrent:10;
        uint32_t Voltage:10;
        uint32_t PeakCurrent:2;
        uint32_t Reserved:1;
        uint32_t EPRModeCap:1;
        uint32_t Unchunked:1;
        uint32_t DualRoleData:1;
        uint32_t USBComCap:1;
        uint32_t UnconstrainedPWR:1;
        uint32_t USBSupendSupported:1;
        uint32_t DualRolePWR:1;
        uint32_t FixedSupply:2;
    }bit;
    uint32_t Data;
}_FixedSupply;

//SPR Programmable Power Supply APDO ®C Source
typedef union{
    struct{
        uint32_t MaxCurrent:7;
        uint32_t Reserved1:1;
        uint32_t MinVoltage:8;
        uint32_t Reserved2:1;
        uint32_t MaxVoltage:8;
        uint32_t Reserved3:2;
        uint32_t PPSWELimited:1;
        uint32_t PPS:2;
        uint32_t APDO:2;
    }bit;
    uint32_t Data;
}_PPSupply;

//°∞EPR Adjustable Voltage Supply APDO ®C Source
typedef union{
    struct{
        uint32_t PDP:8;
        uint32_t MinVoltage:8;
        uint32_t Reserved2:1;
        uint32_t MaxVoltage:9;
        uint32_t PeakCurrent:2;
        uint32_t EPRAVS:2;
        uint32_t APDO:2;
    }bit;
    uint32_t Data;
}_AVSupply;

void SourceCap_Analyes();
void CH224Q_Get_Status();
uint8_t CH224Q_Get_MaxCurrent();
void CH224Q_Get_PDData();
void CH224Q_Fixed_Request_Vol(Vol_State Vol);
void CH224Q_PPS_Request_Vol(uint8_t Vol);
void CH224Q_AVS_Request_Vol(float Vol);
void CH224Q_Request_Vol(Vol_State Vol);

uint8_t CH224Q_Init();
void CH224Q_Pattern_Request(uint8_t pattern);
void CH224Q_PPS_Request(uint8_t Vol);

#endif /* BSP_INC_CH224Q_H_ */
