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

void flushTx();

extern void packetReceived(uint8_t cmd, uint8_t *data, int length);
void serialSendPacket(uint8_t cmd, const uint8_t *data, int length);

#endif /* SERIAL_H_ */
