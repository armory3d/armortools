
let debug = false;
let android = false; // Temp
let ios = false; // Temp
let win_hlsl = true; // GraphicsApi.Direct3D11 && GraphicsApi.Direct3D12
let raytrace = process.argv.indexOf("direct3d12") >= 0;
let build = "painter"; // painter || creator || player

let project = new Project("ArmorPaint");
project.addSources("Sources");
project.addLibrary("iron");
project.addLibrary("zui");
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

if (!android && !ios) {
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
}
else if (process.platform === "linux") {
	project.addDefine("krom_linux");
}
else if (process.platform === "darwin") {
	project.addDefine("krom_darwin");
}

if (debug) {
	project.addDefine("arm_debug");
	project.addShaders("Shaders/debug/*.glsl");
	project.addParameter("--times");
	// project.addParameter("--no-inline");
}
else {
	project.addParameter("-dce full");
}

project.addAssets("Assets/readme/readme.txt", { notinlist: true, destination: "{name}" });

if (raytrace) {
	project.addAssets("Assets/raytrace/*", { notinlist: true, destination: "data/{name}" });
	project.addAssets("Shaders/raytrace/*.cso", { notinlist: true, destination: "data/{name}" });
	project.addAssets("Assets/readme/readme_dxr.txt", { notinlist: true, destination: "{name}" });
}

if (android) {
	project.addAssets("Assets/readme/readme_android.txt", { notinlist: true, destination: "{name}" });
}
else if (ios) {
	project.addAssets("Assets/readme/readme_ios.txt", { notinlist: true, destination: "{name}" });
}
else if (process.platform === "darwin") {
	project.addAssets("Assets/readme/readme_macos.txt", { notinlist: true, destination: "INSTRUCTIONS.txt" });
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

	if (android) {

	}
	else if (ios) {

	}
	else if (process.platform === "win32") {
		project.addAssets("Assets/bin/cmft.exe", { notinlist: true, destination: "data/{name}" });
	}
	else if (process.platform === "linux") {
		project.addAssets("Assets/bin/cmft-linux64", { notinlist: true, destination: "data/{name}" });
	}
	else if (process.platform === "darwin") {
		project.addAssets("Assets/bin/cmft-osx", { notinlist: true, destination: "data/{name}" });
	}

	if (build === "creator") {
		project.addDefine("arm_creator");
	}

	project.addAssets("Assets/painter/export_presets/*", { notinlist: true, destination: "data/export_presets/{name}" });
	project.addAssets("Assets/painter/keymap_presets/*", { notinlist: true, destination: "data/keymap_presets/{name}" });
	if (process.platform === "win32" && win_hlsl && !android && !ios) {
		project.addShaders("Shaders/painter/hlsl/*.glsl", { noprocessing: true, noembed: false });
	}
	else {
		project.addShaders("Shaders/painter/glsl/*.glsl", { noembed: false });
	}
}

if (build === "painter") {
	project.addShaders("Shaders/painter/*.glsl", { noembed: false});
	project.addAssets("Assets/painter/*", { notinlist: true, destination: "data/{name}" });
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
