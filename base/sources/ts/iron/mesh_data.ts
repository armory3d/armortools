
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
	if (raw.scale_pos == 0.0) {
		raw.scale_pos = 1.0;
	}
	if (raw.scale_tex == 0.0) {
		raw.scale_tex = 1.0;
	}
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
	return vertex_data_t.I16_2X_NORM; // short2norm
}

function mesh_data_get_vertex_size(vertex_data: string): i32 {
	if (vertex_data == "short4norm") {
		return 4;
	}
	return 2; // short2norm
}

function mesh_data_build_vertices(vertices: buffer_t, vertex_arrays: vertex_array_t[]) {
	let size: i32 = mesh_data_get_vertex_size(vertex_arrays[0].data);
	let num_verts: i32 = vertex_arrays[0].values.length / size;
	let di: i32 = -1;
	for (let i: i32 = 0; i < num_verts; ++i) {
		for (let va: i32 = 0; va < vertex_arrays.length; ++va) {
			let l: i32 = mesh_data_get_vertex_size(vertex_arrays[va].data);
			for (let o: i32 = 0; o < l; ++o) {
				buffer_set_i16(vertices, ++di * 2, vertex_arrays[va].values[i * l + o]);
			}
		}
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

function mesh_data_build(raw: mesh_data_t) {
	let positions: vertex_array_t = mesh_data_get_vertex_array(raw, "pos");
	let size: i32 = mesh_data_get_vertex_size(positions.data);
	raw._.vertex_buffer = gpu_create_vertex_buffer(math_floor(positions.values.length / size), raw._.structure);
	let vertices: buffer_t = gpu_lock_vertex_buffer(raw._.vertex_buffer);
	mesh_data_build_vertices(vertices, raw.vertex_arrays);
	gpu_vertex_buffer_unlock(raw._.vertex_buffer);

	let id: u32_array_t = raw.index_array;
	let index_buffer: gpu_buffer_t = gpu_create_index_buffer(id.length);
	let indices_array: u32_array_t = gpu_lock_index_buffer(index_buffer);
	for (let i: i32 = 0; i < indices_array.length; ++i) {
		indices_array[i] = id[i];
	}
	gpu_index_buffer_unlock(index_buffer);
	raw._.index_buffer = index_buffer;
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
	gpu_delete_buffer(raw._.vertex_buffer);
	gpu_delete_buffer(raw._.index_buffer);
}
