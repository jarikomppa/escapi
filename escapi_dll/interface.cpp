#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>

#define ESCAPI_DEFINITIONS_ONLY
#include "escapi.h"

#include "conversion.h"
#include "capture.h"
#include "scopedrelease.h"
#include "choosedeviceparam.h"

#define MAXDEVICES 16

struct SimpleCapParams gParams[MAXDEVICES];
CaptureClass *gDevice[MAXDEVICES] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
int gDoCapture[MAXDEVICES];
int gOptions[MAXDEVICES];


void CleanupDevice(int aDevice)
{
	if (gDevice[aDevice])
	{
		gDevice[aDevice]->deinitCapture();
		delete gDevice[aDevice];
		gDevice[aDevice] = 0;
	}
}
HRESULT InitDevice(int aDevice)
{
	if (gDevice[aDevice])
	{
		CleanupDevice(aDevice);
	}
	gDevice[aDevice] = new CaptureClass;
	HRESULT hr = gDevice[aDevice]->initCapture(aDevice);
	if (FAILED(hr))
	{
		delete gDevice[aDevice];
		gDevice[aDevice] = 0;
	}
	return hr;
}



int CountCaptureDevices()
{
	HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

	if (FAILED(hr)) return 0;

	hr = MFStartup(MF_VERSION);

	if (FAILED(hr)) return 0;

	// choose device
	IMFAttributes *attributes = NULL;
	hr = MFCreateAttributes(&attributes, 1);
	ScopedRelease<IMFAttributes> attributes_s(attributes);

	if (FAILED(hr)) return 0;

	hr = attributes->SetGUID(
		MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE,
		MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID
		);
	if (FAILED(hr)) return 0;

	ChooseDeviceParam param = { 0 };
	hr = MFEnumDeviceSources(attributes, &param.mDevices, &param.mCount);

	if (FAILED(hr)) return 0;

	return param.mCount;
}

void GetCaptureDeviceName(int aDevice, char * aNamebuffer, int aBufferlength)
{
	int i;
	if (!aNamebuffer || aBufferlength <= 0)
		return;

	aNamebuffer[0] = 0;

	HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

	if (FAILED(hr)) return;

	hr = MFStartup(MF_VERSION);

	if (FAILED(hr)) return;

	// choose device
	IMFAttributes *attributes = NULL;
	hr = MFCreateAttributes(&attributes, 1);
	ScopedRelease<IMFAttributes> attributes_s(attributes);

	if (FAILED(hr)) return;

	hr = attributes->SetGUID(
		MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE,
		MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID
		);

	if (FAILED(hr)) return;

	ChooseDeviceParam param = { 0 };
	hr = MFEnumDeviceSources(attributes, &param.mDevices, &param.mCount);

	if (FAILED(hr)) return;

	if (aDevice < (signed)param.mCount)
	{
		WCHAR *name = 0;
		UINT32 namelen = 255;
		hr = param.mDevices[aDevice]->GetAllocatedString(
			MF_DEVSOURCE_ATTRIBUTE_FRIENDLY_NAME,
			&name,
			&namelen
			);
		if (SUCCEEDED(hr) && name)
		{
			i = 0;
			while (i < aBufferlength - 1 && i < (signed)namelen && name[i] != 0)
			{
				aNamebuffer[i] = (char)name[i];
				i++;
			}
			aNamebuffer[i] = 0;

			CoTaskMemFree(name);
		}
	}
}

void CheckForFail(int aDevice)
{
	if (!gDevice[aDevice])
		return;

	if (gDevice[aDevice]->mRedoFromStart)
	{
		gDevice[aDevice]->mRedoFromStart = 0;
		gDevice[aDevice]->deinitCapture();
		HRESULT hr = gDevice[aDevice]->initCapture(aDevice);
		if (FAILED(hr))
		{
			delete gDevice[aDevice];
			gDevice[aDevice] = 0;
		}
	}
}


int GetErrorCode(int aDevice)
{
	if (!gDevice[aDevice])
		return 0;
	return gDevice[aDevice]->mErrorCode;
}

int GetErrorLine(int aDevice)
{
	if (!gDevice[aDevice])
		return 0;
	return gDevice[aDevice]->mErrorLine;
}


float GetProperty(int aDevice, int aProp)
{
	CheckForFail(aDevice);
	if (!gDevice[aDevice])
		return 0;
	float val;
	int autoval;
	gDevice[aDevice]->getProperty(aProp, val, autoval);
	return val;
}

int GetPropertyAuto(int aDevice, int aProp)
{
	CheckForFail(aDevice);
	if (!gDevice[aDevice])
		return 0;
	float val;
	int autoval;
	gDevice[aDevice]->getProperty(aProp, val, autoval);
	return autoval;
}

int SetProperty(int aDevice, int aProp, float aValue, int aAutoval)
{
	CheckForFail(aDevice);
	if (!gDevice[aDevice])
		return 0;
	return gDevice[aDevice]->setProperty(aProp, aValue, aAutoval);
}
