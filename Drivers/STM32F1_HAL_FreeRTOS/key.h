/*
 * key.h
 *
 * 按键驱动 — 基于状态机的多功能按键检测
 *
 * 用法:
 *   1. 为每个按键定义一个 KeyHandle_TypeDef, 填入 GPIO 端口/引脚 和回调函数
 *   2. Key_Init() 初始化
 *   3. 每 1ms 调用 Key_Scan() 扫描一个按键
 *   4. 事件触发时自动调用对应的回调函数
 *
 * 支持的事件回调:
 *   ClickCallback           — 单击
 *   DoubleClickCallback     — 双击
 *   LongPressCallback       — 长按 (首次触发)
 *   LongPressRepeatCallback — 长按连发 (周期性触发)
 *   DownCallback            — 按下瞬间 (可选)
 *   UpCallback              — 松开瞬间 (可选)
 */

#ifndef __KEY_H
#define __KEY_H

#include "main.h"

/* 状态机时间阈值 (ms, 可根据需要修改) */
#define KEY_TIME_LONG_PRESS            1000U
#define KEY_TIME_DOUBLE_CLICK          180U
#define KEY_TIME_DOUBLE_CLICK_TIMEOUT  500U
#define KEY_TIME_REPEAT                200U   /* 长按连发间隔, 首次 200ms 后触发 */
#define KEY_SCAN_PERIOD_MS             20U

/* 状态机内部状态 */
typedef enum {
    KEY_STATE_FREE,
    KEY_STATE_PRESSED,
    KEY_STATE_RELEASED,
    KEY_STATE_LONG_PRESSED,
    KEY_STATE_WAIT_DOUBLE_CLICK,
} KeyState_t;

/* 按键 Handle — 每个按键一个实例 */
typedef struct {
    /* ——— 硬件配置 (用户填写) ——— */
    GPIO_TypeDef *GPIO_Port;
    uint16_t      GPIO_Pin;

    /* ——— 事件回调 (用户填写, NULL = 不关心此事件) ——— */
    void (*ClickCallback)(void);
    void (*DoubleClickCallback)(void);
    void (*LongPressCallback)(void);
    void (*LongPressRepeatCallback)(void);
    void (*DownCallback)(void);
    void (*UpCallback)(void);

    /* ——— 运行时状态 (驱动内部维护, 用户勿改) ——— */
    uint8_t       Raw;
    uint8_t       LastRaw;
    uint8_t       State;
    uint16_t      TimeCounter;
} KeyHandle_TypeDef;

/* API */
void Key_Init(KeyHandle_TypeDef *key);
void Key_Scan(KeyHandle_TypeDef *key);

#endif /* __KEY_H */
