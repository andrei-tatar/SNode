#include "hal.h"

static RF24Network *gNnetwork; 

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

#define CMD_SENDPACKET 	0x31
#define CMD_PING_ECHO	0xDE

void packetReceived(uint8_t cmd, uint8_t *data, int length)
{
	switch (cmd)
	{
	//send back an echo
	case CMD_PING_ECHO:
		serialSendPacket(CMD_PING_ECHO, data, length);
		break;
		
	case CMD_SENDPACKET:
		RF24NetworkHeader header(data[0] | (data[1] << 8));
		gNnetwork->write(header, &data[2], length - 2);
		break;
	}
}

void sNodeMain(void)
{		
	#if 1
	while (true)
	{
		/*8int byte;
		while (true)
		{
			byte = serialReadByte();
			if (byte != -1)
				IF_SERIAL_DEBUG(serialByte(byte));
			else
				break;
		}*/
		
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
	gNnetwork = &network;
	
	AES_generateSBox();
	network.enableEncryption(key, iv, AES_MODE_128);

	const bool isReceiver = true;
	uint8_t rxAddress = 0, txAddress = 01;
	
	radio.begin();
	network.begin(90, isReceiver ? rxAddress : txAddress);

	uint32_t last_sent;
	uint32_t led1OffTime = 0, led2OffTime = 0;
	while (true)
	{
		network.update();
	
		uint32_t now = getTime();
		if (now > led1OffTime) LED_OFF(LED1);
		if (now > led2OffTime) LED_OFF(LED2);
		
		// If it's time to send a message, send it!
		if (!isReceiver && now - last_sent >= 1000 )
		{
			LED_ON(LED1);
			last_sent = now;
			
			RF24NetworkHeader header(rxAddress);
			bool ok = network.write(header, &now, sizeof(uint32_t));
			
			led1OffTime = getTime() + (ok ? 100 : 0);
		}
		
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
	}
	#endif
}
