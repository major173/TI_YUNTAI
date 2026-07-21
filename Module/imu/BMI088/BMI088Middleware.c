#include "BMI088Middleware.h"
#include "main.h"
#include "spi.h"
#include <string.h>

#define BMI088_USING_SPI_UNIT   hspi2
/* 目标工程若使用其他 SPI 外设，只需修改上面的句柄名称。 */

/* DMA 缓冲区需要在 AXI SRAM (RAM_D1) 中，并正确处理 D-Cache */
// uint8_t spi_rx_buf[64] __attribute__((section(".axi_sram"))) __attribute__((aligned(32)));
// uint8_t spi_tx_buf[64] __attribute__((section(".axi_sram"))) __attribute__((aligned(32)));

//F407没有 AXI SRAM；没有数据 Cache；不需要 32 字节 Cache 行对齐；链接脚本通常也没有 .axi_sram 段。
//修改为普通静态数组：

static uint8_t spi_rx_buf[64];
static uint8_t spi_tx_buf[64];

extern SPI_HandleTypeDef BMI088_USING_SPI_UNIT;

/* BMI088 加速度计使用 SPI 模式 0，陀螺仪使用 SPI 模式 3。 */
static void BMI088_SPI_SetMode(uint32_t polarity, uint32_t phase)
{
    __HAL_SPI_DISABLE(&BMI088_USING_SPI_UNIT);
    MODIFY_REG(BMI088_USING_SPI_UNIT.Instance->CR1,
               SPI_CR1_CPOL | SPI_CR1_CPHA, polarity | phase);
    __HAL_SPI_ENABLE(&BMI088_USING_SPI_UNIT);
}
/**
************************************************************************
* @brief:      	DWT_init(void)
* @param:       void
* @retval:     	void
* @details:    	初始化 DWT 周期计数器，用于微秒级延迟
************************************************************************
**/
void DWT_Init(void) 
{
    /* 开启 DWT 周期计数器，用于实现微秒级延时。 */
    // 1. 使能 Trace 单元 (DEMCR 寄存器的第 24 位)
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    
    // 2. 解锁 DWT 寄存器访问 (H7/Cortex-M7 必须步骤) F407不需要这一步
    // 0xC5ACCE55 是 ARM 指定的解锁密钥
    //DWT->LAR = 0xC5ACCE55; 
    
    // 3. 清零循环计数器
    DWT->CYCCNT = 0;
    
    // 4. 开启 CYCCNT 计数
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}
/**
************************************************************************
* @brief:      	BMI088_GPIO_init(void)
* @param:       void
* @retval:     	void
* @details:    	BMI088传感器GPIO初始化函数
************************************************************************
**/
void BMI088_GPIO_init(void)
{
    /* 当前工程的 CS/INT GPIO 由 MX_GPIO_Init() 完成。 */
    
}
/**
************************************************************************
* @brief:      	BMI088_com_init(void)
* @param:       void
* @retval:     	void
* @details:    	BMI088传感器通信初始化函数
************************************************************************
**/
void BMI088_com_init(void)
{
    /* 当前工程的 SPI2 和 DMA 由 CubeMX 初始化。 */


}
/**
************************************************************************
* @brief:      	BMI088_delay_ms(uint16_t ms)
* @param:       ms - 要延迟的毫秒数
* @retval:     	void
* @details:    	延迟指定毫秒数的函数，基于微秒延迟实现
************************************************************************
**/
void BMI088_delay_ms(uint16_t ms)
{
    /* 毫秒延时使用 HAL 系统 tick。 */
    HAL_Delay(ms);
}
/**
************************************************************************
* @brief:      	BMI088_delay_us(uint16_t us)
* @param:       us - 要延迟的微秒数
* @retval:     	void
* @details:    	微秒级延迟函数，使用 DWT 周期计数器实现
*               不受 FreeRTOS 调度影响，精度极高
************************************************************************
**/
void BMI088_delay_us(uint16_t us)
{
    /* 将微秒换算为 CPU 周期，利用无符号减法处理计数器回绕。 */
    uint32_t start_tick = DWT->CYCCNT;
    // 使用 SystemCoreClock 动态计算周期，兼容不同频率
    uint32_t delay_ticks = us * (SystemCoreClock / 1000000UL);

    while ((DWT->CYCCNT - start_tick) < delay_ticks)
    {
        __asm("nop");
    }
}
/**
************************************************************************
* @brief:      	BMI088_ACCEL_NS_L(void)
* @param:       void
* @retval:     	void
* @details:    	将BMI088加速度计片选信号置低，使其处于选中状态
************************************************************************
**/
void BMI088_ACCEL_NS_L(void)
{
    /* 加速度计片选有效。 */
    BMI088_SPI_SetMode(SPI_POLARITY_LOW, SPI_PHASE_1EDGE);
    HAL_GPIO_WritePin(ACC_CS_GPIO_Port, ACC_CS_Pin, GPIO_PIN_RESET);
}
/**
************************************************************************
* @brief:      	BMI088_ACCEL_NS_H(void)
* @param:       void
* @retval:     	void
* @details:    	将BMI088加速度计片选信号置高，使其处于非选中状态
************************************************************************
**/
void BMI088_ACCEL_NS_H(void)
{
    /* 加速度计片选无效。 */
    HAL_GPIO_WritePin(ACC_CS_GPIO_Port, ACC_CS_Pin, GPIO_PIN_SET);
}
/**
************************************************************************
* @brief:      	BMI088_GYRO_NS_L(void)
* @param:       void
* @retval:     	void
* @details:    	将BMI088陀螺仪片选信号置低，使其处于选中状态
************************************************************************
**/
void BMI088_GYRO_NS_L(void)
{
    /* 陀螺仪片选有效。 */
    BMI088_SPI_SetMode(SPI_POLARITY_HIGH, SPI_PHASE_2EDGE);
    HAL_GPIO_WritePin(GYRO_CS_GPIO_Port, GYRO_CS_Pin, GPIO_PIN_RESET);
}
/**
************************************************************************
* @brief:      	BMI088_GYRO_NS_H(void)
* @param:       void
* @retval:     	void
* @details:    	将BMI088陀螺仪片选信号置高，使其处于非选中状态
************************************************************************
**/
void BMI088_GYRO_NS_H(void)
{
    /* 陀螺仪片选无效。 */
    HAL_GPIO_WritePin(GYRO_CS_GPIO_Port, GYRO_CS_Pin, GPIO_PIN_SET);
}
/**
************************************************************************
* @brief:      	BMI088_read_write_byte(uint8_t txdata)
* @param:       txdata - 要发送的数据
* @retval:     	uint8_t - 接收到的数据
* @details:    	通过BMI088使用的SPI总线进行单字节的读写操作
************************************************************************
**/
uint8_t BMI088_read_write_byte(uint8_t txdata)
{
    /* 阻塞式 SPI 单字节收发。 */
    uint8_t rx_data;
    HAL_SPI_TransmitReceive(&BMI088_USING_SPI_UNIT, &txdata, &rx_data, 1, 1000);
    return rx_data;
}

