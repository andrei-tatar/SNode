/*
 * plugin.h
 *
 *  Created on: Apr 25, 2014
 *      Author: X550L-User1
 */

#ifndef PLUGINS_H_
#define PLUGINS_H_

#include "RF24Network.h"

enum NodeType {
	//flags
	EnableSleepMode = 0x00010000,
	
	//actual node types
	MainRouter = 0x0001,
	Outputs = 0x0002,
	InputsWithSleep = 0x0003 | EnableSleepMode,
	InputsOutputs = 0x0004,
};

enum InterruptType {
	PortAInterrupt,
};

class Plugin
{
protected:
	RF24Network &network;
	Plugin(RF24Network &network);
	
public:
	static Plugin& Load(RF24Network &network, NodeType type);
	
	virtual void Init() = 0;
	
	virtual void OnNetworkPacketReceived(RF24NetworkHeader &header, const uint8_t *data, uint8_t length);
	virtual void OnSerialPacketReceived(uint8_t cmd, const uint8_t *data, uint8_t length);
	virtual void Loop();
};

#endif /* PLUGINS_H_ */
