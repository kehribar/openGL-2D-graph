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
#include <sys/stat.h>

int n;
int port;
int flags;
unsigned char buf[16384];
struct termios settings;

void fillScreenBuffer(uint16_t* screenBufferPointer,uint16_t size,uint16_t topValue)
{

}

// return 0 for successful hardware/software platform initialization
int platform_init(int argc, char** argv)
{
	
	for (int i = 0; i < argc; ++i)
	{
		printf("[%d] %s\n",i,argv[i]);
	}
		
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

	return 0;
}
