
let flags = globalThis.flags;
flags.name = 'ArmorForge';
flags.package = 'org.armorforge';
flags.with_mpeg_write = true;

let project = new Project(flags.name);
project.addDefine("is_forge");
project.addDefine("is_paint");
project.addProject("../base");

project.addSources("../armorpaint/sources");
project.addSources("../armorpaint/sources/nodes");
project.addShaders("../armorpaint/shaders/*.glsl");
project.addSources("sources");
project.addSources("sources/nodes");
project.addShaders("shaders/*.glsl");
project.addAssets("assets/*", { destination: "data/{name}" });
project.addAssets("assets/keymap_presets/*", { destination: "data/keymap_presets/{name}" });
project.addAssets("assets/licenses/**", { destination: "data/licenses/{name}" });
project.addAssets("assets/plugins/*", { destination: "data/plugins/{name}" });
project.addAssets("../armorpaint/assets/plugins/hello_world.js", { destination: "data/plugins/{name}" });
project.addAssets("assets/meshes/*", { destination: "data/meshes/{name}", noembed: true });
project.addAssets("assets/readme/readme.txt", { destination: "{name}" });

return project;
