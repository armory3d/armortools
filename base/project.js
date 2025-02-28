
let flags = globalThis.flags;
flags.android = os_argv().indexOf("android") >= 0;
flags.ios = os_argv().indexOf("ios") >= 0;
flags.d3d12 = os_argv().indexOf("direct3d12") >= 0;
flags.vulkan = os_argv().indexOf("vulkan") >= 0;
flags.metal = os_argv().indexOf("metal") >= 0;
flags.raytrace = flags.d3d12 || flags.vulkan || flags.metal;
flags.embed = os_argv().indexOf("--embed") >= 0; // os_argv().indexOf("--debug") == -1; // clang 19
flags.physics = true;

flags.with_d3dcompiler = true;
flags.with_nfd = true;
flags.with_compress = true;
flags.with_image_write = true;
flags.with_iron = true;
flags.with_eval = true;

let project = new Project("Base");
let dir = flags.name.toLowerCase();

if (!flags.lite) {
	project.add_define("IDLE_SLEEP");

	if (graphics === "vulkan") {
		project.add_project("./sources/libs/to_spirv");
	}

	if (flags.with_onnx) {
		project.add_define("WITH_ONNX");
		project.add_include_dir("../" + dir + "/onnx/include");
		if (platform === "windows") {
			project.add_lib("../" + dir + "/onnx/win32/onnxruntime");
		}
		else if (platform === "linux") {
			// patchelf --set-rpath . ArmorLab
			project.add_lib("onnxruntime -L" + flags.dirname + "/../" + dir + "/onnx/linux");
			// project.add_lib("onnxruntime_providers_cuda");
			// project.add_lib("onnxruntime_providers_shared");
			// project.add_lib("cublasLt");
			// project.add_lib("cublas");
			// project.add_lib("cudart");
			// project.add_lib("cudnn");
			// project.add_lib("cufft");
			// project.add_lib("curand");
		}
		else if (platform === "macos") {
			project.add_lib("../" + dir + "/onnx/macos/libonnxruntime.1.14.1.dylib");
		}
	}

	project.add_define("WITH_PLUGINS");
	project.add_project("../" + dir + "/plugins");
	project.add_project("plugins");

	project.add_tsfiles("sources/ts");
	project.add_tsfiles("sources/ts/iron");
	project.add_tsfiles("sources/ts/nodes");
	project.add_shaders("shaders/*.glsl");
	project.add_shaders("shaders/draw/*.glsl");
	project.add_assets("assets/*", { destination: "data/{name}" });
	project.add_assets("assets/locale/*", { destination: "data/locale/{name}" });
	project.add_assets("assets/licenses/**", { destination: "data/licenses/{name}" });
	project.add_assets("assets/plugins/*", { destination: "data/plugins/{name}" });
	project.add_assets("assets/themes/*.json", { destination: "data/themes/{name}" });

	if (platform === "linux" && fs_exists(os_cwd() + "/icon.png")) {
		project.add_assets("../" + dir + "/icon.png", { destination: "{name}", noprocessing: true });
	}

	if (flags.embed) {
		project.add_define("WITH_EMBED");
		project.add_define("arm_embed");
	}
	else {
		project.add_assets("assets/extra/*", { destination: "data/{name}" });
	}

	if (flags.physics) {
		project.add_cfiles("sources/libs/asim/asim.c");
		project.add_define("arm_physics");
	}

	// project.add_define("arm_skin");
	// project.add_define("arm_audio");

	if (flags.android) {
		project.add_define("arm_android_rmb");
	}

	if (flags.raytrace) {
		project.add_assets("assets/raytrace/*", { destination: "data/{name}" });

		if (flags.d3d12) {
			project.add_assets("shaders/raytrace/*.cso", { destination: "data/{name}" });
		}
		else if (flags.vulkan) {
			project.add_assets("shaders/raytrace/*.spirv", { destination: "data/{name}" });
		}
		else if (flags.metal) {
			project.add_assets("shaders/raytrace/*.metal", { destination: "data/{name}" });
		}
	}

	let export_version_info = true;
	if (export_version_info) {
		let dir = "../" + flags.name.toLowerCase() + "/build";
		let sha = os_popen(`git log --pretty=format:"%h" -n 1`).stdout.substr(1, 7);
		let date = new Date().toISOString().split("T")[0];
		let data = `{ "sha": "${sha}", "date": "${date}" }`;
		fs_ensuredir(dir);
		fs_writefile(dir + "/version.json", data);
		// Adds version.json to embed.txt list
		project.add_assets(dir + "/version.json", { destination: "data/{name}" });
	}

	let export_data_list = flags.android; // .apk contents
	if (export_data_list) {
		let root = "../" + flags.name.toLowerCase();
		let data_list = {
			"/data/plugins": fs_readdir("../base/assets/plugins").concat(fs_readdir(root + "/assets/plugins")).join(","),
			"/data/export_presets": fs_readdir(root + "/assets/export_presets").join(","),
			"/data/keymap_presets": fs_readdir(root + "/assets/keymap_presets").join(","),
			"/data/locale": fs_readdir("../base/assets/locale").join(","),
			"/data/meshes": fs_readdir(root + "/assets/meshes").join(","),
			"/data/themes": fs_readdir("../base/assets/themes").join(","),
		};
		let dir = "../" + flags.name.toLowerCase() + "/build";
		fs_ensuredir(dir);
		fs_writefile(dir + "/data_list.json", JSON.stringify(data_list));
		project.add_assets(dir + "/data_list.json", { destination: "data/{name}" });
	}
}

