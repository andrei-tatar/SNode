#ifndef INPUTPLUGIN_H_
#define INPUTPLUGIN_H_

#include "plugin.h"

#define SNODE_CMD_INPUTS	0x1D

class InputPlugin: virtual public Plugin {
private:
	uint8_t _prevInputs;
	uint32_t _lastReceivedPacket;
	uint32_t _led2ToggleTime;
	
public:
	volatile static bool InputsChanged;
	
	InputPlugin(RF24Network &network);
	virtual void Init();
	virtual bool Loop();
	virtual bool OnSerialPacketReceived(uint8_t cmd, uint8_t *data, uint8_t length);
};

#endif /* INPUTPLUGIN_H_ */
