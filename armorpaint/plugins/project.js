let project = new Project('plugins');

// project.addFile('sources/**');
project.addFile('sources/plugins.cpp');

project.addDefine('TINYUSDZ_NO_STB_IMAGE_IMPLEMENTATION');

if (platform === "windows") {
	project.addLib('sources/phys_jolt/win32/jolt');
}
else if (platform === "linux") {
	process.env.LIBRARY_PATH = project.basedir + "/sources/phys_jolt/linux";
	project.addLib('jolt');
}
else if (platform === "macos") {
	project.addLib('sources/phys_jolt/macos/libjolt.a');
}

return project;
