
class SlotBrushRaw {
	nodes: zui_nodes_t = zui_nodes_create();
	canvas: zui_node_canvas_t;
	image: image_t = null; // 200px
	image_icon: image_t = null; // 50px
	preview_ready: bool = false;
	id: i32 = 0;
}

class SlotBrush {
	static default_canvas: ArrayBuffer = null;

	static create(c: zui_node_canvas_t = null): SlotBrushRaw {
		let raw: SlotBrushRaw = new SlotBrushRaw();
		for (let brush of project_brushes) if (brush.id >= raw.id) raw.id = brush.id + 1;

		if (c == null) {
			if (SlotBrush.default_canvas == null) { // Synchronous
				let b: ArrayBuffer = data_get_blob("default_brush.arm")
				SlotBrush.default_canvas = b;
			}
			raw.canvas = armpack_decode(SlotBrush.default_canvas);
			raw.canvas.name = "Brush " + (raw.id + 1);
		}
		else {
			raw.canvas = c;
		}

		return raw;
	}
}
