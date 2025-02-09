
let render_path_base_taa_frame: i32 = 0;
let render_path_base_super_sample: f32 = 1.0;
let render_path_base_last_x: f32 = -1.0;
let render_path_base_last_y: f32 = -1.0;
let render_path_base_bloom_mipmaps: render_target_t[];
let render_path_base_bloom_current_mip: i32 = 0;
let render_path_base_bloom_sample_scale: f32;

function render_path_base_init() {
	pipes_init();
	const_data_create_screen_aligned_data();
	render_path_base_super_sample = config_raw.rp_supersample;
}

function render_path_base_apply_config() {
	if (render_path_base_super_sample != config_raw.rp_supersample) {
		render_path_base_super_sample = config_raw.rp_supersample;
		let keys: string[] = map_keys(render_path_render_targets);
		for (let i: i32 = 0; i < keys.length; ++i) {
			let rt: render_target_t = map_get(render_path_render_targets, keys[i]);
			if (rt.width == 0) {
				rt.scale = render_path_base_super_sample;
			}
		}
		render_path_resize();
	}
}

function render_path_base_get_super_sampling(): f32 {
	return render_path_base_super_sample;
}

function render_path_base_draw_compass() {
	compass_render();
}

function render_path_base_begin() {
	// Begin split
	if (context_raw.split_view && !context_raw.paint2d_view) {
		if (context_raw.view_index_last == -1 && context_raw.view_index == -1) {
			// Begin split, draw right viewport first
			context_raw.view_index = 1;
		}
		else {
			// Set current viewport
			context_raw.view_index = mouse_view_x() > base_w() / 2 ? 1 : 0;
		}

		let cam: camera_object_t = scene_camera;
		if (context_raw.view_index_last > -1) {
			// Save current viewport camera
			camera_views[context_raw.view_index_last].v = mat4_clone(cam.base.transform.local);
		}

		let decal: bool = context_is_decal();

		if (context_raw.view_index_last != context_raw.view_index || decal || !config_raw.brush_3d) {
			// Redraw on current viewport change
			context_raw.ddirty = 1;
		}

		transform_set_matrix(cam.base.transform, camera_views[context_raw.view_index].v);
		camera_object_build_mat(cam);
		camera_object_build_proj(cam);
	}

	// Match projection matrix jitter
	let skip_taa: bool =
		context_raw.split_view ||
		context_raw.viewport_mode == viewport_mode_t.PATH_TRACE ||
		((context_raw.tool == workspace_tool_t.CLONE ||
			context_raw.tool == workspace_tool_t.BLUR ||
			context_raw.tool == workspace_tool_t.SMUDGE) && context_raw.pdirty > 0);
	scene_camera.frame = skip_taa ? 0 : render_path_base_taa_frame;
	camera_object_proj_jitter(scene_camera);
	camera_object_build_mat(scene_camera);
}

function render_path_base_end() {
	// End split
	context_raw.view_index_last = context_raw.view_index;
	context_raw.view_index = -1;

	if (context_raw.foreground_event && !mouse_down()) {
		context_raw.foreground_event = false;
		context_raw.pdirty = 0;
	}

	render_path_base_taa_frame++;
}

function render_path_base_ssaa4(): bool {
	return config_raw.rp_supersample == 4;
}

