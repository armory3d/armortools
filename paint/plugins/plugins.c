
#ifdef WITH_PLUGINS

#include "iron_array.h"
#include "iron_map.h"
#include "iron_ui.h"
#include "engine.h"

void *io_svg_parse(char *buf);
void *io_exr_parse(char *buf, size_t len);
void *io_tiff_parse(uint8_t *buf, size_t len);
void *io_gltf_parse(char *buf, size_t size, const char *path);
void *io_fbx_parse(char *buf, size_t size);
void proc_uv_unwrap(void *mesh);

extern any_map_t      *import_mesh_importers;
extern string_array_t *_path_mesh_formats;
string_array_t        *path_mesh_formats(void);
extern any_map_t      *import_texture_importers;
extern string_array_t *_path_texture_formats;
string_array_t        *path_texture_formats(void);
extern any_map_t *util_mesh_unwrappers;

static void *import_exr(char *path) {
	buffer_t *b = data_get_blob(path);
	data_delete_blob(path);
	return io_exr_parse((char *)b->buffer, b->length);
}

static void *import_tiff(char *path) {
	buffer_t *b = data_get_blob(path);
	data_delete_blob(path);
	return io_tiff_parse((uint8_t *)b->buffer, b->length);
}

static void *import_svg(char *path) {
	buffer_t *b = data_get_blob(path);
	data_delete_blob(path);
	return io_svg_parse((char *)b->buffer);
}

static void *import_gltf_glb(char *path) {
	buffer_t *b = data_get_blob(path);
	data_delete_blob(path);
	return io_gltf_parse((char *)b->buffer, b->length, path);
}

static void *import_fbx(char *path) {
	buffer_t *b = data_get_blob(path);
	data_delete_blob(path);
	return io_fbx_parse((char *)b->buffer, b->length);
}

void plugins_init() {
	path_texture_formats(); // Init array
	any_map_set(import_texture_importers, "exr", import_exr);
	any_array_push(_path_texture_formats, "exr");
	any_map_set(import_texture_importers, "svg", import_svg);
	any_array_push(_path_texture_formats, "svg");
	any_map_set(import_texture_importers, "tiff", import_tiff);
	any_array_push(_path_texture_formats, "tiff");
	any_map_set(import_texture_importers, "tif", import_tiff);
	any_array_push(_path_texture_formats, "tif");

	path_mesh_formats(); // Init array
	any_map_set(import_mesh_importers, "gltf", import_gltf_glb);
	any_array_push(_path_mesh_formats, "gltf");
	any_map_set(import_mesh_importers, "glb", import_gltf_glb);
	any_array_push(_path_mesh_formats, "glb");
	any_map_set(import_mesh_importers, "fbx", import_fbx);
	any_array_push(_path_mesh_formats, "fbx");

	any_map_set(util_mesh_unwrappers, "uv_unwrap", proc_uv_unwrap);
}

#endif
