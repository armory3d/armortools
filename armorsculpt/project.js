
let flags = globalThis.flags;
flags.name = 'ArmorSculpt';
flags.package = 'org.armorsculpt';

let project = new Project(flags.name);
project.addDefine("is_sculpt");
project.addProject("../base");

project.addSources("../armorpaint/sources");
project.addSources("../armorpaint/sources/nodes");
project.addSources("sources");
project.addSources("sources/nodes");
project.addShaders("shaders/*.glsl", { embed: flags.embed });
project.addAssets("assets/*", { destination: "data/{name}", embed: flags.embed });
project.addAssets("assets/keymap_presets/*", { destination: "data/keymap_presets/{name}" });
project.addAssets("assets/licenses/**", { destination: "data/licenses/{name}" });
project.addAssets("assets/plugins/*", { destination: "data/plugins/{name}" });
project.addAssets("../armorpaint/assets/plugins/hello_world.js", { destination: "data/plugins/{name}" });
project.addAssets("assets/meshes/*", { destination: "data/meshes/{name}" });
project.addAssets("assets/readme/readme.txt", { destination: "{name}" });

return project;
