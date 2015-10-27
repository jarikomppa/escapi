/* "simplest", example of simply enumerating the available devices with ESCAPI */
#include <stdio.h>
#include "escapi.h"

void main()
{
	int i, j;
	
	/* Initialize ESCAPI */
	
	int devices = setupESCAPI();

	if (devices == 0)
	{
		printf("ESCAPI initialization failure or no devices found.\n");
		return;
	}

  /* Set up capture parameters.
   * ESCAPI will scale the data received from the camera 
   * (with point sampling) to whatever values you want. 
   * Typically the native resolution is 320*240.
   */

	struct SimpleCapParams capture;
	capture.mWidth = 24;
	capture.mHeight = 18;
	capture.mTargetBuf = new int[24 * 18];
	
	/* Initialize capture - only one capture may be active per device,
	 * but several devices may be captured at the same time. 
	 *
	 * 0 is the first device.
	 */
	
	if (initCapture(0, &capture) == 0)
	{
		printf("Capture failed - device may already be in use.\n");
		return;
	}
	
	/* Go through 10 capture loops so that the camera has
	 * had time to adjust to the lighting conditions and
	 * should give us a sane image..	 
	 */
	for (i = 0; i < 10; i++)
	{
		/* request a capture */			
		doCapture(0);
		
		while (isCaptureDone(0) == 0)
		{
			/* Wait until capture is done.
			 * Warning: if capture init failed, or if the capture
			 * simply fails (i.e, user unplugs the web camera), this
			 * will be an infinite loop.
			 */		   
		}
	}
	
	/* now we have the data.. what shall we do with it? let's 
	 * render it in ASCII.. (using 3 top bits of green as the value)
	 */
	char light[] = " .,-o+O0@";
	for (i = 0; i < 18; i++)
	{
		for (j = 0; j < 24; j++)
		{
			printf("%c", light[(capture.mTargetBuf[i*24+j] >> 13) & 7]);
		}
		printf("\n");
	}
	
	deinitCapture(0);	
}