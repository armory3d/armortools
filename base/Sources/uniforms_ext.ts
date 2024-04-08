
let uniforms_ext_vec: vec4_t = vec4_create();
let uniforms_ext_ortho_p: mat4_t = mat4_ortho(-0.5, 0.5, -0.5, 0.5, -0.5, 0.5);

function uniforms_ext_init() {
	uniforms_i32_links = uniforms_ext_i32_link;
	uniforms_f32_links = uniforms_ext_f32_link;
	uniforms_vec2_links = uniforms_ext_vec2_link;
	uniforms_vec3_links = uniforms_ext_vec3_link;
	uniforms_vec4_links = uniforms_ext_vec4_link;
	uniforms_mat4_links = uniforms_ext_mat4_link;
	uniforms_tex_links = uniforms_ext_tex_link;
}

function uniforms_ext_i32_link(object: object_t, mat: material_data_t, link: string): i32 {
	if (link == "_bloomCurrentMip") {
		return render_path_base_bloom_current_mip;
	}
	return 0;
}

function uniforms_ext_f32_link(object: object_t, mat: material_data_t, link: string): f32 {
	if (link == "_brushRadius") {
		///if (is_paint || is_sculpt)
		let decal: bool = context_raw.tool == workspace_tool_t.DECAL || context_raw.tool == workspace_tool_t.TEXT;
		let decal_mask: bool = decal && operator_shortcut(map_get(config_keymap, "decal_mask") + "+" + map_get(config_keymap, "action_paint"), shortcut_type_t.DOWN);
		let brush_decal_mask_radius: f32 = context_raw.brush_decal_mask_radius;
		if (config_raw.brush_3d) {
			brush_decal_mask_radius *= context_raw.paint2d ? 0.55 * ui_view2d_pan_scale : 2.0;
		}
		let radius: f32 = decal_mask ? brush_decal_mask_radius : context_raw.brush_radius;
		let val: f32 = (radius * context_raw.brush_nodes_radius) / 15.0;
		if (config_raw.pressure_radius && pen_down()) {
			val *= pen_pressure * config_raw.pressure_sensitivity;
		}
		let scale2d: f32 = (900 / base_h()) * config_raw.window_scale;

		if (config_raw.brush_3d && !decal) {
			val *= context_raw.paint2d ? 0.55 * scale2d * ui_view2d_pan_scale : 2;
		}
		else {
			val *= scale2d; // Projection ratio
		}
		///end

		///if is_lab
		let radius: f32 = context_raw.brush_radius;
		let val: f32 = radius / 15.0;
		if (config_raw.pressure_radius && pen_down()) {
			val *= pen_pressure * config_raw.pressure_sensitivity;
		}
		val *= 2;
		///end

		return val;
	}
	else if (link == "_vignetteStrength") {
		return config_raw.rp_vignette;
	}
	else if (link == "_grainStrength") {
		return config_raw.rp_grain;
	}
	else if (link == "_coneOffset") {
		return context_raw.vxao_offset;
	}
	else if (link == "_coneAperture") {
		return context_raw.vxao_aperture;
	}
	else if (link == "_bloomSampleScale") {
		return render_path_base_bloom_sample_scale;
	}

	///if (is_paint || is_sculpt)
	else if (link == "_brushScaleX") {
		return 1 / context_raw.brush_scale_x;
	}
	else if (link == "_brushOpacity") {
		let val: f32 = context_raw.brush_opacity * context_raw.brush_nodes_opacity;
		if (config_raw.pressure_opacity && pen_down()) {
			val *= pen_pressure * config_raw.pressure_sensitivity;
		}
		return val;
	}
	else if (link == "_brushHardness") {
		let decal_mask: bool = operator_shortcut(map_get(config_keymap, "decal_mask") + "+" + map_get(config_keymap, "action_paint"), shortcut_type_t.DOWN);
		if (context_raw.tool != workspace_tool_t.BRUSH && context_raw.tool != workspace_tool_t.ERASER && context_raw.tool != workspace_tool_t.CLONE && !decal_mask) {
			return 1.0;
		}
		let val: f32 = context_raw.brush_hardness * context_raw.brush_nodes_hardness;
		if (config_raw.pressure_hardness && pen_down()) {
			val *= pen_pressure * config_raw.pressure_sensitivity;
		}
		if (config_raw.brush_3d) {
			if (context_raw.paint2d) {
				val *= 1.0 / ui_view2d_pan_scale;
			}
			else {
				val *= val;
			}
		}
		return val;
	}
	else if (link == "_brushScale") {
		let fill: bool = context_raw.layer.fill_layer != null;
		let val: f32 = (fill ? context_raw.layer.scale : context_raw.brush_scale) * context_raw.brush_nodes_scale;
		return val;
	}
	else if (link == "_objectId") {
		return array_index_of(project_paint_objects, object.ext);
	}
	///if is_paint
	else if (link == "_dilateRadius") {
		return util_uv_dilatemap != null ? config_raw.dilate_radius : 0.0;
	}
	///end
	else if (link == "_decalLayerDim") {
		return mat4_get_scale(context_raw.layer.decal_mat).z * 0.5;
	}
	else if (link == "_pickerOpacity") {
		return context_raw.picked_color.opacity;
	}
	else if (link == "_pickerOcclusion") {
		return context_raw.picked_color.occlusion;
	}
	else if (link == "_pickerRoughness") {
		return context_raw.picked_color.roughness;
	}
	else if (link == "_pickerMetallic") {
		return context_raw.picked_color.metallic;
	}
	else if (link == "_pickerHeight") {
		return context_raw.picked_color.height;
	}
	///end

	if (parser_material_script_links != null) {
		let keys: string[] = map_keys_to_array(parser_material_script_links);
		for (let i: i32 = 0; i < keys.length; ++i) {
			let key: string = keys[i];
			let asciprt_links: any = parser_material_script_links;
			let script: string = asciprt_links[key];
			let result: f32 = 0.0;
			if (script != "") {
				result = js_eval(script);
			}
			return result;
		}
	}
	return 0.0;
}

