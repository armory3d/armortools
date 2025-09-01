
let flags = globalThis.flags;
flags.with_iron = true;
flags.lite = true;
flags.with_eval = false;

let project = new Project("test");
project.add_project("../../");
project.add_tsfiles("./");
project.add_tsfiles("../../sources/ts");
project.add_shaders("./*.kong");
return project;
