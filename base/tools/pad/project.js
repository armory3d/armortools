
let flags = globalThis.flags;
flags.name = "ArmorPad";
flags.package = "org.armorpad";
flags.with_nfd = true;
flags.with_iron = true;
flags.lite = true;

let project = new Project("ArmorPad");
project.add_tsfiles("sources");

project.add_shaders("../../shaders/draw/*.glsl",);
project.add_assets("../../assets/font_mono.ttf", { destination: "data/{name}" });
project.add_assets("../../assets/text_coloring.json", { destination: "data/{name}" });
project.add_tsfiles("../../sources/ts/iron");
project.add_tsfiles("../../sources/ts/file.ts");
project.add_tsfiles("../../sources/ts/path.ts");

project.add_define("IDLE_SLEEP");
project.add_project("../..");

project.flatten();
return project;
