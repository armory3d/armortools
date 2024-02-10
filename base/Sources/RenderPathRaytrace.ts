
///if (krom_direct3d12 || krom_vulkan || krom_metal)

class RenderPathRaytrace {

	static frame = 0;
	static ready = false;
	static dirty = 0;
	static uvScale = 1.0;
	static first = true;
	static f32a = new Float32Array(24);
	static helpMat = mat4_identity();
	static vb_scale = 1.0;
	static vb: vertex_buffer_t;
	static ib: index_buffer_t;

	static lastEnvmap: image_t = null;
	static isBake = false;

	///if krom_direct3d12
	static ext = ".cso";
	///elseif krom_metal
	static ext = ".metal";
	///else
	static ext = ".spirv";
	///end

	///if is_lab
	static lastTexpaint: image_t = null;
	///end

	static init = () => {
	}

	static commands = (useLiveLayer: bool) => {
		if (!RenderPathRaytrace.ready || RenderPathRaytrace.isBake) {
			RenderPathRaytrace.ready = true;
			RenderPathRaytrace.isBake = false;
			let mode = Context.raw.pathTraceMode == PathTraceMode.TraceCore ? "core" : "full";
			RenderPathRaytrace.raytraceInit("raytrace_brute_" + mode + RenderPathRaytrace.ext);
			RenderPathRaytrace.lastEnvmap = null;
		}

		if (!Context.raw.envmapLoaded) {
			Context.loadEnvmap();
			Context.updateEnvmap();
		}

		let probe = scene_world;
		let savedEnvmap = Context.raw.showEnvmapBlur ? probe._radiance_mipmaps[0] : Context.raw.savedEnvmap;

		if (RenderPathRaytrace.lastEnvmap != savedEnvmap) {
			RenderPathRaytrace.lastEnvmap = savedEnvmap;

			let bnoise_sobol = scene_embedded.get("bnoise_sobol.k");
			let bnoise_scramble = scene_embedded.get("bnoise_scramble.k");
			let bnoise_rank = scene_embedded.get("bnoise_rank.k");

			let l = Base.flatten(true);
			krom_raytrace_set_textures(l.texpaint, l.texpaint_nor, l.texpaint_pack, savedEnvmap.texture_, bnoise_sobol.texture_, bnoise_scramble.texture_, bnoise_rank.texture_);
		}

		///if is_lab
		let l = Base.flatten(true);
		if (l.texpaint != RenderPathRaytrace.lastTexpaint) {
			RenderPathRaytrace.lastTexpaint = l.texpaint;

			let bnoise_sobol = scene_embedded.get("bnoise_sobol.k");
			let bnoise_scramble = scene_embedded.get("bnoise_scramble.k");
			let bnoise_rank = scene_embedded.get("bnoise_rank.k");

			krom_raytrace_set_textures(l.texpaint, l.texpaint_nor, l.texpaint_pack, savedEnvmap.texture_, bnoise_sobol.texture_, bnoise_scramble.texture_, bnoise_rank.texture_);
		}
		///end

		if (Context.raw.pdirty > 0 || RenderPathRaytrace.dirty > 0) {
			Base.flatten(true);
		}

		let cam = scene_camera;
		let ct = cam.base.transform;
		mat4_set_from(RenderPathRaytrace.helpMat, cam.v);
		mat4_mult_mat(RenderPathRaytrace.helpMat, cam.p);
		mat4_get_inv(RenderPathRaytrace.helpMat, RenderPathRaytrace.helpMat);
		RenderPathRaytrace.f32a[0] = transform_world_x(ct);
		RenderPathRaytrace.f32a[1] = transform_world_y(ct);
		RenderPathRaytrace.f32a[2] = transform_world_z(ct);
		RenderPathRaytrace.f32a[3] = RenderPathRaytrace.frame;
		///if krom_metal
		// frame = (frame % (16)) + 1; // _PAINT
		RenderPathRaytrace.frame = RenderPathRaytrace.frame + 1; // _RENDER
		///else
		RenderPathRaytrace.frame = (RenderPathRaytrace.frame % 4) + 1; // _PAINT
		// frame = frame + 1; // _RENDER
		///end
		RenderPathRaytrace.f32a[4] = RenderPathRaytrace.helpMat.m[0];
		RenderPathRaytrace.f32a[5] = RenderPathRaytrace.helpMat.m[1];
		RenderPathRaytrace.f32a[6] = RenderPathRaytrace.helpMat.m[2];
		RenderPathRaytrace.f32a[7] = RenderPathRaytrace.helpMat.m[3];
		RenderPathRaytrace.f32a[8] = RenderPathRaytrace.helpMat.m[4];
		RenderPathRaytrace.f32a[9] = RenderPathRaytrace.helpMat.m[5];
		RenderPathRaytrace.f32a[10] = RenderPathRaytrace.helpMat.m[6];
		RenderPathRaytrace.f32a[11] = RenderPathRaytrace.helpMat.m[7];
		RenderPathRaytrace.f32a[12] = RenderPathRaytrace.helpMat.m[8];
		RenderPathRaytrace.f32a[13] = RenderPathRaytrace.helpMat.m[9];
		RenderPathRaytrace.f32a[14] = RenderPathRaytrace.helpMat.m[10];
		RenderPathRaytrace.f32a[15] = RenderPathRaytrace.helpMat.m[11];
		RenderPathRaytrace.f32a[16] = RenderPathRaytrace.helpMat.m[12];
		RenderPathRaytrace.f32a[17] = RenderPathRaytrace.helpMat.m[13];
		RenderPathRaytrace.f32a[18] = RenderPathRaytrace.helpMat.m[14];
		RenderPathRaytrace.f32a[19] = RenderPathRaytrace.helpMat.m[15];
		RenderPathRaytrace.f32a[20] = scene_world.strength * 1.5;
		if (!Context.raw.showEnvmap) RenderPathRaytrace.f32a[20] = -RenderPathRaytrace.f32a[20];
		RenderPathRaytrace.f32a[21] = Context.raw.envmapAngle;
		RenderPathRaytrace.f32a[22] = RenderPathRaytrace.uvScale;
		///if is_lab
		RenderPathRaytrace.f32a[22] *= scene_meshes[0].data.scale_tex;
		///end

		let framebuffer = render_path_render_targets.get("buf").image;
		krom_raytrace_dispatch_rays(framebuffer.render_target_, RenderPathRaytrace.f32a.buffer);

		if (Context.raw.ddirty == 1 || Context.raw.pdirty == 1) {
			///if krom_metal
			Context.raw.rdirty = 128;
			///else
			Context.raw.rdirty = 4;
			///end
		}
		Context.raw.ddirty--;
		Context.raw.pdirty--;
		Context.raw.rdirty--;

		// Context.raw.ddirty = 1; // _RENDER
	}

