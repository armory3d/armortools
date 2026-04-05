
#include "../global.h"

logic_node_value_t *time_node_get(time_node_t *self, i32 from) {
	if (from == 0) {
		logic_node_value_t *v = GC_ALLOC_INIT(logic_node_value_t, {._f32 = sys_time()});
		return v;
	}
	else if (from == 1) {
		logic_node_value_t *v = GC_ALLOC_INIT(logic_node_value_t, {._f32 = sys_delta()});
		return v;
	}
	else {
		logic_node_value_t *v = GC_ALLOC_INIT(logic_node_value_t, {._f32 = g_context->brush_time});
		return v;
	}
}

time_node_t *time_node_create(ui_node_t *raw, f32_array_t *args) {
	time_node_t *n = GC_ALLOC_INIT(time_node_t, {0});
	n->base        = logic_node_create(n);
	n->base->get   = time_node_get;
	return n;
}
