# MCUs-KeyFramework

基于状态机的非阻塞式单片机按键驱动框架。

项目参考江协科技 STM32 教程中的按键处理思路，在此基础上进行了重构与扩展，支持：

* 单击（Click）
* 双击（Double Click）
* 长按（Long Press）
* 连发（Repeat）
* 按下（Down）
* 松开（Up）
* 持续按下（Pressing）

设计目标：

* 非阻塞式运行
* 支持多按键管理
* 易于移植
* 适用于裸机和 RTOS 环境

---

当前支持：

| 平台 | 环境 | 驱动目录 |
|---|---|---|
| STM32F103 | 标准库 裸机 | `Drivers/STM32F1_StdPeriph/` |
| STM32F103 | HAL 裸机 | `Drivers/STM32F1_HAL/` |
| STM32F103 | HAL + FreeRTOS | `Drivers/STM32F1_HAL_FreeRTOS/` |
| MSPM0G3507 | DriverLib 裸机 | `Drivers/TI_MSPM0G3507/` |

## 工程示例

| 目录 | 说明 |
|---|---|
| `Demos/STM32F1_StdPeriph_Demo/` | STM32F103 标准库 + Keil |
| `Demos/STM32F103C8T6_HAL/` | STM32F103C8T6 HAL 裸机 + CubeIDE |
| `Demos/STM32F103C8T6_FreeRTOS_HAL/` | STM32F103C8T6 HAL + FreeRTOS + CubeIDE |
| `Demos/TI_MSPM0G3507_Demo/` | MSPM0G3507 + CCS Theia |
