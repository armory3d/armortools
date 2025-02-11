
let flags = globalThis.flags;
flags.android = os_argv().indexOf("android") >= 0;
flags.ios = os_argv().indexOf("ios") >= 0;
flags.d3d12 = os_argv().indexOf("direct3d12") >= 0;
flags.vulkan = os_argv().indexOf("vulkan") >= 0;
flags.metal = os_argv().indexOf("metal") >= 0;
flags.raytrace = flags.d3d12 || flags.vulkan || flags.metal;
flags.embed = os_argv().indexOf("--embed") >= 0; // os_argv().indexOf("--debug") == -1; // clang 19
flags.physics = os_argv().indexOf("--debug") == -1;

flags.with_d3dcompiler = true;
flags.with_nfd = true;
flags.with_compress = true;
flags.with_image_write = true;
flags.with_iron = true;
flags.with_eval = true;

let project = new Project("Base");
let dir = flags.name.toLowerCase();

{
	project.add_define("IDLE_SLEEP");

	if (graphics === "vulkan") {
		project.add_project("../armorcore/tools/to_spirv");
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
	project.add_cfiles("sources/plugin_api.c");
	project.add_project("../" + dir + "/plugins");
	project.add_project("plugins");
}

project.add_project("../armorcore");
project.add_tsfiles("sources");
project.add_tsfiles("sources/nodes");
project.add_shaders("../armorcore/shaders/*.glsl");
project.add_shaders("shaders/*.glsl");
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

project.flatten();
return project;
