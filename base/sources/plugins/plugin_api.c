
#include "plugin_api.h"
#include "iron_armpack.h"
#include "iron_array.h"
#include "iron_map.h"
#include "iron_mat4.h"
#include "iron_obj.h"
#include "iron_ui.h"
#include "iron_ui_nodes.h"
#include "iron_vec4.h"

void plugin_embed();

// ██╗     ██╗██████╗
// ██║     ██║██╔══██╗
// ██║     ██║██████╔╝
// ██║     ██║██╔══██╗
// ███████╗██║██████╔╝
// ╚══════╝╚═╝╚═════╝

const char *lib = "\
function buffer_to_string(b) {\
	let str = \"\";\
	let u8a = new Uint8Array(b);\
	for (let i = 0; i < u8a.length; ++i) {\
		str += String.fromCharCode(u8a[i]);\
	}\
	return str;\
}\
\
function string_to_buffer(str) {\
	let u8a = new Uint8Array(str.length);\
	for (let i = 0; i < str.length; ++i) {\
		u8a[i] = str.charCodeAt(i);\
	}\
	return u8a.buffer;\
}\
\
let pos = 0;\
let k_type = \"\";\
\
function armpack_decode(b) {\
	pos = 0;\
	return read(new DataView(b));\
}\
\
function read_u8(v) {\
	let i = v.getUint8(pos);\
	pos++;\
	return i;\
}\
\
function read_i16(v) {\
	let i = v.getInt16(pos, true);\
	pos += 2;\
	return i;\
}\
\
function read_i32(v) {\
	let i = v.getInt32(pos, true);\
	pos += 4;\
	return i;\
}\
\
function read_u32(v) {\
	let i = v.getUint32(pos, true);\
	pos += 4;\
	return i;\
}\
\
function read_f32(v) {\
	let f = v.getFloat32(pos, true);\
	pos += 4;\
	return f;\
}\
\
function read_string(v, len) {\
	let s = \"\";\
	for (let i = 0; i < len; ++i) {\
		s += String.fromCharCode(read_u8(v));\
	}\
	return s;\
}\
\
function read(v) {\
	let b = read_u8(v);\
	switch (b) {\
		case 0xc0: return null;\
		case 0xc2: return false;\
		case 0xc3: return true;\
		case 0xca: { k_type = \"__f32\"; return read_f32(v); }\
		case 0xd2: return read_i32(v);\
		case 0xdb: return read_string(v, read_u32(v));\
		case 0xdd: return read_array(v, read_i32(v));\
		case 0xdf: return read_map(v, read_i32(v));\
	}\
	return 0;\
}\
\
function read_array(v, length) {\
	let b = read_u8(v);\
\
	if (b == 0xca) { /* Typed f32 */ \
		let a = new Array(length);\
		for (let x = 0; x < length; ++x) a[x] = read_f32(v);\
		k_type = \"__f32\";\
		return a;\
	}\
	else if (b == 0xd2) { /* Typed i32 */ \
		let a = new Array(length);\
		for (let x = 0; x < length; ++x) a[x] = read_i32(v);\
		k_type = \"__i32\";\
		return a;\
	}\
	else if (b == 0xd1) { /* Typed i16 */ \
		let a = new Array(length);\
		for (let x = 0; x < length; ++x) a[x] = read_i16(v);\
		k_type = \"__i16\";\
		return a;\
	}\
	else if (b == 0xc4) { /* Typed u8 */ \
		let a = new Array(length);\
		for (let x = 0; x < length; ++x) a[x] = read_u8(v);\
		k_type = \"__u8\";\
		return a;\
	}\
	else { /* Dynamic type-value */ \
		pos--;\
		let a = new Array(length);\
		for (let x = 0; x < length; ++x) a[x] = read(v);\
		return a;\
	}\
}\
\
function read_map(v, length) {\
	let out = {};\
	for (let n = 0; n < length; ++n) {\
		let k = read(v);\
		k_type = \"\";\
		let val = read(v);\
		k += k_type;\
		k_type = \"\";\
		out[k] = val;\
	}\
	return out;\
}\
\
function armpack_encode(d) {\
	pos = 0;\
	write_dummy(d);\
	let b = new ArrayBuffer(pos);\
	let v = new DataView(b);\
	pos = 0;\
	write(v, d);\
	return b;\
}\
\
function write_u8(v, i) {\
	v.setUint8(pos, i);\
	pos += 1;\
}\
\
function write_i16(v, i) {\
	v.setInt16(pos, i, true);\
	pos += 2;\
}\
\
function write_i32(v, i) {\
	v.setInt32(pos, i, true);\
	pos += 4;\
}\
\
function write_f32(v, f) {\
	v.setFloat32(pos, f, true);\
	pos += 4;\
}\
\
function write_string(v, str) {\
	for (let i = 0; i < str.length; ++i) {\
		write_u8(v, str.charCodeAt(i));\
	}\
}\
\
function write(v, d) {\
	if (d == null) {\
		write_u8(v, 0xc0);\
	}\
	else if (typeof d == \"boolean\") {\
		write_u8(v, d ? 0xc3 : 0xc2);\
	}\
	else if (typeof d == \"number\") {\
		if (Number.isInteger(d) && k_type != \"__f32\") {\
			write_u8(v, 0xd2);\
			write_i32(v, d);\
		}\
		else {\
			write_u8(v, 0xca);\
			write_f32(v, d);\
		}\
	}\
	else if (typeof d == \"string\") {\
		write_u8(v, 0xdb);\
		write_i32(v, d.length);\
		write_string(v, d);\
	}\
	else if (Array.isArray(d)) {\
		write_u8(v, 0xdd);\
		write_i32(v, d.length);\
		if (k_type == \"__u8\") {\
			write_u8(v, 0xc4);\
			for (let i = 0; i < d.length; ++i) {\
				write_u8(v, d[i]);\
			}\
		}\
		else if (k_type == \"__i16\") {\
			write_u8(v, 0xd1);\
			for (let i = 0; i < d.length; ++i) {\
				write_i16(v, d[i]);\
			}\
		}\
		else if (k_type == \"__f32\") {\
			write_u8(v, 0xca);\
			for (let i = 0; i < d.length; ++i) {\
				write_f32(v, d[i]);\
			}\
		}\
		else if (k_type == \"__i32\") {\
			write_u8(v, 0xd2);\
			for (let i = 0; i < d.length; ++i) {\
				write_i32(v, d[i]);\
			}\
		}\
		else {\
			for (let i = 0; i < d.length; ++i) {\
				write(v, d[i]);\
			}\
		}\
	}\
	else {\
		write_object(v, d);\
	}\
}\
\
function write_object(v, d) {\
	let f = Object.keys(d);\
	write_u8(v, 0xdf);\
	write_i32(v, f.length);\
	for (let i = 0; i < f.length; ++i) {\
		let k = f[i];\
\
		k_type = \"\";\
		if (k.endsWith(\"__f32\")) {\
			k_type = \"__f32\";\
		}\
		else if (k.endsWith(\"__i32\")) {\
			k_type = \"__i32\";\
		}\
		else if (k.endsWith(\"__i16\")) {\
			k_type = \"__i16\";\
		}\
		else if (k.endsWith(\"__u8\")) {\
			k_type = \"__u8\";\
		}\
\
		write_u8(v, 0xdb);\
		write_i32(v, k.length - k_type.length);\
\
		write_string(v, k.substring(0, k.length - k_type.length));\
		write(v, d[k]);\
		k_type = \"\";\
	}\
}\
\
function write_dummy(d) {\
	if (d == null) {\
		pos += 1;\
	}\
	else if (typeof d == \"boolean\") {\
		pos += 1;\
	}\
	else if (typeof d == \"number\") {\
		pos += 1;\
		pos += 4;\
	}\
	else if (typeof d == \"string\") {\
		pos += 1;\
		pos += 4;\
		pos += d.length;\
	}\
	else if (Array.isArray(d)) {\
		pos += 1;\
		pos += 4;\
		if (k_type == \"__u8\") {\
			pos += 1;\
			for (let i = 0; i < d.length; ++i) {\
				pos += 1;\
			}\
		}\
		else if (k_type == \"__i16\") {\
			pos += 1;\
			for (let i = 0; i < d.length; ++i) {\
				pos += 2;\
			}\
		}\
		else if (k_type == \"__f32\") {\
			pos += 1;\
			for (let i = 0; i < d.length; ++i) {\
				pos += 4;\
			}\
		}\
		else if (k_type == \"__i32\") {\
			pos += 1;\
			for (let i = 0; i < d.length; ++i) {\
				pos += 4;\
			}\
		}\
		else {\
			for (let i = 0; i < d.length; ++i) {\
				write_dummy(d[i]);\
			}\
		}\
	}\
	else {\
		write_object_dummy(d);\
	}\
}\
\
function write_object_dummy(d) {\
	let f = Object.keys(d);\
	pos += 1;\
	pos += 4;\
	for (let i = 0; i < f.length; ++i) {\
		let k = f[i];\
		pos += 1;\
		pos += 4;\
\
		k_type = \"\";\
		if (k.endsWith(\"__f32\")) {\
			k_type = \"__f32\";\
		}\
		else if (k.endsWith(\"__i32\")) {\
			k_type = \"__i32\";\
		}\
		else if (k.endsWith(\"__i16\")) {\
			k_type = \"__i16\";\
		}\
		else if (k.endsWith(\"__u8\")) {\
			k_type = \"__u8\";\
		}\
\
		pos += k.length - k_type.length;\
		write_dummy(d[k]);\
		k_type = \"\";\
	}\
}\
\
globalThis.armpack_encode = armpack_encode;\
globalThis.armpack_decode = armpack_decode;\
globalThis.string_to_buffer = string_to_buffer;\
globalThis.buffer_to_string = buffer_to_string;\
";

