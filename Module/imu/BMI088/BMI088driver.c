#include "BMI088driver.h"
#include "BMI088reg.h"
#include "BMI088Middleware.h"
#include <string.h>
#include <stdio.h>

/* 当前量程对应的原始值换算系数。单位分别为 m/s^2/LSB 和 rad/s/LSB。 */
float BMI088_ACCEL_SEN = BMI088_ACCEL_3G_SEN;
float BMI088_GYRO_SEN = BMI088_GYRO_2000_SEN;



#if defined(BMI088_USE_SPI)
/**
************************************************************************
* @brief:      	BMI088_accel_write_single_reg(reg, data)
* @param:       reg - 寄存器地址
*               data - 写入的数据
* @retval:     	void
* @details:    	通过BMI088加速度计的SPI总线写入单个寄存器的宏定义
************************************************************************
**/
/* 加速度计单寄存器写：拉低 ACC_CS，完成 SPI 事务后释放片选。 */
#define BMI088_accel_write_single_reg(reg, data) \
    {                                            \
        BMI088_ACCEL_NS_L();                     \
        BMI088_write_single_reg((reg), (data));  \
        BMI088_ACCEL_NS_H();                     \
    }
/**
************************************************************************
* @brief:      	BMI088_accel_read_single_reg(reg, data)
* @param:       reg - 寄存器地址
*               data - 读取的寄存器数据
* @retval:     	void
* @details:    	通过BMI088加速度计的SPI总线读取单个寄存器的宏定义
************************************************************************
**/
/* 加速度计单寄存器读。加速度计读地址后需要发送一个 dummy 字节。 */
#define BMI088_accel_read_single_reg(reg, data) \
    {                                           \
        BMI088_ACCEL_NS_L();                    \
        BMI088_read_write_byte((reg) | 0x80);   \
        BMI088_read_write_byte(0x55);           \
        (data) = BMI088_read_write_byte(0x55);  \
        BMI088_ACCEL_NS_H();                    \
    }
/**
************************************************************************
* @brief:      	BMI088_accel_read_muli_reg(reg, data, len)
* @param:       reg - 起始寄存器地址
*               data - 存储读取数据的缓冲区
*               len - 要读取的字节数
* @retval:     	void
* @details:    	通过BMI088加速度计的SPI总线连续读取多个寄存器的宏定义
*               memmove 会正确处理源和目的地址重叠的情况
************************************************************************
**/
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wattributes"
/* 加速度计连续读取。DMA 返回数据的第一个字节是读地址阶段的无效字节，需移除。 */
#define BMI088_accel_read_muli_reg(reg, data, len) \
    {                                              \
        BMI088_ACCEL_NS_L();                       \
        BMI088_DMA_read_muli_reg(reg, data, len + 1); \
        BMI088_ACCEL_NS_H();                       \
        memmove(data, &data[1], len);              \
    }
#pragma GCC diagnostic pop
/**
************************************************************************
* @brief:      	BMI088_gyro_write_single_reg(reg, data)
* @param:       reg - 寄存器地址
*               data - 写入的数据
* @retval:     	void
* @details:    	通过BMI088陀螺仪的SPI总线写入单个寄存器的宏定义
************************************************************************
**/
/* 陀螺仪单寄存器写。 */
#define BMI088_gyro_write_single_reg(reg, data) \
    {                                           \
        BMI088_GYRO_NS_L();                     \
        BMI088_write_single_reg((reg), (data)); \
        BMI088_GYRO_NS_H();                     \
    }
/**
************************************************************************
* @brief:      	BMI088_gyro_read_single_reg(reg, data)
* @param:       reg - 寄存器地址
*               data - 读取的寄存器数据
* @retval:     	void
* @details:    	通过BMI088陀螺仪的SPI总线读取单个寄存器的宏定义
************************************************************************
**/
/* 陀螺仪单寄存器读。陀螺仪与加速度计的 SPI 读时序不同，不能混用片选。 */
#define BMI088_gyro_read_single_reg(reg, data)  \
    {                                           \
        BMI088_GYRO_NS_L();                     \
        BMI088_read_single_reg((reg), &(data)); \
        BMI088_GYRO_NS_H();                     \
    }
