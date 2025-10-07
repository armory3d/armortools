
let nodes_material_categories: string[] = [
	_tr("Input"),
	_tr("Texture"),
	_tr("Color"),
	_tr("Vector"),
	_tr("Converter"),
	// _tr("Neural"),
	_tr("Group")
];

let nodes_material_input: ui_node_t[];
// let nodes_material_output: ui_node_t[];
let nodes_material_texture: ui_node_t[];
let nodes_material_color: ui_node_t[];
let nodes_material_vector: ui_node_t[];
let nodes_material_converter: ui_node_t[];
// let nodes_material_neural: ui_node_t[];
let nodes_material_group: ui_node_t[];

type node_list_t = ui_node_t[];
let nodes_material_list: node_list_t[];
let _nodes_material_nodes: ui_nodes_t;
let _nodes_material_node: ui_node_t;
let _nodes_material_sockets: ui_node_socket_t[];

function nodes_material_init() {
	map_set(ui_nodes_custom_buttons, "nodes_material_vector_curves_button", nodes_material_vector_curves_button);
	map_set(ui_nodes_custom_buttons, "nodes_material_color_ramp_button", nodes_material_color_ramp_button);
	map_set(ui_nodes_custom_buttons, "nodes_material_new_group_button", nodes_material_new_group_button);
	map_set(ui_nodes_custom_buttons, "nodes_material_group_input_button", nodes_material_group_input_button);
	map_set(ui_nodes_custom_buttons, "nodes_material_group_output_button", nodes_material_group_output_button);
	nodes_material_list_init();
}

function nodes_material_list_init() {
	if (nodes_material_list != null) {
		return;
	}

	nodes_material_input = [];
	attribute_node_init();
	camera_data_node_init();
	fresnel_node_init();
	geometry_node_init();
	layer_node_init();
	layer_mask_node_init();
	layer_weight_node_init();
	material_node_init();
	object_info_node_init();
	picker_node_init();
	rgb_node_init();
	script_node_init();
	shader_node_init();
	tangent_node_init();
	texture_coordinate_node_init();
	uv_map_node_init();
	value_node_init();
	vertex_color_node_init();
	wireframe_node_init();

	// nodes_material_output = [];
	// material_output_node_init();

	nodes_material_texture = [];
	brick_texture_node_init();
	checker_texture_node_init();
	curvature_bake_node_init();
	gradient_texture_node_init();
	image_texture_node_init();
	text_texture_node_init();
	magic_texture_node_init();
	musgrave_texture_node_init();
	noise_texture_node_init();
	voronoi_texture_node_init();
	wave_texture_node_init();

	nodes_material_color = [];
	blur_node_init();
	brightness_contrast_node_init();
	gamma_node_init();
	hue_saturation_value_node_init();
	invert_node_init();
	mix_color_node_init();
	quantize_node_init();
	replace_color_node_init();
	warp_node_init();

	nodes_material_vector = [];
	bump_node_init();
	mapping_node_init();
	mix_normal_map_node_init();
	normal_node_init();
	normal_map_node_init();
	vector_curves_node_init();

	nodes_material_converter = [];
	clamp_node_init();
	color_ramp_node_init();
	color_mask_node_init();
	combine_hsv_node_init();
	combine_rgb_node_init();
	combine_xyz_node_init();
	map_range_node_init();
	math2_node_init();
	rgb_to_bw_node_init();
	separate_hsv_node_init();
	separate_rgb_node_init();
	separate_xyz_node_init();
	vector_math2_node_init();

	// nodes_material_neural = [];
	// text_to_photo_node_init();
	// inpaint_node_init();
	// photo_to_pbr_node_init();
	// tiling_node_init();
	// upscale_node_init();
	// variance_node_init();

	nodes_material_group = [];
	group_node_init();

	nodes_material_list = [
		nodes_material_input,
		nodes_material_texture,
		nodes_material_color,
		nodes_material_vector,
		nodes_material_converter,
		// nodes_material_neural,
		nodes_material_group
	];
}

