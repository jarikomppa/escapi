/* "ogl", example of using OpenGL with ESCAPI to create a "funny mirror" effect */

#include <SDL.h>
#include <SDL_opengl.h>
#include <math.h>
#include <stdio.h>
#include "escapi.h"
struct SimpleCapParams capture;
int device;
SDL_Window *window;

void render()
{
	// slow the effect down a bit..
	float tick = SDL_GetTicks() * 0.2f;
	int i, j;

	// Generate a 16x16 grid, and perturb the UV coordinates to 
	// generate the "funny mirrors" effect.
	glBegin(GL_TRIANGLE_STRIP);
	for (j = 0; j < 16; j++)
	{
		for (i = 0; i < 17; i++)
		{
			float u, v;
			u = i * (1 / 16.0f);
			v = j * (1 / 16.0f);
			u += (float)((sin((tick + i*100) * 0.012387) * 0.04) * sin(tick * 0.000514382));
			v += (float)((cos((tick + j*100) * 0.012387) * 0.04) * sin(tick * 0.000714282));
			glTexCoord2f(u, v);
			glVertex2f(i * (640 / 16.0f), j * (480 / 16.0f));

			u = i * (1 / 16.0f);
			v = (j + 1) * (1 / 16.0f);
			u += (float)((sin((tick +       i*100) * 0.012387) * 0.04) * sin(tick * 0.000514382));
			v += (float)((cos((tick + (j + 1)*100) * 0.012387) * 0.04) * sin(tick * 0.000714282));
			glTexCoord2f(u, v);
			glVertex2f(i * (640 / 16.0f), (j + 1) * (480 / 16.0f));
		}
		// create some degenerate surfaces to "turn" the strip after each horizontal span
		glVertex2f(640, (j + 1) * (480 / 16.0f));
		glVertex2f(0, (j + 2) * (480 / 16.0f));
	}
	glEnd();

	// Show what we've got
	SDL_GL_SwapWindow(window);
	// Don't hog all CPU time
	SDL_Delay(1);
	
	if (isCaptureDone(device))
	{
		// OpenGL red and blue are swapped, so we'll need to do some converting here..
		for (i = 0; i < 512 * 512; i++)
			capture.mTargetBuf[i] = (capture.mTargetBuf[i] & 0xff00ff00) |
			                        ((capture.mTargetBuf[i] & 0xff) << 16) |
									((capture.mTargetBuf[i] & 0xff0000) >> 16);

		// Load up the new data
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 512, 512, 0, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)capture.mTargetBuf);
		
		// ..and ask for more
		doCapture(device);
	}
}

// Entry point
int main(int argc, char *argv[])
{
	device = setupESCAPI();
	if (device < 1)
	{
        printf("Unable to init ESCAPI\n");
        exit(1);
	}

	// Use the second device if more than one found
	// (makes my personal test setup easier =)
	if (device > 1)
		device = 1;
	else
		device = 0;

	// Setup ESCAPI to capture a 512x512 image.
	capture.mWidth = 512;
	capture.mHeight = 512;
	capture.mTargetBuf = new int[512 * 512];
	initCapture(device,&capture);
	doCapture(device);


    int width = 640;
    int height = 480;
    int bpp = 32;

	// Setup SDL
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
	{
		printf("Error: %s\n", SDL_GetError());
		return -1;
	}
	// Register SDL_Quit to be called at exit; makes sure things are
	// cleaned up when we quit.
	atexit(SDL_Quit);

	// Setup window
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
	SDL_DisplayMode current;
	SDL_GetCurrentDisplayMode(0, &current);
	window = SDL_CreateWindow("ESCAPI OpenGL example", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480, SDL_WINDOW_OPENGL);
	SDL_GLContext glcontext = SDL_GL_CreateContext(window);


	// Set up OpenGL..
    glViewport(0, 0, 640, 480);
    glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	// Set up the view matrix so that both X and Y coordinates are reversed.
	// OpenGL normally considers 0,0 to be the bottom left corder, so reversing
	// the Y axis makes sense if we want to think the top left to be the origo,
	// and reversing the X axis makes sense since this is supposed to be a "mirror".
	{
		float r = 0, l = 640, b = 480, t = 0, n = -1, f = 1;
		float ortho[16] =
		{
			       2 / (r - l),                  0,                  0,       0,
		                     0,        2 / (t - b),                  0,       0,
		                     0,                  0,           -2/(f-n),       0,
			-(r + l) / (r - l), -(t + b) / (t - b), -(f + n) / (f - n),       1
		};
		glLoadMatrixf(&ortho[0]);
		
	}
	glColor3f(1,1,1);

	// Create our texture
	glEnable(GL_TEXTURE_2D);
    GLuint texname;
    glGenTextures(1,&texname);
    glBindTexture(GL_TEXTURE_2D,texname);

	// Load up some initial data
	glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,256,256,0,GL_RGBA,GL_UNSIGNED_BYTE,(GLvoid*)capture.mTargetBuf);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR); // Linear Filtering
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR); // Linear Filtering

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
			case SDL_KEYUP:
				switch (event.key.keysym.sym)
				{
				case SDLK_ESCAPE:
					SDL_Quit();
					return 0;
					break;
				}
				break;
			case SDL_QUIT:
				SDL_Quit();
				return(0);
			}
		}
	}
	return 0;
}