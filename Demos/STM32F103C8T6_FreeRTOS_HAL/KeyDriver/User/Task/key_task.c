/*
 * key_task.c
 *
 * 按键任务 — 用数组管理所有按键，统一调度扫描
 *
 * 加按键只需两步:
 *   1. 在 KeyTable[] 中加一行
 *   2. 写一个回调函数
 */

#include "main.h"
#include "FreeRTOS.h"
#include "task.h"
#include "key.h"
#include <string.h>

/* 串口句柄 (CubeMX 生成在 main.c 中) */
extern UART_HandleTypeDef huart1;

/*===========================================================================
 * 回调函数声明
 *===========================================================================*/
static void KEY1_ClickCallback(void);
static void KEY1_DoubleClickCallback(void);
static void KEY1_LongPressCallback(void);
static void KEY1_RepeatCallback(void);

static void KEY2_ClickCallback(void);
static void KEY2_DoubleClickCallback(void);
static void KEY2_LongPressCallback(void);
static void KEY2_RepeatCallback(void);

/*===========================================================================
 * 按键数组 — 所有按键在此统一管理
 *
 * 格式: { GPIO_Port, GPIO_Pin, Click, DoubleClick, LongPress, Repeat, Down, Up }
 * 不关心的回调填 NULL
 *===========================================================================*/
static KeyHandle_TypeDef KeyTable[] =
{
    /* KEY1: PB1, 上拉输入, 低电平有效 */
    {
        .GPIO_Port              = KEY1_GPIO_Port,
        .GPIO_Pin               = KEY1_Pin,
        .ClickCallback          = KEY1_ClickCallback,
        .DoubleClickCallback    = KEY1_DoubleClickCallback,
        .LongPressCallback      = KEY1_LongPressCallback,
        .LongPressRepeatCallback = KEY1_RepeatCallback,
        .DownCallback           = NULL,
        .UpCallback             = NULL,
    },

    /* KEY2: PB11, 上拉输入, 低电平有效 */
    {
        .GPIO_Port              = KEY2_GPIO_Port,
        .GPIO_Pin               = KEY2_Pin,
        .ClickCallback          = KEY2_ClickCallback,
        .DoubleClickCallback    = KEY2_DoubleClickCallback,
        .LongPressCallback      = KEY2_LongPressCallback,
        .LongPressRepeatCallback = KEY2_RepeatCallback,
        .DownCallback           = NULL,
        .UpCallback             = NULL,
    },
};

#define KEY_COUNT  (sizeof(KeyTable) / sizeof(KeyTable[0]))

/*===========================================================================
 * KeyTask — FreeRTOS 按键扫描任务, 每 1ms 扫描一次
 *===========================================================================*/
void KeyTask(void *pvParameters)
{
    (void)pvParameters;

    /* 初始化所有按键 */
    for (uint8_t i = 0; i < KEY_COUNT; i++)
    {
        Key_Init(&KeyTable[i]);
    }

    for (;;)
    {
        /* 遍历扫描所有按键 */
        for (uint8_t i = 0; i < KEY_COUNT; i++)
        {
            Key_Scan(&KeyTable[i]);
        }

        vTaskDelay(pdMS_TO_TICKS(20));   /* 20ms 周期, Key_Scan 内部不做消抖计数 */
    }
}

/*===========================================================================
 * 回调函数 — 按键事件发生时由驱动自动调用
 *
 * 注意: 这些函数运行在 KeyTask 上下文中, 不要做耗时操作.
 *       这里的打印仅用于演示, 实际项目中应通过队列转发.
 *
 * USART1 需要在 CubeMX 中配置 (PA9=TX, PA10=RX, 115200-8-N-1).
 *===========================================================================*/

static void KEY1_ClickCallback(void)
{
    HAL_UART_Transmit(&huart1, (uint8_t *)"[KEY1] Click\r\n", 16, 100);
}

static void KEY1_DoubleClickCallback(void)
{
    HAL_UART_Transmit(&huart1, (uint8_t *)"[KEY1] Double Click\r\n", 22, 100);
}

static void KEY1_LongPressCallback(void)
{
    HAL_UART_Transmit(&huart1, (uint8_t *)"[KEY1] Long Press\r\n", 20, 100);
}

static void KEY1_RepeatCallback(void)
{
    HAL_UART_Transmit(&huart1, (uint8_t *)"[KEY1] Repeat\r\n", 16, 100);
}

static void KEY2_ClickCallback(void)
{
    HAL_UART_Transmit(&huart1, (uint8_t *)"[KEY2] Click\r\n", 16, 100);
}

static void KEY2_DoubleClickCallback(void)
{
    HAL_UART_Transmit(&huart1, (uint8_t *)"[KEY2] Double Click\r\n", 22, 100);
}

static void KEY2_LongPressCallback(void)
{
    HAL_UART_Transmit(&huart1, (uint8_t *)"[KEY2] Long Press\r\n", 20, 100);
}

static void KEY2_RepeatCallback(void)
{
    HAL_UART_Transmit(&huart1, (uint8_t *)"[KEY2] Repeat\r\n", 16, 100);
}
