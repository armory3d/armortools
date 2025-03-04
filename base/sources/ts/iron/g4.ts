
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

function g4_begin(render_target: image_t, additional_targets: image_t[] = null) {
	iron_g4_begin(render_target, additional_targets);
}
function g4_clear(color: color_t = 0x00000000, depth: f32 = 0.0, flags: i32 = clear_flag_t.COLOR) {
	kinc_g5_clear(flags, color, depth);
}

function g4_set_tex(unit: kinc_tex_unit_t, tex: image_t) {
	// if (tex == null) {
	// 	return;
	// }
	tex.texture_ != null ? iron_g4_set_texture(unit, tex.texture_) : iron_g4_set_render_target(unit, tex.render_target_);
}

function g4_set_tex_depth(unit: kinc_tex_unit_t, tex: image_t) {
	// if (tex == null) {
	// 	return;
	// }
	iron_g4_set_texture_depth(unit, tex.render_target_);
}

function g4_set_tex_params(tex_unit: kinc_tex_unit_t, u_addressing: tex_addressing_t, v_addressing: tex_addressing_t, minification_filter: tex_filter_t, magnification_filter: tex_filter_t, mipmap_filter: mip_map_filter_t) {
	iron_g4_set_texture_parameters(tex_unit, u_addressing, v_addressing, minification_filter, magnification_filter, mipmap_filter);
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

function _image_create(tex: any): image_t {
	let raw: image_t = {};
	raw.texture_ = tex;
	return raw;
}

function _image_set_size_from_texture(image: image_t, _tex: any) {
	let tex: kinc_g5_texture_t = _tex;
	image.width = tex.width;
	image.height = tex.height;
}

function _image_set_size_from_render_target(image: image_t, _rt: any) {
	let rt: kinc_g5_texture_t = _rt;
	image.width = rt.width;
	image.height = rt.height;
}

function image_from_texture(tex: any): image_t {
	let image: image_t = _image_create(tex);
	_image_set_size_from_texture(image, tex);
	return image;
}

function image_from_bytes(buffer: buffer_t, width: i32, height: i32, format: tex_format_t = tex_format_t.RGBA32): image_t {
	let readable: bool = true;
	let image: image_t = _image_create(null);
	image.format = format;
	image.texture_ = iron_g4_create_texture_from_bytes(buffer, width, height, format, readable);
	_image_set_size_from_texture(image, image.texture_);
	return image;
}

function image_from_encoded_bytes(buffer: buffer_t, format: string, readable: bool = false): image_t {
	let image: image_t = _image_create(null);
	image.texture_ = iron_g4_create_texture_from_encoded_bytes(buffer, format, readable);
	_image_set_size_from_texture(image, image.texture_);
	return image;
}

function image_create(width: i32, height: i32, format: tex_format_t = tex_format_t.RGBA32): image_t {
	let image: image_t = _image_create(null);
	image.format = format;
	image.texture_ = iron_g4_create_texture(width, height, format);
	_image_set_size_from_texture(image, image.texture_);
	return image;
}

function image_create_render_target(width: i32, height: i32, format: tex_format_t = tex_format_t.RGBA32, depth_buffer_bits: i32 = 0): image_t {
	let image: image_t = _image_create(null);
	image.format = format;
	image.render_target_ = iron_g4_create_render_target(width, height, format, depth_buffer_bits);
	_image_set_size_from_render_target(image, image.render_target_);
	return image;
}

function image_format_byte_size(format: tex_format_t): i32 {
	if (format == tex_format_t.RGBA32) {
		return 4;
	}
	if (format == tex_format_t.R8) {
		return 1;
	}
	if (format == tex_format_t.RGBA128) {
		return 16;
	}
	if (format == tex_format_t.RGBA64) {
		return 8;
	}
	if (format == tex_format_t.R32) {
		return 4;
	}
	if (format == tex_format_t.R16) {
		return 2;
	}
	return 4;
}

function image_unload(raw: image_t) {
	iron_unload_image(raw);
	raw.texture_ = null;
	raw.render_target_ = null;
}

function image_lock(raw: image_t, level: i32 = 0): buffer_t {
	return iron_g4_lock_texture(raw.texture_, level);
}

function image_unlock(raw: image_t) {
	kinc_g5_texture_unlock(raw.texture_);
}

function image_get_pixels(raw: image_t): buffer_t {
	if (raw.render_target_ != null) {
		// Minimum size of 32x32 required after https://github.com/Kode/Kinc/commit/3797ebce5f6d7d360db3331eba28a17d1be87833
		let pixels_w: i32 = raw.width < 32 ? 32 : raw.width;
		let pixels_h: i32 = raw.height < 32 ? 32 : raw.height;
		if (raw.pixels == null) {
			raw.pixels = buffer_create(image_format_byte_size(raw.format) * pixels_w * pixels_h);
		}
		iron_g4_get_render_target_pixels(raw.render_target_, raw.pixels);
		return raw.pixels;
	}
	else {
		return iron_g4_get_texture_pixels(raw.texture_);
	}
}

function image_gen_mipmaps(raw: image_t, levels: i32) {
	raw.texture_ == null ? kinc_g5_render_target_generate_mipmaps(raw.render_target_, levels) : kinc_g5_texture_generate_mipmaps(raw.texture_, levels);
}

function image_set_mipmaps(raw: image_t, mipmaps: image_t[]) {
	iron_g4_set_mipmaps(raw.texture_, mipmaps);
}

function image_set_depth_from(raw: image_t, image: image_t) {
	kinc_g5_render_target_set_depth_from(raw.render_target_, image.render_target_);
}

declare type image_t = {
	texture_?: any;
	render_target_?: any;
	format?: tex_format_t;
	readable?: bool;
	pixels?: buffer_t;
	width?: i32;
	height?: i32;
};

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