function nodes_material_vector_curves_button(node_id: i32) {
	let node: ui_node_t = ui_get_node(ui_nodes_get_canvas(true).nodes, node_id);

	let but: ui_node_button_t = node.buttons[0];
	let nhandle: ui_handle_t = ui_nest(ui_handle(__ID__), node.id);
	ui_row3();
	ui_radio(ui_nest(ui_nest(nhandle, 0), 1), 0, "X");
	ui_radio(ui_nest(ui_nest(nhandle, 0), 1), 1, "Y");
	ui_radio(ui_nest(ui_nest(nhandle, 0), 1), 2, "Z");

	// Preview
	let axis: i32 = ui_nest(ui_nest(nhandle, 0), 1).i;
	let val: f32[] = but.default_value;
	ui._y += UI_LINE_H() * 5;

	let num: i32 = val[96 + axis];

	if (num == 0) {
		// Init
		val[96 + 0] = 1;
		val[96 + 1] = 1;
		val[96 + 2] = 1;
	}

	// Edit
	let row: f32[] = [1 / 5, 1 / 5, 3 / 5];
	ui_row(row);
	if (ui_button("+")) {
		// TODO:
		// val[axis * 32 + num * 2 + 0] = 0.0;
		// val[axis * 32 + num * 2 + 1] = 0.0;
		// num++;
		// val[96 + axis] = num;
	}
	if (ui_button("-")) {
		if (num > 1) {
			num--;
			val[96 + axis] = num;
		}
	}

	let ihandle: ui_handle_t = ui_nest(ui_nest(ui_nest(nhandle, 0), 2), axis);
	if (ihandle.init) {
		ihandle.i = 0;
	}

	let i: i32 = math_floor(ui_slider(ihandle, "Index", 0, num - 1, false, 1, true, ui_align_t.LEFT));
	if (i >= num || i < 0) {
		ihandle.f = i = num - 1; // Stay in bounds
	}

	ui_row2();
	ui_nest(ui_nest(nhandle, 0), 3).f = val[axis * 32 + i * 2 + 0];
	ui_nest(ui_nest(nhandle, 0), 4).f = val[axis * 32 + i * 2 + 1];

	let h1: ui_handle_t = ui_nest(ui_nest(nhandle, 0), 3);
	if (h1.init) {
		h1.f = 0.0;
	}
	let h2: ui_handle_t = ui_nest(ui_nest(nhandle, 0), 4);
	if (h2.init) {
		h2.f = 0.0;
	}
	val[axis * 32 + i * 2 + 0] = ui_slider(h1, "X", -1, 1, true, 100, true, ui_align_t.LEFT);
	val[axis * 32 + i * 2 + 1] = ui_slider(h2, "Y", -1, 1, true, 100, true, ui_align_t.LEFT);
}

function nodes_material_color_ramp_button(node_id: i32) {
	let nodes: ui_nodes_t = ui_nodes_get_nodes();
	let node: ui_node_t = ui_get_node(ui_nodes_get_canvas(true).nodes, node_id);

	let but: ui_node_button_t = node.buttons[0];
	let nhandle: ui_handle_t = ui_nest(ui_handle(__ID__), node.id);
	let nx: f32 = ui._x;
	let ny: f32 = ui._y;

	// Preview
	let vals: f32[] = but.default_value; // [r, g, b, a, pos, r, g, b, a, pos, ..]
	let sw: f32 = ui._w / UI_NODES_SCALE();
	for (let i: i32 = 0; i < vals.length / 5; ++i) {
		let pos: f32 = vals[i * 5 + 4];
		let col: i32 = color_from_floats(vals[i * 5 + 0], vals[i * 5 + 1], vals[i * 5 + 2], 1.0);
		ui_fill(pos * sw, 0, (1.0 - pos) * sw, UI_LINE_H() - 2 * UI_NODES_SCALE(), col);
	}
	ui._y += UI_LINE_H();
	// Edit
	let ihandle: ui_handle_t = ui_nest(ui_nest(nhandle, 0), 2);
	let row: f32[] = [1 / 4, 1 / 4, 2 / 4];
	ui_row(row);
	if (ui_button("+")) {
		// TODO:
		// array_push(vals, vals[vals.length - 5]); // r
		// array_push(vals, vals[vals.length - 5]); // g
		// array_push(vals, vals[vals.length - 5]); // b
		// array_push(vals, vals[vals.length - 5]); // a
		// array_push(vals, 1.0); // pos
		// ihandle.f += 1;
	}
	if (ui_button("-") && vals.length > 5) {
		array_pop(vals);
		array_pop(vals);
		array_pop(vals);
		array_pop(vals);
		array_pop(vals);
		ihandle.f -= 1;
	}

	let h: ui_handle_t = ui_nest(ui_nest(nhandle, 0), 1);
	if (h.init) {
		h.i = but.data[0];
	}
	let interpolate_combo: string[] = [tr("Linear"), tr("Constant")];
	but.data[0] = ui_combo(h, interpolate_combo, tr("Interpolate"));

	ui_row2();
	let i: i32 = math_floor(ui_slider(ihandle, "Index", 0, (vals.length / 5) - 1, false, 1, true, ui_align_t.LEFT));
	if (i >= (vals.length * 5) || i < 0) {
		ihandle.f = i = (vals.length / 5) - 1; // Stay in bounds
	}

	ui_nest(ui_nest(nhandle, 0), 3).f = vals[i * 5 + 4];
	vals[i * 5 + 4] = ui_slider(ui_nest(ui_nest(nhandle, 0), 3), "Pos", 0, 1, true, 100, true, ui_align_t.LEFT);
	if (vals[i * 5 + 4] > 1.0) {
		vals[i * 5 + 4] = 1.0; // Stay in bounds
	}
	else if (vals[i * 5 + 4] < 0.0) {
		vals[i * 5 + 4] = 0.0;
	}

	let chandle: ui_handle_t = ui_nest(ui_nest(nhandle, 0), 4);
	chandle.color = color_from_floats(vals[i * 5 + 0], vals[i * 5 + 1], vals[i * 5 + 2], 1.0);
	if (ui_text("", ui_align_t.RIGHT, chandle.color) == ui_state_t.STARTED) {
		let rx: f32 = nx + ui._w - ui_p(37);
		let ry: f32 = ny - ui_p(5);
		nodes._input_started = ui.input_started = false;
		ui_nodes_rgba_popup(chandle, vals.buffer + i * 5, math_floor(rx), math_floor(ry + UI_ELEMENT_H()));
	}
	vals[i * 5 + 0] = color_get_rb(chandle.color) / 255;
	vals[i * 5 + 1] = color_get_gb(chandle.color) / 255;
	vals[i * 5 + 2] = color_get_bb(chandle.color) / 255;
}

