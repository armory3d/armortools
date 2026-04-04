
#include "../global.h"

logic_node_value_t *tex_image_node_get(tex_image_node_t *self, i32 from) {
	string_array_t *ar   = ui_nodes_enum_texts(self->raw->type);
	i32               i    = self->raw->buttons->buffer[0]->default_value->buffer[0];
	char         *file = ar->buffer[i];

	if (from == 0) {
		logic_node_value_t *v = GC_ALLOC_INIT(logic_node_value_t, {._str = string("%s.rgb", file)});
		return v;
	}
	else {
		logic_node_value_t *v = GC_ALLOC_INIT(logic_node_value_t, {._str = string("%s.a", file)});
		return v;
	}
}

tex_image_node_t *tex_image_node_create(ui_node_t *raw, f32_array_t *args) {
	tex_image_node_t *n = GC_ALLOC_INIT(tex_image_node_t, {0});
	n->base             = logic_node_create(n);
	n->base->get        = tex_image_node_get;
	n->raw              = raw;
	return n;
}
