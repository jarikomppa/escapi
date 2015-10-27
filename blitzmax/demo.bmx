' Thanks to Freak from the PureBasic forums for assistance!

Type SimpleCapParams
	Field mTargetBuf:Byte Ptr ' Must be at least mWidth * mHeight * SizeOf(Int) of size! 
	Field mWidth
	Field mHeight
End Type

Global InitCOM () "C"
Global CountCaptureDevices () "C"
Global OpenCaptureDevice (device, scp:Byte Ptr) "C"
Global CloseCaptureDevice (device) "C"
Global GetCapture (device) "C"
Global CaptureDone (device) "C"
Global CaptureDeviceName (device, name:Byte Ptr, namelen) "C"
Global ESCAPIDLLVersion () "C"

Function SetupESCAPI ()

	esc = LoadLibraryA ("escapi.dll")
	
	If esc
	
		InitCOM					= GetProcAddress (esc, "initCOM")
		CountCaptureDevices		= GetProcAddress (esc, "countCaptureDevices")
		OpenCaptureDevice 		= GetProcAddress (esc, "initCapture")
		CloseCaptureDevice		= GetProcAddress (esc, "deinitCapture")
		GetCapture				= GetProcAddress (esc, "doCapture")
		CaptureDone				= GetProcAddress (esc, "isCaptureDone")
		CaptureDeviceName		= GetProcAddress (esc, "getCaptureDeviceName")
		ESCAPIDLLVersion		= GetProcAddress (esc, "ESCAPIDLLVersion")

		If InitCOM = Null Or CountCaptureDevices = Null Or OpenCaptureDevice = Null or..
			CloseCaptureDevice = Null Or GetCapture = Null Or CaptureDone = Null or..
				CaptureDeviceName = Null Or ESCAPIDLLVersion = Null
					DebugLog "Function missing!"
					Return 0
		EndIf

		InitCOM ()
		
		If ESCAPIDLLVersion () < $200
			DebugLog "Old DLL (needs version 2.0+)"
			Return 0
		EndIf
		
	Else
		DebugLog "Failed to open DLL"
		Return 0
	EndIf

	Return 1
	
End Function

Function CaptureDevice$ (device)
	Local cam:Byte [1024]
	CaptureDeviceName (device, cam, 1024)
	Return String.FromCString (cam)
End Function

If SetupESCAPI () = 0
	Notify "Error! Make sure escapi.dll is in same folder and capture device plugged in!"
	End
EndIf

For num = 0 Until CountCaptureDevices ()
	Print "Capture device [" + num + "] name: " + CaptureDevice (num)
Next

device = 0

' To list/select devices...

	'Repeat
	'	device = Int (Input ("Enter capture device number: "))
	'Until device > -1 And device < CountCaptureDevices ()

' Preferred target width/height (ESCAPI scales capture data to this)...

width = 320
height = 240

AppTitle = "Using " + CaptureDevice (device) + "..."

Graphics width, height', 32

' Data structure...

Local scp:SimpleCapParams = New SimpleCapParams

' A PixMap for captured data...

pix:TPixmap = CreatePixmap (width, height, PF_BGRA8888)

' Stick pixmap memory pointer into data structure...

scp.mTargetBuf	= PixmapPixelPtr (pix)
scp.mWidth		= width
scp.mHeight		= height

' Start capture process...

If OpenCaptureDevice (device, scp) = 0
	Print "Failed to initialise capture device!"
	End
EndIf

Repeat

	Cls

	GetCapture (device)
	
	Repeat
		If KeyHit (KEY_ESCAPE) Then quit = True
	Until CaptureDone (device)

	DrawPixmap pix, 0, 0

	Flip

Until quit = True

' Stop capture process...
	
CloseCaptureDevice (device)

End