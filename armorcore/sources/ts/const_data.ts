
let const_data_screen_aligned_vb: vertex_buffer_t = null;
let const_data_screen_aligned_ib: index_buffer_t = null;
let const_data_skydome_vb: vertex_buffer_t = null;
let const_data_skydome_ib: index_buffer_t = null;

function const_data_create_screen_aligned_data() {
	// Over-sized triangle
	let data: f32[] = [-1.0, -1.0, 3.0, -1.0, -1.0, 3.0];
	let indices: i32[] = [0, 1, 2];

	// Mandatory vertex data names and sizes
	let structure: vertex_struct_t = g4_vertex_struct_create();
	g4_vertex_struct_add(structure, "pos", vertex_data_t.F32_2X);
	const_data_screen_aligned_vb = g4_vertex_buffer_create(math_floor(data.length / math_floor(g4_vertex_struct_byte_size(structure) / 4)), structure, usage_t.STATIC);
	let vertices: buffer_t = g4_vertex_buffer_lock(const_data_screen_aligned_vb);
	for (let i: i32 = 0; i < math_floor((vertices.length) / 4); ++i) {
		buffer_set_f32(vertices, i * 4, data[i]);
	}
	g4_vertex_buffer_unlock(const_data_screen_aligned_vb);

	const_data_screen_aligned_ib = g4_index_buffer_create(indices.length);
	let id: u32_array_t = g4_index_buffer_lock(const_data_screen_aligned_ib);
	for (let i: i32 = 0; i < id.length; ++i) {
		id[i] = indices[i];
	}
	g4_index_buffer_unlock(const_data_screen_aligned_ib);
}

///include "const_data.h"
declare let _const_data_skydome_indices: i32_ptr;
declare let _const_data_skydome_indices_count: i32;
declare let _const_data_skydome_pos: f32_ptr;
declare let _const_data_skydome_pos_count: i32;
declare let _const_data_skydome_nor: f32_ptr;
declare let _const_data_skydome_nor_count: i32;

function const_data_create_skydome_data() {
	let structure: vertex_struct_t = g4_vertex_struct_create();
	g4_vertex_struct_add(structure, "pos", vertex_data_t.F32_3X);
	g4_vertex_struct_add(structure, "nor", vertex_data_t.F32_3X);
	let struct_length: i32 = math_floor(g4_vertex_struct_byte_size(structure) / 4);
	const_data_skydome_vb = g4_vertex_buffer_create(math_floor(_const_data_skydome_pos_count / 3), structure, usage_t.STATIC);
	let vertices: buffer_t = g4_vertex_buffer_lock(const_data_skydome_vb);
	for (let i: i32 = 0; i < math_floor((vertices.length) / 4 / struct_length); ++i) {
		buffer_set_f32(vertices, (i * struct_length) * 4, ARRAY_ACCESS(_const_data_skydome_pos, i * 3));
		buffer_set_f32(vertices, (i * struct_length + 1) * 4, ARRAY_ACCESS(_const_data_skydome_pos, i * 3 + 1));
		buffer_set_f32(vertices, (i * struct_length + 2) * 4, ARRAY_ACCESS(_const_data_skydome_pos, i * 3 + 2));
		buffer_set_f32(vertices, (i * struct_length + 3) * 4, ARRAY_ACCESS(_const_data_skydome_nor, i * 3));
		buffer_set_f32(vertices, (i * struct_length + 4) * 4, ARRAY_ACCESS(_const_data_skydome_nor, i * 3 + 1));
		buffer_set_f32(vertices, (i * struct_length + 5) * 4, ARRAY_ACCESS(_const_data_skydome_nor, i * 3 + 2));
	}
	g4_vertex_buffer_unlock(const_data_skydome_vb);

	const_data_skydome_ib = g4_index_buffer_create(_const_data_skydome_indices_count);
	let id: u32_array_t = g4_index_buffer_lock(const_data_skydome_ib);
	for (let i: i32 = 0; i < id.length; ++i) {
		id[i] = ARRAY_ACCESS(_const_data_skydome_indices, i);
	}
	g4_index_buffer_unlock(const_data_skydome_ib);
}
