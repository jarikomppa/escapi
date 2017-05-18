
extern crate gcc;

fn main() {
    let path = std::env::current_dir().expect("Could not get the current dir");
    gcc::Config::new()
        .cpp(true)
        .pic(true)
        .include(path.join("escapi_dll"))
        .include(path.join("common"))
        .include("C:/Program Files (x86)/Windows Kits/8.1/Include/um/shlwapi.h")
        .file("escapi_dll/capture.cpp")
        .file("escapi_dll/conversion.cpp")
        .file("escapi_dll/escapi_dll.cpp")
        .file("escapi_dll/interface.cpp")
        .file("escapi_dll/videobufferlock.cpp")
        .object("ole32.lib")
        .object("oleaut32.lib")
        .object("uuid.lib")
        .object("mf.lib")
        .object("mfplat.lib")
        .object("mfreadwrite.lib")
        .object("mfuuid.lib")
        .object("shlwapi.lib")
        .compile("libescapi.a");
    println!("cargo:rustc-link-lib=static=escapi");
}
