
let render_path_raytrace_frame: i32 = 0;
let render_path_raytrace_ready: bool = false;
let render_path_raytrace_dirty: i32 = 0;
let render_path_raytrace_uv_scale: f32 = 1.0;
let render_path_raytrace_first: bool = true;
let render_path_raytrace_f32a: f32_array_t = f32_array_create(24);
let render_path_raytrace_help_mat: mat4_t = mat4_identity();
let render_path_raytrace_transform: mat4_t;
let render_path_raytrace_vb: vertex_buffer_t;
let render_path_raytrace_ib: index_buffer_t;

let render_path_raytrace_last_envmap: image_t = null;
let render_path_raytrace_is_bake: bool = false;

///if arm_direct3d12
let render_path_raytrace_ext: string = ".cso";
///elseif arm_metal
let render_path_raytrace_ext: string = ".metal";
///else
let render_path_raytrace_ext: string = ".spirv";
///end

let render_path_raytrace_last_texpaint: image_t = null;

function render_path_raytrace_init() {
}

function render_path_raytrace_commands(use_live_layer: bool) {
	if (!render_path_raytrace_ready || render_path_raytrace_is_bake) {
		render_path_raytrace_ready = true;
		render_path_raytrace_is_bake = false;
		let ext: string = "";
		///if is_forge
		ext = "forge_";
		///end
		let mode: string = config_raw.pathtrace_mode == pathtrace_mode_t.FAST ? "core" : "full";
		render_path_raytrace_raytrace_init("raytrace_brute_" + ext + mode + render_path_raytrace_ext);
		render_path_raytrace_last_envmap = null;
	}

	if (!context_raw.envmap_loaded) {
		context_load_envmap();
		context_update_envmap();
	}

	let probe: world_data_t = scene_world;
	let saved_envmap: image_t = context_raw.show_envmap_blur ? probe._.radiance_mipmaps[0] : context_raw.saved_envmap;

	if (render_path_raytrace_last_envmap != saved_envmap) {
		render_path_raytrace_last_envmap = saved_envmap;

		let bnoise_sobol: image_t = map_get(scene_embedded, "bnoise_sobol.k");
		let bnoise_scramble: image_t = map_get(scene_embedded, "bnoise_scramble.k");
		let bnoise_rank: image_t = map_get(scene_embedded, "bnoise_rank.k");

		let l: slot_layer_t = layers_flatten(true);
		iron_raytrace_set_textures(l.texpaint, l.texpaint_nor, l.texpaint_pack, saved_envmap.texture_, bnoise_sobol.texture_, bnoise_scramble.texture_, bnoise_rank.texture_);
	}

	///if is_lab
	let l: slot_layer_t = layers_flatten(true);
	if (l.texpaint != render_path_raytrace_last_texpaint) {
		render_path_raytrace_last_texpaint = l.texpaint;

		let bnoise_sobol: image_t = map_get(scene_embedded, "bnoise_sobol.k");
		let bnoise_scramble: image_t = map_get(scene_embedded, "bnoise_scramble.k");
		let bnoise_rank: image_t = map_get(scene_embedded, "bnoise_rank.k");

		iron_raytrace_set_textures(l.texpaint, l.texpaint_nor, l.texpaint_pack, saved_envmap.texture_, bnoise_sobol.texture_, bnoise_scramble.texture_, bnoise_rank.texture_);
	}
	///end

	if (context_raw.pdirty > 0 || render_path_raytrace_dirty > 0) {
		layers_flatten(true);
	}

	let cam: camera_object_t = scene_camera;
	let ct: transform_t = cam.base.transform;
	render_path_raytrace_help_mat = mat4_clone(cam.v);
	render_path_raytrace_help_mat = mat4_mult_mat(render_path_raytrace_help_mat, cam.p);
	render_path_raytrace_help_mat = mat4_inv(render_path_raytrace_help_mat);
	render_path_raytrace_f32a[0] = transform_world_x(ct);
	render_path_raytrace_f32a[1] = transform_world_y(ct);
	render_path_raytrace_f32a[2] = transform_world_z(ct);
	render_path_raytrace_f32a[3] = render_path_raytrace_frame;
	///if arm_metal
	// render_path_raytrace_frame = (render_path_raytrace_frame % (16)) + 1; // _PAINT
	render_path_raytrace_frame = render_path_raytrace_frame + 1; // _RENDER
	///else
	render_path_raytrace_frame = (render_path_raytrace_frame % 4) + 1; // _PAINT
	// render_path_raytrace_frame = render_path_raytrace_frame + 1; // _RENDER
	///end
	render_path_raytrace_f32a[4] = render_path_raytrace_help_mat.m00;
	render_path_raytrace_f32a[5] = render_path_raytrace_help_mat.m01;
	render_path_raytrace_f32a[6] = render_path_raytrace_help_mat.m02;
	render_path_raytrace_f32a[7] = render_path_raytrace_help_mat.m03;
	render_path_raytrace_f32a[8] = render_path_raytrace_help_mat.m10;
	render_path_raytrace_f32a[9] = render_path_raytrace_help_mat.m11;
	render_path_raytrace_f32a[10] = render_path_raytrace_help_mat.m12;
	render_path_raytrace_f32a[11] = render_path_raytrace_help_mat.m13;
	render_path_raytrace_f32a[12] = render_path_raytrace_help_mat.m20;
	render_path_raytrace_f32a[13] = render_path_raytrace_help_mat.m21;
	render_path_raytrace_f32a[14] = render_path_raytrace_help_mat.m22;
	render_path_raytrace_f32a[15] = render_path_raytrace_help_mat.m23;
	render_path_raytrace_f32a[16] = render_path_raytrace_help_mat.m30;
	render_path_raytrace_f32a[17] = render_path_raytrace_help_mat.m31;
	render_path_raytrace_f32a[18] = render_path_raytrace_help_mat.m32;
	render_path_raytrace_f32a[19] = render_path_raytrace_help_mat.m33;
	render_path_raytrace_f32a[20] = scene_world.strength * 1.5;
	if (!context_raw.show_envmap) {
		render_path_raytrace_f32a[20] = -render_path_raytrace_f32a[20];
	}
	render_path_raytrace_f32a[21] = context_raw.envmap_angle;
	render_path_raytrace_f32a[22] = render_path_raytrace_uv_scale;
	///if is_lab
	render_path_raytrace_f32a[22] *= scene_meshes[0].data.scale_tex;
	///end

	let framebuffer: render_target_t = map_get(render_path_render_targets, "buf");
	iron_raytrace_dispatch_rays(framebuffer._image.render_target_, render_path_raytrace_f32a);

	if (context_raw.ddirty == 1 || context_raw.pdirty == 1) {
		///if arm_metal
		context_raw.rdirty = 128;
		///else
		context_raw.rdirty = 4;
		///end
	}
	context_raw.ddirty--;
	context_raw.pdirty--;
	context_raw.rdirty--;

	// context_raw.ddirty = 1; // _RENDER

	///if is_forge
	context_raw.ddirty = 1;
	///end
}

