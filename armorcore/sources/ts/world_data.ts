
let _world_data_empty_irr: f32_array_t = null;

function world_data_parse(name: string, id: string): world_data_t {
	let format: scene_t = data_get_scene_raw(name);
	let raw: world_data_t = world_data_get_raw_by_name(format.world_datas, id);
	if (raw == null) {
		iron_log("World data '" + id + "' not found!");
		return null;
	}

	raw._ = {};
	raw._.radiance_mipmaps = [];

	let irr: f32_array_t = world_data_set_irradiance(raw);
	raw._.irradiance = irr;
	if (raw.radiance != null) {
		let rad: image_t = data_get_image(raw.radiance);
		raw._.radiance = rad;
		while (raw._.radiance_mipmaps.length < raw.radiance_mipmaps) {
			array_push(raw._.radiance_mipmaps, null);
		}
		let dot: i32 = string_last_index_of(raw.radiance, ".");
		let ext: string = substring(raw.radiance, dot, raw.radiance.length);
		let base: string = substring(raw.radiance, 0, dot);

		for (let i: i32 = 0; i < raw.radiance_mipmaps; ++i) {
			let mipimg: image_t = data_get_image(base + "_" + i + ext, true);
			raw._.radiance_mipmaps[i] = mipimg;
		}
		image_set_mipmaps(raw._.radiance, raw._.radiance_mipmaps);
	}

	return raw;
}

function world_data_get_raw_by_name(datas: world_data_t[], name: string): world_data_t {
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

function world_data_get_empty_irradiance(): f32_array_t {
	if (_world_data_empty_irr == null) {
		_world_data_empty_irr = f32_array_create(28);
		for (let i: i32 = 0; i < _world_data_empty_irr.length; ++i) {
			_world_data_empty_irr[i] = 0.0;
		}
	}
	return _world_data_empty_irr;
}

function world_data_set_irradiance(raw: world_data_t): f32_array_t {
	if (raw.irradiance == null) {
		return world_data_get_empty_irradiance();
	}
	else {
		let b: buffer_t = data_get_blob(raw.irradiance + ".arm");
		let irradiance_parsed: irradiance_t = armpack_decode(b);
		let irr: f32_array_t = f32_array_create(28); // Align to mult of 4 - 27->28
		for (let i: i32 = 0; i < 27; ++i) {
			irr[i] = irradiance_parsed.irradiance[i];
		}
		return irr;
	}
}

function world_data_load_envmap(raw: world_data_t) {
	if (raw.envmap != null) {
		let image: image_t = data_get_image(raw.envmap);
		raw._.envmap = image;
	}
}
