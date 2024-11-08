
let base_pipe_copy: pipeline_t;
let base_pipe_copy8: pipeline_t;
let base_pipe_copy128: pipeline_t;
let base_pipe_copy_bgra: pipeline_t;
let base_pipe_copy_rgb: pipeline_t = null;
let base_pipe_merge: pipeline_t = null;
let base_pipe_merge_r: pipeline_t = null;
let base_pipe_merge_g: pipeline_t = null;
let base_pipe_merge_b: pipeline_t = null;
let base_pipe_merge_a: pipeline_t = null;
let base_pipe_invert8: pipeline_t;
let base_pipe_apply_mask: pipeline_t;
let base_pipe_merge_mask: pipeline_t;
let base_pipe_colorid_to_mask: pipeline_t;
let base_tex0: kinc_tex_unit_t;
let base_tex1: kinc_tex_unit_t;
let base_texmask: kinc_tex_unit_t;
let base_texa: kinc_tex_unit_t;
let base_opac: kinc_const_loc_t;
let base_blending: kinc_const_loc_t;
let base_tex0_mask: kinc_tex_unit_t;
let base_texa_mask: kinc_tex_unit_t;
let base_tex0_merge_mask: kinc_tex_unit_t;
let base_texa_merge_mask: kinc_tex_unit_t;
let base_tex_colorid: kinc_tex_unit_t;
let base_texpaint_colorid: kinc_tex_unit_t;
let base_opac_merge_mask: kinc_const_loc_t;
let base_blending_merge_mask: kinc_const_loc_t;
let base_temp_mask_image: image_t = null;

let base_pipe_copy_r: pipeline_t;
let base_pipe_copy_g: pipeline_t;
let base_pipe_copy_b: pipeline_t;
let base_pipe_copy_a: pipeline_t;
let base_pipe_copy_a_tex: kinc_tex_unit_t;
let base_pipe_inpaint_preview: pipeline_t;
let base_tex0_inpaint_preview: kinc_tex_unit_t;
let base_texa_inpaint_preview: kinc_tex_unit_t;

let base_pipe_cursor: pipeline_t;
let base_cursor_vp: kinc_const_loc_t;
let base_cursor_inv_vp: kinc_const_loc_t;
let base_cursor_mouse: kinc_const_loc_t;
let base_cursor_tex_step: kinc_const_loc_t;
let base_cursor_radius: kinc_const_loc_t;
let base_cursor_camera_right: kinc_const_loc_t;
let base_cursor_tint: kinc_const_loc_t;
let base_cursor_tex: kinc_tex_unit_t;
let base_cursor_gbufferd: kinc_tex_unit_t;

function base_make_merge_pipe(red: bool, green: bool, blue: bool, alpha: bool): pipeline_t {
	let pipe: pipeline_t = g4_pipeline_create();
	pipe.vertex_shader = sys_get_shader("pass.vert");
	pipe.fragment_shader = sys_get_shader("layer_merge.frag");
	let vs: vertex_struct_t = g4_vertex_struct_create();
	g4_vertex_struct_add(vs, "pos", vertex_data_t.F32_2X);
	pipe.input_layout = [vs];
	pipe.color_write_masks_red = [red];
	pipe.color_write_masks_green = [green];
	pipe.color_write_masks_blue = [blue];
	pipe.color_write_masks_alpha = [alpha];
	g4_pipeline_compile(pipe);
	return pipe;
}

