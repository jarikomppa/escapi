"""
A simple python wrapper around escapi

Usage:

from camera import Device

device = Deveice.connect(0, 500, 500)
image = device.get_image()
"""

import os
from ctypes import *
from PIL import Image

def resolve(name):
    f = os.path.join(os.path.dirname(__file__), name)
    return f


class CAPTURE_PROPETIES:
    CAPTURE_BRIGHTNESS = 1,
    CAPTURE_CONTRAST = 2,
    CAPTURE_HUE = 3,
    CAPTURE_SATURATION = 4,
    CAPTURE_SHARPNESS = 5,
    CAPTURE_GAMMA = 6,
    CAPTURE_COLORENABLE = 7,
    CAPTURE_WHITEBALANCE = 8,
    CAPTURE_BACKLIGHTCOMPENSATION = 9,
    CAPTURE_GAIN = 10,
    CAPTURE_PAN = 11,
    CAPTURE_TILT = 12,
    CAPTURE_ROLL = 13,
    CAPTURE_ZOOM = 14,
    CAPTURE_EXPOSURE = 15,
    CAPTURE_IRIS = 16,
    CAPTURE_FOCUS = 17,
    CAPTURE_PROP_MAX = 18,


class SimpleCapParms(Structure):
    _fields_ = [
        ("buffer", POINTER(c_int)),
        ("width", c_int),
        ("height", c_int),
    ]


lib = None


def init():
    global lib
    lib = cdll.LoadLibrary(resolve("escapi32.dll"))
    lib.initCapture.argtypes = [c_int, POINTER(SimpleCapParms)]
    lib.initCapture.restype = c_int
    lib.initCOM()

def device_name(device):
    """
    Get the device name for the given device
    :param device: The number of the device
    :return: The name of the device
    """
    namearry = (c_char * 256)()
    lib.getCaptureDeviceName(device, namearry, 256)
    camearaname = namearry.value
    return camearaname

def init_camera(device, width, height):
    array = (width * height * c_int)()
    options = SimpleCapParms()
    options.width = width
    options.height = height
    options.buffer = array
    lib.initCapture(device, byref(options))
    return array

def get_image(device, width, height, array):
    lib.doCapture(device)

    while lib.isCaptureDone(device) == 0:
        pass

    img = Image.frombuffer('RGBA', (width, height), array, 'raw', 'BGRA', 0, 0)
    return img

def deinit_camera(device):
    lib.deinitCapture(device)


class Device():
    def __init__(self, device, width, height, array):
        self._device = device
        self._array = array
        self._width = width
        self._height = height

    @classmethod
    def connect(cls, device, width, height):
        if not lib:
            init()

        array = init_camera(device, width, height)
        return cls(device, width, height, array)

    def disconnect(self):
        deinit_camera(self._device)

    def get_image(self):
        return get_image(self._device, self._width, self._height, self._array)


