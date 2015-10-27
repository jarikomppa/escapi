/* "enumprops", example of querying properties from a camera */
#include <stdio.h>
#include "escapi.h"

void printprop(int device, int prop, const char* propname)
{
	float v = getCapturePropertyValue(device, prop);
	if (v < 0) 
	{ 
		printf("   (n/a) %s\n", propname); 
	} 
	else 
	{ 
		int a = getCapturePropertyAuto(device, prop);
		printf("%7.2f%% %s%s\n", v * 100, propname, a ? " (Auto)" : "");
	}
}

void main()
{
	int devices = setupESCAPI();

	if (devices == 0)
	{
		printf("ESCAPI initialization failure or no devices found.\n");
		return;
	}

	struct SimpleCapParams capture;
	capture.mWidth = 320;
	capture.mHeight = 240;
	capture.mTargetBuf = new int[320 * 240];

	int i;
	for (i = 0; i < devices; i++)
	{
		char temp[256];
		getCaptureDeviceName(i, temp, 256);
		printf("Device %d: \"%s\"\n", i, temp);

		// To access the properties, we need to open the camera
		// (by initCapture)

		if (initCapture(i, &capture) == 0)
		{
			printf("\tCan't open device\n");
		}
		else
		{
#define PRINTPROP(x) printprop(i, x, #x);
			PRINTPROP(CAPTURE_BRIGHTNESS);			
			PRINTPROP(CAPTURE_CONTRAST);
			PRINTPROP(CAPTURE_HUE);
			PRINTPROP(CAPTURE_SATURATION);
			PRINTPROP(CAPTURE_SHARPNESS);
			PRINTPROP(CAPTURE_GAMMA);
			PRINTPROP(CAPTURE_COLORENABLE);
			PRINTPROP(CAPTURE_WHITEBALANCE);
			PRINTPROP(CAPTURE_BACKLIGHTCOMPENSATION);
			PRINTPROP(CAPTURE_GAIN);
			PRINTPROP(CAPTURE_PAN);
			PRINTPROP(CAPTURE_TILT);
			PRINTPROP(CAPTURE_ROLL);
			PRINTPROP(CAPTURE_ZOOM);
			PRINTPROP(CAPTURE_EXPOSURE);
			PRINTPROP(CAPTURE_IRIS);
			PRINTPROP(CAPTURE_FOCUS);
			deinitCapture(i);
		}
	}
}