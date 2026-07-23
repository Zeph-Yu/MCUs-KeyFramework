#ifndef KEY_H
#define KEY_H

#include "ti_msp_dl_config.h"

/* 扫描周期 (ms) */
#define KEY_CYCLE_TIME              20

/* 按键电平读取结果 */
#define KEY_PRESSED                 1
#define KEY_UNPRESSED               0

/* 状态转移时间阈值 (ms) */
#define KEY_TIME_LONGPRESS          1000    // 长按时间阈值
#define KEY_TIME_DOUBLECLICK        180     // 双击时间阈值 (两次按下间隔)
#define KEY_TIME_REPEAT             100     // 连发间隔阈值
#define KEY_TIME_DOUBLECLICK_TIMEOUT 500    // 双击超时 (等待第二次按下)

/* 按键数量与索引 */
#define KEY_NUM_KEYS                2
#define PA18                        0
#define PB21                        1

/* 按键事件标志 (可组合) */
typedef enum
{
    Event_None       = 0,
    Event_Pressing   = (1U << 0),  // 正在按下 (持续置位)
    Event_Down       = (1U << 1),  // 按下瞬间
    Event_Up         = (1U << 2),  // 松开瞬间
    Event_Click      = (1U << 3),  // 单击
    Event_DoubleClick = (1U << 4), // 双击
    Event_LongPress  = (1U << 5),  // 长按
    Event_Repeat     = (1U << 6),  // 连发 (长按后周期性触发)
} Key_Event;

/* 状态机状态 */
typedef enum
{
    Free,               // 空闲
    Pressed,            // 已按下 (等待松开或长按超时)
    Released,           // 已松开 (等待第二次按下或单击超时)
    LongPressed,        // 长按中 (等待松开或连发间隔)
    WaitDoubleClicked,  // 等待双击释放
} Key_State_t;

/*
 * 按键控制块 — 所有参数集成到一个结构体中
 * 每增加一个按键只需在 KeyTable[] 数组中添加一行
 */
typedef struct
{
    /* ——— 硬件配置 (初始化时设定) ——— */
    GPIO_Regs    *Port;          // GPIO 端口
    uint32_t      Pins;          // GPIO 引脚 (位掩码)
    uint32_t      Iomux;         // IOMUX 索引 (用于 GPIO 初始化)
    uint8_t       TriggerLevel;  // 有效电平: 0=低电平有效, 1=高电平有效

    /* ——— 运行时状态 (Key_Tick 内维护) ——— */
    uint8_t       Raw;           // 当前消抖后的引脚电平
    uint8_t       LastRaw;       // 上一周期的引脚电平
    uint8_t       State;         // 状态机当前状态 (Key_State_t)
    uint16_t      TimeCounter;   // 状态转移计时器 (递减计数, 单位 ms)

    /* ——— 事件标志 (ISR 写入, 主循环读取/清除) ——— */
    volatile uint8_t Flags;      // Key_Event 标志位集合
} Key_t;

/* 按键控制块数组 (定义在 .c 文件中) */
extern Key_t KeyTable[KEY_NUM_KEYS];

/* API */
uint8_t Key_CheckEvent(uint8_t KeyID, uint8_t Flag);
void    Key_Tick(void);

#endif // KEY_H
