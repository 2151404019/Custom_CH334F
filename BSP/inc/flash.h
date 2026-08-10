/*
 * flash.h
 *
 *  Created on: 2026?7?16?
 *      Author: MXQ
 */

#ifndef BSP_INC_FLASH_H_
#define BSP_INC_FLASH_H_

int flash_get(uint16_t index, uint8_t *value);
int flash_set(uint16_t index, uint8_t value);

#endif /* BSP_INC_FLASH_H_ */
