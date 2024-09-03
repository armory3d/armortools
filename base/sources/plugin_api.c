
#include "plugin_api.h"
#include "iron_ui.h"
#include "iron_ui_nodes.h"
#include "iron_array.h"
#include "iron_map.h"
#include "iron_armpack.h"
#include "iron_obj.h"

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

// These could be auto-generated by alang
VOID_FN_STR(console_log)
VOID_FN_STR(console_info)
PTR_FN(plugin_create)
VOID_FN_PTR_CB(plugin_notify_on_ui)
VOID_FN_PTR_CB(plugin_notify_on_update)
VOID_FN_PTR_CB(plugin_notify_on_delete)

static JSObject *ui_files_cb;
static void ui_files_done(char *path) {
	JSValue path_val = JS_NewString(js_ctx, path);
	JSValue argv[] = { path_val };
	js_call_arg(ui_files_cb, 1, argv);
}

void ui_files_show(char *s, bool b0, bool b1, void(*f)(char *));
FN(ui_files_show) {
	char *filters = (char *)JS_ToCString(ctx, argv[0]);
	bool is_save = JS_ToBool(ctx, argv[1]);
	bool open_multiple = JS_ToBool(ctx, argv[2]);
	ui_files_cb = malloc(sizeof(JSValue));
	JSValue dup = JS_DupValue(ctx, argv[3]);
	memcpy(ui_files_cb, &dup, sizeof(JSValue));
	ui_files_show(filters, is_save, open_multiple, ui_files_done);
	return JS_UNDEFINED;
}

void ui_box_show_message(char *s0, char *s1, bool b);
extern bool ui_box_click_to_hide;
FN(ui_box_show_message) {
	char *title = (char *)JS_ToCString(ctx, argv[0]);
	char *message = (char *)JS_ToCString(ctx, argv[1]);
	ui_box_show_message(title, message, true);
	ui_box_click_to_hide = false;
	return JS_UNDEFINED;
}

void *data_get_blob(char *s);
FN(data_get_blob) {
	char *s = (char *)JS_ToCString(ctx, argv[0]);
	buffer_t *b = data_get_blob(s);
	JSValue val = JS_NewArrayBuffer(ctx, b->buffer, b->length, NULL, NULL, 0);
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
	char *to = (char *)JS_ToCString(ctx, argv[0]);
	size_t len;
	void *ab = JS_GetArrayBuffer(ctx, &len, argv[1]);
	buffer_t b = { .buffer = ab, .length = len, .capacity = len };
	iron_file_save_bytes(to, &b, len);
	return JS_UNDEFINED;
}

VOID_FN_CB(context_set_viewport_shader)

void node_shader_add_uniform(void *p, char *s0, char *s1, bool b);
FN(node_shader_add_uniform) {
	int64_t p;
	JS_ToInt64(ctx, &p, argv[0]);
	char *s0 = (char *)JS_ToCString(ctx, argv[1]);
	char *s1 = (char *)JS_ToCString(ctx, argv[2]);
	node_shader_add_uniform((void *)p, s0, s1, false);
	return JS_UNDEFINED;
}

VOID_FN_PTR_STR(node_shader_write)

void *image_from_bytes(void *p, int w, int h, int format);
FN(image_from_bytes) {
	size_t len;
	void *ab = JS_GetArrayBuffer(ctx, &len, argv[0]);
	buffer_t b = { .buffer = ab, .length = len, .capacity = len };
	int64_t w;
	JS_ToInt64(ctx, &w, argv[1]);
	int64_t h;
	JS_ToInt64(ctx, &h, argv[2]);
	int64_t result = (int64_t)image_from_bytes(&b, w, h, 0);
	return JS_NewInt64(ctx, result);
}

extern char *project_filepath;
FN(project_filepath_get) {
	return JS_NewString(ctx, project_filepath);
}

void project_save(bool b);
FN(project_save) {
	project_save(false);
	return JS_UNDEFINED;
}

FN(ui_handle_create) {
	int64_t result = (int64_t)ui_handle_create();
	return JS_NewInt64(ctx, result);
}

FN(ui_handle_set_value) {
	int64_t p;
	JS_ToInt64(ctx, &p, argv[0]);
	ui_handle_t *h = (ui_handle_t *)p;
	double d;
	JS_ToFloat64(ctx, &d, argv[1]);
	h->value = d;
	return JS_UNDEFINED;
}

