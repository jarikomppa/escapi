
extern crate gcc;

fn main() {
    let files = &[
        "escapi_dll/capture.cpp",
        "escapi_dll/conversion.cpp",
        "escapi_dll/escapi_dll.cpp",
        "escapi_dll/interface.cpp",
        "escapi_dll/videobufferlock.cpp",
    ];
    let libs = &[
        "ole32.lib",
        "oleaut32.lib",
        "uuid.lib",
        "mf.lib",
        "mfplat.lib",
        "mfreadwrite.lib",
        "mfuuid.lib",
        "shlwapi.lib",
    ];
    let path = std::env::current_dir().unwrap();
    let compiler = gcc::Config::new()
        .cpp(true)
        .include(path.join("escapi_dll"))
        .include(path.join("common"))
        .flag("/LD")
        .get_compiler();
    let mut cmd = compiler.to_command();
    for file in files {
        cmd.arg(path.join(file));
    }
    for lib in libs {
        cmd.arg(lib);
    }
    cmd.arg("/DLL");
    cmd.arg("/Feescapi.dll");
    let out_dir = std::env::var("OUT_DIR").unwrap();
    cmd.current_dir(&out_dir);
    cmd.output().expect("failed to produce dll");
    std::fs::copy(format!("{}/escapi.dll", out_dir), format!("{}/../../../deps/escapi.dll", out_dir)).unwrap();
}
