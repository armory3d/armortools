bool render_path_raytrace_bake_commands(void (*parse_paint_material)(bool)) {

	if (!render_path_raytrace_ready || !render_path_raytrace_is_bake || render_path_raytrace_bake_last_bake_type != context_raw->bake_type) {

		bool rebuild = !(render_path_raytrace_ready && render_path_raytrace_is_bake && render_path_raytrace_bake_last_bake_type != context_raw->bake_type);
		render_path_raytrace_bake_last_bake_type = context_raw->bake_type;
		render_path_raytrace_ready               = true;
		render_path_raytrace_is_bake             = true;
		gc_unroot(render_path_raytrace_last_envmap);
		render_path_raytrace_last_envmap = NULL;
		gc_unroot(render_path_raytrace_bake_last_layer);
		render_path_raytrace_bake_last_layer = NULL;

		if (any_map_get(render_path_render_targets, "baketex0") != NULL) {
			render_target_t *baketex0 = any_map_get(render_path_render_targets, "baketex0");
			render_target_t *baketex1 = any_map_get(render_path_render_targets, "baketex1");
			render_target_t *baketex2 = any_map_get(render_path_render_targets, "baketex2");
			gpu_delete_texture(baketex0->_image);
			gpu_delete_texture(baketex1->_image);
			gpu_delete_texture(baketex2->_image);
		}

		{
			render_target_t *t = render_target_create();
			t->name            = "baketex0";
			t->width           = config_get_texture_res_x();
			t->height          = config_get_texture_res_y();
			t->format          = "RGBA64";
			render_path_create_render_target(t);
		}
		{
			render_target_t *t = render_target_create();
			t->name            = "baketex1";
			t->width           = config_get_texture_res_x();
			t->height          = config_get_texture_res_y();
			t->format          = "RGBA64";
			render_path_create_render_target(t);
		}
		{
			render_target_t *t = render_target_create();
			t->name            = "baketex2";
			t->width           = config_get_texture_res_x();
			t->height          = config_get_texture_res_y();
			t->format          = "RGBA64"; // Match raytrace_target format
			render_path_create_render_target(t);
		}

		bake_type_t _bake_type = context_raw->bake_type;
		context_raw->bake_type = BAKE_TYPE_INIT;
		parse_paint_material(true);
		render_path_set_target("baketex0", NULL, NULL, GPU_CLEAR_COLOR, 0x00000000, 0.0);
		// Pixels with alpha of 0.0 are skipped during raytracing
		string_t_array_t *additional = any_array_create_from_raw(
		    (void *[]){
		        "baketex1",
		    },
		    1);
		render_path_set_target("baketex0", additional, NULL, GPU_CLEAR_NONE, 0, 0.0);
		render_path_draw_meshes("paint");
		context_raw->bake_type = _bake_type;
		sys_notify_on_next_frame(&render_path_raytrace_bake_commands_134003, parse_paint_material);

		render_path_raytrace_init_shader = true;
		render_path_raytrace_raytrace_init(render_path_raytrace_bake_get_bake_shader_name(), rebuild);

		return false;
	}

	if (!context_raw->envmap_loaded) {
		context_load_envmap();
		context_update_envmap();
	}

	world_data_t  *probe        = scene_world;
	gpu_texture_t *saved_envmap = context_raw->show_envmap_blur ? probe->_->radiance_mipmaps->buffer[0] : context_raw->saved_envmap;

	if (render_path_raytrace_last_envmap != saved_envmap || render_path_raytrace_bake_last_layer != context_raw->layer->texpaint ||
	    render_path_raytrace_bake_last_bake_type2 != context_raw->bake_type) {

		gc_unroot(render_path_raytrace_last_envmap);
		render_path_raytrace_last_envmap = saved_envmap;
		gc_root(render_path_raytrace_last_envmap);
		gc_unroot(render_path_raytrace_bake_last_layer);
		render_path_raytrace_bake_last_layer = context_raw->layer->texpaint;
		gc_root(render_path_raytrace_bake_last_layer);
		render_path_raytrace_bake_last_bake_type2 = context_raw->bake_type;

		gpu_texture_t   *bnoise_sobol             = any_map_get(scene_embedded, "bnoise_sobol.k");
		gpu_texture_t   *bnoise_scramble          = any_map_get(scene_embedded, "bnoise_scramble.k");
		gpu_texture_t   *bnoise_rank              = any_map_get(scene_embedded, "bnoise_rank.k");

		render_target_t *baketex0                 = any_map_get(render_path_render_targets, "baketex0");
		render_target_t *baketex1                 = any_map_get(render_path_render_targets, "baketex1");

		gpu_texture_t   *tex2                     = NULL;
		if (context_raw->bake_type == BAKE_TYPE_LIGHTMAP) {
			slot_layer_t *flat = layers_flatten(true, NULL);
			tex2               = flat->texpaint;
		}
		else {
			render_target_t *texpaint_undo = any_map_get(render_path_render_targets, string_join("texpaint_undo", i32_to_string(history_undo_i)));
			tex2                           = texpaint_undo->_image;
		}

		gpu_raytrace_set_textures(baketex0->_image, baketex1->_image, tex2, saved_envmap, bnoise_sobol, bnoise_scramble, bnoise_rank);
	}

	if (context_raw->brush_time > 0) {
		context_raw->pdirty = 2;
		context_raw->rdirty = 2;
	}

	if (context_raw->pdirty > 0) {
		f32_array_t *f32a            = render_path_raytrace_f32a;
		f32a->buffer[0]              = render_path_raytrace_frame++;
		f32a->buffer[1]              = context_raw->bake_ao_strength;
		f32a->buffer[2]              = context_raw->bake_ao_radius;
		f32a->buffer[3]              = context_raw->bake_ao_offset;
		f32a->buffer[4]              = scene_world->strength;
		f32a->buffer[5]              = context_raw->bake_up_axis;
		f32a->buffer[6]              = context_raw->envmap_angle;

		render_target_t *framebuffer = any_map_get(render_path_render_targets, "baketex2");
		_gpu_raytrace_dispatch_rays(framebuffer->_image, f32a);

		i32       id          = context_raw->layer->id;
		char *texpaint_id = string_join("texpaint", i32_to_string(id));
		render_path_set_target(texpaint_id, NULL, NULL, GPU_CLEAR_NONE, 0, 0.0);
		render_path_bind_target("baketex2", "tex");
		render_path_draw_shader("Scene/copy_pass/copy_pass");

		#ifdef IRON_METAL
		i32 samples_per_frame              = 4;
		#else
		i32 samples_per_frame              = 64;
		#endif

		render_path_raytrace_bake_rays_pix = render_path_raytrace_frame * samples_per_frame;
		render_path_raytrace_bake_rays_counter += samples_per_frame;
		render_path_raytrace_bake_rays_timer += sys_real_delta();
		if (render_path_raytrace_bake_rays_timer >= 1) {
			render_path_raytrace_bake_rays_sec     = render_path_raytrace_bake_rays_counter;
			render_path_raytrace_bake_rays_timer   = 0;
			render_path_raytrace_bake_rays_counter = 0;
		}
		render_path_raytrace_bake_current_sample++;
		iron_delay_idle_sleep();
		return true;
	}
	else {
		render_path_raytrace_frame               = 0;
		render_path_raytrace_bake_rays_timer     = 0;
		render_path_raytrace_bake_rays_counter   = 0;
		render_path_raytrace_bake_current_sample = 0;
		return false;
	}
}

void render_path_raytrace_bake_commands_134003(void (*parse_paint_material)(bool)) {
	parse_paint_material(true);
}

char *render_path_raytrace_bake_get_bake_shader_name() {
	return context_raw->bake_type == BAKE_TYPE_AO            ? string_join("raytrace_bake_ao", render_path_raytrace_ext)
	       : context_raw->bake_type == BAKE_TYPE_LIGHTMAP    ? string_join("raytrace_bake_light", render_path_raytrace_ext)
	       : context_raw->bake_type == BAKE_TYPE_BENT_NORMAL ? string_join("raytrace_bake_bent", render_path_raytrace_ext)
	                                                         : string_join("raytrace_bake_thick", render_path_raytrace_ext);
}
