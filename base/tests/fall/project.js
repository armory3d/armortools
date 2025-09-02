
let flags = globalThis.flags;
flags.with_physics = true;

let project = new Project("test");
project.add_project("../../");
project.add_tsfiles("sources");
project.add_shaders("shaders/*.kong");
project.add_assets("assets/*", { destination: "data/{name}" });

return project;
