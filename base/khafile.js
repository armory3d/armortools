
let flags = {};
globalThis.flags = flags;
flags.android = process.argv.indexOf("android") >= 0;
flags.ios = process.argv.indexOf("ios") >= 0;
flags.win_hlsl = process.platform === "win32" && process.argv.indexOf("opengl") < 0;
flags.d3d12 = process.argv.indexOf("direct3d12") >= 0;
flags.vulkan = process.argv.indexOf("vulkan") >= 0;
flags.metal = process.argv.indexOf("metal") >= 0;
flags.raytrace = flags.d3d12 || flags.vulkan || flags.metal;
flags.snapshot = process.argv.indexOf("--snapshot") >= 0;
flags.plugin_embed = flags.ios;
flags.physics = !flags.ios;
flags.voxels = !flags.raytrace && !flags.android && !flags.ios;

let project = new Project("Base");
project.addSources("Sources");
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
	project.addParameter("--no-traces");
}
else {
	project.addDefine("arm_noembed");
	project.addAssets("Assets/extra/*", { destination: "data/{name}" });
}

project.addParameter("--macro include('arm.logic')");
project.addParameter("-dce full");
project.addDefine("analyzer-optimize");
project.addDefine("js-es=6");
project.addDefine("zui_translate");
project.addDefine("arm_data_dir");
project.addDefine("arm_ltc");
project.addDefine("arm_skip_envmap");
project.addDefine("arm_taa");
project.addDefine("arm_veloc");
project.addDefine("arm_particles");
// project.addDefine("arm_skin");

if (flags.android) {
	project.addDefine("kha_android_rmb");
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
	project.addDefine("rp_voxels");

	if (process.platform === "win32" && flags.win_hlsl) {
		project.addShaders("Shaders/voxel_hlsl/*.glsl", { embed: flags.snapshot, noprocessing: true });
	}
	else {
		project.addShaders("Shaders/voxel_glsl/*.glsl", { embed: flags.snapshot });
	}
}

resolve(project);
