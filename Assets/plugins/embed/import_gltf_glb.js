
let a = Krom_import_gltf;
class R {
	get buffer() { return Krom_import_gltf._buffer(); }
}
let r = new R();

// uv_unwrap.js
let import_gltf_glb = function(path, done) {
	iron.Data.getBlob(path, function(b) {
		let buf = new Uint8Array(r.buffer, a._init(b.bytes.length), b.bytes.length);
		for (let i = 0; i < b.bytes.length; ++i) buf[i] = b.readU8(i);
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
		a._destroy();
		iron.Data.deleteBlob(path);
	});
}

let plugin = new arm.Plugin();
let formats = arm.Path.meshFormats;
let importers = arm.Path.meshImporters;
formats.push("gltf");
formats.push("glb");
importers.h["gltf"] = import_gltf_glb;
importers.h["glb"] = import_gltf_glb;

plugin.delete = function() {
	formats.splice(formats.indexOf("gltf"), 1);
	formats.splice(formats.indexOf("glb"), 1);
	importers.h["gltf"] = null;
	importers.h["glb"] = null;
};
