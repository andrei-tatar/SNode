/*
 * serial.c
 *
 *  Created on: Jan 25, 2014
 *      Author: Andrei
 */

#include "serial.h"

#define BUF_SIZE	512
#define BUF_MASK 	(BUF_SIZE - 1)

static uint8_t txBuffer[BUF_SIZE];
static volatile uint16_t txReadPos = 0;
static uint16_t txWritePos = 0;
static volatile uint16_t txCount = 0;

static uint8_t rxBuffer[MAX_RX_SIZE];

static inline int min(int a, int b)
{
	return a < b ? a : b;
}

const uint8_t packetStart[] = { 0x91, 0xEB, 0xFA, 0x58 };

#define STATE_IDLE			0x01
#define STATE_LENGTH_LSB	0x02
#define STATE_LENGTH_MSB	0x03
#define STATE_DATA			0x04
#define STATE_CHECKSUM		0x05

PE_ISR(uartInterrupt)
{
	uint8_t int_status = UART1_BASE_PTR->S1; 
	
	if (int_status & UART_S1_TDRE_MASK)
	{
		if (txCount)
		{
			//send the next byte
			UART1_BASE_PTR->D = txBuffer[txReadPos];
			txCount--;
			txReadPos = (txReadPos + 1) & BUF_MASK;
		}
		else
		{
			//disable tx empty interrupt 
			UART1_C2 &= ~UART_C2_TIE_MASK;
		}
	}
	
	if (int_status & UART_S1_RDRF_MASK)
	{
		static uint8_t state = STATE_IDLE;
		static uint8_t headerOffset = 0, checksum;
		static int length, dataOffset;
		uint8_t cByte = UART1_BASE_PTR->D;
		
		if (cByte == packetStart[headerOffset])
		{
			if (++headerOffset == 4)
			{
				state = STATE_LENGTH_LSB;
				checksum = 0;
				headerOffset = 0;
				return;
			}
		}
		else
			headerOffset = 0;
		
		switch (state)
		{
			case STATE_IDLE:
				break;
				
			case STATE_LENGTH_LSB:
				length = cByte;
				checksum ^= cByte;
				state = STATE_LENGTH_MSB;
				break;

			case STATE_LENGTH_MSB:
				length |= cByte << 8;
				if (length > MAX_RX_SIZE)
				{
					state = STATE_IDLE;
					headerOffset = 0;
				}

				checksum ^= cByte;
				state = STATE_DATA;
				dataOffset = 0;
				break;

			case STATE_DATA:
				rxBuffer[dataOffset++] = cByte;
				checksum ^= cByte;
				if (dataOffset == length)
					state = STATE_CHECKSUM;
				break;

			case STATE_CHECKSUM:
				if (cByte == checksum)
					packetReceived(rxBuffer[0], (uint8_t*)&rxBuffer[1], length - 1);
				state = STATE_IDLE;
				headerOffset = 0;
				break;
		}
	}
}

static void serialData(const uint8_t *buffer, int length)
{
	while (length)
	{
		__DI();
		int freeSpace = BUF_SIZE - txCount;
		__EI();
		if (freeSpace)
		{
			int toCopy = min(length, BUF_SIZE - txWritePos);
			if (freeSpace < toCopy) toCopy = freeSpace;
			
			memcpy(&txBuffer[txWritePos], buffer, toCopy);
			length -= toCopy;
			txWritePos = (txWritePos + toCopy) & BUF_MASK;
			buffer += toCopy;

			__DI();
			txCount += toCopy;
			__EI();
		}
		
		//make sure tx empty interrupt is enabled
		UART1_C2 |= UART_C2_TIE_MASK;
	}
}
void flushTx()
{
	while (txCount) ;
}

void serialSendPacket(uint8_t cmd, const uint8_t *data, int length)
{
	length += 1;
	
	int totalLength = sizeof(packetStart) + 2 + length + 1;
	int offset = sizeof(packetStart);
	uint8_t aux[totalLength];
	
	memcpy(aux, packetStart, offset);
	aux[offset++] = length & 0xFF;
	aux[offset++] = (length >> 8) & 0xFF;
	aux[offset++] = cmd;
	memcpy(&aux[offset], data, length - 1);
	offset += length - 1;
	
	uint8_t checksum = 0;
	for (int i = sizeof(packetStart); i < offset; i++)
		checksum ^= aux[i];
	
	aux[offset] = checksum;
	
	serialData(aux, totalLength);
}
