#include "stm32f10x.h"      // Device header

uint8_t LEDMode = 0;
static uint16_t LED_TickCounter = 0;

void LED_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin =GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&GPIO_InitStructure);
	
	GPIO_SetBits(GPIOA,GPIO_Pin_2);
}

void LED_ON(void)
{
	GPIO_ResetBits(GPIOA,GPIO_Pin_2);
}

void LED_OFF(void)
{
	GPIO_SetBits(GPIOA,GPIO_Pin_2);
}

void LED_Tick()
{
	if(LEDMode == 0)
	{
		LED_OFF();
	}
	else if(LEDMode == 1)
	{
		LED_ON();	
	}
	else if(LEDMode == 2)
	{
		LED_TickCounter++;
		LED_TickCounter %= 1000;
		if(LED_TickCounter < 500)
		{
			LED_ON();
		}
		else
		{
			LED_OFF();
		}
	}
	else if(LEDMode == 3)
	{
		LED_TickCounter++;
		LED_TickCounter %= 1000;
		if(LED_TickCounter < 900)
		{
			LED_ON();
		}
		else
		{
			LED_OFF();
		}
	}
	else if(LEDMode == 4)
	{
		LED_TickCounter++;
		LED_TickCounter %= 1000;
		if(LED_TickCounter < 100)
		{
			LED_ON();
		}
		else
		{
			LED_OFF();
		}
	}
}
