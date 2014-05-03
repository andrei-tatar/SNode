#include "hal.h"
#include "main.h"
#include "plugin.h"
#include "FLASH.h"

static ReceivedPacket queue[QUEUE_SIZE];
static volatile uint8_t queueWritePos = 0; 
static uint8_t queueReadPos = 0;
static volatile uint8_t queueCount = 0;

UserSettings userSettings;
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

void WaitFlashOperation()
{
	while (!FlashOperationComplete) FLASH_Main(FLASH_DeviceData);
	FlashOperationComplete = false;
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
			
			plugin.OnSerialPacketReceived(packet.Cmd, packet.Data, packet.Length);
		}
	} while (packetsReceived);
}

void sNodeMain(void)
{
	interruptInit();
	
	//some debug sequence
	LED_ON(LED1);
	LED_OFF(LED2);
	delay_ms(50);
	LED_OFF(LED1);
	LED_ON(LED2);
	delay_ms(50);
	LED_OFF(LED2);
	
	if (FLASH_Read(FLASH_DeviceData, FLASH_USER_AREA0_Settings_ADDRESS, &userSettings, sizeof(UserSettings)) == ERR_OK)
	{
		WaitFlashOperation();
	}
	
	if (userSettings.MagicNumber != SETTINGS_MAGIC_NUMBER)
	{				
		//set some default settings
		userSettings.Address = 0;
		userSettings.Channel = 90;
		userSettings.NodeType = MainRouter;
		
		const uint8_t defaultAesKey[] =
		{
			0x72, 0x94, 0x9D, 0x44, 
			0xD0, 0xC0, 0x22, 0x4C, 
			0xF7, 0x2A, 0x78, 0xE7, 
			0x53, 0xFA, 0x1D, 0x68 
		};
		
		const uint8_t defaultAesIv[] =
		{
			0x33, 0x38, 0x36, 0xE8,
			0x59, 0xCD, 0x2E, 0x26, 
			0x4C, 0x1F, 0x9E, 0xAC, 
			0x5E, 0xB9, 0x3A, 0x3D
		};
		
		memcpy(userSettings.AesKey, defaultAesKey, sizeof(defaultAesKey));
		memcpy(userSettings.AesIV, defaultAesIv, sizeof(defaultAesIv));
	}
	
	AES_generateSBox(); //init sandbox for AES encryption
	network.enableEncryption(userSettings.AesKey, userSettings.AesIV, AES_MODE_128); //enable encryption for network

	radio.begin(); //init radio
	network.begin(userSettings.Channel, userSettings.Address); //init network

	Plugin &plugin = Plugin::Load(network, (NodeType)userSettings.NodeType);
	plugin.Init();
	
	while (true)
	{
		network.update();	
		ProcessSerialPackets(plugin);
		ProcessNetworkPackets(plugin);
		bool canSleep = plugin.Loop();
		
		if (canSleep && (userSettings.NodeType & EnableSleepMode))
		{
			serialFlushTx();
			radio.powerDown();
			timerDisable();
			
			Cpu_SetOperationMode(DOM_SLEEP, NULL, NULL);
			
			timerEnable();
			radio.powerUp();
		}
	}
}
