/* Extremely Simple Capture API */
#include <functional>

struct SimpleCapParams
{
	/* Target buffer. 
	 * Must be at least mWidth * mHeight * sizeof(int) of size! 
	 */
	int * mTargetBuf;
	/* Buffer width */
	int mWidth;
	/* Buffer height */
	int mHeight;
};

enum CAPTURE_PROPETIES
{
	CAPTURE_BRIGHTNESS,
	CAPTURE_CONTRAST,
	CAPTURE_HUE,
	CAPTURE_SATURATION,
	CAPTURE_SHARPNESS,
	CAPTURE_GAMMA,
	CAPTURE_COLORENABLE,
	CAPTURE_WHITEBALANCE,
	CAPTURE_BACKLIGHTCOMPENSATION,
	CAPTURE_GAIN,
	CAPTURE_PAN,
	CAPTURE_TILT,
	CAPTURE_ROLL,
	CAPTURE_ZOOM,
	CAPTURE_EXPOSURE,
	CAPTURE_IRIS,
	CAPTURE_FOCUS,
	CAPTURE_PROP_MAX
};


/* Sets up the ESCAPI DLL and the function pointers below. Call this first! */
/* Returns number of capture devices found (same as countCaptureDevices, below) */
extern int setupESCAPI();

/* return the number of capture devices found */
typedef int (*countCaptureDevicesProc)();

/* initCapture tries to open the video capture device. 
 * Returns 0 on failure, 1 on success. 
 * Note: Capture parameter values must not change while capture device
 *       is in use (i.e. between initCapture and deinitCapture).
 *       Do *not* free the target buffer, or change its pointer!
 */
typedef int (*initCaptureProc)(unsigned int deviceno, struct SimpleCapParams *aParams);

/* deinitCapture closes the video capture device. */
typedef void (*deinitCaptureProc)(unsigned int deviceno);

/* doCapture requests video frame to be captured. */
typedef void (*doCaptureProc)(unsigned int deviceno);

/* isCaptureDone returns 1 when the requested frame has been captured.*/
typedef int (*isCaptureDoneProc)(unsigned int deviceno);

/* Get the user-friendly name of a capture device. */
typedef void (*getCaptureDeviceNameProc)(unsigned int deviceno, char *namebuffer, int bufferlength);

/* Get the unique name of a capture device. */
typedef void(*getCaptureDeviceUniqueNameProc)(unsigned int deviceno, char *namebuffer, int bufferlength);

/* Returns the ESCAPI DLL version. 0x200 for 2.0 */
typedef int (*ESCAPIVersionProc)();

/* 
	On properties -
	- Not all cameras support properties at all.
	- Not all properties can be set to auto.
	- Not all cameras support all properties.
	- Messing around with camera properties may lead to weird results, so YMMV.
*/

/* Gets value (0..1) of a camera property (see CAPTURE_PROPERTIES, above) */
typedef float (*getCapturePropertyValueProc)(unsigned int deviceno, int prop);
/* Gets whether the property is set to automatic (see CAPTURE_PROPERTIES, above) */
typedef int(*getCapturePropertyAutoProc)(unsigned int deviceno, int prop);
/* Set camera property to a value (0..1) and whether it should be set to auto. */
typedef int (*setCapturePropertyProc)(unsigned int deviceno, int prop, float value, int autoval);

/*
	All error situations in ESCAPI are considered catastrophic. If such should
	occur, the following functions can be used to check which line reported the
	error, and what the HRESULT of the error was. These may help figure out
	what the problem is.
*/

/* Return line number of error, or 0 if no catastrophic error has occurred. */
typedef int (*getCaptureErrorLineProc)(unsigned int deviceno);
/* Return HRESULT of the catastrophic error, or 0 if none. */
typedef int (*getCaptureErrorCodeProc)(unsigned int deviceno);

/* initCaptureWithOptions allows additional options to be given. Otherwise it's identical with initCapture */
typedef int (*initCaptureWithOptionsProc)(unsigned int deviceno, struct SimpleCapParams *aParams, unsigned int aOptions);

/* Start handle camera connect/disconnect and calling callback for this events. */
typedef void(*registerForDeviceNotificationProc)(std::function<void(bool isArrival)> callback);
/* Stop handle camera connect/disconnect. */
typedef void(*unregisterForDeviceNotificationProc)();

// Options accepted by above:
// Return raw data instead of converted rgb. Using this option assumes you know what you're doing.
#define CAPTURE_OPTION_RAWDATA 1 
// Mask to check for valid options - all options OR:ed together.
#define CAPTURE_OPTIONS_MASK (CAPTURE_OPTION_RAWDATA) 


#ifndef ESCAPI_DEFINITIONS_ONLY
extern countCaptureDevicesProc countCaptureDevices;
extern initCaptureProc initCapture;
extern deinitCaptureProc deinitCapture;
extern doCaptureProc doCapture;
extern isCaptureDoneProc isCaptureDone;
extern getCaptureDeviceNameProc getCaptureDeviceName;
extern getCaptureDeviceUniqueNameProc getCaptureDeviceUniqueName;
extern ESCAPIVersionProc ESCAPIVersion;
extern getCapturePropertyValueProc getCapturePropertyValue;
extern getCapturePropertyAutoProc getCapturePropertyAuto;
extern setCapturePropertyProc setCaptureProperty;
extern getCaptureErrorLineProc getCaptureErrorLine;
extern getCaptureErrorCodeProc getCaptureErrorCode;
extern initCaptureWithOptionsProc initCaptureWithOptions;
extern registerForDeviceNotificationProc registerForDeviceNotification;
extern unregisterForDeviceNotificationProc unregisterForDeviceNotification;
#endif