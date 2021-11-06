
let a = Krom_uv_unwrap;
class R {
	get buffer() { return Krom_uv_unwrap._buffer(); }
}
let r = new R();

// uv_unwrap.js
let plugin = new arm.Plugin();
let h1 = new zui.Handle();
plugin.drawUI = function(ui) {
	if (ui.panel(h1, "UV Unwrap")) {
		if (ui.button("Unwrap Mesh")) {

			for (const po of arm.Project.paintObjects) {

				let raw = po.data.raw;
				let positions = raw.vertex_arrays[0].values;
				let normals = raw.vertex_arrays[1].values;
				let indices = raw.index_arrays[0].values;
				let vertexCount = positions.length / 4;
				let indexCount = indices.length;

				a._setVertexCount(vertexCount);
				a._setIndexCount(indexCount);
				let pa = new Float32Array(r.buffer, a._setPositions(), vertexCount * 3);
				let na = new Float32Array(r.buffer, a._setNormals(), vertexCount * 3);
				let ia = new Uint32Array(r.buffer, a._setIndices(), indexCount);

				let inv = 1 / 32767;

				for (let i = 0; i < vertexCount; i++) {
					pa[i * 3    ] = positions[i * 4    ] * inv;
					pa[i * 3 + 1] = positions[i * 4 + 1] * inv;
					pa[i * 3 + 2] = positions[i * 4 + 2] * inv;
					na[i * 3    ] = normals  [i * 2    ] * inv;
					na[i * 3 + 1] = normals  [i * 2 + 1] * inv;
					na[i * 3 + 2] = positions[i * 4 + 3] * inv;
				}
				for (let i = 0; i < indexCount; i++) {
					ia[i] = indices[i];
				}

				a._unwrap();

				vertexCount = a._getVertexCount();
				indexCount = a._getIndexCount();
				pa = new Float32Array(r.buffer, a._getPositions(), vertexCount * 3);
				na = new Float32Array(r.buffer, a._getNormals(), vertexCount * 3);
				let ua = new Float32Array(r.buffer, a._getUVs(), vertexCount * 2);
				ia = new Uint32Array(r.buffer, a._getIndices(), indexCount);

				let pa16 = new Int16Array(vertexCount * 4);
				let na16 = new Int16Array(vertexCount * 2);
				let ua16 = new Int16Array(vertexCount * 2);
				let ia32 = new Uint32Array(indexCount);

				for (let i = 0; i < vertexCount; i++) {
					pa16[i * 4    ] = pa[i * 3    ] / inv;
					pa16[i * 4 + 1] = pa[i * 3 + 1] / inv;
					pa16[i * 4 + 2] = pa[i * 3 + 2] / inv;
					pa16[i * 4 + 3] = na[i * 3 + 2] / inv;
					na16[i * 2    ] = na[i * 3    ] / inv;
					na16[i * 2 + 1] = na[i * 3 + 1] / inv;
					ua16[i * 2    ] = ua[i * 2    ] / inv;
					ua16[i * 2 + 1] = ua[i * 2 + 1] / inv;
				}
				for (let i = 0; i < indexCount; i++) {
					ia32[i] = ia[i];
				}

				po.data.raw.vertex_arrays[0].values = pa16;
				po.data.raw.vertex_arrays[1].values = na16;
				po.data.raw.vertex_arrays[2].values = ua16;
				po.data.raw.index_arrays[0].values = ia32;

				let geom = po.data.geom;
				geom.indices[0] = ia32;
				geom.ready = false;
				geom.build();

				a._destroy();
			}

			arm.MeshUtil.mergeMesh();
		}
	}
}
