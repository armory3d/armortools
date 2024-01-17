
class SlotMaterial {
	nodes = new Nodes();
	canvas: TNodeCanvas;
	image: Image = null;
	imageIcon: Image = null;
	previewReady = false;
	data: MaterialData;
	id = 0;
	static defaultCanvas: ArrayBuffer = null;

	paintBase = true;
	paintOpac = true;
	paintOcc = true;
	paintRough = true;
	paintMet = true;
	paintNor = true;
	paintHeight = true;
	paintEmis = true;
	paintSubs = true;

	constructor(m: MaterialData = null, c: TNodeCanvas = null) {
		for (let mat of Project.materials) if (mat.id >= this.id) this.id = mat.id + 1;
		this.data = m;

		let w = UtilRender.materialPreviewSize;
		let wIcon = 50;
		this.image = Image.createRenderTarget(w, w);
		this.imageIcon = Image.createRenderTarget(wIcon, wIcon);

		if (c == null) {
			if (SlotMaterial.defaultCanvas == null) { // Synchronous
				Data.getBlob("default_material.arm", (b: ArrayBuffer) => {
					SlotMaterial.defaultCanvas = b;
				});
			}
			this.canvas = ArmPack.decode(SlotMaterial.defaultCanvas);
			this.canvas.name = "Material " + (this.id + 1);
		}
		else {
			this.canvas = c;
		}

		///if (krom_android || krom_ios)
		this.nodes.panX -= 50; // Center initial position
		///end
	}

	unload = () => {
		let _next = () => {
			this.image.unload();
			this.imageIcon.unload();
		}
		Base.notifyOnNextFrame(_next);
	}

	delete = () => {
		this.unload();
		let mpos = Project.materials.indexOf(this);
		array_remove(Project.materials, this);
		if (Project.materials.length > 0) {
			Context.setMaterial(Project.materials[mpos > 0 ? mpos - 1 : 0]);
		}
	}
}
