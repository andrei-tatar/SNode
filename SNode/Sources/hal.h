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
#define LED1		(1 << 6)
#define LED2		(1 << 7)

// Definitions for selecting and enabling the radio
#define CSN_HIGH()	PTD_BASE_PTR->PSOR = CSN_PIN
#define CSN_LOW()	PTD_BASE_PTR->PCOR = CSN_PIN

#define CE_HIGH()	PTD_BASE_PTR->PSOR = CE_PIN
#define CE_LOW()	PTD_BASE_PTR->PCOR = CE_PIN

#define LED_ON(x)	//PTD_BASE_PTR->PSOR = x
#define LED_OFF(x)	//PTD_BASE_PTR->PCOR = x

void delay_ms(uint16_t msec);
void delay_us(uint32_t usec);

extern volatile uint32_t time;

static inline uint32_t getTime()
{
	return time;
}

#endif /* HAL_H_ */
