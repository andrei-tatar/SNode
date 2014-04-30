/*
 * MainRouterPlugin.h
 *
 *  Created on: Apr 25, 2014
 *      Author: X550L-User1
 */

#ifndef MAINROUTERPLUGIN_H_
#define MAINROUTERPLUGIN_H_

/* Instructs a node to send a packet. The first 2 bytes are the destination address (1:LSB, 2:MSB), 
 * the rest of the bytes are the actual data in the packet that will be sent */
#define CMD_SENDPACKET 		0x31

/* Sent when a packet is received. The first 2 bytes are the destination address (1:LSB, 2:MSB),
 * the rest of the bytes are the actual data in the packet that was received */
#define NT_PACKETRECEIVED	0x32




#include "plugin.h"

class MainRouterPlugin: public Plugin {
protected:
	uint32_t led1OffTime;
	uint32_t led2OffTime;
	
	
public:
	MainRouterPlugin(RF24Network &network);
	virtual void Init();
	virtual void OnNetworkPacketReceived(RF24NetworkHeader &header, const uint8_t *data, uint8_t length);
	virtual void OnSerialPacketReceived(uint8_t cmd, const uint8_t *data, uint8_t length);
	virtual void Loop();
};

#endif /* MAINROUTERPLUGIN_H_ */
