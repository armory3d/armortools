let project = new Project('Kongruent');

// project.add_define('KONG_LIBRARY');
project.add_cfiles('sources/libs/*.c');
project.add_cfiles('sources/*.c');
project.add_cfiles('sources/backends/*.c');
// project.add_cfiles('sources/backends/*.cpp'); // d3d12.cpp

if (platform === "windows") {
	project.add_define('_CRT_SECURE_NO_WARNINGS');
	project.add_lib('d3dcompiler');
	// project.add_include_dir('sources/libs/dxc/inc');
	// project.add_lib('sources/libs/dxc/lib/x64/dxcompiler');
}

return project;
