# CubeMX 新工程启动清单

## 1. 新建工程

在 `STM32CubeMX` 里：

1. `New Project`
2. 芯片选 `STM32F407`
3. 工程名建议填：`yuntai_v2`
4. 工程路径建议填：`D:\32CUBEMXCODE\Ti\yuntai\yuntai_v2`
5. Toolchain 选你当前使用的 `CMake`/`STM32CubeIDE` 兼容方案

不要覆盖旧的 `yuntai` 工程。

## 2. Pinout 最小配置

先只配下面这些，别一开始把所有设想都堆进去。

### SYS

- Debug: `Serial Wire`

### SPI2 for BMI088

按旧工程保持：

- `PB13` -> `SPI2_SCK`
- `PC2` -> `SPI2_MISO`
- `PC3` -> `SPI2_MOSI`
- `PD0` -> BMI088 accel CS, 普通 `GPIO_Output`
- `PD1` -> BMI088 gyro CS, 普通 `GPIO_Output`

### USART1 for 步进驱动

- `PA9` -> `USART1_TX`
- `PA10` -> `USART1_RX`

建议先保留 DMA RX/TX，和旧工程一致。

### USART2 for QD4310

- `PD5` -> `USART2_TX`
- `PD6` -> `USART2_RX`

### USART3 for 调试

- `PB10` -> `USART3_TX`
- `PB11` -> `USART3_RX`

### FreeRTOS

先开 `CMSIS_V2`

第一版只保留 1 个默认任务也可以，后面再拆任务。

## 3. Clock

用 F407 常规稳定配置即可，优先保证：

- 串口 115200 正常
- SPI2 正常
- FreeRTOS Tick 正常

如果你没有明确外部晶振约束，先照旧工程或常规 F407 配法，不要在第一天花太多时间打磨时钟。

## 4. NVIC / DMA

建议第一版至少保留：

- `USART1 IRQ`
- `USART2 IRQ`
- `USART3 IRQ`
- `SPI2 IRQ` 如果你的 BMI088 方案需要
- `DMA` for `USART1 RX/TX`

## 5. 生成代码后立刻检查

生成后优先确认这些文件存在：

- `Core/`
- `Drivers/`
- `Middlewares/`
- `startup_stm32f407xx.s`
- `STM32F407XX_FLASH.ld`
- 新的 `.ioc`

然后再把本目录外的自定义结构补回去：

- `APP/`
- `BSP/`
- `Module/`
- `Algorithm/`

## 6. 第一批迁移模块

顺序建议如下：

1. `BMI088`
2. `QD4310`
3. `Emm_V5`
4. `MahonyAHRS`
5. `logger/debug`

不要先迁状态机，不要先迁视觉。

## 7. 第一版任务划分建议

先做最小版：

- `robotInit`
- `robotTaskInit`
- `imuTask`
- `gimbalTask`
- `debugTask`

第二版再加：

- `visionRxTask`
- `chassisLinkTask`
- `stateMachineTask`

## 8. 明确不要做的事

- 不要在新工程里继续使用 `User/` 作为唯一业务目录
- 不要把所有逻辑继续塞进一个 `yuntai.c`
- 不要一开始就接树莓派和 TI 两路联调
- 不要覆盖旧工程
