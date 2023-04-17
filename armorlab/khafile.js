
let project = new Project("ArmorLab");
project.addDefine("is_lab");

await project.addProject("../base");
let flags = globalThis.flags;

project.addSources("Sources");
project.addShaders("Shaders/*.glsl", { embed: flags.snapshot });
project.addAssets("Assets/*", { destination: "data/{name}", embed: flags.snapshot });
project.addAssets("Assets/export_presets/*", { destination: "data/export_presets/{name}" });
project.addAssets("Assets/keymap_presets/*", { destination: "data/keymap_presets/{name}" });
project.addAssets("Assets/licenses/**", { destination: "data/licenses/{name}" });
project.addAssets("Assets/plugins/*", { destination: "data/plugins/{name}" });
project.addAssets("Assets/meshes/*", { destination: "data/meshes/{name}" });
project.addAssets("Assets/models/*.onnx", { destination: "data/models/{name}" });
project.addAssets("Assets/models/LICENSE.txt", { destination: "data/models/LICENSE.txt" });
project.addAssets("Assets/readme/readme.txt", { destination: "{name}" });

if (flags.android) {
	project.addAssets("Assets/readme/readme_android.txt", { destination: "{name}" });
}
else if (flags.ios) {
	project.addAssets("Assets/readme/readme_ios.txt", { destination: "{name}" });
}

if (process.platform === "win32") {
	project.addAssets("../armorcore/Libraries/onnx/win32/*.dll", { destination: "{name}" });
}
else if (process.platform === "linux") {
	project.addAssets("../armorcore/Libraries/onnx/linux/*.so.*", { destination: "{name}" }); // Versioned lib
}

if (flags.raytrace) {
	if (flags.d3d12) {
		project.addAssets("Assets/readme/readme_dxr.txt", { destination: "{name}" });
	}
	else if (flags.vulkan) {
		project.addAssets("Assets/readme/readme_vkrt.txt", { destination: "{name}" });
	}
}

resolve(project);
