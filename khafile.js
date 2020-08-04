
let debug = false;
let android = process.argv.indexOf("android") >= 0;
let ios = process.argv.indexOf("ios") >= 0;
let win_hlsl = process.platform === "win32" && process.argv.indexOf("opengl") < 0;
let d3d12 = process.argv.indexOf("direct3d12") >= 0;
let vulkan = process.argv.indexOf("vulkan") >= 0;
let raytrace = d3d12 || vulkan;
let metal = process.argv.indexOf("metal") >= 0;
let build = "painter"; // painter || creator || player

let project = new Project("ArmorPaint");
project.addSources("Sources");
project.addLibrary("iron");
project.addLibrary("zui");
project.addLibrary("syslib");
project.addLibrary("formatlib");
project.addLibrary("geomlib");
project.addLibrary("nodelib");
project.addLibrary("shaderlib");
project.addShaders("Shaders/common/*.glsl", { noembed: false});
project.addAssets("Assets/common/*", { notinlist: true, destination: "data/{name}" });
project.addAssets("Assets/fonts/*", { notinlist: true, destination: "data/{name}" });
project.addAssets("Assets/locale/*", { notinlist: true, destination: "data/locale/{name}" });
project.addAssets("Assets/licenses/*", { notinlist: true, destination: "data/licenses/{name}" });
project.addAssets("Assets/plugins/*", { notinlist: true, destination: "data/plugins/{name}" });
project.addAssets("Assets/themes/*", { notinlist: true, destination: "data/themes/{name}" });
project.addAssets("Assets/meshes/*", { notinlist: true, destination: "data/meshes/{name}" });

project.addDefine("arm_taa");
project.addDefine("arm_veloc");
project.addDefine("arm_particles");

if (!android) {
	project.addDefine("arm_data_dir");
}

if (android) {
	project.addDefine("krom_android");
	project.addDefine("kha_android");
	project.addDefine("kha_android_rmb");
}
else if (ios) {
	project.addDefine("krom_ios");
	project.addDefine("kha_ios");
}
else if (process.platform === "win32") {
	project.addDefine("krom_windows");
	project.addDefine("kha_windows");
}
else if (process.platform === "linux") {
	project.addDefine("krom_linux");
	project.addDefine("kha_linux");
}
else if (process.platform === "darwin") {
	project.addDefine("krom_darwin");
	project.addDefine("kha_darwin");
}

if (debug) {
	project.addDefine("arm_debug");
	project.addParameter("--times");
	// project.addParameter("--no-inline");
}
else {
	project.addParameter("-dce full");
	project.addDefine("analyzer-optimize");
}

project.addAssets("Assets/readme/readme.txt", { notinlist: true, destination: "{name}" });

if (raytrace) {
	project.addLibrary("xenon");
	project.addAssets("Libraries/xenon/Assets/*", { notinlist: true, destination: "data/{name}" });
	if (d3d12) {
		project.addAssets("Libraries/xenon/Shaders/*.cso", { notinlist: true, destination: "data/{name}" });
		project.addAssets("Assets/readme/readme_dxr.txt", { notinlist: true, destination: "{name}" });
	}
	else if (vulkan) {
		project.addAssets("Libraries/xenon/Shaders/*.spirv", { notinlist: true, destination: "data/{name}" });
		project.addAssets("Assets/readme/readme_vkrt.txt", { notinlist: true, destination: "{name}" });
	}
}

if (android) {
	project.addAssets("Assets/readme/readme_android.txt", { notinlist: true, destination: "{name}" });
}
else if (ios) {
	project.addAssets("Assets/readme/readme_ios.txt", { notinlist: true, destination: "{name}" });
}

if (process.platform !== "darwin" && !raytrace && !android && !ios) {
	project.addDefine("rp_voxelao");
	project.addDefine("arm_voxelgi_revox");

	if (process.platform === "win32" && win_hlsl) {
		project.addShaders("Shaders/voxel_hlsl/*.glsl", { noprocessing: true, noembed: false });
	}
	else {
		project.addShaders("Shaders/voxel_glsl/*.glsl", { noembed: false });
	}
}

if (build === "player") {
	project.addDefine("arm_player");
}
else { // painter, creator
	project.addDefine("arm_painter");
	project.addParameter("--macro include('arm.node.brush')");
	project.addDefine("arm_appwh");
	project.addDefine("arm_skip_envmap");
	project.addDefine("arm_resizable");
	if (build === "creator") {
		project.addDefine("arm_creator");
	}

	project.addAssets("Assets/painter/export_presets/*", { notinlist: true, destination: "data/export_presets/{name}" });
	project.addAssets("Assets/painter/keymap_presets/*", { notinlist: true, destination: "data/keymap_presets/{name}" });
}

if (build === "painter") {
	project.addShaders("Shaders/painter/*.glsl", { noembed: false});
	project.addAssets("Assets/painter/*", { notinlist: true, destination: "data/{name}" });
	if (metal) {
		project.addShaders("Shaders/painter/metal/*.glsl", { noembed: false});
		project.addAssets("Assets/painter/metal/*", { notinlist: true, destination: "data/{name}" });
	}
	project.addDefine("kha_no_ogg");
	project.addDefine("arm_ltc");
}
else { // player, creator
	project.addAssets("Assets/creator/*", { notinlist: true, destination: "data/{name}" });
	project.addAssets("Assets/creator/plugins/*", { notinlist: true, destination: "data/plugins/{name}" });
	project.addShaders("Shaders/creator/*.glsl", { noembed: false});
	project.addDefine("arm_audio");
	project.addDefine("arm_soundcompress");
	project.addDefine("arm_skin");
	project.addDefine("arm_world");
	project.addDefine("arm_physics");
}

resolve(project);
