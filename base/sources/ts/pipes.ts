
let pipes_copy: pipeline_t;
let pipes_copy8: pipeline_t;
let pipes_copy128: pipeline_t;
let pipes_copy_bgra: pipeline_t;
let pipes_copy_rgb: pipeline_t = null;
let pipes_copy_r: pipeline_t;
let pipes_copy_g: pipeline_t;
let pipes_copy_a: pipeline_t;
let pipes_copy_a_tex: kinc_tex_unit_t;

let pipes_merge: pipeline_t = null;
let pipes_merge_r: pipeline_t = null;
let pipes_merge_g: pipeline_t = null;
let pipes_merge_b: pipeline_t = null;
let pipes_merge_mask: pipeline_t;

let pipes_invert8: pipeline_t;
let pipes_apply_mask: pipeline_t;
let pipes_colorid_to_mask: pipeline_t;

let pipes_tex0: kinc_tex_unit_t;
let pipes_tex1: kinc_tex_unit_t;
let pipes_texmask: kinc_tex_unit_t;
let pipes_texa: kinc_tex_unit_t;
let pipes_opac: kinc_const_loc_t;
let pipes_blending: kinc_const_loc_t;
let pipes_tex0_mask: kinc_tex_unit_t;
let pipes_texa_mask: kinc_tex_unit_t;
let pipes_tex0_merge_mask: kinc_tex_unit_t;
let pipes_texa_merge_mask: kinc_tex_unit_t;
let pipes_tex_colorid: kinc_tex_unit_t;
let pipes_texpaint_colorid: kinc_tex_unit_t;
let pipes_opac_merge_mask: kinc_const_loc_t;
let pipes_blending_merge_mask: kinc_const_loc_t;
let pipes_temp_mask_image: image_t = null;

let pipes_inpaint_preview: pipeline_t;
let pipes_tex0_inpaint_preview: kinc_tex_unit_t;
let pipes_texa_inpaint_preview: kinc_tex_unit_t;

let pipes_cursor: pipeline_t;
let pipes_cursor_vp: kinc_const_loc_t;
let pipes_cursor_inv_vp: kinc_const_loc_t;
let pipes_cursor_mouse: kinc_const_loc_t;
let pipes_cursor_tex_step: kinc_const_loc_t;
let pipes_cursor_radius: kinc_const_loc_t;
let pipes_cursor_camera_right: kinc_const_loc_t;
let pipes_cursor_tint: kinc_const_loc_t;
let pipes_cursor_gbufferd: kinc_tex_unit_t;

function _pipes_make_merge(red: bool, green: bool, blue: bool, alpha: bool): pipeline_t {
	let pipe: pipeline_t = g4_pipeline_create();
	pipe.vertex_shader = sys_get_shader("pass.vert");
	pipe.fragment_shader = sys_get_shader("layer_merge.frag");
	let vs: vertex_struct_t = g4_vertex_struct_create();
	g4_vertex_struct_add(vs, "pos", vertex_data_t.F32_2X);
	pipe.input_layout = vs;
	pipe.color_write_masks_red = [red];
	pipe.color_write_masks_green = [green];
	pipe.color_write_masks_blue = [blue];
	pipe.color_write_masks_alpha = [alpha];
	g4_pipeline_compile(pipe);
	return pipe;
}

