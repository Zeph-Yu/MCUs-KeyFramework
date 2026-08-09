#include "stm32f10x.h"
#include "Key.h"

/*
 * 按键列表
 * 只需要初始化硬件配置字段，运行时字段由编译器自动归零
 * (Raw=0, LastRaw=0, State=Free, TimeCounter=0, Flags=0)
 */
Key_t KeyTable[KEY_NUM_KEYS] =
{
    /* Port,   Pin,          TriggerLevel */
    { GPIOB,   GPIO_Pin_1,   0 },         // KEY1: PB1,  上拉 → 低电平有效
    { GPIOB,   GPIO_Pin_11,  0 },         // KEY2: PB11, 上拉 → 低电平有效
    { GPIOB,   GPIO_Pin_13,  1 },         // KEY3: PB13, 下拉 → 高电平有效
    { GPIOB,   GPIO_Pin_15,  1 },         // KEY4: PB15, 下拉 → 高电平有效
};

/*===========================================================================
 * Key_Init — 初始化按键 GPIO
 *===========================================================================*/
void Key_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

    /* 上拉输入 — 对应低电平有效的按键 (KEY1, KEY2) */
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_1 | GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    /* 下拉输入 — 对应高电平有效的按键 (KEY3, KEY4) */
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IPD;
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_13 | GPIO_Pin_15;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
}

/*===========================================================================
 * Key_ReadKey — 读取单个按键的当前电平
 *===========================================================================*/
static uint8_t Key_ReadKey(uint8_t KeyID)
{
    return (GPIO_ReadInputDataBit(KeyTable[KeyID].Port, KeyTable[KeyID].Pin)
            == KeyTable[KeyID].TriggerLevel) ? KEY_PRESSED : KEY_UNPRESSED;
}

/*===========================================================================
 * Key_CheckEvent — 检查并清除按键事件标志
 *
 * PS: Event_Pressing 由 Key_Tick 自动维护，调用此函数不会清除它;
 *       其余事件标志在成功匹配后清除。
 *===========================================================================*/
uint8_t Key_CheckEvent(uint8_t KeyID, uint8_t Flag)
{
    if (KeyTable[KeyID].Flags & Flag)
    {
        if (Flag != Event_Pressing)
        {
            KeyTable[KeyID].Flags &= ~Flag;     // 一次性事件: 读取后自动清除
        }
        return 1;
    }
    return KEY_UNPRESSED;
}

/*===========================================================================
 * Key_Tick — 按键扫描状态机 (由定时器 ISR 每 1ms 调用一次)
 *
 * 扫描周期: 20ms (KEY_CYCLE_TIME)
 * 计时器精度: 1ms (TimeCounter 每 1ms 递减一次，阈值以 ms 为单位)
 *===========================================================================*/
void Key_Tick(void)
{
    static uint8_t Count;   // 扫描周期计数器 (仅此一个模块级静态变量)
    uint8_t        i;       // 局部循环变量 (ARM Cortex-M3 栈空间充足)

    Count++;

    /* 每 1ms 递减各按键的计时器 (独立于扫描周期) */
    for (i = 0; i < KEY_NUM_KEYS; i++)
    {
        if (KeyTable[i].TimeCounter > 0)
        {
            KeyTable[i].TimeCounter--;
        }
    }

    /* 达到扫描周期才执行状态机 */
    if (Count >= KEY_CYCLE_TIME)
    {
        Count = 0;

        for (i = 0; i < KEY_NUM_KEYS; i++)
        {
            /* ——— 1. 更新按键电平 (移位 + 读取) ——— */
            KeyTable[i].LastRaw = KeyTable[i].Raw;
            KeyTable[i].Raw     = Key_ReadKey(i);

            /* ——— 2. Pressing 事件 (持续置位 / 自动清除) ——— */
            if (KeyTable[i].Raw == KEY_PRESSED)
            {
                KeyTable[i].Flags |= Event_Pressing;
            }
            else
            {
                KeyTable[i].Flags &= ~Event_Pressing;
            }

            /* ——— 3. Down / Up 边沿事件 ——— */
            if (KeyTable[i].LastRaw == KEY_UNPRESSED && KeyTable[i].Raw == KEY_PRESSED)
            {
                KeyTable[i].Flags |= Event_Down;
            }
            else if (KeyTable[i].LastRaw == KEY_PRESSED && KeyTable[i].Raw == KEY_UNPRESSED)
            {
                KeyTable[i].Flags |= Event_Up;
            }

            /* ——— 4. 按键状态机 ——— */
            switch (KeyTable[i].State)
            {
            case Free:
                if (KeyTable[i].Raw == KEY_PRESSED)
                {
                    KeyTable[i].TimeCounter = KEY_TIME_LONGPRESS;
                    KeyTable[i].State       = Pressed;
                }
                break;

            case Pressed:
                if (KeyTable[i].Raw == KEY_UNPRESSED)
                {
                    /* 短按松开 → 进入 Released，等待判断是单击还是双击的第一击 */
                    KeyTable[i].TimeCounter = KEY_TIME_DOUBLECLICK;
                    KeyTable[i].State       = Released;
                }
                else if (KeyTable[i].TimeCounter == 0)
                {
                    /* 按下时间超过长按阈值 → 触发长按，进入连发模式 */
                    KeyTable[i].TimeCounter = KEY_TIME_REPEAT;
                    KeyTable[i].Flags      |= Event_LongPress;
                    KeyTable[i].State       = LongPressed;
                }
                break;

            case Released:
                if (KeyTable[i].Raw == KEY_PRESSED)
                {
                    /* 双击窗口内再次按下 → 等待第二次松开 */
                    KeyTable[i].TimeCounter = KEY_TIME_DOUBLECLICK_TIMEOUT;
                    KeyTable[i].State       = WaitDoubleClicked;
                }
                else if (KeyTable[i].TimeCounter == 0)
                {
                    /* 双击窗口超时未再按下 → 确认为单击 */
                    KeyTable[i].Flags |= Event_Click;
                    KeyTable[i].State  = Free;
                }
                break;

            case LongPressed:
                if (KeyTable[i].Raw == KEY_UNPRESSED)
                {
                    /* 长按松开 → 回到空闲 */
                    KeyTable[i].State = Free;
                }
                else if (KeyTable[i].TimeCounter == 0)
                {
                    /* 长按保持 → 周期性触发 Repeat 事件 */
                    KeyTable[i].Flags      |= Event_Repeat;
                    KeyTable[i].TimeCounter = KEY_TIME_REPEAT;
                }
                break;

            case WaitDoubleClicked:
                if (KeyTable[i].Raw == KEY_UNPRESSED)
                {
                    /* 第二次松开 → 确认为双击 */
                    KeyTable[i].Flags |= Event_DoubleClick;
                    KeyTable[i].State  = Free;
                }
                else if (KeyTable[i].TimeCounter == 0)
                {
                    /* 第二次按下后超时未松开 → 转为长按 */
                    KeyTable[i].State = LongPressed;
                }
                break;
            }
        }
    }
}
