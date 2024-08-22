
#include "../../../base/sources/plugin_api.h"

void proc_xatlas_unwrap(void *mesh);
FN(proc_xatlas_unwrap) {
	int64_t mesh;
    JS_ToInt64(ctx, &mesh, argv[0]);
	proc_xatlas_unwrap((void *)mesh);
	return JS_UNDEFINED;
}

void plugin_embed() {
	JSValue global_obj = JS_GetGlobalObject(js_ctx);

	BIND(proc_xatlas_unwrap, 1);

	JS_FreeValue(js_ctx, global_obj);
}