function uniforms_ext_vec2_link(object: object_t, mat: material_data_t, link: string): vec4_t {
	if (link == "_gbufferSize") {
		vec4_set(uniforms_ext_vec, 0, 0, 0);
		let gbuffer2: render_target_t = map_get(render_path_render_targets, "gbuffer2");
		vec4_set(uniforms_ext_vec, gbuffer2._image.width, gbuffer2._image.height, 0);
		return uniforms_ext_vec;
	}
	else if (link == "_cloneDelta") {
		vec4_set(uniforms_ext_vec, context_raw.clone_delta_x, context_raw.clone_delta_y, 0);
		return uniforms_ext_vec;
	}
	else if (link == "_texpaintSize") {
		vec4_set(uniforms_ext_vec, config_get_texture_res_x(), config_get_texture_res_y(), 0);
		return uniforms_ext_vec;
	}
	///if (is_paint || is_sculpt)
	else if (link == "_brushAngle") {
		let brush_angle: f32 = context_raw.brush_angle + context_raw.brush_nodes_angle;
		let angle: f32 = context_raw.layer.fill_layer != null ? context_raw.layer.angle : brush_angle;
		angle *= (math_pi() / 180);
		if (config_raw.pressure_angle && pen_down()) {
			angle *= pen_pressure * config_raw.pressure_sensitivity;
		}
		vec4_set(uniforms_ext_vec, math_cos(-angle), math_sin(-angle), 0);
		return uniforms_ext_vec;
	}
	///end

	return null;
}

