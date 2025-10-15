
let _uniforms_mat: mat4_t  = mat4_identity();
let _uniforms_mat2: mat4_t = mat4_identity();
let _uniforms_mat3: mat3_t = mat3_identity();
let _uniforms_vec: vec4_t  = vec4_create();
let _uniforms_vec2: vec4_t = vec4_create();
let _uniforms_quat: quat_t = quat_create();

let uniforms_tex_links: (o: object_t, md: material_data_t, s: string) => gpu_texture_t  = null;
let uniforms_mat4_links: (o: object_t, md: material_data_t, s: string) => mat4_t  = null;
let uniforms_vec4_links: (o: object_t, md: material_data_t, s: string) => vec4_t  = null;
let uniforms_vec3_links: (o: object_t, md: material_data_t, s: string) => vec4_t  = null;
let uniforms_vec2_links: (o: object_t, md: material_data_t, s: string) => vec2_t  = null;
let uniforms_f32_links: (o: object_t, md: material_data_t, s: string) => f32  = null;
let uniforms_f32_array_links: (o: object_t, md: material_data_t, s: string) => f32_array_t  = null;
let uniforms_i32_links: (o: object_t, md: material_data_t, s: string) => i32 = null;
let uniforms_pos_unpack: f32                                                 = 1.0;
let uniforms_tex_unpack: f32                                                 = 1.0;

function uniforms_set_context_consts(context: shader_context_t, bind_params: string[]) {
	// On shader compile error, _.constants.length will be 0
	if (context.constants != null && context.constants.length == context._.constants.length) {
		for (let i: i32 = 0; i < context.constants.length; ++i) {
			let c: shader_const_t = context.constants[i];
			uniforms_set_context_const(context._.constants[i], c);
		}
	}

	// Texture context constants
	if (bind_params != null) { // Bind targets
		for (let i: i32 = 0; i < math_floor(bind_params.length / 2); ++i) {
			let pos: i32            = i * 2; // bind params = [texture, sampler_id]
			let rt_id: string       = bind_params[pos];
			let sampler_id: string  = bind_params[pos + 1];
			let rt: render_target_t = map_get(render_path_render_targets, rt_id);
			uniforms_bind_render_target(rt, context, sampler_id);
		}
	}

	// Texture links
	if (context.texture_units != null && context.texture_units.length == context._.tex_units.length) {
		for (let j: i32 = 0; j < context.texture_units.length; ++j) {
			let tulink: string = context.texture_units[j].link;
			if (tulink == null) {
				continue;
			}

			if (char_at(tulink, 0) == "$") { // Link to embedded data
				gpu_set_texture(context._.tex_units[j], map_get(scene_embedded, substring(tulink, 1, tulink.length)));
			}
			else if (tulink == "_envmap_radiance") {
				let w: world_data_t = scene_world;
				if (w != null) {
					gpu_set_texture(context._.tex_units[j], w._.radiance);
				}
			}
			else if (tulink == "_envmap_radiance0") {
				let w: world_data_t = scene_world;
				if (w != null) {
					gpu_set_texture(context._.tex_units[j], w._.radiance_mipmaps[0]);
				}
			}
			else if (tulink == "_envmap_radiance1") {
				let w: world_data_t = scene_world;
				if (w != null) {
					gpu_set_texture(context._.tex_units[j], w._.radiance_mipmaps[1]);
				}
			}
			else if (tulink == "_envmap_radiance2") {
				let w: world_data_t = scene_world;
				if (w != null) {
					gpu_set_texture(context._.tex_units[j], w._.radiance_mipmaps[2]);
				}
			}
			else if (tulink == "_envmap_radiance3") {
				let w: world_data_t = scene_world;
				if (w != null) {
					gpu_set_texture(context._.tex_units[j], w._.radiance_mipmaps[3]);
				}
			}
			else if (tulink == "_envmap_radiance4") {
				let w: world_data_t = scene_world;
				if (w != null) {
					gpu_set_texture(context._.tex_units[j], w._.radiance_mipmaps[4]);
				}
			}
			else if (tulink == "_envmap") {
				let w: world_data_t = scene_world;
				if (w != null) {
					gpu_set_texture(context._.tex_units[j], w._.envmap);
				}
			}
		}
	}
}

