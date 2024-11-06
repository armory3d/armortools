
let flags = globalThis.flags;
flags.name = "ArmorSculpt";
flags.package = "org.armorsculpt";

let project = new Project(flags.name);
project.add_define("is_sculpt");
project.add_project("../base");

project.add_tsfiles("../armorpaint/sources");
project.add_tsfiles("../armorpaint/sources/nodes");
project.add_shaders("../armorpaint/shaders/*.glsl");
project.add_tsfiles("sources");
project.add_tsfiles("sources/nodes");
project.add_shaders("shaders/*.glsl");
project.add_assets("assets/*", { destination: "data/{name}" });
project.add_assets("assets/keymap_presets/*", { destination: "data/keymap_presets/{name}" });
project.add_assets("assets/licenses/**", { destination: "data/licenses/{name}" });
project.add_assets("assets/plugins/*", { destination: "data/plugins/{name}" });
project.add_assets("../armorpaint/assets/plugins/hello_world.js", { destination: "data/plugins/{name}" });
project.add_assets("assets/meshes/*", { destination: "data/meshes/{name}", noembed: true });
project.add_assets("assets/readme/readme.txt", { destination: "{name}" });

project.flatten();
return project;
