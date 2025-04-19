
let pipes_copy: iron_gpu_pipeline_t;
let pipes_copy8: iron_gpu_pipeline_t;
let pipes_copy128: iron_gpu_pipeline_t;
let pipes_copy_bgra: iron_gpu_pipeline_t;
let pipes_copy_rgb: iron_gpu_pipeline_t = null;
let pipes_copy_r: iron_gpu_pipeline_t;
let pipes_copy_g: iron_gpu_pipeline_t;
let pipes_copy_a: iron_gpu_pipeline_t;
let pipes_copy_a_tex: iron_gpu_texture_unit_t;

let pipes_merge: iron_gpu_pipeline_t = null;
let pipes_merge_r: iron_gpu_pipeline_t = null;
let pipes_merge_g: iron_gpu_pipeline_t = null;
let pipes_merge_b: iron_gpu_pipeline_t = null;
let pipes_merge_mask: iron_gpu_pipeline_t;

let pipes_invert8: iron_gpu_pipeline_t;
let pipes_apply_mask: iron_gpu_pipeline_t;
let pipes_colorid_to_mask: iron_gpu_pipeline_t;

let pipes_tex0: iron_gpu_texture_unit_t;
let pipes_tex1: iron_gpu_texture_unit_t;
let pipes_texmask: iron_gpu_texture_unit_t;
let pipes_texa: iron_gpu_texture_unit_t;
let pipes_opac: iron_gpu_constant_location_t;
let pipes_blending: iron_gpu_constant_location_t;
let pipes_tex1w: iron_gpu_constant_location_t;
let pipes_tex0_mask: iron_gpu_texture_unit_t;
let pipes_texa_mask: iron_gpu_texture_unit_t;
let pipes_tex0_merge_mask: iron_gpu_texture_unit_t;
let pipes_texa_merge_mask: iron_gpu_texture_unit_t;
let pipes_tex_colorid: iron_gpu_texture_unit_t;
let pipes_texpaint_colorid: iron_gpu_texture_unit_t;
let pipes_opac_merge_mask: iron_gpu_constant_location_t;
let pipes_blending_merge_mask: iron_gpu_constant_location_t;
let pipes_temp_mask_image: iron_gpu_texture_t = null;

let pipes_inpaint_preview: iron_gpu_pipeline_t;
let pipes_tex0_inpaint_preview: iron_gpu_texture_unit_t;
let pipes_texa_inpaint_preview: iron_gpu_texture_unit_t;

let pipes_cursor: iron_gpu_pipeline_t;
let pipes_cursor_vp: iron_gpu_constant_location_t;
let pipes_cursor_inv_vp: iron_gpu_constant_location_t;
let pipes_cursor_mouse: iron_gpu_constant_location_t;
let pipes_cursor_tex_step: iron_gpu_constant_location_t;
let pipes_cursor_radius: iron_gpu_constant_location_t;
let pipes_cursor_camera_right: iron_gpu_constant_location_t;
let pipes_cursor_tint: iron_gpu_constant_location_t;
let pipes_cursor_gbufferd: iron_gpu_texture_unit_t;

let pipes_offset: i32;

function _pipes_make_merge(red: bool, green: bool, blue: bool, alpha: bool): iron_gpu_pipeline_t {
	let pipe: iron_gpu_pipeline_t = gpu_create_pipeline();
	pipe.vertex_shader = sys_get_shader("layer_merge.vert");
	pipe.fragment_shader = sys_get_shader("layer_merge.frag");
	let vs: iron_gpu_vertex_structure_t = gpu_vertex_struct_create();
	gpu_vertex_struct_add(vs, "pos", vertex_data_t.F32_2X);
	pipe.input_layout = vs;
	ARRAY_ACCESS(pipe.color_write_mask_red, 0) = red;
	ARRAY_ACCESS(pipe.color_write_mask_green, 0) = green;
	ARRAY_ACCESS(pipe.color_write_mask_blue, 0) = blue;
	ARRAY_ACCESS(pipe.color_write_mask_alpha, 0) = alpha;
	gpu_compile_pipeline(pipe);
	return pipe;
}

