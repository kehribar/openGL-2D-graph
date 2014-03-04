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

#define SHIFT_SIZE 32

// min-max definitions for the incoming data stream
const uint16_t inputMin = 0;
const uint16_t inputMax = 65535;

// real life converters
double adcStep_mV = 5000.0 / 65536.0;
double timeStep_ms = 1.0 / 25.0;

// serial port file descriptor
int port;

// these will be calculated by the platform_init function
int offset;
double gain;

// main serial port buffer 
uint8_t buf[2*SHIFT_SIZE];

// pre-trigger screen buffer
uint8_t triggerState = 1;
const uint16_t preTriggerSize = 256;
uint16_t preTriggerBuff[preTriggerSize];
uint16_t triggerValue = 99;
uint16_t globalIndex = 0;
uint16_t holdoff = 0;

// ...
extern uint8_t pause_acquisition;
extern uint8_t acq_mode;

int screen_offset = 0;
int screen_offset_step = 10;

void setTrigger(int x,int y)
{
	triggerValue = ((double)(y-screen_offset) / gain) + offset;
	printf("trigVal: %d\n",triggerValue);

	acq_mode = 1;
}

void increaseOffset()
{
	if(screen_offset < (HEIGHT-screen_offset_step))
	{
		screen_offset += screen_offset_step;
		printf("%d\n",screen_offset);
	}
}

void decreaseOffset()
{
	if(screen_offset > (-1 * (HEIGHT-screen_offset_step)))
	{
		screen_offset -= screen_offset_step;
		printf("%d\n",screen_offset);	
	}
}

// this function is automatically called when the openGL engine is idle
void fillScreenBuffer(uint16_t* screenBufferPointer,uint16_t size,uint16_t topValue)
{
	// free running mode
	if(acq_mode == 0)
	{
		uint16_t tmpVal;
		int bytes_avail;
		uint8_t run = 1;

		while(run)
		{
			// read available bytes count in the kernel buffer
			ioctl(port, FIONREAD, &bytes_avail);
			// printf("%d\n",bytes_avail);

			// do we have enough data in the kernel buffer?
			if(bytes_avail >= (2*SHIFT_SIZE))
			{
				// read the bytes
				read(port, buf, (2*SHIFT_SIZE));

				// ioctl(port, FIONREAD, &bytes_avail);
				// printf("%d\n",bytes_avail);

				if(!pause_acquisition)
				{		
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

						// printf("%d\n",tmpVal);
						
						// load the data
						screenBufferPointer[i + size - SHIFT_SIZE] = (uint16_t)((double)(tmpVal-offset) * gain) + screen_offset;
					}
				}
			}
			else
			{
				run = 0;
			}
		}
	}
	// trigger mode
	else if(acq_mode == 1)
	{
		uint8_t run = 1;
		uint16_t tmpVal;
		int bytes_avail;
		uint16_t triggerLocation;
		uint8_t tempBuffer_8b[64];
		uint16_t tempBuffer_16b[32];		

		while(run)
		{
			// read available bytes count in the kernel buffer
			ioctl(port, FIONREAD, &bytes_avail);		

			// check for new packet
			if(bytes_avail >= 64)
			{		
				// read the bytes
				read(port, tempBuffer_8b, 64);

				if((triggerState != 3) && (holdoff == 0))
				{
					// convert 8bit datas into 16bit
					for (int i = 0; i < 32; ++i)
					{
						tmpVal = (uint16_t)tempBuffer_8b[(2*i)+1] + (uint16_t)((uint16_t)tempBuffer_8b[(2*i)+0]<<8);

						tempBuffer_16b[i] = tmpVal;

						if((i > 1) && (triggerState == 0))
						{						
							if(((tempBuffer_16b[i-2]) <= triggerValue) && (tempBuffer_16b[i] >= triggerValue))
							{
								triggerLocation = i-1;
								triggerState = 1; /* found */

								// globalIndex = WIDTH / 4;								
							}			
						}				
					}
					
					// Found ...
					if(triggerState == 1)
					{
						for (int i = triggerLocation; i < 32; ++i)
						{
							screenBufferPointer[globalIndex++] = (uint16_t)((double)(tempBuffer_16b[i]-offset) * gain) + screen_offset;
						}
						triggerState = 2;	
					}	
					// Fill the rest ...
					else if(triggerState == 2)
					{
						int i = 0;
						
						while((triggerState==2) && (i<32))
						{
							if (globalIndex >= WIDTH)
							{
								#if 1
									holdoff = 15;
								#endif
								globalIndex = 0;
								triggerState = 0;								
								// triggerState = 3;				
							}
							else
							{
								screenBufferPointer[globalIndex++] = (uint16_t)((double)(tempBuffer_16b[i++]-offset) * gain) + screen_offset; 
							}				
						}
					}			
				}		
			}
			else
			{
				if(holdoff != 0)
				{
					holdoff--;
				}				
				// printf("%d\n",bytes_avail);
				run = 0;
			}
		}
	}	
}

// return 0 for successful hardware/software platform initialization
int platform_init(int argc, char** argv)
{
	#if 1
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
		port = open(argv[1], O_RDWR | O_NOCTTY | O_NDELAY);
		if (port < 0) 
		{
			printf("Unable to open %s\n", argv[1]);
			return -1;
		}

		// Configure the port
		flags = fcntl(port, F_GETFL, 0);
		fcntl(port, F_SETFL, flags | FNDELAY | O_NONBLOCK);
		tcgetattr(port, &settings);
		cfmakeraw(&settings);
		tcsetattr(port, TCSANOW, &settings);

	#endif

	// calculate the gain
	offset = inputMin;
	gain = (double)HEIGHT/(double)(inputMax-inputMin);

	return 0;
}

void fillMessage_timeDiff(char* targetBuffer, uint16_t x1_cursor, uint16_t x2_cursor)
{
	double timeDiff;

	targetBuffer[0] = 0x00;	

	if(x1_cursor > x2_cursor)
	{
		timeDiff = (double)(x1_cursor-x2_cursor)*timeStep_ms;
	}
	else
	{		
		timeDiff = (double)(x2_cursor-x1_cursor)*timeStep_ms;	
	}
	
	sprintf(targetBuffer,"%4.2f ms",timeDiff);
}

void fillMessage_freqDiff(char* targetBuffer, uint16_t x1_cursor, uint16_t x2_cursor)
{
	double timeDiff;
	double freq;

	targetBuffer[0] = 0x00;	

	if(x1_cursor > x2_cursor)
	{
		timeDiff = (double)(x1_cursor-x2_cursor)*timeStep_ms;
	}
	else
	{		
		timeDiff = (double)(x2_cursor-x1_cursor)*timeStep_ms;	
	}

	freq = 1.0 / timeDiff;
	
	sprintf(targetBuffer,"%4.2f kHz",freq);
}

void fillMessageY1(char* targetBuffer, uint16_t y1_cursor)
{
	double realVoltage;

	targetBuffer[0] = 0x00;

	realVoltage = (double)(((double)y1_cursor / gain) + offset) * adcStep_mV;

	sprintf(targetBuffer,"%4.2f mV",realVoltage);
}

void fillMessageY2(char* targetBuffer, uint16_t y2_cursor)
{
	double realVoltage;

	targetBuffer[0] = 0x00;

	realVoltage = (double)(((double)y2_cursor / gain) + offset) * adcStep_mV;

	sprintf(targetBuffer,"%4.2f mV",realVoltage);
}