function base_make_pipe() {
	///if (is_paint || is_sculpt)
	base_pipe_merge = base_make_merge_pipe(true, true, true, true);
	base_pipe_merge_r = base_make_merge_pipe(true, false, false, false);
	base_pipe_merge_g = base_make_merge_pipe(false, true, false, false);
	base_pipe_merge_b = base_make_merge_pipe(false, false, true, false);
	base_pipe_merge_a = base_make_merge_pipe(false, false, false, true);
	base_tex0 = g4_pipeline_get_tex_unit(base_pipe_merge, "tex0"); // Always binding texpaint.a for blending
	base_tex1 = g4_pipeline_get_tex_unit(base_pipe_merge, "tex1");
	base_texmask = g4_pipeline_get_tex_unit(base_pipe_merge, "texmask");
	base_texa = g4_pipeline_get_tex_unit(base_pipe_merge, "texa");
	base_opac = g4_pipeline_get_const_loc(base_pipe_merge, "opac");
	base_blending = g4_pipeline_get_const_loc(base_pipe_merge, "blending");
	///end

	{
		base_pipe_copy = g4_pipeline_create();
		base_pipe_copy.vertex_shader = sys_get_shader("layer_view.vert");
		base_pipe_copy.fragment_shader = sys_get_shader("layer_copy.frag");
		let vs: vertex_struct_t = g4_vertex_struct_create();
		g4_vertex_struct_add(vs, "pos", vertex_data_t.F32_3X);
		g4_vertex_struct_add(vs, "tex", vertex_data_t.F32_2X);
		g4_vertex_struct_add(vs, "col", vertex_data_t.U8_4X_NORM);
		base_pipe_copy.input_layout = [vs];
		g4_pipeline_compile(base_pipe_copy);
	}

	{
		base_pipe_copy_bgra = g4_pipeline_create();
		base_pipe_copy_bgra.vertex_shader = sys_get_shader("layer_view.vert");
		base_pipe_copy_bgra.fragment_shader = sys_get_shader("layer_copy_bgra.frag");
		let vs: vertex_struct_t = g4_vertex_struct_create();
		g4_vertex_struct_add(vs, "pos", vertex_data_t.F32_3X);
		g4_vertex_struct_add(vs, "tex", vertex_data_t.F32_2X);
		g4_vertex_struct_add(vs, "col", vertex_data_t.U8_4X_NORM);
		base_pipe_copy_bgra.input_layout = [vs];
		g4_pipeline_compile(base_pipe_copy_bgra);
	}

	///if (arm_metal || arm_vulkan || arm_direct3d12)
	{
		base_pipe_copy8 = g4_pipeline_create();
		base_pipe_copy8.vertex_shader = sys_get_shader("layer_view.vert");
		base_pipe_copy8.fragment_shader = sys_get_shader("layer_copy.frag");
		let vs: vertex_struct_t = g4_vertex_struct_create();
		g4_vertex_struct_add(vs, "pos", vertex_data_t.F32_3X);
		g4_vertex_struct_add(vs, "tex", vertex_data_t.F32_2X);
		g4_vertex_struct_add(vs, "col", vertex_data_t.U8_4X_NORM);
		base_pipe_copy8.input_layout = [vs];
		base_pipe_copy8.color_attachment_count = 1;
		base_pipe_copy8.color_attachments[0] = tex_format_t.R8;
		g4_pipeline_compile(base_pipe_copy8);
	}

	{
		base_pipe_copy128 = g4_pipeline_create();
		base_pipe_copy128.vertex_shader = sys_get_shader("layer_view.vert");
		base_pipe_copy128.fragment_shader = sys_get_shader("layer_copy.frag");
		let vs: vertex_struct_t = g4_vertex_struct_create();
		g4_vertex_struct_add(vs, "pos", vertex_data_t.F32_3X);
		g4_vertex_struct_add(vs, "tex", vertex_data_t.F32_2X);
		g4_vertex_struct_add(vs, "col", vertex_data_t.U8_4X_NORM);
		base_pipe_copy128.input_layout = [vs];
		base_pipe_copy128.color_attachment_count = 1;
		base_pipe_copy128.color_attachments[0] = tex_format_t.RGBA128;
		g4_pipeline_compile(base_pipe_copy128);
	}
	///else
	base_pipe_copy8 = base_pipe_copy;
	base_pipe_copy128 = base_pipe_copy;
	///end

	///if (is_paint || is_sculpt)
	{
		base_pipe_invert8 = g4_pipeline_create();
		base_pipe_invert8.vertex_shader = sys_get_shader("layer_view.vert");
		base_pipe_invert8.fragment_shader = sys_get_shader("layer_invert.frag");
		let vs: vertex_struct_t = g4_vertex_struct_create();
		g4_vertex_struct_add(vs, "pos", vertex_data_t.F32_3X);
		g4_vertex_struct_add(vs, "tex", vertex_data_t.F32_2X);
		g4_vertex_struct_add(vs, "col", vertex_data_t.U8_4X_NORM);
		base_pipe_invert8.input_layout = [vs];
		base_pipe_invert8.color_attachment_count = 1;
		base_pipe_invert8.color_attachments[0] = tex_format_t.R8;
		g4_pipeline_compile(base_pipe_invert8);
	}

	{
		base_pipe_apply_mask = g4_pipeline_create();
		base_pipe_apply_mask.vertex_shader = sys_get_shader("pass.vert");
		base_pipe_apply_mask.fragment_shader = sys_get_shader("mask_apply.frag");
		let vs: vertex_struct_t = g4_vertex_struct_create();
		g4_vertex_struct_add(vs, "pos", vertex_data_t.F32_2X);
		base_pipe_apply_mask.input_layout = [vs];
		g4_pipeline_compile(base_pipe_apply_mask);
		base_tex0_mask = g4_pipeline_get_tex_unit(base_pipe_apply_mask, "tex0");
		base_texa_mask = g4_pipeline_get_tex_unit(base_pipe_apply_mask, "texa");
	}

	{
		base_pipe_merge_mask = g4_pipeline_create();
		base_pipe_merge_mask.vertex_shader = sys_get_shader("pass.vert");
		base_pipe_merge_mask.fragment_shader = sys_get_shader("mask_merge.frag");
		let vs: vertex_struct_t = g4_vertex_struct_create();
		g4_vertex_struct_add(vs, "pos", vertex_data_t.F32_2X);
		base_pipe_merge_mask.input_layout = [vs];
		g4_pipeline_compile(base_pipe_merge_mask);
		base_tex0_merge_mask = g4_pipeline_get_tex_unit(base_pipe_merge_mask, "tex0");
		base_texa_merge_mask = g4_pipeline_get_tex_unit(base_pipe_merge_mask, "texa");
		base_opac_merge_mask = g4_pipeline_get_const_loc(base_pipe_merge_mask, "opac");
		base_blending_merge_mask = g4_pipeline_get_const_loc(base_pipe_merge_mask, "blending");
	}

	{
		base_pipe_colorid_to_mask = g4_pipeline_create();
		base_pipe_colorid_to_mask.vertex_shader = sys_get_shader("pass.vert");
		base_pipe_colorid_to_mask.fragment_shader = sys_get_shader("mask_colorid.frag");
		let vs: vertex_struct_t = g4_vertex_struct_create();
		g4_vertex_struct_add(vs, "pos", vertex_data_t.F32_2X);
		base_pipe_colorid_to_mask.input_layout = [vs];
		g4_pipeline_compile(base_pipe_colorid_to_mask);
		base_texpaint_colorid = g4_pipeline_get_tex_unit(base_pipe_colorid_to_mask, "texpaint_colorid");
		base_tex_colorid = g4_pipeline_get_tex_unit(base_pipe_colorid_to_mask, "texcolorid");
	}
	///end

	///if is_lab
	{
		base_pipe_copy_r = g4_pipeline_create();
		base_pipe_copy_r.vertex_shader = sys_get_shader("layer_view.vert");
		base_pipe_copy_r.fragment_shader = sys_get_shader("layer_copy.frag");
		let vs: vertex_struct_t = g4_vertex_struct_create();
		g4_vertex_struct_add(vs, "pos", vertex_data_t.F32_3X);
		g4_vertex_struct_add(vs, "tex", vertex_data_t.F32_2X);
		g4_vertex_struct_add(vs, "col", vertex_data_t.U8_4X_NORM);
		base_pipe_copy_r.input_layout = [vs];
		base_pipe_copy_r.color_write_masks_green = [false];
		base_pipe_copy_r.color_write_masks_blue = [false];
		base_pipe_copy_r.color_write_masks_alpha = [false];
		g4_pipeline_compile(base_pipe_copy_r);
	}

	{
		base_pipe_copy_g = g4_pipeline_create();
		base_pipe_copy_g.vertex_shader = sys_get_shader("layer_view.vert");
		base_pipe_copy_g.fragment_shader = sys_get_shader("layer_copy.frag");
		let vs: vertex_struct_t = g4_vertex_struct_create();
		g4_vertex_struct_add(vs, "pos", vertex_data_t.F32_3X);
		g4_vertex_struct_add(vs, "tex", vertex_data_t.F32_2X);
		g4_vertex_struct_add(vs, "col", vertex_data_t.U8_4X_NORM);
		base_pipe_copy_g.input_layout = [vs];
		base_pipe_copy_g.color_write_masks_red = [false];
		base_pipe_copy_g.color_write_masks_blue = [false];
		base_pipe_copy_g.color_write_masks_alpha = [false];
		g4_pipeline_compile(base_pipe_copy_g);
	}

	{
		base_pipe_copy_b = g4_pipeline_create();
		base_pipe_copy_b.vertex_shader = sys_get_shader("layer_view.vert");
		base_pipe_copy_b.fragment_shader = sys_get_shader("layer_copy.frag");
		let vs: vertex_struct_t = g4_vertex_struct_create();
		g4_vertex_struct_add(vs, "pos", vertex_data_t.F32_3X);
		g4_vertex_struct_add(vs, "tex", vertex_data_t.F32_2X);
		g4_vertex_struct_add(vs, "col", vertex_data_t.U8_4X_NORM);
		base_pipe_copy_b.input_layout = [vs];
		base_pipe_copy_b.color_write_masks_red = [false];
		base_pipe_copy_b.color_write_masks_green = [false];
		base_pipe_copy_b.color_write_masks_alpha = [false];
		g4_pipeline_compile(base_pipe_copy_b);
	}

	{
		base_pipe_inpaint_preview = g4_pipeline_create();
		base_pipe_inpaint_preview.vertex_shader = sys_get_shader("pass.vert");
		base_pipe_inpaint_preview.fragment_shader = sys_get_shader("inpaint_preview.frag");
		let vs: vertex_struct_t = g4_vertex_struct_create();
		g4_vertex_struct_add(vs, "pos", vertex_data_t.F32_2X);
		base_pipe_inpaint_preview.input_layout = [vs];
		g4_pipeline_compile(base_pipe_inpaint_preview);
		base_tex0_inpaint_preview = g4_pipeline_get_tex_unit(base_pipe_inpaint_preview, "tex0");
		base_texa_inpaint_preview = g4_pipeline_get_tex_unit(base_pipe_inpaint_preview, "texa");
	}
	///end
}

