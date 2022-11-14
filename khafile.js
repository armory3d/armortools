
let debug = false;
let android = process.argv.indexOf("android") >= 0;
let ios = process.argv.indexOf("ios") >= 0;
let win_hlsl = process.platform === "win32" && process.argv.indexOf("opengl") < 0;
let d3d12 = process.argv.indexOf("direct3d12") >= 0;
let vulkan = process.argv.indexOf("vulkan") >= 0;
let raytrace = d3d12 || vulkan;
let metal = process.argv.indexOf("metal") >= 0;
let vr = process.argv.indexOf("--vr") >= 0;
let snapshot = process.argv.indexOf("--snapshot") >= 0;
let plugin_embed = ios;
let physics = !ios;

let project = new Project("ArmorPaint");
project.addSources("Sources");
project.addLibrary("iron");
project.addLibrary("zui");
project.addLibrary("armorbase");
project.addShaders("Shaders/*.glsl", { embed: snapshot});
project.addShaders("armorcore/Shaders/*.glsl", { embed: snapshot});
project.addAssets("Assets/*", { destination: "data/{name}", embed: snapshot });
project.addShaders("Libraries/armorbase/Shaders/common/*.glsl", { embed: snapshot});
project.addAssets("Libraries/armorbase/Assets/common/*", { destination: "data/{name}", embed: snapshot });
if (!snapshot) {
	project.addDefine("arm_noembed");
	project.addAssets("Libraries/armorbase/Assets/common/extra/*", { destination: "data/{name}" });
}
project.addAssets("Assets/export_presets/*", { destination: "data/export_presets/{name}" });
project.addAssets("Assets/keymap_presets/*", { destination: "data/keymap_presets/{name}" });
project.addAssets("Assets/locale/*", { destination: "data/locale/{name}" });
project.addAssets("Assets/licenses/**", { destination: "data/licenses/{name}" });
project.addAssets("Assets/plugins/*", { destination: "data/plugins/{name}" });
if (plugin_embed) {
	project.addAssets("Assets/plugins/embed/*", { destination: "data/plugins/{name}" });
}
else {
	project.addAssets("Assets/plugins/wasm/*", { destination: "data/plugins/{name}" });
}
project.addAssets("Assets/meshes/*", { destination: "data/meshes/{name}" });
project.addAssets("Libraries/armorbase/Assets/licenses/**", { destination: "data/licenses/{name}" });
project.addAssets("Libraries/armorbase/Assets/themes/*.json", { destination: "data/themes/{name}" });
if (metal) {
	project.addShaders("Libraries/armorbase/Shaders/common/metal/*.glsl", { embed: snapshot});
	project.addAssets("Libraries/armorbase/Assets/common/metal/*", { destination: "data/{name}" });
}
project.addDefine("js-es=6");
project.addParameter("--macro include('arm.node.brush')");
project.addDefine("kha_no_ogg");
project.addDefine("zui_translate");
project.addDefine("arm_data_dir");
project.addDefine("arm_ltc");
project.addDefine("arm_appwh");
project.addDefine("arm_skip_envmap");
project.addDefine("arm_resizable");
project.addDefine("arm_taa");
project.addDefine("arm_veloc");
project.addDefine("arm_particles");
// project.addDefine("arm_skin");

if (physics) {
	project.addDefine("arm_physics");
	project.addAssets("Assets/plugins/wasm/ammo/*", { destination: "data/plugins/{name}" });
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

if (android || ios) {
	project.addDefine("arm_touchui"); // Use touch friendly UI
	project.addDefine("zui_touchui");
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

if (vr) {
	project.addDefine("arm_vr");
	project.addAssets("Assets/readme/readme_vr.txt", { destination: "{name}" });
}

if (snapshot) {
	project.addDefine("arm_snapshot");
	project.addDefine("arm_image_embed");
	project.addDefine("arm_shader_embed");
	project.addParameter("--no-traces");
}

project.addAssets("Assets/readme/readme.txt", { destination: "{name}" });

if (raytrace) {
	project.addAssets("Libraries/armorbase/Assets/raytrace/*", { destination: "data/{name}", embed: snapshot });
	if (d3d12) {
		project.addAssets("Libraries/armorbase/Shaders/raytrace/*.cso", { destination: "data/{name}", embed: snapshot });
		project.addAssets("Assets/readme/readme_dxr.txt", { destination: "{name}" });
	}
	else if (vulkan) {
		project.addAssets("Libraries/armorbase/Shaders/raytrace/*.spirv", { destination: "data/{name}", embed: snapshot });
		project.addAssets("Assets/readme/readme_vkrt.txt", { destination: "{name}" });
	}
}

if (android) {
	project.addAssets("Assets/readme/readme_android.txt", { destination: "{name}" });
}
else if (ios) {
	project.addAssets("Assets/readme/readme_ios.txt", { destination: "{name}" });
}

if (process.platform !== "darwin" && !raytrace && !android && !ios) {
	project.addDefine("rp_voxels");
	project.addDefine("arm_voxelgi_revox");

	if (process.platform === "win32" && win_hlsl) {
		project.addShaders("Libraries/armorbase/Shaders/voxel_hlsl/*.glsl", { embed: snapshot, noprocessing: true });
	}
	else {
		project.addShaders("Libraries/armorbase/Shaders/voxel_glsl/*.glsl", { embed: snapshot });
	}
}

resolve(project);
