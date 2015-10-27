#pragma once

class VideoBufferLock
{
public:
	VideoBufferLock(IMFMediaBuffer *aBuffer);
	~VideoBufferLock();
	HRESULT LockBuffer(
		LONG  aDefaultStride,    // Minimum stride (with no padding).
		DWORD aHeightInPixels,  // Height of the image, in pixels.
		BYTE  **aScanLine0,    // Receives a pointer to the start of scan line 0.
		LONG  *aStride          // Receives the actual stride.
		);
	void UnlockBuffer();

private:
	IMFMediaBuffer  *mBuffer;
	IMF2DBuffer     *m2DBuffer;

	BOOL            mLocked;
};