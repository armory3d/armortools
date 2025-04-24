
// Red triangle test
// ../../make --run

let pipeline: iron_gpu_pipeline_t;
let vb: any;
let ib: any;

function render() {
	_gpu_begin(null, null, clear_flag_t.COLOR | clear_flag_t.DEPTH, 0xff000000, 1.0);
	gpu_set_pipeline(pipeline);
	gpu_set_vertex_buffer(vb);
	gpu_set_index_buffer(ib);
	gpu_draw();
	_gpu_end();
}

function main() {
	let ops: iron_window_options_t = {
		title: "Iron",
		width: 640,
		height: 480,
		x: -1,
		y: -1,
		mode: window_mode_t.WINDOWED,
		features: window_features_t.RESIZABLE | window_features_t.MINIMIZABLE | window_features_t.MAXIMIZABLE,
		vsync: true,
		frequency: 60,
		use_depth: true
	};
	_iron_init(ops);

	pipeline = gpu_create_pipeline();
	let vs: iron_gpu_vertex_structure_t = gpu_vertex_struct_create();
	gpu_vertex_struct_add(vs, "pos", vertex_data_t.F32_3X);
	let vs_buffer: buffer_t = iron_load_blob("./data/test.vert" + sys_shader_ext());
	let fs_buffer: buffer_t = iron_load_blob("./data/test.frag" + sys_shader_ext());
	let vert: any = gpu_create_shader(vs_buffer, shader_type_t.VERTEX);
	let frag: any = gpu_create_shader(fs_buffer, shader_type_t.FRAGMENT);
	pipeline.vertex_shader = vert;
	pipeline.fragment_shader = frag;
	pipeline.input_layout = vs;
	gpu_compile_pipeline(pipeline);

	let vertices: f32[] = [
		-1.0, -1.0, 0.0,
		 1.0, -1.0, 0.0,
		 0.0,  1.0, 0.0
	];
	let indices: i32[] = [0, 1, 2];

	vb = gpu_create_vertex_buffer(vertices.length / 3, vs.elements, 0);
	let vb_data: buffer_t = gpu_lock_vertex_buffer(vb);
	for (let i: i32 = 0; i < vertices.length; i++) {
		buffer_set_f32(vb_data, i * 4, vertices[i]);
	}
	iron_gpu_vertex_buffer_unlock(vb);

	ib = gpu_create_index_buffer(indices.length);
	let ib_data: u32_array_t = gpu_lock_index_buffer(ib);
	for (let i: i32 = 0; i < indices.length; i++) {
		ib_data[i] = indices[i];
	}
	gpu_index_buffer_unlock(ib);

	_iron_set_update_callback(render);
}
