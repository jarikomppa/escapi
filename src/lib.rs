extern crate winapi;
extern crate libc;
extern crate kernel32;

use std::ffi::CString;
use std::mem::transmute;
use kernel32::{LoadLibraryA, GetProcAddress, FreeLibrary, GetLastError};

#[repr(C)]
struct SimpleCapParams {
    buf: *mut libc::c_int,
    width: libc::c_uint,
    height: libc::c_uint,
}

#[derive(Copy, Clone)]
#[repr(C)]
struct DeviceNo(libc::c_uint);

#[allow(non_snake_case)]
pub struct Cameras {
    countCaptureDevices: extern fn() -> libc::c_int,
    initCapture: extern fn(DeviceNo, *const SimpleCapParams) -> libc::c_int,
    deinitCapture: extern fn(DeviceNo),
    doCapture: extern fn(DeviceNo),
    isCaptureDone: extern fn(DeviceNo) -> libc::c_int,
    getCaptureDeviceName: extern fn(DeviceNo, *mut libc::c_char, libc::c_int),
    ESCAPIVersion: extern fn() -> libc::c_uint,
    #[allow(dead_code)]
    getCapturePropertyValue: extern fn(DeviceNo, libc::c_int) -> libc::c_float,
    #[allow(dead_code)]
    getCapturePropertyAuto: extern fn(DeviceNo, libc::c_int) -> libc::c_int,
    #[allow(dead_code)]
    setCaptureProperty: extern fn(DeviceNo, libc::c_int, libc::c_float, libc::c_int),
    getCaptureErrorLine: extern fn(DeviceNo) -> libc::c_int,
    getCaptureErrorCode: extern fn(DeviceNo) -> libc::c_int,
    capdll: winapi::HMODULE,
}

impl Drop for Cameras {
    fn drop(&mut self) {
        unsafe {
            assert!(FreeLibrary(self.capdll) == 1, "failed to release escapi.dll");
        }
    }
}

pub struct Capture<'a, 'b: 'a> {
    device: &'a Device<'b>,
}

impl<'a, 'b> Capture<'a, 'b> {
    pub fn done(&self) -> bool {
        (self.device.cameras.isCaptureDone)(self.device.device_no) == 1
    }
}

impl<'a, 'b> Drop for Capture<'a, 'b> {
    fn drop(&mut self) {
        assert!(self.done(), "don't drop Capture objects before the capture is done");
    }
}

impl<'a> Device<'a> {
    pub fn do_capture<'b>(&'b mut self) -> Capture<'b, 'a> {
        (self.cameras.doCapture)(self.device_no);
        Capture {
            device: self
        }
    }
    pub fn name(&self) -> String {
        let mut v = vec![0u8; 100];
        (self.cameras.getCaptureDeviceName)(self.device_no, v.as_mut_ptr() as *mut i8, v.len() as i32);
        let null = v.iter().position(|&c| c == 0).expect("null termination character");
        v.truncate(null);
        String::from_utf8(v).expect("device name contains invalid utf8 characters")
    }
    pub fn error_code(&self) -> i32 {
        (self.cameras.getCaptureErrorCode)(self.device_no)
    }
    pub fn error_line(&self) -> i32 {
        (self.cameras.getCaptureErrorLine)(self.device_no)
    }
    pub fn pixels(&mut self) -> &[i32] {
        &self.buf
    }
}

pub struct Device<'a> {
    device_no: DeviceNo,
    buf: Box<[i32]>,
    #[allow(dead_code)]
    params: Box<SimpleCapParams>,
    cameras: &'a Cameras,
}

impl<'a> Drop for Device<'a> {
    fn drop(&mut self) {
        (self.cameras.deinitCapture)(self.device_no)
    }
}

impl Cameras {
    pub fn num_devices(&self) -> usize {
        (self.countCaptureDevices)() as usize
    }
    pub fn version(&self) -> u32 {
        (self.ESCAPIVersion)()
    }
    pub fn init(&self, index: u32, wdt: u32, hgt: u32) -> Result<Device, Error> {
        let mut data = vec![0; (wdt*hgt) as usize].into_boxed_slice();
        let params = Box::new(SimpleCapParams {
            width: wdt,
            height: hgt,
            buf: data.as_mut_ptr(),
        });
        if (self.initCapture)(DeviceNo(index), &*params) == 1 {
            Ok(Device {
                device_no: DeviceNo(index),
                cameras: &self,
                buf: data,
                params: params,
            })
        } else {
            Err(CouldNotOpenDevice(unsafe { GetLastError() }))
        }
    }
}

#[derive(Debug)]
pub enum Error {
    CouldNotLoadEscapiDLL(libc::c_ulong),
    CouldNotOpenDevice(libc::c_ulong),
}

use self::Error::*;

#[allow(non_snake_case)]
pub fn init() -> Result<Cameras, Error> {
    unsafe {
        let escapi_dll = CString::new("escapi.dll").unwrap();
        let capdll = LoadLibraryA(escapi_dll.as_ptr());
        if capdll == std::ptr::null_mut() {
            return Err(CouldNotLoadEscapiDLL(GetLastError()));
        }

        let countCaptureDevices = CString::new("countCaptureDevices").unwrap();
        let initCapture = CString::new("initCapture").unwrap();
        let deinitCapture = CString::new("deinitCapture").unwrap();
        let doCapture = CString::new("doCapture").unwrap();
        let isCaptureDone = CString::new("isCaptureDone").unwrap();
        let initCOM = CString::new("initCOM").unwrap();
        let getCaptureDeviceName = CString::new("getCaptureDeviceName").unwrap();
        let ESCAPIVersion = CString::new("ESCAPIVersion").unwrap();
        let getCapturePropertyValue = CString::new("getCapturePropertyValue").unwrap();
        let getCapturePropertyAuto = CString::new("getCapturePropertyAuto").unwrap();
        let setCaptureProperty = CString::new("setCaptureProperty").unwrap();
        let getCaptureErrorLine = CString::new("getCaptureErrorLine").unwrap();
        let getCaptureErrorCode = CString::new("getCaptureErrorCode").unwrap();

        transmute::<_, extern fn()>(GetProcAddress(capdll, initCOM.as_ptr()))();

        Ok(Cameras {
            countCaptureDevices: transmute(GetProcAddress(capdll, countCaptureDevices.as_ptr())),
            initCapture: transmute(GetProcAddress(capdll, initCapture.as_ptr())),
            deinitCapture: transmute(GetProcAddress(capdll, deinitCapture.as_ptr())),
            doCapture: transmute(GetProcAddress(capdll, doCapture.as_ptr())),
            isCaptureDone: transmute(GetProcAddress(capdll, isCaptureDone.as_ptr())),
            getCaptureDeviceName: transmute(GetProcAddress(capdll, getCaptureDeviceName.as_ptr())),
            ESCAPIVersion: transmute(GetProcAddress(capdll, ESCAPIVersion.as_ptr())),
            getCapturePropertyValue: transmute(GetProcAddress(capdll, getCapturePropertyValue.as_ptr())),
            getCapturePropertyAuto: transmute(GetProcAddress(capdll, getCapturePropertyAuto.as_ptr())),
            setCaptureProperty: transmute(GetProcAddress(capdll, setCaptureProperty.as_ptr())),
            getCaptureErrorLine: transmute(GetProcAddress(capdll, getCaptureErrorLine.as_ptr())),
            getCaptureErrorCode: transmute(GetProcAddress(capdll, getCaptureErrorCode.as_ptr())),
            capdll: capdll,
        })
    }
}
