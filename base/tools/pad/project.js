
let flags = globalThis.flags;
flags.name = "ArmorPad";
flags.package = "org.armorpad";

let project = new Project("ArmorPad");
project.add_project("../..");
project.add_tsfiles("sources");
project.flatten();
return project;
