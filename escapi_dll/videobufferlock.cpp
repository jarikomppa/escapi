#include <mfapi.h>
#include "videobufferlock.h"

VideoBufferLock::VideoBufferLock(IMFMediaBuffer *aBuffer) : m2DBuffer(NULL), mLocked(FALSE)
{
	mBuffer = aBuffer;
	mBuffer->AddRef();

	// Query for the 2-D buffer interface. OK if this fails.
	(void)mBuffer->QueryInterface(IID_PPV_ARGS(&m2DBuffer));
}

VideoBufferLock::~VideoBufferLock()
{
	UnlockBuffer();
	if (mBuffer)
		mBuffer->Release();
	if (m2DBuffer)
		m2DBuffer->Release();
}

HRESULT VideoBufferLock::LockBuffer(
	LONG  aDefaultStride,    // Minimum stride (with no padding).
	DWORD aHeightInPixels,  // Height of the image, in pixels.
	BYTE  **aScanLine0,    // Receives a pointer to the start of scan line 0.
	LONG  *aStride          // Receives the actual stride.
	)
{
	HRESULT hr = S_OK;

	// Use the 2-D version if available.
	if (m2DBuffer)
	{
		hr = m2DBuffer->Lock2D(aScanLine0, aStride);
	}
	else
	{
		// Use non-2D version.
		BYTE *data = NULL;

		hr = mBuffer->Lock(&data, NULL, NULL);
		if (SUCCEEDED(hr))
		{
			*aStride = aDefaultStride;
			if (aDefaultStride < 0)
			{
				// Bottom-up orientation. Return a pointer to the start of the
				// last row *in memory* which is the top row of the image.
				*aScanLine0 = data + abs(aDefaultStride) * (aHeightInPixels - 1);
			}
			else
			{
				// Top-down orientation. Return a pointer to the start of the
				// buffer.
				*aScanLine0 = data;
			}
		}
	}

	mLocked = (SUCCEEDED(hr));

	return hr;
}


void VideoBufferLock::UnlockBuffer()
{
	if (mLocked)
	{
		if (m2DBuffer)
		{
			(void)m2DBuffer->Unlock2D();
		}
		else
		{
			(void)mBuffer->Unlock();
		}
		mLocked = FALSE;
	}
}
