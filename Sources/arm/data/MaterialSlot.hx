package arm.data;

import haxe.Json;
import kha.Image;
import kha.Blob;
import zui.Nodes;
import iron.data.MaterialData;
import iron.data.Data;
import arm.util.RenderUtil;

class MaterialSlot {
	public var nodes = new Nodes();
	public var canvas: TNodeCanvas;
	public var image: Image = null;
	public var imageIcon: Image = null;
	public var previewReady = false;
	public var data: MaterialData;
	public var id = 0;
	static var defaultCanvas: Blob = null;

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

		var w = RenderUtil.matPreviewSize;
		var wIcon = 50;
		image = Image.createRenderTarget(w, w);
		imageIcon = Image.createRenderTarget(wIcon, wIcon);

		if (c == null) {
			if (defaultCanvas == null) { // Synchronous
				Data.getBlob("default_material.arm", function(b: Blob) {
					defaultCanvas = b;
				});
			}
			canvas = iron.system.ArmPack.decode(defaultCanvas.toBytes());
			canvas.name = "Material " + (id + 1);
		}
		else {
			canvas = c;
		}
	}

	public function unload() {
		function _next() {
			image.unload();
			imageIcon.unload();
		}
		App.notifyOnNextFrame(_next);
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
