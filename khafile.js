let project = new Project('ArmorPaint');

project.addSources('Sources');
project.addLibrary("iron");
project.addLibrary("zui");
project.addShaders("Shaders/common/*.glsl", { noembed: false});
project.addAssets("Bundled/common/*", { notinlist: true, destination: "data/{name}" });
project.addAssets("Bundled/defaults/*", { notinlist: true, destination: "data/defaults/{name}" });
project.addAssets("Bundled/licenses/*", { notinlist: true, destination: "data/licenses/{name}" });
project.addAssets("Bundled/plugins/*", { notinlist: true, destination: "data/plugins/{name}" });
project.addAssets("Bundled/themes/*", { notinlist: true, destination: "data/themes/{name}" });
project.addAssets("Bundled/readme/readme.txt", { notinlist: true, destination: "{name}" });

project.addDefine('rp_voxelao');
project.addDefine('arm_voxelgi_revox');
project.addDefine('arm_ltc');
project.addDefine('arm_taa');
project.addDefine('arm_veloc');
project.addDefine('arm_particles');
project.addDefine('arm_data_dir');
// project.addDefine('arm_noembed');

if (process.platform === 'win32') {
	project.addDefine('krom_windows');
}
else if (process.platform === 'linux') {
	project.addDefine('krom_linux');
}
else if (process.platform === 'darwin') {
	project.addDefine('krom_darwin');
}

let debug = false;
let raytrace = process.argv.indexOf("direct3d12") >= 0;
let build = 'painter'; // painter || creator || player

if (debug) {
	project.addDefine('arm_debug');
	project.addShaders("Shaders/debug/*.glsl");
	project.addParameter('--times');
	// project.addParameter('--no-inline');
}
else {
	project.addParameter('-dce full');
}

if (raytrace) {
	project.addAssets("Bundled/raytrace/*", { notinlist: true, destination: "data/{name}" });
	project.addAssets("Shaders/raytrace/*.cso", { notinlist: true, destination: "data/{name}" });
	project.addAssets("Bundled/readme/readme_dxr.txt", { notinlist: true, destination: "{name}" });
}

if (process.platform === 'win32') {
	project.addShaders("Shaders/voxel_hlsl/*.glsl", { noprocessing: true, noembed: false });
}
else {
	project.addShaders("Shaders/voxel_glsl/*.glsl", { noembed: false });
}

if (build === 'player') {
	project.addDefine('arm_player');
}
else { // painter, creator
	project.addDefine('arm_painter');
	project.addParameter('--macro include("arm.nodes.brush")');
	project.addDefine('arm_appwh');
	project.addDefine('arm_skip_envmap');
	project.addDefine('arm_resizable');

	if (process.platform === 'win32') {
		project.addAssets("Bundled/bin/cmft.exe", { notinlist: true, destination: "data/{name}" });
	}
	else if (process.platform === 'linux') {
		project.addAssets("Bundled/bin/cmft-linux64", { notinlist: true, destination: "data/{name}" });
	}
	else if (process.platform === 'darwin') {
		project.addAssets("Bundled/bin/cmft-osx", { notinlist: true, destination: "data/{name}" });
	}

	if (build === 'creator') {
		project.addDefine('arm_creator');
	}
}

if (build === 'painter') {
	project.addAssets("Bundled/painter/*", { notinlist: true, destination: "data/{name}" });
	project.addShaders("Shaders/painter/*.glsl", { noembed: false});
	project.addDefine('kha_no_ogg');
}
else {
	project.addAssets("Bundled/creator/*", { notinlist: true, destination: "data/{name}" });
	project.addShaders("Shaders/creator/*.glsl", { noembed: false});
	project.addDefine('arm_audio');
	project.addDefine('arm_soundcompress');
	project.addDefine('arm_skin');
	project.addDefine('arm_world'); // creator || player
}

resolve(project);
