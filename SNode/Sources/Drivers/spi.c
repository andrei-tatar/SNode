/*
 * spi.c
 *
 *  Created on: Jan 25, 2014
 *      Author: Andrei
 */

#include "spi.h"

uint8_t spi_ReadWriteByte(uint8_t data)
{
	while (!(SPI0_BASE_PTR->S & SPI_S_SPTEF_MASK)); //wait TX to be empty
	SPI0_BASE_PTR->D = data;
	while (!(SPI0_BASE_PTR->S & SPI_S_SPRF_MASK)); //wait RX to be full
	
	volatile uint8_t retData = SPI0_BASE_PTR->D;
	return retData;
}

void spi_WriteBlock(uint8_t *data, uint8_t len)
{
	while (len--)
		spi_ReadWriteByte(*data++);
}
