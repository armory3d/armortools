
let pipes_copy: gpu_pipeline_t;
let pipes_copy8: gpu_pipeline_t;
let pipes_copy128: gpu_pipeline_t;
let pipes_copy_bgra: gpu_pipeline_t;
let pipes_copy_rgb: gpu_pipeline_t = null;
let pipes_copy_r: gpu_pipeline_t;
let pipes_copy_g: gpu_pipeline_t;
let pipes_copy_a: gpu_pipeline_t;
let pipes_copy_a_tex: i32;

let pipes_merge: gpu_pipeline_t = null;
let pipes_merge_r: gpu_pipeline_t = null;
let pipes_merge_g: gpu_pipeline_t = null;
let pipes_merge_b: gpu_pipeline_t = null;
let pipes_merge_mask: gpu_pipeline_t;

let pipes_invert8: gpu_pipeline_t;
let pipes_apply_mask: gpu_pipeline_t;
let pipes_colorid_to_mask: gpu_pipeline_t;

let pipes_tex0: i32;
let pipes_tex1: i32;
let pipes_texmask: i32;
let pipes_texa: i32;
let pipes_opac: i32;
let pipes_blending: i32;
let pipes_tex1w: i32;
let pipes_tex0_mask: i32;
let pipes_texa_mask: i32;
let pipes_tex0_merge_mask: i32;
let pipes_texa_merge_mask: i32;
let pipes_tex_colorid: i32;
let pipes_texpaint_colorid: i32;
let pipes_opac_merge_mask: i32;
let pipes_blending_merge_mask: i32;
let pipes_temp_mask_image: gpu_texture_t = null;

let pipes_inpaint_preview: gpu_pipeline_t;
let pipes_tex0_inpaint_preview: i32;
let pipes_texa_inpaint_preview: i32;

let pipes_cursor: gpu_pipeline_t;
let pipes_cursor_vp: i32;
let pipes_cursor_inv_vp: i32;
let pipes_cursor_mouse: i32;
let pipes_cursor_tex_step: i32;
let pipes_cursor_radius: i32;
let pipes_cursor_camera_right: i32;
let pipes_cursor_tint: i32;
let pipes_cursor_gbufferd: i32;

let pipes_offset: i32;

function _pipes_make_merge(red: bool, green: bool, blue: bool, alpha: bool): gpu_pipeline_t {
	let pipe: gpu_pipeline_t = gpu_create_pipeline();
	pipe.vertex_shader = sys_get_shader("layer_merge.vert");
	pipe.fragment_shader = sys_get_shader("layer_merge.frag");
	let vs: gpu_vertex_structure_t = gpu_vertex_struct_create();
	gpu_vertex_struct_add(vs, "pos", vertex_data_t.F32_2X);
	pipe.input_layout = vs;
	ARRAY_ACCESS(pipe.color_write_mask_red, 0) = red;
	ARRAY_ACCESS(pipe.color_write_mask_green, 0) = green;
	ARRAY_ACCESS(pipe.color_write_mask_blue, 0) = blue;
	ARRAY_ACCESS(pipe.color_write_mask_alpha, 0) = alpha;
	gpu_pipeline_compile(pipe);
	return pipe;
}