function uniforms_set_obj_consts(context: shader_context_t, object: object_t) {
	if (context.constants != null && context.constants.length == context._.constants.length) {
		for (let i: i32 = 0; i < context.constants.length; ++i) {
			let c: shader_const_t = context.constants[i];
			uniforms_set_obj_const(object, context._.constants[i], c);
		}
	}

	// Texture object constants
	// External
	if (uniforms_tex_links != null && context.texture_units != null && context.texture_units.length == context._.tex_units.length) {
		for (let j: i32 = 0; j < context.texture_units.length; ++j) {
			let tu: tex_unit_t = context.texture_units[j];
			if (tu.link == null) {
				continue;
			}

			let image: gpu_texture_t = uniforms_tex_links(object, current_material(object), tu.link);
			if (image != null) {
				gpu_set_texture(context._.tex_units[j], image);
			}
		}
	}
}

function uniforms_bind_render_target(rt: render_target_t, context: shader_context_t, sampler_id: string) {
	if (rt == null) {
		return;
	}

	if (context.texture_units != null && context.texture_units.length == context._.tex_units.length) {
		for (let j: i32 = 0; j < context.texture_units.length; ++j) { // Set texture
			if (sampler_id == context.texture_units[j].name) {
				gpu_set_texture(context._.tex_units[j], rt._image);
			}
		}
	}
}

