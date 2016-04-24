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
    let mut camera = escapi.init(0, W, H, 10).unwrap();

    println!("capture initialized");

    /* now we have the data.. what shall we do with it? let's
     * render it in ASCII.. (using 3 top bits of green as the value)
     */
    for _ in 0..100 {
        let pixels = camera.capture(50).unwrap();
        let light = b" .,-o+O0@";
        for i in 0..H {
            for j in 0..W {
                let idx = i*W + j;
                let val = pixels[(idx * 4) as usize];
                let val = val / 26;
                print!("{}", (val + b'0') as char);
            }
            print!("\n");
        }
        println!("");
    }

    println!("shutting down");
}
