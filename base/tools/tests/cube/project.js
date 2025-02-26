
let flags = globalThis.flags;
flags.with_iron = true;

let project = new Project("Test");
project.add_project("../../../");
project.add_tsfiles("sources");
project.add_shaders("shaders/*.glsl");
project.add_shaders("../../../shaders/*.glsl");
project.add_assets("assets/*", { destination: "data/{name}" });

return project;
