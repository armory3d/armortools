
function mesh_data_parse(name: string, id: string): mesh_data_t {
	let format: scene_t = data_get_scene_raw(name);
	let raw: mesh_data_t = mesh_data_get_raw_by_name(format.mesh_datas, id);
	if (raw == null) {
		iron_log("Mesh data '" + id + "' not found!");
		return null;
	}

	let data: mesh_data_t = mesh_data_create(raw);
	///if arm_skin
	if (raw.skin != null) {
		mesh_data_init_skeleton_transforms(data, raw.skin.transforms_inv);
	}
	///end
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

	// Mesh data
	let indices: u32_array_t[] = [];
	let material_indices: i32[] = [];

	for (let i: i32 = 0; i < raw.index_arrays.length; ++i) {
		let ind: index_array_t = raw.index_arrays[i];
		array_push(indices, ind.values);
		array_push(material_indices, ind.material);
	}

	// Skinning
	// Prepare vertex array for skinning and fill size data
	let vertex_arrays: vertex_array_t[] = raw.vertex_arrays;
	if (raw.skin != null) {
		let va_bone: vertex_array_t = {};
		va_bone.attrib = "bone";
		va_bone.values = null;
		va_bone.data = "short4norm";
		let va_weight: vertex_array_t = {};
		va_weight.attrib = "weight";
		va_weight.values = null;
		va_weight.data = "short4norm";
		array_push(vertex_arrays, va_bone);
		array_push(vertex_arrays, va_weight);
	}

	if (raw.skin != null) {
		let size: i32 = mesh_data_get_vertex_size(vertex_arrays[0].data);
		let vertex_length: i32 = math_floor(vertex_arrays[0].values.length / size);
		let l: i32 = vertex_length * 4;
		let bonea: i16_array_t = i16_array_create(l);
		let weighta: i16_array_t = i16_array_create(l);

		let index: i32 = 0;
		let ai: i32 = 0;
		for (let i: i32 = 0; i < vertex_length; ++i) {
			let bone_count: i32 = raw.skin.bone_count_array[i];
			for (let j: i32 = index; j < index + bone_count; ++j) {
				bonea[ai] = raw.skin.bone_index_array[j];
				weighta[ai] = raw.skin.bone_weight_array[j];
				ai++;
			}
			// Fill unused weights
			for (let j: i32 = bone_count; j < 4; ++j) {
				bonea[ai] = 0;
				weighta[ai] = 0;
				ai++;
			}
			index += bone_count;
		}
		vertex_arrays[vertex_arrays.length - 2].values = bonea;
		vertex_arrays[vertex_arrays.length - 1].values = weighta;
	}

	// Make vertex buffers
	raw._.indices = indices;
	raw._.material_indices = material_indices;
	raw._.structure = mesh_data_get_vertex_struct(raw.vertex_arrays);

	return raw;
}

function mesh_data_get_vertex_struct(vertex_arrays: vertex_array_t[]): vertex_struct_t {
	let structure: vertex_struct_t = g4_vertex_struct_create();
	for (let i: i32 = 0; i < vertex_arrays.length; ++i) {
		g4_vertex_struct_add(structure, vertex_arrays[i].attrib, mesh_data_get_vertex_data(vertex_arrays[i].data));
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

function mesh_data_get(raw: mesh_data_t, vs: vertex_element_t[]): vertex_buffer_t {
	let key: string = "";
	for (let i: i32 = 0; i < vs.length; ++i) {
		let e: vertex_element_t = vs[i];
		key += e.name;
	}
	let vb: vertex_buffer_t = map_get(raw._.vertex_buffer_map, key);
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
		let vstruct: vertex_struct_t = mesh_data_get_vertex_struct(vertex_arrays);
		let size: i32 = mesh_data_get_vertex_size(positions.data);
		vb = g4_vertex_buffer_create(math_floor(positions.values.length / size), vstruct, usage_t.STATIC);
		raw._.vertices = g4_vertex_buffer_lock(vb);
		mesh_data_build_vertices(raw._.vertices, vertex_arrays, 0, has_tex && uvs == null, tex_offset);
		g4_vertex_buffer_unlock(vb);
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
	raw._.vertex_buffer = g4_vertex_buffer_create(math_floor(positions.values.length / size), raw._.structure, usage_t.STATIC);
	raw._.vertices = g4_vertex_buffer_lock(raw._.vertex_buffer);
	mesh_data_build_vertices(raw._.vertices, raw.vertex_arrays);
	g4_vertex_buffer_unlock(raw._.vertex_buffer);

	let struct_str: string = "";
	for (let i: i32 = 0; i < raw._.structure.elements.length; ++i) {
		let e: kinc_vertex_elem_t = raw._.structure.elements[i];
		struct_str += e.name;
	}
	map_set(raw._.vertex_buffer_map, struct_str, raw._.vertex_buffer);

	raw._.index_buffers = [];

	for (let i: i32 = 0; i < raw._.indices.length; ++i) {
		let id: u32_array_t = raw._.indices[i];
		if (id.length == 0) {
			continue;
		}
		let index_buffer: index_buffer_t = g4_index_buffer_create(id.length);

		let indices_array: u32_array_t = g4_index_buffer_lock(index_buffer);
		for (let i: i32 = 0; i < indices_array.length; ++i) {
			indices_array[i] = id[i];
		}

		g4_index_buffer_unlock(index_buffer);
		array_push(raw._.index_buffers, index_buffer);
	}

	raw._.ready = true;
}

///if arm_skin
function mesh_data_add_armature(raw: mesh_data_t, armature: armature_t) {
	for (let i: i32 = 0; i < armature.actions.length; ++i) {
		let a: armature_action_t = armature.actions[i];
		mesh_data_add_action(raw, a.bones, a.name);
	}
}

function mesh_data_add_action(raw: mesh_data_t, bones: obj_t[], name: string) {
	if (bones == null) {
		return;
	}
	if (raw._.actions == null) {
		raw._.actions = map_create();
		raw._.mats = map_create();
	}
	if (map_get(raw._.actions, name) != null) {
		return;
	}
	let action_bones: obj_t[] = [];

	// Set bone references
	for (let i: i32 = 0; i < raw.skin.bone_ref_array.length; ++i) {
		let s: string = raw.skin.bone_ref_array[i];
		for (let j: i32 = 0; j < bones.length; ++j) {
			let b: obj_t = bones[j];
			if (b.name == s) {
				array_push(action_bones, b);
			}
		}
	}
	map_set(raw._.actions, name, action_bones);

	let action_mats: mat4_t[] = [];
	for (let i: i32 = 0; i < action_bones.length; ++i) {
		let b: obj_t = action_bones[i];
		array_push(action_mats, mat4_from_f32_array(b.transform));
	}
	map_set(raw._.mats, name, action_mats);
}

function mesh_data_init_skeleton_transforms(raw: mesh_data_t, transforms_inv: f32_array_t[]) {
	raw._.skeleton_transforms_inv = [];
	for (let i: i32 = 0; i < transforms_inv.length; ++i) {
		let t: f32_array_t = transforms_inv[i];
		let mi = mat4_from_f32_array(t);
		array_push(raw._.skeleton_transforms_inv, mi);
	}
}
///end

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
		let buf: vertex_buffer_t = map_get(raw._.vertex_buffer_map, vertex_buffer_keys[i]);
		if (buf != null) {
			g4_vertex_buffer_delete(buf);
		}
	}
	for (let i: i32 = 0; i < raw._.index_buffers.length; ++i) {
		let buf: index_buffer_t = raw._.index_buffers[i];
		g4_index_buffer_delete(buf);
	}
}
