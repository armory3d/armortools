
let _uniforms_mat: mat4_t = mat4_identity();
let _uniforms_mat2: mat4_t = mat4_identity();
let _uniforms_mat3: mat3_t = mat3_identity();
let _uniforms_vec: vec4_t = vec4_create();
let _uniforms_vec2: vec4_t = vec4_create();
let _uniforms_quat: quat_t = quat_create();

let uniforms_tex_links: (o: object_t, md: material_data_t, s: string)=>image_t = null;
let uniforms_mat4_links: (o: object_t, md: material_data_t, s: string)=>mat4_t = null;
let uniforms_vec4_links: (o: object_t, md: material_data_t, s: string)=>vec4_t = null;
let uniforms_vec3_links: (o: object_t, md: material_data_t, s: string)=>vec4_t = null;
let uniforms_vec2_links: (o: object_t, md: material_data_t, s: string)=>vec2_t = null;
let uniforms_f32_links: (o: object_t, md: material_data_t, s: string)=>f32 = null;
let uniforms_f32_array_links: (o: object_t, md: material_data_t, s: string)=>f32_array_t = null;
let uniforms_i32_links: (o: object_t, md: material_data_t, s: string)=>i32 = null;
let uniforms_pos_unpack: f32 = 1.0;
let uniforms_tex_unpack: f32 = 1.0;

let uniforms_light_size_x: f32 = 3.0;
let uniforms_light_size_y: f32 = 0.005;
let uniforms_light_strength: f32 = 1.0;
let uniforms_light_world: mat4_t = mat4_create(
	0.9502051472663879,
	-0.19612328708171844,
	0.24216893315315247,
	0.7578424215316772,
	0.3094223141670227,
	0.6860249638557434,
	-0.658504068851471,
	-2.0724263191223145,
	-0.03698595613241196,
	0.7006464004516602,
	0.7125493884086609,
	1.9363592863082886,
	0,
	0,
	0,
	1
);

function uniforms_set_context_consts(context: shader_context_t, bind_params: string[]) {
	if (context.constants != null) {
		for (let i: i32 = 0; i < context.constants.length; ++i) {
			let c: shader_const_t = context.constants[i];
			uniforms_set_context_const(context._.constants[i], c);
		}
	}

	// Texture context constants
	if (bind_params != null) { // Bind targets
		for (let i: i32 = 0; i < math_floor(bind_params.length / 2); ++i) {
			let pos: i32 = i * 2; // bind params = [texture, sampler_id]
			let rt_id: string = bind_params[pos];
			let sampler_id: string = bind_params[pos + 1];
			let attach_depth: bool = false; // Attach texture depth if "_" is prepended
			let c: string = char_at(rt_id, 0);
			if (c == "_") {
				attach_depth = true;
				rt_id = substring(rt_id, 1, rt_id.length);
			}
			let rt: render_target_t;
			if (attach_depth) {
				rt = map_get(_render_path_depth_to_render_target, rt_id);
			}
			else {
				rt = map_get(render_path_render_targets, rt_id);
			}
			uniforms_bind_render_target(rt, context, sampler_id, attach_depth);
		}
	}

	// Texture links
	if (context.texture_units != null) {
		for (let j: i32 = 0; j < context.texture_units.length; ++j) {
			let tulink: string = context.texture_units[j].link;
			if (tulink == null) {
				continue;
			}

			if (char_at(tulink, 0) == "$") { // Link to embedded data
				g4_set_tex(context._.tex_units[j], map_get(scene_embedded, substring(tulink, 1, tulink.length)));
				g4_set_tex_params(context._.tex_units[j], tex_addressing_t.REPEAT, tex_addressing_t.REPEAT, tex_filter_t.LINEAR, tex_filter_t.LINEAR, mip_map_filter_t.NONE);
			}
			else if (tulink == "_envmap_radiance") {
				let w: world_data_t = scene_world;
				if (w != null) {
					g4_set_tex(context._.tex_units[j], w._.radiance);
					g4_set_tex_params(context._.tex_units[j], tex_addressing_t.REPEAT, tex_addressing_t.REPEAT, tex_filter_t.LINEAR, tex_filter_t.LINEAR, mip_map_filter_t.LINEAR);
				}
			}
			else if (tulink == "_envmap") {
				let w: world_data_t = scene_world;
				if (w != null) {
					g4_set_tex(context._.tex_units[j], w._.envmap);
					g4_set_tex_params(context._.tex_units[j], tex_addressing_t.REPEAT, tex_addressing_t.REPEAT, tex_filter_t.LINEAR, tex_filter_t.LINEAR, mip_map_filter_t.NONE);
				}
			}
		}
	}
}