function uniforms_ext_vec3_link(object: object_t, mat: material_data_t, link: string): vec4_t {
	let v: vec4_t = null;

	///if (is_paint || is_sculpt)
	if (link == "_brushDirection") {
		v = _uniforms_vec;
		// Discard first paint for directional brush
		let allow_paint: bool = context_raw.prev_paint_vec_x != context_raw.last_paint_vec_x &&
								context_raw.prev_paint_vec_y != context_raw.last_paint_vec_y &&
								context_raw.prev_paint_vec_x > 0 &&
								context_raw.prev_paint_vec_y > 0;
		let x: f32 = context_raw.paint_vec.x;
		let y: f32 = context_raw.paint_vec.y;
		let lastx: f32 = context_raw.prev_paint_vec_x;
		let lasty: f32 = context_raw.prev_paint_vec_y;
		if (context_raw.paint2d) {
			x = vec2d(x);
			lastx = vec2d(lastx);
		}
		let angle: f32 = math_atan2(-y + lasty, x - lastx) - math_pi() / 2;
		vec4_set(v, math_cos(angle), math_sin(angle), allow_paint ? 1 : 0);
		context_raw.prev_paint_vec_x = context_raw.last_paint_vec_x;
		context_raw.prev_paint_vec_y = context_raw.last_paint_vec_y;
		return v;
	}
	else if (link == "_decalLayerLoc") {
		v = _uniforms_vec;
		vec4_set(v, context_raw.layer.decal_mat.m[12], context_raw.layer.decal_mat.m[13], context_raw.layer.decal_mat.m[14]);
		return v;
	}
	else if (link == "_decalLayerNor") {
		v = _uniforms_vec;
		vec4_normalize(vec4_set(v, context_raw.layer.decal_mat.m[8], context_raw.layer.decal_mat.m[9], context_raw.layer.decal_mat.m[10]));
		return v;
	}
	else if (link == "_pickerBase") {
		v = _uniforms_vec;
		vec4_set(v,
			color_get_rb(context_raw.picked_color.base) / 255,
			color_get_gb(context_raw.picked_color.base) / 255,
			color_get_bb(context_raw.picked_color.base) / 255
		);
		return v;
	}
	else if (link == "_pickerNormal") {
		v = _uniforms_vec;
		vec4_set(v,
			color_get_rb(context_raw.picked_color.normal) / 255,
			color_get_gb(context_raw.picked_color.normal) / 255,
			color_get_bb(context_raw.picked_color.normal) / 255
		);
		return v;
	}
	///if arm_physics
	else if (link == "_particleHit") {
		v = _uniforms_vec;
		vec4_set(v, context_raw.particle_hit_x, context_raw.particle_hit_y, context_raw.particle_hit_z);
		return v;
	}
	else if (link == "_particleHitLast") {
		v = _uniforms_vec;
		vec4_set(v, context_raw.last_particle_hit_x, context_raw.last_particle_hit_y, context_raw.last_particle_hit_z);
		return v;
	}
	///end
	///end

	return v;
}

///if (is_paint || is_sculpt)
function vec2d(x: f32) {
	// Transform from 3d viewport coord to 2d view coord
	context_raw.paint2d_view = false;
	let res: f32 = (x * base_w() - base_w()) / ui_view2d_ww;
	context_raw.paint2d_view = true;
	return res;
}
///end