function pipes_init() {
	///if (is_paint || is_sculpt)
	pipes_merge = _pipes_make_merge(true, true, true, true);
	pipes_merge_r = _pipes_make_merge(true, false, false, false);
	pipes_merge_g = _pipes_make_merge(false, true, false, false);
	pipes_merge_b = _pipes_make_merge(false, false, true, false);
	pipes_tex0 = gpu_get_texture_unit(pipes_merge, "tex0"); // Always binding texpaint.a for blending
	pipes_tex0.offset = 0;
	pipes_tex1 = gpu_get_texture_unit(pipes_merge, "tex1");
	pipes_tex1.offset = 1;
	pipes_texmask = gpu_get_texture_unit(pipes_merge, "texmask");
	pipes_texmask.offset = 2;
	pipes_texa = gpu_get_texture_unit(pipes_merge, "texa");
	pipes_texa.offset = 3;
	pipes_offset = 0;
	pipes_opac = pipes_get_constant_location(pipes_merge, "opac", "float");
	pipes_blending = pipes_get_constant_location(pipes_merge, "blending", "int");
	pipes_tex1w = pipes_get_constant_location(pipes_merge, "tex1w", "float");
	///end

	{
		pipes_copy = gpu_create_pipeline();
		pipes_copy.vertex_shader = sys_get_shader("layer_copy.vert");
		pipes_copy.fragment_shader = sys_get_shader("layer_copy.frag");
		let vs: iron_gpu_vertex_structure_t = gpu_vertex_struct_create();
		gpu_vertex_struct_add(vs, "pos", vertex_data_t.F32_3X);
		gpu_vertex_struct_add(vs, "tex", vertex_data_t.F32_2X);
		gpu_vertex_struct_add(vs, "col", vertex_data_t.U8_4X_NORM);
		pipes_copy.input_layout = vs;
		gpu_compile_pipeline(pipes_copy);
	}

	{
		pipes_copy_bgra = gpu_create_pipeline();
		pipes_copy_bgra.vertex_shader = sys_get_shader("layer_copy_bgra.vert");
		pipes_copy_bgra.fragment_shader = sys_get_shader("layer_copy_bgra.frag");
		let vs: iron_gpu_vertex_structure_t = gpu_vertex_struct_create();
		gpu_vertex_struct_add(vs, "pos", vertex_data_t.F32_3X);
		gpu_vertex_struct_add(vs, "tex", vertex_data_t.F32_2X);
		gpu_vertex_struct_add(vs, "col", vertex_data_t.U8_4X_NORM);
		pipes_copy_bgra.input_layout = vs;
		gpu_compile_pipeline(pipes_copy_bgra);
	}

	{
		pipes_copy8 = gpu_create_pipeline();
		pipes_copy8.vertex_shader = sys_get_shader("layer_copy.vert");
		pipes_copy8.fragment_shader = sys_get_shader("layer_copy.frag");
		let vs: iron_gpu_vertex_structure_t = gpu_vertex_struct_create();
		gpu_vertex_struct_add(vs, "pos", vertex_data_t.F32_3X);
		gpu_vertex_struct_add(vs, "tex", vertex_data_t.F32_2X);
		gpu_vertex_struct_add(vs, "col", vertex_data_t.U8_4X_NORM);
		pipes_copy8.input_layout = vs;
		pipes_copy8.color_attachment_count = 1;
		ARRAY_ACCESS(pipes_copy8.color_attachment, 0) = tex_format_t.R8;
		gpu_compile_pipeline(pipes_copy8);
	}

	{
		pipes_copy128 = gpu_create_pipeline();
		pipes_copy128.vertex_shader = sys_get_shader("layer_copy.vert");
		pipes_copy128.fragment_shader = sys_get_shader("layer_copy.frag");
		let vs: iron_gpu_vertex_structure_t = gpu_vertex_struct_create();
		gpu_vertex_struct_add(vs, "pos", vertex_data_t.F32_3X);
		gpu_vertex_struct_add(vs, "tex", vertex_data_t.F32_2X);
		gpu_vertex_struct_add(vs, "col", vertex_data_t.U8_4X_NORM);
		pipes_copy128.input_layout = vs;
		pipes_copy128.color_attachment_count = 1;
		ARRAY_ACCESS(pipes_copy128.color_attachment, 0) = tex_format_t.RGBA128;
		gpu_compile_pipeline(pipes_copy128);
	}

	///if (is_paint || is_sculpt)
	{
		pipes_invert8 = gpu_create_pipeline();
		pipes_invert8.vertex_shader = sys_get_shader("layer_invert.vert");
		pipes_invert8.fragment_shader = sys_get_shader("layer_invert.frag");
		let vs: iron_gpu_vertex_structure_t = gpu_vertex_struct_create();
		gpu_vertex_struct_add(vs, "pos", vertex_data_t.F32_3X);
		gpu_vertex_struct_add(vs, "tex", vertex_data_t.F32_2X);
		gpu_vertex_struct_add(vs, "col", vertex_data_t.U8_4X_NORM);
		pipes_invert8.input_layout = vs;
		pipes_invert8.color_attachment_count = 1;
		ARRAY_ACCESS(pipes_invert8.color_attachment, 0) = tex_format_t.R8;
		gpu_compile_pipeline(pipes_invert8);
	}

	{
		pipes_apply_mask = gpu_create_pipeline();
		pipes_apply_mask.vertex_shader = sys_get_shader("mask_apply.vert");
		pipes_apply_mask.fragment_shader = sys_get_shader("mask_apply.frag");
		let vs: iron_gpu_vertex_structure_t = gpu_vertex_struct_create();
		gpu_vertex_struct_add(vs, "pos", vertex_data_t.F32_2X);
		pipes_apply_mask.input_layout = vs;
		gpu_compile_pipeline(pipes_apply_mask);
		pipes_tex0_mask = gpu_get_texture_unit(pipes_apply_mask, "tex0");
		pipes_tex0_mask.offset = 0;
		pipes_texa_mask = gpu_get_texture_unit(pipes_apply_mask, "texa");
		pipes_texa_mask.offset = 1;
	}

	{
		pipes_merge_mask = gpu_create_pipeline();
		pipes_merge_mask.vertex_shader = sys_get_shader("mask_merge.vert");
		pipes_merge_mask.fragment_shader = sys_get_shader("mask_merge.frag");
		let vs: iron_gpu_vertex_structure_t = gpu_vertex_struct_create();
		gpu_vertex_struct_add(vs, "pos", vertex_data_t.F32_2X);
		pipes_merge_mask.input_layout = vs;
		gpu_compile_pipeline(pipes_merge_mask);
		pipes_tex0_merge_mask = gpu_get_texture_unit(pipes_merge_mask, "tex0");
		pipes_tex0_merge_mask.offset = 0;
		pipes_texa_merge_mask = gpu_get_texture_unit(pipes_merge_mask, "texa");
		pipes_texa_merge_mask.offset = 1;
		pipes_offset = 0;
		pipes_opac_merge_mask = pipes_get_constant_location(pipes_merge_mask, "opac", "float");
		pipes_blending_merge_mask = pipes_get_constant_location(pipes_merge_mask, "blending", "int");
	}

	{
		pipes_colorid_to_mask = gpu_create_pipeline();
		pipes_colorid_to_mask.vertex_shader = sys_get_shader("mask_colorid.vert");
		pipes_colorid_to_mask.fragment_shader = sys_get_shader("mask_colorid.frag");
		let vs: iron_gpu_vertex_structure_t = gpu_vertex_struct_create();
		gpu_vertex_struct_add(vs, "pos", vertex_data_t.F32_2X);
		pipes_colorid_to_mask.input_layout = vs;
		gpu_compile_pipeline(pipes_colorid_to_mask);
		pipes_texpaint_colorid = gpu_get_texture_unit(pipes_colorid_to_mask, "texpaint_colorid");
		pipes_texpaint_colorid.offset = 0;
		pipes_tex_colorid = gpu_get_texture_unit(pipes_colorid_to_mask, "texcolorid");
		pipes_tex_colorid.offset = 1;
	}
	///end

	///if is_lab
	{
		pipes_copy_r = gpu_create_pipeline();
		pipes_copy_r.vertex_shader = sys_get_shader("layer_copy.vert");
		pipes_copy_r.fragment_shader = sys_get_shader("layer_copy.frag");
		let vs: iron_gpu_vertex_structure_t = gpu_vertex_struct_create();
		gpu_vertex_struct_add(vs, "pos", vertex_data_t.F32_3X);
		gpu_vertex_struct_add(vs, "tex", vertex_data_t.F32_2X);
		gpu_vertex_struct_add(vs, "col", vertex_data_t.U8_4X_NORM);
		pipes_copy_r.input_layout = vs;
		ARRAY_ACCESS(pipes_copy_r.color_write_mask_green, 0) = false;
		ARRAY_ACCESS(pipes_copy_r.color_write_mask_blue, 0) = false;
		ARRAY_ACCESS(pipes_copy_r.color_write_mask_alpha, 0) = false;
		gpu_compile_pipeline(pipes_copy_r);
	}

	{
		pipes_copy_g = gpu_create_pipeline();
		pipes_copy_g.vertex_shader = sys_get_shader("layer_copy.vert");
		pipes_copy_g.fragment_shader = sys_get_shader("layer_copy.frag");
		let vs: iron_gpu_vertex_structure_t = gpu_vertex_struct_create();
		gpu_vertex_struct_add(vs, "pos", vertex_data_t.F32_3X);
		gpu_vertex_struct_add(vs, "tex", vertex_data_t.F32_2X);
		gpu_vertex_struct_add(vs, "col", vertex_data_t.U8_4X_NORM);
		pipes_copy_g.input_layout = vs;
		ARRAY_ACCESS(pipes_copy_g.color_write_mask_red, 0) = false;
		ARRAY_ACCESS(pipes_copy_g.color_write_mask_blue, 0) = false;
		ARRAY_ACCESS(pipes_copy_g.color_write_mask_alpha, 0) = false;
		gpu_compile_pipeline(pipes_copy_g);
	}

	{
		pipes_inpaint_preview = gpu_create_pipeline();
		pipes_inpaint_preview.vertex_shader = sys_get_shader("inpaint_preview.vert");
		pipes_inpaint_preview.fragment_shader = sys_get_shader("inpaint_preview.frag");
		let vs: iron_gpu_vertex_structure_t = gpu_vertex_struct_create();
		gpu_vertex_struct_add(vs, "pos", vertex_data_t.F32_2X);
		pipes_inpaint_preview.input_layout = vs;
		gpu_compile_pipeline(pipes_inpaint_preview);
		pipes_tex0_inpaint_preview = gpu_get_texture_unit(pipes_inpaint_preview, "tex0");
		pipes_tex0_inpaint_preview.offset = 0;
		pipes_texa_inpaint_preview = gpu_get_texture_unit(pipes_inpaint_preview, "texa");
		pipes_texa_inpaint_preview.offset = 1;
	}

	{
		pipes_copy_a = gpu_create_pipeline();
		pipes_copy_a.vertex_shader = sys_get_shader("layer_copy_rrrr.vert");
		pipes_copy_a.fragment_shader = sys_get_shader("layer_copy_rrrr.frag");
		let vs: iron_gpu_vertex_structure_t = gpu_vertex_struct_create();
		gpu_vertex_struct_add(vs, "pos", vertex_data_t.F32_2X);
		pipes_copy_a.input_layout = vs;
		ARRAY_ACCESS(pipes_copy_a.color_write_mask_red, 0) = false;
		ARRAY_ACCESS(pipes_copy_a.color_write_mask_green, 0) = false;
		ARRAY_ACCESS(pipes_copy_a.color_write_mask_blue, 0) = false;
		gpu_compile_pipeline(pipes_copy_a);
		pipes_copy_a_tex = gpu_get_texture_unit(pipes_copy_a, "tex");
		pipes_copy_a_tex.offset = 0;
	}
	///end

	{
		pipes_copy_rgb = gpu_create_pipeline();
		pipes_copy_rgb.vertex_shader = sys_get_shader("layer_copy.vert");
		pipes_copy_rgb.fragment_shader = sys_get_shader("layer_copy.frag");
		let vs: iron_gpu_vertex_structure_t = gpu_vertex_struct_create();
		gpu_vertex_struct_add(vs, "pos", vertex_data_t.F32_3X);
		gpu_vertex_struct_add(vs, "tex", vertex_data_t.F32_2X);
		gpu_vertex_struct_add(vs, "col", vertex_data_t.U8_4X_NORM);
		pipes_copy_rgb.input_layout = vs;
		ARRAY_ACCESS(pipes_copy_rgb.color_write_mask_alpha, 0) = false;
		gpu_compile_pipeline(pipes_copy_rgb);
	}

	{
		pipes_cursor = gpu_create_pipeline();
		pipes_cursor.vertex_shader = sys_get_shader("cursor.vert");
		pipes_cursor.fragment_shader = sys_get_shader("cursor.frag");
		let vs: iron_gpu_vertex_structure_t = gpu_vertex_struct_create();
		gpu_vertex_struct_add(vs, "pos", vertex_data_t.I16_4X_NORM);
		gpu_vertex_struct_add(vs, "nor", vertex_data_t.I16_2X_NORM);
		gpu_vertex_struct_add(vs, "tex", vertex_data_t.I16_2X_NORM);
		pipes_cursor.input_layout = vs;
		pipes_cursor.blend_source = blend_factor_t.SOURCE_ALPHA;
		pipes_cursor.blend_destination = blend_factor_t.INV_SOURCE_ALPHA;
		pipes_cursor.depth_write = false;
		pipes_cursor.depth_mode = compare_mode_t.ALWAYS;
		gpu_compile_pipeline(pipes_cursor);
		pipes_offset = 0;
		pipes_cursor_vp = pipes_get_constant_location(pipes_cursor, "VP", "mat4");
		pipes_cursor_inv_vp = pipes_get_constant_location(pipes_cursor, "invVP", "mat4");
		pipes_cursor_mouse = pipes_get_constant_location(pipes_cursor, "mouse", "vec2");
		pipes_cursor_tex_step = pipes_get_constant_location(pipes_cursor, "tex_step", "vec2");
		pipes_cursor_radius = pipes_get_constant_location(pipes_cursor, "radius", "float");
		pipes_cursor_camera_right = pipes_get_constant_location(pipes_cursor, "camera_right", "vec3");
		pipes_cursor_tint = pipes_get_constant_location(pipes_cursor, "tint", "vec3");
		pipes_cursor_gbufferd = gpu_get_texture_unit(pipes_cursor, "gbufferD");
		pipes_cursor_gbufferd.offset = 0;
	}
}

function pipes_get_constant_location(pipe: iron_gpu_pipeline_t, name: string, type: string): iron_gpu_constant_location_t {
	let loc: iron_gpu_constant_location_t = gpu_get_constant_location(pipe, name);
	let ptr: gpu_constant_location_impl_t = ADDRESS(loc.impl);
	let size: i32 = shader_context_type_size(type);
	pipes_offset += shader_context_type_pad(pipes_offset, size);
	ptr.vertexOffset = pipes_offset;
	pipes_offset += size;
	return loc;
}