/**
************************************************************************
* @brief:      	BMI088_gyro_read_muli_reg(reg, data, len)
* @param:       reg - 起始寄存器地址
*               data - 存储读取数据的缓冲区
*               len - 要读取的字节数
* @retval:     	void
* @details:    	通过BMI088陀螺仪的SPI总线连续读取多个寄存器的宏定义
************************************************************************
**/
/* 陀螺仪连续读取，底层实现负责处理是否存在 dummy 字节。 */
#define BMI088_gyro_read_muli_reg(reg, data, len)   \
    {                                               \
        BMI088_GYRO_NS_L();                         \
        BMI088_DMA_read_muli_reg((reg), (data), (len)); \
        BMI088_GYRO_NS_H();                         \
    }

static void BMI088_write_single_reg(uint8_t reg, uint8_t data);
static void BMI088_read_single_reg(uint8_t reg, uint8_t *return_data);
//static void BMI088_write_muli_reg(uint8_t reg, uint8_t* buf, uint8_t len );
static void BMI088_read_muli_reg(uint8_t reg, uint8_t *buf, uint8_t len);

#elif defined(BMI088_USE_IIC)


#endif
/**
************************************************************************
* @brief:      	write_BMI088_accel_reg_data_error_init(void)
* @param:       void
* @retval:     	void
* @details:    	BMI088加速度传感器寄存器数据写入错误处理初始化
************************************************************************
**/
static uint8_t write_BMI088_accel_reg_data_error[BMI088_WRITE_ACCEL_REG_NUM][3] =
    {
        {BMI088_ACC_PWR_CTRL, BMI088_ACC_ENABLE_ACC_ON, BMI088_ACC_PWR_CTRL_ERROR},
        {BMI088_ACC_PWR_CONF, BMI088_ACC_PWR_ACTIVE_MODE, BMI088_ACC_PWR_CONF_ERROR},
        {BMI088_ACC_CONF,  BMI088_ACC_NORMAL| BMI088_ACC_800_HZ | BMI088_ACC_CONF_MUST_Set, BMI088_ACC_CONF_ERROR},
        {BMI088_ACC_RANGE, BMI088_ACC_RANGE_3G, BMI088_ACC_RANGE_ERROR},
        {BMI088_INT1_IO_CTRL, BMI088_ACC_INT1_IO_ENABLE | BMI088_ACC_INT1_GPIO_PP | BMI088_ACC_INT1_GPIO_LOW, BMI088_INT1_IO_CTRL_ERROR},
        {BMI088_INT_MAP_DATA, BMI088_ACC_INT1_DRDY_INTERRUPT, BMI088_INT_MAP_DATA_ERROR}

};
/* 加速度计初始化配置表：寄存器地址、期望写入值、失败时返回的错误码。 */
/**
************************************************************************
* @brief:      	write_BMI088_gyro_reg_data_error_init(void)
* @param:       void
* @retval:     	void
* @details:    	BMI088陀螺仪传感器寄存器数据写入错误处理初始化
************************************************************************
**/
static uint8_t write_BMI088_gyro_reg_data_error[BMI088_WRITE_GYRO_REG_NUM][3] =
    {
        {BMI088_GYRO_RANGE, BMI088_GYRO_2000, BMI088_GYRO_RANGE_ERROR},
        {BMI088_GYRO_BANDWIDTH, BMI088_GYRO_1000_116_HZ | BMI088_GYRO_BANDWIDTH_MUST_Set, BMI088_GYRO_BANDWIDTH_ERROR},
        {BMI088_GYRO_LPM1, BMI088_GYRO_NORMAL_MODE, BMI088_GYRO_LPM1_ERROR},
        {BMI088_GYRO_CTRL, BMI088_DRDY_ON, BMI088_GYRO_CTRL_ERROR},
        {BMI088_GYRO_INT3_INT4_IO_CONF, BMI088_GYRO_INT3_GPIO_PP | BMI088_GYRO_INT3_GPIO_LOW, BMI088_GYRO_INT3_INT4_IO_CONF_ERROR},
        {BMI088_GYRO_INT3_INT4_IO_MAP, BMI088_GYRO_DRDY_IO_INT3, BMI088_GYRO_INT3_INT4_IO_MAP_ERROR}

};
/* 陀螺仪初始化配置表：寄存器地址、期望写入值、失败时返回的错误码。 */
/**
************************************************************************
* @brief:      	BMI088_init(void)
* @param:       void
* @retval:     	uint8_t - 错误代码
* @details:    	BMI088传感器初始化函数，包括GPIO和SPI初始化，以及加速度和陀螺仪的初始化
************************************************************************
**/
uint8_t BMI088_init(void)
{
    /* 先完成板级接口初始化，再分别初始化加速度计和陀螺仪。 */
    static uint8_t dwt_ready = 0U;
    uint8_t error = BMI088_NO_ERROR;
    printf("[BMI088] 开始初始化IMU\r\n");

    if (dwt_ready == 0U)
    {
        DWT_Init();
        dwt_ready = 1U;
    }
    
    // GPIO and SPI  Init .
    BMI088_GPIO_init();
    BMI088_com_init();

    printf("[BMI088] 初始化加速度计...\r\n");
    error |= bmi088_accel_init();
    
    printf("[BMI088] 初始化陀螺仪...\r\n");
    error |= bmi088_gyro_init();

    printf("[BMI088] 初始化完成，状态码: 0x%02X\r\n", error);
    return error;
}
/**
************************************************************************
* @brief:      	bmi088_accel_init(void)
* @param:       void
* @retval:     	uint8_t - 错误代码
* @details:    	BMI088加速度传感器初始化函数，包括通信检查、软件复位、配置寄存器写入及检查
************************************************************************
**/
uint8_t bmi088_accel_init(void)
{
    uint8_t res = 0;
    uint8_t write_reg_num = 0;

    /* 复位前先读取两次 ID，满足 BMI088 SPI 通信建立时间要求。 */
    BMI088_accel_read_single_reg(BMI088_ACC_CHIP_ID, res);
    BMI088_delay_us(BMI088_COM_WAIT_SENSOR_TIME);
    BMI088_accel_read_single_reg(BMI088_ACC_CHIP_ID, res);
    BMI088_delay_us(BMI088_COM_WAIT_SENSOR_TIME);
    
    printf("[ACC] 芯片ID: 0x%02X\r\n", res);

    /* 软件复位加速度计，复位后等待传感器重新启动。 */
    BMI088_accel_write_single_reg(BMI088_ACC_SOFTRESET, BMI088_ACC_SOFTRESET_VALUE);
    BMI088_delay_ms(BMI088_LONG_DELAY_TIME);

    /* 复位后再次读取 ID，确认通信和芯片均正常。 */
    BMI088_accel_read_single_reg(BMI088_ACC_CHIP_ID, res);
    BMI088_delay_us(BMI088_COM_WAIT_SENSOR_TIME);
    BMI088_accel_read_single_reg(BMI088_ACC_CHIP_ID, res);
    BMI088_delay_us(BMI088_COM_WAIT_SENSOR_TIME);

    /* 加速度计 WHO_AM_I 应为 0x1E。 */
    if (res != BMI088_ACC_CHIP_ID_VALUE)
    {
        printf("[ACC] 错误: 芯片ID不匹配 (0x%02X vs 0x%02X)\r\n", res, BMI088_ACC_CHIP_ID_VALUE);
        return BMI088_NO_SENSOR;
    }

    /* 逐项写入配置，并立即回读校验，避免静默使用错误配置。 */
    for (write_reg_num = 0; write_reg_num < BMI088_WRITE_ACCEL_REG_NUM; write_reg_num++)
    {
        BMI088_accel_write_single_reg(write_BMI088_accel_reg_data_error[write_reg_num][0], write_BMI088_accel_reg_data_error[write_reg_num][1]);
        BMI088_delay_us(BMI088_COM_WAIT_SENSOR_TIME);

        BMI088_accel_read_single_reg(write_BMI088_accel_reg_data_error[write_reg_num][0], res);
        BMI088_delay_us(BMI088_COM_WAIT_SENSOR_TIME);

        if (res != write_BMI088_accel_reg_data_error[write_reg_num][1])
        {
            printf("[ACC] 错误: 寄存器0x%02X配置失败\r\n", write_BMI088_accel_reg_data_error[write_reg_num][0]);
            return write_BMI088_accel_reg_data_error[write_reg_num][2];
        }
    }
    printf("[ACC] 初始化成功\r\n");
    return BMI088_NO_ERROR;
}
/**
************************************************************************
* @brief:      	bmi088_gyro_init(void)
* @param:       void
* @retval:     	uint8_t - 错误代码
* @details:    	BMI088陀螺仪传感器初始化函数，包括通信检查、软件复位、配置寄存器写入及检查
************************************************************************
**/
uint8_t bmi088_gyro_init(void)
{
    uint8_t write_reg_num = 0;
    uint8_t res = 0;

    /* 复位前读取陀螺仪 ID。 */
    BMI088_gyro_read_single_reg(BMI088_GYRO_CHIP_ID, res);
    BMI088_delay_us(BMI088_COM_WAIT_SENSOR_TIME);
    BMI088_gyro_read_single_reg(BMI088_GYRO_CHIP_ID, res);
    BMI088_delay_us(BMI088_COM_WAIT_SENSOR_TIME);
    
    printf("[GYRO] 芯片ID: 0x%02X\r\n", res);

    /* 软件复位陀螺仪，复位后等待 80 ms。 */
    BMI088_gyro_write_single_reg(BMI088_GYRO_SOFTRESET, BMI088_GYRO_SOFTRESET_VALUE);
    BMI088_delay_ms(BMI088_LONG_DELAY_TIME);
    /* 复位后重新读取 ID，确认通信恢复。 */
    BMI088_gyro_read_single_reg(BMI088_GYRO_CHIP_ID, res);
    BMI088_delay_us(BMI088_COM_WAIT_SENSOR_TIME);
    BMI088_gyro_read_single_reg(BMI088_GYRO_CHIP_ID, res);
    BMI088_delay_us(BMI088_COM_WAIT_SENSOR_TIME);

    /* 陀螺仪 WHO_AM_I 应为 0x0F。 */
    if (res != BMI088_GYRO_CHIP_ID_VALUE)
    {
        printf("[GYRO] 错误: 芯片ID不匹配 (0x%02X vs 0x%02X)\r\n", res, BMI088_GYRO_CHIP_ID_VALUE);
        return BMI088_NO_SENSOR;
    }

    /* 写入陀螺仪量程、带宽、工作模式和数据就绪中断配置，并回读校验。 */
    for (write_reg_num = 0; write_reg_num < BMI088_WRITE_GYRO_REG_NUM; write_reg_num++)
    {

        BMI088_gyro_write_single_reg(write_BMI088_gyro_reg_data_error[write_reg_num][0], write_BMI088_gyro_reg_data_error[write_reg_num][1]);
        BMI088_delay_us(BMI088_COM_WAIT_SENSOR_TIME);

        BMI088_gyro_read_single_reg(write_BMI088_gyro_reg_data_error[write_reg_num][0], res);
        BMI088_delay_us(BMI088_COM_WAIT_SENSOR_TIME);

        if (res != write_BMI088_gyro_reg_data_error[write_reg_num][1])
        {
            printf("[GYRO] 错误: 寄存器0x%02X配置失败\r\n", write_BMI088_gyro_reg_data_error[write_reg_num][0]);
            return write_BMI088_gyro_reg_data_error[write_reg_num][2];
        }
    }
    printf("[GYRO] 初始化成功\r\n");   
    return BMI088_NO_ERROR;
}
/**
************************************************************************
* @brief:      	BMI088_read(float gyro[3], float accel[3], float *temperate)
* @param:       gyro - 陀螺仪数据数组 (x, y, z)
* @param:       accel - 加速度计数据数组 (x, y, z)
* @param:       temperate - 温度数据指针
* @retval:     	void
* @details:    	读取BMI088传感器数据，包括加速度、陀螺仪和温度
************************************************************************
**/
void BMI088_read(float gyro[3], float accel[3], float *temperate)
{
    /* 一次读取加速度、陀螺仪和温度，输出已经转换为物理单位的浮点数。 */
    uint8_t buf[8] = {0, 0, 0, 0, 0, 0};
    int16_t bmi088_raw_temp;

    /* 加速度计 X/Y/Z：每轴低字节在前，高字节在后。 */
    BMI088_accel_read_muli_reg(BMI088_ACCEL_XOUT_L, buf, 6);

    bmi088_raw_temp = (int16_t)((buf[1]) << 8) | buf[0];
    accel[0] = bmi088_raw_temp * BMI088_ACCEL_SEN;
    bmi088_raw_temp = (int16_t)((buf[3]) << 8) | buf[2];
    accel[1] = bmi088_raw_temp * BMI088_ACCEL_SEN;
    bmi088_raw_temp = (int16_t)((buf[5]) << 8) | buf[4];
    accel[2] = bmi088_raw_temp * BMI088_ACCEL_SEN;

    /* 从陀螺仪 0x00 开始读取 ID、保留字节和三轴数据。 */
    BMI088_gyro_read_muli_reg(BMI088_GYRO_CHIP_ID, buf, 8);
    if(buf[0] == BMI088_GYRO_CHIP_ID_VALUE)
    {
        bmi088_raw_temp = (int16_t)((buf[3]) << 8) | buf[2];
        gyro[0] = bmi088_raw_temp * BMI088_GYRO_SEN;
        bmi088_raw_temp = (int16_t)((buf[5]) << 8) | buf[4];
        gyro[1] = bmi088_raw_temp * BMI088_GYRO_SEN;
        bmi088_raw_temp = (int16_t)((buf[7]) << 8) | buf[6];
        gyro[2] = bmi088_raw_temp * BMI088_GYRO_SEN;
    }
    /* 温度为 11 位有符号值，寄存器布局为 TEMP_M[7:0]、TEMP_L[7:5]。 */
    BMI088_accel_read_muli_reg(BMI088_TEMP_M, buf, 2);

    bmi088_raw_temp = (int16_t)((buf[0] << 3) | (buf[1] >> 5));

    if (bmi088_raw_temp > 1023)
    {
        bmi088_raw_temp -= 2048;
    }

    *temperate = bmi088_raw_temp * BMI088_TEMP_FACTOR + BMI088_TEMP_OFFSET;
}

