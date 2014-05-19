/*
 * hal.c
 *
 *  Created on: Apr 15, 2013
 *      Author: User
 */

#include "hal.h"

/*static uint32_t interruptVector[47] __attribute__ ((aligned (512)));

void interruptInit(void)
{
	void* address = (void*)0;
	memcpy(interruptVector, address, sizeof(interruptVector));
	SCB_VTOR = (uint32_t)interruptVector;
}

void interruptChange(InterruptSource source, void(*newAddress)())
{
	interruptVector[source] = (uint32_t)newAddress;
}*/

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