function uniforms_ext_vec4_link(object: object_t, mat: material_data_t, link: string): vec4_t {
	if (link == "_inputBrush") {
		let down: bool = mouse_down() || pen_down();
		vec4_set(uniforms_ext_vec, context_raw.paint_vec.x, context_raw.paint_vec.y, down ? 1.0 : 0.0, 0.0);

		///if (is_paint || is_sculpt)
		if (context_raw.paint2d) {
			uniforms_ext_vec.x = vec2d(uniforms_ext_vec.x);
		}
		///end

		return uniforms_ext_vec;
	}
	else if (link == "_inputBrushLast") {
		let down: bool = mouse_down() || pen_down();
		vec4_set(uniforms_ext_vec, context_raw.last_paint_vec_x, context_raw.last_paint_vec_y, down ? 1.0 : 0.0, 0.0);

		///if (is_paint || is_sculpt)
		if (context_raw.paint2d) {
			uniforms_ext_vec.x = vec2d(uniforms_ext_vec.x);
		}
		///end

		return uniforms_ext_vec;
	}
	else if (link == "_envmapData") {
		vec4_set(uniforms_ext_vec, context_raw.envmap_angle, math_sin(-context_raw.envmap_angle), math_cos(-context_raw.envmap_angle), scene_world.strength);
		return uniforms_ext_vec;
	}
	else if (link == "_envmapDataWorld") {
		vec4_set(uniforms_ext_vec, context_raw.envmap_angle, math_sin(-context_raw.envmap_angle), math_cos(-context_raw.envmap_angle), context_raw.show_envmap ? scene_world.strength : 1.0);
		return uniforms_ext_vec;
	}
	///if (is_paint || is_sculpt)
	else if (link == "_stencilTransform") {
		vec4_set(uniforms_ext_vec, context_raw.brush_stencil_x, context_raw.brush_stencil_y, context_raw.brush_stencil_scale, context_raw.brush_stencil_angle);
		if (context_raw.paint2d) uniforms_ext_vec.x = vec2d(uniforms_ext_vec.x);
		return uniforms_ext_vec;
	}
	else if (link == "_decalMask") {
		let decal_mask: bool = operator_shortcut(map_get(config_keymap, "decal_mask") + "+" + map_get(config_keymap, "action_paint"), shortcut_type_t.DOWN);
		let val: f32 = (context_raw.brush_radius * context_raw.brush_nodes_radius) / 15.0;
		let scale2d: f32 = (900 / base_h()) * config_raw.window_scale;
		val *= scale2d; // Projection ratio
		vec4_set(uniforms_ext_vec, context_raw.decal_x, context_raw.decal_y, decal_mask ? 1 : 0, val);
		if (context_raw.paint2d) {
			uniforms_ext_vec.x = vec2d(uniforms_ext_vec.x);
		}
		return uniforms_ext_vec;
	}
	///end

	return null;
}

function uniforms_ext_mat4_link(object: object_t, mat: material_data_t, link: string): mat4_t {
	///if (is_paint || is_sculpt)
	if (link == "_decalLayerMatrix") { // Decal layer
		let m: mat4_t = _uniforms_mat;
		mat4_set_from(m, context_raw.layer.decal_mat);
		mat4_get_inv(m, m);
		mat4_mult_mat(m, uniforms_ext_ortho_p);
		return m;
	}
	///end

	return null;
}