// ██████╗ ██╗███╗   ██╗██████╗ ██╗███╗   ██╗ ██████╗ ███████╗
// ██╔══██╗██║████╗  ██║██╔══██╗██║████╗  ██║██╔════╝ ██╔════╝
// ██████╔╝██║██╔██╗ ██║██║  ██║██║██╔██╗ ██║██║  ███╗███████╗
// ██╔══██╗██║██║╚██╗██║██║  ██║██║██║╚██╗██║██║   ██║╚════██║
// ██████╔╝██║██║ ╚████║██████╔╝██║██║ ╚████║╚██████╔╝███████║
// ╚═════╝ ╚═╝╚═╝  ╚═══╝╚═════╝ ╚═╝╚═╝  ╚═══╝ ╚═════╝ ╚══════╝

any_array_t *plugin_gc;

void gc_root(void *ptr);
void gc_run();

// These could be auto-generated by alang
VOID_FN(gc_run)

void *data_get_blob(char *s);
FN(data_get_blob) {
	char     *s   = (char *)JS_ToCString(ctx, argv[0]);
	buffer_t *b   = data_get_blob(s);
	JSValue   val = JS_NewArrayBuffer(ctx, b->buffer, b->length, NULL, NULL, 0);
	return val;
}

void *data_delete_blob(char *s);
FN(data_delete_blob) {
	char *s = (char *)JS_ToCString(ctx, argv[0]);
	data_delete_blob(s);
	return JS_UNDEFINED;
}

