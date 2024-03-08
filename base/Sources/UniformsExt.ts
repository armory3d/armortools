
class UniformsExt {

	static vec: vec4_t = vec4_create();
	static ortho_p: mat4_t = mat4_ortho(-0.5, 0.5, -0.5, 0.5, -0.5, 0.5);

	static init = () => {
		uniforms_i32_links = UniformsExt.link_int;
		uniforms_f32_links = UniformsExt.link_float;
		uniforms_vec2_links = UniformsExt.link_vec2;
		uniforms_vec3_links = UniformsExt.link_vec3;
		uniforms_vec4_links = UniformsExt.link_vec4;
		uniforms_mat4_links = UniformsExt.link_mat4;
		uniforms_tex_links = UniformsExt.link_tex;
	}

	static link_int = (object: object_t, mat: material_data_t, link: string): Null<i32> => {
		if (link == "_bloomCurrentMip") return RenderPathBase.bloom_current_mip;
		return null;
	}

	static link_float = (object: object_t, mat: material_data_t, link: string): Null<f32> => {
		switch (link) {
			case "_brushRadius": {
				///if (is_paint || is_sculpt)
				let decal: bool = Context.raw.tool == workspace_tool_t.DECAL || Context.raw.tool == workspace_tool_t.TEXT;
				let decal_mask: bool = decal && Operator.shortcut(Config.keymap.decal_mask + "+" + Config.keymap.action_paint, ShortcutType.ShortcutDown);
				let brush_decal_mask_radius: f32 = Context.raw.brush_decal_mask_radius;
				if (Config.raw.brush_3d) {
					brush_decal_mask_radius *= Context.raw.paint2d ? 0.55 * UIView2D.pan_scale : 2.0;
				}
				let radius: f32 = decal_mask ? brush_decal_mask_radius : Context.raw.brush_radius;
				let val: f32 = (radius * Context.raw.brush_nodes_radius) / 15.0;
				if (Config.raw.pressure_radius && pen_down()) {
					val *= pen_pressure * Config.raw.pressure_sensitivity;
				}
				let scale2d: f32 = (900 / base_h()) * Config.raw.window_scale;

				if (Config.raw.brush_3d && !decal) {
					val *= Context.raw.paint2d ? 0.55 * scale2d * UIView2D.pan_scale : 2;
				}
				else {
					val *= scale2d; // Projection ratio
				}
				///end

				///if is_lab
				let radius: f32 = Context.raw.brush_radius;
				let val: f32 = radius / 15.0;
				if (Config.raw.pressure_radius && pen_down()) {
					val *= pen_pressure * Config.raw.pressure_sensitivity;
				}
				val *= 2;
				///end

				return val;
			}
			case "_vignetteStrength": {
				return Config.raw.rp_vignette;
			}
			case "_grainStrength": {
				return Config.raw.rp_grain;
			}
			case "_coneOffset": {
				return Context.raw.vxao_offset;
			}
			case "_coneAperture": {
				return Context.raw.vxao_aperture;
			}
			case "_bloomSampleScale": {
				return RenderPathBase.bloom_sample_scale;
			}

			///if (is_paint || is_sculpt)
			case "_brushScaleX": {
				return 1 / Context.raw.brush_scale_x;
			}
			case "_brushOpacity": {
				let val: f32 = Context.raw.brush_opacity * Context.raw.brush_nodes_opacity;
				if (Config.raw.pressure_opacity && pen_down()) {
					val *= pen_pressure * Config.raw.pressure_sensitivity;
				}
				return val;
			}
			case "_brushHardness": {
				let decal_mask: bool = Operator.shortcut(Config.keymap.decal_mask + "+" + Config.keymap.action_paint, ShortcutType.ShortcutDown);
				if (Context.raw.tool != workspace_tool_t.BRUSH && Context.raw.tool != workspace_tool_t.ERASER && Context.raw.tool != workspace_tool_t.CLONE && !decal_mask) return 1.0;
				let val: f32 = Context.raw.brush_hardness * Context.raw.brush_nodes_hardness;
				if (Config.raw.pressure_hardness && pen_down()) {
					val *= pen_pressure * Config.raw.pressure_sensitivity;
				}
				if (Config.raw.brush_3d) {
					if (Context.raw.paint2d) {
						val *= 1.0 / UIView2D.pan_scale;
					}
					else {
						val *= val;
					}
				}
				return val;
			}
			case "_brushScale": {
				let fill: bool = Context.raw.layer.fill_layer != null;
				let val: f32 = (fill ? Context.raw.layer.scale : Context.raw.brush_scale) * Context.raw.brush_nodes_scale;
				return val;
			}
			case "_objectId": {
				return Project.paint_objects.indexOf(object.ext);
			}
			///if is_paint
			case "_dilateRadius": {
				return UtilUV.dilatemap != null ? Config.raw.dilate_radius : 0.0;
			}
			///end
			case "_decalLayerDim": {
				return mat4_get_scale(Context.raw.layer.decal_mat).z * 0.5;
			}
			case "_pickerOpacity": {
				return Context.raw.picked_color.opacity;
			}
			case "_pickerOcclusion": {
				return Context.raw.picked_color.occlusion;
			}
			case "_pickerRoughness": {
				return Context.raw.picked_color.roughness;
			}
			case "_pickerMetallic": {
				return Context.raw.picked_color.metallic;
			}
			case "_pickerHeight": {
				return Context.raw.picked_color.height;
			}
			///end
		}
		if (ParserMaterial.script_links != null) {
			for (let key of ParserMaterial.script_links.keys()) {
				let asciprt_links: any = ParserMaterial.script_links;
				let script: string = asciprt_links[key];
				let result: f32 = 0.0;
				if (script != "") {
					try {
						result = eval(script);
					}
					catch(e: any) {
						Console.log(e);
					}
				}
				return result;
			}
		}
		return null;
	}

