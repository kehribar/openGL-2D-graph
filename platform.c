/*-----------------------------------------------------------------------------
/ openGL playground
/------------------------------------------------------------------------------
/ Fill this file with your USB / RS232 or etc. based hardware platform 
/ specific functions and callbacks.
/------------------------------------------------------------------------------
/ ihsan Kehribar - 2014
/----------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdint.h>
#include <math.h>

#define PI 3.14159265358979323846

void fillScreenBuffer(uint16_t* screenBufferPointer,uint16_t size,uint16_t topValue)
{
	for (int i = 0; i < size; ++i)
	{
		// synthetic sinus signal
		screenBufferPointer[i] = ((double)(topValue/2) * sin(2*PI*((double)i/(double)size))) + topValue/2;
	}
}

// return 0 for successful hardware/software platform initialization
int platform_init(int argc, char** argv)
{
	/*
	for (int i = 0; i < argc; ++i)
	{
		printf("[%d] %s\n",i,argv[i]);
	}
	*/

	return 0;
}
