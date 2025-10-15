
function _import_arm_get_legacy_node_socket_array(old: map_t<string, any>, key: string): ui_node_socket_t[] {
	let sockets: ui_node_socket_t[] = [];
	let ias: any[]                  = map_get(old, key);
	for (let i: i32 = 0; i < ias.length; ++i) {
		let old: map_t<string, any> = ias[i];
		let s: ui_node_socket_t     = {};
		s.id                        = armpack_map_get_i32(old, "id");
		s.node_id                   = armpack_map_get_i32(old, "node_id");
		s.name                      = map_get(old, "name");
		s.type                      = map_get(old, "type");
		s.color                     = armpack_map_get_i32(old, "color");
		if (s.type == "VALUE") {
			let x: f32      = armpack_map_get_f32(old, "default_value");
			s.default_value = f32_array_create_x(x);
		}
		else { // VECTOR, RGBA
			let dv: map_t<string, any> = map_get(old, "default_value");
			let x: f32                 = armpack_map_get_f32(dv, "0");
			let y: f32                 = armpack_map_get_f32(dv, "1");
			let z: f32                 = armpack_map_get_f32(dv, "2");
			if (s.type == "VECTOR") {
				s.default_value = f32_array_create_xyz(x, y, z);
			}
			else { // RGBA
				let w: f32      = armpack_map_get_f32(dv, "3");
				s.default_value = f32_array_create_xyzw(x, y, z, w);
			}
		}
		s.min = armpack_map_get_f32(old, "min");
		s.max = armpack_map_get_f32(old, "max");
		if (s.max == 0.0) {
			s.max = 1.0;
		}
		s.precision = armpack_map_get_f32(old, "precision");
		if (s.precision == 0.0) {
			s.precision = 100.0;
		}
		s.display = armpack_map_get_i32(old, "display");
		array_push(sockets, s);
	}
	return sockets;
}

