
let project = new Project("amake");

// alang
project.add_define("NO_GC");
project.add_define("NO_IRON_API");
project.add_define("NO_IRON_START");
project.add_tsfiles("./"); // alang.ts
project.add_include_dir("./"); // iron.h
project.add_cfiles("build/iron.c");
project.add_include_dir("../../sources");
project.add_cfiles("../../sources/iron_string.c");
project.add_cfiles("../../sources/iron_array.c");
project.add_cfiles("../../sources/iron_map.c");
project.add_cfiles("../../sources/iron_armpack.c");
project.add_cfiles("../../sources/iron_json.c");
project.add_cfiles("../../sources/iron_gc.c");

project.add_include_dir("../../sources/libs");
project.add_cfiles("../../sources/libs/quickjs-amalgam.c");
project.add_define("JS_DEFAULT_STACK_SIZE=8388608"); // 8 * 1024 * 1024
project.add_define("QJS_BUILD_LIBC");
project.add_cfiles("main.c");
project.add_cfiles("aimage.c");

project.add_cfiles("ashader.c");
project.add_cfiles('../../sources/libs/kong/libs/*.c');
project.add_cfiles('../../sources/libs/kong/*.c');
project.add_cfiles('../../sources/libs/kong/backends/*.c');
// project.add_cfiles('../../sources/libs/kong/backends/*.cpp'); // d3d12.cpp

if (platform === "windows") {
	project.add_define('_CRT_SECURE_NO_WARNINGS');
	project.add_lib('d3dcompiler');
	// project.add_include_dir('../../sources/libs/kong/libs/dxc/inc');
	// project.add_lib('../../sources/libs/kong/libs/dxc/lib/x64/dxcompiler');
	// hlslbin
	project.add_lib("dxguid");
}
else if (platform === "macos") {
	project.add_cfiles("../../sources/backends/data/mac.plist");
}
else if (platform === "linux") {
	project.add_lib('dl');
}

return project;