	static link_vec2 = (object: object_t, mat: material_data_t, link: string): vec4_t => {
		switch (link) {
			case "_gbufferSize": {
				vec4_set(UniformsExt.vec, 0, 0, 0);
				let gbuffer2: render_target_t = render_path_render_targets.get("gbuffer2");
				vec4_set(UniformsExt.vec, gbuffer2._image.width, gbuffer2._image.height, 0);
				return UniformsExt.vec;
			}
			case "_cloneDelta": {
				vec4_set(UniformsExt.vec, Context.raw.clone_delta_x, Context.raw.clone_delta_y, 0);
				return UniformsExt.vec;
			}
			case "_texpaintSize": {
				vec4_set(UniformsExt.vec, Config.get_texture_res_x(), Config.get_texture_res_y(), 0);
				return UniformsExt.vec;
			}
			///if (is_paint || is_sculpt)
			case "_brushAngle": {
				let brush_angle: f32 = Context.raw.brush_angle + Context.raw.brush_nodes_angle;
				let angle: f32 = Context.raw.layer.fill_layer != null ? Context.raw.layer.angle : brush_angle;
				angle *= (Math.PI / 180);
				if (Config.raw.pressure_angle && pen_down()) {
					angle *= pen_pressure * Config.raw.pressure_sensitivity;
				}
				vec4_set(UniformsExt.vec, Math.cos(-angle), Math.sin(-angle), 0);
				return UniformsExt.vec;
			}
			///end
		}
		return null;
	}

