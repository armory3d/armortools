
let render_path_raytrace_bake_rays_pix: i32 = 0;
let render_path_raytrace_bake_rays_sec: i32 = 0;
let render_path_raytrace_bake_current_sample: i32 = 0;
let render_path_raytrace_bake_rays_timer: f32 = 0.0;
let render_path_raytrace_bake_rays_counter: i32 = 0;
let render_path_raytrace_bake_last_layer: image_t = null;
let render_path_raytrace_bake_last_bake: i32 = 0;

function render_path_raytrace_bake_commands(parse_paint_material: (b?: bool)=>void): bool {

	if (!render_path_raytrace_ready || !render_path_raytrace_is_bake || render_path_raytrace_bake_last_bake != context_raw.bake_type) {
		let rebuild: bool = !(render_path_raytrace_ready && render_path_raytrace_is_bake && render_path_raytrace_bake_last_bake != context_raw.bake_type);
		render_path_raytrace_bake_last_bake = context_raw.bake_type;
		render_path_raytrace_ready = true;
		render_path_raytrace_is_bake = true;
		render_path_raytrace_last_envmap = null;
		render_path_raytrace_bake_last_layer = null;

		if (map_get(render_path_render_targets, "baketex0") != null) {
			let baketex0: render_target_t = map_get(render_path_render_targets, "baketex0");
			let baketex1: render_target_t = map_get(render_path_render_targets, "baketex1");
			let baketex2: render_target_t = map_get(render_path_render_targets, "baketex2");
			image_unload(baketex0._image);
			image_unload(baketex1._image);
			image_unload(baketex2._image);
		}

		{
			let t: render_target_t = render_target_create();
			t.name = "baketex0";
			t.width = config_get_texture_res_x();
			t.height = config_get_texture_res_y();
			t.format = "RGBA64";
			render_path_create_render_target(t);
		}
		{
			let t: render_target_t = render_target_create();
			t.name = "baketex1";
			t.width = config_get_texture_res_x();
			t.height = config_get_texture_res_y();
			t.format = "RGBA64";
			render_path_create_render_target(t);
		}
		{
			let t: render_target_t = render_target_create();
			t.name = "baketex2";
			t.width = config_get_texture_res_x();
			t.height = config_get_texture_res_y();
			t.format = "RGBA64"; // Match raytrace_target format
			render_path_create_render_target(t);
		}

		let _bake_type: bake_type_t = context_raw.bake_type;
		context_raw.bake_type = bake_type_t.INIT;
		parse_paint_material(true);
		render_path_set_target("baketex0");
		render_path_clear_target(0x00000000); // Pixels with alpha of 0.0 are skipped during raytracing
		let additional: string[] = ["baketex1"];
		render_path_set_target("baketex0", additional);
		render_path_draw_meshes("paint");
		context_raw.bake_type = _bake_type;
		app_notify_on_next_frame(parse_paint_material);

		render_path_raytrace_first = true;
		render_path_raytrace_raytrace_init(render_path_raytrace_bake_get_bake_shader_name(), rebuild, true);

		return false;
	}

	if (!context_raw.envmap_loaded) {
		context_load_envmap();
		context_update_envmap();
	}

	let probe: world_data_t = scene_world;
	let saved_envmap: image_t = context_raw.show_envmap_blur ? probe._.radiance_mipmaps[0] : context_raw.saved_envmap;

	if (render_path_raytrace_last_envmap != saved_envmap || render_path_raytrace_bake_last_layer != context_raw.layer.texpaint) {
		render_path_raytrace_last_envmap = saved_envmap;
		render_path_raytrace_bake_last_layer = context_raw.layer.texpaint;

		let bnoise_sobol: image_t = map_get(scene_embedded, "bnoise_sobol.k");
		let bnoise_scramble: image_t = map_get(scene_embedded, "bnoise_scramble.k");
		let bnoise_rank: image_t = map_get(scene_embedded, "bnoise_rank.k");

		let baketex0: render_target_t = map_get(render_path_render_targets, "baketex0");
		let baketex1: render_target_t = map_get(render_path_render_targets, "baketex1");
		let texpaint_undo: render_target_t = map_get(render_path_render_targets, "texpaint_undo" + history_undo_i);

		iron_raytrace_set_textures(baketex0._image, baketex1._image, texpaint_undo._image, saved_envmap.texture_, bnoise_sobol.texture_, bnoise_scramble.texture_, bnoise_rank.texture_);
	}

	if (context_raw.brush_time > 0) {
		context_raw.pdirty = 2;
		context_raw.rdirty = 2;
	}

	if (context_raw.pdirty > 0) {
		let f32a: f32_array_t = render_path_raytrace_f32a;
		f32a[0] = render_path_raytrace_frame++;
		f32a[1] = context_raw.bake_ao_strength;
		f32a[2] = context_raw.bake_ao_radius;
		f32a[3] = context_raw.bake_ao_offset;
		f32a[4] = scene_world.strength;
		f32a[5] = context_raw.bake_up_axis;
		f32a[6] = context_raw.envmap_angle;

		let framebuffer: render_target_t = map_get(render_path_render_targets, "baketex2");
		iron_raytrace_dispatch_rays(framebuffer._image.render_target_, f32a);

		let id: i32 = context_raw.layer.id;
		let texpaint_id: string = "texpaint" + id;
		render_path_set_target(texpaint_id);
		render_path_bind_target("baketex2", "tex");
		render_path_draw_shader("shader_datas/copy_pass/copy_pass");

		///if arm_metal
		let samples_per_frame: i32 = 4;
		///else
		let samples_per_frame: i32 = 64;
		///end

		render_path_raytrace_bake_rays_pix = render_path_raytrace_frame * samples_per_frame;
		render_path_raytrace_bake_rays_counter += samples_per_frame;
		render_path_raytrace_bake_rays_timer += time_real_delta();
		if (render_path_raytrace_bake_rays_timer >= 1) {
			render_path_raytrace_bake_rays_sec = render_path_raytrace_bake_rays_counter;
			render_path_raytrace_bake_rays_timer = 0;
			render_path_raytrace_bake_rays_counter = 0;
		}
		render_path_raytrace_bake_current_sample++;
		iron_delay_idle_sleep();
		return true;
	}
	else {
		render_path_raytrace_frame = 0;
		render_path_raytrace_bake_rays_timer = 0;
		render_path_raytrace_bake_rays_counter = 0;
		render_path_raytrace_bake_current_sample = 0;
		return false;
	}
}

function render_path_raytrace_bake_get_bake_shader_name(): string {
	return context_raw.bake_type == bake_type_t.AO  		? "raytrace_bake_ao" + render_path_raytrace_ext :
		   context_raw.bake_type == bake_type_t.LIGHTMAP 	? "raytrace_bake_light" + render_path_raytrace_ext :
		   context_raw.bake_type == bake_type_t.BENT_NORMAL ? "raytrace_bake_bent" + render_path_raytrace_ext :
															  "raytrace_bake_thick" + render_path_raytrace_ext;
}