function base_make_pipe_copy_rgb() {
	base_pipe_copy_rgb = g4_pipeline_create();
	base_pipe_copy_rgb.vertex_shader = sys_get_shader("layer_view.vert");
	base_pipe_copy_rgb.fragment_shader = sys_get_shader("layer_copy.frag");
	let vs: vertex_struct_t = g4_vertex_struct_create();
	g4_vertex_struct_add(vs, "pos", vertex_data_t.F32_3X);
	g4_vertex_struct_add(vs, "tex", vertex_data_t.F32_2X);
	g4_vertex_struct_add(vs, "col", vertex_data_t.U8_4X_NORM);
	base_pipe_copy_rgb.input_layout = [vs];
	base_pipe_copy_rgb.color_write_masks_alpha = [false];
	g4_pipeline_compile(base_pipe_copy_rgb);
}

function base_make_pipe_copy_a() {
	base_pipe_copy_a = g4_pipeline_create();
	base_pipe_copy_a.vertex_shader = sys_get_shader("pass.vert");
	base_pipe_copy_a.fragment_shader = sys_get_shader("layer_copy_rrrr.frag");
	let vs: vertex_struct_t = g4_vertex_struct_create();
	g4_vertex_struct_add(vs, "pos", vertex_data_t.F32_2X);
	base_pipe_copy_a.input_layout = [vs];
	base_pipe_copy_a.color_write_masks_red = [false];
	base_pipe_copy_a.color_write_masks_green = [false];
	base_pipe_copy_a.color_write_masks_blue = [false];
	g4_pipeline_compile(base_pipe_copy_a);
	base_pipe_copy_a_tex = g4_pipeline_get_tex_unit(base_pipe_copy_a, "tex");
}

