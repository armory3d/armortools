
let flags = globalThis.flags;
flags.android = os_argv().indexOf("android") >= 0;
flags.ios = os_argv().indexOf("ios") >= 0;
flags.d3d12 = os_argv().indexOf("direct3d12") >= 0;
flags.vulkan = os_argv().indexOf("vulkan") >= 0;
flags.metal = os_argv().indexOf("metal") >= 0;
flags.raytrace = flags.d3d12 || flags.vulkan || flags.metal;
flags.embed = os_argv().indexOf("--embed") >= 0;
// flags.physics = true;
flags.physics = false;
flags.voxels = !flags.raytrace && !flags.android && !flags.ios;

flags.with_d3dcompiler = true;
flags.with_nfd = true;
flags.with_tinydir = true;
flags.with_zlib = true;
flags.with_stb_image_write = true;
flags.with_g2 = true;
flags.with_iron = true;
flags.with_zui = true;
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
project.addShaders("../armorcore/shaders/*.glsl", { embed: flags.embed });
project.addShaders("shaders/*.glsl", { embed: flags.embed });
project.addAssets("assets/*", { destination: "data/{name}", embed: flags.embed });
project.addAssets("assets/locale/*", { destination: "data/locale/{name}" });
project.addAssets("assets/licenses/**", { destination: "data/licenses/{name}" });
project.addAssets("assets/plugins/*", { destination: "data/plugins/{name}" });
project.addAssets("assets/themes/*.json", { destination: "data/themes/{name}" });

if (flags.embed) {
	project.addDefine("arm_embed");
	project.addDefine("arm_image_embed");
	project.addDefine("arm_shader_embed");
}
else {
	project.addDefine("arm_noembed");
	project.addAssets("assets/extra/*", { destination: "data/{name}" });
}

project.addDefine("arm_particles");
// project.addDefine("arm_skin");
// project.addDefine("arm_audio");

if (flags.android) {
	project.addDefine("krom_android_rmb");
}

if (flags.raytrace) {
	project.addAssets("assets/raytrace/*", { destination: "data/{name}", embed: flags.embed });

	if (flags.d3d12) {
		project.addAssets("shaders/raytrace/*.cso", { destination: "data/{name}", embed: flags.embed });
	}
	else if (flags.vulkan) {
		project.addAssets("shaders/raytrace/*.spirv", { destination: "data/{name}", embed: flags.embed });
	}
	else if (flags.metal) {
		project.addAssets("shaders/raytrace/*.metal", { destination: "data/{name}", embed: flags.embed });
	}
}

if (flags.voxels) {
	project.addDefine("arm_voxels");

	if (os_platform() === "win32") {
		project.addShaders("shaders/voxel_hlsl/*.glsl", { embed: flags.embed, noprocessing: true });
	}
	else {
		project.addShaders("shaders/voxel_glsl/*.glsl", { embed: flags.embed });
	}
}

let export_version_info = true;
if (export_version_info) {
	let dir = "../" + flags.name.toLowerCase() + "/build";
	let sha = os_exec(`git log --pretty=format:"%h" -n 1`).toString().substr(1, 7);
	let date = new Date().toISOString().split("T")[0];
	let data = `{ "sha": "${sha}", "date": "${date}" }`;
	fs_ensuredir(dir);
	fs_writefile(dir + "/version.json", data);
	// Adds version.json to embed.txt list
	project.addAssets(dir + "/version.json", { destination: "data/{name}", embed: flags.embed });
}

project.flatten();
return project;
