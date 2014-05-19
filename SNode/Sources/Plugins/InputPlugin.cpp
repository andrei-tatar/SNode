#include "IO_Map.h"
#include "InputPlugin.h"
#include "hal.h"

#define ENTER_SLEEP_DELAY	5000

InputPlugin::InputPlugin(RF24Network &network)
	:Plugin(network)
{
	_led2ToggleTime = 0;
	_enterSleepTime = ENTER_SLEEP_DELAY;
	
	_prevInputs = 0xFF;
}

void InputPlugin::Init()
{
	//enable clock to PORTA
	SIM_SCGC5 |= SIM_SCGC5_PORTA_MASK;

	//set as inputs
	GPIOA_PDDR &= (uint32_t)~(uint32_t)(GPIO_PDDR_PDD(0x1E));
	
	/* Initialization of Port Control registers */
	//disable interrupt, set as GPIO with pull enable
	
	//PA1
	PORTA_PCR1 = (uint32_t)((PORTA_PCR1 & (uint32_t)~(uint32_t)(
			PORT_PCR_ISF_MASK |
			PORT_PCR_MUX(0x06)
	)) | (uint32_t)(
			PORT_PCR_MUX(0x01) | 
			PORT_PCR_PE_MASK
	));
	
	//PA2
	PORTA_PCR2 = (uint32_t)((PORTA_PCR2 & (uint32_t)~(uint32_t)(
			PORT_PCR_ISF_MASK |
			PORT_PCR_MUX(0x06)
	)) | (uint32_t)(
			PORT_PCR_MUX(0x01) | 
			PORT_PCR_PE_MASK
	));                                  

	//PA3
	PORTA_PCR3 = (uint32_t)((PORTA_PCR3 & (uint32_t)~(uint32_t)(
			PORT_PCR_ISF_MASK |
			PORT_PCR_MUX(0x06)
	)) | (uint32_t)(
			PORT_PCR_MUX(0x01) | 
			PORT_PCR_PE_MASK
	));                                  

	//PA4
	PORTA_PCR4 = (uint32_t)((PORTA_PCR4 & (uint32_t)~(uint32_t)(
			PORT_PCR_ISF_MASK |
			PORT_PCR_MUX(0x06)
	)) | (uint32_t)(
			PORT_PCR_MUX(0x01) | 
			PORT_PCR_PE_MASK
	));
}

bool InputPlugin::OnSerialPacketReceived(uint8_t cmd, uint8_t *data, uint8_t length)
{
	_enterSleepTime = getTime() + ENTER_SLEEP_DELAY;
	return Plugin::OnSerialPacketReceived(cmd, data, length);
}

bool InputPlugin::Loop()
{
	uint32_t now = getTime();
	bool shouldSleep = now > _enterSleepTime;
	bool baseShouldSleep = Plugin::Loop();
	
	if (!shouldSleep)
		if (now > _led2ToggleTime)
		{
			TurnLed(LED2, 100);
			_led2ToggleTime = now + 200;
		}
	
	shouldSleep &= baseShouldSleep;
	
	uint8_t inputValues = (PTA_BASE_PTR->PDIR & 0x1E) >> 1;
	if (_prevInputs == inputValues)
		return shouldSleep;
	
	_prevInputs = inputValues;
	RF24NetworkHeader header = RF24NetworkHeader(0, 0);
	uint8_t data[2] = { SNODE_CMD_INPUTS , inputValues };
	bool ok = network.write(header, data, sizeof(data));
	TurnLed(LED1, ok ? 50 : 300);
	return false;
}
