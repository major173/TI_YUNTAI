# 重构计划

## 目标架构

参考 `ROBOCON26_R2`，但保留 F407 项目的必要简化。

### 目录

- `APP/robot_init/`
- `APP/robot_task/`
- `APP/gimbal_task/`
- `APP/vision_task/`
- `APP/chassis_link_task/`
- `APP/debug_task/`
- `APP/state_machine_task/`
- `BSP/bsp_uart/`
- `BSP/bsp_spi/`
- `BSP/bsp_time/`
- `Module/imu/`
- `Module/motor/`
- `Module/stepper/`
- `Module/controller/`
- `Module/protocol/`
- `Module/topics/`
- `Module/logger/`
- `Algorithm/`

## 迁移映射

旧工程到新工程的建议迁移关系：

- `User/Device/BMI088/*` -> `Module/imu/bmi088/*`
- `User/Device/QD4310.*` -> `Module/motor/qd4310_uart.*`
- `User/Device/Emm_V5.*` -> `Module/stepper/emm_v5.*`
- `Algorithm/MahonyAHRS.*` -> `Algorithm/`
- `User/App/yuntai.*` -> 拆到 `APP/gimbal_task/` 和 `Module/controller/`

## 第一阶段完成标准

- BMI088 稳定读数
- QD4310 能使能、停机、给速度
- 步进驱动能使能、停机、位置控制
- 代码目录已经按新结构组织

## 第二阶段完成标准

- 完成树莓派串口协议
- 完成 TI 底盘串口协议
- 完成云台模式机
- 实现 yaw 保持 + 视觉跟踪 + 丢目标搜索
