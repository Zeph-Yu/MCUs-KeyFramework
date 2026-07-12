#ifndef __Key_H
#define __Key_H

#define KEY_CYCLE_TIME			20  //按键扫描周期，单位：ms

#define KEY_PRESSED 			1
#define KEY_UNPRESSED 			0

#define KEY_TIME_LONGPRESS 		        1000  //长按时间阈值，单位：ms
#define KEY_TIME_DOUBLECLICK 	        180  //双击时间阈值，单位：ms
#define KEY_TIME_REPEAT 		        100  //重复时间阈值，单位：ms
#define KEY_TIME_DOUBLECLICK_TIMEOUT    500  //双击超时时间阈值，单位：ms

#define KEY_NUM_KEYS				4  //按键数量
#define KEY1                        0
#define KEY2                        1
#define KEY3                        2
#define KEY4                        3

typedef enum
{
    Event_None = 0,
    Event_Pressing = (1U << 0),
    Event_Down = (1U << 1),
    Event_Up = (1U << 2),
    Event_Click = (1U << 3),
    Event_DoubleClick = (1U << 4),
    Event_LongPress = (1U << 5),
    Event_Repeat = (1U << 6),
}Key_Event;

typedef enum
{
    Free,                   //空闲
    Pressed,                //按键已按下
    Released,               //按键已松开
    LongPressed,            //按键已长按
    WaitDoubleClicked,      //等待双击释放
}Key_State_t;

typedef struct
{
    GPIO_TypeDef* Port;
    uint16_t Pin;
    uint8_t TriggerLevel;
}Key_t;

extern volatile uint8_t Key_Flags[KEY_NUM_KEYS];

void Key_Init(void);
uint8_t Key_CheckEvent(uint8_t KeyID, uint8_t Flag);
void Key_Tick(void);

#endif
