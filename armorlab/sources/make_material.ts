
let make_material_default_scon: shader_context_t = null;
let make_material_default_mcon: material_context_t = null;
let make_material_height_used: bool = false;

type parse_node_preview_result_t = {
	scon: shader_context_t;
	mcon: material_context_t;
};

function make_material_parse_mesh_material() {
	let m: material_data_t = project_material_data;

	for (let i: i32 = 0; i < m._.shader.contexts.length; ++i) {
		let c: shader_context_t = m._.shader.contexts[i];
		if (c.name == "mesh") {
			array_remove(m._.shader.contexts, c);
			array_remove(m._.shader._.contexts, c);
			make_material_delete_context(c);
			break;
		}
	}

	let mm: material_t = {
		name: "Material",
		canvas: null
	};

	let con: node_shader_context_t = make_mesh_run(mm);
	let scon: shader_context_t = shader_context_create(con.data);
	let override_context: _shader_override_t = {};
	// if (con.frag.shared_samplers.length > 0) {
		// let sampler: string = con.frag.shared_samplers[0];
		// override_context.shared_sampler = substring(sampler, string_last_index_of(sampler, " ") + 1, sampler.length);
	// }
	if (!context_raw.texture_filter) {
		override_context.filter = "point";
	}
	override_context.addressing = "repeat";
	scon._.override_context = override_context;
	array_push(m._.shader.contexts, scon);
	array_push(m._.shader._.contexts, scon);

	context_raw.ddirty = 2;

	///if arm_voxels
	make_material_make_voxel(m);
	///end

	///if (arm_direct3d12 || arm_vulkan)
	render_path_raytrace_dirty = 1;
	///end
}

///if arm_voxels
function make_material_make_voxel(m: material_data_t) {
	let rebuild: bool = true; // height_used;
	if (config_raw.rp_gi != false && rebuild) {
		let scon: shader_context_t = null;
		for (let i: i32 = 0; i < m._.shader._.contexts.length; ++i) {
			let c: shader_context_t = m._.shader._.contexts[i];
			if (c.name == "voxel") {
				scon = c;
				break;
			}
		}
		if (scon != null) make_voxel_run(scon);
	}
}
///end

function make_material_parse_paint_material() {
	let m: material_data_t = project_material_data;
	let scon: shader_context_t = null;
	let mcon: material_context_t = null;
	for (let i: i32 = 0; i < m._.shader.contexts.length; ++i) {
		let c: shader_context_t = m._.shader.contexts[i];
		if (c.name == "paint") {
			array_remove(m._.shader.contexts, c);
			array_remove(m._.shader._.contexts, c);
			if (c != make_material_default_scon) {
				make_material_delete_context(c);
			}
			break;
		}
	}
	for (let i: i32 = 0; i < m.contexts.length; ++i) {
		let c: material_context_t = m.contexts[i];
		if (c.name == "paint") {
			array_remove(m.contexts, c);
			array_remove(m._.contexts, c);
			break;
		}
	}

	let sdata: material_t = { name: "Material", canvas: null };
	let mcon2: material_context_t = { name: "paint", bind_textures: [] };
	let con: node_shader_context_t = make_paint_run(sdata, mcon2);

	let compile_error: bool = false;
	let scon2: shader_context_t;
	let _scon: shader_context_t = shader_context_create(con.data);
	if (_scon == null) {
		compile_error = true;
	}
	scon2 = _scon;

	if (compile_error) {
		return;
	}

	let override_context: _shader_override_t = {};
	override_context.addressing = "repeat";
	scon2._.override_context = override_context;
	let mcon3: material_context_t = material_context_create(mcon2);

	array_push(m._.shader.contexts, scon2);
	array_push(m._.shader._.contexts, scon2);
	array_push(m.contexts, mcon3);
	array_push(m._.contexts, mcon3);

	if (make_material_default_scon == null) {
		make_material_default_scon = scon2;
	}
	if (make_material_default_mcon == null) {
		make_material_default_mcon = mcon3;
	}
}

function make_material_get_displace_strength(): f32 {
	let sc: vec4_t = context_main_object().base.transform.scale;
	return config_raw.displace_strength * 0.02 * sc.x;
}

function make_material_voxelgi_half_extents(): string {
	let ext: i32 = context_raw.vxao_ext;
	return "const vec3 voxelgi_half_extents = vec3(" + ext + ", " + ext + ", " + ext + ");";
}

function make_material_delete_context(c: shader_context_t) {
	app_notify_on_next_frame(function (c: shader_context_t) { // Ensure pipeline is no longer in use
		shader_context_delete(c);
	}, c);
}

function make_material_parse_brush() {
	// parser_logic_parse(context_raw.brush.canvas);
}

function make_material_parse_mesh_preview_material(md: material_data_t = null) {
}

function make_material_parse_node_preview_material(node: ui_node_t, group: ui_node_canvas_t = null, parents: ui_node_t[] = null): parse_node_preview_result_t {
	return null;
}

function make_material_parse_particle_material() {
}
