
let a = Krom_import_svg;
class R {
	get buffer() { return Krom_import_svg._buffer(); }
}
let r = new R();

// import_svg.js
let import_svg = function(path, done) {
	Data.getBlob(path, function(b) {
		let buf_off = a._init(b.byteLength + 1); //// Allocate r.buffer
		let buf = new Uint8Array(r.buffer, buf_off, b.byteLength + 1);
		let bbuf = new Uint8Array(b);
		for (let i = 0; i < b.byteLength; ++i) buf[i] = bbuf[i];
		buf[b.byteLength] = 0;

		a._parse();
		let w = a._get_pixels_w();
		let h = a._get_pixels_h();
		let pixels = r.buffer.slice(a._get_pixels(), a._get_pixels() + w * h * 4);
		let image = Image.fromBytes(pixels, w, h);
		done(image);

		// a._destroy(); //// Destroys r.buffer
		Data.deleteBlob(path);
	});
}

let plugin = Plugin.create();
let formats = Path.textureFormats;
let importers = Path.textureImporters;
formats.push("svg");
importers.set("svg", import_svg);

plugin.delete = function() {
	formats.splice(formats.indexOf("svg"), 1);
	importers.delete("svg");
};
