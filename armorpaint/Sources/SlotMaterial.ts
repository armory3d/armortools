
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

class SlotMaterial {
	static default_canvas: ArrayBuffer = null;

	static create(m: material_data_t = null, c: zui_node_canvas_t = null): SlotMaterialRaw {
		let raw: SlotMaterialRaw = new SlotMaterialRaw();
		for (let mat of Project.materials) if (mat.id >= raw.id) raw.id = mat.id + 1;
		raw.data = m;

		let w: i32 = UtilRender.material_preview_size;
		let w_icon: i32 = 50;
		raw.image = image_create_render_target(w, w);
		raw.image_icon = image_create_render_target(w_icon, w_icon);

		if (c == null) {
			if (SlotMaterial.default_canvas == null) { // Synchronous
				let b: ArrayBuffer = data_get_blob("default_material.arm");
				SlotMaterial.default_canvas = b;
			}
			raw.canvas = armpack_decode(SlotMaterial.default_canvas);
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

	static unload = (raw: SlotMaterialRaw) => {
		let _next = () => {
			image_unload(raw.image);
			image_unload(raw.image_icon);
		}
		base_notify_on_next_frame(_next);
	}

	static delete = (raw: SlotMaterialRaw) => {
		SlotMaterial.unload(raw);
		let mpos: i32 = Project.materials.indexOf(raw);
		array_remove(Project.materials, this);
		if (Project.materials.length > 0) {
			Context.set_material(Project.materials[mpos > 0 ? mpos - 1 : 0]);
		}
	}
}
