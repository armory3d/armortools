
let flags = globalThis.flags;
flags.name = 'ArmorForge';
flags.package = 'org.armorforge';
flags.with_mpeg_write = true;

let project = new Project(flags.name);
project.addDefine("is_forge");
project.addDefine("is_paint");
await project.addProject("../base");

project.addSources("../armorpaint/Sources"); ////
project.addShaders("../armorpaint/Shaders/*.glsl", { embed: flags.snapshot }); ////
project.addSources("Sources");
project.addSources("Sources/nodes");
project.addShaders("Shaders/*.glsl", { embed: flags.snapshot });
project.addAssets("Assets/*", { destination: "data/{name}", embed: flags.snapshot });
project.addAssets("Assets/keymap_presets/*", { destination: "data/keymap_presets/{name}" });
project.addAssets("Assets/licenses/**", { destination: "data/licenses/{name}" });
project.addAssets("Assets/plugins/*", { destination: "data/plugins/{name}" });
project.addAssets("Assets/meshes/*", { destination: "data/meshes/{name}" });
project.addAssets("Assets/readme/readme.txt", { destination: "{name}" });

if (flags.android) {
	project.addAssets("Assets/readme/readme_android.txt", { destination: "{name}" });
}
else if (flags.ios) {
	project.addAssets("Assets/readme/readme_ios.txt", { destination: "{name}" });
}

if (flags.physics) {
	project.addDefine("arm_physics");
	project.addAssets("Assets/plugins/ammo/*", { destination: "data/plugins/{name}" });
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