function pipes_init() {
	///if (is_paint || is_sculpt)
	pipes_merge = _pipes_make_merge(true, true, true, true);
	pipes_merge_r = _pipes_make_merge(true, false, false, false);
	pipes_merge_g = _pipes_make_merge(false, true, false, false);
	pipes_merge_b = _pipes_make_merge(false, false, true, false);
	pipes_tex0 = g4_pipeline_get_tex_unit(pipes_merge, "tex0"); // Always binding texpaint.a for blending
	pipes_tex1 = g4_pipeline_get_tex_unit(pipes_merge, "tex1");
	pipes_texmask = g4_pipeline_get_tex_unit(pipes_merge, "texmask");
	pipes_texa = g4_pipeline_get_tex_unit(pipes_merge, "texa");
	pipes_opac = g4_pipeline_get_const_loc(pipes_merge, "opac");
	pipes_blending = g4_pipeline_get_const_loc(pipes_merge, "blending");
	///end

	{
		pipes_copy = g4_pipeline_create();
		pipes_copy.vertex_shader = sys_get_shader("layer_view.vert");
		pipes_copy.fragment_shader = sys_get_shader("layer_copy.frag");
		let vs: vertex_struct_t = g4_vertex_struct_create();
		g4_vertex_struct_add(vs, "pos", vertex_data_t.F32_3X);
		g4_vertex_struct_add(vs, "tex", vertex_data_t.F32_2X);
		g4_vertex_struct_add(vs, "col", vertex_data_t.U8_4X_NORM);
		pipes_copy.input_layout = vs;
		g4_pipeline_compile(pipes_copy);
	}

	{
		pipes_copy_bgra = g4_pipeline_create();
		pipes_copy_bgra.vertex_shader = sys_get_shader("layer_view.vert");
		pipes_copy_bgra.fragment_shader = sys_get_shader("layer_copy_bgra.frag");
		let vs: vertex_struct_t = g4_vertex_struct_create();
		g4_vertex_struct_add(vs, "pos", vertex_data_t.F32_3X);
		g4_vertex_struct_add(vs, "tex", vertex_data_t.F32_2X);
		g4_vertex_struct_add(vs, "col", vertex_data_t.U8_4X_NORM);
		pipes_copy_bgra.input_layout = vs;
		g4_pipeline_compile(pipes_copy_bgra);
	}

	{
		pipes_copy8 = g4_pipeline_create();
		pipes_copy8.vertex_shader = sys_get_shader("layer_view.vert");
		pipes_copy8.fragment_shader = sys_get_shader("layer_copy.frag");
		let vs: vertex_struct_t = g4_vertex_struct_create();
		g4_vertex_struct_add(vs, "pos", vertex_data_t.F32_3X);
		g4_vertex_struct_add(vs, "tex", vertex_data_t.F32_2X);
		g4_vertex_struct_add(vs, "col", vertex_data_t.U8_4X_NORM);
		pipes_copy8.input_layout = vs;
		pipes_copy8.color_attachment_count = 1;
		pipes_copy8.color_attachments[0] = tex_format_t.R8;
		g4_pipeline_compile(pipes_copy8);
	}

	{
		pipes_copy128 = g4_pipeline_create();
		pipes_copy128.vertex_shader = sys_get_shader("layer_view.vert");
		pipes_copy128.fragment_shader = sys_get_shader("layer_copy.frag");
		let vs: vertex_struct_t = g4_vertex_struct_create();
		g4_vertex_struct_add(vs, "pos", vertex_data_t.F32_3X);
		g4_vertex_struct_add(vs, "tex", vertex_data_t.F32_2X);
		g4_vertex_struct_add(vs, "col", vertex_data_t.U8_4X_NORM);
		pipes_copy128.input_layout = vs;
		pipes_copy128.color_attachment_count = 1;
		pipes_copy128.color_attachments[0] = tex_format_t.RGBA128;
		g4_pipeline_compile(pipes_copy128);
	}

	///if (is_paint || is_sculpt)
	{
		pipes_invert8 = g4_pipeline_create();
		pipes_invert8.vertex_shader = sys_get_shader("layer_view.vert");
		pipes_invert8.fragment_shader = sys_get_shader("layer_invert.frag");
		let vs: vertex_struct_t = g4_vertex_struct_create();
		g4_vertex_struct_add(vs, "pos", vertex_data_t.F32_3X);
		g4_vertex_struct_add(vs, "tex", vertex_data_t.F32_2X);
		g4_vertex_struct_add(vs, "col", vertex_data_t.U8_4X_NORM);
		pipes_invert8.input_layout = vs;
		pipes_invert8.color_attachment_count = 1;
		pipes_invert8.color_attachments[0] = tex_format_t.R8;
		g4_pipeline_compile(pipes_invert8);
	}

	{
		pipes_apply_mask = g4_pipeline_create();
		pipes_apply_mask.vertex_shader = sys_get_shader("pass.vert");
		pipes_apply_mask.fragment_shader = sys_get_shader("mask_apply.frag");
		let vs: vertex_struct_t = g4_vertex_struct_create();
		g4_vertex_struct_add(vs, "pos", vertex_data_t.F32_2X);
		pipes_apply_mask.input_layout = vs;
		g4_pipeline_compile(pipes_apply_mask);
		pipes_tex0_mask = g4_pipeline_get_tex_unit(pipes_apply_mask, "tex0");
		pipes_texa_mask = g4_pipeline_get_tex_unit(pipes_apply_mask, "texa");
	}

	{
		pipes_merge_mask = g4_pipeline_create();
		pipes_merge_mask.vertex_shader = sys_get_shader("pass.vert");
		pipes_merge_mask.fragment_shader = sys_get_shader("mask_merge.frag");
		let vs: vertex_struct_t = g4_vertex_struct_create();
		g4_vertex_struct_add(vs, "pos", vertex_data_t.F32_2X);
		pipes_merge_mask.input_layout = vs;
		g4_pipeline_compile(pipes_merge_mask);
		pipes_tex0_merge_mask = g4_pipeline_get_tex_unit(pipes_merge_mask, "tex0");
		pipes_texa_merge_mask = g4_pipeline_get_tex_unit(pipes_merge_mask, "texa");
		pipes_opac_merge_mask = g4_pipeline_get_const_loc(pipes_merge_mask, "opac");
		pipes_blending_merge_mask = g4_pipeline_get_const_loc(pipes_merge_mask, "blending");
	}

	{
		pipes_colorid_to_mask = g4_pipeline_create();
		pipes_colorid_to_mask.vertex_shader = sys_get_shader("pass.vert");
		pipes_colorid_to_mask.fragment_shader = sys_get_shader("mask_colorid.frag");
		let vs: vertex_struct_t = g4_vertex_struct_create();
		g4_vertex_struct_add(vs, "pos", vertex_data_t.F32_2X);
		pipes_colorid_to_mask.input_layout = vs;
		g4_pipeline_compile(pipes_colorid_to_mask);
		pipes_texpaint_colorid = g4_pipeline_get_tex_unit(pipes_colorid_to_mask, "texpaint_colorid");
		pipes_tex_colorid = g4_pipeline_get_tex_unit(pipes_colorid_to_mask, "texcolorid");
	}
	///end

	///if is_lab
	{
		pipes_copy_r = g4_pipeline_create();
		pipes_copy_r.vertex_shader = sys_get_shader("layer_view.vert");
		pipes_copy_r.fragment_shader = sys_get_shader("layer_copy.frag");
		let vs: vertex_struct_t = g4_vertex_struct_create();
		g4_vertex_struct_add(vs, "pos", vertex_data_t.F32_3X);
		g4_vertex_struct_add(vs, "tex", vertex_data_t.F32_2X);
		g4_vertex_struct_add(vs, "col", vertex_data_t.U8_4X_NORM);
		pipes_copy_r.input_layout = vs;
		pipes_copy_r.color_write_masks_green = [false];
		pipes_copy_r.color_write_masks_blue = [false];
		pipes_copy_r.color_write_masks_alpha = [false];
		g4_pipeline_compile(pipes_copy_r);
	}

	{
		pipes_copy_g = g4_pipeline_create();
		pipes_copy_g.vertex_shader = sys_get_shader("layer_view.vert");
		pipes_copy_g.fragment_shader = sys_get_shader("layer_copy.frag");
		let vs: vertex_struct_t = g4_vertex_struct_create();
		g4_vertex_struct_add(vs, "pos", vertex_data_t.F32_3X);
		g4_vertex_struct_add(vs, "tex", vertex_data_t.F32_2X);
		g4_vertex_struct_add(vs, "col", vertex_data_t.U8_4X_NORM);
		pipes_copy_g.input_layout = vs;
		pipes_copy_g.color_write_masks_red = [false];
		pipes_copy_g.color_write_masks_blue = [false];
		pipes_copy_g.color_write_masks_alpha = [false];
		g4_pipeline_compile(pipes_copy_g);
	}

	{
		pipes_inpaint_preview = g4_pipeline_create();
		pipes_inpaint_preview.vertex_shader = sys_get_shader("pass.vert");
		pipes_inpaint_preview.fragment_shader = sys_get_shader("inpaint_preview.frag");
		let vs: vertex_struct_t = g4_vertex_struct_create();
		g4_vertex_struct_add(vs, "pos", vertex_data_t.F32_2X);
		pipes_inpaint_preview.input_layout = vs;
		g4_pipeline_compile(pipes_inpaint_preview);
		pipes_tex0_inpaint_preview = g4_pipeline_get_tex_unit(pipes_inpaint_preview, "tex0");
		pipes_texa_inpaint_preview = g4_pipeline_get_tex_unit(pipes_inpaint_preview, "texa");
	}

	{
		pipes_copy_a = g4_pipeline_create();
		pipes_copy_a.vertex_shader = sys_get_shader("pass.vert");
		pipes_copy_a.fragment_shader = sys_get_shader("layer_copy_rrrr.frag");
		let vs: vertex_struct_t = g4_vertex_struct_create();
		g4_vertex_struct_add(vs, "pos", vertex_data_t.F32_2X);
		pipes_copy_a.input_layout = vs;
		pipes_copy_a.color_write_masks_red = [false];
		pipes_copy_a.color_write_masks_green = [false];
		pipes_copy_a.color_write_masks_blue = [false];
		g4_pipeline_compile(pipes_copy_a);
		pipes_copy_a_tex = g4_pipeline_get_tex_unit(pipes_copy_a, "tex");
	}
	///end

	{
		pipes_copy_rgb = g4_pipeline_create();
		pipes_copy_rgb.vertex_shader = sys_get_shader("layer_view.vert");
		pipes_copy_rgb.fragment_shader = sys_get_shader("layer_copy.frag");
		let vs: vertex_struct_t = g4_vertex_struct_create();
		g4_vertex_struct_add(vs, "pos", vertex_data_t.F32_3X);
		g4_vertex_struct_add(vs, "tex", vertex_data_t.F32_2X);
		g4_vertex_struct_add(vs, "col", vertex_data_t.U8_4X_NORM);
		pipes_copy_rgb.input_layout = vs;
		pipes_copy_rgb.color_write_masks_alpha = [false];
		g4_pipeline_compile(pipes_copy_rgb);
	}

	{
		pipes_cursor = g4_pipeline_create();
		pipes_cursor.vertex_shader = sys_get_shader("cursor.vert");
		pipes_cursor.fragment_shader = sys_get_shader("cursor.frag");
		let vs: vertex_struct_t = g4_vertex_struct_create();
		///if (arm_metal || arm_vulkan)
		g4_vertex_struct_add(vs, "tex", vertex_data_t.I16_2X_NORM);
		///else
		g4_vertex_struct_add(vs, "pos", vertex_data_t.I16_4X_NORM);
		g4_vertex_struct_add(vs, "nor", vertex_data_t.I16_2X_NORM);
		g4_vertex_struct_add(vs, "tex", vertex_data_t.I16_2X_NORM);
		///end
		pipes_cursor.input_layout = vs;
		pipes_cursor.blend_source = blend_factor_t.SOURCE_ALPHA;
		pipes_cursor.blend_dest = blend_factor_t.INV_SOURCE_ALPHA;
		pipes_cursor.depth_write = false;
		pipes_cursor.depth_mode = compare_mode_t.ALWAYS;
		g4_pipeline_compile(pipes_cursor);
		pipes_cursor_vp = g4_pipeline_get_const_loc(pipes_cursor, "VP");
		pipes_cursor_inv_vp = g4_pipeline_get_const_loc(pipes_cursor, "invVP");
		pipes_cursor_mouse = g4_pipeline_get_const_loc(pipes_cursor, "mouse");
		pipes_cursor_tex_step = g4_pipeline_get_const_loc(pipes_cursor, "tex_step");
		pipes_cursor_radius = g4_pipeline_get_const_loc(pipes_cursor, "radius");
		pipes_cursor_camera_right = g4_pipeline_get_const_loc(pipes_cursor, "camera_right");
		pipes_cursor_tint = g4_pipeline_get_const_loc(pipes_cursor, "tint");
		pipes_cursor_gbufferd = g4_pipeline_get_tex_unit(pipes_cursor, "gbufferD");
	}
}
