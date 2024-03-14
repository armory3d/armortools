
class SlotMaterialRaw {
	nodes: zui_nodes_t = zui_nodes_create();
	canvas: zui_node_canvas_t;
	image: image_t = null;
	image_icon: image_t = null;
	preview_ready: bool = false;
	data: material_data_t;
	id: i32 = 0;

	paint_base: bool = true;
	paint_opac: bool = true;
	paint_occ: bool = true;
	paint_rough: bool = true;
	paint_met: bool = true;
	paint_nor: bool = true;
	paint_height: bool = true;
	paint_emis: bool = true;
	paint_subs: bool = true;
}

let slot_material_default_canvas: ArrayBuffer = null;

function slot_material_create(m: material_data_t = null, c: zui_node_canvas_t = null): SlotMaterialRaw {
	let raw: SlotMaterialRaw = new SlotMaterialRaw();
	for (let mat of project_materials) if (mat.id >= raw.id) raw.id = mat.id + 1;
	raw.data = m;

	let w: i32 = util_render_material_preview_size;
	let w_icon: i32 = 50;
	raw.image = image_create_render_target(w, w);
	raw.image_icon = image_create_render_target(w_icon, w_icon);

	if (c == null) {
		if (slot_material_default_canvas == null) { // Synchronous
			let b: ArrayBuffer = data_get_blob("default_material.arm");
			slot_material_default_canvas = b;
		}
		raw.canvas = armpack_decode(slot_material_default_canvas);
		raw.canvas.name = "Material " + (raw.id + 1);
	}
	else {
		raw.canvas = c;
	}

	///if (krom_android || krom_ios)
	raw.nodes.panX -= 50; // Center initial position
	///end

	return raw;
}

function slot_material_unload(raw: SlotMaterialRaw) {
	let _next = () => {
		image_unload(raw.image);
		image_unload(raw.image_icon);
	}
	base_notify_on_next_frame(_next);
}

function slot_material_delete(raw: SlotMaterialRaw) {
	slot_material_unload(raw);
	let mpos: i32 = project_materials.indexOf(raw);
	array_remove(project_materials, this);
	if (project_materials.length > 0) {
		context_set_material(project_materials[mpos > 0 ? mpos - 1 : 0]);
	}
}
