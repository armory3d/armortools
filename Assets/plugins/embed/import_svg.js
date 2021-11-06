
let a = Krom_import_svg;
class R {
	get buffer() { return Krom_import_svg._buffer(); }
}
let r = new R();

// import_svg.js
let import_svg = function(path, done) {
	iron.Data.getBlob(path, function(b) {
		let buf = new Uint8Array(r.buffer, a._init(b.bytes.length + 1), b.bytes.length + 1);
		for (let i = 0; i < b.bytes.length; ++i) buf[i] = b.readU8(i);
		buf[b.bytes.length] = 0;

		a._parse();
		let w = a._get_pixels_w();
		let h = a._get_pixels_h();
		let pixels = r.buffer.slice(a._get_pixels(), a._get_pixels() + w * h * 4);
		let image = core.Image.fromBytes(core.Bytes.ofData(pixels), w, h);
		done(image);

		a._destroy();
		iron.Data.deleteBlob(path);
	});
}

let plugin = new arm.Plugin();
let formats = arm.Path.textureFormats;
let importers = arm.Path.textureImporters;
formats.push("svg");
importers.h["svg"] = import_svg;

plugin.delete = function() {
	formats.splice(formats.indexOf("svg"), 1);
	importers.h["svg"] = null;
};

