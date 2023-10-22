
// Imported by armorcore/kfile.js
flags.name = 'ArmorPaint';
flags.package = 'org.armorpaint';
flags.with_d3dcompiler = true;
flags.with_nfd = true;
flags.with_tinydir = true;
flags.with_zlib = true;
flags.with_stb_image_write = true;
flags.with_g2 = true;
flags.with_iron = true;
flags.with_zui = true;

flags.on_project_created = async function(project) {
	project.addDefine('IDLE_SLEEP');

	if (graphics === 'vulkan') {
		project.addDefine('KORE_VKRT');
		await project.addProject('../armorpaint/glsl_to_spirv');
	}
	if (platform === 'ios') {
		await project.addProject('../armorpaint/plugins');
	}
}
