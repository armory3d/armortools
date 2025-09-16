
#include "../../../base/sources/plugins/plugin_api.h"
#include "iron_array.h"
#include "iron_map.h"
#include "iron_ui.h"
#include "iron_ui_nodes.h"

void proc_xatlas_unwrap(void *mesh);
FN(proc_xatlas_unwrap) {
	uint64_t mesh;
	JS_ToBigUint64(ctx, &mesh, argv[0]);
	proc_xatlas_unwrap((void *)mesh);
	return JS_UNDEFINED;
}

void plugin_uv_unwrap_button();
FN(plugin_uv_unwrap_button) {
	plugin_uv_unwrap_button();
	return JS_UNDEFINED;
}

void *io_svg_parse(char *buf);
FN(io_svg_parse) {
	size_t len;
	void *ab = JS_GetArrayBuffer(ctx, &len, argv[0]);
	return JS_NewBigUint64(ctx, (uint64_t)io_svg_parse(ab));
}

void *io_exr_parse(char *buf);
FN(io_exr_parse) {
	size_t len;
	void *ab = JS_GetArrayBuffer(ctx, &len, argv[0]);
	return JS_NewBigUint64(ctx, (uint64_t)io_exr_parse(ab));
}

void *io_usd_parse(char *buf, size_t size);
FN(io_usd_parse) {
	size_t len;
	void *ab = JS_GetArrayBuffer(ctx, &len, argv[0]);
	return JS_NewBigUint64(ctx, (uint64_t)io_usd_parse(ab, len));
}

void *io_gltf_parse(char *buf, size_t size, const char *path);
FN(io_gltf_parse) {
	size_t len;
	void *ab = JS_GetArrayBuffer(ctx, &len, argv[0]);
	const char *path = JS_ToCString(ctx, argv[1]);
	return JS_NewBigUint64(ctx, (uint64_t)io_gltf_parse(ab, len, path));
}

void *io_fbx_parse(char *buf, size_t size);
FN(io_fbx_parse) {
	size_t len;
	void *ab = JS_GetArrayBuffer(ctx, &len, argv[0]);
	return JS_NewBigUint64(ctx, (uint64_t)io_fbx_parse(ab, len));
}

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

VOID_FN_CB(context_set_viewport_shader)

void node_shader_add_constant(void *p, char *s0, char *s1, bool b);
FN(node_shader_add_constant) {
	uint64_t p;
	JS_ToBigUint64(ctx, &p, argv[0]);
	char *s0 = (char *)JS_ToCString(ctx, argv[1]);
	char *s1 = (char *)JS_ToCString(ctx, argv[2]);
	node_shader_add_constant((void *)p, s0, s1, false);
	return JS_UNDEFINED;
}

VOID_FN_PTR_STR(node_shader_write_frag)

extern char *project_filepath;
FN(project_filepath_get) {
	return JS_NewString(ctx, project_filepath);
}

void project_save(bool b);
FN(project_save) {
	project_save(false);
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

extern void *parser_material_kong;
FN(parser_material_kong_get) {
	return JS_NewBigUint64(ctx, (uint64_t)parser_material_kong);
}

char *parser_material_parse_value_input(void *inp, bool vector_as_grayscale);
FN(parser_material_parse_value_input) {
	uint64_t *node;
	JS_ToBigUint64(ctx, &node, argv[0]);
	int64_t i;
	JS_ToInt64(ctx, &i, argv[1]);
	char *s = parser_material_parse_value_input(((ui_node_t *)node)->inputs->buffer[i], false);
	return JS_NewString(ctx, s);
}

extern any_array_t *nodes_brush_categories;
extern any_array_t *nodes_brush_list;
void nodes_brush_list_init();
FN(nodes_brush_category_add) {
	char *category_name = (char *)JS_ToCString(ctx, argv[0]);
	any_array_push(nodes_brush_categories, category_name);
	size_t len;
	void *ab = JS_GetArrayBuffer(ctx, &len, argv[1]);
	buffer_t b = { .buffer = ab, .length = len, .capacity = len };
	nodes_brush_list_init();
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

void plugin_embed() {
	JSValue global_obj = JS_GetGlobalObject(js_ctx);

	BIND(proc_xatlas_unwrap, 1);
	BIND(plugin_uv_unwrap_button, 0);
	BIND(io_svg_parse, 1);
	BIND(io_exr_parse, 1);
	BIND(io_usd_parse, 1);
	BIND(io_gltf_parse, 2);
	BIND(io_fbx_parse, 1);

	BIND(console_log, 1);
	BIND(console_info, 1);
	BIND(plugin_create, 0);
	BIND(plugin_notify_on_ui, 2);
	BIND(plugin_notify_on_update, 2);
	BIND(plugin_notify_on_delete, 2);
	BIND(ui_files_show, 4);
	BIND(ui_box_show_message, 2);
	BIND(context_set_viewport_shader, 1);
	BIND(node_shader_add_constant, 3);
	BIND(node_shader_write_frag, 2);
	BIND(project_filepath_get, 0);
	BIND(project_save, 0);

	BIND(nodes_material_category_add, 2);
	BIND(nodes_material_category_remove, 1);
	BIND(parser_material_custom_nodes_set, 2);
	BIND(parser_material_custom_nodes_delete, 1);
	BIND(parser_material_kong_get, 0);
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

	JS_FreeValue(js_ctx, global_obj);
}
