
let project = new Project("amake");

{
	// alang
	project.add_define("NO_GC");
	project.add_define("NO_IRON_API");
	project.add_define("NO_KINC_START");
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
}

project.add_include_dir("../../sources/libs");
project.add_cfiles("../../sources/libs/quickjs/*.c");
project.add_cfiles("main.c");
project.add_cfiles("aimage.c");

{
	project.add_cfiles("ashader.c");
	if (platform === "linux") {
		project.add_project("../to_spirv"); // Replace with https://github.com/Kode/Kongruent
		project.flatten();
	}
}

if (platform === "linux") {
	// quickjs
	project.add_lib("m");
	project.add_lib("dl");
	project.add_define("_GNU_SOURCE");
	project.add_define("environ=__environ");
	project.add_define("sighandler_t=__sighandler_t");
}
else if (platform === "windows") {
	// quickjs
	project.add_define("WIN32_LEAN_AND_MEAN");
	project.add_define("_WIN32_WINNT=0x0602");

	// hlslbin
	project.add_lib("d3dcompiler");
	project.add_lib("dxguid");
}
else if (platform === "macos") {
	project.add_cfiles("../../sources/backends/macos/kinc/backend/mac.plist");
}

// QuickJS changes:
// quickjs-libc.c#85 (fixes "import * as os from 'os';" crash):
// #define USE_WORKER -> //#define USE_WORKER
// "quickjs.h#259" (fixes "Maximum call stack size exceeded" in alang):
// #define JS_DEFAULT_STACK_SIZE (256 * 1024) -> #define JS_DEFAULT_STACK_SIZE (8 * 1024 * 1024)

return project;
