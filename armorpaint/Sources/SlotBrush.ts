
class SlotBrushRaw {
	nodes = zui_nodes_create();
	canvas: zui_node_canvas_t;
	image: image_t = null; // 200px
	imageIcon: image_t = null; // 50px
	previewReady = false;
	id = 0;
}

class SlotBrush {
	static defaultCanvas: ArrayBuffer = null;

	static create(c: zui_node_canvas_t = null): SlotBrushRaw {
		let raw = new SlotBrushRaw();
		for (let brush of Project.brushes) if (brush.id >= raw.id) raw.id = brush.id + 1;

		if (c == null) {
			if (SlotBrush.defaultCanvas == null) { // Synchronous
				data_get_blob("default_brush.arm", (b: ArrayBuffer) => {
					SlotBrush.defaultCanvas = b;
				});
			}
			raw.canvas = armpack_decode(SlotBrush.defaultCanvas);
			raw.canvas.name = "Brush " + (raw.id + 1);
		}
		else {
			raw.canvas = c;
		}

		return raw;
	}
}
