
let a = Krom_import_fbx;
class R {
	get buffer() { return Krom_import_fbx._buffer(); }
}
let r = new R();

// import_fbx.js
let import_fbx = function(path, done) {
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
		let texa = a._get_uvs() > 0 ? new Int16Array(r.buffer, a._get_uvs(), vertex_count * 2) : null;
		let cola = a._get_colors() > 0 ? new Int16Array(r.buffer, a._get_colors(), vertex_count * 3) : null;

		// Use .slice() for now as the next mesh will overwrite buffer data corrupting the old vertex data
		done({
			name: a._get_name(),
			posa: posa.slice(),
			nora: nora.slice(),
			texa: texa != null ? texa.slice() : null,
			cola: cola != null ? cola.slice() : null,
			inda: inda.slice(),
			scale_pos: a._get_scale_pos(),
			scale_tex: 1.0,
			transform: a._get_transform(),
			has_next: a._has_next()
		});
		// a._destroy(); //// Destroys r.buffer
		// Data.deleteBlob(path);
	});
}

let plugin = Plugin.create();
let formats = Path.meshFormats;
let importers = Path.meshImporters;
formats.push("fbx");
importers.set("fbx", import_fbx);

plugin.delete = function() {
	formats.splice(formats.indexOf("fbx"), 1);
	importers.delete("fbx");
};
