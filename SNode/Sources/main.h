/*
 * main.h
 *
 *  Created on: Feb 22, 2014
 *      Author: X550L-User1
 */

#ifndef MAIN_H_
#define MAIN_H_

#include <stdint.h>
#include "serial.h"

typedef struct 
{
	uint8_t Cmd;
	uint8_t Data[MAX_RX_SIZE];
	uint8_t Length;
} ReceivedPacket;

#define QUEUE_SIZE 		8
#define QUEUE_SIZE_MASK	(QUEUE_SIZE - 1)

void sNodeMain(void);

#endif /* MAIN_H_ */
