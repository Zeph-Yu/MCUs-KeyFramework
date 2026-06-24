#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "OLED.h"
#include "Key.h"
#include "Timer.h"

uint16_t Num1, Num2;

int main(void)	
{
	OLED_Init();
	Key_Init();
	Timer_Init();
	
	OLED_Clear();
	OLED_ShowString(1, 1, "Num1:");
	OLED_ShowString(2, 1, "Num2:");

	while(1)
	{
		// /*示例1：按下PB1，Num1置1，松开后置0*/
		// if(Key_CheckEvent(KEY1, Event_Hold))
		// {
		// 	Num1 = 1; 
		// }
		// else
		// {
		// 	Num1 = 0;
		// }

		// /*示例2：按下按键瞬间Num1++，松开按键瞬间Num2++*/
		// if(Key_CheckEvent(KEY1, Event_Down))
		// {
		// 	Num1++;
		// }
		// if(Key_CheckEvent(KEY1, Event_Up))
		// {
		// 	Num2++;
		// }

		// /*示例3：单击Num1++、双击Num1+100，长按Num1清零*/
		// if(Key_CheckEvent(KEY1, Event_Click))
		// {
		// 	Num1++;
		// }
		// if(Key_CheckEvent(KEY1, Event_DoubleClick))
		// {
		// 	Num1 += 100;
		// }
		// if(Key_CheckEvent(KEY1, Event_LongPress))
		// {
		// 	Num1 = 0;
		// }

		/*示例4：单击Num1++，长按Num1快速++*/
		if(Key_CheckEvent(KEY1, Event_Click) || Key_CheckEvent(KEY1, Event_Repeat))
		{
			Num1++;
		}
		if(Key_CheckEvent(KEY2, Event_Click) || Key_CheckEvent(KEY2, Event_Repeat))
		{
			Num1--;
		}
		if(Key_CheckEvent(KEY3,Event_Click))
		{
			Num1 = 0;
		}
		if(Key_CheckEvent(KEY4,Event_LongPress))
		{
			Num1 = 9999;
		}

		OLED_ShowNum(1, 6, Num1, 5);
		OLED_ShowNum(2, 6, Num2, 5);
	}
}

//定时器2中断服务函数
void TIM2_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM2,TIM_IT_Update)==SET)
	{
		Key_Tick();
		TIM_ClearITPendingBit(TIM2,TIM_IT_Update);
	}
}