function uniforms_set_obj_consts(context: shader_context_t, object: object_t) {
	if (context.constants != null) {
		for (let i: i32 = 0; i < context.constants.length; ++i) {
			let c: shader_const_t = context.constants[i];
			uniforms_set_obj_const(object, context._.constants[i], c);
		}
	}

	// Texture object constants
	// External
	if (uniforms_tex_links != null) {
		if (context.texture_units != null) {
			for (let j: i32 = 0; j < context.texture_units.length; ++j) {
				let tu: tex_unit_t = context.texture_units[j];
				if (tu.link == null) {
					continue;
				}

				let image: image_t = uniforms_tex_links(object, current_material(object), tu.link);
				if (image != null) {
					ends_with(tu.link, "_depth") ?
						g4_set_tex_depth(context._.tex_units[j], image) :
						g4_set_tex(context._.tex_units[j], image);
					g4_set_tex_params(context._.tex_units[j], tex_addressing_t.REPEAT, tex_addressing_t.REPEAT, tex_filter_t.LINEAR, tex_filter_t.LINEAR, mip_map_filter_t.NONE);
				}
			}
		}
	}
}

function uniforms_bind_render_target(rt: render_target_t, context: shader_context_t, sampler_id: string, attach_depth: bool) {
	if (rt == null) {
		return;
	}

	let tus: tex_unit_t[] = context.texture_units;

	for (let j: i32 = 0; j < tus.length; ++j) { // Set texture
		if (sampler_id == tus[j].name) {

			if (attach_depth) {
				g4_set_tex_depth(context._.tex_units[j], rt._image); // sampler2D
			}
			else {
				g4_set_tex(context._.tex_units[j], rt._image); // sampler2D
			}

			if (rt.mipmaps) {
				g4_set_tex_params(context._.tex_units[j], tex_addressing_t.CLAMP, tex_addressing_t.CLAMP, tex_filter_t.LINEAR, tex_filter_t.LINEAR, mip_map_filter_t.LINEAR);
				continue;
			}

			if (starts_with(rt.name, "bloom")) {
				// Use bilinear filter for bloom mips to get correct blur
				g4_set_tex_params(context._.tex_units[j], tex_addressing_t.CLAMP, tex_addressing_t.CLAMP, tex_filter_t.LINEAR, tex_filter_t.LINEAR, mip_map_filter_t.NONE);
				continue;
			}

			if (attach_depth) {
				g4_set_tex_params(context._.tex_units[j], tex_addressing_t.CLAMP, tex_addressing_t.CLAMP, tex_filter_t.POINT, tex_filter_t.POINT, mip_map_filter_t.NONE);
				continue;
			}

			// No filtering when sampling render targets
			let oc: _shader_override_t = context._.override_context;
			let allow_params: bool = oc == null || oc.shared_sampler == null || oc.shared_sampler == sampler_id;
			if (allow_params) {
				let addressing: tex_addressing_t = (oc != null && oc.addressing == "repeat") ? tex_addressing_t.REPEAT : tex_addressing_t.CLAMP;
				let filter: tex_filter_t = (oc != null && oc.filter == "point") ? tex_filter_t.POINT : tex_filter_t.LINEAR;
				g4_set_tex_params(context._.tex_units[j], addressing, addressing, filter, filter, mip_map_filter_t.NONE);
			}
		}
	}
}

