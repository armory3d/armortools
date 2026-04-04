
#include "../global.h"

logic_node_value_t *float_node_get(float_node_t *self, i32 from) {
	if (self->base->inputs->length > 0) {
		return logic_node_input_get(self->base->inputs->buffer[0]);
	}
	else {
		logic_node_value_t *v = GC_ALLOC_INIT(logic_node_value_t, {._f32 = self->value});
		return v;
	}
}

gpu_texture_t *float_node_get_as_image(float_node_t *self, i32 from) {
	if (self->base->inputs->length > 0) {
		return logic_node_input_get_as_image(self->base->inputs->buffer[0]);
	}
	if (self->image != NULL) {
		gpu_delete_texture(self->image);
	}
	buffer_t *b = buffer_create(16);
	buffer_set_f32(b, 0, self->value);
	buffer_set_f32(b, 4, self->value);
	buffer_set_f32(b, 8, self->value);
	buffer_set_f32(b, 12, 1.0);
	self->image = gpu_create_texture_from_bytes(b, 1, 1, GPU_TEXTURE_FORMAT_RGBA128);
	return self->image;
}

void float_node_set(float_node_t *self, f32_array_t *value) {
	if (self->base->inputs->length > 0) {
		logic_node_input_set(self->base->inputs->buffer[0], value);
	}
	else {
		self->value = value->buffer[0];
	}
}

float_node_t *float_node_create(ui_node_t *raw, f32_array_t *args) {
	float_node_t *n       = GC_ALLOC_INIT(float_node_t, {0});
	n->base               = logic_node_create(n);
	n->base->get          = float_node_get;
	n->base->get_as_image = float_node_get_as_image;
	n->base->set          = float_node_set;
	n->value              = args == NULL ? 0.5 : args->buffer[0];
	return n;
}
