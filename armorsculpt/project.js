
let flags = globalThis.flags;
flags.name = 'ArmorSculpt';
flags.package = 'org.armorsculpt';

let project = new Project(flags.name);
project.addDefine("is_sculpt");
project.addProject("../base");

project.addSources("../armorpaint/Sources"); ////
project.addSources("Sources");
project.addSources("Sources/nodes");
project.addShaders("Shaders/*.glsl", { embed: flags.embed });
project.addAssets("Assets/*", { destination: "data/{name}", embed: flags.embed });
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

return project;
