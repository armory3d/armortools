
function _util_mesh_unique_data_count(): i32 {
	let count: i32 = 0;
	for (let i: i32 = 0; i < project_paint_objects.length; ++i) {
		let found: bool = false;
		for (let j: i32 = 0; j < i; ++j) {
			if (project_paint_objects[i].data == project_paint_objects[j].data) {
				found = true;
				break;
			}
		}
		if (!found) {
			count++;
		}
	}
	return count;
}

function util_mesh_ext_pack_uvs(texa: i16_array_t) {
    // Scale tex coords into global atlas
	let atlas_w: i32 = config_get_texture_res();
	let item_i: i32 = _util_mesh_unique_data_count();
	let item_w: i32 = 2048;
	let atlas_stride: i32 = atlas_w / item_w;
	let atlas_step: i32 = 32767 / atlas_stride;
	let item_x: i32 = (item_i % atlas_stride) * atlas_step;
	let item_y: i32 = math_floor(item_i / atlas_stride) * atlas_step;
	for (let i: i32 = 0; i < texa.length / 2; ++i) {
		texa[i * 2] = texa[i * 2] / atlas_stride + item_x;
		texa[i * 2 + 1] = texa[i * 2 + 1] / atlas_stride + item_y;
	}
}
