
// Imported by armorcore/kfile.js
flags.name = 'ArmorLab';
flags.package = 'org.armorlab';
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

	project.addDefine('WITH_ONNX');
	project.addIncludeDir("../armorlab/onnx/include");
	if (platform === Platform.Windows) {
		project.addLib('../armorlab/onnx/win32/onnxruntime');
	}
	else if (platform === Platform.Linux) {
		// patchelf --set-rpath . ArmorLab
		project.addLib('onnxruntime -L' + __dirname + '/../armorlab/onnx/linux');
	}
	else if (platform === Platform.OSX) {
		project.addLib('../armorlab/onnx/macos/libonnxruntime.1.14.1.dylib');
	}

	if (graphics === 'vulkan') {
		project.addDefine('KORE_VKRT');
		await project.addProject('../armorlab/glsl_to_spirv');
	}
	await project.addProject('../armorlab/plugins');
}