function uniforms_set_context_const(location: i32, c: shader_const_t): bool {
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
			let v: vec4_t       = vec4_create(transform_world_x(tr), transform_world_y(tr), transform_world_z(tr));
			let bounds: f32     = camera.data.far_plane * 0.9;
			let v2: vec4_t      = vec4_create(bounds, bounds, bounds);
			m                   = mat4_compose(v, _uniforms_quat, v2);
			m                   = mat4_mult_mat(m, camera.v);
			m                   = mat4_mult_mat(m, camera.p);
		}
		else { // Unknown uniform
			return false;
		}

		gpu_set_matrix4(location, m);
		return true;
	}
	else if (c.type == "vec4") {
		let v: vec4_t = vec4_nan();
		if (c.link == "_envmap_irradiance0") {
			let fa: f32_array_t = scene_world == null ? world_data_get_empty_irradiance() : scene_world._.irradiance;
			v.x                 = fa[0];
			v.y                 = fa[1];
			v.z                 = fa[2];
			v.w                 = fa[3];
		}
		else if (c.link == "_envmap_irradiance1") {
			let fa: f32_array_t = scene_world == null ? world_data_get_empty_irradiance() : scene_world._.irradiance;
			v.x                 = fa[4];
			v.y                 = fa[5];
			v.z                 = fa[6];
			v.w                 = fa[7];
		}
		else if (c.link == "_envmap_irradiance2") {
			let fa: f32_array_t = scene_world == null ? world_data_get_empty_irradiance() : scene_world._.irradiance;
			v.x                 = fa[8];
			v.y                 = fa[9];
			v.z                 = fa[10];
			v.w                 = fa[11];
		}
		else if (c.link == "_envmap_irradiance3") {
			let fa: f32_array_t = scene_world == null ? world_data_get_empty_irradiance() : scene_world._.irradiance;
			v.x                 = fa[12];
			v.y                 = fa[13];
			v.z                 = fa[14];
			v.w                 = fa[15];
		}
		else if (c.link == "_envmap_irradiance4") {
			let fa: f32_array_t = scene_world == null ? world_data_get_empty_irradiance() : scene_world._.irradiance;
			v.x                 = fa[16];
			v.y                 = fa[17];
			v.z                 = fa[18];
			v.w                 = fa[19];
		}
		else if (c.link == "_envmap_irradiance5") {
			let fa: f32_array_t = scene_world == null ? world_data_get_empty_irradiance() : scene_world._.irradiance;
			v.x                 = fa[20];
			v.y                 = fa[21];
			v.z                 = fa[22];
			v.w                 = fa[23];
		}
		else if (c.link == "_envmap_irradiance6") {
			let fa: f32_array_t = scene_world == null ? world_data_get_empty_irradiance() : scene_world._.irradiance;
			v.x                 = fa[24];
			v.y                 = fa[25];
			v.z                 = fa[26];
			v.w                 = fa[27];
		}
		else {
			return false;
		}

		if (!vec4_isnan(v)) {
			gpu_set_float4(location, v.x, v.y, v.z, v.w);
		}
		else {
			gpu_set_float4(location, 0, 0, 0, 0);
		}
		return true;
	}
	else if (c.type == "vec3") {
		let v: vec4_t = vec4_nan();

		if (c.link == "_camera_pos") {
			v = vec4_create(transform_world_x(camera.base.transform), transform_world_y(camera.base.transform), transform_world_z(camera.base.transform));
		}
		else if (c.link == "_camera_look") {
			v = vec4_norm(camera_object_look_world(camera));
		}
		else {
			return false;
		}

		if (!vec4_isnan(v)) {
			gpu_set_float3(location, v.x, v.y, v.z);
		}
		else {
			gpu_set_float3(location, 0.0, 0.0, 0.0);
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
			let zfar: f32  = camera.data.far_plane;
			v.x            = zfar / (zfar - znear);
			v.y            = (-zfar * znear) / (zfar - znear);
		}
		else if (starts_with(c.link, "_size(")) {
			let tex: string          = substring(c.link, 6, c.link.length - 1);
			let image: gpu_texture_t = uniforms_tex_links(null, null, tex);
			if (image != null) {
				v.x = image.width;
				v.y = image.height;
			}
			else if (_render_path_bind_params != null) {
				for (let i: i32 = 0; i < math_floor(_render_path_bind_params.length / 2); ++i) {
					let pos: i32           = i * 2; // bind params = [texture, sampler_id]
					let sampler_id: string = _render_path_bind_params[pos + 1];
					if (sampler_id == tex) {
						let rt_id: string       = _render_path_bind_params[pos];
						let rt: render_target_t = map_get(render_path_render_targets, rt_id);
						v.x                     = rt.width;
						v.y                     = rt.height;
					}
				}
			}
		}
		else {
			return false;
		}

		if (!vec4_isnan(v)) {
			gpu_set_float2(location, v.x, v.y);
		}
		else {
			gpu_set_float2(location, 0.0, 0.0);
		}
		return true;
	}
	else if (c.type == "float") {
		let f: f32 = 0.0;

		if (c.link == "_time") {
			f = sys_time();
		}
		else if (c.link == "_aspect_ratio_window") {
			f = sys_w() / sys_h();
		}
		else {
			return false;
		}

		gpu_set_float(location, f);
		return true;
	}
	else if (c.type == "floats") {
		let fa: f32_array_t = null;

		if (c.link == "_envmap_irradiance") {
			fa = scene_world == null ? world_data_get_empty_irradiance() : scene_world._.irradiance;
		}

		if (fa != null) {
			gpu_set_floats(location, fa);
			return true;
		}
	}
	else if (c.type == "int") {
		let i: i32 = 0;

		if (c.link == "_envmap_num_mipmaps") {
			let w: world_data_t = scene_world;
			i                   = w != null ? w.radiance_mipmaps + 1 - 2 : 1; // Include basecolor and exclude 2 scaled mips
		}
		else {
			return false;
		}

		gpu_set_int(location, i);
		return true;
	}
	return false;
}

