# TI_YUNTAI

基于 `STM32F407VET6` 的 2025 年电赛云台控制工程。

这个仓库不再按“单个外设测试工程”维护，而是按最终比赛成品来组织。目标是把云台板做成一个独立、清晰、可扩展的控制节点，负责姿态感知、目标跟踪、云台执行和与底盘/视觉的通信。

## 项目目标

本工程聚焦 2025 年电赛题目中的云台部分，最终要完成的不是“电机能转”，而是一套完整的二维云台控制系统：

- `BMI088` 提供角速度和姿态信息
- `QD4310` 负责 `yaw` 轴闭环控制
- `ZDT Y42 + 1:10 行星减速箱` 负责 `pitch` 轴位置控制
- 与 `TI` 底盘板通信，接收底盘状态和循迹阶段信息
- 与树莓派视觉模块通信，接收目标偏差和置信度
- 在 `FreeRTOS` 下实现任务化、模块化控制架构

## 最终成品应该具备的功能

云台部分最终至少要具备下面这些能力：

- 上电初始化和自检
- BMI088 正常通信
- BMI088 零偏标定和姿态解算
- `yaw` 轴定向保持
- `pitch` 轴回零、限位和目标角控制
- 接收视觉目标偏差并执行自动跟踪
- 根据视觉置信度决定是否锁定目标
- 目标丢失后执行搜索和重新捕获
- 接收底盘角速度并做前馈补偿
- 输出调试信息，便于联调

## 当前硬件方案

### 主控

- `STM32F407VET6`
- `FreeRTOS + CMSIS V2`
- `HAL Timebase = TIM6`

### 传感器

- `BMI088`
  - 接口：`SPI2`
  - 作用：获取角速度、加速度，做姿态解算和航向稳定

### 执行器

- `QD4310`
  - 接口：`USART2`
  - 作用：驱动云台 `yaw` 轴

- `ZDT Y42 第二代闭环步进电机 + 1:10 行星减速箱`
  - 接口：`USART1`
  - 作用：驱动云台 `pitch` 轴

### 外部协同模块

- `TI 底盘板`
  - 负责循迹、底盘运动、底盘陀螺仪和比赛流程中的移动部分
  - 向云台板提供底盘状态、角速度、当前模式等信息

- `树莓派视觉模块`
  - 负责图像处理和目标识别
  - 向云台板提供目标偏差、是否有效、置信度等信息

## 当前引脚配置

以下引脚以当前工程 [`yuntai_v2.ioc`](D:/32CUBEMXCODE/Ti/yuntai/yuntai_v2/yuntai_v2.ioc) 为准。

### BMI088 / SPI2

- `PB13` -> `SPI2_SCK`
- `PC2` -> `SPI2_MISO`
- `PC3` -> `SPI2_MOSI`
- `PD0` -> `ACC_CS`
- `PD1` -> `GYRO_CS`

### 步进电机串口

- `PA9` -> `USART1_TX`
- `PA10` -> `USART1_RX`

说明：

- 当前 `USART1` 已开启 `DMA RX/TX`
- 后续适合做步进驱动串口协议收发

### QD4310 串口

- `PD5` -> `USART2_TX`
- `PD6` -> `USART2_RX`

### 调试串口

- `PB10` -> `USART3_TX`
- `PB11` -> `USART3_RX`

### 下载调试接口

- `PA13` -> `SWDIO`
- `PA14` -> `SWCLK`

## 当前时钟配置

当前工程采用标准 `STM32F407` 高性能配置：

- `HSE = 8 MHz`
- `SYSCLK = 168 MHz`
- `AHB = 168 MHz`
- `APB1 = 42 MHz`
- `APB2 = 84 MHz`

另外当前配置是：

- `FreeRTOS` 使用 `SysTick`
- `HAL Tick` 使用 `TIM6`

这样可以避免 `HAL` 和 `FreeRTOS` 同时抢占 `SysTick` 带来的问题。

## 推荐控制思路

### 1. 树莓派应该给云台什么数据

树莓派不要直接给“电机转多少”，而应该给“目标观测结果”。

推荐输出结构：

- `valid`
- `yaw_err_rad`
- `pitch_err_rad`
- `confidence`
- `target_id`
- `timestamp`

含义可以理解为：

- `yaw_err_rad`：目标中心相对画面中心的水平角误差
- `pitch_err_rad`：目标中心相对画面中心的俯仰角误差
- `confidence`：当前识别可信度

这样做的好处是视觉只负责“看见什么”，云台板负责“怎么动”。

### 2. 云台板负责什么

F407 需要把下面几类信息融合起来：

- 视觉偏差
- BMI088 姿态信息
- 底盘角速度或底盘模式
- 当前云台自身位置和状态

然后计算出两个控制参考量：

- `yaw_ref`
- `pitch_ref`

再分别下发给：

- `QD4310` 执行 `yaw`
- `ZDT Y42` 执行 `pitch`

### 3. 两个轴推荐的控制方式

