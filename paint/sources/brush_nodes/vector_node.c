
#include "../global.h"

logic_node_value_t *vector_node_get(vector_node_t *self, i32 from) {
	f32 x                 = logic_node_input_get(self->base->inputs->buffer[0])->_f32;
	f32 y                 = logic_node_input_get(self->base->inputs->buffer[1])->_f32;
	f32 z                 = logic_node_input_get(self->base->inputs->buffer[2])->_f32;
	self->value.x         = x;
	self->value.y         = y;
	self->value.z         = z;
	logic_node_value_t *v = GC_ALLOC_INIT(logic_node_value_t, {._vec4 = self->value});
	return v;
}

gpu_texture_t *vector_node_get_as_image(vector_node_t *self, i32 from) {
	// let x: f32 = logic_node_input_get(self.base.inputs[0]);
	// let y: f32 = logic_node_input_get(self.base.inputs[1]);
	// let z: f32 = logic_node_input_get(self.base.inputs[2]);
	if (self->image != NULL) {
		gpu_delete_texture(self->image);
	}
	buffer_t     *b  = buffer_create(16);
	float_node_t *n0 = self->base->inputs->buffer[0]->node;
	float_node_t *n1 = self->base->inputs->buffer[1]->node;
	float_node_t *n2 = self->base->inputs->buffer[2]->node;
	buffer_set_f32(b, 0, n0->value);
	buffer_set_f32(b, 4, n1->value);
	buffer_set_f32(b, 8, n2->value);
	buffer_set_f32(b, 12, 1.0);
	self->image = gpu_create_texture_from_bytes(b, 1, 1, GPU_TEXTURE_FORMAT_RGBA128);
	return self->image;
}

void vector_node_set(vector_node_t *self, f32_array_t *value) {
	logic_node_input_set(self->base->inputs->buffer[0], f32_array_create_x(value->buffer[0]));
	logic_node_input_set(self->base->inputs->buffer[1], f32_array_create_x(value->buffer[1]));
	logic_node_input_set(self->base->inputs->buffer[2], f32_array_create_x(value->buffer[2]));
}

vector_node_t *vector_node_create(ui_node_t *raw, f32_array_t *args) {
	vector_node_t *n      = GC_ALLOC_INIT(vector_node_t, {0});
	n->base               = logic_node_create(n);
	n->base->get          = vector_node_get;
	n->base->get_as_image = vector_node_get_as_image;
	n->base->set          = vector_node_set;
	n->value              = vec4_create(0.0, 0.0, 0.0, 1.0);

	if (args != NULL) {
		logic_node_add_input(n->base, float_node_create(NULL, f32_array_create_x(args->buffer[0])), 0);
		logic_node_add_input(n->base, float_node_create(NULL, f32_array_create_x(args->buffer[1])), 0);
		logic_node_add_input(n->base, float_node_create(NULL, f32_array_create_x(args->buffer[2])), 0);
	}

	return n;
}
