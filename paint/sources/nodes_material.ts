
let nodes_material_categories: string[] = [ _tr("Input"), _tr("Texture"), _tr("Color"), _tr("Utilities"), _tr("Neural"), _tr("Group") ];

let nodes_material_input: ui_node_t[];
let nodes_material_texture: ui_node_t[];
let nodes_material_color: ui_node_t[];
let nodes_material_utilities: ui_node_t[];
let nodes_material_neural: ui_node_t[];
let nodes_material_group: ui_node_t[];

type node_list_t                        = ui_node_t[];
let  nodes_material_list: node_list_t[] = null;

function nodes_material_init() {
	if (nodes_material_list != null) {
		return;
	}

	nodes_material_input = [];
	attribute_node_init();
	camera_data_node_init();
	rgb_node_init(); // color_node_init
	fresnel_node_init();
	geometry_node_init();
	layer_node_init();
	layer_mask_node_init();
	layer_weight_node_init();
	material_node_init();
	object_info_node_init();
	picker_node_init();
	script_node_init();
	shader_node_init();
	tangent_node_init();
	texture_coordinate_node_init();
	uv_map_node_init();
	value_node_init();
	wireframe_node_init();

	nodes_material_texture = [];
	brick_texture_node_init();
	camera_texture_node_init();
	checker_texture_node_init();
	curvature_bake_node_init();
	gradient_texture_node_init();
	image_texture_node_init();
	text_texture_node_init();
	magic_texture_node_init();
	noise_texture_node_init();
	voronoi_texture_node_init();
	wave_texture_node_init();
	gabor_texture_node_init();

	nodes_material_color = [];
	blur_node_init();
	brightness_contrast_node_init();
	gamma_node_init();
	hue_saturation_value_node_init();
	invert_color_node_init();
	mix_color_node_init();
	quantize_node_init();
	replace_color_node_init();
	warp_node_init();

	nodes_material_utilities = [];
	bump_node_init();
	mapping_node_init();
	mix_normal_map_node_init();
	normal_node_init();
	normal_map_node_init();
	vector_curves_node_init();
	clamp_node_init();
	color_ramp_node_init();
	color_mask_node_init();
	combine_xyz_node_init();
	map_range_node_init();
	math2_node_init();
	rgb_to_bw_node_init();
	separate_xyz_node_init();
	vector_math2_node_init();

	nodes_material_neural = [];
	if (config_raw.experimental) {
		edit_image_node_init();
		image_to_depth_node_init();
		image_to_normal_map_node_init();
		image_to_pbr_node_init();
		inpaint_image_node_init();
		text_to_image_node_init();
		tile_image_node_init();
		upscale_image_node_init();
	}

	nodes_material_group = [];
	group_node_init();

	nodes_material_list =
	    [ nodes_material_input, nodes_material_texture, nodes_material_color, nodes_material_utilities, nodes_material_neural, nodes_material_group ];
}

function nodes_material_get_node_t(node_type: string): ui_node_t {
	for (let i: i32 = 0; i < nodes_material_list.length; ++i) {
		let c: ui_node_t[] = nodes_material_list[i];
		for (let i: i32 = 0; i < c.length; ++i) {
			let n: ui_node_t = c[i];
			if (n.type == node_type) {
				return n;
			}
		}
	}
	return null;
}

function nodes_material_create_node(node_type: string, group: node_group_t = null): ui_node_t {
	let n: ui_node_t = nodes_material_get_node_t(node_type);
	if (n == null) {
		return null;
	}
	let canvas: ui_node_canvas_t = group != null ? group.canvas : ui_nodes_get_canvas();
	let nodes: ui_nodes_t        = group != null ? group.nodes : context_raw.material.nodes;
	let node: ui_node_t          = ui_nodes_make_node(n, nodes, canvas);
	array_push(canvas.nodes, node);
	return node;
}
