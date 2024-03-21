/// <reference path='./nodes/image_texture_node.ts'/>
/// <reference path='./nodes/rgb_node.ts'/>
/// <reference path='./nodes/inpaint_node.ts'/>
/// <reference path='./nodes/photo_to_pbr_node.ts'/>
/// <reference path='./nodes/text_to_photo_node.ts'/>
/// <reference path='./nodes/tiling_node.ts'/>
/// <reference path='./nodes/upscale_node.ts'/>
/// <reference path='./nodes/variance_node.ts'/>

let nodes_brush_categories: string[] = [_tr("Input"), _tr("Model")];

let nodes_brush_list: zui_node_t[][] = [
	[ // Input
		image_texture_node.def,
		rgb_node.def,
	],
	[ // Model
		inpaint_node.def,
		photo_to_pbr_node.def,
		text_to_photo_node.def,
		tiling_node.def,
		upscale_node.def,
		variance_node.def,
	]
];

let nodes_brush_creates: map_t<string, any>;

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
}

function nodes_brush_create_node(node_type: string): zui_node_t {
	for (let i: i32 = 0; i < nodes_brush_list.length; ++i) {
		let c = nodes_brush_list[i];
		for (let j: i32 = 0; j < c.length; ++j) {
			let n = c[j];
			if (n.type == node_type) {
				let canvas = project_canvas;
				let nodes = project_nodes;
				let node = ui_nodes_make_node(n, nodes, canvas);
				array_push(canvas.nodes, node);
				return node;
			}
		}
	}
	return null;
}
