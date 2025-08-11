
function mesh_data_parse(name: string, id: string): mesh_data_t {
	let format: scene_t = data_get_scene_raw(name);
	let raw: mesh_data_t = mesh_data_get_raw_by_name(format.mesh_datas, id);
	if (raw == null) {
		iron_log("Mesh data '" + id + "' not found!");
		return null;
	}

	let data: mesh_data_t = mesh_data_create(raw);
	return data;
}

function mesh_data_get_raw_by_name(datas: mesh_data_t[], name: string): mesh_data_t {
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

function mesh_data_create(raw: mesh_data_t): mesh_data_t {
	raw._ = {};

	if (!raw.scale_pos) {
		raw.scale_pos = 1.0;
	}
	if (!raw.scale_tex) {
		raw.scale_tex = 1.0;
	}

	raw._.refcount = 0;
	raw._.vertex_buffer_map = map_create();
	raw._.ready = false;
	raw._.structure = mesh_data_get_vertex_struct(raw.vertex_arrays);

	return raw;
}

function mesh_data_get_vertex_struct(vertex_arrays: vertex_array_t[]): gpu_vertex_structure_t {
	let structure: gpu_vertex_structure_t = {};
	for (let i: i32 = 0; i < vertex_arrays.length; ++i) {
		gpu_vertex_struct_add(structure, vertex_arrays[i].attrib, mesh_data_get_vertex_data(vertex_arrays[i].data));
	}
	return structure;
}

function mesh_data_get_vertex_data(data: string): vertex_data_t {
	if (data == "short4norm") {
		return vertex_data_t.I16_4X_NORM;
	}
	else if (data == "short2norm") {
		return vertex_data_t.I16_2X_NORM;
	}
	else {
		return vertex_data_t.I16_4X_NORM;
	}
}

function mesh_data_build_vertices(vertices: buffer_t, vertex_arrays: vertex_array_t[], offset: i32 = 0, fake_uvs: bool = false, uvs_index: i32 = -1) {
	let size: i32 = mesh_data_get_vertex_size(vertex_arrays[0].data);
	let num_verts: i32 = vertex_arrays[0].values.length / size;
	let di: i32 = -1 + offset;
	for (let i: i32 = 0; i < num_verts; ++i) {
		for (let va: i32 = 0; va < vertex_arrays.length; ++va) {
			let l: i32 = mesh_data_get_vertex_size(vertex_arrays[va].data);
			if (fake_uvs && va == uvs_index) { // Add fake uvs if uvs where "asked" for but not found
				for (let j: i32 = 0; j < l; ++j) {
					buffer_set_i16(vertices, ++di * 2, 0);
				}
				continue;
			}
			for (let o: i32 = 0; o < l; ++o) {
				buffer_set_i16(vertices, ++di * 2, vertex_arrays[va].values[i * l + o]);
			}
		}
	}
}

function mesh_data_get_vertex_size(vertex_data: string): i32 {
	if (vertex_data == "short4norm") {
		return 4;
	}
	else if (vertex_data == "short2norm") {
		return 2;
	}
	else {
		return 0;
	}
}

function mesh_data_get_vertex_array(raw: mesh_data_t, name: string): vertex_array_t {
	for (let i: i32 = 0; i < raw.vertex_arrays.length; ++i) {
		if (raw.vertex_arrays[i].attrib == name) {
			return raw.vertex_arrays[i];
		}
	}
	return null;
}

function mesh_data_get(raw: mesh_data_t, vs: vertex_element_t[]): gpu_buffer_t {
	let key: string = "";
	for (let i: i32 = 0; i < vs.length; ++i) {
		let e: vertex_element_t = vs[i];
		key += e.name;
	}
	let vb: gpu_buffer_t = map_get(raw._.vertex_buffer_map, key);
	if (vb == null) {
		let vertex_arrays: vertex_array_t[] = [];
		let has_tex: bool = false;
		let tex_offset: i32 = -1;
		let has_col: bool = false;
		for (let e: i32 = 0; e < vs.length; ++e) {
			if (vs[e].name == "tex") {
				has_tex = true;
				tex_offset = e;
			}
			else if (vs[e].name == "col") {
				has_col = true;
			}
			for (let va: i32 = 0; va < raw.vertex_arrays.length; ++va) {
				let name: string = vs[e].name;
				if (name == raw.vertex_arrays[va].attrib) {
					array_push(vertex_arrays, raw.vertex_arrays[va]);
				}
			}
		}
		// Multi-mat mesh with different vertex structures
		let positions: vertex_array_t = mesh_data_get_vertex_array(raw, "pos");
		let uvs: vertex_array_t = mesh_data_get_vertex_array(raw, "tex");
		let cols: vertex_array_t = mesh_data_get_vertex_array(raw, "col");
		let vstruct: gpu_vertex_structure_t = mesh_data_get_vertex_struct(vertex_arrays);
		let size: i32 = mesh_data_get_vertex_size(positions.data);
		vb = gpu_create_vertex_buffer(math_floor(positions.values.length / size), vstruct);
		let vertices: buffer_t = gpu_lock_vertex_buffer(vb);
		mesh_data_build_vertices(vertices, vertex_arrays, 0, has_tex && uvs == null, tex_offset);
		gpu_vertex_buffer_unlock(vb);
		map_set(raw._.vertex_buffer_map, key, vb);
		if (has_tex && uvs == null) {
			iron_log("Geometry " + raw.name + " is missing UV map");
		}
		if (has_col && cols == null) {
			iron_log("Geometry " + raw.name + " is missing vertex colors");
		}
	}
	return vb;
}

function mesh_data_build(raw: mesh_data_t) {
	if (raw._.ready) {
		return;
	}

	let positions: vertex_array_t = mesh_data_get_vertex_array(raw, "pos");
	let size: i32 = mesh_data_get_vertex_size(positions.data);
	raw._.vertex_buffer = gpu_create_vertex_buffer(math_floor(positions.values.length / size), raw._.structure);
	let vertices: buffer_t = gpu_lock_vertex_buffer(raw._.vertex_buffer);
	mesh_data_build_vertices(vertices, raw.vertex_arrays);
	gpu_vertex_buffer_unlock(raw._.vertex_buffer);

	let struct_str: string = "";
	for (let i: i32 = 0; i < raw._.structure.size; ++i) {
		let e: gpu_vertex_element_t = ADDRESS(ARRAY_ACCESS(raw._.structure.elements, i));
		struct_str += e.name;
	}
	map_set(raw._.vertex_buffer_map, struct_str, raw._.vertex_buffer);

	raw._.index_buffers = [];

	for (let i: i32 = 0; i < raw.index_arrays.length; ++i) {
		let id: u32_array_t = raw.index_arrays[i].values;
		let index_buffer: gpu_buffer_t = gpu_create_index_buffer(id.length);
		let indices_array: u32_array_t = gpu_lock_index_buffer(index_buffer);
		for (let i: i32 = 0; i < indices_array.length; ++i) {
			indices_array[i] = id[i];
		}
		gpu_index_buffer_unlock(index_buffer);
		array_push(raw._.index_buffers, index_buffer);
	}

	raw._.ready = true;
}

function mesh_data_calculate_aabb(raw: mesh_data_t): vec4_t {
	let aabb_min: vec4_t = vec4_create(-0.01, -0.01, -0.01);
	let aabb_max: vec4_t = vec4_create(0.01, 0.01, 0.01);
	let aabb: vec4_t = vec4_create();
	let i: i32 = 0;
	let positions: vertex_array_t = mesh_data_get_vertex_array(raw, "pos");
	while (i < positions.values.length) {
		if (positions.values[i] > aabb_max.x) {
			aabb_max.x = positions.values[i];
		}
		if (positions.values[i + 1] > aabb_max.y) {
			aabb_max.y = positions.values[i + 1];
		}
		if (positions.values[i + 2] > aabb_max.z) {
			aabb_max.z = positions.values[i + 2];
		}
		if (positions.values[i] < aabb_min.x) {
			aabb_min.x = positions.values[i];
		}
		if (positions.values[i + 1] < aabb_min.y) {
			aabb_min.y = positions.values[i + 1];
		}
		if (positions.values[i + 2] < aabb_min.z) {
			aabb_min.z = positions.values[i + 2];
		}
		i += 4;
	}
	aabb.x = (math_abs(aabb_min.x) + math_abs(aabb_max.x)) / 32767 * raw.scale_pos;
	aabb.y = (math_abs(aabb_min.y) + math_abs(aabb_max.y)) / 32767 * raw.scale_pos;
	aabb.z = (math_abs(aabb_min.z) + math_abs(aabb_max.z)) / 32767 * raw.scale_pos;
	return aabb;
}

function mesh_data_delete(raw: mesh_data_t) {
	let vertex_buffer_keys: string[] = map_keys(raw._.vertex_buffer_map);
	for (let i: i32 = 0; i < vertex_buffer_keys.length; ++i) {
		let buf: gpu_buffer_t = map_get(raw._.vertex_buffer_map, vertex_buffer_keys[i]);
		if (buf != null) {
			gpu_delete_buffer(buf);
		}
	}
	for (let i: i32 = 0; i < raw._.index_buffers.length; ++i) {
		let buf: gpu_buffer_t = raw._.index_buffers[i];
		gpu_delete_buffer(buf);
	}
}
