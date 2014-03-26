#include "hal.h"
#include "main.h"
#include "Commands.h"

#include "FLASH.h"
#include "Inputs.h"
#include "O0.h"
#include "O1.h"
#include "O2.h"
#include "O3.h"

static ReceivedPacket queue[QUEUE_SIZE];
static volatile uint8_t queueWritePos = 0; 
static uint8_t queueReadPos = 0;
static volatile uint8_t queueCount = 0;

static UserSettings settings;
static RF24 radio;
static RF24Network network = RF24Network(radio);
static uint32_t led1OffTime = 0, led2OffTime = 0;

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

static void ProcessSerialPacket(const ReceivedPacket &packet)
{
	bool ok;
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
		
	case CMD_SENDPACKET:
		LED_ON(LED1);
		header = RF24NetworkHeader(packet.Data[0] | (packet.Data[1] << 8));
		ok = network.write(header, &packet.Data[2], packet.Length - 2);
		led1OffTime = getTime() + (ok ? 100 : 0);
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
		settings.NodeType = (NodeConfigType::NodeConfigurationType)nodeType;
		
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

static void ProcessNetworkPacket(RF24NetworkHeader &header, const uint8_t *data, uint8_t length)
{
	bool ok;
	uint8_t buffer[3];
	
	switch (*data++)
	{
	case SNODE_CMD_OUTPUT:
		O0_PutVal(NULL, *data & 0x01);
		O1_PutVal(NULL, *data & 0x02);
		O2_PutVal(NULL, *data & 0x04);
		O3_PutVal(NULL, *data & 0x08);
		break;
	case SNODE_CMD_ECHO:
		LED_ON(LED1);
		header = RF24NetworkHeader(header.from_node);
		buffer[0] = SNODE_CMD_ECHO_R;
		ok = network.write(header, buffer, 1);
		led1OffTime = getTime() + (ok ? 100 : 0);
		break;
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

static void configureNodeFromSettings()
{
	if (settings.NodeType & NodeConfigType::CustomNodeType)
		return;
	
	//enable outputs
	if (!(settings.NodeType & NodeConfigType::EnableSleepMode))
	{
		O0_Init(NULL); O1_Init(NULL);
		O2_Init(NULL); O3_Init(NULL);
	}
	
	//enable inputs
	if (settings.NodeType & NodeConfigType::InputEnableMask)
	{
		Inputs_Init(NULL);
		uint32_t interruptsMask = 0;
		
		//enable event for enabled inputs
		if (settings.NodeType & NodeConfigType::Input0Enable) interruptsMask |= Inputs_FIELD_0_PIN_0;
		if (settings.NodeType & NodeConfigType::Input1Enable) interruptsMask |= Inputs_FIELD_0_PIN_1;
		if (settings.NodeType & NodeConfigType::Input2Enable) interruptsMask |= Inputs_FIELD_0_PIN_2;
		if (settings.NodeType & NodeConfigType::Input3Enable) interruptsMask |= Inputs_FIELD_0_PIN_3;
		
		Inputs_SetPortEventCondition(NULL, interruptsMask, LDD_GPIO_BOTH);
	}
}

static void sendInputs()
{
	static uint8_t buffer[3] = { SNODE_CMD_INPUTS };
	uint8_t length = 2;
	
	buffer[1] = SNodeChange::NoChanges;
	
	if (InputPinsChanged)
	{
		buffer[1] |= SNodeChange::InputValues;
		buffer[length++] = Inputs_GetFieldValue(NULL, Input);
		InputPinsChanged = false;
	}
	
	if (buffer[1])
	{
		LED_ON(LED1);
		RF24NetworkHeader header = RF24NetworkHeader(0); //TODO: keep root address as a setting?
		bool ok = network.write(header, buffer, length);
		led1OffTime = getTime() + (ok ? 100 : 0);
	}
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
	
	configureNodeFromSettings();
	
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
		
		sendInputs();
		
		network.update();
	
		uint32_t now = getTime();
		if (now > led1OffTime) LED_OFF(LED1);
		if (now > led2OffTime) LED_OFF(LED2);
		
		ProcessSerialPackets();
		ProcessNetworkPackets();
	}
}
