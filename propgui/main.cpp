#include "imgui.h"
#include "imgui_impl_sdl.h"
#include <stdio.h>
#include <SDL.h>
#include <SDL_opengl.h>

#include "escapi.h"

struct SimpleCapParams capture[16];
int active[16];
int devicecount;
GLuint texture[16];
bool showdisabled = false;

void toggle(int device)
{
	if (active[device])
	{
		deinitCapture(device);
		active[device] = 0;
	}
	else
	{
		initCapture(device, &capture[device]);
		active[device] = 1;
		doCapture(device);
	}
}

void devicewindow(int device)
{
	char camname[256];
	getCaptureDeviceName(device, camname, 256);
	float prop[CAPTURE_PROP_MAX];
	bool propauto[CAPTURE_PROP_MAX];

	int i;
	for (i = 0; i < CAPTURE_PROP_MAX; i++)
	{
		prop[i] = getCapturePropertyValue(device, i);
		propauto[i] = getCapturePropertyAuto(device, i) != 0;
	}

	if (isCaptureDone(device))
	{
		// OpenGL red and blue are swapped, so we'll need to do some converting here..
		int i;
		for (i = 0; i < 256 * 256; i++)
			capture[device].mTargetBuf[i] = (capture[device].mTargetBuf[i] & 0xff00ff00) |
			((capture[device].mTargetBuf[i] & 0xff) << 16) |
			((capture[device].mTargetBuf[i] & 0xff0000) >> 16);

		glBindTexture(GL_TEXTURE_2D, texture[device]);
		// Load up the new data
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 256, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)capture[device].mTargetBuf);

		// ..and ask for more
		doCapture(device);
	}
	bool opened = true;
	ImGui::Begin(camname, &opened, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize);
	ImVec2 picsize(256, 256);

	ImGui::Image((ImTextureID)texture[device], picsize);

	bool disabled_bool = false;

#define CTRLPROP(x) \
	if (prop[x] < 0) \
		{ \
		if (showdisabled) \
				{\
			ImGui::Checkbox("", &disabled_bool); \
			ImGui::SameLine(); \
			ImGui::PushItemWidth(200);\
			ImGui::SliderFloat(#x + 8, &prop[x], -1, -1, ""); \
			ImGui::PopItemWidth();\
				} \
		} \
		else \
		{ \
		bool changed = false; \
		changed |= ImGui::Checkbox("", &propauto[x]); \
		ImGui::SameLine(); \
		ImGui::PushItemWidth(200);\
		changed |= ImGui::SliderFloat(#x+8, &prop[x], 0, 1); \
		ImGui::PopItemWidth();\
		if (changed) \
		{ \
			setCaptureProperty(device, x, prop[x], propauto[x]?1:0); \
		} \
	}
	ImGui::SameLine();
	ImGui::BeginChild("foo", ImVec2(420, 256), false, ImGuiWindowFlags_HorizontalScrollbar);
	ImGui::Text("Auto   Value");
	CTRLPROP(CAPTURE_BRIGHTNESS);
	CTRLPROP(CAPTURE_CONTRAST);
	CTRLPROP(CAPTURE_HUE);
	CTRLPROP(CAPTURE_SATURATION);
	CTRLPROP(CAPTURE_SHARPNESS);
	CTRLPROP(CAPTURE_GAMMA);
	CTRLPROP(CAPTURE_COLORENABLE);
	CTRLPROP(CAPTURE_WHITEBALANCE);
	CTRLPROP(CAPTURE_BACKLIGHTCOMPENSATION);
	CTRLPROP(CAPTURE_GAIN);
	CTRLPROP(CAPTURE_PAN);
	CTRLPROP(CAPTURE_TILT);
	CTRLPROP(CAPTURE_ROLL);
	CTRLPROP(CAPTURE_ZOOM);
	CTRLPROP(CAPTURE_EXPOSURE);
	CTRLPROP(CAPTURE_IRIS);
	CTRLPROP(CAPTURE_FOCUS);
	ImGui::EndChild();

	ImGui::End();

	if (!opened)
		toggle(device);
}

int main(int, char**)
{
	devicecount = setupESCAPI();
	if (devicecount < 1)
	{
		fprintf(stderr, "Unable to init ESCAPI\n");
		exit(1);
	}
	
	// Setup SDL
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
	{
        printf("Error: %s\n", SDL_GetError());
        return -1;
	}

    // Setup window
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
	SDL_DisplayMode current;
	SDL_GetCurrentDisplayMode(0, &current);
	SDL_Window *window = SDL_CreateWindow("ESCAPI complex example", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_OPENGL|SDL_WINDOW_RESIZABLE);
	SDL_GLContext glcontext = SDL_GL_CreateContext(window);

    // Setup ImGui binding
    ImGui_ImplSdl_Init(window);

    bool show_test_window = true;
    bool show_another_window = false;
    ImVec4 clear_color = ImColor(114, 144, 154);

	// Setup ESCAPI to capture a 256x256 image.
	int i;
	for (i = 0; i < 16; i++)
	{
		active[i] = 0;

		capture[i].mWidth = 256;
		capture[i].mHeight = 256;
		capture[i].mTargetBuf = new int[256 * 256];

		glGenTextures(1, &texture[i]);
		glBindTexture(GL_TEXTURE_2D, texture[i]);

		// Load up some initial data
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 256, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)capture[i].mTargetBuf);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); // Linear Filtering
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // Linear Filtering
	}

    // Main loop
	bool done = false;
    while (!done)
    {

        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            ImGui_ImplSdl_ProcessEvent(&event);
            if (event.type == SDL_QUIT)
                done = true;
        }
        ImGui_ImplSdl_NewFrame(window);
		//ImGui::ShowTestWindow();

		ImGui::Begin("Main", 0, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize);
		ImGui::Checkbox("Show disabled controls", &showdisabled);
		for (i = 0; i < devicecount; i++)
		{
			char temp[256];
			getCaptureDeviceName(i, temp, 256);
			if (ImGui::Button(temp))
				toggle(i);
		}
		ImGui::End();

		for (i = 0; i < devicecount; i++)
		{
			if (active[i])
			{
				devicewindow(i);
			}
		}

        // Rendering
        glViewport(0, 0, (int)ImGui::GetIO().DisplaySize.x, (int)ImGui::GetIO().DisplaySize.y);
        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui::Render();
        SDL_GL_SwapWindow(window);
    }

    // Cleanup
    ImGui_ImplSdl_Shutdown();
    SDL_GL_DeleteContext(glcontext);  
	SDL_DestroyWindow(window);
	SDL_Quit();

    return 0;
}
