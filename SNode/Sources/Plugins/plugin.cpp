#include "plugin.h"

#include "OutputsPlugin.h"
#include "MainRouterPlugin.h"
#include "InputPlugin.h"
#include "InputsOutputsPlugin.h"

Plugin::Plugin(RF24Network &network)
	:network(network)
{
}

Plugin& Plugin::Load(RF24Network &network, NodeType type)
{	
	switch (type & 0xFFFF)
	{
	case MainRouter:
		return * new MainRouterPlugin(network);
	case Outputs:
		return * new OutputsPlugin(network);
	case InputsWithSleep:
		return * new InputPlugin(network);
	case InputsOutputs:
		return * new InputsOutputsPlugin(network);
	}
	
	while (1);
}

void Plugin::OnNetworkPacketReceived(RF24NetworkHeader &header, const uint8_t *data, uint8_t length)
{
	//do nothing
}

void Plugin::OnSerialPacketReceived(uint8_t cmd, const uint8_t *data, uint8_t length)
{
	//do nothing
}

void Plugin::Loop()
{
	//do nothing
}
