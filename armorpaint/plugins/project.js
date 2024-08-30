let project = new Project("plugins");

project.addFile("plugins.c");
project.addFile("proc_xatlas/**");
project.addFile("io_svg/**");
project.addFile("io_usd/**");
project.addDefine("TINYUSDZ_NO_STB_IMAGE_IMPLEMENTATION");
project.addFile("io_gltf/**");
project.addFile("io_fbx/**");

if (flags.physics) {
	project.addFile("phys_jolt/phys_jolt.cpp");
	project.addFile("phys_jolt/Jolt/**");
	project.addIncludeDir("phys_jolt");
	project.addDefine("JPH_NO_DEBUG");
}

return project;