#if defined(BMI088_USE_SPI)
/**
************************************************************************
* @brief:      	BMI088_write_single_reg(uint8_t reg, uint8_t data)
* @param:       reg - 寄存器地址
* @param:       data - 写入的数据
* @retval:     	void
* @details:    	向BMI088传感器写入单个寄存器的数据
************************************************************************
**/
static void BMI088_write_single_reg(uint8_t reg, uint8_t data)
{
    /* SPI 写事务：寄存器地址后紧跟写入数据。 */
    BMI088_read_write_byte(reg);
    BMI088_read_write_byte(data);
}
/**
************************************************************************
* @brief:      	BMI088_read_single_reg(uint8_t reg, uint8_t *return_data)
* @param:       reg - 寄存器地址
* @param:       return_data - 读取的寄存器数据
* @retval:     	void
* @details:    	从BMI088传感器读取单个寄存器的数据
************************************************************************
**/
static void BMI088_read_single_reg(uint8_t reg, uint8_t *return_data)
{
    /* SPI 读事务：地址最高位置 1，再发送 dummy 字节取得返回数据。 */
    BMI088_read_write_byte(reg | 0x80);
    *return_data = BMI088_read_write_byte(0x55);
}

//static void BMI088_write_muli_reg(uint8_t reg, uint8_t* buf, uint8_t len )
//{
//    BMI088_read_write_byte( reg );
//    while( len != 0 )
//    {

//        BMI088_read_write_byte( *buf );
//        buf ++;
//        len --;
//    }

//}
/**
************************************************************************
* @brief:      	BMI088_read_muli_reg(uint8_t reg, uint8_t *buf, uint8_t len)
* @param:       reg - 起始寄存器地址
*               buf - 存储读取数据的缓冲区
*               len - 要读取的字节数
* @retval:     	void
* @details:    	从BMI088传感器连续读取多个寄存器的数据
************************************************************************
**/
static void BMI088_read_muli_reg(uint8_t reg, uint8_t *buf, uint8_t len) __attribute__((unused));
static void BMI088_read_muli_reg(uint8_t reg, uint8_t *buf, uint8_t len)
{
    /* 连续读取 len 个寄存器，调用者负责片选控制。 */
    BMI088_read_write_byte(reg | 0x80);

    while (len != 0)
    {

        *buf = BMI088_read_write_byte(0x55);
        buf++;
        len--;
    }
}
#elif defined(BMI088_USE_IIC)

#endif
