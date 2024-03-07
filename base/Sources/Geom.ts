
class Geom {

	static make_plane(size_x: f32 = 1.0, size_y: f32 = 1.0, verts_x: i32 = 2, verts_y: i32 = 2, uv_scale: f32 = 1.0): raw_mesh_t {

		let mesh: raw_mesh_t = {};
		mesh.scalePos = 1.0;
		mesh.scaleTex = 1.0;
		mesh.name = "";
		mesh.hasNext = false;

		// Pack positions to (-1, 1) range
		let halfX: f32 = size_x / 2;
		let halfY: f32 = size_y / 2;
		mesh.scalePos = Math.max(halfX, halfY);
		let inv: f32 = (1 / mesh.scalePos) * 32767;

		mesh.posa = new Int16Array(verts_x * verts_y * 4);
		mesh.nora = new Int16Array(verts_x * verts_y * 2);
		mesh.texa = new Int16Array(verts_x * verts_y * 2);
		mesh.inda = new Uint32Array((verts_x - 1) * (verts_y - 1) * 6);
		let stepX: f32 = size_x / (verts_x - 1);
		let stepY: f32 = size_y / (verts_y - 1);
		for (let i: i32 = 0; i < verts_x * verts_y; ++i) {
			let x: f32 = (i % verts_x) * stepX - halfX;
			let y: f32 = Math.floor(i / verts_x) * stepY - halfY;
			mesh.posa[i * 4    ] = Math.floor(x * inv);
			mesh.posa[i * 4 + 1] = Math.floor(y * inv);
			mesh.posa[i * 4 + 2] = 0;
			mesh.nora[i * 2    ] = 0;
			mesh.nora[i * 2 + 1] = 0;
			mesh.posa[i * 4 + 3] = 32767;
			x = (i % verts_x) / (verts_x - 1);
			y = 1.0 - Math.floor(i / verts_x) / (verts_y - 1);
			mesh.texa[i * 2    ] = (Math.floor(x * 32767 * uv_scale) - 1) % 32767;
			mesh.texa[i * 2 + 1] = (Math.floor(y * 32767 * uv_scale) - 1) % 32767;
		}
		for (let i: i32 = 0; i < (verts_x - 1) * (verts_y - 1); ++i) {
			let x: f32 = i % (verts_x - 1);
			let y: f32 = Math.floor(i / (verts_y - 1));
			mesh.inda[i * 6    ] = y * verts_x + x;
			mesh.inda[i * 6 + 1] = y * verts_x + x + 1;
			mesh.inda[i * 6 + 2] = (y + 1) * verts_x + x;
			mesh.inda[i * 6 + 3] = y * verts_x + x + 1;
			mesh.inda[i * 6 + 4] = (y + 1) * verts_x + x + 1;
			mesh.inda[i * 6 + 5] = (y + 1) * verts_x + x;
		}

		return mesh;
	}

	static make_uv_sphere(radius: f32 = 1.0, widthSegments: i32 = 32, heightSegments: i32 = 16, stretchUV: bool = true, uvScale: f32 = 1.0): raw_mesh_t {

		let mesh: raw_mesh_t = {};
		mesh.scalePos = 1.0;
		mesh.scaleTex = 1.0;
		mesh.name = "";
		mesh.hasNext = false;

		// Pack positions to (-1, 1) range
		mesh.scalePos = radius;
		mesh.scaleTex = uvScale;
		let inv: f32 = (1 / mesh.scalePos) * 32767;
		let pi2: f32 = Math.PI * 2;

		let widthVerts: i32 = widthSegments + 1;
		let heightVerts: i32 = heightSegments + 1;
		mesh.posa = new Int16Array(widthVerts * heightVerts * 4);
		mesh.nora = new Int16Array(widthVerts * heightVerts * 2);
		mesh.texa = new Int16Array(widthVerts * heightVerts * 2);
		mesh.inda = new Uint32Array(widthSegments * heightSegments * 6 - widthSegments * 6);

		let nor: vec4_t = vec4_create();
		let pos: i32 = 0;
		for (let y: i32 = 0; y < heightVerts; ++y) {
			let v: f32 = y / heightSegments;
			let vFlip: f32 = 1.0 - v;
			if (!stretchUV) vFlip /= 2;
			let uOff: f32 = y == 0 ? 0.5 / widthSegments : y == heightSegments ? -0.5 / widthSegments : 0.0;
			for (let x: i32 = 0; x < widthVerts; ++x) {
				let u: f32 = x / widthSegments;
				let uPI2: f32 = u * pi2;
				let vPI: f32  = v * Math.PI;
				let vPIsin: f32 = Math.sin(vPI);
				let vx: f32 = -radius * Math.cos(uPI2) * vPIsin;
				let vy: f32 =  radius * Math.sin(uPI2) * vPIsin;
				let vz: f32 = -radius * Math.cos(vPI);
				let i4: i32 = pos * 4;
				let i2: i32 = pos * 2;
				mesh.posa[i4    ] = Math.floor(vx * inv);
				mesh.posa[i4 + 1] = Math.floor(vy * inv);
				mesh.posa[i4 + 2] = Math.floor(vz * inv);
				vec4_normalize(vec4_set(nor, vx, vy, vz));
				mesh.posa[i4 + 3] = Math.floor(nor.z * 32767);
				mesh.nora[i2    ] = Math.floor(nor.x * 32767);
				mesh.nora[i2 + 1] = Math.floor(nor.y * 32767);
				mesh.texa[i2    ] = (Math.floor((u + uOff) * 32767) - 1) % 32767;
				mesh.texa[i2 + 1] = (Math.floor(vFlip      * 32767) - 1) % 32767;
				pos++;
			}
		}

		pos = 0;
		let heightSegments1: i32 = heightSegments - 1;
		for (let y: i32 = 0; y < heightSegments; ++y) {
			for (let x: i32 = 0; x < widthSegments; ++x) {
				let x1: i32 = x + 1;
				let y1: i32 = y + 1;
				let a: f32 = y  * widthVerts + x1;
				let b: f32 = y  * widthVerts + x;
				let c: f32 = y1 * widthVerts + x;
				let d: f32 = y1 * widthVerts + x1;
				if (y > 0) {
					mesh.inda[pos++] = a;
					mesh.inda[pos++] = b;
					mesh.inda[pos++] = d;
				}
				if (y < heightSegments1) {
					mesh.inda[pos++] = b;
					mesh.inda[pos++] = c;
					mesh.inda[pos++] = d;
				}
			}
		}

		return mesh;
	}
}

type raw_mesh_t = {
	posa?: Int16Array;
	nora?: Int16Array;
	texa?: Int16Array;
	inda?: Uint32Array;
	scalePos?: f32;
	scaleTex?: f32;
	name?: string;
	hasNext?: bool;
};
