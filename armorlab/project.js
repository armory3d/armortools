
let flags = globalThis.flags;
flags.name = 'ArmorLab';
flags.package = 'org.armorlab';
flags.with_onnx = true;

let project = new Project(flags.name);
project.addDefine("is_lab");
project.addProject("../base");

project.addSources("sources");
project.addSources("sources/nodes");
project.addShaders("shaders/*.glsl", { embed: flags.embed });
project.addAssets("assets/*", { destination: "data/{name}", embed: flags.embed });
project.addAssets("assets/export_presets/*", { destination: "data/export_presets/{name}" });
project.addAssets("assets/keymap_presets/*", { destination: "data/keymap_presets/{name}" });
project.addAssets("assets/licenses/**", { destination: "data/licenses/{name}" });
project.addAssets("assets/plugins/*", { destination: "data/plugins/{name}" });
project.addAssets("assets/meshes/*", { destination: "data/meshes/{name}" });
project.addAssets("assets/models/*.onnx", { destination: "data/models/{name}" });
project.addAssets("assets/models/LICENSE.txt", { destination: "data/models/LICENSE.txt" });
project.addAssets("assets/readme/readme.txt", { destination: "{name}" });

if (flags.android) {
	project.addAssets("assets/readme/readme_android.txt", { destination: "{name}" });
}
else if (flags.ios) {
	project.addAssets("assets/readme/readme_ios.txt", { destination: "{name}" });
}

if (process.platform === "win32") {
	project.addAssets("onnx/win32/*.dll", { destination: "{name}" });
}
else if (process.platform === "linux") {
	project.addAssets("onnx/linux/*.so.*", { destination: "{name}" }); // Versioned lib
}

if (flags.raytrace) {
	if (flags.d3d12) {
		project.addAssets("assets/readme/readme_dxr.txt", { destination: "{name}" });
	}
	else if (flags.vulkan) {
		project.addAssets("assets/readme/readme_vkrt.txt", { destination: "{name}" });
	}
}

return project;
