
class SlotMaterialRaw {
	nodes = new Nodes();
	canvas: TNodeCanvas;
	image: Image = null;
	imageIcon: Image = null;
	previewReady = false;
	data: MaterialData;
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

	static create(m: MaterialData = null, c: TNodeCanvas = null): SlotMaterialRaw {
		let raw = new SlotMaterialRaw();
		for (let mat of Project.materials) if (mat.id >= raw.id) raw.id = mat.id + 1;
		raw.data = m;

		let w = UtilRender.materialPreviewSize;
		let wIcon = 50;
		raw.image = Image.createRenderTarget(w, w);
		raw.imageIcon = Image.createRenderTarget(wIcon, wIcon);

		if (c == null) {
			if (SlotMaterial.defaultCanvas == null) { // Synchronous
				Data.getBlob("default_material.arm", (b: ArrayBuffer) => {
					SlotMaterial.defaultCanvas = b;
				});
			}
			raw.canvas = ArmPack.decode(SlotMaterial.defaultCanvas);
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
			raw.image.unload();
			raw.imageIcon.unload();
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
