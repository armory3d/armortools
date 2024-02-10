
class UniformsExt {

	static vec = vec4_create();
	static orthoP = mat4_ortho(-0.5, 0.5, -0.5, 0.5, -0.5, 0.5);

	static init = () => {
		uniforms_i32_links = UniformsExt.linkInt;
		uniforms_f32_links = UniformsExt.linkFloat;
		uniforms_vec2_links = UniformsExt.linkVec2;
		uniforms_vec3_links = UniformsExt.linkVec3;
		uniforms_vec4_links = UniformsExt.linkVec4;
		uniforms_mat4_links = UniformsExt.linkMat4;
		uniforms_tex_links = UniformsExt.linkTex;
	}

	static linkInt = (object: object_t, mat: material_data_t, link: string): Null<i32> => {
		if (link == "_bloomCurrentMip") return RenderPathBase.bloomCurrentMip;
		return null;
	}

	static linkFloat = (object: object_t, mat: material_data_t, link: string): Null<f32> => {
		switch (link) {
			case "_brushRadius": {
				///if (is_paint || is_sculpt)
				let decal = Context.raw.tool == WorkspaceTool.ToolDecal || Context.raw.tool == WorkspaceTool.ToolText;
				let decalMask = decal && Operator.shortcut(Config.keymap.decal_mask + "+" + Config.keymap.action_paint, ShortcutType.ShortcutDown);
				let brushDecalMaskRadius = Context.raw.brushDecalMaskRadius;
				if (Config.raw.brush_3d) {
					brushDecalMaskRadius *= Context.raw.paint2d ? 0.55 * UIView2D.panScale : 2.0;
				}
				let radius = decalMask ? brushDecalMaskRadius : Context.raw.brushRadius;
				let val = (radius * Context.raw.brushNodesRadius) / 15.0;
				if (Config.raw.pressure_radius && pen_down()) {
					val *= pen_pressure * Config.raw.pressure_sensitivity;
				}
				let scale2d = (900 / Base.h()) * Config.raw.window_scale;

				if (Config.raw.brush_3d && !decal) {
					val *= Context.raw.paint2d ? 0.55 * scale2d * UIView2D.panScale : 2;
				}
				else {
					val *= scale2d; // Projection ratio
				}
				///end

				///if is_lab
				let radius = Context.raw.brushRadius;
				let val = radius / 15.0;
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
				return Context.raw.vxaoOffset;
			}
			case "_coneAperture": {
				return Context.raw.vxaoAperture;
			}
			case "_bloomSampleScale": {
				return RenderPathBase.bloomSampleScale;
			}

			///if (is_paint || is_sculpt)
			case "_brushScaleX": {
				return 1 / Context.raw.brushScaleX;
			}
			case "_brushOpacity": {
				let val = Context.raw.brushOpacity * Context.raw.brushNodesOpacity;
				if (Config.raw.pressure_opacity && pen_down()) {
					val *= pen_pressure * Config.raw.pressure_sensitivity;
				}
				return val;
			}
			case "_brushHardness": {
				let decal = Context.raw.tool == WorkspaceTool.ToolDecal || Context.raw.tool == WorkspaceTool.ToolText;
				let decalMask = Operator.shortcut(Config.keymap.decal_mask + "+" + Config.keymap.action_paint, ShortcutType.ShortcutDown);
				if (Context.raw.tool != WorkspaceTool.ToolBrush && Context.raw.tool != WorkspaceTool.ToolEraser && Context.raw.tool != WorkspaceTool.ToolClone && !decalMask) return 1.0;
				let val = Context.raw.brushHardness * Context.raw.brushNodesHardness;
				if (Config.raw.pressure_hardness && pen_down()) {
					val *= pen_pressure * Config.raw.pressure_sensitivity;
				}
				if (Config.raw.brush_3d) {
					if (Context.raw.paint2d) {
						val *= 1.0 / UIView2D.panScale;
					}
					else {
						val *= val;
					}
				}
				return val;
			}
			case "_brushScale": {
				let fill = Context.raw.layer.fill_layer != null;
				let val = (fill ? Context.raw.layer.scale : Context.raw.brushScale) * Context.raw.brushNodesScale;
				return val;
			}
			case "_objectId": {
				return Project.paintObjects.indexOf(object.ext);
			}
			///if is_paint
			case "_dilateRadius": {
				return UtilUV.dilatemap != null ? Config.raw.dilate_radius : 0.0;
			}
			///end
			case "_decalLayerDim": {
				return mat4_get_scale(Context.raw.layer.decalMat).z * 0.5;
			}
			case "_pickerOpacity": {
				return Context.raw.pickedColor.opacity;
			}
			case "_pickerOcclusion": {
				return Context.raw.pickedColor.occlusion;
			}
			case "_pickerRoughness": {
				return Context.raw.pickedColor.roughness;
			}
			case "_pickerMetallic": {
				return Context.raw.pickedColor.metallic;
			}
			case "_pickerHeight": {
				return Context.raw.pickedColor.height;
			}
			///end
		}
		if (ParserMaterial.script_links != null) {
			for (let key of ParserMaterial.script_links.keys()) {
				let asciprt_links: any = ParserMaterial.script_links;
				let script = asciprt_links[key];
				let result = 0.0;
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

	static linkVec2 = (object: object_t, mat: material_data_t, link: string): vec4_t => {
		switch (link) {
			case "_gbufferSize": {
				vec4_set(UniformsExt.vec, 0, 0, 0);
				let gbuffer2 = render_path_render_targets.get("gbuffer2");
				vec4_set(UniformsExt.vec, gbuffer2.image.width, gbuffer2.image.height, 0);
				return UniformsExt.vec;
			}
			case "_cloneDelta": {
				vec4_set(UniformsExt.vec, Context.raw.cloneDeltaX, Context.raw.cloneDeltaY, 0);
				return UniformsExt.vec;
			}
			case "_texpaintSize": {
				vec4_set(UniformsExt.vec, Config.getTextureResX(), Config.getTextureResY(), 0);
				return UniformsExt.vec;
			}
			///if (is_paint || is_sculpt)
			case "_brushAngle": {
				let brushAngle = Context.raw.brushAngle + Context.raw.brushNodesAngle;
				let angle = Context.raw.layer.fill_layer != null ? Context.raw.layer.angle : brushAngle;
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

	static linkVec3 = (object: object_t, mat: material_data_t, link: string): vec4_t => {
		let v: vec4_t = null;
		switch (link) {
			///if (is_paint || is_sculpt)
			case "_brushDirection": {
				v = _uniforms_vec;
				// Discard first paint for directional brush
				let allowPaint = Context.raw.prevPaintVecX != Context.raw.lastPaintVecX &&
								 Context.raw.prevPaintVecY != Context.raw.lastPaintVecY &&
								 Context.raw.prevPaintVecX > 0 &&
								 Context.raw.prevPaintVecY > 0;
				let x = Context.raw.paintVec.x;
				let y = Context.raw.paintVec.y;
				let lastx = Context.raw.prevPaintVecX;
				let lasty = Context.raw.prevPaintVecY;
				if (Context.raw.paint2d) {
					x = UniformsExt.vec2d(x);
					lastx = UniformsExt.vec2d(lastx);
				}
				let angle = Math.atan2(-y + lasty, x - lastx) - Math.PI / 2;
				vec4_set(v, Math.cos(angle), Math.sin(angle), allowPaint ? 1 : 0);
				Context.raw.prevPaintVecX = Context.raw.lastPaintVecX;
				Context.raw.prevPaintVecY = Context.raw.lastPaintVecY;
				return v;
			}
			case "_decalLayerLoc": {
				v = _uniforms_vec;
				vec4_set(v, Context.raw.layer.decalMat.m[12], Context.raw.layer.decalMat.m[13], Context.raw.layer.decalMat.m[14]);
				return v;
			}
			case "_decalLayerNor": {
				v = _uniforms_vec;
				vec4_normalize(vec4_set(v, Context.raw.layer.decalMat.m[8], Context.raw.layer.decalMat.m[9], Context.raw.layer.decalMat.m[10]));
				return v;
			}
			case "_pickerBase": {
				v = _uniforms_vec;
				vec4_set(v,
					color_get_rb(Context.raw.pickedColor.base) / 255,
					color_get_gb(Context.raw.pickedColor.base) / 255,
					color_get_bb(Context.raw.pickedColor.base) / 255
				);
				return v;
			}
			case "_pickerNormal": {
				v = _uniforms_vec;
				vec4_set(v,
					color_get_rb(Context.raw.pickedColor.normal) / 255,
					color_get_gb(Context.raw.pickedColor.normal) / 255,
					color_get_bb(Context.raw.pickedColor.normal) / 255
				);
				return v;
			}
			///if arm_physics
			case "_particleHit": {
				v = _uniforms_vec;
				vec4_set(v, Context.raw.particleHitX, Context.raw.particleHitY, Context.raw.particleHitZ);
				return v;
			}
			case "_particleHitLast": {
				v = _uniforms_vec;
				vec4_set(v, Context.raw.lastParticleHitX, Context.raw.lastParticleHitY, Context.raw.lastParticleHitZ);
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
		Context.raw.paint2dView = false;
		let res = (x * Base.w() - Base.w()) / UIView2D.ww;
		Context.raw.paint2dView = true;
		return res;
	}
	///end

	static linkVec4 = (object: object_t, mat: material_data_t, link: string): vec4_t => {
		switch (link) {
			case "_inputBrush": {
				let down = mouse_down() || pen_down();
				vec4_set(UniformsExt.vec, Context.raw.paintVec.x, Context.raw.paintVec.y, down ? 1.0 : 0.0, 0.0);

				///if (is_paint || is_sculpt)
				if (Context.raw.paint2d) {
					UniformsExt.vec.x = UniformsExt.vec2d(UniformsExt.vec.x);
				}
				///end

				return UniformsExt.vec;
			}
			case "_inputBrushLast": {
				let down = mouse_down() || pen_down();
				vec4_set(UniformsExt.vec, Context.raw.lastPaintVecX, Context.raw.lastPaintVecY, down ? 1.0 : 0.0, 0.0);

				///if (is_paint || is_sculpt)
				if (Context.raw.paint2d) {
					UniformsExt.vec.x = UniformsExt.vec2d(UniformsExt.vec.x);
				}
				///end

				return UniformsExt.vec;
			}
			case "_envmapData": {
				vec4_set(UniformsExt.vec, Context.raw.envmapAngle, Math.sin(-Context.raw.envmapAngle), Math.cos(-Context.raw.envmapAngle), scene_world.strength);
				return UniformsExt.vec;
			}
			case "_envmapDataWorld": {
				vec4_set(UniformsExt.vec, Context.raw.envmapAngle, Math.sin(-Context.raw.envmapAngle), Math.cos(-Context.raw.envmapAngle), Context.raw.showEnvmap ? scene_world.strength : 1.0);
				return UniformsExt.vec;
			}
			///if (is_paint || is_sculpt)
			case "_stencilTransform": {
				vec4_set(UniformsExt.vec, Context.raw.brushStencilX, Context.raw.brushStencilY, Context.raw.brushStencilScale, Context.raw.brushStencilAngle);
				if (Context.raw.paint2d) UniformsExt.vec.x = UniformsExt.vec2d(UniformsExt.vec.x);
				return UniformsExt.vec;
			}
			case "_decalMask": {
				let decal = Context.raw.tool == WorkspaceTool.ToolDecal || Context.raw.tool == WorkspaceTool.ToolText;
				let decalMask = Operator.shortcut(Config.keymap.decal_mask + "+" + Config.keymap.action_paint, ShortcutType.ShortcutDown);
				let val = (Context.raw.brushRadius * Context.raw.brushNodesRadius) / 15.0;
				let scale2d = (900 / Base.h()) * Config.raw.window_scale;
				val *= scale2d; // Projection ratio
				vec4_set(UniformsExt.vec, Context.raw.decalX, Context.raw.decalY, decalMask ? 1 : 0, val);
				if (Context.raw.paint2d) UniformsExt.vec.x = UniformsExt.vec2d(UniformsExt.vec.x);
				return UniformsExt.vec;
			}
			///end
		}
		return null;
	}

	static linkMat4 = (object: object_t, mat: material_data_t, link: string): mat4_t => {
		switch (link) {
			///if (is_paint || is_sculpt)
			case "_decalLayerMatrix": { // Decal layer
				let camera = scene_camera;
				let m = _uniforms_mat;
				mat4_set_from(m, Context.raw.layer.decalMat);
				mat4_get_inv(m, m);
				mat4_mult_mat(m, UniformsExt.orthoP);
				return m;
			}
			///end
		}
		return null;
	}

	static linkTex = (object: object_t, mat: material_data_t, link: string): image_t => {
		switch (link) {
			case "_texpaint_undo": {
				///if (is_paint || is_sculpt)
				let i = History.undoI - 1 < 0 ? Config.raw.undo_steps - 1 : History.undoI - 1;
				return render_path_render_targets.get("texpaint_undo" + i).image;
				///end

				///if is_lab
				return null;
				///end
			}
			case "_texpaint_nor_undo": {
				///if (is_paint || is_sculpt)
				let i = History.undoI - 1 < 0 ? Config.raw.undo_steps - 1 : History.undoI - 1;
				return render_path_render_targets.get("texpaint_nor_undo" + i).image;
				///end

				///if is_lab
				return null;
				///end
			}
			case "_texpaint_pack_undo": {
				///if (is_paint || is_sculpt)
				let i = History.undoI - 1 < 0 ? Config.raw.undo_steps - 1 : History.undoI - 1;
				return render_path_render_targets.get("texpaint_pack_undo" + i).image;
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
				if (Project.assets.length == 0) return render_path_render_targets.get("empty_white").image;
				else return Project.getImage(Project.assets[Context.raw.colorIdHandle.position]);
			}
			case "_textexttool": { // Opacity map for text
				return Context.raw.textToolImage;
			}
			case "_texbrushmask": {
				return Context.raw.brushMaskImage;
			}
			case "_texbrushstencil": {
				return Context.raw.brushStencilImage;
			}
			case "_texparticle": {
				return render_path_render_targets.get("texparticle").image;
			}
			///end

			///if is_paint
			case "_texuvmap": {
				if (!UtilUV.uvmapCached) {
					let _init = () => {
						UtilUV.cacheUVMap();
					}
					app_notify_on_init(_init);
				}
				return UtilUV.uvmap;
			}
			case "_textrianglemap": {
				if (!UtilUV.trianglemapCached) {
					let _init = () => {
						UtilUV.cacheTriangleMap();
					}
					app_notify_on_init(_init);
				}
				return UtilUV.trianglemap;
			}
			case "_texuvislandmap": {
				let _init = () => {
					UtilUV.cacheUVIslandMap();
				}
				app_notify_on_init(_init);
				return UtilUV.uvislandmapCached ? UtilUV.uvislandmap :render_path_render_targets.get("empty_black").image;
			}
			case "_texdilatemap": {
				return UtilUV.dilatemap;
			}
			///end
		}

		if (link.startsWith("_texpaint_pack_vert")) {
			let tid = link.substr(link.length - 1);
			return render_path_render_targets.get("texpaint_pack" + tid).image;
		}

		if (link.startsWith("_texpaint_vert")) {
			///if (is_paint || is_sculpt)
			let tid = Number(link.substr(link.length - 1));
			return tid < Project.layers.length ? Project.layers[tid].texpaint : null;
			///end

			///if is_lab
			return BrushOutputNode.inst.texpaint;
			///end
		}
		if (link.startsWith("_texpaint_nor")) {
			///if is_paint
			let tid = Number(link.substr(link.length - 1));
			return tid < Project.layers.length ? Project.layers[tid].texpaint_nor : null;
			///end

			///if is_lab
			return BrushOutputNode.inst.texpaint_nor;
			///end
		}
		if (link.startsWith("_texpaint_pack")) {
			///if is_paint
			let tid = Number(link.substr(link.length - 1));
			return tid < Project.layers.length ? Project.layers[tid].texpaint_pack : null;
			///end

			///if is_lab
			return BrushOutputNode.inst.texpaint_pack;
			///end
		}
		if (link.startsWith("_texpaint")) {
			///if (is_paint || is_sculpt)
			let tid = Number(link.substr(link.length - 1));
			return tid < Project.layers.length ? Project.layers[tid].texpaint : null;
			///end

			///if is_lab
			return BrushOutputNode.inst.texpaint;
			///end
		}

		///if (is_paint || is_sculpt)
		if (link.startsWith("_texblur_")) {
			let id = link.substr(9);
			return Context.raw.nodePreviews != null ? Context.raw.nodePreviews.get(id) :render_path_render_targets.get("empty_black").image;
		}
		if (link.startsWith("_texwarp_")) {
			let id = link.substr(9);
			return Context.raw.nodePreviews != null ? Context.raw.nodePreviews.get(id) :render_path_render_targets.get("empty_black").image;
		}
		if (link.startsWith("_texbake_")) {
			let id = link.substr(9);
			return Context.raw.nodePreviews != null ? Context.raw.nodePreviews.get(id) :render_path_render_targets.get("empty_black").image;
		}
		///end

		return null;
	}
}
