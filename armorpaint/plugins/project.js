let project = new Project("plugins");

project.add_cfiles("plugins.c");
project.add_cfiles("proc_xatlas/**");
project.add_cfiles("io_svg/**");
project.add_cfiles("io_usd/**");
project.add_define("TINYUSDZ_NO_STB_IMAGE_IMPLEMENTATION");
project.add_cfiles("io_gltf/**");
project.add_cfiles("io_fbx/**");

if (flags.physics) {
	project.add_cfiles("phys_jolt/phys_jolt.cpp");
	project.add_cfiles("phys_jolt/Jolt/**");
	project.add_include_dir("phys_jolt");
	project.add_define("JPH_NO_DEBUG");
}

return project;
