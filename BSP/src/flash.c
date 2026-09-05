/*
 * flash.c
 *
 *  Created on: 2026?7?16?
 *      Author: MXQ
 */

#include "ch32v00x.h"

#define FLASH_PAGE_256_START   ((uint32_t)0x08003F00)
#define FLASH_PAGE_256_END     ((uint32_t)0x08003FFF)
#define FLASH_PAGE_256_SIZE    (FLASH_PAGE_256_END - FLASH_PAGE_256_START + 1)

/**
 * @brief 从固定Page256地址读取半字数据
 * @param offset: 相对于Page256起始地址的偏移量(0-63)
 * @param value: 存储读取值的指针
 * @return int: 0-成功, -1-失败
 */
static int Flash_Read(uint16_t offset, uint16_t *value)
{
    if(offset > FLASH_PAGE_256_SIZE/2) return -1;  // 最大偏移量63 (0x3FC0-0x3FFF, 2字节对齐)

    *value = *(__IO uint16_t*)(FLASH_PAGE_256_START + offset * 2);
    return 0;
}

/**
 * @brief 向固定Page256地址写入半字数据
 * @param offset: 相对于Page256起始地址的偏移量(0-63)
 * @param value: 要写入的值
 * @return int: 0-成功, -1-失败
 */
static int Flash_Write(uint16_t offset, uint16_t value)
{
    FLASH_Status status;

    if(offset > FLASH_PAGE_256_SIZE/2) return -1;

    uint32_t address = FLASH_PAGE_256_START + offset * 2;

    FLASH_Unlock();
    status = FLASH_ProgramHalfWord(address, value);
    FLASH_Lock();

    return (status == FLASH_COMPLETE) ? 0 : -1;
}

/**
 * @brief 擦除固定的Page256
 * @return int: 0-成功, -1-失败
 */
static int Flash_Erase(void)
{
    FLASH_Status status;

    FLASH_Unlock();
    status = FLASH_ErasePage(FLASH_PAGE_256_START);
    FLASH_Lock();

    if(status != FLASH_COMPLETE) return -1;

    // 验证擦除是否成功
    for(uint32_t addr = FLASH_PAGE_256_START; addr <= FLASH_PAGE_256_END; addr += 2)
    {
        if(*(__IO uint16_t*)addr != 0xFFFF)
        {
            return -1;
        }
    }

    return 0;
}

/**
 * @brief 从固定Page256读取一个字节
 * @param index: 要读取的字节索引(0-63)
 * @param value: 存储读取值的指针
 * @return int: 0-成功, -1-失败
 */
int flash_get(uint16_t index, uint8_t *value)
{
    if(index >= FLASH_PAGE_256_SIZE) return -1;

    // 读取16位数据
    uint16_t halfword;
    uint16_t offset = index / 2;

    if(Flash_Read(offset, &halfword)) {
        return -1;
    }

    // 根据索引是奇数还是偶数决定取高字节还是低字节
    if(index % 2) {
        *value = (halfword >> 8) & 0xFF;  // 高字节
    } else {
        *value = halfword & 0xFF;         // 低字节
    }

    return 0;
}

/**
 * @brief 向固定Page256写入一个字节
 * @param index: 要写入的字节索引(0-63)
 * @param value: 要写入的值
 * @return int: 0-成功, -1-失败
 */
int flash_set(uint16_t index, uint8_t value)
{
    printf("flash write 0x%x\r\n", value);

    if(index >= FLASH_PAGE_256_SIZE) return -1;

    // 1. 先读取整个页面的数据到缓冲区
    uint8_t page_buffer[FLASH_PAGE_256_SIZE];
    for(int i = 0; i < FLASH_PAGE_256_SIZE; i++) {
        if(flash_get(i, &page_buffer[i])) {
            printf("i = %d return\r\n", i);
            return -1;
        }
    }

    // 2. 修改缓冲区中的指定字节
    page_buffer[index] = value;

    // 3. 擦除整个页面
    if(Flash_Erase()) {
        return -1;
    }

    // 4. 将缓冲区数据写回Flash
    for(int i = 0; i < FLASH_PAGE_256_SIZE / 2; i++) {
        uint16_t offset = i;
        uint16_t halfword = (page_buffer[2*i+1] << 8) | page_buffer[2*i];

        if(Flash_Write(offset, halfword)) {
            return -1;
        }
    }

    return 0;
}

