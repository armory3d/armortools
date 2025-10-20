
function color_ramp_node_init() {
	array_push(nodes_material_utilities, color_ramp_node_def);
	map_set(parser_material_node_vectors, "VALTORGB", color_ramp_node_vector);
	map_set(ui_nodes_custom_buttons, "nodes_material_color_ramp_button", nodes_material_color_ramp_button);
}

function color_ramp_node_vector(node: ui_node_t, socket: ui_node_socket_t): string {
	let fac: string    = parser_material_parse_value_input(node.inputs[0]);
	let data0: i32     = node.buttons[0].data[0];
	let interp: string = data0 == 0 ? "LINEAR" : "CONSTANT";
	let elems: f32[]   = node.buttons[0].default_value;
	let len: i32       = elems.length / 5;
	if (len == 1) {
		return parser_material_vec3(elems);
	}
	// Write cols array
	let cols_var: string = parser_material_node_name(node) + "_cols";
	parser_material_write(parser_material_kong, "var " + cols_var + ": float3[" + len + "];"); // TODO: Make const
	for (let i: i32 = 0; i < len; ++i) {
		let tmp: f32[] = [];
		array_push(tmp, elems[i * 5]);
		array_push(tmp, elems[i * 5 + 1]);
		array_push(tmp, elems[i * 5 + 2]);
		parser_material_write(parser_material_kong, cols_var + "[" + i + "] = " + parser_material_vec3(tmp) + ";");
	}
	// Get index
	let fac_var: string = parser_material_node_name(node) + "_fac";
	parser_material_write(parser_material_kong, "var " + fac_var + ": float = " + fac + ";");
	let index: string = "0";
	for (let i: i32 = 1; i < len; ++i) {
		let e: f32 = elems[i * 5 + 4];
		index += " + (" + fac_var + " > " + e + " ? 1 : 0)";
	}
	// Write index
	let index_var: string = parser_material_node_name(node) + "_i";
	parser_material_write(parser_material_kong, "var " + index_var + ": int = " + index + ";");
	if (interp == "CONSTANT") {
		return cols_var + "[" + index_var + "]";
	}
	else { // Linear
		// Write facs array
		let facs_var: string = parser_material_node_name(node) + "_facs";
		parser_material_write(parser_material_kong, "var " + facs_var + ": float[" + len + "];"); // TODO: Make const
		for (let i: i32 = 0; i < len; ++i) {
			let e: f32 = elems[i * 5 + 4];
			parser_material_write(parser_material_kong, facs_var + "[" + i + "] = " + e + ";");
		}
		// Mix color
		// float f = (pos - start) * (1.0 / (finish - start))
		// TODO: index_var + 1 out of bounds
		return "lerp3(" + cols_var + "[" + index_var + "], " + cols_var + "[" + index_var + " + 1], (" + fac_var + " - " + facs_var + "[" + index_var +
		       "]) * (1.0 / (" + facs_var + "[" + index_var + " + 1] - " + facs_var + "[" + index_var + "]) ))";
	}
}

