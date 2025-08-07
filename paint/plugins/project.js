let project = new Project("plugins");

project.add_cfiles("plugins.c");
project.add_cfiles("proc_xatlas/**");
project.add_cfiles("io_svg/**");
project.add_cfiles("io_usd/**");
project.add_define("TINYUSDZ_NO_STB_IMAGE_IMPLEMENTATION");
project.add_cfiles("io_gltf/**");
project.add_cfiles("io_fbx/**");

return project;
