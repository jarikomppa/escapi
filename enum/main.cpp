/* "enum", example of simply enumerating the available devices with ESCAPI */
#include <stdio.h>
#include "escapi.h"

void main()
{
	int devices = setupESCAPI();

	if (devices == 0)
	{
		printf("ESCAPI initialization failure or no devices found.\n");
		return;
	}

	int i;
	for (i = 0; i < devices; i++)
	{
		char temp[256];
		getCaptureDeviceName(i, temp, 256);
		printf("Device %d: \"%s\"\n", i, temp);
	}

}