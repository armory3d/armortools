let project = new Project("plugins");

project.add_cfiles("plugins.c");

if (flags.physics) {
	project.add_cfiles("phys_jolt/phys_jolt.cpp");
	project.add_cfiles("phys_jolt/Jolt/**");
	project.add_include_dir("phys_jolt");
	project.add_define("JPH_NO_DEBUG");
}

return project;