function base_make_cursor_pipe() {
	base_pipe_cursor = g4_pipeline_create();
	base_pipe_cursor.vertex_shader = sys_get_shader("cursor.vert");
	base_pipe_cursor.fragment_shader = sys_get_shader("cursor.frag");
	let vs: vertex_struct_t = g4_vertex_struct_create();
	///if (arm_metal || arm_vulkan)
	g4_vertex_struct_add(vs, "tex", vertex_data_t.I16_2X_NORM);
	///else
	g4_vertex_struct_add(vs, "pos", vertex_data_t.I16_4X_NORM);
	g4_vertex_struct_add(vs, "nor", vertex_data_t.I16_2X_NORM);
	g4_vertex_struct_add(vs, "tex", vertex_data_t.I16_2X_NORM);
	///end
	base_pipe_cursor.input_layout = [vs];
	base_pipe_cursor.blend_source = blend_factor_t.SOURCE_ALPHA;
	base_pipe_cursor.blend_dest = blend_factor_t.INV_SOURCE_ALPHA;
	base_pipe_cursor.depth_write = false;
	base_pipe_cursor.depth_mode = compare_mode_t.ALWAYS;
	g4_pipeline_compile(base_pipe_cursor);
	base_cursor_vp = g4_pipeline_get_const_loc(base_pipe_cursor, "VP");
	base_cursor_inv_vp = g4_pipeline_get_const_loc(base_pipe_cursor, "invVP");
	base_cursor_mouse = g4_pipeline_get_const_loc(base_pipe_cursor, "mouse");
	base_cursor_tex_step = g4_pipeline_get_const_loc(base_pipe_cursor, "tex_step");
	base_cursor_radius = g4_pipeline_get_const_loc(base_pipe_cursor, "radius");
	base_cursor_camera_right = g4_pipeline_get_const_loc(base_pipe_cursor, "camera_right");
	base_cursor_tint = g4_pipeline_get_const_loc(base_pipe_cursor, "tint");
	base_cursor_gbufferd = g4_pipeline_get_tex_unit(base_pipe_cursor, "gbufferD");
	base_cursor_tex = g4_pipeline_get_tex_unit(base_pipe_cursor, "tex");
}
