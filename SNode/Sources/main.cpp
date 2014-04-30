#include "hal.h"
#include "main.h"
#include "Commands.h"
#include "plugin.h"

#include "FLASH.h"

static ReceivedPacket queue[QUEUE_SIZE];
static volatile uint8_t queueWritePos = 0; 
static uint8_t queueReadPos = 0;
static volatile uint8_t queueCount = 0;

static UserSettings settings;
static RF24 radio;
static RF24Network network = RF24Network(radio);

bool FlashOperationComplete = false;
volatile bool InputPinsChanged = false;

void packetReceived(uint8_t cmd, const uint8_t *data, int length)
{
	if (queueCount == QUEUE_SIZE)
		return;
	
	queue[queueWritePos].Cmd = cmd;
	memcpy(queue[queueWritePos].Data, data, length);
	queue[queueWritePos].Length = length;
	
	queueWritePos = (queueWritePos + 1) & QUEUE_SIZE_MASK;
	queueCount ++;
}

static void WaitFlashOperation()
{
	while (!FlashOperationComplete) FLASH_Main(FLASH_DeviceData);
	FlashOperationComplete = false;
}

static void ProcessSerialPacket(Plugin &plugin, const ReceivedPacket &packet)
{
	RF24NetworkHeader header;
	const uint8_t *constBuf;
	uint8_t *buf;
	static uint8_t data[10];
	
	switch (packet.Cmd)
	{
	//send back an echo
	case CMD_PING_ECHO:
		serialSendPacket(NT_PING_ECHO, packet.Data, packet.Length);
		break;
		
	case CMD_SETTINGS:
		settings.MagicNumber = SETTINGS_MAGIC_NUMBER;
		constBuf = packet.Data;
		
		settings.HashCode  = *constBuf++ << (8 * 0);
		settings.HashCode |= *constBuf++ << (8 * 1);
		settings.HashCode |= *constBuf++ << (8 * 2);
		settings.HashCode |= *constBuf++ << (8 * 3);
		
		memcpy(settings.AesKey, constBuf, 16); constBuf += 16;
		memcpy(settings.AesIV, constBuf, 16); constBuf += 16;
		
		uint32_t nodeType;
		nodeType  = *constBuf++ << (8 * 0);
		nodeType |= *constBuf++ << (8 * 1);
		nodeType |= *constBuf++ << (8 * 2);
		nodeType |= *constBuf++ << (8 * 3);
		settings.NodeType = nodeType;
		
		settings.Address  = *constBuf++;
		settings.Address |= *constBuf++ << 8;
		
		settings.Channel  = *constBuf++;
		
		uint16_t result;
		if ((result = FLASH_Erase(FLASH_DeviceData, 
				FLASH_USER_AREA0_Settings_ADDRESS, FLASH_ERASABLE_UNIT_SIZE)) == ERR_OK)
		{
			WaitFlashOperation();
		
			if (FLASH_Write(FLASH_DeviceData, &settings, 
					FLASH_USER_AREA0_Settings_ADDRESS, sizeof(UserSettings)) == ERR_OK)
			{
				WaitFlashOperation();
				
				//do a reset to apply the settings
				Cpu_SystemReset();
			}
		}
		break;
		
	case CMD_GET_SETTINGS_H:
		buf = data;
		
		*buf++ = (settings.HashCode >> (8 * 0)) & 0xFF;
		*buf++ = (settings.HashCode >> (8 * 1)) & 0xFF;
		*buf++ = (settings.HashCode >> (8 * 2)) & 0xFF;
		*buf   = (settings.HashCode >> (8 * 3)) & 0xFF;
		
		serialSendPacket(NT_SETTINGS_H, data, 4);
		break;
		
	case CMD_GETID:
		buf = data;
		
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
		plugin.OnSerialPacketReceived(packet.Cmd, packet.Data, packet.Length);
		break;
	}
}

static void ProcessNetworkPackets(Plugin &plugin)
{
	//check if any network packets available and process them
	while (network.available())
	{
		RF24NetworkHeader header;
		uint8_t buffer[24]; 
		network.read(header, buffer, sizeof(buffer));
		plugin.OnNetworkPacketReceived(header, buffer, sizeof(buffer));
	}
}

static void ProcessSerialPackets(Plugin &plugin)
{
	bool packetsReceived = false;
	do
	{
		__DI();
		bool packetsReceived = queueCount != 0;
		__EI();
		
		if (packetsReceived)
		{
			ReceivedPacket &packet = queue[queueReadPos];
			queueReadPos = (queueReadPos + 1) & QUEUE_SIZE_MASK;
			__DI();
			queueCount--;
			__EI();
			
			ProcessSerialPacket(plugin, packet);
		}
	} while (packetsReceived);
}

void sNodeMain(void)
{
	interruptInit();
	
	if (FLASH_Read(FLASH_DeviceData, FLASH_USER_AREA0_Settings_ADDRESS, &settings, sizeof(UserSettings)) == ERR_OK)
	{
		WaitFlashOperation();
	}
	
	if (settings.MagicNumber != SETTINGS_MAGIC_NUMBER)
	{				
		//set some default settings
		settings.Address = 0;
		settings.Channel = 90;
		settings.NodeType = InputsOutputs;
		//settings.NodeType = MainRouter;
	}
	
	AES_generateSBox(); //init sandbox for AES encryption
	network.enableEncryption(settings.AesKey, settings.AesIV, AES_MODE_128); //enable encryption for network

	radio.begin(); //init radio
	network.begin(settings.Channel, settings.Address); //init network

	serialSendPacket(NT_BOOTED, NULL, 0);
	
	Plugin &plugin = Plugin::Load(network, (NodeType)settings.NodeType);
	plugin.Init();
	
	while (true)
	{
		if (settings.NodeType & EnableSleepMode)
		{
			radio.powerDown();
			//the other parameters are not used
			Cpu_SetOperationMode(DOM_SLEEP, NULL, NULL);
			radio.powerUp();
		}
		
		network.update();	
		ProcessSerialPackets(plugin);
		ProcessNetworkPackets(plugin);
		plugin.Loop();
	}
}
