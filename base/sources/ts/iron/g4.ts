
function g4_vertex_struct_create(): kinc_g5_vertex_structure_t {
	let raw: kinc_g5_vertex_structure_t = {};
	return raw;
}

function g4_vertex_struct_add(raw: kinc_g5_vertex_structure_t, name: string, data: vertex_data_t) {
	let e: kinc_g5_vertex_element_t = ADDRESS(ARRAY_ACCESS(raw.elements, raw.size));
	e.name = name;
	e.data = data;
	raw.size++;
}

function g4_vertex_struct_byte_size(raw: kinc_g5_vertex_structure_t): i32 {
	let byte_size: i32 = 0;
	for (let i: i32 = 0; i < raw.size; ++i) {
		let elem: kinc_g5_vertex_element_t = ADDRESS(ARRAY_ACCESS(raw.elements, i));
		byte_size += g4_vertex_struct_data_byte_size(elem.data);
	}
	return byte_size;
}

function g4_vertex_struct_data_byte_size(data: vertex_data_t): i32 {
	if (data == vertex_data_t.F32_1X) {
		return 1 * 4;
	}
	if (data == vertex_data_t.F32_2X) {
		return 2 * 4;
	}
	if (data == vertex_data_t.F32_3X) {
		return 3 * 4;
	}
	if (data == vertex_data_t.F32_4X) {
		return 4 * 4;
	}
	if (data == vertex_data_t.U8_4X_NORM) {
		return 4 * 1;
	}
	if (data == vertex_data_t.I16_2X_NORM) {
		return 2 * 2;
	}
	if (data == vertex_data_t.I16_4X_NORM) {
		return 4 * 2;
	}
	return 0;
}

function g4_set_floats(loc: kinc_const_loc_t, values: f32_array_t) {
	let b: buffer_t = {
		buffer: values.buffer,
		length: values.length * 4
	};
	iron_g4_set_floats(loc, b);
}

function g4_set_vec3(loc: kinc_const_loc_t, v: vec4_t) {
	iron_g4_set_float3(loc, v.x, v.y, v.z);
}

function g4_draw(start: i32 = 0, count: i32 = -1) {
	iron_g4_draw_indexed_vertices(start, count);
}

function image_from_bytes(buffer: buffer_t, width: i32, height: i32, format: tex_format_t = tex_format_t.RGBA32): kinc_g5_texture_t {
	return iron_g4_create_texture_from_bytes(buffer, width, height, format, true);
}

function image_from_encoded_bytes(buffer: buffer_t, format: string, readable: bool = false): kinc_g5_texture_t {
	return iron_g4_create_texture_from_encoded_bytes(buffer, format, readable);
}

function image_create(width: i32, height: i32, format: tex_format_t = tex_format_t.RGBA32): kinc_g5_texture_t {
	return iron_g4_create_texture(width, height, format);
}

declare type kinc_g5_pipeline_t = {
	input_layout?: any;
	vertex_shader?: any;
	fragment_shader?: any;

	cull_mode?: cull_mode_t;
	depth_write?: bool;
	depth_mode?: compare_mode_t;

	blend_source?: blend_factor_t;
	blend_destination?: blend_factor_t;
	alpha_blend_source?: blend_factor_t;
	alpha_blend_destination?: blend_factor_t;

	color_write_mask_red?: any;
	color_write_mask_green?: any;
	color_write_mask_blue?: any;
	color_write_mask_alpha?: any;

	color_attachment?: any;
	color_attachment_count?: i32;
	depth_attachment_bits?: i32;

	impl?: any;
};

declare type kinc_g5_shader_t = {
	impl?: any;
};

declare type kinc_g5_vertex_element_t = {
	name?: string;
	data?: vertex_data_t;
};

declare type kinc_g5_vertex_structure_t = {
	elements?: any; // kinc_g5_vertex_element_t[KINC_G5_MAX_VERTEX_ELEMENTS];
	size?: i32;
};

declare type kinc_g5_index_buffer_t = {
	impl?: any;
};

declare type kinc_g5_vertex_buffer_t = {
	impl?: any;
};

type kinc_const_loc_t = any;
type kinc_tex_unit_t = any;

enum clear_flag_t {
	COLOR = 1,
	DEPTH = 2,
}

enum tex_filter_t {
	POINT,
	LINEAR,
	ANISOTROPIC,
}

enum mip_map_filter_t {
	NONE,
	POINT,
	LINEAR,
}

enum tex_addressing_t {
	REPEAT,
	MIRROR,
	CLAMP,
}

enum usage_t {
	STATIC,
	DYNAMIC,
	READABLE,
}

enum tex_format_t {
	RGBA32,
	RGBA64,
	RGBA128,
	R8,
	R16,
	R32,
}

enum vertex_data_t {
	F32_1X,
	F32_2X,
	F32_3X,
	F32_4X,
	U8_4X_NORM,
	I16_2X_NORM,
	I16_4X_NORM,
}

enum blend_factor_t {
	BLEND_ONE,
	BLEND_ZERO,
	SOURCE_ALPHA,
	DEST_ALPHA,
	INV_SOURCE_ALPHA,
	INV_DEST_ALPHA,
	SOURCE_COLOR,
	DEST_COLOR,
	INV_SOURCE_COLOR,
	INV_DEST_COLOR,
}

enum compare_mode_t {
	ALWAYS,
	NEVER,
	EQUAL,
	NOT_EQUAL,
	LESS,
	LESS_EQUAL,
	GREATER,
	GREATER_EQUAL,
}

enum cull_mode_t {
	CLOCKWISE,
	COUNTER_CLOCKWISE,
	NONE,
}

enum shader_type_t {
	FRAGMENT,
	VERTEX,
}
