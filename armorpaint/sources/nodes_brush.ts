
let nodes_brush_categories: string[] = [
	_tr("Nodes")
];

let nodes_brush_category0: ui_node_t[];
let nodes_brush_list: node_list_t[];

let nodes_brush_creates: map_t<string, (args: f32_array_t)=>logic_node_ext_t>;

function nodes_brush_init() {
	nodes_brush_creates = map_create();
	map_set(nodes_brush_creates, "brush_output_node", brush_output_node_create);
	// map_set(nodes_brush_creates, "tex_image_node", tex_image_node_create);
	map_set(nodes_brush_creates, "TEX_IMAGE", tex_image_node_create);
	map_set(nodes_brush_creates, "input_node", input_node_create);
	map_set(nodes_brush_creates, "math_node", math_node_create);
	map_set(nodes_brush_creates, "random_node", random_node_create);
	map_set(nodes_brush_creates, "separate_vector_node", separate_vector_node_create);
	map_set(nodes_brush_creates, "time_node", time_node_create);
	map_set(nodes_brush_creates, "float_node", float_node_create);
	map_set(nodes_brush_creates, "vector_node", vector_node_create);
	map_set(nodes_brush_creates, "vector_math_node", vector_math_node_create);

	nodes_brush_category0 = [
		tex_image_node_def,
		input_node_def,
		math_node_def,
		random_node_def,
		separate_vector_node_def,
		time_node_def,
		float_node_def,
		vector_node_def,
		vector_math_node_def
	];
	nodes_brush_list = [
		nodes_brush_category0
	];
}

function nodes_brush_create_node(node_type: string): ui_node_t {
	for (let i: i32 = 0; i < nodes_brush_list.length; ++i) {
		let c: ui_node_t[] = nodes_brush_list[i];
		for (let i: i32 = 0; i < c.length; ++i) {
			let n: ui_node_t = c[i];
			if (n.type == node_type) {
				let canvas: ui_node_canvas_t = context_raw.brush.canvas;
				let nodes: ui_nodes_t = context_raw.brush.nodes;
				let node: ui_node_t = ui_nodes_make_node(n, nodes, canvas);
				array_push(canvas.nodes, node);
				return node;
			}
		}
	}
	return null;
}
