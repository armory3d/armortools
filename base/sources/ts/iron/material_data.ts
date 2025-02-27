
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
		// "mesh" will fetch both "mesh" and "meshheight" contexts
		if (substring(c.name, 0, name.length) == name) {
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

			let image: image_t = data_get_image(tex.file, false);
			array_push(raw._.textures, image);

			// Set mipmaps
			if (tex.mipmaps != null) {
				let mipmaps: image_t[] = [];
				while (mipmaps.length < tex.mipmaps.length) {
					array_push(mipmaps, null);
				}

				for (let j: i32 = 0; j < tex.mipmaps.length; ++j) {
					let name: string = tex.mipmaps[j];
					let mipimg: image_t = data_get_image(name);
					mipmaps[j] = mipimg;
				}

				image_set_mipmaps(image, mipmaps);
				tex.mipmaps = null;
				tex.generate_mipmaps = false;
			}
			else if (tex.generate_mipmaps == true && image != null) {
				image_gen_mipmaps(image, 1000);
				tex.mipmaps = null;
				tex.generate_mipmaps = false;
			}
		}
	}

	return raw;
}

function material_context_set_tex_params(raw: material_context_t, texture_index: i32, context: shader_context_t, unit_index: i32) {
	// This function is called by mesh_object_t for samplers set using material context
	shader_context_set_tex_params(context, unit_index, raw.bind_textures[texture_index]);
}
