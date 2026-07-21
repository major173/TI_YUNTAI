#ifndef BMI088MIDDLEWARE_H
#define BMI088MIDDLEWARE_H

#include "stdint.h"

/* 选择 BMI088 通信总线。当前工程使用 SPI，I2C 接口尚未实现。 */
#define BMI088_USE_SPI
//#define BMI088_USE_IIC

/* 平台相关初始化接口：可由主程序提前调用，也可在驱动初始化时调用。 */
extern void DWT_Init(void);
extern void BMI088_GPIO_init(void);
extern void BMI088_com_init(void);
/* 延时接口，驱动层不直接依赖具体定时器实现。 */
extern void BMI088_delay_ms(uint16_t ms);
extern void BMI088_delay_us(uint16_t us);

#if defined(BMI088_USE_SPI)
/* 加速度计片选：低电平选中，高电平释放。 */
extern void BMI088_ACCEL_NS_L(void);
extern void BMI088_ACCEL_NS_H(void);

/* 陀螺仪片选：低电平选中，高电平释放。 */
extern void BMI088_GYRO_NS_L(void);
extern void BMI088_GYRO_NS_H(void);

/* SPI 全双工收发一个字节。发送 dummy 字节时返回传感器读出的数据。 */
extern uint8_t BMI088_read_write_byte(uint8_t reg);
/* 连续寄存器 DMA 读取，buf 至少能容纳 len 个字节。 */
extern void BMI088_DMA_read_muli_reg(uint8_t reg, uint8_t *buf, uint8_t len);

#elif defined(BMI088_USE_IIC)

#endif

#endif
