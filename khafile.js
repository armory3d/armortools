let project = new Project('ArmorPaint');

project.addSources('Sources');
project.addLibrary("iron");
project.addLibrary("zui");
project.addShaders("Bundled/Shaders/*.glsl", { noembed: false});
project.addAssets("Bundled/Shaders/*.arm", { notinlist: true, destination: "data/{name}" });
project.addAssets("Bundled/data/**", { notinlist: true, destination: "data/{name}" });
project.addAssets("Bundled/defaults/**", { notinlist: true, destination: "data/defaults/{name}" });
project.addAssets("Bundled/licenses/**", { notinlist: true, destination: "data/licenses/{name}" });
project.addAssets("Bundled/plugins/**", { notinlist: true, destination: "data/plugins/{name}" });
project.addAssets("Bundled/themes/**", { notinlist: true, destination: "data/themes/{name}" });
project.addAssets("Bundled/readme.txt", { notinlist: true, destination: "{name}" });
project.addDefine('arm_deferred');
project.addDefine('arm_voxelgi_revox');
project.addDefine('arm_ltc');
project.addDefine('rp_hdr');
project.addDefine('rp_renderer=Deferred');
project.addDefine('rp_background=World');
project.addDefine('rp_render_to_texture');
project.addDefine('rp_compositornodes');
project.addDefine('rp_antialiasing=TAA');
project.addDefine('arm_veloc');
project.addDefine('arm_taa');
project.addDefine('rp_supersampling=4');
project.addDefine('rp_ssgi=RTAO');
project.addDefine('rp_bloom');
project.addDefine('rp_ssr');
project.addDefine('rp_overlays');
project.addDefine('rp_voxelao');
project.addDefine('rp_voxelgi_resolution=256');
project.addDefine('rp_voxelgi_resolution_z=1.0');
project.addDefine('rp_gbuffer2');
project.addDefine('arm_particles');
project.addDefine('arm_config');
project.addDefine('arm_data_dir');
// project.addDefine('arm_noembed');

let debug = false;
let raytrace = process.argv.indexOf("direct3d12") >= 0;
let build = 'painter'; // painter || creator || player

if (debug) {
	project.addDefine('arm_debug');
	project.addShaders("Bundled/Shaders/debug_draw/*");
	project.addParameter('--times');
	// project.addParameter('--no-inline');
}
else {
	project.addParameter('-dce full');
}

if (raytrace) {
	project.addAssets("Bundled/Shaders/raytrace/*", { notinlist: true, destination: "data/{name}" });
}

if (process.platform === 'win32') {
	project.addShaders("Bundled/Shaders/hlsl/*.glsl", { noprocessing: true, noembed: false });
}
else {
	project.addShaders("Bundled/Shaders/glsl/*.glsl", { noembed: false });
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
		project.addDefine('krom_windows');
		project.addAssets("Bundled/bin/cmft.exe", { notinlist: true, destination: "data/{name}" });
	}
	else if (process.platform === 'linux') {
		project.addDefine('krom_linux');
		project.addAssets("Bundled/bin/cmft-linux64", { notinlist: true, destination: "data/{name}" });
	}
	else if (process.platform === 'darwin') {
		project.addDefine('krom_darwin');
		project.addAssets("Bundled/bin/cmft-osx", { notinlist: true, destination: "data/{name}" });
	}

	if (build === 'creator') {
		project.addDefine('arm_creator');
	}
}

if (build === 'painter') {
	project.addDefine('kha_no_ogg');
}
else {
	project.addAssets("Bundled/creator/**", { notinlist: true, destination: "data/{name}" });
	project.addShaders("Bundled/creator/*.glsl", { noembed: false });
	project.addDefine('rp_water');
	project.addDefine('arm_hosek');
	project.addDefine('arm_audio');
	project.addDefine('arm_soundcompress');
	project.addDefine('arm_skin');
}

resolve(project);
