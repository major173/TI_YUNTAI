# yuntai_v2

基于 `STM32F407` 的云台重构工程。

这个工程的目标不是在旧 `yuntai` 工程上继续打补丁，而是：

- 从新的 `CubeMX .ioc` 重新开始
- 按 `APP / BSP / Module / Algorithm` 分层
- 保留旧工程作为驱动和协议参考
- 逐步迁移 `BMI088`、`QD4310`、`Emm_V5`，而不是一次性搬空

## 目录约定

- `APP/`：任务、状态机、系统初始化
- `BSP/`：板级外设包装，尽量薄
- `Module/`：设备驱动、控制器、协议、消息总线
- `Algorithm/`：姿态解算、滤波、数学工具
- `doc/`：配置说明、迁移计划

## 推荐第一阶段目标

1. 新建 `yuntai_v2.ioc`
2. 只开最小外设：
   - `SPI2` for `BMI088`
   - `USART1` for `Emm_V5`
   - `USART2` for `QD4310`
   - `USART3` for debug
   - `FreeRTOS`
3. 跑通三个最小验证：
   - BMI088 读数正常
   - QD4310 使能和速度控制正常
   - 步进驱动使能、回零或位置控制正常
4. 再接入：
   - 树莓派视觉串口
   - TI 底盘串口
   - 云台状态机

## 参考来源

- 旧工程：`D:\\32CUBEMXCODE\\Ti\\yuntai\\yuntai`
- 架构参考：`D:\\32CUBEMXCODE\\2026_RC\\ROBOCON26_R2\\ROBOCON26_R2`
