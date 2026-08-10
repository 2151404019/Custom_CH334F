/*
 * CH334.h
 *
 *  Created on: 2026?7?16?
 *      Author: MXQ
 */

#ifndef BSP_INC_CH334_H_
#define BSP_INC_CH334_H_

void CH334_Init();

void CH334_NRST();
void CH334_Ctrl(uint8_t sta);

uint8_t Process();

#endif /* BSP_INC_CH334_H_ */
