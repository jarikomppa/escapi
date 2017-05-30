#pragma once

#include <Windows.h>
#include <Dbt.h>
#include <ks.h>
#include <ksmedia.h>
#include <functional> 

#include <Mfidl.h>

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
void RegisterForDeviceNotificationFromInterface(const std::function<void(bool isArrival)>& callback);
void UnregisterForDeviceNotificationFromInterface();