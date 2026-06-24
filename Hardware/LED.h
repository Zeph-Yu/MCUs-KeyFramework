#ifndef __LED_H
#define __LED_H

extern uint8_t LEDMode;

void LED_Init(void);
void LED_ON(void);
void LED_OFF(void);
void LED_Tick(void);

#endif
