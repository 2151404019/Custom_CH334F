#include "debug.h"
#include "btn.h"
#include "CH224Q.h"
#include "CH334.h"
#include "TIM.h"
#include "I2C.h"
#include "LED_MUX.h"
#include "ADC.h"
#include "flash.h"
#include "UART.h"
#include "Protocol.h"

#include "lwbtn/lwbtn.h"

extern _CH224Q_typedef CH224Q;
extern LED_Alive_t led_alive[6];

volatile uint32_t tick = 0;
uint32_t time;

uint8_t point;                  //选中的电压类型
uint8_t PPS_Point;              //PPS
uint32_t alive_time;            //按键操作时间记录
uint8_t first_click = 1;        //第一次按键操作标志

uint8_t Charge_dec;             //充电口检查标志
uint8_t Request_flag = 0;       //fixed\pps 申请标志
uint8_t PPS_Sta;                //PPS模式 标志
uint8_t PPS_change;             //修改PPS vol 标志

//extern volatile uint16_t PD_Vol;

void USARTx_CFG(void)
{
    GPIO_InitTypeDef  GPIO_InitStructure = {0};
    USART_InitTypeDef USART_InitStructure = {0};

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD | RCC_APB2Periph_USART1, ENABLE);

    /* USART1 TX-->D.5   RX-->D.6 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_30MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOD, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOD, &GPIO_InitStructure);

    USART_InitStructure.USART_BaudRate = 115200;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;

    USART_Init(USART1, &USART_InitStructure);
    USART_Cmd(USART1, ENABLE);
}

void led_menu_clear()
{
    volatile int i;
    for (i = 0; i < CH224Q.kind; ++i) {
        LED_Buf_Change(led_alive[i].sel, on);
    }
}

void led_sta_clear()
{
    volatile int i;
    for (i = 0; i < CH224Q.kind; ++i) {
        LED_Buf_Change(led_alive[i].set, off);
    }
}

void led_pps_clear()
{
    volatile int i;
    for (i = 0; i < 8; ++i) {
        LED_Buf_Change((FUNC_PPS_1Bit << i), off);
    }
}

void button_callback(struct lwbtn* lw, struct lwbtn_btn* btn, lwbtn_evt_t evt)
{
//    uint8_t btn_index = (btn - lw->btns);

    switch(evt){
        case LWBTN_EVT_ONPRESS:         //按键按下事件
            break;
        case LWBTN_EVT_ONRELEASE:       //按键松开事件
            alive_time = tick;
            break;
        case LWBTN_EVT_KEEPALIVE:       //按键长按事件
            Request_flag = 1;

            if (led_alive[point].set == FUNC_PPS_sta && PPS_change == 1) {
                CH224Q.PPS_ctrl ^= (1 << PPS_Point);

            }

            if (led_alive[point].set == FUNC_PPS_sta && PPS_Sta == 0) {
                PPS_Sta = 1;
                PPS_change = 1;

                CH224Q_PPS_Request(CH224Q.PPS_ctrl);

                led_pps_clear();
                LED_PPS_Vol_Set(CH224Q.PPS_ctrl);
                LED_Buf_Change(FUNC_PPS_1Bit << PPS_Point, blink);

                LED_Buf_Change(led_alive[point].sel, on);
            }

            break;
        case LWBTN_EVT_ONCLICK:
            if (btn->click.cnt == 2) {

            }
            else if (btn->click.cnt == 1) {
                if (first_click) {
                    first_click = 0;
                }
                else {
                    if (led_alive[point].set == FUNC_PPS_sta && PPS_Sta == 1) {
                        PPS_Point++;

                        if (PPS_Point > 7) {
                            PPS_Point = 0;
                        }

                    }
                    else {
                        point++;

                        if (point > CH224Q.kind - 1) {
                            point = 0;
                        }
                    }
                }
            }

            if (led_alive[point].set == FUNC_PPS_sta  && PPS_Sta == 1) {

                led_pps_clear();
                LED_PPS_Vol_Set(CH224Q.PPS_ctrl);
                LED_Buf_Change(FUNC_PPS_1Bit << PPS_Point, blink);
            }
            else if (led_alive[point].set == FUNC_PPS_sta  && PPS_Sta == 0) {
                PPS_Point = 0;

                led_menu_clear();
                LED_Buf_Change(led_alive[point].sel, blink);
            }
            else {
                led_pps_clear();
                led_menu_clear();
                LED_Buf_Change(led_alive[point].sel, blink);
            }

            break;
    }
}

int main(void)
{
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
    SystemCoreClockUpdate();
    Delay_Init();

    UART_Init(115200);
//    USART_Printf_Init(115200);
    printf("SystemClk:%d\r\n",SystemCoreClock);
    printf( "ChipID:%08x\r\n", DBGMCU_GetCHIPID() );

    I2C_Slave_Init(0xA0);
    TIM1_Count_Init(10 - 1, 96 - 1);    //1ms
    CH334_Init();

    while(Process());

    ADC_DMA_Init();
    button_init();
//    LED_IO_Init();
    I2C_Master_Init();
//    Delay_Ms(3000);         //延时先这样等QC，后面while中轮询等待

    Charge_dec = CH224Q_Init();

    TIM2_init(10 - 1, 48 - 1);

    uint8_t buf[1];
    for (int i = 0; i < 0x20; ++i) {
        flash_get(i, buf);
        printf("  %x  ", buf[0]);
    }

    while(1)
    {
//        if (tick - time > 1000) {
//            time = tick;
//        }

        if (tick - alive_time > 5000 && first_click == 0) {
            first_click = 1;
            PPS_Sta = 0;
            PPS_change = 0;

            led_menu_clear();

            if (led_alive[point].sel == FUNC_PPS) {
                led_pps_clear();

                if (CH224Q.PPS_ctrl > CH224Q.PPS_Vol.PPS_max/ 100) {
                    CH224Q.PPS_ctrl = CH224Q.PPS_Vol.PPS_max/ 100;
                }

                if (CH224Q.PPS_ctrl < CH224Q.PPS_Vol.PPS_min/ 100) {
                    CH224Q.PPS_ctrl = CH224Q.PPS_Vol.PPS_min/ 100;
                }
                LED_PPS_Vol_Set(CH224Q.PPS_ctrl);

                CH224Q_PPS_Request_Vol(CH224Q.PPS_ctrl);
            }
        }

        if (Request_flag == 1) {
            Request_flag = 0;

            led_sta_clear();
            LED_Buf_Change(led_alive[point].set, on);
            CH224Q_Pattern_Request(led_alive[point].vol_kind);
        }

        Decode_Process();
        get_btn();
    }
}

void EXTI7_0_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void EXTI7_0_IRQHandler(void)
{
    if (EXTI_GetITStatus(EXTI_Line0) != RESET) {

        EXTI_ClearITPendingBit(EXTI_Line0);
    }
}

void TIM1_UP_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void TIM1_UP_IRQHandler(void)
{
    if (TIM_GetITStatus(TIM1, TIM_IT_Update)) {
        tick++;
    }
    TIM_ClearITPendingBit(TIM1, TIM_IT_Update);
}

void TIM2_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void TIM2_IRQHandler(void)
{
    if (TIM_GetITStatus(TIM2, TIM_IT_Update)) {
//        LED_MUX_Process();
    }
    TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
}
