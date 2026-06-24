#include "stm32f10x.h"  // Device header
#include "Key.h"

Key_t KeyTable[] =
{
    {GPIOB, GPIO_Pin_1 ,0},
    {GPIOB, GPIO_Pin_11,0},
    {GPIOB, GPIO_Pin_13,1},
    {GPIOB, GPIO_Pin_15,1},
};

volatile uint8_t Key_Flags[KEY_NUM_KEYS] = {0};//按键事件标志数组

void Key_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure;
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;  //Pull-up input mode
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1|GPIO_Pin_11;  //Configure the pins of the key
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB,&GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;  //Pull-down input mode
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13|GPIO_Pin_15;  //Configure the pins of the key
	GPIO_Init(GPIOB,&GPIO_InitStructure);
}

static uint8_t Key_ReadKey(uint8_t KeyID)  //读取按键是否被按下
{
	return GPIO_ReadInputDataBit(KeyTable[KeyID].Port, KeyTable[KeyID].Pin) 
	== KeyTable[KeyID].TriggerLevel ? KEY_PRESSED : KEY_UNPRESSED;
}

//接口类函数都会有的bug:如果在清零前读取了按键事件标志位，那么就会出现读取到的标志位是1，但是在下一次扫描中又被清零了，导致事件丢失。
//最好的做法是设置缓冲区，清零前关闭中断，清零后打开中断
uint8_t Key_CheckEvent(uint8_t KeyID, uint8_t Flag)  //Check if the key event has occurred
{
	if(Key_Flags[KeyID] & Flag)
	{
		if(Flag != Event_Pressing)  //Pressing事件标志会自动在Key_Tick函数中清0
		{
			Key_Flags[KeyID] &= ~Flag;
		}
		return 1;
	}
	return KEY_UNPRESSED;
}

void Key_Tick(void)  //Key scanning function
{
	static uint8_t Count;
	static uint8_t i;	
	static uint8_t CurrRaw[KEY_NUM_KEYS], PrevRaw[KEY_NUM_KEYS]; //按键的状态，0表示没有按键被按下，1表示1号按键被按下，2表示2号按键被按下...其他可以自定义
	static Key_State_t State[KEY_NUM_KEYS];  //按键状态机
	static uint16_t TimeCounter[KEY_NUM_KEYS];  //按键状态转移时间域值计时器

	Count++;

	for(i = 0; i < KEY_NUM_KEYS; i++)
	{
		if(TimeCounter[i] > 0)  //TimerCounter归0认为达到状态转移时间阈值
		{
			TimeCounter[i]--;
		}
	}

	if(Count >= KEY_CYCLE_TIME)
	{
		Count = 0; 

		for(i = 0; i < KEY_NUM_KEYS; i++)
		{
		PrevRaw[i] = CurrRaw[i];
		CurrRaw[i] = Key_ReadKey(i);
		//检测Pressing事件并自动清除Pressing标志位
		if(CurrRaw[i] == KEY_PRESSED)
		{
			Key_Flags[i] |= Event_Pressing;
		}
		else
		{
			Key_Flags[i] &= ~Event_Pressing;  //中断自动清除Pressing标志位，其他标志位会在Key_CheckEvent函数中手动清除
		}
		//检测Down和Up事件
		if(PrevRaw[i] == KEY_UNPRESSED && CurrRaw[i] == KEY_PRESSED)  //按键按下
		{
			Key_Flags[i] |= Event_Down;
		}
		else if(PrevRaw[i] == KEY_PRESSED && CurrRaw[i] == KEY_UNPRESSED)  //按键松开
		{
			Key_Flags[i] |= Event_Up;
		}

		//以下是按键状态机的实现，详见笔记状态转移图
		switch(State[i])
		{
		case Free:
			if(CurrRaw[i] == KEY_PRESSED)
			{
				TimeCounter[i] = KEY_TIME_LONGPRESS;  //设置长按状态转移时间阈值
				State[i] = Pressed;
			}
			break;
		case Pressed:
			if(CurrRaw[i] == KEY_UNPRESSED)
			{
				TimeCounter[i] = KEY_TIME_DOUBLECLICK; //设置双击状态转移时间阈值
				State[i] = Released;
			}
			else if(TimeCounter[i] == 0)	//如果在按下状态下达到长按时间阈值
			{
				TimeCounter[i] = KEY_TIME_REPEAT;  //设置重复状态转移时间阈值
				Key_Flags[i] |= Event_LongPress;  //触发长按事件
				State[i] = LongPressed;
			}
			break;
		case Released:
			if(CurrRaw[i] == KEY_PRESSED)
			{
				TimeCounter[i] = KEY_TIME_DOUBLECLICK_TIMEOUT;  //设置双击超时时间阈值
				State[i] = WaitDoubleClicked;
			}
			else if(TimeCounter[i] == 0)
			{
				Key_Flags[i] |= Event_Click;	//触发单击事件
				State[i] = Free;
			}
			break;
		case LongPressed:
			if(CurrRaw[i] == KEY_UNPRESSED)
			{
				State[i] = Free;
			}
			else if(TimeCounter[i] == 0)
			{
				Key_Flags[i] |= Event_Repeat;  //触发重复事件
				TimeCounter[i] = KEY_TIME_REPEAT;  //重新设置重复状态转移时间阈值
			}
			break;
		case WaitDoubleClicked:
			if(CurrRaw[i] == KEY_UNPRESSED)
			{
				Key_Flags[i] |= Event_DoubleClick;	//触发双击事件
				State[i] = Free;
			}
			else if(TimeCounter[i] == 0)
			{
				State[i] = LongPressed;  //如果在等待双击状态下达到双击超时时间阈值则认为是长按
			}
			break;
		}

		}
	}
}
