#include "IO_Map.h"
#include "InputPlugin.h"
#include "hal.h"

PE_ISR(interruptInputPluginPORTA)
{
	uint32_t state = PORTA_BASE_PTR->ISFR & 0b11110;
	PORTA_BASE_PTR->ISFR = state;
	
	InputPlugin::InputsChanged = true;
	LED_TOGGLE(LED1);
}

volatile bool InputPlugin::InputsChanged = true;

InputPlugin::InputPlugin(RF24Network &network)
	:Plugin(network)
{
	_led2ToggleTime = 0;
	_lastReceivedPacket = 0;
	
	_prevInputs = 0xFF;
	interruptChange(InterruptPORTA, &interruptInputPluginPORTA);
}

void InputPlugin::Init()
{
	//enable clock to PORTD and PORTE
	SIM_SCGC5 |= SIM_SCGC5_PORTA_MASK;

	/* GPIOA_PDDR: PDD&=~0x1E */
	GPIOA_PDDR &= (uint32_t)~(uint32_t)(GPIO_PDDR_PDD(0x1E));                                   
	/* Initialization of Port Control registers */
	/* PORTA_PCR1: ISF=0,MUX=1 */
	PORTA_PCR1 = (uint32_t)((PORTA_PCR1 & (uint32_t)~(uint32_t)(
			PORT_PCR_ISF_MASK |
			PORT_PCR_MUX(0x06)
	)) | (uint32_t)(
			PORT_PCR_MUX(0x01)
	));                                  
	/* PORTA_PCR2: ISF=0,MUX=1 */
	PORTA_PCR2 = (uint32_t)((PORTA_PCR2 & (uint32_t)~(uint32_t)(
			PORT_PCR_ISF_MASK |
			PORT_PCR_MUX(0x06)
	)) | (uint32_t)(
			PORT_PCR_MUX(0x01)
	));                                  
	/* PORTA_PCR3: ISF=0,MUX=1 */
	PORTA_PCR3 = (uint32_t)((PORTA_PCR3 & (uint32_t)~(uint32_t)(
			PORT_PCR_ISF_MASK |
			PORT_PCR_MUX(0x06)
	)) | (uint32_t)(
			PORT_PCR_MUX(0x01)
	));                                  
	/* PORTA_PCR4: ISF=0,MUX=1 */
	PORTA_PCR4 = (uint32_t)((PORTA_PCR4 & (uint32_t)~(uint32_t)(
			PORT_PCR_ISF_MASK |
			PORT_PCR_MUX(0x06)
	)) | (uint32_t)(
			PORT_PCR_MUX(0x01)
	));                                  
	/* PORTA_PCR1: ISF=1,IRQC=0x0B */
	PORTA_PCR1 = (uint32_t)((PORTA_PCR1 & (uint32_t)~(uint32_t)(
			PORT_PCR_IRQC(0x04)
	)) | (uint32_t)(
			PORT_PCR_PE_MASK |
			PORT_PCR_ISF_MASK |
			PORT_PCR_IRQC(0x0B)
	));                                  
	/* PORTA_PCR2: ISF=1,IRQC=0x0B */
	PORTA_PCR2 = (uint32_t)((PORTA_PCR2 & (uint32_t)~(uint32_t)(
			PORT_PCR_IRQC(0x04)
	)) | (uint32_t)(
			PORT_PCR_PE_MASK |
			PORT_PCR_ISF_MASK |
			PORT_PCR_IRQC(0x0B)
	));                                  
	/* PORTA_PCR3: ISF=1,IRQC=0x0B */
	PORTA_PCR3 = (uint32_t)((PORTA_PCR3 & (uint32_t)~(uint32_t)(
			PORT_PCR_IRQC(0x04)
	)) | (uint32_t)(
			PORT_PCR_PE_MASK |
			PORT_PCR_ISF_MASK |
			PORT_PCR_IRQC(0x0B)
	));                                  
	/* PORTA_PCR4: ISF=1,IRQC=0x0B */
	PORTA_PCR4 = (uint32_t)((PORTA_PCR4 & (uint32_t)~(uint32_t)(
			PORT_PCR_IRQC(0x04)
	)) | (uint32_t)(
			PORT_PCR_PE_MASK |
			PORT_PCR_ISF_MASK |
			PORT_PCR_IRQC(0x0B)
	));                                  
	/* NVIC_IPR7: PRI_30=0x80 */
	NVIC_IPR7 = (uint32_t)((NVIC_IPR7 & (uint32_t)~(uint32_t)(
			NVIC_IP_PRI_30(0x7F)
	)) | (uint32_t)(
			NVIC_IP_PRI_30(0x80)
	));                                  
	/* NVIC_ISER: SETENA|=0x40000000 */
	NVIC_ISER |= NVIC_ISER_SETENA(0x40000000);
}

bool InputPlugin::OnSerialPacketReceived(uint8_t cmd, uint8_t *data, uint8_t length)
{
	_lastReceivedPacket = getTime();
	return Plugin::OnSerialPacketReceived(cmd, data, length);
}

bool InputPlugin::Loop()
{
	uint32_t now = getTime();
	bool shouldSleep = (now - _lastReceivedPacket) > 5000;
	bool r = Plugin::Loop();
	
	if (!shouldSleep)
	{
		if (now > _led2ToggleTime)
		{
			LED_TOGGLE(LED2);
			_led2ToggleTime = now + 50;
		}
	}
	else
		LED_OFF(LED2);
	
	
	if (!InputsChanged) return r && shouldSleep;
	InputsChanged = false;
	
	uint8_t inputValues = (PTA_BASE_PTR->PDIR & 0x1E) >> 1;
	if (_prevInputs == inputValues)
		return r && shouldSleep;
	
	_prevInputs = inputValues;
	RF24NetworkHeader header = RF24NetworkHeader(0, 0);
	uint8_t data[2] = { SNODE_CMD_INPUTS , inputValues };
	bool ok = network.write(header, data, sizeof(data));
	TurnLed(LED1, ok ? 50 : 300);
	return false;
}
