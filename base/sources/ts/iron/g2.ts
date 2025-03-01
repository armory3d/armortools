
let _g2_color: color_t;
let _g2_font: g2_font_t;
let _g2_font_size: i32 = 0;
let _g2_pipeline: pipeline_t;
let _g2_transformation: mat3_t = mat3_nan();
let _g2_render_target: image_t;

let _g2_current: image_t = null;
let _g2_in_use: bool = false;
let _g2_font_glyphs: i32[] = _g2_make_glyphs(32, 127);
let _g2_font_glyphs_last: i32[] = _g2_font_glyphs;
let _g2_thrown: bool = false;
let _g2_mat: f32_array_t = f32_array_create(9);
let _g2_initialized: bool = false;

function g2_set_color(c: color_t) {
	draw_set_color(c);
	_g2_color = c;
}

function g2_set_font_and_size(font: g2_font_t, font_size: i32) {
	g2_font_init(font);
	draw_set_font(font.font_, font_size);
}

function g2_set_font(f: g2_font_t) {
	if (_g2_font_size != 0) {
		g2_set_font_and_size(f, _g2_font_size);
	}
	_g2_font = f;
}

function g2_set_font_size(i: i32) {
	if (_g2_font.font_ != null) {
		g2_set_font_and_size(_g2_font, i);
	}
	_g2_font_size = i;
}

function g2_set_pipeline(p: pipeline_t) {
	draw_set_pipeline(p == null ? null : p.pipeline_);
	_g2_pipeline = p;
}

function g2_set_bilinear_filter(bilinear: bool) {
	draw_set_bilinear_filter(bilinear);
}

function g2_set_transformation(m: mat3_t) {
	if (mat3_isnan(m)) {
		draw_set_transform(null);
	}
	else {
		_g2_mat[0] = m.m00;
		_g2_mat[1] = m.m01;
		_g2_mat[2] = m.m02;
		_g2_mat[3] = m.m10;
		_g2_mat[4] = m.m11;
		_g2_mat[5] = m.m12;
		_g2_mat[6] = m.m20;
		_g2_mat[7] = m.m21;
		_g2_mat[8] = m.m22;
		draw_set_transform(_g2_mat);
	}
}

function _g2_make_glyphs(start: i32, end: i32): i32[] {
	let ar: i32[] = [];
	for (let i: i32 = start; i < end; ++i) {
		array_push(ar, i);
	}
	return ar;
}

function g2_init() {
	if (!_g2_initialized) {
		draw_init(
			iron_load_blob(data_path() + "draw_image.vert" + sys_shader_ext()),
			iron_load_blob(data_path() + "draw_image.frag" + sys_shader_ext()),
			iron_load_blob(data_path() + "draw_colored.vert" + sys_shader_ext()),
			iron_load_blob(data_path() + "draw_colored.frag" + sys_shader_ext()),
			iron_load_blob(data_path() + "draw_text.vert" + sys_shader_ext()),
			iron_load_blob(data_path() + "draw_text.frag" + sys_shader_ext())
		);
		_g2_initialized = true;
	}
}

function g2_draw_scaled_sub_image(img: image_t, sx: f32, sy: f32, sw: f32, sh: f32, dx: f32, dy: f32, dw: f32, dh: f32) {
	draw_scaled_sub_image(img, sx, sy, sw, sh, dx, dy, dw, dh);
}

function g2_draw_sub_image(img: image_t, x: f32, y: f32, sx: f32, sy: f32, sw: f32, sh: f32) {
	g2_draw_scaled_sub_image(img, sx, sy, sw, sh, x, y, sw, sh);
}

function g2_draw_scaled_image(img: image_t, dx: f32, dy: f32, dw: f32, dh: f32) {
	g2_draw_scaled_sub_image(img, 0, 0, img.width, img.height, dx, dy, dw, dh);
}

function g2_draw_image(img: image_t, x: f32, y: f32) {
	g2_draw_scaled_sub_image(img, 0, 0, img.width, img.height, x, y, img.width, img.height);
}

