#include "plugin.h"

#include "OutputsPlugin.h"
#include "MainRouterPlugin.h"
#include "InputPlugin.h"
#include "InputsOutputsPlugin.h"

#include "FLASH.h"
#include "main.h"
#include "hal.h"

/* Echo command expects a reply with the same command and same data it received */
#define CMD_PING_ECHO		0xDE

/* The settings (AES key, channel, address, node type, etc.) are sent with this command
 * offset,	size,	data
 * 4		4		settings hash code (used to identify if settings changed)
 * 4		16		AES Key 
 * 20		16		AES IV 
 * 36		4		Node Type (LSB first)
 * 40		2		Address (LSB first)
 * 42		1		Channel */
#define CMD_SETTINGS		0xA9

/* Instructs the node to send the storred settings hash code */
#define CMD_GET_SETTINGS_H	0xAB

/* Resets the node */
#define CMD_RESET			0x41

/* Requests the unique ID of the node */
#define CMD_GETID			0x7E

/* Notifies the node has booted and inited */
#define NT_BOOTED			0x2F

/* Sends the node id (the next 10 bytes represent the unique chip id)*/
#define NT_NODEID			0x53

/* Sends the settings hash code (4 bytes) */
#define NT_SETTINGS_H		0x3D

/* Sent as a repy to CMD_PING_ECHO */
#define NT_PING_ECHO		CMD_PING_ECHO

#define SNODE_CMD_ECHO		0xE0
#define SNODE_CMD_ECHO_R	0xE1

Plugin::Plugin(RF24Network &network)
	:network(network)
{
	serialSendPacket(NT_BOOTED, NULL, 0);
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

void Plugin::TurnLed(uint32_t led, uint16_t delay)
{
	LED_ON(led);
	
	switch (led)
	{
	case LED1:
		led1OffTime = getTime() + delay;
		break;
	case LED2:
		led2OffTime = getTime() + delay;
		break;
	}
}

bool Plugin::OnNetworkPacketReceived(RF24NetworkHeader &header, const uint8_t *data, uint8_t length)
{
	TurnLed(LED2, 50);
	
	uint8_t txdata[24];
	
	switch (data[0])
	{
	case SNODE_CMD_ECHO:
		memcpy(txdata, data, sizeof(txdata));
		header.to_node = header.from_node;
		txdata[0] = SNODE_CMD_ECHO_R;
		network.write(header, txdata, length);
		break;
		
	default:
		return false;
	}
	
	return true;
}

bool Plugin::OnSerialPacketReceived(uint8_t cmd, const uint8_t *data, uint8_t length)
{
	RF24NetworkHeader header;
	uint8_t *buf;
	static uint8_t auxdata[10];
	
	switch (cmd)
	{
	//send back an echo
	case CMD_PING_ECHO:
		serialSendPacket(NT_PING_ECHO, data, length);
		break;
		
	case CMD_SETTINGS:
		userSettings.MagicNumber = SETTINGS_MAGIC_NUMBER;
		
		userSettings.HashCode  = *data++ << (8 * 0);
		userSettings.HashCode |= *data++ << (8 * 1);
		userSettings.HashCode |= *data++ << (8 * 2);
		userSettings.HashCode |= *data++ << (8 * 3);
		
		memcpy(userSettings.AesKey, data, 16); data += 16;
		memcpy(userSettings.AesIV, data, 16); data += 16;
		
		uint32_t nodeType;
		nodeType  = *data++ << (8 * 0);
		nodeType |= *data++ << (8 * 1);
		nodeType |= *data++ << (8 * 2);
		nodeType |= *data++ << (8 * 3);
		userSettings.NodeType = nodeType;
		
		userSettings.Address  = *data++;
		userSettings.Address |= *data++ << 8;
		
		userSettings.Channel  = *data++;
		
		uint16_t result;
		if ((result = FLASH_Erase(FLASH_DeviceData, 
				FLASH_USER_AREA0_Settings_ADDRESS, FLASH_ERASABLE_UNIT_SIZE)) == ERR_OK)
		{
			WaitFlashOperation();
		
			if (FLASH_Write(FLASH_DeviceData, &userSettings, 
					FLASH_USER_AREA0_Settings_ADDRESS, sizeof(UserSettings)) == ERR_OK)
			{
				WaitFlashOperation();
				
				//do a reset to apply the settings
				Cpu_SystemReset();
			}
		}
		break;
		
	case CMD_GET_SETTINGS_H:
		buf = auxdata;
		
		*buf++ = (userSettings.HashCode >> (8 * 0)) & 0xFF;
		*buf++ = (userSettings.HashCode >> (8 * 1)) & 0xFF;
		*buf++ = (userSettings.HashCode >> (8 * 2)) & 0xFF;
		*buf   = (userSettings.HashCode >> (8 * 3)) & 0xFF;
		
		serialSendPacket(NT_SETTINGS_H, data, 4);
		break;
		
	case CMD_GETID:
		buf = auxdata;
		
		*buf++ = (SIM_UIDL  >> (8 * 0)) & 0xFF; *buf++ = (SIM_UIDL  >> (8 * 1)) & 0xFF;
		*buf++ = (SIM_UIDL  >> (8 * 2)) & 0xFF; *buf++ = (SIM_UIDL  >> (8 * 3)) & 0xFF;
		*buf++ = (SIM_UIDML >> (8 * 0)) & 0xFF; *buf++ = (SIM_UIDML >> (8 * 1)) & 0xFF;
		*buf++ = (SIM_UIDML >> (8 * 2)) & 0xFF; *buf++ = (SIM_UIDML >> (8 * 3)) & 0xFF;
		*buf++ = (SIM_UIDMH >> (8 * 0)) & 0xFF; *buf   = (SIM_UIDMH >> (8 * 1)) & 0xFF;
		
		serialSendPacket(NT_NODEID, data, 10);
		break;
		
	case CMD_RESET:
		Cpu_SystemReset();
		break;
		
	default:
		return false;
	}
	
	return true;
}

void Plugin::Loop()
{
	uint32_t now = getTime();
	if (now > led1OffTime) LED_OFF(LED1);
	if (now > led2OffTime) LED_OFF(LED2);
}