function uniforms_set_obj_const(obj: object_t, loc: i32, c: shader_const_t) {
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
		else if (uniforms_mat4_links != null) {
			m = uniforms_mat4_links(obj, current_material(obj), c.link);
		}

		if (mat4_isnan(m)) {
			return;
		}
		gpu_set_matrix4(loc, m);
	}
	else if (c.type == "mat3") {
		let m: mat3_t = mat3_nan();

		if (c.link == "_normal_matrix") {
			let m4: mat4_t = mat4_inv(obj.transform.world);
			m4             = mat4_transpose3x3(m4);
			m              = mat3_set_from4(m4);
		}
		else if (c.link == "_view_matrix3") {
			m = mat3_set_from4(camera.v);
		}

		if (mat3_isnan(m)) {
			return;
		}
		gpu_set_matrix3(loc, m);
	}
	else if (c.type == "vec4") {
		let v: vec4_t = vec4_nan();

		if (uniforms_vec4_links != null) {
			v = uniforms_vec4_links(obj, current_material(obj), c.link);
		}

		if (vec4_isnan(v)) {
			return;
		}
		gpu_set_float4(loc, v.x, v.y, v.z, v.w);
	}
	else if (c.type == "vec3") {
		let v: vec4_t = vec4_nan();

		if (c.link == "_dim") { // Model space
			let d: vec4_t = obj.transform.dim;
			let s: vec4_t = obj.transform.scale;
			v             = vec4_create((d.x / s.x), (d.y / s.y), (d.z / s.z));
		}
		else if (c.link == "_half_dim") { // Model space
			let d: vec4_t = obj.transform.dim;
			let s: vec4_t = obj.transform.scale;
			v             = vec4_create((d.x / s.x) / 2, (d.y / s.y) / 2, (d.z / s.z) / 2);
		}
		else if (uniforms_vec3_links != null) {
			v = uniforms_vec3_links(obj, current_material(obj), c.link);
		}

		if (vec4_isnan(v)) {
			return;
		}
		gpu_set_float3(loc, v.x, v.y, v.z);
	}
	else if (c.type == "vec2") {
		let v: vec2_t = vec2_nan();

		if (uniforms_vec2_links != null) {
			v = uniforms_vec2_links(obj, current_material(obj), c.link);
		}

		if (vec2_isnan(v)) {
			return;
		}
		gpu_set_float2(loc, v.x, v.y);
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
		gpu_set_float(loc, f);
	}
	else if (c.type == "floats") {
		let fa: f32_array_t = null;

		if (uniforms_f32_array_links != null) {
			fa = uniforms_f32_array_links(obj, current_material(obj), c.link);
		}

		if (fa == null) {
			return;
		}
		gpu_set_floats(loc, fa);
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

		gpu_set_int(loc, i);
	}
}

function uniforms_set_material_consts(context: shader_context_t, material_context: material_context_t) {
	if (material_context.bind_constants != null) {
		for (let i: i32 = 0; i < material_context.bind_constants.length; ++i) {
			let matc: bind_const_t = material_context.bind_constants[i];
			let pos: i32           = -1;
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
					gpu_set_texture(context._.tex_units[j], material_context._.textures[i]);
					break;
				}
			}
		}
	}
}

function current_material(object: object_t): material_data_t {
	if (object != null && object.ext != null) {
		let mo: mesh_object_t = object.ext;
		return mo.material;
	}
	return null;
}

function uniforms_set_material_const(location: i32, shader_const: shader_const_t, material_const: bind_const_t) {
	if (shader_const.type == "vec4") {
		gpu_set_float4(location, material_const.vec[0], material_const.vec[1], material_const.vec[2], material_const.vec[3]);
	}
	else if (shader_const.type == "vec3") {
		gpu_set_float3(location, material_const.vec[0], material_const.vec[1], material_const.vec[2]);
	}
	else if (shader_const.type == "vec2") {
		gpu_set_float2(location, material_const.vec[0], material_const.vec[1]);
	}
	else if (shader_const.type == "float") {
		gpu_set_float(location, material_const.vec[0]);
	}
	else if (shader_const.type == "bool") {
		gpu_set_bool(location, material_const.vec[0] > 0.0);
	}
	else if (shader_const.type == "int") {
		gpu_set_int(location, math_floor(material_const.vec[0]));
	}
}
