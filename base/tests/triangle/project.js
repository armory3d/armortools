
let flags = globalThis.flags;

let project = new Project("test");
project.add_project("../../");
project.add_cfiles("./");
project.add_shaders("./*.kong");
return project;