function render_path_base_is_cached(): bool {
	if (sys_width() == 0 || sys_height() == 0) {
		return true;
	}

	let mx: f32 = render_path_base_last_x;
	let my: f32 = render_path_base_last_y;
	render_path_base_last_x = mouse_view_x();
	render_path_base_last_y = mouse_view_y();

	if (context_raw.ddirty <= 0 && context_raw.rdirty <= 0 && context_raw.pdirty <= 0) {
		if (mx != render_path_base_last_x || my != render_path_base_last_y || mouse_locked) {
			context_raw.ddirty = 0;
		}
		///if (arm_metal || arm_android)
		if (context_raw.ddirty > -6) {
		///else
		if (context_raw.ddirty > -2) {
		///end
			render_path_set_target("");
			render_path_bind_target("taa", "tex");
			render_path_base_ssaa4() ?
				render_path_draw_shader("shader_datas/supersample_resolve/supersample_resolve") :
				render_path_draw_shader("shader_datas/copy_pass/copy_pass");
			render_path_paint_commands_cursor();
			if (context_raw.ddirty <= 0) {
				context_raw.ddirty--;
			}
		}
		render_path_base_end();
		return true;
	}
	return false;
}

function render_path_base_commands(draw_commands: ()=>void) {
	if (render_path_base_is_cached()) {
		return;
	}
	render_path_base_begin();

	render_path_paint_begin();
	render_path_base_draw_split(draw_commands);
	render_path_base_draw_gbuffer();
	render_path_paint_draw();

	if (context_raw.viewport_mode == viewport_mode_t.PATH_TRACE) {
		let use_live_layer: bool = context_raw.tool == workspace_tool_t.MATERIAL;
		render_path_raytrace_draw(use_live_layer);
		context_raw.foreground_event = false; // render_path_paint_end();
		render_path_base_end();
		return;
	}

	draw_commands();
	render_path_paint_end();
	render_path_base_end();
}

function render_path_base_draw_bloom(tex: string) {
	if (config_raw.rp_bloom == false) {
		return;
	}

	if (render_path_base_bloom_mipmaps == null) {
		render_path_base_bloom_mipmaps = [];

		let prev_scale: f32 = 1.0;
		for (let i: i32 = 0; i < 10; ++i) {
			let t: render_target_t = render_target_create();
			t.name = "bloom_mip_" + i;
			t.width = 0;
			t.height = 0;
			prev_scale *= 0.5;
			t.scale = prev_scale;
			t.format = "RGBA64";
			array_push(render_path_base_bloom_mipmaps, render_path_create_render_target(t));
		}

		render_path_load_shader("shader_datas/bloom_pass/bloom_downsample_pass");
		render_path_load_shader("shader_datas/bloom_pass/bloom_upsample_pass");
	}

	let bloom_radius: f32 = 6.5;
	let min_dim: f32 = math_min(render_path_current_w, render_path_current_h);
	let log_min_dim: f32 = math_max(1.0, math_log2(min_dim) + (bloom_radius - 8.0));
	let num_mips: i32 = math_floor(log_min_dim);
	render_path_base_bloom_sample_scale = 0.5 + log_min_dim - num_mips;

	for (let i: i32 = 0; i < num_mips; ++i) {
		render_path_base_bloom_current_mip = i;
		render_path_set_target(render_path_base_bloom_mipmaps[i].name);
		render_path_clear_target(0x00000000);
		render_path_bind_target(i == 0 ? tex : render_path_base_bloom_mipmaps[i - 1].name, "tex");
		render_path_draw_shader("shader_datas/bloom_pass/bloom_downsample_pass");
	}
	for (let i: i32 = 0; i < num_mips; ++i) {
		let mip_level: i32 = num_mips - 1 - i;
		render_path_base_bloom_current_mip = mip_level;
		render_path_set_target(mip_level == 0 ? tex : render_path_base_bloom_mipmaps[mip_level - 1].name);
		render_path_bind_target(render_path_base_bloom_mipmaps[mip_level].name, "tex");
		render_path_draw_shader("shader_datas/bloom_pass/bloom_upsample_pass");
	}
}

function render_path_base_draw_split(draw_commands: ()=>void) {
	if (context_raw.split_view && !context_raw.paint2d_view) {
		context_raw.ddirty = 2;
		let cam: camera_object_t = scene_camera;

		context_raw.view_index = context_raw.view_index == 0 ? 1 : 0;
		transform_set_matrix(cam.base.transform, camera_views[context_raw.view_index].v);
		camera_object_build_mat(cam);
		camera_object_build_proj(cam);

		render_path_base_draw_gbuffer();

		let use_live_layer: bool = context_raw.tool == workspace_tool_t.MATERIAL;
		context_raw.viewport_mode == viewport_mode_t.PATH_TRACE ?
			render_path_raytrace_draw(use_live_layer) :
			draw_commands();

		context_raw.view_index = context_raw.view_index == 0 ? 1 : 0;
		transform_set_matrix(cam.base.transform, camera_views[context_raw.view_index].v);
		camera_object_build_mat(cam);
		camera_object_build_proj(cam);
	}
}

function render_path_base_init_ssao() {
	{
		let t: render_target_t = render_target_create();
		t.name = "singlea";
		t.width = 0;
		t.height = 0;
		t.format = "R8";
		t.scale = render_path_base_get_super_sampling();
		render_path_create_render_target(t);
	}
	{
		let t: render_target_t = render_target_create();
		t.name = "singleb";
		t.width = 0;
		t.height = 0;
		t.format = "R8";
		t.scale = render_path_base_get_super_sampling();
		render_path_create_render_target(t);
	}
	render_path_load_shader("shader_datas/ssao_pass/ssao_pass");
	render_path_load_shader("shader_datas/ssao_blur_pass/ssao_blur_pass_x");
	render_path_load_shader("shader_datas/ssao_blur_pass/ssao_blur_pass_y");
}

function render_path_base_draw_ssao() {
	let ssao: bool = config_raw.rp_ssao != false && context_raw.camera_type == camera_type_t.PERSPECTIVE;
	if (ssao && context_raw.ddirty > 0 && render_path_base_taa_frame > 0) {
		if (map_get(render_path_render_targets, "singlea") == null) {
			render_path_base_init_ssao();
		}

		render_path_set_target("singlea");
		render_path_bind_target("_main", "gbufferD");
		render_path_bind_target("gbuffer0", "gbuffer0");
		render_path_draw_shader("shader_datas/ssao_pass/ssao_pass");

		render_path_set_target("singleb");
		render_path_bind_target("singlea", "tex");
		render_path_bind_target("gbuffer0", "gbuffer0");
		render_path_draw_shader("shader_datas/ssao_blur_pass/ssao_blur_pass_x");

		render_path_set_target("singlea");
		render_path_bind_target("singleb", "tex");
		render_path_bind_target("gbuffer0", "gbuffer0");
		render_path_draw_shader("shader_datas/ssao_blur_pass/ssao_blur_pass_y");
	}
}

function render_path_base_draw_deferred_light() {
	render_path_set_target("tex");
	render_path_bind_target("_main", "gbufferD");
	render_path_bind_target("gbuffer0", "gbuffer0");
	render_path_bind_target("gbuffer1", "gbuffer1");
	let ssao: bool = config_raw.rp_ssao != false && context_raw.camera_type == camera_type_t.PERSPECTIVE;
	if (ssao && render_path_base_taa_frame > 0) {
		render_path_bind_target("singlea", "ssaotex");
	}
	else {
		render_path_bind_target("empty_white", "ssaotex");
	}


	render_path_draw_shader("shader_datas/deferred_light/deferred_light");

	render_path_set_depth_from("tex", "gbuffer0"); // Bind depth for world pass

	render_path_set_target("tex");
	render_path_draw_skydome("shader_datas/world_pass/world_pass");

	render_path_set_depth_from("tex", "gbuffer1"); // Unbind depth
}

// function render_path_base_draw_motion_blur() {
// 	if (config_raw.rp_motionblur != false) {
// 		render_path_set_target("buf");
// 		render_path_bind_target("tex", "tex");
// 		render_path_bind_target("gbuffer0", "gbuffer0");
// 		///if (rp_motionblur == "Camera")
// 		{
// 			render_path_bind_target("_main", "gbufferD");
// 			render_path_draw_shader("shader_datas/motion_blur_pass/motion_blur_pass");
// 		}
// 		///else
// 		{
// 			render_path_bind_target("gbuffer2", "sveloc");
// 			render_path_draw_shader("shader_datas/motion_blur_veloc_pass/motion_blur_veloc_pass");
// 		}
// 		///end
// 		render_path_set_target("tex");
// 		render_path_bind_target("buf", "tex");
// 		render_path_draw_shader("shader_datas/copy_pass/copy_pass");
// 	}
// }

// function render_path_base_draw_histogram() {
// 	{
// 		let t: render_target_t = RenderTarget.create();
// 		t.name = "histogram";
// 		t.width = 1;
// 		t.height = 1;
// 		t.format = "RGBA64";
// 		render_path_create_render_target(t);

// 		render_path_load_shader("shader_datas/histogram_pass/histogram_pass");
// 	}

// 	render_path_set_target("histogram");
// 	render_path_bind_target("taa", "tex");
// 	render_path_draw_shader("shader_datas/histogram_pass/histogram_pass");
// }

function render_path_base_draw_taa() {
	let current: string = render_path_base_taa_frame % 2 == 0 ? "buf2" : "taa2";
	let last: string = render_path_base_taa_frame % 2 == 0 ? "taa2" : "buf2";

	render_path_set_target(current);
	render_path_clear_target(0x00000000);
	render_path_bind_target("buf", "color_tex");
	render_path_draw_shader("shader_datas/smaa_edge_detect/smaa_edge_detect");

	render_path_set_target("taa");
	render_path_clear_target(0x00000000);
	render_path_bind_target(current, "edges_tex");
	render_path_draw_shader("shader_datas/smaa_blend_weight/smaa_blend_weight");

	render_path_set_target(current);
	render_path_bind_target("buf", "color_tex");
	render_path_bind_target("taa", "blend_tex");
	render_path_bind_target("gbuffer2", "sveloc");
	render_path_draw_shader("shader_datas/smaa_neighborhood_blend/smaa_neighborhood_blend");

	let skip_taa: bool = context_raw.split_view;
	if (skip_taa) {
		render_path_set_target("taa");
		render_path_bind_target(current, "tex");
		render_path_draw_shader("shader_datas/copy_pass/copy_pass");
	}
	else {
		render_path_set_target("taa");
		render_path_bind_target(current, "tex");
		render_path_bind_target(last, "tex2");
		render_path_bind_target("gbuffer2", "sveloc");
		render_path_draw_shader("shader_datas/taa_pass/taa_pass");
	}

	if (render_path_base_ssaa4()) {
		render_path_set_target("");
		render_path_bind_target(render_path_base_taa_frame % 2 == 0 ? "taa2" : "taa", "tex");
		render_path_draw_shader("shader_datas/supersample_resolve/supersample_resolve");
	}
	else {
		render_path_set_target("");
		render_path_bind_target(render_path_base_taa_frame == 0 ? current : "taa", "tex");
		render_path_draw_shader("shader_datas/copy_pass/copy_pass");
	}
}

function render_path_base_draw_gbuffer() {
	render_path_set_target("gbuffer0"); // Only clear gbuffer0
	///if arm_metal
	render_path_clear_target(0x00000000, 1.0, clear_flag_t.COLOR | clear_flag_t.DEPTH);
	///else
	render_path_clear_target(0, 1.0, clear_flag_t.DEPTH);
	///end
	if (make_mesh_layer_pass_count == 1) {
		render_path_set_target("gbuffer2");
		render_path_clear_target(0xff000000);
	}
	let additional: string[] = ["gbuffer1", "gbuffer2"];
	render_path_set_target("gbuffer0", additional);
	render_path_paint_bind_layers();
	render_path_draw_meshes("mesh");
	render_path_paint_unbind_layers();
	if (make_mesh_layer_pass_count > 1) {
		render_path_base_make_gbuffer_copy_textures();
		for (let i: i32 = 1; i < make_mesh_layer_pass_count; ++i) {
			let ping: string = i % 2 == 1 ? "_copy" : "";
			let pong: string = i % 2 == 1 ? "" : "_copy";
			if (i == make_mesh_layer_pass_count - 1) {
				render_path_set_target("gbuffer2" + ping);
				render_path_clear_target(0xff000000);
			}
			let g1ping: string = "gbuffer1" + ping;
			let g2ping: string = "gbuffer2" + ping;
			let additional: string[] = [g1ping, g2ping];
			render_path_set_target("gbuffer0" + ping, additional);
			render_path_bind_target("gbuffer0" + pong, "gbuffer0");
			render_path_bind_target("gbuffer1" + pong, "gbuffer1");
			render_path_bind_target("gbuffer2" + pong, "gbuffer2");
			render_path_paint_bind_layers();
			render_path_draw_meshes("mesh" + i);
			render_path_paint_unbind_layers();
		}
		if (make_mesh_layer_pass_count % 2 == 0) {
			render_path_base_copy_to_gbuffer();
		}
	}

	let hide: bool = operator_shortcut(map_get(config_keymap, "stencil_hide"), shortcut_type_t.DOWN) || keyboard_down("control");
	let is_decal: bool = base_is_decal_layer();
	if (is_decal && !hide) {
		line_draw_color = 0xff000000;
		line_draw_strength = 0.005;
		line_draw_render(context_raw.layer.decal_mat);
	}
}

function render_path_base_make_gbuffer_copy_textures() {
	let copy: render_target_t = map_get(render_path_render_targets, "gbuffer0_copy");
	let g0: render_target_t = map_get(render_path_render_targets, "gbuffer0");
	if (copy == null || copy._image.width != g0._image.width || copy._image.height != g0._image.height) {
		{
			let t: render_target_t = render_target_create();
			t.name = "gbuffer0_copy";
			t.width = 0;
			t.height = 0;
			t.format = "RGBA64";
			t.scale = render_path_base_get_super_sampling();
			t.depth_buffer = "main";
			render_path_create_render_target(t);
		}
		{
			let t: render_target_t = render_target_create();
			t.name = "gbuffer1_copy";
			t.width = 0;
			t.height = 0;
			t.format = "RGBA64";
			t.scale = render_path_base_get_super_sampling();
			render_path_create_render_target(t);
		}
		{
			let t: render_target_t = render_target_create();
			t.name = "gbuffer2_copy";
			t.width = 0;
			t.height = 0;
			t.format = "RGBA64";
			t.scale = render_path_base_get_super_sampling();
			render_path_create_render_target(t);
		}

		///if arm_metal
		// TODO: Fix depth attach for gbuffer0_copy on metal
		// Use resize to re-create buffers from scratch for now
		render_path_resize();
		///end
	}
}

function render_path_base_copy_to_gbuffer() {
	let additional: string[] = ["gbuffer1", "gbuffer2"];
	render_path_set_target("gbuffer0", additional);
	render_path_bind_target("gbuffer0_copy", "tex0");
	render_path_bind_target("gbuffer1_copy", "tex1");
	render_path_bind_target("gbuffer2_copy", "tex2");
	render_path_draw_shader("shader_datas/copy_mrt3_pass/copy_mrt3RGBA64_pass");
}