	static link_vec3 = (object: object_t, mat: material_data_t, link: string): vec4_t => {
		let v: vec4_t = null;
		switch (link) {
			///if (is_paint || is_sculpt)
			case "_brushDirection": {
				v = _uniforms_vec;
				// Discard first paint for directional brush
				let allow_paint: bool = Context.raw.prev_paint_vec_x != Context.raw.last_paint_vec_x &&
								 	   Context.raw.prev_paint_vec_y != Context.raw.last_paint_vec_y &&
								 	   Context.raw.prev_paint_vec_x > 0 &&
								 	   Context.raw.prev_paint_vec_y > 0;
				let x: f32 = Context.raw.paint_vec.x;
				let y: f32 = Context.raw.paint_vec.y;
				let lastx: f32 = Context.raw.prev_paint_vec_x;
				let lasty: f32 = Context.raw.prev_paint_vec_y;
				if (Context.raw.paint2d) {
					x = UniformsExt.vec2d(x);
					lastx = UniformsExt.vec2d(lastx);
				}
				let angle: f32 = Math.atan2(-y + lasty, x - lastx) - Math.PI / 2;
				vec4_set(v, Math.cos(angle), Math.sin(angle), allow_paint ? 1 : 0);
				Context.raw.prev_paint_vec_x = Context.raw.last_paint_vec_x;
				Context.raw.prev_paint_vec_y = Context.raw.last_paint_vec_y;
				return v;
			}
			case "_decalLayerLoc": {
				v = _uniforms_vec;
				vec4_set(v, Context.raw.layer.decal_mat.m[12], Context.raw.layer.decal_mat.m[13], Context.raw.layer.decal_mat.m[14]);
				return v;
			}
			case "_decalLayerNor": {
				v = _uniforms_vec;
				vec4_normalize(vec4_set(v, Context.raw.layer.decal_mat.m[8], Context.raw.layer.decal_mat.m[9], Context.raw.layer.decal_mat.m[10]));
				return v;
			}
			case "_pickerBase": {
				v = _uniforms_vec;
				vec4_set(v,
					color_get_rb(Context.raw.picked_color.base) / 255,
					color_get_gb(Context.raw.picked_color.base) / 255,
					color_get_bb(Context.raw.picked_color.base) / 255
				);
				return v;
			}
			case "_pickerNormal": {
				v = _uniforms_vec;
				vec4_set(v,
					color_get_rb(Context.raw.picked_color.normal) / 255,
					color_get_gb(Context.raw.picked_color.normal) / 255,
					color_get_bb(Context.raw.picked_color.normal) / 255
				);
				return v;
			}
			///if arm_physics
			case "_particleHit": {
				v = _uniforms_vec;
				vec4_set(v, Context.raw.particle_hit_x, Context.raw.particle_hit_y, Context.raw.particle_hit_z);
				return v;
			}
			case "_particleHitLast": {
				v = _uniforms_vec;
				vec4_set(v, Context.raw.last_particle_hit_x, Context.raw.last_particle_hit_y, Context.raw.last_particle_hit_z);
				return v;
			}
			///end
			///end
		}

		return v;
	}

	///if (is_paint || is_sculpt)
	static vec2d = (x: f32) => {
		// Transform from 3d viewport coord to 2d view coord
		Context.raw.paint2d_view = false;
		let res: f32 = (x * base_w() - base_w()) / UIView2D.ww;
		Context.raw.paint2d_view = true;
		return res;
	}
	///end

