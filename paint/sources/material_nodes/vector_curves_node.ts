
function vector_curves_node_init() {
	array_push(nodes_material_utilities, vector_curves_node_def);
	map_set(parser_material_node_vectors, "CURVE_VEC", vector_curves_node_vector);
	map_set(ui_nodes_custom_buttons, "nodes_material_vector_curves_button", nodes_material_vector_curves_button);
}

function parser_material_vector_curve(name: string, fac: string, points: f32_ptr, num: i32): string {
	// Write Ys array
	let ys_var: string = name + "_ys";
	parser_material_write(parser_material_kong, "var " + ys_var + ": float[" + num + "];"); // TODO: Make const
	for (let i: i32 = 0; i < num; ++i) {
		let p: f32 = ARRAY_ACCESS(points, i * 2 + 1);
		parser_material_write(parser_material_kong, ys_var + "[" + i + "] = " + p + ";");
	}
	// Get index
	let fac_var: string = name + "_fac";
	parser_material_write(parser_material_kong, "var " + fac_var + ": float = " + fac + ";");
	let index: string = "0";
	for (let i: i32 = 1; i < num; ++i) {
		let p: f32 = ARRAY_ACCESS(points, i * 2 + 0);
		index += " + (" + fac_var + " > " + p + " ? 1 : 0)";
	}
	// Write index
	let index_var: string = name + "_i";
	parser_material_write(parser_material_kong, "var " + index_var + ": int = " + index + ";");
	// Linear
	// Write Xs array
	let facs_var: string = name + "_xs";
	parser_material_write(parser_material_kong, "var " + facs_var + ": float[" + num + "];"); // TODO: Make const
	for (let i: i32 = 0; i < num; ++i) {
		let p: f32 = ARRAY_ACCESS(points, i * 2 + 0);
		parser_material_write(parser_material_kong, "" + facs_var + "[" + i + "] = " + p + ";");
	}
	// Map vector
	return "0.0"; ////
	              // return "lerp(" +
	              // 	ys_var + "[" + index_var + "], " + ys_var + "[" + index_var + " + 1], (" + fac_var + " - " +
	              // 	facs_var + "[" + index_var + "]) * (1.0 / (" + facs_var + "[" + index_var + " + 1] - " + facs_var + "[" + index_var + "])))";
}

function vector_curves_node_vector(node: ui_node_t, socket: ui_node_socket_t): string {
	let fac: string         = parser_material_parse_value_input(node.inputs[0]);
	let vec: string         = parser_material_parse_vector_input(node.inputs[1]);
	let curves: f32_array_t = node.buttons[0].default_value;
	if (curves[96] == 0.0) {
		curves[96] = 1.0;
		curves[97] = 1.0;
		curves[98] = 1.0;
	}
	let name: string = parser_material_node_name(node);
	let vc0: string  = parser_material_vector_curve(name + "0", vec + ".x", curves.buffer + 32 * 0, curves[96]);
	let vc1: string  = parser_material_vector_curve(name + "1", vec + ".y", curves.buffer + 32 * 1, curves[97]);
	let vc2: string  = parser_material_vector_curve(name + "2", vec + ".z", curves.buffer + 32 * 2, curves[98]);
	// mapping.curves[0].points[0].handle_type // bezier curve
	return "(float3(" + vc0 + ", " + vc1 + ", " + vc2 + ") * " + fac + ")";
}

function nodes_material_vector_curves_button(node_id: i32) {
	let node: ui_node_t = ui_get_node(ui_nodes_get_canvas(true).nodes, node_id);

	let but: ui_node_button_t = node.buttons[0];
	let nhandle: ui_handle_t  = ui_nest(ui_handle(__ID__), node.id);
	ui_row3();
	ui_radio(ui_nest(ui_nest(nhandle, 0), 1), 0, "X");
	ui_radio(ui_nest(ui_nest(nhandle, 0), 1), 1, "Y");
	ui_radio(ui_nest(ui_nest(nhandle, 0), 1), 2, "Z");

	// Preview
	let axis: i32  = ui_nest(ui_nest(nhandle, 0), 1).i;
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
	let row: f32[] = [ 1 / 5, 1 / 5, 3 / 5 ];
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

let vector_curves_node_def: ui_node_t = {
	id : 0,
	name : _tr("Vector Curves"),
	type : "CURVE_VEC",
	x : 0,
	y : 0,
	color : 0xff522c99,
	inputs : [
		{
			id : 0,
			node_id : 0,
			name : _tr("Factor"),
			type : "VALUE",
			color : 0xffa1a1a1,
			default_value : f32_array_create_x(1.0),
			min : 0.0,
			max : 1.0,
			precision : 100,
			display : 0
		},
		{
			id : 0,
			node_id : 0,
			name : _tr("Vector"),
			type : "VECTOR",
			color : 0xff6363c7,
			default_value : f32_array_create_xyz(0.0, 0.0, 0.0),
			min : 0.0,
			max : 1.0,
			precision : 100,
			display : 0
		}
	],
	outputs : [ {
		id : 0,
		node_id : 0,
		name : _tr("Vector"),
		type : "VECTOR",
		color : 0xff6363c7,
		default_value : f32_array_create_xyz(0.0, 0.0, 0.0),
		min : 0.0,
		max : 1.0,
		precision : 100,
		display : 0
	} ],
	buttons : [ {
		name : "nodes_material_vector_curves_button",
		type : "CUSTOM",
		output : 0,
		default_value : f32_array_create(96 + 3), // x - [0, 32], y - [33, 64], z - [65, 96], x_len, y_len, z_len
		data : null,
		min : 0.0,
		max : 1.0,
		precision : 100,
		height : 8.5
	} ],
	width : 0,
	flags : 0
};
