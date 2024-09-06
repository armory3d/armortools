
let flags = globalThis.flags;
flags.name = 'ArmorLab';
flags.package = 'org.armorlab';
flags.with_onnx = true;

let project = new Project(flags.name);
project.addDefine("is_lab");
project.addProject("../base");

project.addSources("sources");
project.addSources("sources/nodes");
project.addShaders("shaders/*.glsl");
project.addAssets("assets/*", { destination: "data/{name}" });
project.addAssets("../armorpaint/assets/export_presets/*", { destination: "data/export_presets/{name}" });
project.addAssets("assets/keymap_presets/*", { destination: "data/keymap_presets/{name}" });
project.addAssets("assets/licenses/**", { destination: "data/licenses/{name}" });
project.addAssets("assets/plugins/*", { destination: "data/plugins/{name}" });
project.addAssets("../armorpaint/assets/plugins/hello_world.js", { destination: "data/plugins/{name}" });
project.addAssets("../armorpaint/assets/plugins/hello_node_brush.js", { destination: "data/plugins/{name}" });
project.addAssets("../armorpaint/assets/plugins/import_svg.js", { destination: "data/plugins/{name}" });
project.addAssets("assets/meshes/*", { destination: "data/meshes/{name}", noembed: true });
project.addAssets("assets/models/*.onnx", { destination: "data/models/{name}" });
project.addAssets("assets/models/LICENSE.txt", { destination: "data/models/LICENSE.txt" });
project.addAssets("assets/readme/readme.txt", { destination: "{name}" });

if (platform === "win32") {
	project.addAssets("onnx/win32/*.dll", { destination: "{name}" });
}
else if (platform === "linux") {
	project.addAssets("onnx/linux/*.so.*", { destination: "{name}" }); // Versioned lib
}

return project;
