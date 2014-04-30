#include "OutputsPlugin.h"

#include "IO_Map.h"
#include "hal.h"

OutputsPlugin::OutputsPlugin(RF24Network &network)
:Plugin(network)
{ }

#define O0	(1 << 0) //PE0
#define P0	(PTE_BASE_PTR)

#define O1	(1 << 1) //PE0
#define P1	(PTE_BASE_PTR)

#define O2	(1 << 6) //PD6
#define P2	(PTD_BASE_PTR)

#define O3	(1 << 7) //PD7
#define P3	(PTD_BASE_PTR)

void OutputsPlugin::Init()
{
	//enable clock to PORTD and PORTE
	SIM_SCGC5 |= SIM_SCGC5_PORTD_MASK | SIM_SCGC5_PORTE_MASK;
	
	/* Configure pin as output */
	/* GPIOE_PDDR: PDD|=1 */
	GPIOE_PDDR |= GPIO_PDDR_PDD(0x01);                                   
	/* Set initialization value */
	/* GPIOE_PDOR: PDO&=~1 */
	GPIOE_PDOR &= (uint32_t)~(uint32_t)(GPIO_PDOR_PDO(0x01));                                   
	/* Initialization of Port Control register */
	/* PORTE_PCR0: ISF=0,MUX=1 */
	PORTE_PCR0 = (uint32_t)((PORTE_PCR0 & (uint32_t)~(uint32_t)(
			PORT_PCR_ISF_MASK |
			PORT_PCR_MUX(0x06)
	)) | (uint32_t)(
			PORT_PCR_MUX(0x01)
	));

	/* Configure pin as output */
	/* GPIOE_PDDR: PDD|=2 */
	GPIOE_PDDR |= GPIO_PDDR_PDD(0x02);                                   
	/* Set initialization value */
	/* GPIOE_PDOR: PDO&=~2 */
	GPIOE_PDOR &= (uint32_t)~(uint32_t)(GPIO_PDOR_PDO(0x02));                                   
	/* Initialization of Port Control register */
	/* PORTE_PCR1: ISF=0,MUX=1 */
	PORTE_PCR1 = (uint32_t)((PORTE_PCR1 & (uint32_t)~(uint32_t)(
			PORT_PCR_ISF_MASK |
			PORT_PCR_MUX(0x06)
	)) | (uint32_t)(
			PORT_PCR_MUX(0x01)
	));     
	
	/* Configure pin as output */
	/* GPIOD_PDDR: PDD|=0x40 */
	GPIOD_PDDR |= GPIO_PDDR_PDD(0x40);                                   
	/* Set initialization value */
	/* GPIOD_PDOR: PDO&=~0x40 */
	GPIOD_PDOR &= (uint32_t)~(uint32_t)(GPIO_PDOR_PDO(0x40));                                   
	/* Initialization of Port Control register */
	/* PORTD_PCR6: ISF=0,MUX=1 */
	PORTD_PCR6 = (uint32_t)((PORTD_PCR6 & (uint32_t)~(uint32_t)(
			PORT_PCR_ISF_MASK |
			PORT_PCR_MUX(0x06)
	)) | (uint32_t)(
			PORT_PCR_MUX(0x01)
	));    

	/* Configure pin as output */
	/* GPIOD_PDDR: PDD|=0x80 */
	GPIOD_PDDR |= GPIO_PDDR_PDD(0x80);                                   
	/* Set initialization value */
	/* GPIOD_PDOR: PDO&=~0x80 */
	GPIOD_PDOR &= (uint32_t)~(uint32_t)(GPIO_PDOR_PDO(0x80));                                   
	/* Initialization of Port Control register */
	/* PORTD_PCR7: ISF=0,MUX=1 */
	PORTD_PCR7 = (uint32_t)((PORTD_PCR7 & (uint32_t)~(uint32_t)(
			PORT_PCR_ISF_MASK |
			PORT_PCR_MUX(0x06)
	)) | (uint32_t)(
			PORT_PCR_MUX(0x01)
	));                               
}

void OutputsPlugin::OnNetworkPacketReceived(RF24NetworkHeader &header, const uint8_t *data, uint8_t length)
{
	LED_TOGGLE(LED1);
	switch (data[0])
	{
	case SNODE_CMD_OUTPUT:
		LED_TOGGLE(LED2);
		
		if (data[1] & 0x01) P0->PSOR = O0; else P0->PCOR = O0;
		if (data[1] & 0x02) P1->PSOR = O1; else P1->PCOR = O1;
		if (data[1] & 0x04) P2->PSOR = O2; else P2->PCOR = O2;
		if (data[1] & 0x08) P3->PSOR = O3; else P3->PCOR = O3;
		
		break;
	}
}
