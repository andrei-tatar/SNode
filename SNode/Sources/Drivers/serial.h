/*
 * serial.h
 *
 *  Created on: Jan 25, 2014
 *      Author: Andrei
 */

#ifndef SERIAL_H_
#define SERIAL_H_

#include "UART1.h"
#include <stdint.h>
#include <string.h>

#define MAX_RX_SIZE	64

void flushTx();

extern void packetReceived(uint8_t cmd, const uint8_t *data, int length);
void serialSendPacket(uint8_t cmd, const void *data, int length);

void debugString(const char *str);

#endif /* SERIAL_H_ */