{
	project.add_include_dir("sources");

	function add_gpu_backend(name) {
		project.add_cfiles("sources/backends/" + name + "_gpu.c");
		project.add_define("BACKEND_GPU_H=\"backends/" + name + "_gpu.h\"");
	}

	function add_sys_backend(name) {
		project.add_cfiles("sources/backends/" + name + "_system.c");
		project.add_define("BACKEND_SYS_H=\"backends/" + name + "_system.h\"");
	}

	function add_net_backend(name) {
		project.add_cfiles("sources/backends/" + name + "_net.c");
		project.add_define("BACKEND_NET_H=\"backends/" + name + "_net.h\"");
	}

	function add_thread_backend(name) {
		project.add_cfiles("sources/backends/" + name + "_thread.c");
		project.add_define("BACKEND_THREAD_H=\"backends/" + name + "_thread.h\"");
	}

	if (platform === "windows") {
		add_sys_backend("windows");
		project.add_lib("dxguid");
		project.add_lib("dsound");
		project.add_lib("dinput8");
		project.add_define("_CRT_SECURE_NO_WARNINGS");
		project.add_define("_WINSOCK_DEPRECATED_NO_WARNINGS");
		project.add_lib("ws2_32");
		project.add_lib("Winhttp");
		project.add_lib("wbemuuid");

		if (graphics === "direct3d12" || graphics === "default") {
			add_gpu_backend("direct3d12");
			project.add_define("KINC_DIRECT3D12");
			project.add_lib("dxgi");
			project.add_lib("d3d12");
		}
		else {
			throw new Error("Graphics API " + graphics + " is not available for Windows.");
		}
	}
	else if (platform === "macos") {
		add_sys_backend("apple");
		add_sys_backend("macos");
		add_sys_backend("posix");
		if (graphics === "metal" || graphics === "default") {
			add_gpu_backend("metal");
			project.add_define("KINC_METAL");
			project.add_lib("Metal");
			project.add_lib("MetalKit");
		}
		else {
			throw new Error("Graphics API " + graphics + " is not available for macOS.");
		}
		project.add_lib("IOKit");
		project.add_lib("Cocoa");
		project.add_lib("AppKit");
		project.add_lib("CoreAudio");
		project.add_lib("CoreData");
		project.add_lib("CoreMedia");
		project.add_lib("CoreVideo");
		project.add_lib("AVFoundation");
		project.add_lib("Foundation");
	}
	else if (platform === "ios") {
		add_sys_backend("apple");
		add_sys_backend("ios");
		add_sys_backend("posix");
		if (graphics === "metal" || graphics === "default") {
			add_gpu_backend("metal");
			project.add_define("KINC_METAL");
			project.add_lib("Metal");
		}
		else {
			throw new Error("Graphics API " + graphics + " is not available for iOS.");
		}
		project.add_lib("UIKit");
		project.add_lib("Foundation");
		project.add_lib("CoreGraphics");
		project.add_lib("QuartzCore");
		project.add_lib("CoreAudio");
		project.add_lib("AudioToolbox");
		project.add_lib("CoreMotion");
		project.add_lib("AVFoundation");
		project.add_lib("CoreFoundation");
		project.add_lib("CoreVideo");
		project.add_lib("CoreMedia");
	}
	else if (platform === "android") {
		project.add_define("KINC_ANDROID");
		add_sys_backend("android");
		add_sys_backend("posix");
		if (graphics === "vulkan" || graphics === "default") {
			add_gpu_backend("vulkan");
			project.add_define("KINC_VULKAN");
			project.add_define("VK_USE_PLATFORM_ANDROID_KHR");
			project.add_lib("vulkan");
			project.add_define("KINC_ANDROID_API=24");
		}
		else {
			throw new Error("Graphics API " + graphics + " is not available for Android.");
		}
		project.add_lib("log");
		project.add_lib("android");
		project.add_lib("EGL");
		project.add_lib("GLESv3");
		project.add_lib("OpenSLES");
		project.add_lib("OpenMAXAL");
	}
	else if (platform === "wasm") {
		project.add_define("KINC_WASM");
		add_sys_backend("wasm");
		project.add_include_dir("miniclib");
		project.add_cfiles("sources/libs/miniclib/**");
		if (graphics === "webgpu" || graphics === "default") {
			add_gpu_backend("webgpu");
			project.add_define("KINC_WEBGPU");
		}
		else {
			throw new Error("Graphics API " + graphics + " is not available for Wasm.");
		}
	}
	else if (platform === "linux") {
		add_sys_backend("linux");
		add_thread_backend("posix");
		project.add_lib("asound");
		project.add_lib("dl");
		project.add_lib("udev");
		if (graphics === "vulkan" || graphics === "default") {
			add_gpu_backend("vulkan");
			project.add_lib("vulkan");
			project.add_define("KINC_VULKAN");
		}
		else {
			throw new Error("Graphics API " + graphics + " is not available for Linux.");
		}
		project.add_define("_POSIX_C_SOURCE=200112L");
		project.add_define("_XOPEN_SOURCE=600");
	}
}

