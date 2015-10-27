/* "multicap", multiple capture example for ESCAPI */
#include <stdio.h>
#include "SDL.h"

#include "escapi.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// SDL stuff
SDL_Window *gSdlWindow;
SDL_Renderer *gSdlRenderer;
SDL_Texture *gSdlScreenTexture;
unsigned int *gSdlScreenPixels;

// Font used to print device names
int font_x, font_y, font_comp;
unsigned int *font = 0;

// Capture structures
struct SimpleCapParams capture[4];

// Number of devices
int devices = 0;

// Device names (overwritten with what's queried from devices)
char devicenames[4][24] = { "device 1", "device 2", "device 3", "device 4" };

unsigned int saturated_add(int aPix1, int aPix2)
{
	int r = ((aPix1 >> 0) & 0xff) + ((aPix2 >> 0) & 0xff);
	if (r > 0xff) r = 0xff;

	int g = ((aPix1 >> 8) & 0xff) + ((aPix2 >> 8) & 0xff);
	if (g > 0xff) g = 0xff;

	int b = ((aPix1 >> 16) & 0xff) + ((aPix2 >> 16) & 0xff);
	if (b > 0xff) b = 0xff;

	return 0xff000000 | (b << 16) | (g << 8) | (r << 0);
}

unsigned int saturated_sub(int aPix1, int aPix2)
{
	int r = ((aPix1 >> 0) & 0xff) - ((aPix2 >> 0) & 0xff);
	if (r < 0) r = 0;

	int g = ((aPix1 >> 8) & 0xff) - ((aPix2 >> 8) & 0xff);
	if (g < 0) g = 0;

	int b = ((aPix1 >> 16) & 0xff) - ((aPix2 >> 16) & 0xff);
	if (b < 0) b = 0;

	return 0xff000000 | (b << 16) | (g << 8) | (r << 0);

}


// Simple printer, draws one character, used by drawstring()
void drawchar(char ch, int x, int y)
{
	int i, j;
	for (i = 0; i < 24; i++)
	{
		for (j = 0; j < font_x; j++)
		{
			unsigned int p = font[(i + (ch - 32) * 24) * font_x + j];
			unsigned int b = gSdlScreenPixels[(y + i) * 640 + x + j];
			unsigned int c = gSdlScreenPixels[(y + i + 2) * 640 + x + j + 2];
			gSdlScreenPixels[(y + i) * 640 + x + j] = saturated_add(b, p);
			gSdlScreenPixels[(y + i + 2) * 640 + x + j + 2] = saturated_sub(c, p);
		}
	}
}

// Simple way to print text strings.
void drawstring(char * string, int x, int y)
{
	if (!font) // Don't crash if font wasn't loaded
		return;

	while (*string)
	{
		drawchar(*string, x, y);
		x += 14;
		string++;
	}
}

// Main rendering function
void render()
{
	// Declare a couple of variables
	int i, j, k;

	for (k = 0; k < devices; k++)
	{
		if (isCaptureDone(k))
		{
			int linofs = 0;
			// Arrange devices in a grid
			int xofs = (k & 1) ? 320 : 0;
			int yofs = (k & 2) ? (240 * 640) : 0;
			int ofs;

			// Draw to gSdlScreenTexture
			for (i = 0; i < 240; i++)
			{
				for (j = 0, ofs = yofs + xofs; j < 320; j++, ofs++, linofs++)
				{
					gSdlScreenPixels[ofs] = capture[k].mTargetBuf[linofs];
				}
				yofs += 640;
			}

			// Ask for another frame
			doCapture(k);

			// Draw the device's name over the captured image
			drawstring(devicenames[k], xofs, (k & 2) ? 240 : 0);
		}
	}

	// Don't hog all CPU time
	SDL_Delay(1);

	SDL_UpdateTexture(gSdlScreenTexture, NULL, gSdlScreenPixels, 640 * sizeof(Uint32));
	SDL_RenderCopy(gSdlRenderer, gSdlScreenTexture, NULL, NULL);


	// Tell SDL to update the whole gSdlScreenTexture
	SDL_RenderPresent(gSdlRenderer);

}


// Entry point
int main(int argc, char *argv[])
{
	// Initialize SDL's subsystems - in this case, only video.
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		printf("Unable to init SDL: %s\n", SDL_GetError());
		exit(1);
	}

	// Register SDL_Quit to be called at exit; makes sure things are
	// cleaned up when we quit.
	atexit(SDL_Quit);

	// Setup ESCAPI; returns number of available devices
	devices = setupESCAPI();
	if (devices < 1)
	{
		printf("Unable to init ESCAPI\n");
		exit(1);
	}

	// We only want max. 4 devices, so cap if there's more..
	if (devices > 4)
		devices = 4;

	// Set up capture for the available devices (and ask for data)
	for (int i = 0; i < devices; i++)
	{
		capture[i].mWidth = 320;
		capture[i].mHeight = 240;
		capture[i].mTargetBuf = new int[320 * 240];
		getCaptureDeviceName(i, devicenames[i], 24);
		initCapture(i, &capture[i]);
		doCapture(i);
	}

	// Attempt to create a 640x480 gSdlWindow with 32bit gSdlScreenPixels.
	gSdlWindow = SDL_CreateWindow(
		"ESCAPI MultiCap",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		640, 480, 0);

	// If we fail, return error.
	if (gSdlWindow == NULL)
	{
		printf("Unable to set 640x480 video: %s\n", SDL_GetError());
		exit(1);
	}

	gSdlRenderer = SDL_CreateRenderer(gSdlWindow, -1, 0);

	gSdlScreenTexture = SDL_CreateTexture(gSdlRenderer,
		SDL_PIXELFORMAT_ARGB8888,
		SDL_TEXTUREACCESS_STREAMING,
		640, 480);

	gSdlScreenPixels = new unsigned int[640 * 480];

	// load font
	font = (unsigned int *)stbi_load("font14x24.png", &font_x, &font_y, &font_comp, 4);

	// Main loop: loop forever.
	while (1)
	{
		// Render stuff
		render();

		// Poll for events, and handle the ones we care about.
		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
			case SDL_KEYDOWN:
				break;
			case SDL_KEYUP:
				// If escape is pressed, return (and thus, quit)
				if (event.key.keysym.sym == SDLK_ESCAPE)
				{
					int k;
					for (k = 0; k < devices; k++)
						deinitCapture(k);
					return 0;
				}
				break;
			case SDL_QUIT:
				return(0);
			}
		}
	}
	return 0;
}