function g2_draw_rect(x: f32, y: f32, width: f32, height: f32, strength: f32 = 1.0) {
	draw_rect(x, y, width, height, strength);
}

function g2_fill_rect(x: f32, y: f32, width: f32, height: f32) {
	draw_filled_rect(x, y, width, height);
}

function g2_draw_string(text: string, x: f32, y: f32) {
	draw_string(text, x, y);
}

function g2_draw_line(x0: f32, y0: f32, x1: f32, y1: f32, strength: f32 = 1.0) {
	draw_line(x0, y0, x1, y1, strength);
}

function g2_draw_line_aa(x0: f32, y0: f32, x1: f32, y1: f32, strength: f32 = 1.0) {
	draw_line_aa(x0, y0, x1, y1, strength);
}

function g2_fill_triangle(x0: f32, y0: f32, x1: f32, y1: f32, x2: f32, y2: f32) {
	draw_filled_triangle(x0, y0, x1, y1, x2, y2);
}

function g2_scissor(x: i32, y: i32, width: i32, height: i32) {
	draw_end(); // flush
	g4_scissor(x, y, width, height);
}

function g2_disable_scissor() {
	draw_end(); // flush
	g4_disable_scissor();
}

function g2_begin(render_target: image_t = null) {
	if (_g2_in_use && !_g2_thrown) {
		_g2_thrown = true;
		kinc_log("End before you begin");
	}
	_g2_in_use = true;
	_g2_current = render_target;

	draw_begin();

	if (render_target != null) {
		draw_set_render_target(render_target.render_target_);
	}
	else {
		draw_restore_render_target();
	}
}

function g2_end() {
	if (!_g2_in_use && !_g2_thrown) {
		_g2_thrown = true;
		kinc_log("Begin before you end");
	}
	_g2_in_use = false;
	_g2_current = null;

	draw_end();
}

function g2_clear(color: color_t = 0x00000000) {
	g4_clear(color);
}

function g2_fill_circle(cx: f32, cy: f32, radius: f32, segments: i32 = 0) {
	draw_filled_circle(cx, cy, radius, segments);
}

function g2_draw_circle(cx: f32, cy: f32, radius: f32, segments: i32 = 0, strength: f32 = 1.0) {
	draw_circle(cx, cy, radius, segments, strength);
}

function g2_draw_cubic_bezier(x: f32[], y: f32[], segments: i32 = 20, strength: f32 = 1.0) {
	draw_cubic_bezier(x, y, segments, strength);
}

function g2_font_init(raw: g2_font_t) {
	if (_g2_font_glyphs_last != _g2_font_glyphs) {
		_g2_font_glyphs_last = _g2_font_glyphs;
		draw_font_set_glyphs(_g2_font_glyphs);
	}
	if (raw.glyphs != _g2_font_glyphs) {
		raw.glyphs = _g2_font_glyphs;
		raw.font_ = draw_font_init(raw.blob, raw.index);
	}
}

function g2_font_create(blob: buffer_t, index: i32 = 0): g2_font_t {
	let raw: g2_font_t = {};
	raw.blob = blob;
	raw.index = index;
	return raw;
}

function g2_font_height(raw: g2_font_t, size: i32): f32 {
	g2_font_init(raw);
	return draw_font_height(raw.font_, size);
}

function g2_font_width(raw: g2_font_t, size: i32, str: string): f32 {
	g2_font_init(raw);
	return draw_string_width(raw.font_, size, str);
}

function g2_font_unload(raw: g2_font_t) {
	raw.blob = null;
}

function g2_font_set_font_index(raw: g2_font_t, index: i32) {
	raw.index = index;
	_g2_font_glyphs = array_slice(_g2_font_glyphs, 0, _g2_font_glyphs.length); // Trigger atlas update
}

function g2_font_clone(raw: g2_font_t): g2_font_t {
	return g2_font_create(raw.blob, raw.index);
}

declare type g2_font_t = {
	font_?: any; // draw_font_t
	blob?: buffer_t;
	glyphs?: i32[];
	index?: i32;
};
