
#include "../../../base/sources/plugin_api.h"

void plugin_embed_base() {
	JSValue global_obj = JS_GetGlobalObject(js_ctx);

	JS_FreeValue(js_ctx, global_obj);
}
