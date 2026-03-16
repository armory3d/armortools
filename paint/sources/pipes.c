
#include "global.h"

gpu_pipeline_t *_pipes_make_merge(bool red, bool green, bool blue, bool alpha) {
	gpu_pipeline_t *pipe       = gpu_create_pipeline();
	pipe->vertex_shader        = sys_get_shader("layer_merge.vert");
	pipe->fragment_shader      = sys_get_shader("layer_merge.frag");
	gpu_vertex_structure_t *vs = GC_ALLOC_INIT(gpu_vertex_structure_t, {0});
	gpu_vertex_struct_add(vs, "pos", GPU_VERTEX_DATA_F32_2X);
	pipe->input_layout              = vs;
	pipe->color_write_mask_red[0]   = red;
	pipe->color_write_mask_green[0] = green;
	pipe->color_write_mask_blue[0]  = blue;
	pipe->color_write_mask_alpha[0] = alpha;
	gpu_pipeline_compile(pipe);
	return pipe;
}

void pipes_init() {

	pipes_merge = _pipes_make_merge(true, true, true, true);
	gc_root(pipes_merge);

	pipes_merge_r = _pipes_make_merge(true, false, false, false);
	gc_root(pipes_merge_r);

	pipes_merge_g = _pipes_make_merge(false, true, false, false);
	gc_root(pipes_merge_g);

	pipes_merge_b = _pipes_make_merge(false, false, true, false);
	gc_root(pipes_merge_b);
	pipes_tex0     = 0; // Always binding texpaint.a for blending
	pipes_tex1     = 1;
	pipes_texmask  = 2;
	pipes_texa     = 3;
	pipes_offset   = 0;
	pipes_opac     = pipes_get_constant_location("float");
	pipes_blending = pipes_get_constant_location("int");
	pipes_tex1w    = pipes_get_constant_location("float");

	{

		pipes_copy = gpu_create_pipeline();
		gc_root(pipes_copy);
		pipes_copy->vertex_shader   = sys_get_shader("layer_copy.vert");
		pipes_copy->fragment_shader = sys_get_shader("layer_copy.frag");
		gpu_vertex_structure_t *vs  = GC_ALLOC_INIT(gpu_vertex_structure_t, {0});
		gpu_vertex_struct_add(vs, "pos", GPU_VERTEX_DATA_F32_2X);
		pipes_copy->input_layout = vs;
		gpu_pipeline_compile(pipes_copy);
	}

	{

		pipes_copy_bgra = gpu_create_pipeline();
		gc_root(pipes_copy_bgra);
		pipes_copy_bgra->vertex_shader   = sys_get_shader("layer_copy_bgra.vert");
		pipes_copy_bgra->fragment_shader = sys_get_shader("layer_copy_bgra.frag");
		gpu_vertex_structure_t *vs       = GC_ALLOC_INIT(gpu_vertex_structure_t, {0});
		gpu_vertex_struct_add(vs, "pos", GPU_VERTEX_DATA_F32_2X);
		pipes_copy_bgra->input_layout = vs;
		gpu_pipeline_compile(pipes_copy_bgra);
	}

	{

		pipes_copy8 = gpu_create_pipeline();
		gc_root(pipes_copy8);
		pipes_copy8->vertex_shader   = sys_get_shader("layer_copy.vert");
		pipes_copy8->fragment_shader = sys_get_shader("layer_copy.frag");
		gpu_vertex_structure_t *vs   = GC_ALLOC_INIT(gpu_vertex_structure_t, {0});
		gpu_vertex_struct_add(vs, "pos", GPU_VERTEX_DATA_F32_2X);
		pipes_copy8->input_layout           = vs;
		pipes_copy8->color_attachment_count = 1;
		pipes_copy8->color_attachment[0]    = GPU_TEXTURE_FORMAT_R8;
		gpu_pipeline_compile(pipes_copy8);
	}

	{

		pipes_copy64 = gpu_create_pipeline();
		gc_root(pipes_copy64);
		pipes_copy64->vertex_shader   = sys_get_shader("layer_copy.vert");
		pipes_copy64->fragment_shader = sys_get_shader("layer_copy.frag");
		gpu_vertex_structure_t *vs    = GC_ALLOC_INIT(gpu_vertex_structure_t, {0});
		gpu_vertex_struct_add(vs, "pos", GPU_VERTEX_DATA_F32_2X);
		pipes_copy64->input_layout           = vs;
		pipes_copy64->color_attachment_count = 1;
		pipes_copy64->color_attachment[0]    = GPU_TEXTURE_FORMAT_RGBA64;
		gpu_pipeline_compile(pipes_copy64);
	}

	{

		pipes_copy128 = gpu_create_pipeline();
		gc_root(pipes_copy128);
		pipes_copy128->vertex_shader   = sys_get_shader("layer_copy.vert");
		pipes_copy128->fragment_shader = sys_get_shader("layer_copy.frag");
		gpu_vertex_structure_t *vs     = GC_ALLOC_INIT(gpu_vertex_structure_t, {0});
		gpu_vertex_struct_add(vs, "pos", GPU_VERTEX_DATA_F32_2X);
		pipes_copy128->input_layout           = vs;
		pipes_copy128->color_attachment_count = 1;
		pipes_copy128->color_attachment[0]    = GPU_TEXTURE_FORMAT_RGBA128;
		gpu_pipeline_compile(pipes_copy128);
	}

	{

		pipes_invert8 = gpu_create_pipeline();
		gc_root(pipes_invert8);
		pipes_invert8->vertex_shader   = sys_get_shader("layer_invert.vert");
		pipes_invert8->fragment_shader = sys_get_shader("layer_invert.frag");
		gpu_vertex_structure_t *vs     = GC_ALLOC_INIT(gpu_vertex_structure_t, {0});
		gpu_vertex_struct_add(vs, "pos", GPU_VERTEX_DATA_F32_2X);
		pipes_invert8->input_layout           = vs;
		pipes_invert8->color_attachment_count = 1;
		pipes_invert8->color_attachment[0]    = GPU_TEXTURE_FORMAT_R8;
		gpu_pipeline_compile(pipes_invert8);
	}

	{

		pipes_apply_mask = gpu_create_pipeline();
		gc_root(pipes_apply_mask);
		pipes_apply_mask->vertex_shader   = sys_get_shader("mask_apply.vert");
		pipes_apply_mask->fragment_shader = sys_get_shader("mask_apply.frag");
		gpu_vertex_structure_t *vs        = GC_ALLOC_INIT(gpu_vertex_structure_t, {0});
		gpu_vertex_struct_add(vs, "pos", GPU_VERTEX_DATA_F32_2X);
		pipes_apply_mask->input_layout = vs;
		gpu_pipeline_compile(pipes_apply_mask);
		pipes_tex0_mask = 0;
		pipes_texa_mask = 1;
	}

	{

		pipes_merge_mask = gpu_create_pipeline();
		gc_root(pipes_merge_mask);
		pipes_merge_mask->vertex_shader   = sys_get_shader("mask_merge.vert");
		pipes_merge_mask->fragment_shader = sys_get_shader("mask_merge.frag");
		gpu_vertex_structure_t *vs        = GC_ALLOC_INIT(gpu_vertex_structure_t, {0});
		gpu_vertex_struct_add(vs, "pos", GPU_VERTEX_DATA_F32_2X);
		pipes_merge_mask->input_layout = vs;
		gpu_pipeline_compile(pipes_merge_mask);
		pipes_tex0_merge_mask     = 0;
		pipes_texa_merge_mask     = 1;
		pipes_offset              = 0;
		pipes_opac_merge_mask     = pipes_get_constant_location("float");
		pipes_blending_merge_mask = pipes_get_constant_location("int");
	}

	{

		pipes_colorid_to_mask = gpu_create_pipeline();
		gc_root(pipes_colorid_to_mask);
		pipes_colorid_to_mask->vertex_shader   = sys_get_shader("mask_colorid.vert");
		pipes_colorid_to_mask->fragment_shader = sys_get_shader("mask_colorid.frag");
		gpu_vertex_structure_t *vs             = GC_ALLOC_INIT(gpu_vertex_structure_t, {0});
		gpu_vertex_struct_add(vs, "pos", GPU_VERTEX_DATA_F32_2X);
		pipes_colorid_to_mask->input_layout = vs;
		gpu_pipeline_compile(pipes_colorid_to_mask);
		pipes_texpaint_colorid = 0;
		pipes_tex_colorid      = 1;
	}

	{

		pipes_copy_rgb = gpu_create_pipeline();
		gc_root(pipes_copy_rgb);
		pipes_copy_rgb->vertex_shader   = sys_get_shader("layer_copy.vert");
		pipes_copy_rgb->fragment_shader = sys_get_shader("layer_copy.frag");
		gpu_vertex_structure_t *vs      = GC_ALLOC_INIT(gpu_vertex_structure_t, {0});
		gpu_vertex_struct_add(vs, "pos", GPU_VERTEX_DATA_F32_2X);
		pipes_copy_rgb->input_layout              = vs;
		pipes_copy_rgb->color_write_mask_alpha[0] = false;
		gpu_pipeline_compile(pipes_copy_rgb);
	}

	{

		pipes_cursor = gpu_create_pipeline();
		gc_root(pipes_cursor);
		pipes_cursor->vertex_shader   = sys_get_shader("cursor.vert");
		pipes_cursor->fragment_shader = sys_get_shader("cursor.frag");
		gpu_vertex_structure_t *vs    = GC_ALLOC_INIT(gpu_vertex_structure_t, {0});
		gpu_vertex_struct_add(vs, "pos", GPU_VERTEX_DATA_I16_4X_NORM);
		gpu_vertex_struct_add(vs, "nor", GPU_VERTEX_DATA_I16_2X_NORM);
		gpu_vertex_struct_add(vs, "tex", GPU_VERTEX_DATA_I16_2X_NORM);
		pipes_cursor->input_layout      = vs;
		pipes_cursor->blend_source      = GPU_BLEND_SOURCE_ALPHA;
		pipes_cursor->blend_destination = GPU_BLEND_INV_SOURCE_ALPHA;
		pipes_cursor->depth_write       = false;
		pipes_cursor->depth_mode        = GPU_COMPARE_MODE_ALWAYS;
		gpu_pipeline_compile(pipes_cursor);
		pipes_offset              = 0;
		pipes_cursor_vp           = pipes_get_constant_location("mat4");
		pipes_cursor_inv_vp       = pipes_get_constant_location("mat4");
		pipes_cursor_mouse        = pipes_get_constant_location("vec2");
		pipes_cursor_tex_step     = pipes_get_constant_location("vec2");
		pipes_cursor_radius       = pipes_get_constant_location("float");
		pipes_cursor_camera_right = pipes_get_constant_location("vec3");
		pipes_cursor_tint         = pipes_get_constant_location("vec3");
		pipes_cursor_gbufferd     = 0;
	}
}

i32 pipes_get_constant_location(char *type) {
	i32 size = shader_context_type_size(type);
	pipes_offset += shader_context_type_pad(pipes_offset, size);
	i32 loc = pipes_offset;
	pipes_offset += size;
	return loc;
}
