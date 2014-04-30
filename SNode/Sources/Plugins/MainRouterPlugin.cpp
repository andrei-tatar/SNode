/*
 * MainRouterPlugin.cpp
 *
 *  Created on: Apr 25, 2014
 *      Author: X550L-User1
 */

#include "MainRouterPlugin.h"
#include "hal.h"
#include "serial.h"

MainRouterPlugin::MainRouterPlugin(RF24Network &network)
:Plugin(network)
{

}

void MainRouterPlugin::Init()
{
}

void MainRouterPlugin::OnNetworkPacketReceived(RF24NetworkHeader &header, const uint8_t *data, uint8_t length)
{
	LED_ON(LED2);
	uint8_t buffer[length + 2];
	buffer[0] = (header.from_node >> (8 * 0)) & 0xFF;
	buffer[1] = (header.from_node >> (8 * 1)) & 0xFF;
	memcpy(&buffer[2], data, length);
	serialSendPacket(NT_PACKETRECEIVED, buffer, sizeof(buffer));	
	led2OffTime = getTime() + 50;
}

void MainRouterPlugin::OnSerialPacketReceived(uint8_t cmd, const uint8_t *data, uint8_t length)
{
	RF24NetworkHeader header;
	bool ok;
	
	switch (cmd)
	{
	case CMD_SENDPACKET:
		LED_ON(LED1);
		header = RF24NetworkHeader(data[0] | (data[1] << 8));
		ok = network.write(header, &data[2], length - 2);
		led1OffTime = getTime() + (ok ? 50 : 300);
		break;
	}
}

void MainRouterPlugin::Loop()
{
	uint32_t now = getTime();
	if (now > led1OffTime) LED_OFF(LED1);
	if (now > led2OffTime) LED_OFF(LED2);
}