	static raytraceInit = (shaderName: string, build = true) => {
		if (RenderPathRaytrace.first) {
			RenderPathRaytrace.first = false;
			scene_embed_data("bnoise_sobol.k", () => {});
			scene_embed_data("bnoise_scramble.k", () => {});
			scene_embed_data("bnoise_rank.k", () => {});
		}

		data_get_blob(shaderName, (shader: ArrayBuffer) => {
			if (build) RenderPathRaytrace.buildData();
			let bnoise_sobol = scene_embedded.get("bnoise_sobol.k");
			let bnoise_scramble = scene_embedded.get("bnoise_scramble.k");
			let bnoise_rank = scene_embedded.get("bnoise_rank.k");
			krom_raytrace_init(shader, RenderPathRaytrace.vb.buffer_, RenderPathRaytrace.ib.buffer_, RenderPathRaytrace.vb_scale);
		});
	}

	static buildData = () => {
		if (Context.raw.mergedObject == null) UtilMesh.mergeMesh();
		///if is_paint
		let mo = !Context.layerFilterUsed() ? Context.raw.mergedObject : Context.raw.paintObject;
		///else
		let mo = scene_meshes[0];
		///end
		let md = mo.data;
		let mo_scale = mo.base.transform.scale.x; // Uniform scale only
		RenderPathRaytrace.vb_scale = md.scale_pos * mo_scale;
		if (mo.base.parent != null) RenderPathRaytrace.vb_scale *= mo.base.parent.transform.scale.x;
		RenderPathRaytrace.vb = md._vertex_buffer;
		RenderPathRaytrace.ib = md._index_buffers[0];
	}

	static draw = (useLiveLayer: bool) => {
		let isLive = Config.raw.brush_live && RenderPathPaint.liveLayerDrawn > 0;
		if (Context.raw.ddirty > 1 || Context.raw.pdirty > 0 || isLive) RenderPathRaytrace.frame = 0;

		///if krom_metal
		// Delay path tracing additional samples while painting
		let down = mouse_down() || pen_down();
		if (Context.inViewport() && down) RenderPathRaytrace.frame = 0;
		///end

		RenderPathRaytrace.commands(useLiveLayer);

		if (Config.raw.rp_bloom != false) {
			RenderPathBase.drawBloom("buf");
		}
		render_path_set_target("buf");
		render_path_draw_meshes("overlay");
		render_path_set_target("buf");
		RenderPathBase.drawCompass();
		render_path_set_target("taa");
		render_path_bind_target("buf", "tex");
		render_path_draw_shader("shader_datas/compositor_pass/compositor_pass");
		render_path_set_target("");
		render_path_bind_target("taa", "tex");
		render_path_draw_shader("shader_datas/copy_pass/copy_pass");
		if (Config.raw.brush_3d) {
			RenderPathPaint.commandsCursor();
		}
	}
}

///end