function uniforms_set_context_const(location: kinc_const_loc_t, c: shader_const_t): bool {
	if (c.link == null) {
		return true;
	}

	let camera: camera_object_t = scene_camera;

	if (c.type == "mat4") {
		let m: mat4_t = mat4_nan();
		if (c.link == "_view_matrix") {
			m = camera.v;
		}
		else if (c.link == "_proj_matrix") {
			m = camera.p;
		}
		else if (c.link == "_inv_proj_matrix") {
			m = mat4_inv(camera.p);
		}
		else if (c.link == "_view_proj_matrix") {
			m = camera.vp;
		}
		else if (c.link == "_inv_view_proj_matrix") {
			m = mat4_mult_mat(camera.v, camera.p);
			m = mat4_inv(m);
		}
		else if (c.link == "_skydome_matrix") {
			let tr: transform_t = camera.base.transform;
			let v: vec4_t = vec4_create(transform_world_x(tr), transform_world_y(tr), transform_world_z(tr));
			let bounds: f32 = camera.data.far_plane * 0.9;
			let v2: vec4_t = vec4_create(bounds, bounds, bounds);
			m = mat4_compose(v, _uniforms_quat, v2);
			m = mat4_mult_mat(m, camera.v);
			m = mat4_mult_mat(m, camera.p);
		}
		else { // Unknown uniform
			return false;
		}

		g4_set_mat(location, m);
		return true;
	}
	else if (c.type == "vec4") {
		let v: vec4_t = vec4_nan();
		// if (c.link == "") {}
		// else {
			return false;
		// }

		if (!vec4_isnan(v)) {
			g4_set_float4(location, v.x, v.y, v.z, v.w);
		}
		else {
			g4_set_float4(location, 0, 0, 0, 0);
		}
		return true;
	}
	else if (c.type == "vec3") {
		let v: vec4_t = vec4_nan();

		if (c.link == "_light_area0") {
			let sx: f32 = uniforms_light_size_x * uniforms_light_strength * 0.25;
			let sy: f32 = uniforms_light_size_y * uniforms_light_strength * 0.25;
			v = vec4_create(-sx, sy, 0.0);
			v = vec4_apply_mat(v, uniforms_light_world);
		}
		else if (c.link == "_light_area1") {
			let sx: f32 = uniforms_light_size_x * uniforms_light_strength * 0.25;
			let sy: f32 = uniforms_light_size_y * uniforms_light_strength * 0.25;
			v = vec4_create(sx, sy, 0.0);
			v = vec4_apply_mat(v, uniforms_light_world);
		}
		else if (c.link == "_light_area2") {
			let sx: f32 = uniforms_light_size_x * uniforms_light_strength * 0.25;
			let sy: f32 = uniforms_light_size_y * uniforms_light_strength * 0.25;
			v = vec4_create(sx, -sy, 0.0);
			v = vec4_apply_mat(v, uniforms_light_world);
		}
		else if (c.link == "_light_area3") {
			let sx: f32 = uniforms_light_size_x * uniforms_light_strength * 0.25;
			let sy: f32 = uniforms_light_size_y * uniforms_light_strength * 0.25;
			v = vec4_create(-sx, -sy, 0.0);
			v = vec4_apply_mat(v, uniforms_light_world);
		}
		else if (c.link == "_camera_pos") {
			v = vec4_create(transform_world_x(camera.base.transform), transform_world_y(camera.base.transform), transform_world_z(camera.base.transform));
		}
		else if (c.link == "_camera_look") {
			v = vec4_norm(camera_object_look_world(camera));
		}
		else {
			return false;
		}

		if (!vec4_isnan(v)) {
			g4_set_float3(location, v.x, v.y, v.z);
		}
		else {
			g4_set_float3(location, 0.0, 0.0, 0.0);
		}
		return true;
	}
	else if (c.type == "vec2") {
		let v: vec4_t = vec4_nan();

		if (c.link == "_vec2x") {
			v.x = 1.0;
			v.y = 0.0;
		}
		else if (c.link == "_vec2x_inv") {
			v.x = 1.0 / render_path_current_w;
			v.y = 0.0;
		}
		else if (c.link == "_vec2x2") {
			v.x = 2.0;
			v.y = 0.0;
		}
		else if (c.link == "_vec2x2_inv") {
			v.x = 2.0 / render_path_current_w;
			v.y = 0.0;
		}
		else if (c.link == "_vec2y") {
			v.x = 0.0;
			v.y = 1.0;
		}
		else if (c.link == "_vec2y_inv") {
			v.x = 0.0;
			v.y = 1.0 / render_path_current_h;
		}
		else if (c.link == "_vec2y2") {
			v.x = 0.0;
			v.y = 2.0;
		}
		else if (c.link == "_vec2y2_inv") {
			v.x = 0.0;
			v.y = 2.0 / render_path_current_h;
		}
		else if (c.link == "_vec2y3") {
			v.x = 0.0;
			v.y = 3.0;
		}
		else if (c.link == "_vec2y3_inv") {
			v.x = 0.0;
			v.y = 3.0 / render_path_current_h;
		}
		else if (c.link == "_screen_size") {
			v.x = render_path_current_w;
			v.y = render_path_current_h;
		}
		else if (c.link == "_screen_size_inv") {
			v.x = 1.0 / render_path_current_w;
			v.y = 1.0 / render_path_current_h;
		}
		else if (c.link == "_camera_plane_proj") {
			let znear: f32 = camera.data.near_plane;
			let zfar: f32 = camera.data.far_plane;
			v.x = zfar / (zfar - znear);
			v.y = (-zfar * znear) / (zfar - znear);
		}
		else {
			return false;
		}

		if (!vec4_isnan(v)) {
			g4_set_float2(location, v.x, v.y);
		}
		else {
			g4_set_float2(location, 0.0, 0.0);
		}
		return true;
	}
	else if (c.type == "float") {
		let f: f32 = 0.0;

		if (c.link == "_time") {
			f = time_time();
		}
		else if (c.link == "_aspect_ratio_window") {
			f = app_w() / app_h();
		}
		else {
			return false;
		}

		g4_set_float(location, f);
		return true;
	}
	else if (c.type == "floats") {
		let fa: f32_array_t = null;

		if (c.link == "_envmap_irradiance") {
			fa = scene_world == null ? world_data_get_empty_irradiance() : scene_world._.irradiance;
		}

		if (fa != null) {
			g4_set_floats(location, fa);
			return true;
		}
	}
	else if (c.type == "int") {
		let i: i32 = 0;

		if (c.link == "_envmap_num_mipmaps") {
			let w: world_data_t = scene_world;
			i = w != null ? w.radiance_mipmaps + 1 - 2 : 1; // Include basecolor and exclude 2 scaled mips
		}
		else {
			return false;
		}

		g4_set_int(location, i);
		return true;
	}
	return false;
}

