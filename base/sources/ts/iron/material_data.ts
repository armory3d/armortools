
let _material_data_uid_counter: i32 = 0;

function material_data_create(raw: material_data_t, file: string = ""): material_data_t {
	raw._ = {};
	raw._.uid = ++_material_data_uid_counter; // Start from 1

	let ref: string[] = string_split(raw.shader, "/");
	let object_file: string = "";
	let data_ref: string = "";
	if (ref.length == 2) { // File reference
		object_file = ref[0];
		data_ref = ref[1];
	}
	else { // Local data
		object_file = file;
		data_ref = raw.shader;
	}

	let b: shader_data_t = data_get_shader(object_file, data_ref);
	raw._.shader = b;

	// Contexts have to be in the same order as in raw data for now
	raw._.contexts = [];
	while (raw._.contexts.length < raw.contexts.length) {
		array_push(raw._.contexts, null);
	}

	for (let i: i32 = 0; i < raw.contexts.length; ++i) {
		let c: material_context_t = raw.contexts[i];
		let self: material_context_t = material_context_create(c);
		raw._.contexts[i] = self;
	}
	return raw;
}

function material_data_parse(file: string, name: string): material_data_t {
	let format: scene_t = data_get_scene_raw(file);
	let raw: material_data_t = material_data_get_raw_by_name(format.material_datas, name);
	if (raw == null) {
		iron_log("Material data '" + name + "' not found!");
		return null;
	}
	return material_data_create(raw, file);
}

function material_data_get_raw_by_name(datas: material_data_t[], name: string): material_data_t {
	if (name == "") {
		return datas[0];
	}
	for (let i: i32 = 0; i < datas.length; ++i) {
		if (datas[i].name == name) {
			return datas[i];
		}
	}
	return null;
}

function material_data_get_context(raw: material_data_t, name: string): material_context_t {
	for (let i: i32 = 0; i < raw._.contexts.length; ++i) {
		let c: material_context_t = raw._.contexts[i];
		if (c.name == name) {
			return c;
		}
	}
	return null;
}

function material_context_create(raw: material_context_t): material_context_t {
	raw._ = {};
	if (raw.bind_textures != null && raw.bind_textures.length > 0) {
		raw._.textures = [];
		for (let i: i32 = 0; i < raw.bind_textures.length; ++i) {
			let tex: bind_tex_t = raw.bind_textures[i];
			if (tex.file == "") { // Empty texture
				continue;
			}
			let image: gpu_texture_t = data_get_image(tex.file);
			array_push(raw._.textures, image);
		}
	}

	return raw;
}
