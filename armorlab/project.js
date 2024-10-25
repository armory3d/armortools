
let flags = globalThis.flags;
flags.name = "ArmorLab";
flags.package = "org.armorlab";
flags.with_onnx = true;

let project = new Project(flags.name);
project.add_define("is_lab");
project.add_project("../base");

project.add_tsfiles("sources");
project.add_tsfiles("sources/nodes");
project.add_shaders("shaders/*.glsl");
project.add_assets("assets/*", { destination: "data/{name}" });
project.add_assets("../armorpaint/assets/export_presets/*", { destination: "data/export_presets/{name}" });
project.add_assets("assets/keymap_presets/*", { destination: "data/keymap_presets/{name}" });
project.add_assets("assets/licenses/**", { destination: "data/licenses/{name}" });
project.add_assets("assets/plugins/*", { destination: "data/plugins/{name}" });
project.add_assets("../armorpaint/assets/plugins/hello_world.js", { destination: "data/plugins/{name}" });
project.add_assets("../armorpaint/assets/plugins/hello_node_brush.js", { destination: "data/plugins/{name}" });
project.add_assets("../armorpaint/assets/plugins/import_svg.js", { destination: "data/plugins/{name}" });
project.add_assets("assets/meshes/*", { destination: "data/meshes/{name}", noembed: true });
project.add_assets("assets/models/*.onnx", { destination: "data/models/{name}" });
project.add_assets("assets/models/*.json", { destination: "data/models/{name}" });
project.add_assets("assets/models/LICENSE.txt", { destination: "data/models/LICENSE.txt" });
project.add_assets("assets/readme/readme.txt", { destination: "{name}" });

if (platform === "windows") {
	project.add_assets("onnx/win32/*.dll", { destination: "{name}" });
}
else if (platform === "linux") {
	project.add_assets("onnx/linux/*.so.*", { destination: "{name}" }); // Versioned lib
}

project.flatten();
return project;
