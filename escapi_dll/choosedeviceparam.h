#pragma once

struct ChooseDeviceParam
{
	IMFActivate **mDevices;    // Array of IMFActivate pointers.
	UINT32      mCount;          // Number of elements in the array.
	UINT32      mSelection;      // Selected device, by array index.

	~ChooseDeviceParam()
	{
		unsigned int i;
		for (i = 0; i < mCount; i++)
		{
			if (mDevices[i])
				mDevices[i]->Release();
		}
		CoTaskMemFree(mDevices);
	}
};
