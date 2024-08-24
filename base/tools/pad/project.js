
let flags = globalThis.flags;
flags.name = 'ArmorPad';
flags.package = 'org.armorpad';
flags.with_nfd = true;
flags.with_tinydir = true;
flags.with_g2 = true;
flags.with_iron = true;
flags.with_zui = true;

let project = new Project("ArmorPad");
project.addSources("sources");

let root = "../../../";
project.addShaders(root + "armorcore/shaders/*.glsl",);
project.addAssets(root + "base/assets/font_mono.ttf", { destination: "data/{name}" });
project.addAssets(root + "base/assets/text_coloring.json", { destination: "data/{name}" });

project.addDefine('IDLE_SLEEP');
project.addProject(root + "armorcore");

return project;
