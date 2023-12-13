package arm;

import zui.Zui.Nodes;
import zui.Zui.TNodeCanvas;
import iron.System;
import iron.MaterialData;
import iron.Data;
import iron.ArmPack;

class SlotMaterial {
	public var nodes = new Nodes();
	public var canvas: TNodeCanvas;
	public var image: Image = null;
	public var imageIcon: Image = null;
	public var previewReady = false;
	public var data: MaterialData;
	public var id = 0;
	static var defaultCanvas: js.lib.ArrayBuffer = null;

	public var paintBase = true;
	public var paintOpac = true;
	public var paintOcc = true;
	public var paintRough = true;
	public var paintMet = true;
	public var paintNor = true;
	public var paintHeight = true;
	public var paintEmis = true;
	public var paintSubs = true;

	public function new(m: MaterialData = null, c: TNodeCanvas = null) {
		for (mat in Project.materials) if (mat.id >= id) id = mat.id + 1;
		data = m;

		var w = UtilRender.materialPreviewSize;
		var wIcon = 50;
		image = Image.createRenderTarget(w, w);
		imageIcon = Image.createRenderTarget(wIcon, wIcon);

		if (c == null) {
			if (defaultCanvas == null) { // Synchronous
				Data.getBlob("default_material.arm", function(b: js.lib.ArrayBuffer) {
					defaultCanvas = b;
				});
			}
			canvas = ArmPack.decode(defaultCanvas);
			canvas.name = "Material " + (id + 1);
		}
		else {
			canvas = c;
		}

		#if (krom_android || krom_ios)
		nodes.panX -= 50; // Center initial position
		#end
	}

	public function unload() {
		function _next() {
			image.unload();
			imageIcon.unload();
		}
		Base.notifyOnNextFrame(_next);
	}

	public function delete() {
		unload();
		var mpos = Project.materials.indexOf(this);
		Project.materials.remove(this);
		if (Project.materials.length > 0) {
			Context.setMaterial(Project.materials[mpos > 0 ? mpos - 1 : 0]);
		}
	}
}
