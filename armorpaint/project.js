
let flags = globalThis.flags;
flags.name = 'ArmorPaint';
flags.package = 'org.armorpaint';

let project = new Project(flags.name);
project.addDefine("is_paint");
project.addProject("../base");

project.addSources("sources");
project.addSources("sources/nodes");
project.addShaders("shaders/*.glsl");
project.addAssets("assets/*", { destination: "data/{name}" });
project.addAssets("assets/export_presets/*", { destination: "data/export_presets/{name}" });
project.addAssets("assets/keymap_presets/*", { destination: "data/keymap_presets/{name}" });
project.addAssets("assets/licenses/**", { destination: "data/licenses/{name}" });
project.addAssets("assets/plugins/*", { destination: "data/plugins/{name}" });
project.addAssets("assets/meshes/*", { destination: "data/meshes/{name}", noembed: true });
project.addAssets("assets/readme/readme.txt", { destination: "{name}" });

project.flatten();
return project;
