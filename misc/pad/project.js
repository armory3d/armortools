
let flags = globalThis.flags;
flags.name = 'ArmorPad';
flags.package = 'org.armorpad';
flags.with_nfd = true;
flags.with_tinydir = true;
flags.with_g2 = true;
flags.with_iron = true;
flags.with_zui = true;
flags.on_c_project_created = function(c_project, platform, graphics) {
	c_project.addDefine('IDLE_SLEEP');
}

let project = new Project("ArmorPad");
let root = "../../";
project.addSources("Sources");
project.addAssets("Assets/themes/*.json", { destination: "data/themes/{name}" });
project.addShaders(root + "armorcore/Shaders/*.glsl",);
project.addAssets(root + "base/Assets/font_mono.ttf", { destination: "data/{name}" });
project.addAssets(root + "base/Assets/text_coloring.json", { destination: "data/{name}" });

resolve(project);