/**
* @brief  新增：多字节 DMA 读取函数 (针对传感器数据)
**/
void BMI088_DMA_read_muli_reg(uint8_t reg, uint8_t *buf, uint8_t len)
{
    /* DMA 连续读取：先发读地址，再发送 dummy 字节接收数据。 */
    // 1. 准备发送缓冲区（最高位1为读）
    /* 清空发送缓冲区，确保地址后的字节均为 dummy。 */
    memset(spi_tx_buf, 0, 64);
    spi_tx_buf[0] = reg | 0x80;
    
    // 2. 清除 TX 缓冲区的 D-Cache，确保 DMA 能读取最新数据  F407没有数据 Cache，不需要这一步
    //SCB_CleanDCache_by_Addr((uint32_t*)spi_tx_buf, 64);

    // 3. 启动 DMA 传输 (传输长度为 地址 + 数据)
    /* DMA 长度包含 1 个地址字节和 len 个数据字节。 */
    HAL_SPI_TransmitReceive_DMA(&BMI088_USING_SPI_UNIT, spi_tx_buf, spi_rx_buf, len + 1);

    // 4. 轮询等待 DMA 完成
    /* 当前实现阻塞等待 DMA 完成，移植时建议增加超时保护。 */
    while (HAL_SPI_GetState(&BMI088_USING_SPI_UNIT) != HAL_SPI_STATE_READY);
    
    // 5. 使 RX 缓冲区的 D-Cache 失效，强制 CPU 从内存读取 DMA 传输的新数据 
    /* 使 RX Cache 失效，强制 CPU 读取 DMA 写入的数据。 */
    //SCB_InvalidateDCache_by_Addr((uint32_t*)spi_rx_buf, 64);

    // 6. 拷贝数据（跳过第一个字节，它是发送地址时收到的无效字节）
    /* 跳过地址阶段收到的无效字节，只返回有效寄存器数据。 */
    memcpy(buf, &spi_rx_buf[1], len);
}
