
let a = Krom_import_usdc;
class R {
	get buffer() { return Krom_import_usdc._buffer(); }
}
let r = new R();

// import_usdc.js
let import_usdc = function(path, done) {
	Data.getBlob(path, function(b) {
		let buf_off = a._init(b.byteLength); //// Allocate r.buffer
		let buf = new Uint8Array(r.buffer, buf_off, b.byteLength);
		let bbuf = new Uint8Array(b);
		for (let i = 0; i < b.byteLength; ++i) buf[i] = bbuf[i];
		a._parse();
		let vertex_count = a._get_vertex_count();
		let index_count = a._get_index_count();
		let inda = new Uint32Array(r.buffer, a._get_indices(), index_count);
		let posa = new Int16Array(r.buffer, a._get_positions(), vertex_count * 4);
		let nora = new Int16Array(r.buffer, a._get_normals(), vertex_count * 2);
		let texa = new Int16Array(r.buffer, a._get_uvs(), vertex_count * 2);
		let name = path.split("\\").pop().split("/").pop().split(".").shift();
		done({
			name: name,
			posa: posa,
			nora: nora,
			texa: texa,
			inda: inda,
			scale_pos: a._get_scale_pos(),
			scale_tex: 1.0
		});
		// a._destroy(); //// Destroys r.buffer
		Data.deleteBlob(path);
	});
}

let plugin = Plugin.create();
let formats = Path.meshFormats;
let importers = Path.meshImporters;
formats.push("usdc");
importers.set("usdc", import_usdc);

plugin.delete = function() {
	formats.splice(formats.indexOf("usdc"), 1);
	importers.delete("usdc");
};