function nodes_material_new_group_button(node_id: i32) {
	let node: ui_node_t = ui_get_node(ui_nodes_get_canvas(true).nodes, node_id);

	if (node.name == "New Group") {
		for (let i: i32 = 1; i < 999; ++i) {
			node.name = tr("Group") + " " + i;

			let found: bool = false;
			for (let i: i32 = 0; i < project_material_groups.length; ++i) {
				let g: node_group_t = project_material_groups[i];
				let cname: string = g.canvas.name;
				if (cname == node.name) {
					found = true;
					break;
				}
			}
			if (!found) {
				break;
			}
		}

		let canvas: ui_node_canvas_t = {
			name: node.name,
			nodes: [
				{
					id: 0,
					name: _tr("Group Input"),
					type: "GROUP_INPUT",
					x: 50,
					y: 200,
					color: 0xff448c6d,
					inputs: [],
					outputs: [],
					buttons: [
						{
							name: "nodes_material_group_input_button",
							type: "CUSTOM",
							height: 1
						}
					],
					width: 0,
					flags: 0
				},
				{
					id: 1,
					name: _tr("Group Output"),
					type: "GROUP_OUTPUT",
					x: 450,
					y: 200,
					color: 0xff448c6d,
					inputs: [],
					outputs: [],
					buttons: [
						{
							name: "nodes_material_group_output_button",
							type: "CUSTOM",
							height: 1
						}
					],
					width: 0,
					flags: 0
				}
			],
			links: []
		};
		let ng: node_group_t = {
			canvas: canvas,
			nodes: ui_nodes_create()
		};
		array_push(project_material_groups, ng);
	}

	let group: node_group_t = null;
	for (let i: i32 = 0; i < project_material_groups.length; ++i) {
		let g: node_group_t = project_material_groups[i];
		let cname: string = g.canvas.name;
		if (cname == node.name) {
			group = g;
			break;
		}
	}

	if (ui_button(tr("Nodes"))) {
		array_push(ui_nodes_group_stack, group);
	}
}

function nodes_material_group_input_button(node_id: i32) {
	let nodes: ui_nodes_t = ui_nodes_get_nodes();
	let node: ui_node_t = ui_get_node(ui_nodes_get_canvas(true).nodes, node_id);
	nodes_material_add_socket_button(nodes, node, node.outputs);
}

function nodes_material_group_output_button(node_id: i32) {
	let nodes: ui_nodes_t = ui_nodes_get_nodes();
	let node: ui_node_t = ui_get_node(ui_nodes_get_canvas(true).nodes, node_id);
	nodes_material_add_socket_button(nodes, node, node.inputs);
}

