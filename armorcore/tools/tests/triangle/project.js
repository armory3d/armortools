
let flags = globalThis.flags;
flags.with_iron = true;

let project = new Project("Test");
project.add_project("../../../");
project.add_tsfiles("./");
return project;