FN(ui_handle_get_value) {
	int64_t p;
	JS_ToInt64(ctx, &p, argv[0]);
	ui_handle_t *h = (ui_handle_t *)p;
	return JS_NewFloat64(ctx, h->value);
}

FN(ui_panel) {
	int64_t p;
	JS_ToInt64(ctx, &p, argv[0]);
	char *s = (char *)JS_ToCString(ctx, argv[1]);
	bool result = ui_panel((void *)p, s, false, false);
	return JS_NewBool(ctx, result);
}

FN(ui_button) {
	char *s = (char *)JS_ToCString(ctx, argv[0]);
	bool result = ui_button(s, UI_ALIGN_CENTER, "");
	return JS_NewBool(ctx, result);
}

FN(ui_text) {
	char *s = (char *)JS_ToCString(ctx, argv[0]);
	ui_text(s, UI_ALIGN_LEFT, 0);
	return JS_UNDEFINED;
}

FN(ui_text_input) {
	int64_t p;
	JS_ToInt64(ctx, &p, argv[0]);
	char *s = (char *)JS_ToCString(ctx, argv[1]);
	ui_text_input((void *)p, s, UI_ALIGN_LEFT, true, false);
	return JS_UNDEFINED;
}

FN(ui_slider) {
	int64_t p;
	JS_ToInt64(ctx, &p, argv[0]);
	char *s = (char *)JS_ToCString(ctx, argv[1]);
	double from = 0.0;
	double to = 1.0;
	bool filled = true;
	double prec = 100.0;
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
	int64_t p;
	JS_ToInt64(ctx, &p, argv[0]);
	char *s = (char *)JS_ToCString(ctx, argv[1]);
	ui_check((void *)p, s, "");
	return JS_UNDEFINED;
}

FN(ui_radio) {
	int64_t p;
	JS_ToInt64(ctx, &p, argv[0]);
	int32_t pos;
	JS_ToInt32(ctx, &pos, argv[1]);
	char *s = (char *)JS_ToCString(ctx, argv[2]);
	ui_radio((void *)p, pos, s, "");
	return JS_UNDEFINED;
}

FN(ui_row) {
	JSValue val_len = JS_GetPropertyStr(ctx, argv[0], "length");
	int len;
	JS_ToInt32(ctx, &len, val_len);
	f32_array_t *ratios = f32_array_create(len);
	for (int i = 0; i < len; ++i) {
		JSValue val = JS_GetPropertyUint32(ctx, argv[0], i);
		double f;
		JS_ToFloat64(ctx, &f, val);
		ratios->buffer[i] = f;
	}
	ui_row(ratios);
	return JS_UNDEFINED;
}

FN(ui_combo) {
	int64_t p;
	JS_ToInt64(ctx, &p, argv[0]);

	JSValue val_len = JS_GetPropertyStr(ctx, argv[1], "length");
	int len;
	JS_ToInt32(ctx, &len, val_len);
	char_ptr_array_t *texts = any_array_create(len);
	for (int i = 0; i < len; ++i) {
		JSValue val = JS_GetPropertyUint32(ctx, argv[1], i);
		char *s = (char *)JS_ToCString(ctx, val);
		texts->buffer[i] = s;
	}

	char *label = (char *)JS_ToCString(ctx, argv[2]);

	ui_combo((void *)p, texts, label, true, UI_ALIGN_LEFT, true);
	return JS_UNDEFINED;
}

extern any_array_t *nodes_material_categories;
extern any_array_t *nodes_material_list;
FN(nodes_material_category_add) {
	char *category_name = (char *)JS_ToCString(ctx, argv[0]);
	any_array_push(nodes_material_categories, category_name);
	size_t len;
	void *ab = JS_GetArrayBuffer(ctx, &len, argv[1]);
	buffer_t b = { .buffer = ab, .length = len, .capacity = len };
	any_array_push(nodes_material_list, armpack_decode(&b));
	return JS_UNDEFINED;
}

FN(nodes_material_category_remove) {
	char *category_name = (char *)JS_ToCString(ctx, argv[0]);
	int i = array_index_of(nodes_material_categories, category_name);
	array_splice(nodes_material_list, i, 1);
	array_splice(nodes_material_categories, i, 1);
	return JS_UNDEFINED;
}