	static link_vec4 = (object: object_t, mat: material_data_t, link: string): vec4_t => {
		switch (link) {
			case "_inputBrush": {
				let down: bool = mouse_down() || pen_down();
				vec4_set(UniformsExt.vec, Context.raw.paint_vec.x, Context.raw.paint_vec.y, down ? 1.0 : 0.0, 0.0);

				///if (is_paint || is_sculpt)
				if (Context.raw.paint2d) {
					UniformsExt.vec.x = UniformsExt.vec2d(UniformsExt.vec.x);
				}
				///end

				return UniformsExt.vec;
			}
			case "_inputBrushLast": {
				let down: bool = mouse_down() || pen_down();
				vec4_set(UniformsExt.vec, Context.raw.last_paint_vec_x, Context.raw.last_paint_vec_y, down ? 1.0 : 0.0, 0.0);

				///if (is_paint || is_sculpt)
				if (Context.raw.paint2d) {
					UniformsExt.vec.x = UniformsExt.vec2d(UniformsExt.vec.x);
				}
				///end

				return UniformsExt.vec;
			}
			case "_envmapData": {
				vec4_set(UniformsExt.vec, Context.raw.envmap_angle, Math.sin(-Context.raw.envmap_angle), Math.cos(-Context.raw.envmap_angle), scene_world.strength);
				return UniformsExt.vec;
			}
			case "_envmapDataWorld": {
				vec4_set(UniformsExt.vec, Context.raw.envmap_angle, Math.sin(-Context.raw.envmap_angle), Math.cos(-Context.raw.envmap_angle), Context.raw.show_envmap ? scene_world.strength : 1.0);
				return UniformsExt.vec;
			}
			///if (is_paint || is_sculpt)
			case "_stencilTransform": {
				vec4_set(UniformsExt.vec, Context.raw.brush_stencil_x, Context.raw.brush_stencil_y, Context.raw.brush_stencil_scale, Context.raw.brush_stencil_angle);
				if (Context.raw.paint2d) UniformsExt.vec.x = UniformsExt.vec2d(UniformsExt.vec.x);
				return UniformsExt.vec;
			}
			case "_decalMask": {
				let decal_mask: bool = Operator.shortcut(Config.keymap.decal_mask + "+" + Config.keymap.action_paint, ShortcutType.ShortcutDown);
				let val: f32 = (Context.raw.brush_radius * Context.raw.brush_nodes_radius) / 15.0;
				let scale2d: f32 = (900 / base_h()) * Config.raw.window_scale;
				val *= scale2d; // Projection ratio
				vec4_set(UniformsExt.vec, Context.raw.decal_x, Context.raw.decal_y, decal_mask ? 1 : 0, val);
				if (Context.raw.paint2d) UniformsExt.vec.x = UniformsExt.vec2d(UniformsExt.vec.x);
				return UniformsExt.vec;
			}
			///end
		}
		return null;
	}

	static link_mat4 = (object: object_t, mat: material_data_t, link: string): mat4_t => {
		switch (link) {
			///if (is_paint || is_sculpt)
			case "_decalLayerMatrix": { // Decal layer
				let m: mat4_t = _uniforms_mat;
				mat4_set_from(m, Context.raw.layer.decal_mat);
				mat4_get_inv(m, m);
				mat4_mult_mat(m, UniformsExt.ortho_p);
				return m;
			}
			///end
		}
		return null;
	}

