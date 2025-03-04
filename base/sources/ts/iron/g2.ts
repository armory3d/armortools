
declare type draw_font_t = {
	blob?: any; // unsigned char *
	images?: any; // draw_font_image_t *
	m_capacity?: i32;
	m_images_len?: i32;
	offset?: 32;

	buf?: buffer_t;
	glyphs?: i32[];
	index?: i32;
};

let _g2_current: image_t = null;
let _g2_in_use: bool = false;
let _g2_font_glyphs: i32[] = _g2_make_glyphs(32, 127);
let _g2_font_glyphs_last: i32[] = _g2_font_glyphs;
let _g2_thrown: bool = false;
let _g2_mat: f32_array_t = f32_array_create(9);

function g2_set_font(f: draw_font_t, size: i32) {
	g2_font_init(f);
	draw_set_font(f, size);
	draw_font_size = size;
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

function g2_font_init(raw: draw_font_t) {
	if (_g2_font_glyphs_last != _g2_font_glyphs) {
		_g2_font_glyphs_last = _g2_font_glyphs;
		draw_font_set_glyphs(_g2_font_glyphs);
	}
	if (raw.glyphs != _g2_font_glyphs) {
		raw.glyphs = _g2_font_glyphs;
		draw_font_init(raw, raw.buf.buffer, raw.index);
	}
}

function g2_font_height(raw: draw_font_t, size: i32): f32 {
	g2_font_init(raw);
	return draw_font_height(raw, size);
}

function g2_font_width(raw: draw_font_t, size: i32, str: string): f32 {
	g2_font_init(raw);
	return draw_string_width(raw, size, str);
}

function g2_font_unload(raw: draw_font_t) {
	raw.buf = null;
}

function g2_font_set_font_index(raw: draw_font_t, index: i32) {
	raw.index = index;
	_g2_font_glyphs = array_slice(_g2_font_glyphs, 0, _g2_font_glyphs.length); // Trigger atlas update
}

function g2_font_create(buf: buffer_t, index: i32 = 0): draw_font_t {
	let raw: draw_font_t = {};
	raw.buf = buf;
	raw.index = index;
	return raw;
}

function g2_font_clone(raw: draw_font_t): draw_font_t {
	return g2_font_create(raw.buf, raw.index);
}