function render_path_raytrace_raytrace_init(shader_name: string, build: bool = true, bake: bool = false) {
	if (render_path_raytrace_first) {
		render_path_raytrace_first = false;
		scene_embed_data("bnoise_sobol.k");
		scene_embed_data("bnoise_scramble.k");
		scene_embed_data("bnoise_rank.k");

		let shader: buffer_t = data_get_blob(shader_name);
		iron_raytrace_init(shader);
	}

	if (build) {
		render_path_raytrace_build_data(bake);
	}

	{
		iron_raytrace_as_init();

		///if is_forge
		for (let i: i32 = 0; i < project_paint_objects.length; ++i) {
			let po: mesh_object_t = project_paint_objects[i];
			if (!po.base.visible) {
				continue;
			}
			iron_raytrace_as_add(po.data._.vertex_buffer.buffer_, po.data._.index_buffers[0].buffer_, po.base.transform.world_unpack);
		}
		///else
		iron_raytrace_as_add(render_path_raytrace_vb.buffer_, render_path_raytrace_ib.buffer_, render_path_raytrace_transform);
		///end

		let vb_full: vertex_buffer_t = context_raw.merged_object.data._.vertex_buffer;
		let ib_full: index_buffer_t = context_raw.merged_object.data._.index_buffers[0];

		iron_raytrace_as_build(vb_full.buffer_, ib_full.buffer_);
	}
}

function render_path_raytrace_build_data(bake: bool = false) {
	if (context_raw.merged_object == null) {
		util_mesh_merge();
	}

	///if is_paint
	let mo: mesh_object_t = !context_layer_filter_used() ? context_raw.merged_object : context_raw.paint_object;
	///else
	let mo: mesh_object_t = scene_meshes[0];
	///end

	///if is_forge
	render_path_raytrace_transform = mo.base.transform.world_unpack;
	///else
	render_path_raytrace_transform = mat4_identity();
	///end

	if (!bake) {
		let sc: f32 = mo.base.transform.scale.x * mo.data.scale_pos;
		if (mo.base.parent != null) {
			sc *= mo.base.parent.transform.scale.x;
		}
		render_path_raytrace_transform = mat4_scale(render_path_raytrace_transform, vec4_create(sc, sc, sc));
	}

	render_path_raytrace_vb = mo.data._.vertex_buffer;
	render_path_raytrace_ib = mo.data._.index_buffers[0];
}

function render_path_raytrace_draw(use_live_layer: bool) {
	let is_live: bool = config_raw.brush_live && render_path_paint_live_layer_drawn > 0;
	if (context_raw.ddirty > 1 || context_raw.pdirty > 0 || is_live) {
		render_path_raytrace_frame = 0;
	}

	///if arm_metal
	// Delay path tracing additional samples while painting
	let down: bool = mouse_down() || pen_down();
	if (context_in_viewport() && down) {
		render_path_raytrace_frame = 0;
	}
	///end

	render_path_raytrace_commands(use_live_layer);
	render_path_base_draw_bloom("buf");
	render_path_set_target("buf");
	render_path_draw_meshes("overlay");
	render_path_set_target("buf");
	render_path_base_draw_compass();
	render_path_set_target("taa");
	render_path_bind_target("buf", "tex");
	render_path_draw_shader("shader_datas/compositor_pass/compositor_pass");
	render_path_set_target("");
	render_path_bind_target("taa", "tex");
	render_path_draw_shader("shader_datas/copy_pass/copy_pass");
	if (config_raw.brush_3d) {
		render_path_paint_commands_cursor();
	}
}
