let project = new Project("plugins");

project.addFile("plugins.c");
project.addFile("proc_xatlas/**");
project.addFile("io_svg/**");
project.addFile("io_usd/**");
project.addFile("io_gltf/**");
project.addFile("io_fbx/**");

project.addDefine("TINYUSDZ_NO_STB_IMAGE_IMPLEMENTATION");

if (flags.physics) {
	project.addFile("phys_jolt/phys_jolt.cpp");
	project.addIncludeDir("phys_jolt");
	project.addDefine("JPH_NO_DEBUG");
	project.addDefine("JPH_OBJECT_STREAM");

	if (platform === "windows") {
		project.addLib("phys_jolt/win32/Jolt");
	}
	else if (platform === "linux") {
		project.addLib("Jolt -L" + project.basedir + "/phys_jolt/linux");
	}
	else if (platform === "macos") {
		project.addLib("phys_jolt/macos/libJolt.a");
	}
}

return project;
