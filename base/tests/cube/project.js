
let flags = globalThis.flags;
flags.with_iron = true;
flags.lite = true;
flags.with_eval = false;

let project = new Project("test");
project.add_project("../../");
project.add_tsfiles("sources");
project.add_tsfiles("../../sources/ts");
project.add_shaders("../../shaders/draw/*.kong");
project.add_shaders("shaders/*.kong");
project.add_assets("assets/*", { destination: "data/{name}" });

return project;
