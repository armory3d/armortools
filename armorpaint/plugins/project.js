let project = new Project("plugins");

project.addFile("sources/plugins.c");
project.addFile("sources/proc_xatlas/**");

project.addDefine("TINYUSDZ_NO_STB_IMAGE_IMPLEMENTATION");

if (flags.physics) {
	project.addFile("sources/phys_jolt/phys_jolt.cpp");
	project.addIncludeDir("sources/phys_jolt");
	project.addDefine("JPH_NO_DEBUG");
	project.addDefine("JPH_OBJECT_STREAM");

	if (platform === "windows") {
		project.addLib("sources/phys_jolt/win32/Jolt");
	}
	else if (platform === "linux") {
		project.addLib("Jolt -L" + project.basedir + "/sources/phys_jolt/linux");
	}
	else if (platform === "macos") {
		project.addLib("sources/phys_jolt/macos/libJolt.a");
	}
}

return project;
