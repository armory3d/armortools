let project = new Project("to_spirv");

project.add_define("KRAFIX_LIBRARY");
project.add_cfiles("to_spirv.cpp");

project.add_include_dir("glslang");
project.add_cfiles("glslang/**");

return project;
