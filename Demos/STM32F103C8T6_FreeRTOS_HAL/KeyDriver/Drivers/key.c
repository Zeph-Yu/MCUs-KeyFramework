/*
 * key.c
 *
 * 按键驱动实现 — 5 状态状态机
 *
 * Key_Scan() 每 20ms 调用一次 (由 KeyTask 控制),
 * 内部不做消抖计数, 每次调用都完整执行 GPIO 读取 + 状态机.
 * TimeCounter 每次递减 KEY_SCAN_PERIOD_MS (20ms).
 */

#include "key.h"

/*===========================================================================
 * Key_Init
 *===========================================================================*/
void Key_Init(KeyHandle_TypeDef *key)
{
    key->Raw         = 0;
    key->LastRaw     = 0;
    key->State       = KEY_STATE_FREE;
    key->TimeCounter = 0;
}

/*===========================================================================
 * Key_Scan — 每 20ms 调用一次 (由 KeyTask 的 vTaskDelay 控制周期)
 *===========================================================================*/
void Key_Scan(KeyHandle_TypeDef *key)
{
    uint8_t pressed;

    /* 递减计时器, 每次 20ms */
    if (key->TimeCounter > 0)
    {
        if (key->TimeCounter > KEY_SCAN_PERIOD_MS)
        {
            key->TimeCounter -= KEY_SCAN_PERIOD_MS;
        }
        else
        {
            key->TimeCounter = 0;
        }
    }

    /* ——— 1. 读取 GPIO ——— */
    key->LastRaw = key->Raw;
    key->Raw     = (HAL_GPIO_ReadPin(key->GPIO_Port, key->GPIO_Pin) == GPIO_PIN_RESET) ? 1U : 0U;
    pressed      = key->Raw;  /* 1 = 按下 (低电平有效) */

    /* ——— 2. Down / Up 边沿 ——— */
    if (key->LastRaw == 0 && key->Raw == 1)
    {
        if (key->DownCallback) key->DownCallback();
    }
    else if (key->LastRaw == 1 && key->Raw == 0)
    {
        if (key->UpCallback) key->UpCallback();
    }

    /* ——— 3. 状态机 ——— */
    switch (key->State)
    {
    case KEY_STATE_FREE:
        if (pressed)
        {
            key->TimeCounter = KEY_TIME_LONG_PRESS;
            key->State       = KEY_STATE_PRESSED;
        }
        break;

    case KEY_STATE_PRESSED:
        if (!pressed)
        {
            /* 短按松开 → 等待双击判定 */
            key->TimeCounter = KEY_TIME_DOUBLE_CLICK;
            key->State       = KEY_STATE_RELEASED;
        }
        else if (key->TimeCounter == 0)
        {
            /* 长按超时 */
            if (key->LongPressCallback) key->LongPressCallback();
            key->TimeCounter = KEY_TIME_REPEAT;
            key->State       = KEY_STATE_LONG_PRESSED;
        }
        break;

    case KEY_STATE_RELEASED:
        if (pressed)
        {
            /* 双击窗口内再次按下 */
            key->TimeCounter = KEY_TIME_DOUBLE_CLICK_TIMEOUT;
            key->State       = KEY_STATE_WAIT_DOUBLE_CLICK;
        }
        else if (key->TimeCounter == 0)
        {
            /* 双击窗口超时 → 单击确认 */
            if (key->ClickCallback) key->ClickCallback();
            key->State = KEY_STATE_FREE;
        }
        break;

    case KEY_STATE_LONG_PRESSED:
        if (!pressed)
        {
            /* 长按松开 */
            key->State = KEY_STATE_FREE;
        }
        else if (key->TimeCounter == 0)
        {
            /* 长按连发 */
            if (key->LongPressRepeatCallback) key->LongPressRepeatCallback();
            key->TimeCounter = KEY_TIME_REPEAT;
        }
        break;

    case KEY_STATE_WAIT_DOUBLE_CLICK:
        if (!pressed)
        {
            /* 第二次松开 → 双击确认 */
            if (key->DoubleClickCallback) key->DoubleClickCallback();
            key->State = KEY_STATE_FREE;
        }
        else if (key->TimeCounter == 0)
        {
            /* 第二次按下超时 → 转为长按 */
            key->State = KEY_STATE_LONG_PRESSED;
        }
        break;
    }
}
