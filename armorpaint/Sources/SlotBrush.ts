
class SlotBrushRaw {
	nodes = Nodes.create();
	canvas: TNodeCanvas;
	image: image_t = null; // 200px
	imageIcon: image_t = null; // 50px
	previewReady = false;
	id = 0;
}

class SlotBrush {
	static defaultCanvas: ArrayBuffer = null;

	static create(c: TNodeCanvas = null): SlotBrushRaw {
		let raw = new SlotBrushRaw();
		for (let brush of Project.brushes) if (brush.id >= raw.id) raw.id = brush.id + 1;

		if (c == null) {
			if (SlotBrush.defaultCanvas == null) { // Synchronous
				Data.getBlob("default_brush.arm", (b: ArrayBuffer) => {
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
