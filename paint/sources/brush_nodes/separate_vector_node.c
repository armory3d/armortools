
#include "../global.h"

logic_node_value_t *separate_vector_node_get(separate_vector_node_t *self, i32 from) {
	vec4_t vector = logic_node_input_get(self->base->inputs->buffer[0])->_vec4;
	if (from == 0) {
		logic_node_value_t *v = GC_ALLOC_INIT(logic_node_value_t, {._f32 = vector.x});
		return v;
	}
	else if (from == 1) {
		logic_node_value_t *v = GC_ALLOC_INIT(logic_node_value_t, {._f32 = vector.y});
		return v;
	}
	else {
		logic_node_value_t *v = GC_ALLOC_INIT(logic_node_value_t, {._f32 = vector.z});
		return v;
	}
}

separate_vector_node_t *separate_vector_node_create(ui_node_t *raw, f32_array_t *args) {
	separate_vector_node_t *n = GC_ALLOC_INIT(separate_vector_node_t, {0});
	n->base                   = logic_node_create(n);
	n->base->get              = separate_vector_node_get;
	return n;
}
