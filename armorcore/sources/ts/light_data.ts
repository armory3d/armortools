
function light_data_parse(name: string, id: string): light_data_t {
	let format: scene_t = data_get_scene_raw(name);
	let raw: light_data_t = light_data_get_raw_by_name(format.light_datas, id);
	if (raw == null) {
		iron_log("Light data '" + id + "' not found!");
	}
	return raw;
}

function light_data_get_raw_by_name(datas: light_data_t[], name: string): light_data_t {
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
