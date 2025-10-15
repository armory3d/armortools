
function add_gpu_backend(name) {
	project.add_cfiles("sources/backends/" + name + "_gpu.*");
	project.add_define("BACKEND_GPU_H=\"backends/" + name + "_gpu.h\"");
}

function add_sys_backend(name) {
	project.add_cfiles("sources/backends/" + name + "_system.*");
	project.add_define("BACKEND_SYS_H=\"backends/" + name + "_system.h\"");
}

function add_net_backend(name) {
	project.add_cfiles("sources/backends/" + name + "_net.*");
	project.add_define("BACKEND_NET_H=\"backends/" + name + "_net.h\"");
}

function add_thread_backend(name) {
	project.add_cfiles("sources/backends/" + name + "_thread.*");
	project.add_define("BACKEND_THREAD_H=\"backends/" + name + "_thread.h\"");
}

function get_version_code() {
	const now   = new Date();
	const year  = now.getFullYear().toString().slice(-2);
	const month = (now.getMonth() + 1).toString().padStart(2, '0');
	const day   = now.getDate().toString().padStart(2, '0');
	return parseInt(year + month + day, 10);
}

// flags: make.js/load_project
let flags   = globalThis.flags;
let dir     = flags.name.substr(5).toLowerCase(); // ArmorPaint -> paint
let project = new Project("Base");
project.add_include_dir("sources");
project.add_include_dir("sources/libs");
project.add_tsfiles("sources/ts");
project.add_shaders("shaders/*.kong");
project.add_assets("assets/*", {destination : "data/{name}"});
project.add_assets("assets/licenses/**", {destination : "data/licenses/{name}"});
project.add_assets("assets/themes/*.json", {destination : "data/themes/{name}"});
project.add_cfiles("sources/*.c");
// project.add_cfiles("sources/iron.c");
project.add_cfiles("sources/libs/gc.c");
project.add_cfiles("sources/libs/kong/dir.c");
project.add_define("IRON_C_PATH=\"" + os_cwd() + "/build/iron.c" +
                   "\"");
project.add_define("EMBED_H_PATH=\"" + os_cwd() + "/build/embed.h" +
                   "\"");

if (platform == "windows") {
	add_sys_backend("windows");
	add_net_backend("windows");
	add_thread_backend("windows");
	add_gpu_backend("direct3d12");
	project.add_define("_CRT_SECURE_NO_WARNINGS");
	project.add_define("_WINSOCK_DEPRECATED_NO_WARNINGS");
	project.add_define("IRON_DIRECT3D12");
	project.add_lib("dxguid");
	project.add_lib("Winhttp");
	project.add_lib("dxgi");
	project.add_lib("d3d12");
	project.add_lib("Dwmapi"); // DWMWA_USE_IMMERSIVE_DARK_MODE
	if (flags.with_audio) {
		project.add_lib("dsound");
	}
	if (flags.with_gamepad) {
		project.add_lib("dinput8");
	}
	if (flags.with_d3dcompiler) {
		project.add_define("WITH_D3DCOMPILER");
		project.add_lib("d3d11");
		project.add_lib("d3dcompiler");
	}
}
else if (platform == "linux") {
	add_sys_backend("linux");
	add_net_backend("posix");
	add_thread_backend("posix");
	add_gpu_backend("vulkan");
	project.add_define("IRON_VULKAN");
	project.add_define("_POSIX_C_SOURCE=200809L");
	project.add_lib("X11");
	project.add_lib("Xi");
	project.add_lib("Xcursor");
	project.add_lib("Xrandr");
	project.add_lib("ssl");
	project.add_lib("vulkan");
	if (flags.with_audio) {
		project.add_lib("asound");
	}
	if (flags.with_gamepad) {
		project.add_lib("udev");
	}
}
else if (platform == "macos") {
	add_sys_backend("macos");
	add_net_backend("apple");
	add_thread_backend("apple");
	add_gpu_backend("metal");
	project.add_cfiles("sources/backends/data/mac.plist");
	project.add_define("IRON_METAL");
}
else if (platform == "ios") {
	add_sys_backend("ios");
	add_net_backend("apple");
	add_thread_backend("apple");
	add_gpu_backend("metal");
	project.add_cfiles("sources/backends/data/ios.plist");
	project.add_cfiles("sources/backends/ios_file_dialog.m");
	project.add_define("IRON_METAL");
	project.target_options.ios.build   = get_version_code() + "";
	project.target_options.ios.version = get_version_code() + "";
}
else if (platform == "android") {
	add_sys_backend("android");
	add_thread_backend("posix");
	add_gpu_backend("vulkan");
	project.add_cfiles("sources/backends/android_file_dialog.c");
	project.add_cfiles("sources/backends/android_net.c");
	project.add_cfiles("sources/backends/android_native_app_glue.c");
	project.add_define("IRON_ANDROID");
	project.add_define("IRON_VULKAN");
	project.add_define("VK_USE_PLATFORM_ANDROID_KHR");
	project.add_define("arm_android_rmb");
	project.add_lib("vulkan");
	project.add_lib("log");
	project.add_lib("android");
	if (flags.with_audio) {
		project.add_lib("OpenSLES");
	}
	project.target_options.android.package           = flags.package;
	project.target_options.android.permissions       = [ "android.permission.READ_MEDIA_IMAGES", "android.permission.INTERNET" ];
	project.target_options.android.screenOrientation = [ "sensorLandscape" ];
	project.target_options.android.minSdkVersion     = 35; // android 15
	project.target_options.android.targetSdkVersion  = 36;
	project.target_options.android.versionCode       = get_version_code();
	project.target_options.android.versionName       = "1.0 alpha";
}
else if (platform == "wasm") {
	add_sys_backend("wasm");
	add_thread_backend("wasm");
	add_gpu_backend("webgpu");
	project.add_cfiles("sources/libs/miniclib/**");
	project.add_define("IRON_WASM");
	project.add_define("IRON_WEBGPU");
	project.add_include_dir("miniclib");
}

