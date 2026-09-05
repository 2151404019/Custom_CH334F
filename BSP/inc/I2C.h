/*
 * I2C.h
 *
 *  Created on: 2026?7?13?
 *      Author: MXQ
 */

#ifndef BSP_INC_I2C_H_
#define BSP_INC_I2C_H_

void I2C_Slave_Init(uint16_t addr);
void I2C_Master_Init();

uint8_t I2C_WriteData(uint8_t addr,uint8_t *data, uint8_t len, uint32_t timeout);
uint8_t I2C_WriteData8Bit(uint8_t addr, uint8_t reg, uint8_t data);
uint8_t I2C_WriteData16Bit(uint8_t addr, uint8_t reg, uint16_t data);

uint8_t I2C_ReadData8Bit(uint8_t addr, uint8_t reg, uint8_t *data);
uint8_t I2C_ReadData(uint8_t addr, uint8_t reg, uint8_t len, uint8_t *buf);

void EEP_Write(uint8_t reg, uint8_t data);
void EEP_Read();

#endif /* BSP_INC_I2C_H_ */
