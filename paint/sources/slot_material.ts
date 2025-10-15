
type slot_material_t = {
	nodes?: ui_nodes_t;
	canvas?: ui_node_canvas_t;
	image?: gpu_texture_t;
	image_icon?: gpu_texture_t;
	preview_ready?: bool;
	data?: material_data_t;
	id?: i32;
	paint_base?: bool;
	paint_opac?: bool;
	paint_occ?: bool;
	paint_rough?: bool;
	paint_met?: bool;
	paint_nor?: bool;
	paint_height?: bool;
	paint_emis?: bool;
	paint_subs?: bool;
};

let slot_material_default_canvas: buffer_t = null;

function slot_material_create(m: material_data_t = null, c: ui_node_canvas_t = null): slot_material_t {
	let raw: slot_material_t = {};
	raw.nodes                = ui_nodes_create();
	raw.preview_ready        = false;
	raw.id                   = 0;
	raw.paint_base           = true;
	raw.paint_opac           = true;
	raw.paint_occ            = true;
	raw.paint_rough          = true;
	raw.paint_met            = true;
	raw.paint_nor            = true;
	raw.paint_height         = true;
	raw.paint_emis           = true;
	raw.paint_subs           = true;

	for (let i: i32 = 0; i < project_materials.length; ++i) {
		let mat: slot_material_t = project_materials[i];
		if (mat.id >= raw.id) {
			raw.id = mat.id + 1;
		}
	}
	raw.data = m;

	let w: i32      = util_render_material_preview_size;
	let w_icon: i32 = 50;
	raw.image       = gpu_create_render_target(w, w, tex_format_t.RGBA64);
	raw.image_icon  = gpu_create_render_target(w_icon, w_icon, tex_format_t.RGBA64);

	if (c == null) {
		if (slot_material_default_canvas == null) { // Synchronous
			let b: buffer_t              = data_get_blob("default_material.arm");
			slot_material_default_canvas = b;
		}
		raw.canvas      = armpack_decode(slot_material_default_canvas);
		raw.canvas      = util_clone_canvas(raw.canvas); // Clone to create GC references
		let id: i32     = (raw.id + 1);
		raw.canvas.name = "Material " + id;
	}
	else {
		raw.canvas = util_clone_canvas(c);
	}

	if (config_raw.node_previews) {
		for (let i: i32 = 0; i < raw.canvas.nodes.length; ++i) {
			let n: ui_node_t = raw.canvas.nodes[i];
			n.flags |= _ui_node_flag_t.PREVIEW;
		}
	}

	/// if (arm_android || arm_ios)
	raw.nodes.pan_x -= 50; // Center initial position
	/// else
	raw.nodes.pan_x += 110;
	/// end
	raw.nodes.pan_y += 120;

	return raw;
}

function slot_material_unload(raw: slot_material_t) {
	gpu_delete_texture(raw.image);
	gpu_delete_texture(raw.image_icon);
}

function slot_material_delete(raw: slot_material_t) {
	slot_material_unload(raw);
	let mpos: i32 = array_index_of(project_materials, raw);
	array_remove(project_materials, raw);
	if (project_materials.length > 0) {
		context_set_material(project_materials[mpos > 0 ? mpos - 1 : 0]);
	}
}
