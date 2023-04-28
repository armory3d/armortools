
let project = new Project("ArmorPad");
let root = "../../";
project.addSources("Sources");
project.addAssets("Assets/themes/*.json", { destination: "data/themes/{name}" });
project.addShaders(root + "armorcore/Shaders/*.glsl",);
project.addAssets(root + "base/Assets/font_mono.ttf", { destination: "data/{name}" });
project.addAssets(root + "base/Assets/text_coloring.json", { destination: "data/{name}" });

resolve(project);
