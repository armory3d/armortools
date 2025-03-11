
let flags = globalThis.flags;
flags.with_iron = true;
flags.lite = true;

let project = new Project("test");
project.add_project("../../");
project.add_tsfiles("sources");
project.add_cfiles("../../sources/libs/asim.c");
project.add_tsfiles("../../sources/ts/iron");
project.add_shaders("../../shaders/draw/*.glsl");
project.add_shaders("shaders/*.glsl");
project.add_assets("assets/*", { destination: "data/{name}" });

return project;
