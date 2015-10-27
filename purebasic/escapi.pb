;/* Extremely Simple Capture API */

Structure SimpleCapParams
   *mTargetBuf ; Must be at least mWidth * mHeight * SizeOf(int) of size!
   mWidth.l
   mHeight.l
EndStructure

;/* Return the number of capture devices found */
PrototypeC countCaptureDevicesProc()

; /* initCapture tries To open the video capture device.
;  * Returns 0 on failure, 1 on success.
;  * Note: Capture parameter values must Not change While capture device
;  *       is in use (i.e. between initCapture And deinitCapture).
;  *       Do *Not* free the target buffer, Or change its pointer!
;  */
PrototypeC initCaptureProc(deviceno, *aParams.SimpleCapParams)

;/* deinitCapture closes the video capture device. */
PrototypeC deinitCaptureProc(deviceno)

;/* doCapture requests video frame To be captured. */
PrototypeC doCaptureProc(deviceno)

;/* isCaptureDone returns 1 when the requested frame has been captured.*/
PrototypeC isCaptureDoneProc(deviceno)


;/* Get the user-friendly name of a capture device. */
PrototypeC getCaptureDeviceNameProc(deviceno, *namebuffer, bufferlength)


;/* Returns the ESCAPI DLL version. 0x200 For 2.0 */
PrototypeC ESCAPIDLLVersionProc()

; marked as "internal" in the example
PrototypeC initCOMProc()

Global countCaptureDevices.countCaptureDevicesProc
Global initCapture.initCaptureProc
Global deinitCapture.deinitCaptureProc
Global doCapture.doCaptureProc
Global isCaptureDone.isCaptureDoneProc
Global getCaptureDeviceName.getCaptureDeviceNameProc
Global ESCAPIDLLVersion.ESCAPIDLLVersionProc


Procedure setupESCAPI()
   
  ; load library
  capdll = OpenLibrary(#PB_Any, "escapi.dll")
  If capdll = 0
    ProcedureReturn 0
  EndIf
 
   ;/* Fetch function entry points */
   countCaptureDevices  = GetFunction(capdll, "countCaptureDevices")
   initCapture          = GetFunction(capdll, "initCapture")
   deinitCapture        = GetFunction(capdll, "deinitCapture")
   doCapture            = GetFunction(capdll, "doCapture")
   isCaptureDone        = GetFunction(capdll, "isCaptureDone")
   initCOM.initCOMProc  = GetFunction(capdll, "initCOM")
   getCaptureDeviceName = GetFunction(capdll, "getCaptureDeviceName")
   ESCAPIDLLVersion     = GetFunction(capdll, "ESCAPIDLLVersion")
   
   If countCaptureDevices = 0 Or initCapture = 0 Or deinitCapture = 0 Or doCapture = 0 Or isCaptureDone = 0 Or initCOM = 0 Or getCaptureDeviceName = 0 Or ESCAPIDLLVersion = 0
     ProcedureReturn 0
   EndIf
   
   ;/* Verify DLL version */
   If ESCAPIDLLVersion() < $200
     ProcedureReturn 0
   EndIf
 
   ;/* Initialize COM.. */
   initCOM(); 
 
  ; returns number of devices found
  ProcedureReturn countCaptureDevices()
EndProcedure 