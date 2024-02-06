
class SlotMaterialRaw {
	nodes = zui_nodes_create();
	canvas: zui_node_canvas_t;
	image: image_t = null;
	imageIcon: image_t = null;
	previewReady = false;
	data: material_data_t;
	id = 0;

	paintBase = true;
	paintOpac = true;
	paintOcc = true;
	paintRough = true;
	paintMet = true;
	paintNor = true;
	paintHeight = true;
	paintEmis = true;
	paintSubs = true;
}

class SlotMaterial {
	static defaultCanvas: ArrayBuffer = null;

	static create(m: material_data_t = null, c: zui_node_canvas_t = null): SlotMaterialRaw {
		let raw = new SlotMaterialRaw();
		for (let mat of Project.materials) if (mat.id >= raw.id) raw.id = mat.id + 1;
		raw.data = m;

		let w = UtilRender.materialPreviewSize;
		let wIcon = 50;
		raw.image = image_create_render_target(w, w);
		raw.imageIcon = image_create_render_target(wIcon, wIcon);

		if (c == null) {
			if (SlotMaterial.defaultCanvas == null) { // Synchronous
				data_get_blob("default_material.arm", (b: ArrayBuffer) => {
					SlotMaterial.defaultCanvas = b;
				});
			}
			raw.canvas = armpack_decode(SlotMaterial.defaultCanvas);
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
			image_unload(raw.imageIcon);
		}
		Base.notifyOnNextFrame(_next);
	}

	static delete = (raw: SlotMaterialRaw) => {
		SlotMaterial.unload(raw);
		let mpos = Project.materials.indexOf(raw);
		array_remove(Project.materials, this);
		if (Project.materials.length > 0) {
			Context.setMaterial(Project.materials[mpos > 0 ? mpos - 1 : 0]);
		}
	}
}
