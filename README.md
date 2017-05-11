# ESCAPI
Extremely Simple Capture API

Copyright (c)2015 Jari Komppa
http://iki.fi/sol

## Binaries

Binaries available at http://iki.fi/sol/zip/escapi3.zip

## Usage

Add escapi.cpp to your project. This file contains code 
to load the escapi.dll. See the samples ('simplest' recommended)
for API usage.

## Rust Bindings Usage

Just add the crate in your project and start using it. **NOTE**:
make sure that Windows SDK and Media Foundation headers
are accessible for your Rust toolchain, otherwise the library
build will fail (i.e. it should work fine with MSVC toolchain
and it might require some changes if you're using mingw toolchain).

## License

ESCAPI is released under the unlicense. In short, use for any purpose 
as long as you don't hold me responsible for anything. It would be 
nice if you'd toss me a mail if you play with this thing.

Some examples use external libraries with different licenses.

## What is ESCAPI?

A fairly easy to use webcam (or other video input device) capture 
API.

Version 2.0 adds support for multiple capture devices and requesting
the capture device names, as well as new examples.

Version 2.1 updates the examples, including purebasic and blizmax
examples, as well as an OpenGL based 'funny mirrors'-example, and 
fills out the top 8 "alpha" bits in the captured data with 0xff.

Version 3.0 is complete rewrite using windows media foundation
instead of directshow. The version adds interface for playing
with camera properties, new examples, automatic camera resolution
selection, 64 bit builds, and source release. Version 3.0 onwards
requires windows vista or later (7, 8, 8.1, 10..).

## Motivation

One of the last things that I added to 'textmedia' was webcam support.
I got interested in webcams for a while, thinking I might whip up
some kind of "eye toy"-ish game using a web cam. 

One of the bad sides about webcams is that the programming API is
so complicated - the only way to use them back then was through DirectShow.
Simply getting data from the camera is a fairly complicated process,
while not giving you much control. 

So, to get rid of the directmedia SDK requirement, I split the required
code into a separate DLL, and now I present to you the ESCAPI:

- setupESCAPI - Initialize the whole library. (in escapi.cpp)
- countCaptureDevices - Request number of capture devices available.
- getCaptureDeviceName - Request the printable name of a capture device.
- initCapture - Tries to open the video capture device. Returns 0 on failure, 1 on success.
- doCapture - Requests a video frame to be captured.
- isCaptureDone - Returns 1 when the requested frame has been captured.
- deinitCapture - Closes the video capture device.
  
So basically, you call setup to initialize the library,
call init to start the capture device, and call doCapture to 
start the capture process. When the capture is done, you can ask for 
another frame. Etc.

Unfortunately, "eye toy"-wise, the webcams on PCs are quite laggy,
and this varies from a camera to the next. My logitech messenger
camera has a lag or about one second, while my creative instant camera
is has a more tolerable lag. Both cameras also perform some automatic 
contrast trickery and other stuff like that which won't be quite game-friendly.

Thus, I never ended up doing that "eye toy"-like game. Maybe one day =)

## Dependencies

Some of the examples use external libraries:

- SDL2, under zlib/libpng license, http://libsdl.org
- some stb libraries, public domain, https://github.com/nothings/stb
- ocornut's ImGui, MIT license, https://github.com/ocornut/imgui