extern any_map_t *parser_material_custom_nodes;
FN(parser_material_custom_nodes_set) {
	char *node_type = (char *)JS_ToCString(ctx, argv[0]);
	JSValue *p = malloc(sizeof(JSValue));\
	JSValue dup = JS_DupValue(ctx, argv[1]);\
	memcpy(p, &dup, sizeof(JSValue));\
	any_map_set(parser_material_custom_nodes, node_type, p);
	return JS_UNDEFINED;
}

FN(parser_material_custom_nodes_delete) {
	char *node_type = (char *)JS_ToCString(ctx, argv[0]);
	map_delete(parser_material_custom_nodes, node_type);
	return JS_UNDEFINED;
}

extern void *parser_material_frag;
FN(parser_material_frag_get) {
	return JS_NewInt64(ctx, (int64_t)parser_material_frag);
}

char *parser_material_parse_value_input(void *inp, bool vector_as_grayscale);
FN(parser_material_parse_value_input) {
	int64_t *node;
	JS_ToInt64(ctx, &node, argv[0]);
	int64_t i;
	JS_ToInt64(ctx, &i, argv[1]);
	char *s = parser_material_parse_value_input(((ui_node_t *)node)->inputs->buffer[i], false);
	return JS_NewString(ctx, s);
}

extern any_array_t *nodes_brush_categories;
extern any_array_t *nodes_brush_list;
FN(nodes_brush_category_add) {
	char *category_name = (char *)JS_ToCString(ctx, argv[0]);
	any_array_push(nodes_brush_categories, category_name);
	size_t len;
	void *ab = JS_GetArrayBuffer(ctx, &len, argv[1]);
	buffer_t b = { .buffer = ab, .length = len, .capacity = len };
	any_array_push(nodes_brush_list, armpack_decode(&b));
	return JS_UNDEFINED;
}

FN(nodes_brush_category_remove) {
	char *category_name = (char *)JS_ToCString(ctx, argv[0]);
	int i = array_index_of(nodes_brush_categories, category_name);
	array_splice(nodes_brush_list, i, 1);
	array_splice(nodes_brush_categories, i, 1);
	return JS_UNDEFINED;
}

extern any_map_t *parser_logic_custom_nodes;
FN(parser_logic_custom_nodes_set) {
	char *node_type = (char *)JS_ToCString(ctx, argv[0]);
	JSValue *p = malloc(sizeof(JSValue));\
	JSValue dup = JS_DupValue(ctx, argv[1]);\
	memcpy(p, &dup, sizeof(JSValue));\
	any_map_set(parser_logic_custom_nodes, node_type, p);
	return JS_UNDEFINED;
}

FN(parser_logic_custom_nodes_delete) {
	char *node_type = (char *)JS_ToCString(ctx, argv[0]);
	map_delete(parser_logic_custom_nodes, node_type);
	return JS_UNDEFINED;
}

extern any_map_t *util_mesh_unwrappers;
FN(util_mesh_unwrappers_set) {
	char *plugin_name = (char *)JS_ToCString(ctx, argv[0]);
	JSValue *p = malloc(sizeof(JSValue));\
	JSValue dup = JS_DupValue(ctx, argv[1]);\
	memcpy(p, &dup, sizeof(JSValue));\
	any_map_set(util_mesh_unwrappers, plugin_name, p);
	return JS_UNDEFINED;
}

FN(util_mesh_unwrappers_delete) {
	char *plugin_name = (char *)JS_ToCString(ctx, argv[0]);
	map_delete(util_mesh_unwrappers, plugin_name);
	return JS_UNDEFINED;
}

extern any_map_t *path_mesh_importers;
extern char_ptr_array_t *path_mesh_formats;
FN(path_mesh_importers_set) {
	char *format_name = (char *)JS_ToCString(ctx, argv[0]);
	JSValue *p = malloc(sizeof(JSValue));\
	JSValue dup = JS_DupValue(ctx, argv[1]);\
	memcpy(p, &dup, sizeof(JSValue));\
	any_map_set(path_mesh_importers, format_name, p);
	any_array_push(path_mesh_formats, format_name);
	return JS_UNDEFINED;
}

