extern crate escapi;
extern crate image;

/* "simplest", example of simply enumerating the available devices with ESCAPI */

fn main() {
    println!("devices: {}", escapi::num_devices());

    /* Set up capture parameters.
    * ESCAPI will scale the data received from the camera
    * (with point sampling) to whatever values you want.
    * Typically the native resolution is 320*240.
    */
    const W: u32 = 320;
    const H: u32 = 240;

    let mut camera = escapi::init(0, W, H, 15).expect("Could not initialize the camera");
    println!("capture initialized, device name: {}", camera.name());

    for i in 0..15 {
        println!("Frame #{}, captured and saved as image.png", i);
        let (width, height) = (camera.capture_width(), camera.capture_height());
        let pixels = camera.capture().expect("Could not capture an image");

        // Lets' convert it to RGB.
        let mut buffer = vec![0; width as usize * height as usize * 3];
        for i in 0..pixels.len() / 4 {
            buffer[i * 3] = pixels[i * 4 + 2];
            buffer[i * 3 + 1] = pixels[i * 4 + 1];
            buffer[i * 3 + 2] = pixels[i * 4];
        }

        image::save_buffer("image.png",
                           &buffer,
                           width,
                           height,
                           image::ColorType::RGB(8)).expect("Could not save an image");
    }

    println!("shutting down");
}
