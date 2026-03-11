
let flags     = globalThis.flags;
flags.name    = "ArmorPad";
flags.package = "org.armorpad";

let project = new Project("ArmorPad");
project.add_project("../../");
project.add_cfiles("main.c");

return project;