function uniforms_ext_tex_link(object: object_t, mat: material_data_t, link: string): image_t {
	if (link == "_texpaint_undo") {
		///if (is_paint || is_sculpt)
		let i: i32 = history_undo_i - 1 < 0 ? config_raw.undo_steps - 1 : history_undo_i - 1;
		return map_get(render_path_render_targets, "texpaint_undo" + i)._image;
		///end

		///if is_lab
		return null;
		///end
	}
	else if (link == "_texpaint_nor_undo") {
		///if (is_paint || is_sculpt)
		let i: i32 = history_undo_i - 1 < 0 ? config_raw.undo_steps - 1 : history_undo_i - 1;
		return map_get(render_path_render_targets, "texpaint_nor_undo" + i)._image;
		///end

		///if is_lab
		return null;
		///end
	}
	else if (link == "_texpaint_pack_undo") {
		///if (is_paint || is_sculpt)
		let i: i32 = history_undo_i - 1 < 0 ? config_raw.undo_steps - 1 : history_undo_i - 1;
		return map_get(render_path_render_targets, "texpaint_pack_undo" + i)._image;
		///end

		///if is_lab
		return null;
		///end
	}
	else if (link == "_ltcMat") {
		if (const_data_ltc_mat_tex == null) {
			const_data_init_ltc();
		}
		return const_data_ltc_mat_tex;
	}
	else if (link == "_ltcMag") {
		if (const_data_ltc_mag_tex == null) {
			const_data_init_ltc();
		}
		return const_data_ltc_mag_tex;
	}
	///if (is_paint || is_sculpt)
	else if (link == "_texcolorid") {
		if (project_assets.length == 0) {
			return map_get(render_path_render_targets, "empty_white")._image;
		}
		else {
			return project_get_image(project_assets[context_raw.colorid_handle.position]);
		}
	}
	else if (link == "_textexttool") { // Opacity map for text
		return context_raw.text_tool_image;
	}
	else if (link == "_texbrushmask") {
		return context_raw.brush_mask_image;
	}
	else if (link == "_texbrushstencil") {
		return context_raw.brush_stencil_image;
	}
	else if (link == "_texparticle") {
		return map_get(render_path_render_targets, "texparticle")._image;
	}
	///end

	///if is_paint
	else if (link == "_texuvmap") {
		if (!util_uv_uvmap_cached) {
			app_notify_on_init(util_uv_cache_uv_map);
		}
		return util_uv_uvmap;
	}
	else if (link == "_textrianglemap") {
		if (!util_uv_trianglemap_cached) {
			app_notify_on_init(util_uv_cache_triangle_map);
		}
		return util_uv_trianglemap;
	}
	else if (link == "_texuvislandmap") {
		app_notify_on_init(util_uv_cache_uv_island_map);
		return util_uv_uvislandmap_cached ? util_uv_uvislandmap :map_get(render_path_render_targets, "empty_black")._image;
	}
	else if (link == "_texdilatemap") {
		return util_uv_dilatemap;
	}
	///end

	if (starts_with(link, "_texpaint_pack_vert")) {
		let tid: string = substring(link, link.length - 1, link.length);
		return map_get(render_path_render_targets, "texpaint_pack" + tid)._image;
	}

	if (starts_with(link, "_texpaint_vert")) {
		///if (is_paint || is_sculpt)
		let tid: i32 = Number(substring(link, link.length - 1, link.length));
		return tid < project_layers.length ? project_layers[tid].texpaint : null;
		///end

		///if is_lab
		return brush_output_node_inst.texpaint;
		///end
	}
	if (starts_with(link, "_texpaint_nor")) {
		///if is_paint
		let tid: i32 = Number(substring(link, link.length - 1, link.length));
		return tid < project_layers.length ? project_layers[tid].texpaint_nor : null;
		///end

		///if is_lab
		return brush_output_node_inst.texpaint_nor;
		///end
	}
	if (starts_with(link, "_texpaint_pack")) {
		///if is_paint
		let tid: i32 = Number(substring(link, link.length - 1, link.length));
		return tid < project_layers.length ? project_layers[tid].texpaint_pack : null;
		///end

		///if is_lab
		return brush_output_node_inst.texpaint_pack;
		///end
	}
	if (starts_with(link, "_texpaint")) {
		///if (is_paint || is_sculpt)
		let tid: i32 = Number(substring(link, link.length - 1, link.length));
		return tid < project_layers.length ? project_layers[tid].texpaint : null;
		///end

		///if is_lab
		return brush_output_node_inst.texpaint;
		///end
	}

	///if (is_paint || is_sculpt)
	if (starts_with(link, "_texblur_")) {
		let id: string = substring(link, 9, link.length);
		return context_raw.node_previews != null ? map_get(context_raw.node_previews, id) :map_get(render_path_render_targets, "empty_black")._image;
	}
	if (starts_with(link, "_texwarp_")) {
		let id: string = substring(link, 9, link.length);
		return context_raw.node_previews != null ? map_get(context_raw.node_previews, id) :map_get(render_path_render_targets, "empty_black")._image;
	}
	if (starts_with(link, "_texbake_")) {
		let id: string = substring(link, 9, link.length);
		return context_raw.node_previews != null ? map_get(context_raw.node_previews, id) :map_get(render_path_render_targets, "empty_black")._image;
	}
	///end

	return null;
}