FN(path_mesh_importers_delete) {
	char *format_name = (char *)JS_ToCString(ctx, argv[0]);
	map_delete(path_mesh_importers, format_name);
	array_splice(path_mesh_formats, char_ptr_array_index_of(path_mesh_formats, format_name), 1);
	return JS_UNDEFINED;
}

extern any_map_t *path_texture_importers;
extern char_ptr_array_t *path_texture_formats;
FN(path_texture_importers_set) {
	char *format_name = (char *)JS_ToCString(ctx, argv[0]);
	JSValue *p = malloc(sizeof(JSValue));\
	JSValue dup = JS_DupValue(ctx, argv[1]);\
	memcpy(p, &dup, sizeof(JSValue));\
	any_map_set(path_texture_importers, format_name, p);
	any_array_push(path_texture_formats, format_name);
	return JS_UNDEFINED;
}

FN(path_texture_importers_delete) {
	char *format_name = (char *)JS_ToCString(ctx, argv[0]);
	map_delete(path_texture_importers, format_name);
	array_splice(path_texture_formats, char_ptr_array_index_of(path_texture_formats, format_name), 1);
	return JS_UNDEFINED;
}

FN(plugin_api_make_raw_mesh) {
	raw_mesh_t *mesh = calloc(sizeof(raw_mesh_t), 1);
	mesh->name = (char *)JS_ToCString(ctx, argv[0]);

	size_t len;
	void *ab = JS_GetArrayBuffer(ctx, &len, argv[1]);
	mesh->posa = malloc(sizeof(i16_array_t));
	mesh->posa->buffer = malloc(len);
	memcpy(mesh->posa->buffer, ab, len);
	mesh->posa->length = len / 2;

	ab = JS_GetArrayBuffer(ctx, &len, argv[2]);
	mesh->nora = malloc(sizeof(i16_array_t));
	mesh->nora->buffer = malloc(len);
	memcpy(mesh->nora->buffer, ab, len);
	mesh->nora->length = len / 2;

	ab = JS_GetArrayBuffer(ctx, &len, argv[3]);
	mesh->inda = malloc(sizeof(u32_array_t));
	mesh->inda->buffer = malloc(len);
	memcpy(mesh->inda->buffer, ab, len);
	mesh->inda->length = len / 4;

	double d;
	JS_ToFloat64(ctx, &d, argv[4]);
	mesh->scale_pos = d;
	mesh->scale_tex = 1.0;
	return JS_NewInt64(ctx, (int64_t)mesh);
}

void plugin_api_init() {
	JSValue global_obj = JS_GetGlobalObject(js_ctx);

	js_eval(lib);

	BIND(console_log, 1);
	BIND(console_info, 1);
	BIND(plugin_create, 0);
	BIND(plugin_notify_on_ui, 2);
	BIND(plugin_notify_on_update, 2);
	BIND(plugin_notify_on_delete, 2);
	BIND(ui_files_show, 4);
	BIND(ui_box_show_message, 2);
	BIND(data_get_blob, 1);
	BIND(data_delete_blob, 1);
	BIND(iron_file_save_bytes, 3);
	BIND(context_set_viewport_shader, 1);
	BIND(node_shader_add_uniform, 3);
	BIND(node_shader_write, 2);
	BIND(image_from_bytes, 3);
	BIND(project_filepath_get, 0);
	BIND(project_save, 0);

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

	BIND(nodes_material_category_add, 2);
	BIND(nodes_material_category_remove, 1);
	BIND(parser_material_custom_nodes_set, 2);
	BIND(parser_material_custom_nodes_delete, 1);
	BIND(parser_material_frag_get, 0);
	BIND(parser_material_parse_value_input, 2);

	BIND(nodes_brush_category_add, 2);
	BIND(nodes_brush_category_remove, 1);
	BIND(parser_logic_custom_nodes_set, 2);
	BIND(parser_logic_custom_nodes_delete, 1);

	BIND(util_mesh_unwrappers_set, 2);
	BIND(util_mesh_unwrappers_delete, 1);
	BIND(path_mesh_importers_set, 2);
	BIND(path_mesh_importers_delete, 1);
	BIND(path_texture_importers_set, 2);
	BIND(path_texture_importers_delete, 1);

	BIND(plugin_api_make_raw_mesh, 5);

	plugin_embed();

	JS_FreeValue(js_ctx, global_obj);
}
