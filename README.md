# MCUs-KeyFramework

基于状态机的非阻塞式单片机按键框架。

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

* STM32F103（标准库）

计划支持：

* MSPM0G3507
* STM32 HAL
* FreeRTOS 环境