void *iron_file_save_bytes(char *s, buffer_t *b, int l);
FN(iron_file_save_bytes) {
	char    *to = (char *)JS_ToCString(ctx, argv[0]);
	size_t   len;
	void    *ab = JS_GetArrayBuffer(ctx, &len, argv[1]);
	buffer_t b  = {.buffer = ab, .length = len, .capacity = len};
	iron_file_save_bytes(to, &b, len);
	return JS_UNDEFINED;
}

void *gpu_create_texture_from_bytes(void *p, int w, int h, int format);
FN(gpu_create_texture_from_bytes) {
	size_t   len;
	void    *ab = JS_GetArrayBuffer(ctx, &len, argv[0]);
	buffer_t b  = {.buffer = ab, .length = len, .capacity = len};
	int64_t  w;
	JS_ToInt64(ctx, &w, argv[1]);
	int64_t h;
	JS_ToInt64(ctx, &h, argv[2]);
	int64_t format = 0;
	if (argc > 3) {
		JS_ToInt64(ctx, &format, argv[3]);
	}
	uint64_t result = (uint64_t)gpu_create_texture_from_bytes(&b, w, h, format);
	return JS_NewBigUint64(ctx, result);
}

FN(ui_handle_create) {
	uint64_t result = (uint64_t)ui_handle_create();
	any_array_push(plugin_gc, (void *)result);
	return JS_NewBigUint64(ctx, result);
}

