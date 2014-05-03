/*
 * InputsOutputsPlugin.h
 *
 *  Created on: Apr 28, 2014
 *      Author: X550L-User1
 */

#ifndef INPUTSOUTPUTSPLUGIN_H_
#define INPUTSOUTPUTSPLUGIN_H_

#include "plugin.h"
#include "InputPlugin.h"
#include "OutputsPlugin.h"

class InputsOutputsPlugin: public InputPlugin, OutputsPlugin {
public:
	InputsOutputsPlugin(RF24Network &network);
	
	virtual void Init();
	virtual bool OnNetworkPacketReceived(RF24NetworkHeader &header, uint8_t *data, uint8_t length);
	virtual bool Loop();
};

#endif /* INPUTSOUTPUTSPLUGIN_H_ */
