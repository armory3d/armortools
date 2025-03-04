
let pipes_copy: kinc_g5_pipeline_t;
let pipes_copy8: kinc_g5_pipeline_t;
let pipes_copy128: kinc_g5_pipeline_t;
let pipes_copy_bgra: kinc_g5_pipeline_t;
let pipes_copy_rgb: kinc_g5_pipeline_t = null;
let pipes_copy_r: kinc_g5_pipeline_t;
let pipes_copy_g: kinc_g5_pipeline_t;
let pipes_copy_a: kinc_g5_pipeline_t;
let pipes_copy_a_tex: kinc_tex_unit_t;

let pipes_merge: kinc_g5_pipeline_t = null;
let pipes_merge_r: kinc_g5_pipeline_t = null;
let pipes_merge_g: kinc_g5_pipeline_t = null;
let pipes_merge_b: kinc_g5_pipeline_t = null;
let pipes_merge_mask: kinc_g5_pipeline_t;

let pipes_invert8: kinc_g5_pipeline_t;
let pipes_apply_mask: kinc_g5_pipeline_t;
let pipes_colorid_to_mask: kinc_g5_pipeline_t;

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

let pipes_inpaint_preview: kinc_g5_pipeline_t;
let pipes_tex0_inpaint_preview: kinc_tex_unit_t;
let pipes_texa_inpaint_preview: kinc_tex_unit_t;

let pipes_cursor: kinc_g5_pipeline_t;
let pipes_cursor_vp: kinc_const_loc_t;
let pipes_cursor_inv_vp: kinc_const_loc_t;
let pipes_cursor_mouse: kinc_const_loc_t;
let pipes_cursor_tex_step: kinc_const_loc_t;
let pipes_cursor_radius: kinc_const_loc_t;
let pipes_cursor_camera_right: kinc_const_loc_t;
let pipes_cursor_tint: kinc_const_loc_t;
let pipes_cursor_gbufferd: kinc_tex_unit_t;

function _pipes_make_merge(red: bool, green: bool, blue: bool, alpha: bool): kinc_g5_pipeline_t {
	let pipe: kinc_g5_pipeline_t = iron_g4_create_pipeline();
	pipe.vertex_shader = sys_get_shader("pass.vert");
	pipe.fragment_shader = sys_get_shader("layer_merge.frag");
	let vs: kinc_g5_vertex_structure_t = g4_vertex_struct_create();
	g4_vertex_struct_add(vs, "pos", vertex_data_t.F32_2X);
	pipe.input_layout = vs;
	ARRAY_ACCESS(pipe.color_write_mask_red, 0) = red;
	ARRAY_ACCESS(pipe.color_write_mask_green, 0) = green;
	ARRAY_ACCESS(pipe.color_write_mask_blue, 0) = blue;
	ARRAY_ACCESS(pipe.color_write_mask_alpha, 0) = alpha;
	iron_g4_compile_pipeline(pipe);
	return pipe;
}