FN(ui_handle_set_value) {
	uint64_t p;
	JS_ToBigUint64(ctx, &p, argv[0]);
	ui_handle_t *h = (ui_handle_t *)p;
	double       d;
	JS_ToFloat64(ctx, &d, argv[1]);
	h->f = d;
	return JS_UNDEFINED;
}

FN(ui_handle_get_value) {
	uint64_t p;
	JS_ToBigUint64(ctx, &p, argv[0]);
	ui_handle_t *h = (ui_handle_t *)p;
	return JS_NewFloat64(ctx, h->f);
}

FN(ui_panel) {
	uint64_t p;
	JS_ToBigUint64(ctx, &p, argv[0]);
	char *s      = (char *)JS_ToCString(ctx, argv[1]);
	bool  result = ui_panel((void *)p, s, false, false);
	return JS_NewBool(ctx, result);
}

FN(ui_button) {
	char *s      = (char *)JS_ToCString(ctx, argv[0]);
	bool  result = ui_button(s, UI_ALIGN_CENTER, "");
	return JS_NewBool(ctx, result);
}

FN(ui_text) {
	char *s = (char *)JS_ToCString(ctx, argv[0]);
	ui_text(s, UI_ALIGN_LEFT, 0);
	return JS_UNDEFINED;
}

FN(ui_text_input) {
	uint64_t p;
	JS_ToBigUint64(ctx, &p, argv[0]);
	char *s = (char *)JS_ToCString(ctx, argv[1]);
	ui_text_input((void *)p, s, UI_ALIGN_LEFT, true, false);
	return JS_UNDEFINED;
}

FN(ui_slider) {
	uint64_t p;
	JS_ToBigUint64(ctx, &p, argv[0]);
	char  *s      = (char *)JS_ToCString(ctx, argv[1]);
	double from   = 0.0;
	double to     = 1.0;
	bool   filled = true;
	double prec   = 100.0;
	if (argc > 2) {
		JS_ToFloat64(ctx, &from, argv[2]);
	}
	if (argc > 3) {
		JS_ToFloat64(ctx, &to, argv[3]);
	}
	if (argc > 4) {
		filled = JS_ToBool(ctx, argv[4]);
	}
	if (argc > 5) {
		JS_ToFloat64(ctx, &prec, argv[5]);
	}
	ui_slider((void *)p, s, from, to, filled, prec, true, UI_ALIGN_LEFT, true);
	return JS_UNDEFINED;
}

FN(ui_check) {
	uint64_t p;
	JS_ToBigUint64(ctx, &p, argv[0]);
	char *s = (char *)JS_ToCString(ctx, argv[1]);
	ui_check((void *)p, s, "");
	return JS_UNDEFINED;
}

FN(ui_radio) {
	uint64_t p;
	JS_ToBigUint64(ctx, &p, argv[0]);
	int32_t pos;
	JS_ToInt32(ctx, &pos, argv[1]);
	char *s = (char *)JS_ToCString(ctx, argv[2]);
	ui_radio((void *)p, pos, s, "");
	return JS_UNDEFINED;
}

