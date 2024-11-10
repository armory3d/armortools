#include "vertexbuffer.h"

static void init_vertex_element(kinc_g4_vertex_element_t *element, const char *name, kinc_g4_vertex_data_t data) {
	element->name = name;
	element->data = data;
}

void kinc_g4_vertex_structure_init(kinc_g4_vertex_structure_t *structure) {
	structure->size = 0;
	structure->instanced = false;
}

void kinc_g4_vertex_structure_add(kinc_g4_vertex_structure_t *structure, const char *name, kinc_g4_vertex_data_t data) {
	init_vertex_element(&structure->elements[structure->size++], name, data);
}

void kinc_g4_set_vertex_buffer(kinc_g4_vertex_buffer_t *buffer) {
	kinc_g4_vertex_buffer_t *buffers[1] = {buffer};
	kinc_g4_set_vertex_buffers(buffers, 1);
}
