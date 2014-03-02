#include "hal.h"
#include "main.h"
#include "Commands.h"
#include "FLASH.h"

static ReceivedPacket queue[QUEUE_SIZE];
static volatile uint8_t queueWritePos = 0; 
static uint8_t queueReadPos = 0;
static volatile uint8_t queueCount = 0;

static UserSettings settings;
static RF24 radio;
static RF24Network network = RF24Network(radio);
static uint32_t led1OffTime = 0, led2OffTime = 0;

bool FlashOperationComplete = false;

void packetReceived(uint8_t cmd, uint8_t *data, int length)
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

static void ProcessSerialPacket(ReceivedPacket &packet)
{
	bool ok;
	RF24NetworkHeader header;
	uint8_t *buf;
	static uint8_t data[10];
	
	switch (packet.Cmd)
	{
	//send back an echo
	case CMD_PING_ECHO:
		serialSendPacket(NT_PING_ECHO, packet.Data, packet.Length);
		break;
		
	case CMD_SENDPACKET:
		LED_ON(LED1);
		header = RF24NetworkHeader(packet.Data[0] | (packet.Data[1] << 8));
		ok = network.write(header, &packet.Data[2], packet.Length - 2);
		led1OffTime = getTime() + (ok ? 100 : 0);
		break;
		
	case CMD_SETTINGS:
		settings.MagicNumber = SETTINGS_MAGIC_NUMBER;
		buf = packet.Data;
		
		settings.HashCode  = *buf++ << (8 * 0);
		settings.HashCode |= *buf++ << (8 * 1);
		settings.HashCode |= *buf++ << (8 * 2);
		settings.HashCode |= *buf++ << (8 * 3);
		
		memcpy(settings.AesKey, buf, 16); buf += 16;
		memcpy(settings.AesIV, buf, 16); buf += 16;
		
		uint32_t nodeType;
		nodeType  = *buf++ << (8 * 0);
		nodeType |= *buf++ << (8 * 1);
		nodeType |= *buf++ << (8 * 2);
		nodeType |= *buf++ << (8 * 3);
		settings.NodeType = (NodeConfigType::NodeConfigurationType)nodeType;
		
		settings.Address  = *buf++;
		settings.Address |= *buf++ << 8;
		
		settings.Channel  = *buf++;
		
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
		
		*buf++ = (SIM_UIDL  >> (8 * 0)) & 0xFF;
		*buf++ = (SIM_UIDL  >> (8 * 1)) & 0xFF;
		*buf++ = (SIM_UIDL  >> (8 * 2)) & 0xFF;
		*buf++ = (SIM_UIDL  >> (8 * 3)) & 0xFF;
		
		*buf++ = (SIM_UIDML >> (8 * 0)) & 0xFF;
		*buf++ = (SIM_UIDML >> (8 * 1)) & 0xFF;
		*buf++ = (SIM_UIDML >> (8 * 2)) & 0xFF;
		*buf++ = (SIM_UIDML >> (8 * 3)) & 0xFF;
		
		*buf++ = (SIM_UIDMH >> (8 * 0)) & 0xFF;
		*buf   = (SIM_UIDMH >> (8 * 1)) & 0xFF;
		
		serialSendPacket(NT_NODEID, data, 10);
		break;
		
	case CMD_RESET:
		Cpu_SystemReset();
		break;
	}
}

static void ProcessNetworkPacket(RF24NetworkHeader &header, uint8_t *data, uint8_t length)
{
	switch (*data++)
	{
	
	}
}

static void ProcessNetworkPackets()
{
	//check if any network packets available and process them
	while (network.available())
	{
		LED_ON(LED2);
		led2OffTime = getTime() + 100;
		
		RF24NetworkHeader header;
		uint8_t buffer[26]; //2 bytes from address and 24 bytes data 
		network.read(header, &buffer[2], 24);
		
		if ((settings.NodeType & NodeConfigType::RedirectPacketsToSerialPort) != 0)
		{
			buffer[0] = (header.from_node >> (8 * 0)) & 0xFF;
			buffer[1] = (header.from_node >> (8 * 1)) & 0xFF;
			serialSendPacket(NT_PACKETRECEIVED, buffer, sizeof(buffer));
		}
		
		ProcessNetworkPacket(header, &buffer[2], 24);
	}
}

static void ProcessSerialPackets()
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
			
			ProcessSerialPacket(packet);
		}
	} while (packetsReceived);
}

void sNodeMain(void)
{		
	if (FLASH_Read(FLASH_DeviceData, FLASH_USER_AREA0_Settings_ADDRESS, &settings, sizeof(UserSettings)) == ERR_OK)
	{
		WaitFlashOperation();
	}
	
	if (settings.MagicNumber != SETTINGS_MAGIC_NUMBER)
	{				
		//set some default settings
		settings.Address = 0;
		settings.Channel = 90;
		settings.NodeType = NodeConfigType::RedirectPacketsToSerialPort;
	}
	
	AES_generateSBox(); //init sandbox for AES encryption
	network.enableEncryption(settings.AesKey, settings.AesIV, AES_MODE_128); //enable encryption for network

	radio.begin(); //init radio
	network.begin(settings.Channel, settings.Address); //init network

	serialSendPacket(NT_BOOTED, NULL, 0);
	
	while (true)
	{
		if (settings.NodeType & NodeConfigType::EnableSleepMode)
		{
			radio.powerDown();
			//the other parameters are not used
			Cpu_SetOperationMode(DOM_SLEEP, NULL, NULL);
			radio.powerUp();
		}
		
		network.update();
	
		uint32_t now = getTime();
		if (now > led1OffTime) LED_OFF(LED1);
		if (now > led2OffTime) LED_OFF(LED2);
		
		ProcessSerialPackets();
		ProcessNetworkPackets();
	}
}
