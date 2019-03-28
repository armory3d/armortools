package arm;

import iron.RenderPath;
import arm.ui.*;

class LayerSlot {
	public static var counter = 0;
	public var id = 0;
	public var visible = true;
	public var ext = "";

	public var name:String;

	public var texpaint:kha.Image;
	public var texpaint_nor:kha.Image;
	public var texpaint_pack:kha.Image;

	public var texpaint_preview:kha.Image; // Layer preview

	public var texpaint_mask:kha.Image = null;
	public var texpaint_mask_preview:kha.Image;
	public var maskOpacity = 1.0;

	// For undo layer
	public var targetLayer:LayerSlot = null;
	public var targetObject:iron.object.MeshObject = null;

	static var first = true;

	public var objectMask = 0;
	public var paintBase = true;
	public var paintOpac = true;
	public var paintOcc = true;
	public var paintRough = true;
	public var paintMet = true;
	public var paintNor = true;
	public var paintHeight = false;
	public var paintEmis = false;
	public var paintSubs = false;

	public function new(ext = "") {

		if (first) {
			first = false;
			{
				var t = new RenderTargetRaw();
				t.name = "texpaint_blend0";
				t.width = Config.getTextureRes();
				t.height = Config.getTextureRes();
				t.format = 'R8';
				RenderPath.active.createRenderTarget(t);
			}
			{
				var t = new RenderTargetRaw();
				t.name = "texpaint_blend1";
				t.width = Config.getTextureRes();
				t.height = Config.getTextureRes();
				t.format = 'R8';
				RenderPath.active.createRenderTarget(t);
			}
		}

		if (ext == "") {
			id = counter++;
			ext = id + "";
		}
		this.ext = ext;
		name = "Layer " + (id + 1);

		{
			var t = new RenderTargetRaw();
			t.name = "texpaint" + ext;
			t.width = Config.getTextureRes();
			t.height = Config.getTextureRes();
			t.format = 'RGBA32';
			t.depth_buffer = "paintdb";
			texpaint = RenderPath.active.createRenderTarget(t).image;
		}
		{
			var t = new RenderTargetRaw();
			t.name = "texpaint_nor" + ext;
			t.width = Config.getTextureRes();
			t.height = Config.getTextureRes();
			t.format = 'RGBA32';
			texpaint_nor = RenderPath.active.createRenderTarget(t).image;
		}
		{
			var t = new RenderTargetRaw();
			t.name = "texpaint_pack" + ext;
			t.width = Config.getTextureRes();
			t.height = Config.getTextureRes();
			t.format = 'RGBA32';
			texpaint_pack = RenderPath.active.createRenderTarget(t).image;
		}

		texpaint_preview = kha.Image.createRenderTarget(200, 200, kha.graphics4.TextureFormat.RGBA32);
	}

	public function unload() {
		// Set null depth so paintdb stays alive
		if (UITrait.inst.layers.length > 0 && UITrait.inst.layers[0] != this) {
			texpaint.setDepthStencilFrom(texpaint_nor);
		}

		texpaint.unload();
		texpaint_nor.unload();
		texpaint_pack.unload();

		RenderPath.active.renderTargets.remove("texpaint" + ext);
		RenderPath.active.renderTargets.remove("texpaint_nor" + ext);
		RenderPath.active.renderTargets.remove("texpaint_pack" + ext);

		texpaint_preview.unload();

		deleteMask();
	}

	public function swap(other:LayerSlot) {
		var tp = texpaint;
		var tp_nor = texpaint_nor;
		var tp_pack = texpaint_pack;

		RenderPath.active.renderTargets.get("texpaint" + ext).image.setDepthStencilFrom(other.texpaint);

		RenderPath.active.renderTargets.get("texpaint" + ext).image = other.texpaint;
		RenderPath.active.renderTargets.get("texpaint_nor" + ext).image = other.texpaint_nor;
		RenderPath.active.renderTargets.get("texpaint_pack" + ext).image = other.texpaint_pack;
		
		RenderPath.active.renderTargets.get("texpaint" + other.ext).image = texpaint;
		RenderPath.active.renderTargets.get("texpaint_nor" + other.ext).image = texpaint_nor;
		RenderPath.active.renderTargets.get("texpaint_pack" + other.ext).image = texpaint_pack;

		texpaint = other.texpaint;
		texpaint_nor = other.texpaint_nor;
		texpaint_pack = other.texpaint_pack;

		other.texpaint = tp;
		other.texpaint_nor = tp_nor;
		other.texpaint_pack = tp_pack;
	}

	var createMaskColor:Int;

	public function createMask(color:Int) {
		if (texpaint_mask != null) return;

		createMaskColor = color;

		{
			var t = new RenderTargetRaw();
			t.name = "texpaint_mask" + ext;
			t.width = Config.getTextureRes();
			t.height = Config.getTextureRes();
			t.format = 'R8';
			texpaint_mask = RenderPath.active.createRenderTarget(t).image;
		}

		texpaint_mask_preview = kha.Image.createRenderTarget(200, 200, kha.graphics4.TextureFormat.L8);

		iron.App.notifyOnRender(clearMask);
	}

	function clearMask(g:kha.graphics4.Graphics) {
		g.end();

		texpaint_mask.g4.begin();
		texpaint_mask.g4.clear(createMaskColor);
		texpaint_mask.g4.end();

		g.begin();
		iron.App.removeRender(clearMask);
	}

	public function deleteMask() {
		if (texpaint_mask == null) return;

		texpaint_mask.unload();
		RenderPath.active.renderTargets.remove("texpaint_mask" + ext);
		texpaint_mask = null;

		texpaint_mask_preview.unload();
	}
}
