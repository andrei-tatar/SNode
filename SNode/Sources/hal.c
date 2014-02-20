/*
 * hal.c
 *
 *  Created on: Apr 15, 2013
 *      Author: User
 */

#include "hal.h"

volatile uint32_t time = 0;

void delay_ms(uint16_t msec)
{
	register uint32_t endTime = time + msec;
	while (endTime > time);
}

#define _NOP() asm volatile("nop")

void delay_us(uint32_t usec)
{
	while (usec--)
	{
		_NOP();_NOP();_NOP();_NOP();_NOP();_NOP();_NOP();_NOP();
		_NOP();_NOP();_NOP();_NOP();_NOP();_NOP();_NOP();_NOP();_NOP();
	}
}

PE_ISR(sysTickTimerInterrupt)
{
	time += 1;
}