FN(ui_row) {
	JSValue val_len = JS_GetPropertyStr(ctx, argv[0], "length");
	int     len;
	JS_ToInt32(ctx, &len, val_len);
	f32_array_t *ratios = f32_array_create(len);
	for (int i = 0; i < len; ++i) {
		JSValue val = JS_GetPropertyUint32(ctx, argv[0], i);
		double  f;
		JS_ToFloat64(ctx, &f, val);
		ratios->buffer[i] = f;
	}
	ui_row(ratios);
	return JS_UNDEFINED;
}

FN(ui_combo) {
	uint64_t p;
	JS_ToBigUint64(ctx, &p, argv[0]);

	JSValue val_len = JS_GetPropertyStr(ctx, argv[1], "length");
	int     len;
	JS_ToInt32(ctx, &len, val_len);
	char_ptr_array_t *texts = any_array_create(len);
	for (int i = 0; i < len; ++i) {
		JSValue val      = JS_GetPropertyUint32(ctx, argv[1], i);
		char   *s        = (char *)JS_ToCString(ctx, val);
		texts->buffer[i] = s;
	}

	char *label = (char *)JS_ToCString(ctx, argv[2]);

	ui_combo((void *)p, texts, label, true, UI_ALIGN_LEFT, true);
	return JS_UNDEFINED;
}

FN(plugin_api_make_raw_mesh) {
	raw_mesh_t *mesh = calloc(sizeof(raw_mesh_t), 1);
	mesh->name       = (char *)JS_ToCString(ctx, argv[0]);

	size_t len;
	void  *ab          = JS_GetArrayBuffer(ctx, &len, argv[1]);
	mesh->posa         = malloc(sizeof(i16_array_t));
	mesh->posa->buffer = malloc(len);
	memcpy(mesh->posa->buffer, ab, len);
	mesh->posa->length = len / 2;

	ab                 = JS_GetArrayBuffer(ctx, &len, argv[2]);
	mesh->nora         = malloc(sizeof(i16_array_t));
	mesh->nora->buffer = malloc(len);
	memcpy(mesh->nora->buffer, ab, len);
	mesh->nora->length = len / 2;

	ab                 = JS_GetArrayBuffer(ctx, &len, argv[3]);
	mesh->inda         = malloc(sizeof(u32_array_t));
	mesh->inda->buffer = malloc(len);
	memcpy(mesh->inda->buffer, ab, len);
	mesh->inda->length = len / 4;

	double d;
	JS_ToFloat64(ctx, &d, argv[4]);
	mesh->scale_pos = d;
	mesh->scale_tex = 1.0;
	return JS_NewBigUint64(ctx, (uint64_t)mesh);
}

void transform_rotate(void *raw, vec4_t axis, float f);
FN(transform_rotate) {
	uint64_t p;
	JS_ToBigUint64(ctx, &p, argv[0]);
	vec4_t axis;
	axis.x = 0.0;
	axis.y = 0.0;
	axis.z = 1.0;
	double d;
	JS_ToFloat64(ctx, &d, argv[1]);
	transform_rotate((void *)p, axis, d);
	return JS_UNDEFINED;
}

void plugin_api_init() {
	JSValue global_obj = JS_GetGlobalObject(js_ctx);

	js_eval(lib);
	plugin_gc = any_array_create(0);
	gc_root(plugin_gc);

	BIND(gc_run, 0);
	BIND(data_get_blob, 1);
	BIND(data_delete_blob, 1);
	BIND(iron_file_save_bytes, 3);
	BIND(gpu_create_texture_from_bytes, 4);

	BIND(ui_handle_create, 0);
	BIND(ui_handle_set_value, 2);
	BIND(ui_handle_get_value, 1);
	BIND(ui_panel, 2);
	BIND(ui_button, 1);
	BIND(ui_text, 1);
	BIND(ui_text_input, 2);
	BIND(ui_slider, 5);
	BIND(ui_check, 2);
	BIND(ui_radio, 3);
	BIND(ui_row, 1);
	BIND(ui_combo, 3);

	BIND(plugin_api_make_raw_mesh, 5);

	BIND(transform_rotate, 3);

	plugin_embed();

	JS_FreeValue(js_ctx, global_obj);
}
