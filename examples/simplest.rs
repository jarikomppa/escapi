extern crate escapi;
use escapi::*;

/* "simplest", example of simply enumerating the available devices with ESCAPI */

fn main() {
    /* Initialize ESCAPI */
    let escapi = init().unwrap();

    println!("devices: {}", escapi.num_devices());

    /* Set up capture parameters.
    * ESCAPI will scale the data received from the camera
    * (with point sampling) to whatever values you want.
    * Typically the native resolution is 320*240.
    */
    const W: u32 = 32;
    const H: u32 = 24;

    /* Initialize capture - only one capture may be active per device,
     * but several devices may be captured at the same time.
     *
     * 0 is the first device.
     */
    let mut camera = escapi.init(0, W, H).unwrap();

    println!("capture initialized");

    /* Go through 10 capture loops so that the camera has
     * had time to adjust to the lighting conditions and
     * should give us a sane image..
     */
    for i in 0..10 {
        println!("request a capture: {}", i);
        /* request a capture */
        let capture = camera.do_capture();
        println!("waiting for capture");

        while !capture.done() {
            std::thread::sleep(std::time::Duration::from_millis(1));
            /* Wait until capture is done.
             * Warning: if capture init failed, or if the capture
             * simply fails (i.e, user unplugs the web camera), this
             * will be an infinite loop.
             */
        }
        println!("capture successful");
    }
    println!("code: {}", camera.error_code());
    println!("line: {}", camera.error_line());

    /* now we have the data.. what shall we do with it? let's
     * render it in ASCII.. (using 3 top bits of green as the value)
     */
    let light = b" .,-o+O0@";
    for i in 0..H {
        for j in 0..W {
            let idx = camera.pixels()[(i*W+j) as usize] as isize;
            let idx = (idx >> 13) & 7;
            print!("{}", light[idx as usize] as char);
        }
        println!("");
    }

    println!("shutting down");
}
