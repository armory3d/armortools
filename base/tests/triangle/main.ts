
// Red triangle test
// ../../../make --graphics vulkan --run

let pipeline: iron_g5_pipeline_t;
let vb: any;
let ib: any;

function render() {
	_iron_g4_begin(null, null);

	let flags: i32 = 0;
	flags |= 1; // Color
	flags |= 2; // Depth
	iron_g5_clear(0xff000000, 1.0, flags);

	iron_g5_set_pipeline(pipeline);
	iron_g4_set_vertex_buffer(vb);
	iron_g4_set_index_buffer(ib);
	iron_g4_draw_indexed_vertices(0, -1);

	_iron_g4_end();
}

function main() {
	let resizable: i32 = 1;
	let minimizable: i32 = 2;
	let maximizable: i32 = 4;
	_iron_init("Iron", 640, 480, true, 0, resizable | minimizable | maximizable, -1, -1, 60);

	pipeline = iron_g4_create_pipeline();
	let f32_3x: vertex_data_t = vertex_data_t.F32_3X;
	let elem: iron_g5_vertex_element_t = { name: "pos", data: f32_3x };
	let elems: iron_g5_vertex_element_t[] = [elem];

	let structure0: iron_g5_vertex_structure_t = { elements: elems };

	let vs_buffer: buffer_t = iron_load_blob("./data/test.vert.spirv");
	let fs_buffer: buffer_t = iron_load_blob("./data/test.frag.spirv");

	let vert: any = iron_g4_create_shader(vs_buffer, shader_type_t.VERTEX);
	let frag: any = iron_g4_create_shader(fs_buffer, shader_type_t.FRAGMENT);

	let masks: bool[] = [true, true, true, true, true, true, true, true];
	let attachments: i32[] = [0];

	pipeline.cull_mode = 0;
	pipeline.depth_write = false;
	pipeline.depth_mode = 0;
	pipeline.blend_source = 0;
	pipeline.blend_destination = 0;
	pipeline.alpha_blend_source = 0;
	pipeline.alpha_blend_destination = 0;
	pipeline.color_write_mask_red = masks;
	pipeline.color_write_mask_green = masks;
	pipeline.color_write_mask_blue = masks;
	pipeline.color_write_mask_alpha = masks;
	pipeline.color_attachment_count = 1;
	pipeline.color_attachment = attachments;
	pipeline.depth_attachment_bits = 0;
	pipeline.vertex_shader = vert;
	pipeline.fragment_shader = frag;
	pipeline.input_layout = structure0;

	iron_g4_compile_pipeline(pipeline);

	let vertices: f32[] = [
		-1.0, -1.0, 0.0,
		 1.0, -1.0, 0.0,
		 0.0,  1.0, 0.0
	];
	let indices: i32[] = [0, 1, 2];

	vb = iron_g4_create_vertex_buffer(vertices.length / 3, structure0.elements, 0);
	let vb_data: buffer_t = iron_g4_lock_vertex_buffer(vb);
	for (let i: i32 = 0; i < vertices.length; i++) {
		buffer_set_f32(vb_data, i * 4, vertices[i]);
	}
	iron_g4_vertex_buffer_unlock_all(vb);

	ib = iron_g4_create_index_buffer(indices.length);
	let ib_data: u32_array_t = iron_g4_lock_index_buffer(ib);
	for (let i: i32 = 0; i < indices.length; i++) {
		ib_data[i] = indices[i];
	}
	iron_g4_index_buffer_unlock_all(ib);

	_iron_set_update_callback(render);
}
