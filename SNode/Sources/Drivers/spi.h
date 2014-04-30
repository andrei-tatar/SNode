/*
 * spi.h
 *
 *  Created on: Jan 25, 2014
 *      Author: Andrei
 */

#ifndef SPI_H_
#define SPI_H_

#include <stdint.h>
#include "SPI0.h"

uint8_t spi_ReadWriteByte(uint8_t data);
void spi_WriteBlock(uint8_t *data, uint8_t len);

#endif /* SPI_H_ */