	static link_tex = (object: object_t, mat: material_data_t, link: string): image_t => {
		switch (link) {
			case "_texpaint_undo": {
				///if (is_paint || is_sculpt)
				let i: i32 = History.undo_i - 1 < 0 ? Config.raw.undo_steps - 1 : History.undo_i - 1;
				return render_path_render_targets.get("texpaint_undo" + i)._image;
				///end

				///if is_lab
				return null;
				///end
			}
			case "_texpaint_nor_undo": {
				///if (is_paint || is_sculpt)
				let i: i32 = History.undo_i - 1 < 0 ? Config.raw.undo_steps - 1 : History.undo_i - 1;
				return render_path_render_targets.get("texpaint_nor_undo" + i)._image;
				///end

				///if is_lab
				return null;
				///end
			}
			case "_texpaint_pack_undo": {
				///if (is_paint || is_sculpt)
				let i: i32 = History.undo_i - 1 < 0 ? Config.raw.undo_steps - 1 : History.undo_i - 1;
				return render_path_render_targets.get("texpaint_pack_undo" + i)._image;
				///end

				///if is_lab
				return null;
				///end
			}

			case "_ltcMat": {
				if (const_data_ltc_mat_tex == null) const_data_init_ltc();
				return const_data_ltc_mat_tex;
			}
			case "_ltcMag": {
				if (const_data_ltc_mag_tex == null) const_data_init_ltc();
				return const_data_ltc_mag_tex;
			}

			///if (is_paint || is_sculpt)
			case "_texcolorid": {
				if (Project.assets.length == 0) return render_path_render_targets.get("empty_white")._image;
				else return Project.get_image(Project.assets[Context.raw.colorid_handle.position]);
			}
			case "_textexttool": { // Opacity map for text
				return Context.raw.text_tool_image;
			}
			case "_texbrushmask": {
				return Context.raw.brush_mask_image;
			}
			case "_texbrushstencil": {
				return Context.raw.brush_stencil_image;
			}
			case "_texparticle": {
				return render_path_render_targets.get("texparticle")._image;
			}
			///end

			///if is_paint
			case "_texuvmap": {
				if (!UtilUV.uvmap_cached) {
					let _init = () => {
						UtilUV.cache_uv_map();
					}
					app_notify_on_init(_init);
				}
				return UtilUV.uvmap;
			}
			case "_textrianglemap": {
				if (!UtilUV.trianglemap_cached) {
					let _init = () => {
						UtilUV.cache_triangle_map();
					}
					app_notify_on_init(_init);
				}
				return UtilUV.trianglemap;
			}
			case "_texuvislandmap": {
				let _init = () => {
					UtilUV.cache_uv_island_map();
				}
				app_notify_on_init(_init);
				return UtilUV.uvislandmap_cached ? UtilUV.uvislandmap :render_path_render_targets.get("empty_black")._image;
			}
			case "_texdilatemap": {
				return UtilUV.dilatemap;
			}
			///end
		}

		if (link.startsWith("_texpaint_pack_vert")) {
			let tid: string = link.substr(link.length - 1);
			return render_path_render_targets.get("texpaint_pack" + tid)._image;
		}

		if (link.startsWith("_texpaint_vert")) {
			///if (is_paint || is_sculpt)
			let tid: i32 = Number(link.substr(link.length - 1));
			return tid < Project.layers.length ? Project.layers[tid].texpaint : null;
			///end

			///if is_lab
			return BrushOutputNode.inst.texpaint;
			///end
		}
		if (link.startsWith("_texpaint_nor")) {
			///if is_paint
			let tid: i32 = Number(link.substr(link.length - 1));
			return tid < Project.layers.length ? Project.layers[tid].texpaint_nor : null;
			///end

			///if is_lab
			return BrushOutputNode.inst.texpaint_nor;
			///end
		}
		if (link.startsWith("_texpaint_pack")) {
			///if is_paint
			let tid: i32 = Number(link.substr(link.length - 1));
			return tid < Project.layers.length ? Project.layers[tid].texpaint_pack : null;
			///end

			///if is_lab
			return BrushOutputNode.inst.texpaint_pack;
			///end
		}
		if (link.startsWith("_texpaint")) {
			///if (is_paint || is_sculpt)
			let tid: i32 = Number(link.substr(link.length - 1));
			return tid < Project.layers.length ? Project.layers[tid].texpaint : null;
			///end

			///if is_lab
			return BrushOutputNode.inst.texpaint;
			///end
		}

		///if (is_paint || is_sculpt)
		if (link.startsWith("_texblur_")) {
			let id: string = link.substr(9);
			return Context.raw.node_previews != null ? Context.raw.node_previews.get(id) :render_path_render_targets.get("empty_black")._image;
		}
		if (link.startsWith("_texwarp_")) {
			let id: string = link.substr(9);
			return Context.raw.node_previews != null ? Context.raw.node_previews.get(id) :render_path_render_targets.get("empty_black")._image;
		}
		if (link.startsWith("_texbake_")) {
			let id: string = link.substr(9);
			return Context.raw.node_previews != null ? Context.raw.node_previews.get(id) :render_path_render_targets.get("empty_black")._image;
		}
		///end

		return null;
	}
}
