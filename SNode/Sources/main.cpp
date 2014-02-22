#include "hal.h"
#include "main.h"
#include "Commands.h"

//stupid bug where AES freezes because the variables are not aligned
//TODO: test if still needs aligned
const uint8_t key[] __attribute__ ((aligned (4))) =	
			{
					0x2b,0x7e,0x15,0x16,0x28,0xae,0xd2,0xa6,
					0xab,0xf7,0x15,0x88,0x09,0xcf,0x4f,0x3c
			};
const uint8_t iv[] =
			{
					0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,
					0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f
			};

ReceivedPacket queue[QUEUE_SIZE];
volatile uint8_t queueWritePos = 0; 
uint8_t queueReadPos = 0;
volatile uint8_t queueCount = 0;

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

void sNodeMain(void)
{		
	#if 0
	while (true)
	{
		LED_ON(LED1);
		delay_ms(500);
		LED_OFF(LED1);
		LED_ON(LED2);
		delay_ms(500);
		LED_OFF(LED2);
	}
	#endif
		
	#if 1
	RF24 radio;	
	RF24Network network = RF24Network(radio);
	
	AES_generateSBox();
	network.enableEncryption(key, iv, AES_MODE_128);

	uint8_t address = 0, channel = 90;
	
	radio.begin();
	network.begin(channel, address);

	serialSendPacket(CMD_PING_ECHO, NULL, 0);
	
	uint32_t led1OffTime = 0, led2OffTime = 0;
	
	while (true)
	{
		network.update();
	
		uint32_t now = getTime();
		if (now > led1OffTime) LED_OFF(LED1);
		if (now > led2OffTime) LED_OFF(LED2);
		
		//check if any message available
		while (network.available())
		{
			LED_ON(LED2);
			led2OffTime = now + 100;
			
			RF24NetworkHeader header;
			uint8_t buffer[26]; //2 bytes from address and 24 bytes data 
			network.read(header, &buffer[2], 24);
			buffer[0] = (header.from_node >> (8 * 0)) & 0xFF;
			buffer[1] = (header.from_node >> (8 * 1)) & 0xFF;
			serialSendPacket(CMD_SENDPACKET, buffer, sizeof(buffer));
		}
		
		__DI();
		bool packetsReceived = queueCount != 0;
		__EI();
		
		if (packetsReceived)
		{
			switch (queue[queueReadPos].Cmd)
			{
			//send back an echo
			case CMD_PING_ECHO:
				serialSendPacket(CMD_PING_ECHO, queue[queueReadPos].Data, queue[queueReadPos].Length);
				break;
				
			case CMD_SENDPACKET:
				LED_ON(LED1);
				RF24NetworkHeader header(queue[queueReadPos].Data[0] | (queue[queueReadPos].Data[1] << 8));
				bool ok = network.write(header, &queue[queueReadPos].Data[2], queue[queueReadPos].Length - 2);
				led1OffTime = getTime() + (ok ? 100 : 0);
				break;
			}
			
			queueReadPos = (queueReadPos + 1) & QUEUE_SIZE_MASK;
			__DI();
			queueCount--;
			__EI();
		}
	}
	#endif
}