function uniforms_set_obj_const(obj: object_t, loc: kinc_const_loc_t, c: shader_const_t) {
	if (c.link == null) {
		return;
	}

	let camera: camera_object_t = scene_camera;
	if (c.type == "mat4") {
		let m: mat4_t = mat4_nan();

		if (c.link == "_world_matrix") {
			m = obj.transform.world_unpack;
		}
		else if (c.link == "_inv_world_matrix") {
			m = mat4_inv(obj.transform.world_unpack);
		}
		else if (c.link == "_world_view_proj_matrix") {
			m = mat4_mult_mat(obj.transform.world_unpack, camera.v);
			m = mat4_mult_mat(m, camera.p);
		}
		else if (c.link == "_world_wiew_matrix") {
			m = mat4_mult_mat(obj.transform.world_unpack, camera.v);
		}
		else if (c.link == "_prev_world_view_proj_matrix") {
			let mo: mesh_object_t = obj.ext;
			m = mat4_mult_mat(mo.prev_matrix, camera.prev_v);
			// m = mat4_mult_mat(m, camera.prev_p);
			m = mat4_mult_mat(m, camera.p);
		}
		else if (uniforms_mat4_links != null) {
			m = uniforms_mat4_links(obj, current_material(obj), c.link);
		}

		if (mat4_isnan(m)) {
			return;
		}
		g4_set_mat(loc, m);
	}
	else if (c.type == "mat3") {
		let m: mat3_t = mat3_nan();

		if (c.link == "_normal_matrix") {
			let m4: mat4_t = mat4_inv(obj.transform.world);
			m4 = mat4_transpose3x3(m4);
			m = mat3_set_from4(m4);
		}
		else if (c.link == "_view_matrix3") {
			m = mat3_set_from4(camera.v);
		}

		if (mat3_isnan(m)) {
			return;
		}
		g4_set_mat3(loc, m);
	}
	else if (c.type == "vec4") {
		let v: vec4_t = vec4_nan();

		if (uniforms_vec4_links != null) {
			v = uniforms_vec4_links(obj, current_material(obj), c.link);
		}

		if (vec4_isnan(v)) {
			return;
		}
		g4_set_float4(loc, v.x, v.y, v.z, v.w);
	}
	else if (c.type == "vec3") {
		let v: vec4_t = vec4_nan();

		if (c.link == "_dim") { // Model space
			let d: vec4_t = obj.transform.dim;
			let s: vec4_t = obj.transform.scale;
			v = vec4_create((d.x / s.x), (d.y / s.y), (d.z / s.z));
		}
		else if (c.link == "_half_dim") { // Model space
			let d: vec4_t = obj.transform.dim;
			let s: vec4_t = obj.transform.scale;
			v = vec4_create((d.x / s.x) / 2, (d.y / s.y) / 2, (d.z / s.z) / 2);
		}
		else if (uniforms_vec3_links != null) {
			v = uniforms_vec3_links(obj, current_material(obj), c.link);
		}

		if (vec4_isnan(v)) {
			return;
		}
		g4_set_float3(loc, v.x, v.y, v.z);
	}
	else if (c.type == "vec2") {
		let v: vec2_t = vec2_nan();

		if (uniforms_vec2_links != null) {
			v = uniforms_vec2_links(obj, current_material(obj), c.link);
		}

		if (vec2_isnan(v)) {
			return;
		}
		g4_set_float2(loc, v.x, v.y);
	}
	else if (c.type == "float") {
		let f: f32 = f32_nan();

		if (c.link == "_object_info_index") {
			f = obj.uid;
		}
		else if (c.link == "_object_info_material_index") {
			f = current_material(obj)._.uid;
		}
		else if (c.link == "_object_info_random") {
			f = obj.urandom;
		}
		else if (c.link == "_pos_unpack") {
			f = uniforms_pos_unpack;
		}
		else if (c.link == "_tex_unpack") {
			f = uniforms_tex_unpack;
		}
		else if (uniforms_f32_links != null) {
			f = uniforms_f32_links(obj, current_material(obj), c.link);
		}

		if (f32_isnan(f)) {
			return;
		}
		g4_set_float(loc, f);
	}
	else if (c.type == "floats") {
		let fa: f32_array_t = null;

		if (c.link == "_skin_bones") {
			///if arm_skin
			if (obj.animation != null) {
				fa = obj.animation.ext.skin_buffer;
			}
			///end
		}
		else if (uniforms_f32_array_links != null) {
			fa = uniforms_f32_array_links(obj, current_material(obj), c.link);
		}

		if (fa == null) {
			return;
		}
		g4_set_floats(loc, fa);
	}
	else if (c.type == "int") {
		let i: i32 = INT_MAX;

		if (c.link == "_uid") {
			i = obj.uid;
		}
		else if (uniforms_i32_links != null) {
			i = uniforms_i32_links(obj, current_material(obj), c.link);
		}

		if (i == INT_MAX) {
			return;
		}

		g4_set_int(loc, i);
	}
}

