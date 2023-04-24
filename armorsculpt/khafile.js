
let project = new Project("ArmorSculpt");
project.addDefine("is_sculpt");

await project.addProject("../base");
let flags = globalThis.flags;

project.addSources("../armorpaint/Sources"); ////
project.addSources("Sources");
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

if (flags.plugin_embed) {
	project.addAssets("Assets/plugins/embed/*", { destination: "data/plugins/{name}" });
}
else {
	project.addAssets("Assets/plugins/wasm/*", { destination: "data/plugins/{name}" });
}

if (flags.physics) {
	project.addDefine("arm_physics");
	project.addAssets("Assets/plugins/wasm/ammo/*", { destination: "data/plugins/{name}" });
}

resolve(project);
