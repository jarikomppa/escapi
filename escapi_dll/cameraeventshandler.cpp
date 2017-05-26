#include "cameraeventshandler.h"
#include <thread>

HDEVNOTIFY  gDeviceNotify = NULL;
std::thread gMsgLoop;
BOOL gIsWorking;
HWND gHwnd;
bool gIsAccept;
std::function<void(bool isArrival)> gCallback;

void CreateMessageOnlyWindow()
{
	static LPCWSTR class_name = (LPCWSTR)"HANDLER_CLASS";
	WNDCLASSEX wx = {};
	wx.cbSize = sizeof(WNDCLASSEX);
	wx.lpfnWndProc = WndProc;    
	wx.hInstance = NULL;
	wx.lpszClassName = class_name;
	
	if (RegisterClassEx(&wx))
	{
		gHwnd = CreateWindowEx(0, class_name, (LPCWSTR)"handler", 0, 0, 0, 0, 0, HWND_MESSAGE, NULL, NULL, NULL);
	}

	DEV_BROADCAST_DEVICEINTERFACE di = { 0 };
	di.dbcc_size = sizeof(di);
	di.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
	di.dbcc_classguid = KSCATEGORY_CAPTURE;

	gDeviceNotify = RegisterDeviceNotification(
		gHwnd,
		&di,
		DEVICE_NOTIFY_WINDOW_HANDLE
	);

	MSG msg;
	BOOL bRet;

	while (gIsWorking)
	{
		bRet = GetMessage(&msg, NULL, 0, 0);
		if (bRet == -1)
		{
			PostQuitMessage(0);
			break;
		}
		else
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_DEVICECHANGE:
	{
		if (lParam != 0)
		{
			// call only once for device connection / disconnection event
			// https://stackoverflow.com/questions/44183743/wm-devicechange-comes-twice-when-disconnecting-connecting-device
			gIsAccept = !gIsAccept;
			if (gIsAccept)
			{
				switch (wParam)
				{
				case DBT_DEVICEARRIVAL:
					gCallback(true);
					break;
				case DBT_DEVICEREMOVECOMPLETE:
					gCallback(false);
					break;
				default:
					// don't need to handle other wParams
					break;
				}
			}
		}
		break;
	}
	case WM_CLOSE:
		DestroyWindow(hwnd);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}

	return 0;
}

void RegisterForDeviceNotificationFromInterface(const std::function<void(bool isArrival)>& callback)
{
	gCallback = callback;

	HANDLE procHandle = GetCurrentProcess();
	gIsWorking = true;
	gMsgLoop = std::thread(&CreateMessageOnlyWindow);
}

void UnregisterForDeviceNotificationFromInterface()
{
	if (gDeviceNotify)
		UnregisterDeviceNotification(gDeviceNotify);

	gIsWorking = false;
	PostMessage(gHwnd, WM_CLOSE, 0, 0);

	if (gMsgLoop.joinable())
		gMsgLoop.join();
}