function pipes_init() {
	///if (is_paint || is_sculpt)
	pipes_merge = _pipes_make_merge(true, true, true, true);
	pipes_merge_r = _pipes_make_merge(true, false, false, false);
	pipes_merge_g = _pipes_make_merge(false, true, false, false);
	pipes_merge_b = _pipes_make_merge(false, false, true, false);
	pipes_tex0 = 0; // Always binding texpaint.a for blending
	pipes_tex1 = 1;
	pipes_texmask = 2;
	pipes_texa = 3;
	pipes_offset = 0;
	pipes_opac = pipes_get_constant_location("float");
	pipes_blending = pipes_get_constant_location("int");
	pipes_tex1w = pipes_get_constant_location("float");
	///end

	{
		pipes_copy = gpu_create_pipeline();
		pipes_copy.vertex_shader = sys_get_shader("layer_copy.vert");
		pipes_copy.fragment_shader = sys_get_shader("layer_copy.frag");
		let vs: gpu_vertex_structure_t = gpu_vertex_struct_create();
		gpu_vertex_struct_add(vs, "pos", vertex_data_t.F32_2X);
		pipes_copy.input_layout = vs;
		gpu_pipeline_compile(pipes_copy);
	}

	{
		pipes_copy_bgra = gpu_create_pipeline();
		pipes_copy_bgra.vertex_shader = sys_get_shader("layer_copy_bgra.vert");
		pipes_copy_bgra.fragment_shader = sys_get_shader("layer_copy_bgra.frag");
		let vs: gpu_vertex_structure_t = gpu_vertex_struct_create();
		gpu_vertex_struct_add(vs, "pos", vertex_data_t.F32_2X);
		pipes_copy_bgra.input_layout = vs;
		gpu_pipeline_compile(pipes_copy_bgra);
	}

	{
		pipes_copy8 = gpu_create_pipeline();
		pipes_copy8.vertex_shader = sys_get_shader("layer_copy.vert");
		pipes_copy8.fragment_shader = sys_get_shader("layer_copy.frag");
		let vs: gpu_vertex_structure_t = gpu_vertex_struct_create();
		gpu_vertex_struct_add(vs, "pos", vertex_data_t.F32_2X);
		pipes_copy8.input_layout = vs;
		pipes_copy8.color_attachment_count = 1;
		ARRAY_ACCESS(pipes_copy8.color_attachment, 0) = tex_format_t.R8;
		gpu_pipeline_compile(pipes_copy8);
	}

	{
		pipes_copy128 = gpu_create_pipeline();
		pipes_copy128.vertex_shader = sys_get_shader("layer_copy.vert");
		pipes_copy128.fragment_shader = sys_get_shader("layer_copy.frag");
		let vs: gpu_vertex_structure_t = gpu_vertex_struct_create();
		gpu_vertex_struct_add(vs, "pos", vertex_data_t.F32_2X);
		pipes_copy128.input_layout = vs;
		pipes_copy128.color_attachment_count = 1;
		ARRAY_ACCESS(pipes_copy128.color_attachment, 0) = tex_format_t.RGBA128;
		gpu_pipeline_compile(pipes_copy128);
	}

	///if (is_paint || is_sculpt)
	{
		pipes_invert8 = gpu_create_pipeline();
		pipes_invert8.vertex_shader = sys_get_shader("layer_invert.vert");
		pipes_invert8.fragment_shader = sys_get_shader("layer_invert.frag");
		let vs: gpu_vertex_structure_t = gpu_vertex_struct_create();
		gpu_vertex_struct_add(vs, "pos", vertex_data_t.F32_2X);
		pipes_invert8.input_layout = vs;
		pipes_invert8.color_attachment_count = 1;
		ARRAY_ACCESS(pipes_invert8.color_attachment, 0) = tex_format_t.R8;
		gpu_pipeline_compile(pipes_invert8);
	}

	{
		pipes_apply_mask = gpu_create_pipeline();
		pipes_apply_mask.vertex_shader = sys_get_shader("mask_apply.vert");
		pipes_apply_mask.fragment_shader = sys_get_shader("mask_apply.frag");
		let vs: gpu_vertex_structure_t = gpu_vertex_struct_create();
		gpu_vertex_struct_add(vs, "pos", vertex_data_t.F32_2X);
		pipes_apply_mask.input_layout = vs;
		gpu_pipeline_compile(pipes_apply_mask);
		pipes_tex0_mask = 0;
		pipes_texa_mask = 1;
	}

	{
		pipes_merge_mask = gpu_create_pipeline();
		pipes_merge_mask.vertex_shader = sys_get_shader("mask_merge.vert");
		pipes_merge_mask.fragment_shader = sys_get_shader("mask_merge.frag");
		let vs: gpu_vertex_structure_t = gpu_vertex_struct_create();
		gpu_vertex_struct_add(vs, "pos", vertex_data_t.F32_2X);
		pipes_merge_mask.input_layout = vs;
		gpu_pipeline_compile(pipes_merge_mask);
		pipes_tex0_merge_mask = 0;
		pipes_texa_merge_mask = 1;
		pipes_offset = 0;
		pipes_opac_merge_mask = pipes_get_constant_location("float");
		pipes_blending_merge_mask = pipes_get_constant_location("int");
	}

	{
		pipes_colorid_to_mask = gpu_create_pipeline();
		pipes_colorid_to_mask.vertex_shader = sys_get_shader("mask_colorid.vert");
		pipes_colorid_to_mask.fragment_shader = sys_get_shader("mask_colorid.frag");
		let vs: gpu_vertex_structure_t = gpu_vertex_struct_create();
		gpu_vertex_struct_add(vs, "pos", vertex_data_t.F32_2X);
		pipes_colorid_to_mask.input_layout = vs;
		gpu_pipeline_compile(pipes_colorid_to_mask);
		pipes_texpaint_colorid = 0;
		pipes_tex_colorid = 1;
	}
	///end

	///if is_lab
	{
		pipes_copy_r = gpu_create_pipeline();
		pipes_copy_r.vertex_shader = sys_get_shader("layer_copy.vert");
		pipes_copy_r.fragment_shader = sys_get_shader("layer_copy.frag");
		let vs: gpu_vertex_structure_t = gpu_vertex_struct_create();
		gpu_vertex_struct_add(vs, "pos", vertex_data_t.F32_2X);
		pipes_copy_r.input_layout = vs;
		ARRAY_ACCESS(pipes_copy_r.color_write_mask_green, 0) = false;
		ARRAY_ACCESS(pipes_copy_r.color_write_mask_blue, 0) = false;
		ARRAY_ACCESS(pipes_copy_r.color_write_mask_alpha, 0) = false;
		gpu_pipeline_compile(pipes_copy_r);
	}

	{
		pipes_copy_g = gpu_create_pipeline();
		pipes_copy_g.vertex_shader = sys_get_shader("layer_copy.vert");
		pipes_copy_g.fragment_shader = sys_get_shader("layer_copy.frag");
		let vs: gpu_vertex_structure_t = gpu_vertex_struct_create();
		gpu_vertex_struct_add(vs, "pos", vertex_data_t.F32_2X);
		pipes_copy_g.input_layout = vs;
		ARRAY_ACCESS(pipes_copy_g.color_write_mask_red, 0) = false;
		ARRAY_ACCESS(pipes_copy_g.color_write_mask_blue, 0) = false;
		ARRAY_ACCESS(pipes_copy_g.color_write_mask_alpha, 0) = false;
		gpu_pipeline_compile(pipes_copy_g);
	}

	{
		pipes_inpaint_preview = gpu_create_pipeline();
		pipes_inpaint_preview.vertex_shader = sys_get_shader("inpaint_preview.vert");
		pipes_inpaint_preview.fragment_shader = sys_get_shader("inpaint_preview.frag");
		let vs: gpu_vertex_structure_t = gpu_vertex_struct_create();
		gpu_vertex_struct_add(vs, "pos", vertex_data_t.F32_2X);
		pipes_inpaint_preview.input_layout = vs;
		gpu_pipeline_compile(pipes_inpaint_preview);
		pipes_tex0_inpaint_preview = 0;
		pipes_texa_inpaint_preview = 1;
	}

	{
		pipes_copy_a = gpu_create_pipeline();
		pipes_copy_a.vertex_shader = sys_get_shader("layer_copy_rrrr.vert");
		pipes_copy_a.fragment_shader = sys_get_shader("layer_copy_rrrr.frag");
		let vs: gpu_vertex_structure_t = gpu_vertex_struct_create();
		gpu_vertex_struct_add(vs, "pos", vertex_data_t.F32_2X);
		pipes_copy_a.input_layout = vs;
		ARRAY_ACCESS(pipes_copy_a.color_write_mask_red, 0) = false;
		ARRAY_ACCESS(pipes_copy_a.color_write_mask_green, 0) = false;
		ARRAY_ACCESS(pipes_copy_a.color_write_mask_blue, 0) = false;
		gpu_pipeline_compile(pipes_copy_a);
		pipes_copy_a_tex = 0;
	}
	///end

	{
		pipes_copy_rgb = gpu_create_pipeline();
		pipes_copy_rgb.vertex_shader = sys_get_shader("layer_copy.vert");
		pipes_copy_rgb.fragment_shader = sys_get_shader("layer_copy.frag");
		let vs: gpu_vertex_structure_t = gpu_vertex_struct_create();
		gpu_vertex_struct_add(vs, "pos", vertex_data_t.F32_2X);
		pipes_copy_rgb.input_layout = vs;
		ARRAY_ACCESS(pipes_copy_rgb.color_write_mask_alpha, 0) = false;
		gpu_pipeline_compile(pipes_copy_rgb);
	}

	{
		pipes_cursor = gpu_create_pipeline();
		pipes_cursor.vertex_shader = sys_get_shader("cursor.vert");
		pipes_cursor.fragment_shader = sys_get_shader("cursor.frag");
		let vs: gpu_vertex_structure_t = gpu_vertex_struct_create();
		gpu_vertex_struct_add(vs, "pos", vertex_data_t.I16_4X_NORM);
		gpu_vertex_struct_add(vs, "nor", vertex_data_t.I16_2X_NORM);
		gpu_vertex_struct_add(vs, "tex", vertex_data_t.I16_2X_NORM);
		gpu_vertex_struct_add(vs, "col", vertex_data_t.I16_4X_NORM);
		pipes_cursor.input_layout = vs;
		pipes_cursor.blend_source = blend_factor_t.SOURCE_ALPHA;
		pipes_cursor.blend_destination = blend_factor_t.INV_SOURCE_ALPHA;
		pipes_cursor.depth_write = false;
		pipes_cursor.depth_mode = compare_mode_t.ALWAYS;
		gpu_pipeline_compile(pipes_cursor);
		pipes_offset = 0;
		pipes_cursor_vp = pipes_get_constant_location("mat4");
		pipes_cursor_inv_vp = pipes_get_constant_location("mat4");
		pipes_cursor_mouse = pipes_get_constant_location("vec2");
		pipes_cursor_tex_step = pipes_get_constant_location("vec2");
		pipes_cursor_radius = pipes_get_constant_location("float");
		pipes_cursor_camera_right = pipes_get_constant_location("vec3");
		pipes_cursor_tint = pipes_get_constant_location("vec3");
		pipes_cursor_gbufferd = 0;
	}
}

function pipes_get_constant_location(type: string): i32 {
	let size: i32 = shader_context_type_size(type);
	pipes_offset += shader_context_type_pad(pipes_offset, size);
	let loc: i32 = pipes_offset;
	pipes_offset += size;
	return loc;
}
