/*
 * InputsOutputsPlugin.cpp
 *
 *  Created on: Apr 28, 2014
 *      Author: X550L-User1
 */

#include "InputsOutputsPlugin.h"

InputsOutputsPlugin::InputsOutputsPlugin(RF24Network &network)
:Plugin(network), InputPlugin(network), OutputsPlugin(network)
{
}

void InputsOutputsPlugin::Init()
{
	InputPlugin::Init();
	OutputsPlugin::Init();
}
	
void InputsOutputsPlugin::OnNetworkPacketReceived(RF24NetworkHeader &header, const uint8_t *data, uint8_t length)
{
	OutputsPlugin::OnNetworkPacketReceived(header, data, length);
}

void InputsOutputsPlugin::Loop()
{
	InputPlugin::Loop();
}

