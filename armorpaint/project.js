
let flags = globalThis.flags;
flags.name = "ArmorPaint";
flags.package = "org.armorpaint";

let project = new Project(flags.name);
project.add_define("is_paint");
project.add_project("../base");

project.add_tsfiles("sources");
project.add_tsfiles("sources/nodes");
project.add_shaders("shaders/*.glsl");
project.add_assets("assets/*", { destination: "data/{name}" });
project.add_assets("assets/export_presets/*", { destination: "data/export_presets/{name}" });
project.add_assets("assets/keymap_presets/*", { destination: "data/keymap_presets/{name}" });
project.add_assets("assets/licenses/**", { destination: "data/licenses/{name}" });
project.add_assets("assets/plugins/*", { destination: "data/plugins/{name}" });
project.add_assets("assets/meshes/*", { destination: "data/meshes/{name}", noembed: true });
project.add_assets("assets/readme/readme.txt", { destination: "{name}" });

project.flatten();
return project;
