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
	if (l.texpaint_mask == null) return;
	let bytes_img = l.texpaint.getPixels().b.bufferValue;
	let bytes_mask = l.texpaint_mask.getPixels().b.bufferValue;
	let view_img = new Uint8Array(bytes_img);
	let view_mask = new Uint8Array(bytes_mask);
	for (let i = 0; i < view_img.length / 4; ++i) {
		view_img[i * 4 + 3] = view_mask[i];
	}

	let bytes_out = new arm.Bytes(new ArrayBuffer(w * h * 4));
	Krom.texsynthInpaint(w, h, bytes_out.b.bufferValue, bytes_img, tiling);

	let image = arm.Image.fromBytes(bytes_out, w, h);
	var asset = {name: "tex_synth.png", file: "/tex_synth.png", id: arm.Project.assetId++};
	iron.Data.cachedImages.h[asset.file] = image;
	arm.Project.assets.push(asset);
	arm.Project.assetNames.push(asset.name);
	arm.Project.assetMap.h[asset.id] = image;
}
