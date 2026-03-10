
let flags = globalThis.flags;

let project = new Project("test");
project.add_project("../../");
project.add_cfiles("sources/main.c");
project.add_shaders("shaders/*.kong");
project.add_assets("assets/*", {destination : "data/{name}"});

return project;