if (fs_exists(os_cwd() + "/icon.png")) {
	project.icon = "icon.png";
	if (platform === "macos" && fs_exists(os_cwd() + "/icon_macos.png")) {
		project.icon = "icon_macos.png";
	}
}

project.add_include_dir("sources/libs");
project.add_cfiles("sources/libs/gc.c");
project.add_cfiles("sources/libs/dir.c");
project.add_include_dir("sources");
project.add_cfiles("sources/iron.c");
project.add_define("IRON_C_PATH=\"" + os_cwd() + "/build/iron.c" + "\"");
project.add_define("EMBED_H_PATH=\"" + os_cwd() + "/build/embed.h" + "\"");

if (flags.with_audio) {
	project.add_define("KINC_A1");
	project.add_define("KINC_A2");
	project.add_define("WITH_AUDIO");
	project.add_define("arm_audio");
	project.add_cfiles("sources/libs/stb_vorbis.c");
}

if (flags.with_eval) {
	project.add_define("WITH_EVAL");
	project.add_cfiles("sources/libs/quickjs/*.c");
	if (platform === "linux") {
		project.add_lib("m");
		project.add_define("_GNU_SOURCE");
		project.add_define("environ=__environ");
		project.add_define("sighandler_t=__sighandler_t");
	}
	else if (platform === "windows") {
		project.add_define("WIN32_LEAN_AND_MEAN");
		project.add_define("_WIN32_WINNT=0x0602");
	}
}

if (flags.with_iron) {
	project.add_define("WITH_IRON");
	project.add_cfiles("sources/*.c");
}

if (platform === "windows") {
	project.add_lib("Dbghelp"); // Stack walk
	project.add_lib("Dwmapi"); // DWMWA_USE_IMMERSIVE_DARK_MODE
	if (flags.with_d3dcompiler) {
		project.add_define("WITH_D3DCOMPILER");
		project.add_lib("d3d11");
		project.add_lib("d3dcompiler");
	}
}
else if (platform === "android") {
	project.add_define("IDLE_SLEEP");
	project.target_options.android.package = flags.package;
	project.target_options.android.permissions = ["android.permission.WRITE_EXTERNAL_STORAGE", "android.permission.READ_EXTERNAL_STORAGE", "android.permission.INTERNET"];
	project.target_options.android.screenOrientation = ["sensorLandscape"];
	project.target_options.android.minSdkVersion = 30;
	project.target_options.android.targetSdkVersion = 33;
	project.target_options.android.versionCode = 240000;
	project.target_options.android.versionName = "1.0 alpha";
}
else if (platform === "ios") {
	project.add_define("IDLE_SLEEP");
}

if (flags.with_nfd && (platform === "windows" || platform === "linux" || platform === "macos")) {
	project.add_define("WITH_NFD");
	project.add_include_dir("sources/libs/nfd");
	project.add_cfiles("sources/libs/nfd/nfd_common.c");

	if (platform === "windows") {
		project.add_cfiles("sources/libs/nfd/nfd_win.cpp");
	}
	else if (platform === "linux") {
		project.add_cfiles("sources/libs/nfd/nfd_gtk.c");
		project.add_include_dir("/usr/include/gtk-3.0");
		project.add_include_dir("/usr/include/glib-2.0");
		project.add_include_dir("/usr/lib/x86_64-linux-gnu/glib-2.0/include");
		project.add_include_dir("/usr/include/pango-1.0");
		project.add_include_dir("/usr/include/cairo");
		project.add_include_dir("/usr/include/gdk-pixbuf-2.0");
		project.add_include_dir("/usr/include/atk-1.0");
		project.add_include_dir("/usr/lib64/glib-2.0/include");
		project.add_include_dir("/usr/lib/glib-2.0/include");
		project.add_include_dir("/usr/include/harfbuzz");
		project.add_lib("gtk-3");
		project.add_lib("gobject-2.0");
		project.add_lib("glib-2.0");
	}
	else {
		project.add_cfiles("sources/libs/nfd/nfd_cocoa.m");
	}
}

if (flags.with_compress) {
	project.add_define("WITH_COMPRESS");
}

if (flags.with_image_write) {
	project.add_define("WITH_IMAGE_WRITE");
}

if (flags.with_video_write) {
	project.add_define("WITH_VIDEO_WRITE");
	project.add_cfiles("sources/libs/minimp4.c");
	project.add_cfiles("sources/libs/minih264e.c");
}

project.flatten();
return project;