function uniforms_set_material_consts(context: shader_context_t, material_context: material_context_t) {
	if (material_context.bind_constants != null) {
		for (let i: i32 = 0; i < material_context.bind_constants.length; ++i) {
			let matc: bind_const_t = material_context.bind_constants[i];
			let pos: i32 = -1;
			for (let i: i32 = 0; i < context.constants.length; ++i) {
				let name: string = context.constants[i].name;
				if (name == matc.name) {
					pos = i;
					break;
				}
			}
			if (pos == -1) {
				continue;
			}
			let c: shader_const_t = context.constants[pos];

			uniforms_set_material_const(context._.constants[pos], c, matc);
		}
	}

	if (material_context._.textures != null) {
		for (let i: i32 = 0; i < material_context._.textures.length; ++i) {
			let mname: string = material_context.bind_textures[i].name;

			for (let j: i32 = 0; j < context._.tex_units.length; ++j) {
				let sname: string = context.texture_units[j].name;
				if (mname == sname) {
					g4_set_tex(context._.tex_units[j], material_context._.textures[i]);
					// After texture sampler have been assigned, set texture parameters
					material_context_set_tex_params(material_context, i, context, j);
					break;
				}
			}
		}
	}
}

function current_material(object: object_t): material_data_t {
	if (object != null && object.ext != null) {
		let mo: mesh_object_t = object.ext;
		if (mo.materials != null) {
			return mo.materials[mo.material_index];
		}
	}
	return null;
}

function uniforms_set_material_const(location: kinc_const_loc_t, shader_const: shader_const_t, material_const: bind_const_t) {
	if (shader_const.type == "vec4") {
		g4_set_float4(location, material_const.vec[0], material_const.vec[1], material_const.vec[2], material_const.vec[3]);
	}
	else if (shader_const.type == "vec3") {
		g4_set_float3(location, material_const.vec[0], material_const.vec[1], material_const.vec[2]);
	}
	else if (shader_const.type == "vec2") {
		g4_set_float2(location, material_const.vec[0], material_const.vec[1]);
	}
	else if (shader_const.type == "float") {
		g4_set_float(location,  material_const.vec[0]);
	}
	else if (shader_const.type == "bool") {
		g4_set_bool(location, material_const.vec[0] > 0.0);
	}
	else if (shader_const.type == "int") {
		g4_set_int(location, math_floor(material_const.vec[0]));
	}
}
