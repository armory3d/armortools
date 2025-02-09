
// Red triangle test
// ../../../make --graphics vulkan --run

let vs: string = "#version 330\n\
in vec3 pos; \
void main() { \
	gl_Position = vec4(pos, 1.0); \
} \
";

let fs: string = "#version 330\n\
out vec4 frag_color; \
void main() { \
	frag_color = vec4(1.0, 0.0, 0.0, 1.0); \
} \
";

let vertices: f32[] = [
	-1.0, -1.0, 0.0,
	 1.0, -1.0, 0.0,
	 0.0,  1.0, 0.0
];
let indices: i32[] = [0, 1, 2];
let pipeline: any;
let vb: any;
let ib: any;

function render() {
	iron_g4_begin(null, null);

	let flags: i32 = 0;
	flags |= 1; // Color
	flags |= 2; // Depth
	iron_g4_clear(flags, 0xff000000, 1.0);

	iron_g4_set_pipeline(pipeline);
	iron_g4_set_vertex_buffer(vb);
	iron_g4_set_index_buffer(ib);
	iron_g4_draw_indexed_vertices(0, -1);

	iron_g4_end();
}

function main() {
	let resizable: i32 = 1;
	let minimizable: i32 = 2;
	let maximizable: i32 = 4;
	iron_init("ArmorCore", 640, 480, true, 0, resizable | minimizable | maximizable, -1, -1, 60);

	pipeline = iron_g4_create_pipeline();
	let f32_3x: vertex_data_t = vertex_data_t.F32_3X;
	let elem: kinc_vertex_elem_t = { name: "pos", data: f32_3x };
	let elems: kinc_vertex_elem_t[] = [elem];

	let structure0: vertex_struct_t = { elements: elems };
	let vert: any = iron_g4_create_vertex_shader_from_source(vs);
	let frag: any = iron_g4_create_fragment_shader_from_source(fs);

	let masks: bool[] = [true, true, true, true, true, true, true, true];
	let attachments: i32[] = [0];

	let state: iron_pipeline_state_t = {
		cull_mode: 0,
		depth_write: false,
		depth_mode: 0,
		blend_source: 0,
		blend_dest: 0,
		alpha_blend_source: 0,
		alpha_blend_dest: 0,
		color_write_masks_red: masks,
		color_write_masks_green: masks,
		color_write_masks_blue: masks,
		color_write_masks_alpha: masks,
		color_attachment_count: 1,
		color_attachments: attachments,
		depth_attachment_bits: 0
	};

	iron_g4_compile_pipeline(pipeline, structure0, null, null, null, 1, vert, frag, null, state);

	vb = iron_g4_create_vertex_buffer(vertices.length / 3, structure0.elements, 0, 0);
	let vb_data: buffer_t = iron_g4_lock_vertex_buffer(vb);
	for (let i: i32 = 0; i < vertices.length; i++) {
		buffer_set_f32(vb_data, i * 4, vertices[i]);
	}
	iron_g4_unlock_vertex_buffer(vb);

	ib = iron_g4_create_index_buffer(indices.length);
	let ib_data: u32_array_t = iron_g4_lock_index_buffer(ib);
	for (let i: i32 = 0; i < indices.length; i++) {
		ib_data[i] = indices[i];
	}
	iron_g4_unlock_index_buffer(ib);

	iron_set_update_callback(render);
}
