/*
 * hal.h
 *
 *  Created on: Apr 15, 2013
 *      Author: User
 */

#ifndef HAL_H_
#define HAL_H_

#include <stdint.h>
#include "PTD.h"
#include "Cpu.h"
#include "serial.h"
#include "spi.h"
#include "RF24.h"
#include "RF24Network.h"
#include "aes.h"

#define CSN_PIN		(1 << 4)
#define CE_PIN		(1 << 5)
#define LED1		(1 << 2)
#define LED2		(1 << 3)

// Definitions for selecting and enabling the radio
#define CSN_HIGH()	PTD_BASE_PTR->PSOR = CSN_PIN
#define CSN_LOW()	PTD_BASE_PTR->PCOR = CSN_PIN

#define CE_HIGH()	PTD_BASE_PTR->PSOR = CE_PIN
#define CE_LOW()	PTD_BASE_PTR->PCOR = CE_PIN

#define LED_ON(x)	PTC_BASE_PTR->PSOR = x
#define LED_OFF(x)	PTC_BASE_PTR->PCOR = x
#define LED_TOGGLE(x) PTC_BASE_PTR->PTOR = x

void delay_ms(uint16_t msec);
void delay_us(uint32_t usec);

extern volatile uint32_t time;

static inline uint32_t getTime()
{
	return time;
}

enum InterruptSource {
	InterruptI2C_0 = 24,
	InterruptI2C_1 = 25,
	InterruptSPI_0 = 26,
	InterruptSPI_1 = 27,
	InterruptUART_0 = 28,
	InterruptUART_1 = 29,
	InterruptUART_2 = 30,
	InterruptADC_0 = 31,
	InterruptCMP_0 = 32,
	InterruptTPM_0 = 33,
	InterruptTPM_1 = 34,
	InterruptTPM_2 = 35,
	InterruptRTC_Alarm = 36,
	InterruptRTC_Seconds = 37,
	InterruptPIT = 38,
	InterruptDAC_0 = 41,
	InterruptPORTA = 46,
	InterruptPORTD = 47,
};

void interruptInit(void);
void interruptChange(InterruptSource source, void(*newAddress)());

#endif /* HAL_H_ */
