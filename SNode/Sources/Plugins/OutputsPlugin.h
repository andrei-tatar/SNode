#ifndef OUTPUTSPLUGIN_H_
#define OUTPUTSPLUGIN_H_

#include "plugin.h"

#define SNODE_CMD_OUTPUT	0x4A

class OutputsPlugin : virtual public Plugin {
public:
	OutputsPlugin(RF24Network &network);
	virtual void Init();
	virtual void OnNetworkPacketReceived(RF24NetworkHeader &header, const uint8_t *data, uint8_t length);	
};

#endif /* OUTPUTSPLUGIN_H_ */
