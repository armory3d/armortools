
let upscale_result: gpu_texture_t = null;

function upscale_node_init() {
	array_push(nodes_material_neural, upscale_node_def);
	map_set(parser_material_node_vectors, "NEURAL_UPSCALE", upscale_node_vector);
	map_set(ui_nodes_custom_buttons, "upscale_node_button", upscale_node_button);
}

function upscale_node_vector(node: ui_node_t, socket: ui_node_socket_t): string {
	if (upscale_result == null) {
		return("float3(0.0, 0.0, 0.0)");
	}
	let tex_name: string = parser_material_node_name(node);
	map_set(data_cached_images, tex_name, upscale_result);
	let tex: bind_tex_t = parser_material_make_bind_tex(tex_name, tex_name);
	let texstore: string = parser_material_texture_store(node, tex, tex_name, color_space_t.AUTO);
	return texstore + ".rgb";
}

function upscale_node_button(node_id: i32) {
	let node: ui_node_t = ui_get_node(ui_nodes_get_canvas(true).nodes, node_id);
}

let upscale_node_def: ui_node_t = {
	id: 0,
	name: _tr("Upscale"),
	type: "upscale_node",
	x: 0,
	y: 0,
	color: 0xff4982a0,
	inputs: [
		{
			id: 0,
			node_id: 0,
			name: _tr("Vector"),
			type: "VECTOR",
			color: 0xff6363c7,
			default_value: f32_array_create_xyz(0.0, 0.0, 0.0),
			min: 0.0,
			max: 1.0,
			precision: 100,
			display: 0
		},
		{
			id: 0,
			node_id: 0,
			name: _tr("Color"),
			type: "RGBA",
			color: 0xffc7c729,
			default_value: f32_array_create_xyzw(0.0, 0.0, 0.0, 1.0),
			min: 0.0,
			max: 1.0,
			precision: 100,
			display: 0
		}
	],
	outputs: [
		{
			id: 0,
			node_id: 0,
			name: _tr("Color"),
			type: "RGBA",
			color: 0xffc7c729,
			default_value: f32_array_create_xyzw(0.0, 0.0, 0.0, 1.0),
			min: 0.0,
			max: 1.0,
			precision: 100,
			display: 0
		}
	],
	buttons: [],
	width: 0,
	flags: 0
};
