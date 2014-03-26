/*
 * main.h
 *
 *  Created on: Feb 22, 2014
 *      Author: X550L-User1
 */

#ifndef MAIN_H_
#define MAIN_H_

#include <stdint.h>
#include "serial.h"

typedef struct 
{
	uint8_t Cmd;
	uint8_t Data[MAX_RX_SIZE];
	uint8_t Length;
} ReceivedPacket;

#define QUEUE_SIZE 		8
#define QUEUE_SIZE_MASK	(QUEUE_SIZE - 1)

namespace SNodeChange
{
	typedef enum SNodeContent
	{
		NoChanges = 0x00,
		Adc0Value = 0x01,
		Adc1Value = 0x02,
		Adc2Value = 0x04,
		Adc3Value = 0x08,
		InputValues = 0x10,
	} SNodeContent;
}

namespace NodeConfigType
{
	typedef enum NodeConfigurationType
	{
		RedirectPacketsToSerialPort = 0x80000000,
		EnableSleepMode = 0x40000000,
		WakeOnlyFromInterrupts = 0x20000000,
		CustomNodeType = 0x10000000,
		
		InputEnableMask = 0xF,
		Input0Enable = 0x1,
		Input1Enable = 0x2,
		Input2Enable = 0x4,
		Input3Enable = 0x8,

		AdcEnableMask = 0xF << 4,
		Adc0Enable = 0x1 << 4,
		Adc1Enable = 0x2 << 4,
		Adc2Enable = 0x4 << 4,
		Adc3Enable = 0x8 << 4,

		PwmEnableMask = 0xF << 8,
		Pwm0Enable = 0x1 << 8,
		Pwm1Enable = 0x2 << 8,
		Pwm2Enable = 0x4 << 8,
		Pwm3Enable = 0x8 << 8,

		SleepTimeMask = 0x3F << 12,
		SleepMultiplierMask = 0xC0 << 12,
		SleepMultiplier1Xsec = 0x0 << 18,
		SleepMultiplier10Xsec = 0x1 << 18,
		SleepMultiplier100Xsec = 0x2 << 18,
		SleepMultiplier1000Xsec = 0x3 << 18,
	} NodeConfigurationType;
}

typedef struct
{
#define SETTINGS_MAGIC_NUMBER	0x48C6FAF0 
	
	uint32_t MagicNumber;
	uint32_t HashCode;
	uint8_t AesKey[16];
	uint8_t AesIV[16];
	
	NodeConfigType::NodeConfigurationType NodeType;
	
	uint16_t Address;
	uint8_t Channel;
} UserSettings;

void sNodeMain(void);

extern bool FlashOperationComplete;
extern volatile bool InputPinsChanged;

#endif /* MAIN_H_ */