if (graphics == "metal" || (graphics == "vulkan" && platform != "android")) {
	project.add_define("IRON_BGRA");
}

if (flags.with_kong) {
	project.add_define("WITH_KONG");
	project.add_cfiles("sources/libs/kong/libs/*.c");
	project.add_cfiles("sources/libs/kong/*.c");
	project.add_cfiles("sources/libs/kong/backends/*.c");
}

if (flags.with_plugins) {
	project.add_define("WITH_PLUGINS");
	project.add_project("../" + dir + "/plugins");
	project.add_cfiles("sources/plugins/plugin_api.c");
}

if (flags.embed) {
	project.add_define("WITH_EMBED");
	project.add_define("arm_embed");
}
else {
	project.add_assets("assets/extra/*", {destination : "data/{name}"});
}

if (flags.with_physics) {
	project.add_define("arm_physics");
	project.add_cfiles("sources/libs/asim.c");
}

if (flags.with_raytrace) {
	project.add_assets("assets/raytrace/*", {destination : "data/{name}"});
	if (graphics == "direct3d12") {
		project.add_assets("shaders/raytrace/*.cso", {destination : "data/{name}"});
	}
	else if (graphics == "vulkan") {
		project.add_assets("shaders/raytrace/*.spirv", {destination : "data/{name}"});
	}
	else if (graphics == "metal") {
		project.add_assets("shaders/raytrace/*.metal", {destination : "data/{name}"});
	}
}

if (flags.export_version_info) {
	let dir  = "../" + flags.name.substr(5).toLowerCase() + "/build";
	let sha  = os_popen(`git log --pretty=format:"%h" -n 1`).stdout.substr(1, 7);
	let date = new Date().toISOString().split("T")[0];
	let data = `{ "sha": "${sha}", "date": "${date}" }`;
	fs_ensuredir(dir);
	fs_writefile(dir + "/version.json", data);
	// Adds version.json to embed.txt list
	project.add_assets(dir + "/version.json", {destination : "data/{name}"});
}

if (flags.export_data_list) {
	let root      = "../" + flags.name.substr(5).toLowerCase();
	let data_list = {
		"/data/plugins" : fs_readdir(root + "/assets/plugins").join(","),
		"/data/export_presets" : fs_readdir(root + "/assets/export_presets").join(","),
		"/data/keymap_presets" : fs_readdir(root + "/assets/keymap_presets").join(","),
		"/data/locale" : fs_readdir(root + "/assets/locale").join(","),
		"/data/meshes" : fs_readdir(root + "/assets/meshes").join(","),
		"/data/themes" : fs_readdir("../base/assets/themes").join(","),
	};
	let dir = "../" + flags.name.substr(5).toLowerCase() + "/build";
	fs_ensuredir(dir);
	fs_writefile(dir + "/data_list.json", JSON.stringify(data_list));
	project.add_assets(dir + "/data_list.json", {destination : "data/{name}"});
}

if (flags.with_audio) {
	project.add_define("IRON_A1");
	project.add_define("IRON_A2");
	project.add_define("WITH_AUDIO");
	project.add_define("arm_audio");
	project.add_cfiles("sources/libs/stb_vorbis.c");
}

if (flags.with_eval) {
	project.add_define("WITH_EVAL");
	project.add_cfiles("sources/libs/quickjs-amalgam.c");
	project.add_define("QJS_BUILD_LIBC");
	// project.add_cfiles("tools/amake/alang.c");
	// project.add_cfiles("tools/amake/alang_eval.c");
}

if (flags.with_gamepad) {
	project.add_define("WITH_GAMEPAD");
}

if (flags.idle_sleep) {
	project.add_define("IDLE_SLEEP");
}

if (fs_exists(os_cwd() + "/icon.png")) {
	project.icon = "icon.png";
	if (platform == "macos" && fs_exists(os_cwd() + "/icon_macos.png")) {
		project.icon = "icon_macos.png";
	}
	else if (platform == "ios" && fs_exists(os_cwd() + "/icon_ios.png")) {
		project.icon = "icon_ios.png";
	}
	else if (platform == "linux") {
		project.add_assets("../" + dir + "/icon.png", {destination : "{name}", noprocessing : true, noembed : true});
	}
}

if (flags.with_nfd && (platform == "windows" || platform == "linux" || platform == "macos")) {
	project.add_define("WITH_NFD");
	project.add_cfiles("sources/libs/nfd.c");
	if (platform == "linux") {
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
	else if (platform == "macos") {
		project.add_cfiles("sources/libs/nfd.m");
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
