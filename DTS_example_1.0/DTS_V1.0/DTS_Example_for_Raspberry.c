/*
 *	description : DTS Example Code.
 *          Version : 1.0
 *          Board : Raspberry 2 Model B V1.1
 *          Support Module : DTS-SIL300-B, DTS-L300-V2
 * 	Further information:
 *		https://projects.drogon.net/raspberry-pi/wiringpi/
 *		https://www.diwellshop.com/
 * 	Revision history.
 * 	1.0 2020.01.02 : First version is released.
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "wiringPi.h"
#include "wiringPiSPI.h"

#define LED		21			// LED PORT NUM 21
#define SCE		22			// SCE PORT NUM 22
#define spi_chn0		0			// SPI Channel 0
//#define SPEED_500Khz	500000		// SPI Speed
#define SPEED_1MHz	1000000		// SPI Speed
#define SPI_MODE3	3			// SPI MODE
#define OBJECT	0xA0		// COMMAND(Read Object Temp.)
#define SENSOR	0xA1		// COMMAND(Read Sensor Temp.)

int16_t iSensor, iObject;

int16_t SPI_COMMAND(uint8_t ADR)
{
	uint8_t Data_Buf[3];

	Data_Buf[0] = ADR;
	Data_Buf[1] = 0x22;
	Data_Buf[2] = 0x22;
	
	digitalWrite(SCE, 0);  				// SCE LOW
	delayMicroseconds(10);				// delay 10us

	wiringPiSPIDataRW (spi_chn0, Data_Buf, 1);		// transfer 1st byte.
	delayMicroseconds(10);				// delay 10us
	wiringPiSPIDataRW (spi_chn0, Data_Buf+1, 1);	            // transfer 2nd byte
	delayMicroseconds(10);				// delay 10us
	wiringPiSPIDataRW (spi_chn0, Data_Buf+2, 1);		// transfer 3rd byte
	delayMicroseconds(10);				// delay 10us

	digitalWrite(SCE, 1);  				// SCE HIGH
	return (Data_Buf[2]*256+Data_Buf[1]);			// High + Lo byte
}

int main(void)
{
	wiringPiSetup();					// Wiring Pi setup
 	if(wiringPiSetupGpio() == -1)
	return 1;
	pinMode(LED, OUTPUT);				// LED Port Output (not necessary)
	pinMode(SCE, OUTPUT);				// SCE Port Output
	digitalWrite(SCE,1);					// SCE high

	wiringPiSPISetupMode(spi_chn0, SPEED_1MHz, SPI_MODE3); //SPI0, 1Mhz, SPI Mode3 Setting
	delay(500);					// wait 500ms
	while(1)
	{
		iSensor = SPI_COMMAND(SENSOR);		// Read Sensor temp.
		digitalWrite(LED, 1);			// LED ON(not necessary)
		delayMicroseconds(10);			// delay 10us	
		iObject = SPI_COMMAND(OBJECT);		// Read Object temp.
		digitalWrite(LED,0);				// LED OFF(not necessary)
		delay(500);				// Wait 500ms
		printf("Sensor : %5.2f  , Object : %5.2f \n", (double)iSensor/100, (double)iObject/100);
	}
	return 0;
}