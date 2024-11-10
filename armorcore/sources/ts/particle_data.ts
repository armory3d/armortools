
function particle_data_parse(name: string, id: string): particle_data_t {
	let format: scene_t = data_get_scene_raw(name);
	let raw: particle_data_t = particle_data_get_raw_by_name(format.particle_datas, id);
	if (raw == null) {
		iron_log("Particle data '" + id + "' not found!");
	}
	return raw;
}

function particle_data_get_raw_by_name(datas: particle_data_t[], name: string): particle_data_t {
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
