
class SlotBrush {
	nodes = new Nodes();
	canvas: TNodeCanvas;
	image: Image = null; // 200px
	imageIcon: Image = null; // 50px
	previewReady = false;
	id = 0;
	static defaultCanvas: ArrayBuffer = null;

	constructor(c: TNodeCanvas = null) {
		for (let brush of Project.brushes) if (brush.id >= this.id) this.id = brush.id + 1;

		if (c == null) {
			if (SlotBrush.defaultCanvas == null) { // Synchronous
				Data.getBlob("default_brush.arm", (b: ArrayBuffer) => {
					SlotBrush.defaultCanvas = b;
				});
			}
			this.canvas = ArmPack.decode(SlotBrush.defaultCanvas);
			this.canvas.name = "Brush " + (this.id + 1);
		}
		else {
			this.canvas = c;
		}
	}
}
