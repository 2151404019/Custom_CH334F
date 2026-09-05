/*
 * Protocol.c
 *
 *  Created on: 2026?8?8?
 *      Author: MXQ
 */

#include <stdint.h>
#include <string.h>

#include "flash.h"
#include "UART.h"
#include "debug.h"

#define FRAME_HEAD1             0x4d
#define FRAME_HEAD2             0x58
#define FRAME_HEAD3             0x51

#define PROTOCOL_ERROR          0x20
#define PROTOCOL_INFO           0x21

#define FRAME_HEAD1_ERROR       0x4d
#define FRAME_HEAD2_ERROR       0x58
#define FRAME_HEAD3_ERROR       0x51
#define PROTOCOL_CMD_ERROR      0x33
#define FRAME_CRC_ERROR         0x61

#define CMD_DONE                0xFF

typedef enum{
    frame_head1,
    frame_head2,
    frame_head3,

    protocol_len,

    protocol_cmd,

    protocol_data,

    frame_crc,

    error_none,                  //无错误

    frame_head1_error,
    frame_head2_error,
    frame_head3_error,

    protocol_cmd_error,

    frame_crc_error

}PROTOCOL_STATE;

typedef enum{
    CH334F_Cmd,
    ops_reset,

    cmd_end
}Response_cmd;

uint8_t rx_pos;

PROTOCOL_STATE state;
PROTOCOL_STATE sta = error_none;

Response_cmd cmd;
uint8_t len;
uint8_t data[UART_RX_BUF_SIZE/2];
uint8_t Count;

uint8_t crc_buf[UART_RX_BUF_SIZE/2 + 1];

extern uint8_t uart_rx_buf[UART_RX_BUF_SIZE];

uint8_t flash_wr_ops = 0;

//sta --> PROTOCOL_INFO / PROTOCOL_ERROR
static void UART_Response(uint8_t status, uint8_t dat)
{
    uint8_t buf[2];

    buf[0] = status;
    buf[1] = dat;

    UART_DMA_Send(buf, 2);
}

static uint8_t crc8(uint8_t *buf, uint8_t length)
{
    uint8_t crc = 0;

    while(length--)
    {
        crc ^= *buf++;

        for (uint8_t i = 0; i < 8; ++i) {
            if (crc & 0x80) {
                crc = (crc << 1)^0x07;
            }
            else {
                crc <<= 1;
            }
        }
    }

    return crc;
}

static void command_execute(uint8_t cmd, uint8_t *buff, uint8_t length)
{
    uint8_t e = 0;

    switch (cmd) {
        case CH334F_Cmd:
            for (uint8_t i = 0; i < length; ++i) {
                if (flash_set(flash_wr_ops, buff[i]) == -1) {
                    e = 1;
                }

                flash_wr_ops++;
            }
            break;
        case ops_reset:
            flash_wr_ops = 0;
            break;
        default:
            break;
    }

    if (e) {
        printf("flash_set fail\r\n");
    }
    UART_Response(PROTOCOL_INFO, CMD_DONE);
}

static void Protocol_Response(uint8_t sta)
{
    switch (sta) {
        case frame_head1_error:
            UART_Response(PROTOCOL_ERROR, FRAME_HEAD1_ERROR);
            break;
        case frame_head2_error:
            UART_Response(PROTOCOL_ERROR, FRAME_HEAD2_ERROR);
            break;
        case frame_head3_error:
            UART_Response(PROTOCOL_ERROR, FRAME_HEAD3_ERROR);
            break;
        case protocol_cmd_error:
            UART_Response(PROTOCOL_ERROR, PROTOCOL_CMD_ERROR);
            break;
        case frame_crc_error:
            UART_Response(PROTOCOL_ERROR, FRAME_CRC_ERROR);
            break;
        default:
            break;
    }
}

static uint8_t protocol_input(uint8_t dat)
{
    sta = error_none;                //存错误状态

    switch (state) {
        case frame_head1:
            if (dat == FRAME_HEAD1) {
                state = frame_head2;
            }
            else {
                sta = frame_head1_error;
            }
            break;
        case frame_head2:
            if (dat == FRAME_HEAD2) {
                state = frame_head3;
            }
            else {
                state = frame_head1;
                sta = frame_head2_error;
            }
            break;
        case frame_head3:
            if (dat == FRAME_HEAD3) {
                state = protocol_len;
            }
            else {
                state = frame_head1;
                sta = frame_head3_error;
            }
            break;
        case protocol_len:
            len = dat;
            Count = 0;
            state = protocol_cmd;
            break;
        case protocol_cmd:
            cmd = dat;

            if (cmd >= cmd_end) {
                sta = protocol_cmd_error;       //预留，无效cmd，也能做空cmd，不算错协议
            }

            if (len > 1) {
                state = protocol_data;
            }
            else {
                state = frame_crc;          //只有cmd数据
            }
            break;
        case protocol_data:
            data[Count++] = dat;

            if (Count >= len - 1) {
                state = frame_crc;
            }
            break;
        case frame_crc:
            crc_buf[0] = cmd;
            memcpy(&crc_buf[1], data, len - 1);

            if (crc8(crc_buf, len) == dat) {
                command_execute(cmd, data, len - 1);
            }
            else {
                sta = frame_crc_error;
            }

            state = frame_head1;        //准备开始新的一轮
            break;
        default:
            break;
    }

    if (sta > error_none) {             //响应数据错误
//    if (sta==protocol_cmd_error ||
//            sta==frame_crc_error) {
        Protocol_Response(sta);

        return 1;
    }

    return 0;
}

void Decode_Process()
{
    uint8_t pos;

    pos = UART_DMA_GetRxLen();

    if (pos == rx_pos) {
        return;
    }

    if (pos > rx_pos) {
        for (uint8_t i = rx_pos; i < pos; ++i) {
            if (protocol_input(uart_rx_buf[i])) {
//                rx_pos = pos;
//                break;                              //协议出错直接跳出
            }
        }
    }
    else {
        for (uint8_t i = rx_pos; i < UART_RX_BUF_SIZE; ++i) {
            if (protocol_input(uart_rx_buf[i])) {
//                rx_pos = pos;
//                break;                              //协议出错直接跳出
            }
        }

        for (uint8_t i = 0; i < pos; ++i) {
            if (protocol_input(uart_rx_buf[i])) {
//                rx_pos = pos;
//                break;                              //协议出错直接跳出
            }
        }
    }

    rx_pos = pos;
}
