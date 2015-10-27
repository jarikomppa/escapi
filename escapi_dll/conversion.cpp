#include <mfapi.h>
#include "conversion.h"


ConversionFunction gFormatConversions[] =
{
	{ MFVideoFormat_RGB32, TransformImage_RGB32 },
	{ MFVideoFormat_RGB24, TransformImage_RGB24 },
	{ MFVideoFormat_YUY2, TransformImage_YUY2 },
	{ MFVideoFormat_NV12, TransformImage_NV12 }
};

const DWORD gConversionFormats = 4;



void TransformImage_RGB24(
	BYTE*       aDest,
	LONG        aDestStride,
	const BYTE* aSrc,
	LONG        aSrcStride,
	DWORD       aWidthInPixels,
	DWORD       aHeightInPixels
	)
{
	for (DWORD y = 0; y < aHeightInPixels; y++)
	{
		RGBTRIPLE *srcPel = (RGBTRIPLE*)aSrc;
		DWORD *destPel = (DWORD*)aDest;

		for (DWORD x = 0; x < aWidthInPixels; x++)
		{
			destPel[x] = (
				(srcPel[x].rgbtRed << 16) |
				(srcPel[x].rgbtGreen << 8) |
				(srcPel[x].rgbtBlue << 0) |
				(0xff << 24)
				);
		}

		aSrc += aSrcStride;
		aDest += aDestStride;
	}
}


void TransformImage_RGB32(
	BYTE*       aDest,
	LONG        aDestStride,
	const BYTE* aSrc,
	LONG        aSrcStride,
	DWORD       aWidthInPixels,
	DWORD       aHeightInPixels
	)
{
	MFCopyImage(aDest, aDestStride, aSrc, aSrcStride, aWidthInPixels * 4, aHeightInPixels);
}


__forceinline BYTE Clip(int aClr)
{
	return (BYTE)(aClr < 0 ? 0 : (aClr > 255 ? 255 : aClr));
}

__forceinline RGBQUAD ConvertYCrCbToRGB(
	int aY,
	int aCr,
	int aCb
	)
{
	RGBQUAD rgbq;

	int c = aY - 16;
	int d = aCb - 128;
	int e = aCr - 128;

	rgbq.rgbRed = Clip((298 * c + 409 * e + 128) >> 8);
	rgbq.rgbGreen = Clip((298 * c - 100 * d - 208 * e + 128) >> 8);
	rgbq.rgbBlue = Clip((298 * c + 516 * d + 128) >> 8);

	return rgbq;
}


void TransformImage_YUY2(
	BYTE*       aDest,
	LONG        aDestStride,
	const BYTE* aSrc,
	LONG        aSrcStride,
	DWORD       aWidthInPixels,
	DWORD       aHeightInPixels
	)
{
	for (DWORD y = 0; y < aHeightInPixels; y++)
	{
		RGBQUAD *destPel = (RGBQUAD*)aDest;
		WORD    *srcPel = (WORD*)aSrc;

		for (DWORD x = 0; x < aWidthInPixels; x += 2)
		{
			// Byte order is U0 Y0 V0 Y1

			int y0 = (int)LOBYTE(srcPel[x]);
			int u0 = (int)HIBYTE(srcPel[x]);
			int y1 = (int)LOBYTE(srcPel[x + 1]);
			int v0 = (int)HIBYTE(srcPel[x + 1]);

			destPel[x] = ConvertYCrCbToRGB(y0, v0, u0);
			destPel[x + 1] = ConvertYCrCbToRGB(y1, v0, u0);
		}

		aSrc += aSrcStride;
		aDest += aDestStride;
	}

}



void TransformImage_NV12(
	BYTE* aDst,
	LONG aDstStride,
	const BYTE* aSrc,
	LONG aSrcStride,
	DWORD aWidthInPixels,
	DWORD aHeightInPixels
	)
{
	const BYTE* bitsY = aSrc;
	const BYTE* bitsCb = bitsY + (aHeightInPixels * aSrcStride);
	const BYTE* bitsCr = bitsCb + 1;

	for (UINT y = 0; y < aHeightInPixels; y += 2)
	{
		const BYTE* lineY1 = bitsY;
		const BYTE* lineY2 = bitsY + aSrcStride;
		const BYTE* lineCr = bitsCr;
		const BYTE* lineCb = bitsCb;

		LPBYTE dibLine1 = aDst;
		LPBYTE dibLine2 = aDst + aDstStride;

		for (UINT x = 0; x < aWidthInPixels; x += 2)
		{
			int  y0 = (int)lineY1[0];
			int  y1 = (int)lineY1[1];
			int  y2 = (int)lineY2[0];
			int  y3 = (int)lineY2[1];
			int  cb = (int)lineCb[0];
			int  cr = (int)lineCr[0];

			RGBQUAD r = ConvertYCrCbToRGB(y0, cr, cb);
			dibLine1[0] = r.rgbBlue;
			dibLine1[1] = r.rgbGreen;
			dibLine1[2] = r.rgbRed;
			dibLine1[3] = 0; // Alpha

			r = ConvertYCrCbToRGB(y1, cr, cb);
			dibLine1[4] = r.rgbBlue;
			dibLine1[5] = r.rgbGreen;
			dibLine1[6] = r.rgbRed;
			dibLine1[7] = 0; // Alpha

			r = ConvertYCrCbToRGB(y2, cr, cb);
			dibLine2[0] = r.rgbBlue;
			dibLine2[1] = r.rgbGreen;
			dibLine2[2] = r.rgbRed;
			dibLine2[3] = 0; // Alpha

			r = ConvertYCrCbToRGB(y3, cr, cb);
			dibLine2[4] = r.rgbBlue;
			dibLine2[5] = r.rgbGreen;
			dibLine2[6] = r.rgbRed;
			dibLine2[7] = 0; // Alpha

			lineY1 += 2;
			lineY2 += 2;
			lineCr += 2;
			lineCb += 2;

			dibLine1 += 8;
			dibLine2 += 8;
		}

		aDst += (2 * aDstStride);
		bitsY += (2 * aSrcStride);
		bitsCr += aSrcStride;
		bitsCb += aSrcStride;
	}
}
