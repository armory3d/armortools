#pragma once

#include <kinc/global.h>
#include <kinc/graphics4/vertexstructure.h>

/*! \file vertexstructure.h
    \brief Provides types for setting up the structure of vertices in a vertex-buffer.
*/

typedef kinc_g4_vertex_data_t kinc_g5_vertex_data_t;
typedef kinc_g4_vertex_element_t kinc_g5_vertex_element_t;
typedef kinc_g4_vertex_structure_t kinc_g5_vertex_structure_t;

static inline void kinc_g5_vertex_structure_init(kinc_g5_vertex_structure_t *structure) {
	kinc_g4_vertex_structure_init(structure);
}

static inline void kinc_g5_vertex_structure_add(kinc_g5_vertex_structure_t *structure, const char *name, kinc_g5_vertex_data_t data) {
	kinc_g4_vertex_structure_add(structure, name, data);
}
