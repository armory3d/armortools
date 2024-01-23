
let flags = globalThis.flags;
flags.android = process.argv.indexOf("android") >= 0;
flags.ios = process.argv.indexOf("ios") >= 0;
flags.d3d12 = process.argv.indexOf("direct3d12") >= 0;
flags.vulkan = process.argv.indexOf("vulkan") >= 0;
flags.metal = process.argv.indexOf("metal") >= 0;
flags.raytrace = flags.d3d12 || flags.vulkan || flags.metal;
flags.snapshot = process.argv.indexOf("--snapshot") >= 0;
flags.physics = true;
flags.voxels = !flags.raytrace && !flags.android && !flags.ios;

flags.with_d3dcompiler = true;
flags.with_nfd = true;
flags.with_tinydir = true;
flags.with_zlib = true;
flags.with_stb_image_write = true;
flags.with_g2 = true;
flags.with_iron = true;
flags.with_zui = true;

flags.on_c_project_created = async function(c_project, platform, graphics) {
	c_project.addDefine("IDLE_SLEEP");
	let dir = flags.name.toLowerCase();

	if (graphics === "vulkan") {
		c_project.addDefine("KORE_VKRT");
		await c_project.addProject("../" + dir + "/glsl_to_spirv");
	}

	if (flags.with_onnx) {
		c_project.addDefine("WITH_ONNX");
		c_project.addIncludeDir("../" + dir + "/onnx/include");
		console.log(platform);
		if (platform === "win32") {
			c_project.addLib("../" + dir + "/onnx/win32/onnxruntime");
		}
		else if (platform === "linux") {
			// patchelf --set-rpath . ArmorLab
			c_project.addLib("onnxruntime -L" + flags.dirname + "/../" + dir + "/onnx/linux");
			// c_project.addLib("onnxruntime_providers_cuda");
			// c_project.addLib("onnxruntime_providers_shared");
			// c_project.addLib("cublasLt");
			// c_project.addLib("cublas");
			// c_project.addLib("cudart");
			// c_project.addLib("cudnn");
			// c_project.addLib("cufft");
			// c_project.addLib("curand");
		}
		else if (platform === "osx") {
			c_project.addLib("../" + dir + "/onnx/macos/libonnxruntime.1.14.1.dylib");
		}
	}

	await c_project.addProject("../" + dir + "/Plugins");
};

let project = new Project("Base");
project.addSources("Sources");
project.addSources("Sources/nodes");
project.addShaders("../armorcore/Shaders/*.glsl", { embed: flags.snapshot });
project.addShaders("Shaders/*.glsl", { embed: flags.snapshot });
project.addAssets("Assets/*", { destination: "data/{name}", embed: flags.snapshot });
project.addAssets("Assets/locale/*", { destination: "data/locale/{name}" });
project.addAssets("Assets/licenses/**", { destination: "data/licenses/{name}" });
project.addAssets("Assets/plugins/*", { destination: "data/plugins/{name}" });
project.addAssets("Assets/themes/*.json", { destination: "data/themes/{name}" });

if (flags.snapshot) {
	project.addDefine("arm_snapshot");
	project.addDefine("arm_image_embed");
	project.addDefine("arm_shader_embed");
}
else {
	project.addDefine("arm_noembed");
	project.addAssets("Assets/extra/*", { destination: "data/{name}" });
}

project.addDefine("arm_particles");
// project.addDefine("arm_skin");
// project.addDefine("arm_audio");

if (flags.android) {
	project.addDefine("krom_android_rmb");
}

if (flags.raytrace) {
	project.addAssets("Assets/raytrace/*", { destination: "data/{name}", embed: flags.snapshot });

	if (flags.d3d12) {
		project.addAssets("Shaders/raytrace/*.cso", { destination: "data/{name}", embed: flags.snapshot });
	}
	else if (flags.vulkan) {
		project.addAssets("Shaders/raytrace/*.spirv", { destination: "data/{name}", embed: flags.snapshot });
	}
	else if (flags.metal) {
		project.addAssets("Shaders/raytrace/*.metal", { destination: "data/{name}", embed: flags.snapshot });
	}
}

if (flags.voxels) {
	project.addDefine("arm_voxels");

	if (process.platform === "win32") {
		project.addShaders("Shaders/voxel_hlsl/*.glsl", { embed: flags.snapshot, noprocessing: true });
	}
	else {
		project.addShaders("Shaders/voxel_glsl/*.glsl", { embed: flags.snapshot });
	}
}

let export_version_info = true;
if (export_version_info) {
	const fs = require("fs");
	let dir = "../" + flags.name.toLowerCase() + "/build";
	let sha = require("child_process").execSync(`git log --pretty=format:"%h" -n 1`).toString().substr(1, 7);
	let date = new Date().toISOString().split("T")[0];
	let data = `{ "sha": "${sha}", "date": "${date}" }`;
	fs.ensureDirSync(dir);
	fs.writeFileSync(dir + "/version.json", data);
	// Adds version.json to embed.txt list
	project.addAssets(dir + "/version.json", { destination: "data/{name}", embed: flags.snapshot });
}

resolve(project);
