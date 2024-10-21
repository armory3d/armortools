
let nodes_brush_categories: string[] = [
	_tr("Input"),
	_tr("Model")
];

let nodes_brush_input: ui_node_t[];
let nodes_brush_model: ui_node_t[];
let nodes_brush_list: node_list_t[];

let nodes_brush_creates: map_t<string, (args: f32_array_t)=>logic_node_ext_t>;

function nodes_brush_init() {
	nodes_brush_creates = map_create();
	map_set(nodes_brush_creates, "brush_output_node", brush_output_node_create);
	map_set(nodes_brush_creates, "image_texture_node", image_texture_node_create);
	map_set(nodes_brush_creates, "rgb_node", rgb_node_create);
	map_set(nodes_brush_creates, "inpaint_node", inpaint_node_create);
	map_set(nodes_brush_creates, "photo_to_pbr_node", photo_to_pbr_node_create);
	map_set(nodes_brush_creates, "text_to_photo_node", text_to_photo_node_create);
	map_set(nodes_brush_creates, "tiling_node", tiling_node_create);
	map_set(nodes_brush_creates, "upscale_node", upscale_node_create);
	map_set(nodes_brush_creates, "variance_node", variance_node_create);

	map_set(nodes_brush_creates, "float_node", float_node_create);
	map_set(nodes_brush_creates, "vector_node", vector_node_create);
	map_set(nodes_brush_creates, "color_node", color_node_create);

	nodes_brush_input = [
		image_texture_node_def,
		rgb_node_def,
	];

	nodes_brush_model = [
		inpaint_node_def,
		photo_to_pbr_node_def,
		text_to_photo_node_def,
		tiling_node_def,
		upscale_node_def,
		variance_node_def,
	];

	nodes_brush_list = [
		nodes_brush_input,
		nodes_brush_model
	];

	map_set(ui_nodes_custom_buttons, "inpaint_node_button", inpaint_node_button);
	map_set(ui_nodes_custom_buttons, "text_to_photo_node_button", text_to_photo_node_button);
	map_set(ui_nodes_custom_buttons, "tiling_node_button", tiling_node_button);
	map_set(ui_nodes_custom_buttons, "variance_node_button", variance_node_button);
}

function nodes_brush_create_node(node_type: string): ui_node_t {
	for (let i: i32 = 0; i < nodes_brush_list.length; ++i) {
		let c: ui_node_t[] = nodes_brush_list[i];
		for (let j: i32 = 0; j < c.length; ++j) {
			let n: ui_node_t = c[j];
			if (n.type == node_type) {
				let canvas: ui_node_canvas_t = project_canvas;
				let nodes: ui_nodes_t = project_nodes;
				let node: ui_node_t = ui_nodes_make_node(n, nodes, canvas);
				array_push(canvas.nodes, node);
				return node;
			}
		}
	}
	return null;
}
