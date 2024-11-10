#pragma once

#include <kinc/global.h>

#include <kinc/graphics4/vertexstructure.h>

/*! \file vertexstructure.h
    \brief Provides types for setting up the structure of vertices in a vertex-buffer.
*/

#ifdef __cplusplus
extern "C" {
#endif

typedef kinc_g4_vertex_data_t kinc_g5_vertex_data_t;

// typedef kinc_g4_vertex_attribute_t kinc_g5_vertex_attribute_t;

typedef kinc_g4_vertex_element_t kinc_g5_vertex_element_t;

typedef kinc_g4_vertex_structure_t kinc_g5_vertex_structure_t;

/// <summary>
/// Initializes a vertex-structure.
/// </summary>
/// <param name="structure">The structure to initialize</param>
/// <returns></returns>
static inline void kinc_g5_vertex_structure_init(kinc_g5_vertex_structure_t *structure) {
	kinc_g4_vertex_structure_init(structure);
}

/// <summary>
/// Adds an element to a vertex-structure.
/// </summary>
/// <param name="structure">The structure to add an element to</param>
/// <param name="name">The name to use for the new element</param>
/// <param name="data">The type of data to assign for the new element</param>
/// <returns></returns>
static inline void kinc_g5_vertex_structure_add(kinc_g5_vertex_structure_t *structure, const char *name, kinc_g5_vertex_data_t data) {
	kinc_g4_vertex_structure_add(structure, name, data);
}

#ifdef __cplusplus
}
#endif
