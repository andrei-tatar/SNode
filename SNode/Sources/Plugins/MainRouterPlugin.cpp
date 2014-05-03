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

bool MainRouterPlugin::OnNetworkPacketReceived(RF24NetworkHeader &header, uint8_t *data, uint8_t length)
{
	if (Plugin::OnNetworkPacketReceived(header, data, length))
		return true;
	
	uint8_t buffer[length + 2];
	buffer[0] = (header.from_node >> (8 * 0)) & 0xFF;
	buffer[1] = (header.from_node >> (8 * 1)) & 0xFF;
	memcpy(&buffer[2], data, length);
	serialSendPacket(NT_PACKETRECEIVED, buffer, sizeof(buffer));	
	return true;
}

bool MainRouterPlugin::OnSerialPacketReceived(uint8_t cmd, uint8_t *data, uint8_t length)
{
	if (Plugin::OnSerialPacketReceived(cmd, data, length))
		return true;
	
	RF24NetworkHeader header;
	bool ok;
	
	switch (cmd)
	{
	case CMD_SENDPACKET:
		header = RF24NetworkHeader(data[0] | (data[1] << 8));
		ok = network.write(header, &data[2], length - 2);
		TurnLed(LED1, ok ? 50 : 300);
		break;
		
	default:
		return false;
	}
	
	return true;
}
