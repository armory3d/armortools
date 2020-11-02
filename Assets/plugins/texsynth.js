// Example-based texture synthesis written in Rust
// https://github.com/EmbarkStudios/texture-synthesis

// texsynth library is linked with armorcore until multi-threaded execution is supported via webassembly
// https://github.com/armory3d/armorpaint_plugins/tree/master/plugin_texsynth

let plugin = new arm.Plugin();
let h1 = new zui.Handle();

plugin.drawUI = function(ui) {
	if (ui.panel(h1, "Texture Synthesis")) {
		if (ui.button("Inpaint")) {
			texsynthInpaint(false);
		}
		if (ui.button("Tiling")) {
			texsynthInpaint(true);
		}
	}
};

function texsynthInpaint(tiling) {
	let w = arm.Config.getTextureResX();
	let h = arm.Config.getTextureResY();
	let l = arm.Context.layer;

	let bytes_img = l.texpaint.getPixels().b.bufferValue;
	let bytes_mask = l.texpaint_mask != null ? l.texpaint_mask.getPixels().b.bufferValue : new ArrayBuffer(w * h);
	let bytes_out = new core.Bytes(new ArrayBuffer(w * h * 4));
	Krom.texsynthInpaint(w, h, bytes_out.b.bufferValue, bytes_img, bytes_mask, tiling);
	let image = core.Image.fromBytes(bytes_out, w, h);

	function _next() {
		arm.Context.layerIsMask = false;
		arm.History.applyFilter();

		l.deleteMask();
		l.texpaint.unload();

		l.texpaint = core.Image.createRenderTarget(w, h);
		let g2 = l.texpaint.get_g2();
		g2.begin(false);
		g2.drawImage(image, 0, 0);
		g2.end();

		let rts = iron.RenderPath.active.renderTargets;
		rts.h["texpaint" + l.ext].image = l.texpaint;
		arm.MakeMaterial.parseMeshMaterial();
		arm.App.redrawUI();
		arm.Context.layerPreviewDirty = true;
	}
	arm.App.notifyOnNextFrame(_next);
}
