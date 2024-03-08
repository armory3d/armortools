
///if (krom_direct3d12 || krom_vulkan || krom_metal)

class RenderPathRaytraceBake {

	static rays_pix: i32 = 0;
	static rays_sec: i32 = 0;
	static current_sample: i32 = 0;
	static rays_timer: f32 = 0.0;
	static rays_counter: i32 = 0;
	static last_layer: image_t = null;
	static last_bake: i32 = 0;

	static commands = (parsePaintMaterial: (b?: bool)=>void): bool => {

		if (!RenderPathRaytrace.ready || !RenderPathRaytrace.is_bake || RenderPathRaytraceBake.last_bake != Context.raw.bake_type) {
			let rebuild: bool = !(RenderPathRaytrace.ready && RenderPathRaytrace.is_bake && RenderPathRaytraceBake.last_bake != Context.raw.bake_type);
			RenderPathRaytraceBake.last_bake = Context.raw.bake_type;
			RenderPathRaytrace.ready = true;
			RenderPathRaytrace.is_bake = true;
			RenderPathRaytrace.last_envmap = null;
			RenderPathRaytraceBake.last_layer = null;

			if (render_path_render_targets.get("baketex0") != null) {
				image_unload(render_path_render_targets.get("baketex0")._image);
				image_unload(render_path_render_targets.get("baketex1")._image);
				image_unload(render_path_render_targets.get("baketex2")._image);
			}

			{
				let t: render_target_t = render_target_create();
				t.name = "baketex0";
				t.width = Config.get_texture_res_x();
				t.height = Config.get_texture_res_y();
				t.format = "RGBA64";
				render_path_create_render_target(t);
			}
			{
				let t: render_target_t = render_target_create();
				t.name = "baketex1";
				t.width = Config.get_texture_res_x();
				t.height = Config.get_texture_res_y();
				t.format = "RGBA64";
				render_path_create_render_target(t);
			}
			{
				let t: render_target_t = render_target_create();
				t.name = "baketex2";
				t.width = Config.get_texture_res_x();
				t.height = Config.get_texture_res_y();
				t.format = "RGBA64"; // Match raytrace_target format
				render_path_create_render_target(t);
			}

			let _bake_type: bake_type_t = Context.raw.bake_type;
			Context.raw.bake_type = bake_type_t.INIT;
			parsePaintMaterial();
			render_path_set_target("baketex0");
			render_path_clear_target(0x00000000); // Pixels with alpha of 0.0 are skipped during raytracing
			render_path_set_target("baketex0", ["baketex1"]);
			render_path_draw_meshes("paint");
			Context.raw.bake_type = _bake_type;
			let _next = () => {
				parsePaintMaterial();
			}
			base_notify_on_next_frame(_next);

			RenderPathRaytrace.raytrace_init(RenderPathRaytraceBake.get_bake_shader_name(), rebuild);

			return false;
		}

		if (!Context.raw.envmap_loaded) {
			Context.load_envmap();
			Context.update_envmap();
		}
		let probe: world_data_t = scene_world;
		let saved_envmap: image_t = Context.raw.show_envmap_blur ? probe._.radiance_mipmaps[0] : Context.raw.saved_envmap;
		if (RenderPathRaytrace.last_envmap != saved_envmap || RenderPathRaytraceBake.last_layer != Context.raw.layer.texpaint) {
			RenderPathRaytrace.last_envmap = saved_envmap;
			RenderPathRaytraceBake.last_layer = Context.raw.layer.texpaint;

			let baketex0: image_t = render_path_render_targets.get("baketex0")._image;
			let baketex1: image_t = render_path_render_targets.get("baketex1")._image;
			let bnoise_sobol: image_t = scene_embedded.get("bnoise_sobol.k");
			let bnoise_scramble: image_t = scene_embedded.get("bnoise_scramble.k");
			let bnoise_rank: image_t = scene_embedded.get("bnoise_rank.k");
			let texpaint_undo: image_t = render_path_render_targets.get("texpaint_undo" + History.undo_i)._image;
			krom_raytrace_set_textures(baketex0, baketex1, texpaint_undo, saved_envmap.texture_, bnoise_sobol.texture_, bnoise_scramble.texture_, bnoise_rank.texture_);
		}

		if (Context.raw.brush_time > 0) {
			Context.raw.pdirty = 2;
			Context.raw.rdirty = 2;
		}

		if (Context.raw.pdirty > 0) {
			let f32a: Float32Array = RenderPathRaytrace.f32a;
			f32a[0] = RenderPathRaytrace.frame++;
			f32a[1] = Context.raw.bake_ao_strength;
			f32a[2] = Context.raw.bake_ao_radius;
			f32a[3] = Context.raw.bake_ao_offset;
			f32a[4] = scene_world.strength;
			f32a[5] = Context.raw.bake_up_axis;
			f32a[6] = Context.raw.envmap_angle;

			let framebuffer: image_t = render_path_render_targets.get("baketex2")._image;
			krom_raytrace_dispatch_rays(framebuffer.render_target_, f32a.buffer);

			render_path_set_target("texpaint" + Context.raw.layer.id);
			render_path_bind_target("baketex2", "tex");
			render_path_draw_shader("shader_datas/copy_pass/copy_pass");

			///if krom_metal
			let samples_per_frame: i32 = 4;
			///else
			let samples_per_frame: i32 = 64;
			///end

			RenderPathRaytraceBake.rays_pix = RenderPathRaytrace.frame * samples_per_frame;
			RenderPathRaytraceBake.rays_counter += samples_per_frame;
			RenderPathRaytraceBake.rays_timer += time_real_delta();
			if (RenderPathRaytraceBake.rays_timer >= 1) {
				RenderPathRaytraceBake.rays_sec = RenderPathRaytraceBake.rays_counter;
				RenderPathRaytraceBake.rays_timer = 0;
				RenderPathRaytraceBake.rays_counter = 0;
			}
			RenderPathRaytraceBake.current_sample++;
			krom_delay_idle_sleep();
			return true;
		}
		else {
			RenderPathRaytrace.frame = 0;
			RenderPathRaytraceBake.rays_timer = 0;
			RenderPathRaytraceBake.rays_counter = 0;
			RenderPathRaytraceBake.current_sample = 0;
			return false;
		}
	}

	static get_bake_shader_name = (): string => {
		return Context.raw.bake_type == bake_type_t.AO  		? "raytrace_bake_ao" + RenderPathRaytrace.ext :
			   Context.raw.bake_type == bake_type_t.LIGHTMAP 	? "raytrace_bake_light" + RenderPathRaytrace.ext :
			   Context.raw.bake_type == bake_type_t.BENT_NORMAL  ? "raytrace_bake_bent" + RenderPathRaytrace.ext :
												  				  "raytrace_bake_thick" + RenderPathRaytrace.ext;
	}
}

///end
