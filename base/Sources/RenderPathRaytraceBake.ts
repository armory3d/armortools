
///if (krom_direct3d12 || krom_vulkan || krom_metal)

class RenderPathRaytraceBake {

	static raysPix = 0;
	static raysSec = 0;
	static currentSample = 0;
	static raysTimer = 0.0;
	static raysCounter = 0;
	static lastLayer: image_t = null;
	static lastBake = 0;

	static commands = (parsePaintMaterial: (b?: bool)=>void): bool => {

		if (!RenderPathRaytrace.ready || !RenderPathRaytrace.isBake || RenderPathRaytraceBake.lastBake != Context.raw.bakeType) {
			let rebuild = !(RenderPathRaytrace.ready && RenderPathRaytrace.isBake && RenderPathRaytraceBake.lastBake != Context.raw.bakeType);
			RenderPathRaytraceBake.lastBake = Context.raw.bakeType;
			RenderPathRaytrace.ready = true;
			RenderPathRaytrace.isBake = true;
			RenderPathRaytrace.lastEnvmap = null;
			RenderPathRaytraceBake.lastLayer = null;

			if (render_path_render_targets.get("baketex0") != null) {
				image_unload(render_path_render_targets.get("baketex0").image);
				image_unload(render_path_render_targets.get("baketex1").image);
				image_unload(render_path_render_targets.get("baketex2").image);
			}

			{
				let t = render_target_create();
				t.name = "baketex0";
				t.width = Config.getTextureResX();
				t.height = Config.getTextureResY();
				t.format = "RGBA64";
				render_path_create_render_target(t);
			}
			{
				let t = render_target_create();
				t.name = "baketex1";
				t.width = Config.getTextureResX();
				t.height = Config.getTextureResY();
				t.format = "RGBA64";
				render_path_create_render_target(t);
			}
			{
				let t = render_target_create();
				t.name = "baketex2";
				t.width = Config.getTextureResX();
				t.height = Config.getTextureResY();
				t.format = "RGBA64"; // Match raytrace_target format
				render_path_create_render_target(t);
			}

			let _bakeType = Context.raw.bakeType;
			Context.raw.bakeType = BakeType.BakeInit;
			parsePaintMaterial();
			render_path_set_target("baketex0");
			render_path_clear_target(0x00000000); // Pixels with alpha of 0.0 are skipped during raytracing
			render_path_set_target("baketex0", ["baketex1"]);
			render_path_draw_meshes("paint");
			Context.raw.bakeType = _bakeType;
			let _next = () => {
				parsePaintMaterial();
			}
			Base.notifyOnNextFrame(_next);

			RenderPathRaytrace.raytraceInit(RenderPathRaytraceBake.getBakeShaderName(), rebuild);

			return false;
		}

		if (!Context.raw.envmapLoaded) {
			Context.loadEnvmap();
			Context.updateEnvmap();
		}
		let probe = scene_world;
		let savedEnvmap = Context.raw.showEnvmapBlur ? probe._radiance_mipmaps[0] : Context.raw.savedEnvmap;
		if (RenderPathRaytrace.lastEnvmap != savedEnvmap || RenderPathRaytraceBake.lastLayer != Context.raw.layer.texpaint) {
			RenderPathRaytrace.lastEnvmap = savedEnvmap;
			RenderPathRaytraceBake.lastLayer = Context.raw.layer.texpaint;

			let baketex0 = render_path_render_targets.get("baketex0").image;
			let baketex1 = render_path_render_targets.get("baketex1").image;
			let bnoise_sobol = scene_embedded.get("bnoise_sobol.k");
			let bnoise_scramble = scene_embedded.get("bnoise_scramble.k");
			let bnoise_rank = scene_embedded.get("bnoise_rank.k");
			let texpaint_undo = render_path_render_targets.get("texpaint_undo" + History.undoI).image;
			krom_raytrace_set_textures(baketex0, baketex1, texpaint_undo, savedEnvmap.texture_, bnoise_sobol.texture_, bnoise_scramble.texture_, bnoise_rank.texture_);
		}

		if (Context.raw.brushTime > 0) {
			Context.raw.pdirty = 2;
			Context.raw.rdirty = 2;
		}

		if (Context.raw.pdirty > 0) {
			let f32a = RenderPathRaytrace.f32a;
			f32a[0] = RenderPathRaytrace.frame++;
			f32a[1] = Context.raw.bakeAoStrength;
			f32a[2] = Context.raw.bakeAoRadius;
			f32a[3] = Context.raw.bakeAoOffset;
			f32a[4] = scene_world.strength;
			f32a[5] = Context.raw.bakeUpAxis;
			f32a[6] = Context.raw.envmapAngle;

			let framebuffer = render_path_render_targets.get("baketex2").image;
			krom_raytrace_dispatch_rays(framebuffer.render_target_, f32a.buffer);

			render_path_set_target("texpaint" + Context.raw.layer.id);
			render_path_bind_target("baketex2", "tex");
			render_path_draw_shader("shader_datas/copy_pass/copy_pass");

			///if krom_metal
			let samplesPerFrame = 4;
			///else
			let samplesPerFrame = 64;
			///end

			RenderPathRaytraceBake.raysPix = RenderPathRaytrace.frame * samplesPerFrame;
			RenderPathRaytraceBake.raysCounter += samplesPerFrame;
			RenderPathRaytraceBake.raysTimer += time_real_delta();
			if (RenderPathRaytraceBake.raysTimer >= 1) {
				RenderPathRaytraceBake.raysSec = RenderPathRaytraceBake.raysCounter;
				RenderPathRaytraceBake.raysTimer = 0;
				RenderPathRaytraceBake.raysCounter = 0;
			}
			RenderPathRaytraceBake.currentSample++;
			krom_delay_idle_sleep();
			return true;
		}
		else {
			RenderPathRaytrace.frame = 0;
			RenderPathRaytraceBake.raysTimer = 0;
			RenderPathRaytraceBake.raysCounter = 0;
			RenderPathRaytraceBake.currentSample = 0;
			return false;
		}
	}

	static getBakeShaderName = (): string => {
		return Context.raw.bakeType == BakeType.BakeAO  		? "raytrace_bake_ao" + RenderPathRaytrace.ext :
			   Context.raw.bakeType == BakeType.BakeLightmap 	? "raytrace_bake_light" + RenderPathRaytrace.ext :
			   Context.raw.bakeType == BakeType.BakeBentNormal  ? "raytrace_bake_bent" + RenderPathRaytrace.ext :
												  				  "raytrace_bake_thick" + RenderPathRaytrace.ext;
	}
}

///end
