/*-----------------------------------------------------------------------------
/ openGL playground
/------------------------------------------------------------------------------
/ Fill this file with your USB / RS232 or etc. based hardware platform 
/ specific functions and callbacks.
/------------------------------------------------------------------------------
/ ihsan Kehribar - 2014
/----------------------------------------------------------------------------*/
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include "globalDefinitions.h"

#define SHIFT_SIZE 256

// min-max definitions for the incoming data stream
const uint16_t inputMin = 0;
const uint16_t inputMax = 1024;

// serial port file descriptor
int port;

// these will be calculated by the platform_init function
int offset;
double gain;

// main serial port buffer 
uint8_t buf[2*SHIFT_SIZE];

// this function is automatically called when the openGL engine is idle
void fillScreenBuffer(uint16_t* screenBufferPointer,uint16_t size,uint16_t topValue)
{
	uint16_t tmpVal;
	int bytes_avail;

	// read available bytes count in the kernel buffer
	ioctl(port, FIONREAD, &bytes_avail);

	// do we have enough data in the kernel buffer?
	if(bytes_avail >= (2*SHIFT_SIZE))
	{
		// read the bytes
		read(port, buf, (2*SHIFT_SIZE));
		
		// shift the screen buffer by the amount of new data
		for (int i = 0; i < (size-SHIFT_SIZE); ++i)
		{
			screenBufferPointer[i] = screenBufferPointer[i + SHIFT_SIZE];
		}

		// load the new data to the screen buffer
		for (int i = 0; i < SHIFT_SIZE; ++i)
		{
			// cast two 8bits data to single 16bits variable
			tmpVal = (uint16_t)buf[(2*i)+1] + (uint16_t)((uint16_t)buf[(2*i)+0]<<8);
			
			// load the data
			screenBufferPointer[i + size - SHIFT_SIZE] = (uint16_t)((double)(tmpVal-offset) * gain);
		}
	}
}

// return 0 for successful hardware/software platform initialization
int platform_init(int argc, char** argv)
{
	int flags;
	struct termios settings;

	#if 0
		for (int i = 0; i < argc; ++i)
		{
			printf("[%d] %s\n",i,argv[i]);
		}
	#endif
		
	if (argc < 2) 
	{
		printf("Example: serial_listen [port_path]\n");
		return -1;
	}

	// Open the serial port
	port = open(argv[1], O_RDWR);
	if (port < 0) 
	{
		printf("Unable to open %s\n", argv[1]);
		return -1;
	}

	// Configure the port
	flags = fcntl(port, F_GETFL, 0);
	fcntl(port, F_SETFL, flags | O_NONBLOCK);
	tcgetattr(port, &settings);
	cfmakeraw(&settings);
	tcsetattr(port, TCSANOW, &settings);

	// calculate the gain
	offset = inputMin;
	gain = (double)HEIGHT/(double)(inputMax-inputMin);

	return 0;
}
