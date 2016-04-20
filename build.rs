
extern crate gcc;

fn main() {
    /*
    gcc::Config::new()
                .cpp(true)
                .file("escapi_dll/capture.cpp")
                .file("escapi_dll/conversion.cpp")
                .file("escapi_dll/escapi_dll.cpp")
                .file("escapi_dll/interface.cpp")
                .file("escapi_dll/videobufferlock.cpp")
                .include("escapi_dll")
                .include("common")
                .flag("/MD")
                .compile("libescapi.a");
                */
    gcc::Config::new()
                .file("common/escapi.cpp")
                .include("common")
                .flag("/TC")
                .compile("libescapi_c.a");
}
