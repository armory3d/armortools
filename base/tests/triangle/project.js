
let flags = globalThis.flags;
flags.with_iron = true;
flags.lite = true;

let project = new Project("test");
project.add_project("../../");
project.add_tsfiles("./");
project.add_tsfiles("../../sources/ts/iron");
project.add_shaders("./*.kong");
return project;
