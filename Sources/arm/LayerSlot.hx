package arm;

import iron.RenderPath;

class LayerSlot {
	static var counter = 0;
	public var id = 0;
	public var visible = true;
	public var ext = "";

	public var texpaint:kha.Image;
	public var texpaint_nor:kha.Image;
	public var texpaint_pack:kha.Image;
	public var texpaint_opt:kha.Image;

	// For undo layer
	public var targetLayer:LayerSlot = null;
	public var targetObject:iron.object.MeshObject = null;

	public function new(ext = "") {
		if (ext == "") {
			id = counter++;
			ext = id + "";
		}
		this.ext = ext;

		{
			var t = new RenderTargetRaw();
			t.name = "texpaint" + ext;
			t.width = UITrait.inst.getTextureRes();
			t.height = UITrait.inst.getTextureRes();
			t.format = 'RGBA32';
			t.depth_buffer = "paintdb";
			texpaint = RenderPath.active.createRenderTarget(t).image;
		}
		{
			var t = new RenderTargetRaw();
			t.name = "texpaint_nor" + ext;
			t.width = UITrait.inst.getTextureRes();
			t.height = UITrait.inst.getTextureRes();
			t.format = 'RGBA32';
			texpaint_nor = RenderPath.active.createRenderTarget(t).image;
		}
		{
			var t = new RenderTargetRaw();
			t.name = "texpaint_pack" + ext;
			t.width = UITrait.inst.getTextureRes();
			t.height = UITrait.inst.getTextureRes();
			t.format = 'RGBA32';
			texpaint_pack = RenderPath.active.createRenderTarget(t).image;
		}

		if (UITrait.inst.paintHeight) make_texpaint_opt();
	}

	public function make_texpaint_opt() {
		if (texpaint_opt != null) return;

		{
			var t = new RenderTargetRaw();
			t.name = "texpaint_opt" + ext;
			t.width = UITrait.inst.getTextureRes();
			t.height = UITrait.inst.getTextureRes();
			t.format = 'RGBA32';
			texpaint_opt = RenderPath.active.createRenderTarget(t).image;
		}
	}

	public function unload() {
		texpaint.unload();
		texpaint_nor.unload();
		texpaint_pack.unload();

		RenderPath.active.renderTargets.remove("texpaint" + ext);
		RenderPath.active.renderTargets.remove("texpaint_nor" + ext);
		RenderPath.active.renderTargets.remove("texpaint_pack" + ext);

		if (texpaint_opt != null) {
			texpaint_opt.unload();
			RenderPath.active.renderTargets.remove("texpaint_opt" + ext);
		}
	}

	public function swap(other:LayerSlot) {
		var tp = texpaint;
		var tp_nor = texpaint_nor;
		var tp_pack = texpaint_pack;

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

		if (texpaint_opt != null) {
			var tp_opt = texpaint_opt;
			RenderPath.active.renderTargets.get("texpaint_opt" + ext).image = other.texpaint_opt;
			RenderPath.active.renderTargets.get("texpaint_opt" + other.ext).image = texpaint_opt;
			texpaint_opt = other.texpaint_opt;
			other.texpaint_opt = tp_opt;
		}
		
		// var tp_rt = rt;
		// rt = other.rt;
		// other.rt = tp_rt;
	}
}
