
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

		c.nodes       = [];
		let ns: any[] = map_get(old, "nodes");
		for (let i: i32 = 0; i < ns.length; ++i) {
			let old: map_t<string, any> = ns[i];
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

function import_arm_get_node_socket_array(old: map_t<string, any>, key: string): ui_node_socket_t[] {
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
		s.default_value             = map_get(old, "default_value");
		s.min                       = armpack_map_get_f32(old, "min");
		s.max                       = armpack_map_get_f32(old, "max");
		s.precision                 = armpack_map_get_f32(old, "precision");
		s.display                   = armpack_map_get_i32(old, "display");
		array_push(sockets, s);
	}
	return sockets;
}

function import_arm_get_node_canvas_array(map: map_t<string, any>, key: string): ui_node_canvas_t[] {
	let cas: any[] = map_get(map, key);
	if (cas == null) {
		return null;
	}
	let ar: ui_node_canvas_t[] = [];
	for (let i: i32 = 0; i < cas.length; ++i) {
		let old: map_t<string, any> = cas[i];
		let c: ui_node_canvas_t     = {};
		c.name                      = map_get(old, "name");
		c.nodes                     = [];
		let ns: any[]               = map_get(old, "nodes");
		for (let i: i32 = 0; i < ns.length; ++i) {
			let old: map_t<string, any> = ns[i];
			let n: ui_node_t            = {};
			n.id                        = armpack_map_get_i32(old, "id");
			n.name                      = map_get(old, "name");
			n.type                      = map_get(old, "type");
			n.x                         = armpack_map_get_f32(old, "x");
			n.y                         = armpack_map_get_f32(old, "y");
			n.color                     = armpack_map_get_i32(old, "color");
			n.inputs                    = import_arm_get_node_socket_array(old, "inputs");
			n.outputs                   = import_arm_get_node_socket_array(old, "outputs");
			n.buttons                   = [];
			let bas: any[]              = map_get(old, "buttons");
			for (let i: i32 = 0; i < bas.length; ++i) {
				let old: map_t<string, any> = bas[i];
				let b: ui_node_button_t     = {};
				b.name                      = map_get(old, "name");
				b.type                      = map_get(old, "type");
				b.output                    = armpack_map_get_i32(old, "output");
				b.default_value             = map_get(old, "default_value");
				b.data                      = map_get(old, "data");
				b.min                       = armpack_map_get_f32(old, "min");
				b.max                       = armpack_map_get_f32(old, "max");
				b.precision                 = armpack_map_get_f32(old, "precision");
				b.height                    = armpack_map_get_f32(old, "height");
				array_push(n.buttons, b);
			}
			n.width = armpack_map_get_f32(old, "width");
			n.flags = armpack_map_get_i32(old, "flags");
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
	return has_version && has_zero && has_dot && (has_eight || has_nine);
}

function import_arm_is_version_2(b: buffer_t): bool {
	let has_version: bool = b[10] == 118; // 'v'
	let has_two: bool     = b[22] == 50;  // '2'
	if (has_version && has_two) {
		return true;
	}
	return false;
}

function import_arm_is_old(b: buffer_t): bool {
	return import_arm_is_legacy(b) || import_arm_is_version_2(b);
}

function import_arm_from_legacy(old: map_t<string, any>): project_format_t {
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

function import_arm_from_version_2(old: map_t<string, any>): project_format_t {
	let project: project_format_t = {};
	project.version               = manifest_version_project;
	project.assets                = map_get(old, "assets");
	project.is_bgra               = armpack_map_get_i32(old, "is_bgra") > 0;
	let pas: any[]                = map_get(old, "packed_assets");
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
	project.envmap          = map_get(old, "envmap");
	project.envmap_strength = armpack_map_get_f32(old, "envmap_strength");
	project.camera_world    = map_get(old, "camera_world");
	project.camera_origin   = map_get(old, "camera_origin");
	project.camera_fov      = armpack_map_get_f32(old, "camera_fov");
	let ss: any[]           = map_get(old, "swatches");
	project.swatches        = [];
	for (let i: i32 = 0; i < ss.length; ++i) {
		let old: map_t<string, any> = ss[i];
		let s: swatch_color_t       = {};
		s.base                      = armpack_map_get_i32(old, "base");
		s.opacity                   = armpack_map_get_f32(old, "opacity");
		s.occlusion                 = armpack_map_get_f32(old, "occlusion");
		s.roughness                 = armpack_map_get_f32(old, "roughness");
		s.metallic                  = armpack_map_get_f32(old, "metallic");
		s.normal                    = armpack_map_get_i32(old, "normal");
		s.emission                  = armpack_map_get_f32(old, "emission");
		s.height                    = armpack_map_get_f32(old, "height");
		s.subsurface                = armpack_map_get_f32(old, "subsurface");
		array_push(project.swatches, s);
	}
	project.brush_nodes    = import_arm_get_node_canvas_array(old, "brush_nodes");
	project.brush_icons    = map_get(old, "brush_icons");
	project.material_nodes = import_arm_get_node_canvas_array(old, "material_nodes");
	if (map_get(old, "material_groups") != null) {
		project.material_groups = import_arm_get_node_canvas_array(old, "material_groups");
	}
	project.material_icons = map_get(old, "material_icons");
	project.font_assets    = map_get(old, "font_assets");
	let lds: any[]         = map_get(old, "layer_datas");
	project.layer_datas    = [];
	for (let i: i32 = 0; i < lds.length; ++i) {
		let old: map_t<string, any> = lds[i];
		let ld: layer_data_t        = {};
		ld.name                     = map_get(old, "name");
		ld.res                      = armpack_map_get_i32(old, "res");
		ld.bpp                      = armpack_map_get_i32(old, "bpp");
		ld.texpaint                 = map_get(old, "texpaint");
		ld.uv_scale                 = armpack_map_get_f32(old, "uv_scale");
		ld.uv_rot                   = armpack_map_get_f32(old, "uv_rot");
		ld.uv_type                  = armpack_map_get_i32(old, "uv_type");
		ld.decal_mat                = map_get(old, "decal_mat");
		ld.opacity_mask             = armpack_map_get_f32(old, "opacity_mask");
		ld.fill_layer               = armpack_map_get_i32(old, "fill_layer");
		ld.object_mask              = armpack_map_get_i32(old, "object_mask");
		ld.blending                 = armpack_map_get_i32(old, "blending");
		ld.parent                   = armpack_map_get_i32(old, "parent");
		ld.visible                  = armpack_map_get_i32(old, "visible") > 0;
		ld.texpaint_nor             = map_get(old, "texpaint_nor");
		ld.texpaint_pack            = map_get(old, "texpaint_pack");
		ld.paint_base               = armpack_map_get_i32(old, "paint_base") > 0;
		ld.paint_opac               = armpack_map_get_i32(old, "paint_opac") > 0;
		ld.paint_occ                = armpack_map_get_i32(old, "paint_occ") > 0;
		ld.paint_rough              = armpack_map_get_i32(old, "paint_rough") > 0;
		ld.paint_met                = armpack_map_get_i32(old, "paint_met") > 0;
		ld.paint_nor                = armpack_map_get_i32(old, "paint_nor") > 0;
		ld.paint_nor_blend          = armpack_map_get_i32(old, "paint_nor_blend") > 0;
		ld.paint_height             = armpack_map_get_i32(old, "paint_height") > 0;
		ld.paint_height_blend       = armpack_map_get_i32(old, "paint_height_blend") > 0;
		ld.paint_emis               = armpack_map_get_i32(old, "paint_emis") > 0;
		ld.paint_subs               = armpack_map_get_i32(old, "paint_subs") > 0;
		////
		ld.uv_map = 0;
		////
		array_push(project.layer_datas, ld);
	}
	let ms: any[]      = map_get(old, "mesh_datas");
	project.mesh_datas = [];
	for (let i: i32 = 0; i < ms.length; ++i) {
		let old: map_t<string, any> = ms[i];
		let md: mesh_data_t         = {};
		md.name                     = map_get(old, "name");
		md.scale_pos                = armpack_map_get_f32(old, "scale_pos");
		md.scale_tex                = armpack_map_get_f32(old, "scale_tex");
		let vas: any[]              = map_get(old, "vertex_arrays");
		md.vertex_arrays            = [];
		for (let i: i32 = 0; i < vas.length; ++i) {
			let old: map_t<string, any> = vas[i];
			let va: vertex_array_t      = {};
			va.attrib                   = map_get(old, "attrib");
			va.data                     = map_get(old, "data");
			va.values                   = map_get(old, "values");
			array_push(md.vertex_arrays, va);
		}
		md.index_array = map_get(old, "index_array");
		array_push(project.mesh_datas, md);
	}
	project.mesh_assets   = map_get(old, "mesh_assets");
	project.mesh_icons    = map_get(old, "mesh_icons");
	project.atlas_objects = map_get(old, "atlas_objects");
	project.atlas_names   = map_get(old, "atlas_names");
	return project;
}

function import_arm_from_old(b: buffer_t): project_format_t {
	let old: map_t<string, any> = armpack_decode_to_map(b);
	if (import_arm_is_legacy(b)) {
		return import_arm_from_legacy(old);
	}
	if (import_arm_is_version_2(b)) {
		return import_arm_from_version_2(old);
	}
	return null;
}