function pipes_init() {
	///if (is_paint || is_sculpt)
	pipes_merge = _pipes_make_merge(true, true, true, true);
	pipes_merge_r = _pipes_make_merge(true, false, false, false);
	pipes_merge_g = _pipes_make_merge(false, true, false, false);
	pipes_merge_b = _pipes_make_merge(false, false, true, false);
	pipes_tex0 = iron_g4_get_texture_unit(pipes_merge, "tex0"); // Always binding texpaint.a for blending
	pipes_tex1 = iron_g4_get_texture_unit(pipes_merge, "tex1");
	pipes_texmask = iron_g4_get_texture_unit(pipes_merge, "texmask");
	pipes_texa = iron_g4_get_texture_unit(pipes_merge, "texa");
	pipes_opac = iron_g4_get_constant_location(pipes_merge, "opac");
	pipes_blending = iron_g4_get_constant_location(pipes_merge, "blending");
	///end

	{
		pipes_copy = iron_g4_create_pipeline();
		pipes_copy.vertex_shader = sys_get_shader("layer_view.vert");
		pipes_copy.fragment_shader = sys_get_shader("layer_copy.frag");
		let vs: kinc_g5_vertex_structure_t = g4_vertex_struct_create();
		g4_vertex_struct_add(vs, "pos", vertex_data_t.F32_3X);
		g4_vertex_struct_add(vs, "tex", vertex_data_t.F32_2X);
		g4_vertex_struct_add(vs, "col", vertex_data_t.U8_4X_NORM);
		pipes_copy.input_layout = vs;
		iron_g4_compile_pipeline(pipes_copy);
	}

	{
		pipes_copy_bgra = iron_g4_create_pipeline();
		pipes_copy_bgra.vertex_shader = sys_get_shader("layer_view.vert");
		pipes_copy_bgra.fragment_shader = sys_get_shader("layer_copy_bgra.frag");
		let vs: kinc_g5_vertex_structure_t = g4_vertex_struct_create();
		g4_vertex_struct_add(vs, "pos", vertex_data_t.F32_3X);
		g4_vertex_struct_add(vs, "tex", vertex_data_t.F32_2X);
		g4_vertex_struct_add(vs, "col", vertex_data_t.U8_4X_NORM);
		pipes_copy_bgra.input_layout = vs;
		iron_g4_compile_pipeline(pipes_copy_bgra);
	}

	{
		pipes_copy8 = iron_g4_create_pipeline();
		pipes_copy8.vertex_shader = sys_get_shader("layer_view.vert");
		pipes_copy8.fragment_shader = sys_get_shader("layer_copy.frag");
		let vs: kinc_g5_vertex_structure_t = g4_vertex_struct_create();
		g4_vertex_struct_add(vs, "pos", vertex_data_t.F32_3X);
		g4_vertex_struct_add(vs, "tex", vertex_data_t.F32_2X);
		g4_vertex_struct_add(vs, "col", vertex_data_t.U8_4X_NORM);
		pipes_copy8.input_layout = vs;
		pipes_copy8.color_attachment_count = 1;
		ARRAY_ACCESS(pipes_copy8.color_attachment, 0) = tex_format_t.R8;
		iron_g4_compile_pipeline(pipes_copy8);
	}

	{
		pipes_copy128 = iron_g4_create_pipeline();
		pipes_copy128.vertex_shader = sys_get_shader("layer_view.vert");
		pipes_copy128.fragment_shader = sys_get_shader("layer_copy.frag");
		let vs: kinc_g5_vertex_structure_t = g4_vertex_struct_create();
		g4_vertex_struct_add(vs, "pos", vertex_data_t.F32_3X);
		g4_vertex_struct_add(vs, "tex", vertex_data_t.F32_2X);
		g4_vertex_struct_add(vs, "col", vertex_data_t.U8_4X_NORM);
		pipes_copy128.input_layout = vs;
		pipes_copy128.color_attachment_count = 1;
		ARRAY_ACCESS(pipes_copy128.color_attachment, 0) = tex_format_t.RGBA128;
		iron_g4_compile_pipeline(pipes_copy128);
	}

	///if (is_paint || is_sculpt)
	{
		pipes_invert8 = iron_g4_create_pipeline();
		pipes_invert8.vertex_shader = sys_get_shader("layer_view.vert");
		pipes_invert8.fragment_shader = sys_get_shader("layer_invert.frag");
		let vs: kinc_g5_vertex_structure_t = g4_vertex_struct_create();
		g4_vertex_struct_add(vs, "pos", vertex_data_t.F32_3X);
		g4_vertex_struct_add(vs, "tex", vertex_data_t.F32_2X);
		g4_vertex_struct_add(vs, "col", vertex_data_t.U8_4X_NORM);
		pipes_invert8.input_layout = vs;
		pipes_invert8.color_attachment_count = 1;
		ARRAY_ACCESS(pipes_invert8.color_attachment, 0) = tex_format_t.R8;
		iron_g4_compile_pipeline(pipes_invert8);
	}

	{
		pipes_apply_mask = iron_g4_create_pipeline();
		pipes_apply_mask.vertex_shader = sys_get_shader("pass.vert");
		pipes_apply_mask.fragment_shader = sys_get_shader("mask_apply.frag");
		let vs: kinc_g5_vertex_structure_t = g4_vertex_struct_create();
		g4_vertex_struct_add(vs, "pos", vertex_data_t.F32_2X);
		pipes_apply_mask.input_layout = vs;
		iron_g4_compile_pipeline(pipes_apply_mask);
		pipes_tex0_mask = iron_g4_get_texture_unit(pipes_apply_mask, "tex0");
		pipes_texa_mask = iron_g4_get_texture_unit(pipes_apply_mask, "texa");
	}

	{
		pipes_merge_mask = iron_g4_create_pipeline();
		pipes_merge_mask.vertex_shader = sys_get_shader("pass.vert");
		pipes_merge_mask.fragment_shader = sys_get_shader("mask_merge.frag");
		let vs: kinc_g5_vertex_structure_t = g4_vertex_struct_create();
		g4_vertex_struct_add(vs, "pos", vertex_data_t.F32_2X);
		pipes_merge_mask.input_layout = vs;
		iron_g4_compile_pipeline(pipes_merge_mask);
		pipes_tex0_merge_mask = iron_g4_get_texture_unit(pipes_merge_mask, "tex0");
		pipes_texa_merge_mask = iron_g4_get_texture_unit(pipes_merge_mask, "texa");
		pipes_opac_merge_mask = iron_g4_get_constant_location(pipes_merge_mask, "opac");
		pipes_blending_merge_mask = iron_g4_get_constant_location(pipes_merge_mask, "blending");
	}

	{
		pipes_colorid_to_mask = iron_g4_create_pipeline();
		pipes_colorid_to_mask.vertex_shader = sys_get_shader("pass.vert");
		pipes_colorid_to_mask.fragment_shader = sys_get_shader("mask_colorid.frag");
		let vs: kinc_g5_vertex_structure_t = g4_vertex_struct_create();
		g4_vertex_struct_add(vs, "pos", vertex_data_t.F32_2X);
		pipes_colorid_to_mask.input_layout = vs;
		iron_g4_compile_pipeline(pipes_colorid_to_mask);
		pipes_texpaint_colorid = iron_g4_get_texture_unit(pipes_colorid_to_mask, "texpaint_colorid");
		pipes_tex_colorid = iron_g4_get_texture_unit(pipes_colorid_to_mask, "texcolorid");
	}
	///end

	///if is_lab
	{
		pipes_copy_r = iron_g4_create_pipeline();
		pipes_copy_r.vertex_shader = sys_get_shader("layer_view.vert");
		pipes_copy_r.fragment_shader = sys_get_shader("layer_copy.frag");
		let vs: kinc_g5_vertex_structure_t = g4_vertex_struct_create();
		g4_vertex_struct_add(vs, "pos", vertex_data_t.F32_3X);
		g4_vertex_struct_add(vs, "tex", vertex_data_t.F32_2X);
		g4_vertex_struct_add(vs, "col", vertex_data_t.U8_4X_NORM);
		pipes_copy_r.input_layout = vs;
		ARRAY_ACCESS(pipes_copy_r.color_write_mask_green, 0) = false;
		ARRAY_ACCESS(pipes_copy_r.color_write_mask_blue, 0) = false;
		ARRAY_ACCESS(pipes_copy_r.color_write_mask_alpha, 0) = false;
		iron_g4_compile_pipeline(pipes_copy_r);
	}

	{
		pipes_copy_g = iron_g4_create_pipeline();
		pipes_copy_g.vertex_shader = sys_get_shader("layer_view.vert");
		pipes_copy_g.fragment_shader = sys_get_shader("layer_copy.frag");
		let vs: kinc_g5_vertex_structure_t = g4_vertex_struct_create();
		g4_vertex_struct_add(vs, "pos", vertex_data_t.F32_3X);
		g4_vertex_struct_add(vs, "tex", vertex_data_t.F32_2X);
		g4_vertex_struct_add(vs, "col", vertex_data_t.U8_4X_NORM);
		pipes_copy_g.input_layout = vs;
		ARRAY_ACCESS(pipes_copy_g.color_write_mask_red, 0) = false;
		ARRAY_ACCESS(pipes_copy_g.color_write_mask_blue, 0) = false;
		ARRAY_ACCESS(pipes_copy_g.color_write_mask_alpha, 0) = false;
		iron_g4_compile_pipeline(pipes_copy_g);
	}

	{
		pipes_inpaint_preview = iron_g4_create_pipeline();
		pipes_inpaint_preview.vertex_shader = sys_get_shader("pass.vert");
		pipes_inpaint_preview.fragment_shader = sys_get_shader("inpaint_preview.frag");
		let vs: kinc_g5_vertex_structure_t = g4_vertex_struct_create();
		g4_vertex_struct_add(vs, "pos", vertex_data_t.F32_2X);
		pipes_inpaint_preview.input_layout = vs;
		iron_g4_compile_pipeline(pipes_inpaint_preview);
		pipes_tex0_inpaint_preview = iron_g4_get_texture_unit(pipes_inpaint_preview, "tex0");
		pipes_texa_inpaint_preview = iron_g4_get_texture_unit(pipes_inpaint_preview, "texa");
	}

	{
		pipes_copy_a = iron_g4_create_pipeline();
		pipes_copy_a.vertex_shader = sys_get_shader("pass.vert");
		pipes_copy_a.fragment_shader = sys_get_shader("layer_copy_rrrr.frag");
		let vs: kinc_g5_vertex_structure_t = g4_vertex_struct_create();
		g4_vertex_struct_add(vs, "pos", vertex_data_t.F32_2X);
		pipes_copy_a.input_layout = vs;
		ARRAY_ACCESS(pipes_copy_a.color_write_mask_red, 0) = false;
		ARRAY_ACCESS(pipes_copy_a.color_write_mask_green, 0) = false;
		ARRAY_ACCESS(pipes_copy_a.color_write_mask_blue, 0) = false;
		iron_g4_compile_pipeline(pipes_copy_a);
		pipes_copy_a_tex = iron_g4_get_texture_unit(pipes_copy_a, "tex");
	}
	///end

	{
		pipes_copy_rgb = iron_g4_create_pipeline();
		pipes_copy_rgb.vertex_shader = sys_get_shader("layer_view.vert");
		pipes_copy_rgb.fragment_shader = sys_get_shader("layer_copy.frag");
		let vs: kinc_g5_vertex_structure_t = g4_vertex_struct_create();
		g4_vertex_struct_add(vs, "pos", vertex_data_t.F32_3X);
		g4_vertex_struct_add(vs, "tex", vertex_data_t.F32_2X);
		g4_vertex_struct_add(vs, "col", vertex_data_t.U8_4X_NORM);
		pipes_copy_rgb.input_layout = vs;
		ARRAY_ACCESS(pipes_copy_rgb.color_write_mask_alpha, 0) = false;
		iron_g4_compile_pipeline(pipes_copy_rgb);
	}

	{
		pipes_cursor = iron_g4_create_pipeline();
		pipes_cursor.vertex_shader = sys_get_shader("cursor.vert");
		pipes_cursor.fragment_shader = sys_get_shader("cursor.frag");
		let vs: kinc_g5_vertex_structure_t = g4_vertex_struct_create();
		///if (arm_metal || arm_vulkan)
		g4_vertex_struct_add(vs, "tex", vertex_data_t.I16_2X_NORM);
		///else
		g4_vertex_struct_add(vs, "pos", vertex_data_t.I16_4X_NORM);
		g4_vertex_struct_add(vs, "nor", vertex_data_t.I16_2X_NORM);
		g4_vertex_struct_add(vs, "tex", vertex_data_t.I16_2X_NORM);
		///end
		pipes_cursor.input_layout = vs;
		pipes_cursor.blend_source = blend_factor_t.SOURCE_ALPHA;
		pipes_cursor.blend_destination = blend_factor_t.INV_SOURCE_ALPHA;
		pipes_cursor.depth_write = false;
		pipes_cursor.depth_mode = compare_mode_t.ALWAYS;
		iron_g4_compile_pipeline(pipes_cursor);
		pipes_cursor_vp = iron_g4_get_constant_location(pipes_cursor, "VP");
		pipes_cursor_inv_vp = iron_g4_get_constant_location(pipes_cursor, "invVP");
		pipes_cursor_mouse = iron_g4_get_constant_location(pipes_cursor, "mouse");
		pipes_cursor_tex_step = iron_g4_get_constant_location(pipes_cursor, "tex_step");
		pipes_cursor_radius = iron_g4_get_constant_location(pipes_cursor, "radius");
		pipes_cursor_camera_right = iron_g4_get_constant_location(pipes_cursor, "camera_right");
		pipes_cursor_tint = iron_g4_get_constant_location(pipes_cursor, "tint");
		pipes_cursor_gbufferd = iron_g4_get_texture_unit(pipes_cursor, "gbufferD");
	}
}