function nodes_material_add_socket_button(nodes: ui_nodes_t, node: ui_node_t, sockets: ui_node_socket_t[]) {
	if (ui_button(tr("Add"))) {
		_nodes_material_nodes = nodes;
		_nodes_material_node = node;
		_nodes_material_sockets = sockets;
		ui_menu_draw(function () {
			let nodes: ui_nodes_t = _nodes_material_nodes;
			let node: ui_node_t = _nodes_material_node;
			let sockets: ui_node_socket_t[] = _nodes_material_sockets;

			let group_stack: node_group_t[] = ui_nodes_group_stack;
			let c: ui_node_canvas_t = group_stack[group_stack.length - 1].canvas;
			if (ui_menu_button(tr("RGBA"))) {
				array_push(sockets, nodes_material_create_socket(nodes, node, null, "RGBA", c));
				nodes_material_sync_sockets(node);
			}
			if (ui_menu_button(tr("Vector"))) {
				array_push(sockets, nodes_material_create_socket(nodes, node, null, "VECTOR", c));
				nodes_material_sync_sockets(node);
			}
			if (ui_menu_button(tr("Value"))) {
				array_push(sockets, nodes_material_create_socket(nodes, node, null, "VALUE", c));
				nodes_material_sync_sockets(node);
			}
		});
	}
}

function nodes_material_sync_sockets(node: ui_node_t) {
	let group_stack: node_group_t[] = ui_nodes_group_stack;
	let c: ui_node_canvas_t = group_stack[group_stack.length - 1].canvas;
	for (let i: i32 = 0; i < project_materials.length; ++i) {
		let m: slot_material_t = project_materials[i];
		nodes_material_sync_group_sockets(m.canvas, c.name, node);
	}
	for (let i: i32 = 0; i < project_material_groups.length; ++i) {
		let g: node_group_t = project_material_groups[i];
		nodes_material_sync_group_sockets(g.canvas, c.name, node);
	}
}

function nodes_material_sync_group_sockets(canvas: ui_node_canvas_t, group_name: string, node: ui_node_t) {
	for (let i: i32 = 0; i < canvas.nodes.length; ++i) {
		let n: ui_node_t = canvas.nodes[i];
		if (n.type == "GROUP" && n.name == group_name) {
			let is_inputs: bool = node.name == "Group Input";
			let old_sockets: ui_node_socket_t[] = is_inputs ? n.inputs : n.outputs;
			let sockets: ui_node_socket_t[] = util_clone_canvas_sockets(is_inputs ? node.outputs : node.inputs);
			if (is_inputs) {
				n.inputs = sockets;
			}
			else {
				n.outputs = sockets;
			}
			for (let i: i32 = 0; i < sockets.length; ++i) {
				let s: ui_node_socket_t = sockets[i];
				s.node_id = n.id;
			}
			let num_sockets: i32 = sockets.length < old_sockets.length ? sockets.length : old_sockets.length;
			for (let i: i32 = 0; i < num_sockets; ++i) {
				if (sockets[i].type == old_sockets[i].type) {
					sockets[i].default_value = old_sockets[i].default_value;
				}
			}
		}
	}
}

function nodes_material_get_socket_color(type: string): i32 {
	return type == "RGBA" ? 0xffc7c729 : type == "VECTOR" ? 0xff6363c7 : 0xffa1a1a1;
}

function nodes_material_get_socket_default_value(type: string): f32_array_t {
	return type == "RGBA" ? f32_array_create_xyzw(0.8, 0.8, 0.8, 1.0) : type == "VECTOR" ? f32_array_create_xyz(0.0, 0.0, 0.0) : f32_array_create_x(0.0);
}

function nodes_material_get_socket_name(type: string): string {
	return type == "RGBA" ? _tr("Color") : type == "VECTOR" ? _tr("Vector") : _tr("Value");
}

function nodes_material_create_socket(nodes: ui_nodes_t, node: ui_node_t, name: string, type: string, canvas: ui_node_canvas_t, min: f32 = 0.0, max: f32 = 1.0, default_value: any = null): ui_node_socket_t {
	let soc: ui_node_socket_t = {
		id: ui_get_socket_id(canvas.nodes),
		node_id: node.id,
		name: name == null ? nodes_material_get_socket_name(type) : name,
		type: type,
		color: nodes_material_get_socket_color(type),
		default_value: default_value == null ? nodes_material_get_socket_default_value(type) : default_value,
		min: min,
		max: max,
		precision: 100
	};
	return soc;
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
	let nodes: ui_nodes_t = group != null ? group.nodes : context_raw.material.nodes;
	let node: ui_node_t = ui_nodes_make_node(n, nodes, canvas);
	array_push(canvas.nodes, node);
	return node;
}