- `yaw` 轴：
  - 推荐 `角度外环 + 速度内环`
  - 外环放在 `STM32F407`
  - 如果 QD4310 自带较成熟速度能力，就把它当执行器用

- `pitch` 轴：
  - 推荐做位置控制
  - 控制链路是 `目标角度 -> 输出轴角度 -> 电机脉冲/位置命令`

### 4. 底盘补偿为什么有必要

如果底盘在转弯或循迹抖动，视觉只靠误差闭环会慢半拍。  
因此底盘板最好给云台板额外提供：

- `chassis_yaw_rate`
- `vehicle_mode`
- `line_follow_mode`
- `alive`

云台用这些量做 `yaw` 前馈补偿，跟踪会更稳。

## 建议的软件架构

项目目录建议按下面方式长期维护：

- `APP/`
  - 任务入口、状态机、系统初始化
- `BSP/`
  - 板级外设封装
- `Module/`
  - 电机驱动、协议、控制器、消息模块
- `Algorithm/`
  - 姿态解算、滤波、数学工具

这个分层思路参考了 `ROBOCON26_R2`，但会保留 F407 工程需要的简化版本。

## 推荐任务架构

建议最终拆成这些任务：

- `imuTask`
  - 读取 BMI088
  - 输出 IMU 状态

- `visionRxTask`
  - 接收树莓派的目标信息
  - 输出视觉目标状态

- `chassisLinkTask`
  - 接收 TI 底盘板状态
  - 输出底盘状态

- `gimbalTask`
  - 执行 `yaw/pitch` 控制主循环

- `stateMachineTask`
  - 管理待机、回零、搜索、跟踪、丢失恢复等模式

- `debugTask`
  - 输出调试串口日志

- `robotTaskInit`
  - 统一创建全部任务

## 推荐消息结构

### `imu_state_t`

- `yaw`
- `pitch`
- `roll`
- `gyro_x`
- `gyro_y`
- `gyro_z`
- `timestamp`

### `vision_target_t`

- `valid`
- `yaw_err_rad`
- `pitch_err_rad`
- `confidence`
- `target_id`
- `timestamp`

### `chassis_state_t`

- `alive`
- `chassis_yaw_rate`
- `line_follow_mode`
- `vehicle_mode`

### `gimbal_ref_t`

- `mode`
- `yaw_ref`
- `pitch_ref`
- `track_enable`

### `gimbal_status_t`

- `yaw_now`
- `pitch_now`
- `yaw_locked`
- `pitch_homed`
- `target_locked`
- `error_code`

## 功能开发顺序

建议按这个顺序推进，不要一上来就做整套自动跟踪：

1. 跑通 `BMI088`
2. 跑通 `QD4310`
3. 跑通 `ZDT Y42`
4. 建立 `robot_init / robot_task / gimbal_task`
5. 建立视觉通信任务
6. 建立底盘通信任务
7. 最后做状态机和整机联调

## GitHub 参考方向

下面这些公开项目和资料，对当前工程思路有直接参考价值：

- [abcuer/2025-NUEDC-E-Ti_CAR](https://github.com/abcuer/2025-NUEDC-E-Ti_CAR)
  - 展示了 `TI 底盘 + 视觉 + 云台执行` 这种分工方式

- [abcuer/target_track_stepper_system](https://github.com/abcuer/target_track_stepper_system)
  - 适合参考“视觉跟踪 + 步进执行 + 任务拆分”

- [DoveOutland/STM32_BMI088_MahonyAHRS](https://github.com/DoveOutland/STM32_BMI088_MahonyAHRS)
  - 适合参考 `BMI088 + Mahony` 姿态解算链路

从这些项目可以归纳出一个很明确的结论：

- 视觉负责输出目标信息，不直接控制电机
- 云台板必须自己做实时闭环
- 真正的难点不在“串口收发”，而在“模式管理、稳定控制、模块协同”

## 机械换算注意事项

`ZDT Y42 + 1:10 行星减速箱` 做 `pitch` 控制时，必须先明确下面几个参数：

- 电机步距角
- 细分数
- 减速比
- 输出轴每度对应多少脉冲

如果这个换算没做清楚，后面的角度控制一定会偏。

## 当前工程状态

当前 `yuntai_v2` 已完成：

- 新建 `CubeMX .ioc`
- 开启 `FreeRTOS`
- 设置 `TIM6` 作为 `HAL timebase`
- 完成基础串口和 `SPI2` 初始化
- 建立新的 Git 仓库并连接远端

当前还未完成：

- 按目标架构重构代码目录
- 迁移 BMI088 驱动
- 迁移 QD4310 驱动
- 迁移步进电机驱动
- 建立消息总线
- 建立状态机

## 本地参考

- 旧引脚与历史驱动参考：`D:\32CUBEMXCODE\Ti\yuntai\yuntai`
- 软件架构参考：`D:\32CUBEMXCODE\2026_RC\ROBOCON26_R2\ROBOCON26_R2`
