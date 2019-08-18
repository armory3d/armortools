let project = new Project('ArmorPaint');

project.addSources('Sources');
project.addLibrary("iron");
project.addLibrary("zui");
project.addShaders("compiled/Shaders/*.glsl", { noembed: false});
project.addAssets("compiled/Assets/**", { notinlist: true , destination: "data/{name}" });
project.addAssets("compiled/Shaders/*.arm", { notinlist: true , destination: "data/{name}" });
project.addAssets("Bundled/data/**", { notinlist: true , destination: "data/{name}" });
project.addAssets("Bundled/defaults/**", { notinlist: true , destination: "data/defaults/{name}" });
project.addAssets("Bundled/licenses/**", { notinlist: true , destination: "data/licenses/{name}" });
project.addAssets("Bundled/plugins/**", { notinlist: true , destination: "data/plugins/{name}" });
project.addAssets("Bundled/themes/**", { notinlist: true , destination: "data/themes/{name}" });
project.addAssets("Bundled/readme.txt", { notinlist: true , destination: "{name}" });
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
project.addDefine('arm_appwh');
project.addDefine('arm_skip_envmap');
project.addDefine('arm_particles');
project.addDefine('arm_config');
project.addDefine('arm_resizable');
project.addDefine('arm_data_dir');
project.addDefine('kha_no_ogg');
// project.addDefine('arm_audio');
// project.addDefine('arm_noembed');
// project.addDefine('arm_soundcompress');
// project.addDefine('arm_skin');
project.addParameter('--macro include("arm.nodes.brush")');
project.addParameter('-dce full');
// project.addParameter('--no-inline');

let debug = false;
if (debug) {
	project.addDefine('arm_debug');
	project.addShaders("Bundled/Shaders/debug_draw/**");
	project.addParameter('--times');
}

let raytrace = process.argv.indexOf("direct3d12") >= 0;
if (raytrace) {
	project.addAssets("Bundled/raytrace/**", { notinlist: true , destination: "data/{name}" });
}

if (process.platform === 'win32') {
	project.addShaders("compiled/Hlsl/*.glsl", { noprocessing: true, noembed: false });
}
else {
	project.addShaders("compiled/Glsl/*.glsl", { noembed: false });
}

if (process.platform === 'win32') {
	project.addDefine('krom_windows');
	project.addAssets("Bundled/cmft/cmft.exe", { notinlist: true , destination: "data/{name}" });
}
else if (process.platform === 'linux') {
	project.addDefine('krom_linux');
	project.addAssets("Bundled/cmft/cmft-linux64", { notinlist: true , destination: "data/{name}" });
}
else if (process.platform === 'darwin') {
	project.addDefine('krom_darwin');
	project.addAssets("Bundled/cmft/cmft-osx", { notinlist: true , destination: "data/{name}" });
}

let world = false;
if (world) {
	project.addAssets("Bundled/ext/**", { notinlist: true , destination: "data/{name}" });
	project.addShaders("Bundled/ext/*.glsl", { noembed: false });
	project.addDefine('rp_water');
}

resolve(project);
