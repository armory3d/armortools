
let flags = globalThis.flags;
flags.android = os_argv().indexOf("android") >= 0;
flags.ios = os_argv().indexOf("ios") >= 0;
flags.d3d12 = os_argv().indexOf("direct3d12") >= 0;
flags.vulkan = os_argv().indexOf("vulkan") >= 0;
flags.metal = os_argv().indexOf("metal") >= 0;
flags.raytrace = flags.d3d12 || flags.vulkan || flags.metal;
flags.embed = os_argv().indexOf("--embed") >= 0; // os_argv().indexOf("--debug") == -1; // clang 19
flags.physics = os_argv().indexOf("--debug") == -1;
flags.voxels = !flags.raytrace && !flags.android && !flags.ios;

flags.with_d3dcompiler = true;
flags.with_nfd = true;
flags.with_tinydir = true;
flags.with_zlib = true;
flags.with_stb_image_write = true;
flags.with_g2 = true;
flags.with_iron = true;
flags.with_ui = true;
flags.with_eval = true;

let project = new Project("Base");

{
	project.addDefine("IDLE_SLEEP");
	let dir = flags.name.toLowerCase();

	if (graphics === "vulkan") {
		project.addDefine("KINC_VKRT");
		project.addProject("../armorcore/tools/ashader/to_spirv");
	}

	if (flags.with_onnx) {
		project.addDefine("WITH_ONNX");
		project.addIncludeDir("../" + dir + "/onnx/include");
		if (platform === "win32") {
			project.addLib("../" + dir + "/onnx/win32/onnxruntime");
		}
		else if (platform === "linux") {
			// patchelf --set-rpath . ArmorLab
			project.addLib("onnxruntime -L" + flags.dirname + "/../" + dir + "/onnx/linux");
			// project.addLib("onnxruntime_providers_cuda");
			// project.addLib("onnxruntime_providers_shared");
			// project.addLib("cublasLt");
			// project.addLib("cublas");
			// project.addLib("cudart");
			// project.addLib("cudnn");
			// project.addLib("cufft");
			// project.addLib("curand");
		}
		else if (platform === "macos") {
			project.addLib("../" + dir + "/onnx/macos/libonnxruntime.1.14.1.dylib");
		}
	}

	project.addDefine('WITH_PLUGINS');
	project.addFile("sources/plugin_api.c");
	project.addProject("../" + dir + "/plugins");
}

project.addProject("../armorcore");
project.addSources("sources");
project.addSources("sources/nodes");
project.addShaders("../armorcore/shaders/*.glsl");
project.addShaders("shaders/*.glsl");
project.addAssets("assets/*", { destination: "data/{name}" });
project.addAssets("assets/locale/*", { destination: "data/locale/{name}" });
project.addAssets("assets/licenses/**", { destination: "data/licenses/{name}" });
project.addAssets("assets/plugins/*", { destination: "data/plugins/{name}" });
project.addAssets("assets/themes/*.json", { destination: "data/themes/{name}" });

if (flags.embed) {
	project.addDefine("WITH_EMBED");
	project.addDefine("arm_embed");
}
else {
	project.addAssets("assets/extra/*", { destination: "data/{name}" });
}

if (flags.physics) {
	project.addDefine("arm_physics");
}

project.addDefine("arm_particles");
// project.addDefine("arm_skin");
// project.addDefine("arm_audio");

if (flags.android) {
	project.addDefine("arm_android_rmb");
}

if (flags.raytrace) {
	project.addAssets("assets/raytrace/*", { destination: "data/{name}" });

	if (flags.d3d12) {
		project.addAssets("shaders/raytrace/*.cso", { destination: "data/{name}" });
		project.addAssets("assets/readme/readme_dxr.txt", { destination: "{name}" });
	}
	else if (flags.vulkan) {
		project.addAssets("shaders/raytrace/*.spirv", { destination: "data/{name}" });
		project.addAssets("assets/readme/readme_vkrt.txt", { destination: "{name}" });
	}
	else if (flags.metal) {
		project.addAssets("shaders/raytrace/*.metal", { destination: "data/{name}" });
	}
}

if (flags.voxels) {
	project.addDefine("arm_voxels");

	if (os_platform() === "win32") {
		project.addShaders("shaders/voxel_hlsl/*.glsl", { noprocessing: true });
	}
	else {
		project.addShaders("shaders/voxel_glsl/*.glsl");
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
	project.addAssets(dir + "/version.json", { destination: "data/{name}" });
}

project.flatten();
return project;
