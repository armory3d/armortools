
let flags = globalThis.flags;
flags.name = 'ArmorPad';
flags.package = 'org.armorpad';
flags.with_nfd = true;
flags.with_tinydir = true;
flags.with_g2 = true;
flags.with_iron = true;
flags.with_zui = true;
flags.on_c_project_created = function(c_project) {
	c_project.addDefine('IDLE_SLEEP');
}

let project = new Project("ArmorPad");
let root = "../../";
project.addSources("sources");
project.addShaders(root + "armorcore/shaders/*.glsl",);
project.addAssets(root + "base/assets/font_mono.ttf", { destination: "data/{name}" });
project.addAssets(root + "base/assets/text_coloring.json", { destination: "data/{name}" });

return project;