function nodes_material_color_ramp_button(node_id: i32) {
	let nodes: ui_nodes_t = ui_nodes_get_nodes();
	let node: ui_node_t   = ui_get_node(ui_nodes_get_canvas(true).nodes, node_id);

	let but: ui_node_button_t = node.buttons[0];
	let nhandle: ui_handle_t  = ui_nest(ui_handle(__ID__), node.id);
	let nx: f32               = ui._x;
	let ny: f32               = ui._y;

	// Preview
	let vals: f32[] = but.default_value; // [r, g, b, a, pos, r, g, b, a, pos, ..]
	let sw: f32     = ui._w / UI_NODES_SCALE();
	for (let i: i32 = 0; i < vals.length / 5; ++i) {
		let pos: f32 = vals[i * 5 + 4];
		let col: i32 = color_from_floats(vals[i * 5 + 0], vals[i * 5 + 1], vals[i * 5 + 2], 1.0);
		ui_fill(pos * sw, 0, (1.0 - pos) * sw, UI_LINE_H() - 2 * UI_NODES_SCALE(), col);
	}
	ui._y += UI_LINE_H();
	// Edit
	let ihandle: ui_handle_t = ui_nest(ui_nest(nhandle, 0), 2);
	let row: f32[]           = [ 1 / 4, 1 / 4, 2 / 4 ];
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
	let interpolate_combo: string[] = [ tr("Linear"), tr("Constant") ];
	but.data[0]                     = ui_combo(h, interpolate_combo, tr("Interpolate"));

	ui_row2();
	let i: i32 = math_floor(ui_slider(ihandle, "Index", 0, (vals.length / 5) - 1, false, 1, true, ui_align_t.LEFT));
	if (i >= (vals.length * 5) || i < 0) {
		ihandle.f = i = (vals.length / 5) - 1; // Stay in bounds
	}

	ui_nest(ui_nest(nhandle, 0), 3).f = vals[i * 5 + 4];
	vals[i * 5 + 4]                   = ui_slider(ui_nest(ui_nest(nhandle, 0), 3), "Pos", 0, 1, true, 100, true, ui_align_t.LEFT);
	if (vals[i * 5 + 4] > 1.0) {
		vals[i * 5 + 4] = 1.0; // Stay in bounds
	}
	else if (vals[i * 5 + 4] < 0.0) {
		vals[i * 5 + 4] = 0.0;
	}

	let chandle: ui_handle_t = ui_nest(ui_nest(nhandle, 0), 4);
	chandle.color            = color_from_floats(vals[i * 5 + 0], vals[i * 5 + 1], vals[i * 5 + 2], 1.0);
	if (ui_text("", ui_align_t.RIGHT, chandle.color) == ui_state_t.STARTED) {
		let rx: f32          = nx + ui._w - ui_p(37);
		let ry: f32          = ny - ui_p(5);
		nodes._input_started = ui.input_started = false;
		ui_nodes_rgba_popup(chandle, vals.buffer + i * 5, math_floor(rx), math_floor(ry + UI_ELEMENT_H()));
	}
	vals[i * 5 + 0] = color_get_rb(chandle.color) / 255;
	vals[i * 5 + 1] = color_get_gb(chandle.color) / 255;
	vals[i * 5 + 2] = color_get_bb(chandle.color) / 255;
}

let color_ramp_node_def: ui_node_t = {
	id : 0,
	name : _tr("Color Ramp"),
	type : "VALTORGB",
	x : 0,
	y : 0,
	color : 0xff62676d,
	inputs : [ {
		id : 0,
		node_id : 0,
		name : _tr("Factor"),
		type : "VALUE",
		color : 0xffa1a1a1,
		default_value : f32_array_create_x(0.5),
		min : 0.0,
		max : 1.0,
		precision : 100,
		display : 0
	} ],
	outputs : [
		{
			id : 0,
			node_id : 0,
			name : _tr("Color"),
			type : "RGBA",
			color : 0xffc7c729,
			default_value : f32_array_create_xyzw(0.0, 0.0, 0.0, 1.0),
			min : 0.0,
			max : 1.0,
			precision : 100,
			display : 0
		},
		{
			id : 0,
			node_id : 0,
			name : _tr("Alpha"),
			type : "VALUE",
			color : 0xffa1a1a1,
			default_value : f32_array_create_x(0.0),
			min : 0.0,
			max : 1.0,
			precision : 100,
			display : 0
		}
	],
	buttons : [ {
		name : "nodes_material_color_ramp_button",
		type : "CUSTOM",
		output : 0,
		default_value : f32_array_create_xyzwv(1.0, 1.0, 1.0, 1.0, 0.0),
		data : u8_array_create(1),
		min : 0.0,
		max : 1.0,
		precision : 100,
		height : 4.5
	} ],
	width : 0,
	flags : 0
};
