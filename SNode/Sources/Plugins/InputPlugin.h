#ifndef INPUTPLUGIN_H_
#define INPUTPLUGIN_H_

#include "plugin.h"

#define SNODE_CMD_INPUTS	0x1D

class InputPlugin: virtual public Plugin {
public:
	volatile static bool InputsChanged;
	
	InputPlugin(RF24Network &network);
	virtual void Init();
	virtual void Loop();
};

#endif /* INPUTPLUGIN_H_ */