function _import_arm_get_legacy_node_canvas_array(map: map_t<string, any>, key: string): ui_node_canvas_t[] {
	let cas: any[] = map_get(map, key);
	if (cas == null) {
		return null;
	}
	let ar: ui_node_canvas_t[] = [];
	for (let i: i32 = 0; i < cas.length; ++i) {
		let old: map_t<string, any> = cas[i];
		let c: ui_node_canvas_t     = {};
		c.name                      = map_get(old, "name");

		c.nodes        = [];
		let nas: any[] = map_get(old, "nodes");
		for (let i: i32 = 0; i < nas.length; ++i) {
			let old: map_t<string, any> = nas[i];
			let n: ui_node_t            = {};

			n.id      = armpack_map_get_i32(old, "id");
			n.name    = map_get(old, "name");
			n.type    = map_get(old, "type");
			n.x       = armpack_map_get_f32(old, "x");
			n.y       = armpack_map_get_f32(old, "y");
			n.color   = armpack_map_get_i32(old, "color");
			n.inputs  = _import_arm_get_legacy_node_socket_array(old, "inputs");
			n.outputs = _import_arm_get_legacy_node_socket_array(old, "outputs");

			n.buttons      = [];
			let bas: any[] = map_get(old, "buttons");
			for (let i: i32 = 0; i < bas.length; ++i) {
				let old: map_t<string, any> = bas[i];
				let b: ui_node_button_t     = {};
				b.name                      = map_get(old, "name");
				b.type                      = map_get(old, "type");
				b.output                    = armpack_map_get_i32(old, "output");

				if (b.type == "ENUM") {
					let x: f32      = armpack_map_get_i32(old, "default_value");
					b.default_value = f32_array_create_x(x);

					if (b.name == "File") {
						let data_string: string = map_get(old, "data");
						b.data                  = sys_string_to_buffer(data_string);
					}
					else {
						let data_strings: string[] = map_get(old, "data");
						let joined: string         = string_array_join(data_strings, "\n");
						b.data                     = sys_string_to_buffer(joined);
					}
				}
				else if (b.type == "BOOL") {
					let x: f32      = armpack_map_get_i32(old, "default_value");
					b.default_value = f32_array_create_x(x);
				}
				else if (b.type == "CUSTOM") {
					if (b.name == "arm.shader.NodesMaterial.newGroupButton") {
						b.name = "nodes_material_new_group_button";
					}
					else if (b.name == "arm.shader.NodesMaterial.groupOutputButton") {
						b.name = "nodes_material_group_output_button";
					}
					else if (b.name == "arm.shader.NodesMaterial.groupInputButton") {
						b.name = "nodes_material_group_input_button";
					}
					else if (b.name == "arm.shader.NodesMaterial.vectorCurvesButton") {
						b.name = "nodes_material_vector_curves_button";
					}
					else if (b.name == "arm.shader.NodesMaterial.colorRampButton") {
						b.name = "nodes_material_color_ramp_button";
					}
				}

				b.min       = armpack_map_get_f32(old, "min");
				b.max       = armpack_map_get_f32(old, "max");
				b.precision = armpack_map_get_f32(old, "precision");
				b.height    = armpack_map_get_f32(old, "height");
				array_push(n.buttons, b);
			}

			n.width = armpack_map_get_f32(old, "width");
			n.flags = 0;

			array_push(c.nodes, n);
		}

		c.links        = [];
		let las: any[] = map_get(old, "links");
		for (let i: i32 = 0; i < las.length; ++i) {
			let old: map_t<string, any> = las[i];
			let l: ui_node_link_t       = {};
			l.id                        = armpack_map_get_i32(old, "id");
			l.from_id                   = armpack_map_get_i32(old, "from_id");
			l.from_socket               = armpack_map_get_i32(old, "from_socket");
			l.to_id                     = armpack_map_get_i32(old, "to_id");
			l.to_socket                 = armpack_map_get_i32(old, "to_socket");
			array_push(c.links, l);
		}

		array_push(ar, c);
	}
	return ar;
}

function import_arm_is_legacy(b: buffer_t): bool {
	// Cloud materials are at version 0.8 / 0.9
	let has_version: bool = b[10] == 118; // 'v'
	let has_zero: bool    = b[22] == 48;  // '0'
	let has_dot: bool     = b[23] == 46;  // '.'
	let has_eight: bool   = b[24] == 56;  // '8'
	let has_nine: bool    = b[24] == 57;  // '9'
	if (has_version && has_zero && has_dot && (has_eight || has_nine)) {
		return true;
	}
	return false;
}

function import_arm_from_legacy(b: buffer_t): project_format_t {
	// Deprecated
	let old: map_t<string, any>   = armpack_decode_to_map(b);
	let project: project_format_t = {};
	project.version               = manifest_version_project;
	project.assets                = map_get(old, "assets");
	if (project.assets == null) {
		project.assets = [];
	}
	project.is_bgra = armpack_map_get_i32(old, "is_bgra") > 0;
	let pas: any[]  = map_get(old, "packed_assets");
	if (pas != null) {
		project.packed_assets = [];
		for (let i: i32 = 0; i < pas.length; ++i) {
			let old: map_t<string, any> = pas[i];
			let pa: packed_asset_t      = {};
			pa.name                     = map_get(old, "name");
			pa.bytes                    = map_get(old, "bytes");
			array_push(project.packed_assets, pa);
		}
	}
	project.brush_nodes     = _import_arm_get_legacy_node_canvas_array(old, "brush_nodes");
	project.brush_icons     = map_get(old, "brush_icons");
	project.material_nodes  = _import_arm_get_legacy_node_canvas_array(old, "material_nodes");
	project.material_groups = _import_arm_get_legacy_node_canvas_array(old, "material_groups");
	project.material_icons  = map_get(old, "material_icons");
	return project;
}
