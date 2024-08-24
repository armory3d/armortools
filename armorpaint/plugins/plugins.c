
#include "../../../base/sources/plugin_api.h"

void proc_xatlas_unwrap(void *mesh);
FN(proc_xatlas_unwrap) {
	int64_t mesh;
	JS_ToInt64(ctx, &mesh, argv[0]);
	proc_xatlas_unwrap((void *)mesh);
	return JS_UNDEFINED;
}

void *io_svg_parse(char *buf);
FN(io_svg_parse) {
	size_t len;
	void *ab = JS_GetArrayBuffer(ctx, &len, argv[0]);
	return JS_NewInt64(ctx, (int64_t)io_svg_parse(ab));
}

void *io_usd_parse(char *buf, size_t size);
FN(io_usd_parse) {
	size_t len;
	void *ab = JS_GetArrayBuffer(ctx, &len, argv[0]);
	return JS_NewInt64(ctx, (int64_t)io_usd_parse(ab, len));
}

void *io_gltf_parse(char *buf, size_t size);
FN(io_gltf_parse) {
	size_t len;
	void *ab = JS_GetArrayBuffer(ctx, &len, argv[0]);
	return JS_NewInt64(ctx, (int64_t)io_gltf_parse(ab, len));
}

void *io_fbx_parse(char *buf, size_t size);
FN(io_fbx_parse) {
	size_t len;
	void *ab = JS_GetArrayBuffer(ctx, &len, argv[0]);
	return JS_NewInt64(ctx, (int64_t)io_fbx_parse(ab, len));
}

void plugin_embed() {
	JSValue global_obj = JS_GetGlobalObject(js_ctx);

	BIND(proc_xatlas_unwrap, 1);
	BIND(io_svg_parse, 1);
	BIND(io_usd_parse, 1);
	BIND(io_gltf_parse, 1);
	BIND(io_fbx_parse, 1);

	JS_FreeValue(js_ctx, global_obj);
}
