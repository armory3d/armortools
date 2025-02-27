
function camera_data_parse(name: string, id: string): camera_data_t {
	let format: scene_t = data_get_scene_raw(name);
	let raw: camera_data_t = camera_data_get_raw_by_name(format.camera_datas, id);
	if (raw == null) {
		iron_log("Camera data '" + id + "' not found!");
	}
	return raw;
}

function camera_data_get_raw_by_name(datas: camera_data_t[], name: string): camera_data_t {
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
