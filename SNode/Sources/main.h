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

#include "plugin.h"

typedef struct 
{
	uint8_t Cmd;
	uint8_t Data[MAX_RX_SIZE];
	uint8_t Length;
} ReceivedPacket;

#define QUEUE_SIZE 		8
#define QUEUE_SIZE_MASK	(QUEUE_SIZE - 1)

typedef struct
{
#define SETTINGS_MAGIC_NUMBER	0x48C6FAF0 
	
	uint32_t MagicNumber;
	uint32_t HashCode;
	uint8_t AesKey[16];
	uint8_t AesIV[16];
	
	uint32_t NodeType;
	
	uint16_t Address;
	uint8_t Channel;
} UserSettings;

void sNodeMain(void);

extern bool FlashOperationComplete;

#endif /* MAIN_H_ */
