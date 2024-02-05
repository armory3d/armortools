
class Geom {

	static make_plane(sizeX = 1.0, sizeY = 1.0, vertsX = 2, vertsY = 2, uvScale = 1.0): TRawMesh {

		let mesh: TRawMesh = {};
		mesh.scalePos = 1.0;
		mesh.scaleTex = 1.0;
		mesh.name = "";
		mesh.hasNext = false;

		// Pack positions to (-1, 1) range
		let halfX = sizeX / 2;
		let halfY = sizeY / 2;
		mesh.scalePos = Math.max(halfX, halfY);
		let inv = (1 / mesh.scalePos) * 32767;

		mesh.posa = new Int16Array(vertsX * vertsY * 4);
		mesh.nora = new Int16Array(vertsX * vertsY * 2);
		mesh.texa = new Int16Array(vertsX * vertsY * 2);
		mesh.inda = new Uint32Array((vertsX - 1) * (vertsY - 1) * 6);
		let stepX = sizeX / (vertsX - 1);
		let stepY = sizeY / (vertsY - 1);
		for (let i = 0; i < vertsX * vertsY; ++i) {
			let x = (i % vertsX) * stepX - halfX;
			let y = Math.floor(i / vertsX) * stepY - halfY;
			mesh.posa[i * 4    ] = Math.floor(x * inv);
			mesh.posa[i * 4 + 1] = Math.floor(y * inv);
			mesh.posa[i * 4 + 2] = 0;
			mesh.nora[i * 2    ] = 0;
			mesh.nora[i * 2 + 1] = 0;
			mesh.posa[i * 4 + 3] = 32767;
			x = (i % vertsX) / (vertsX - 1);
			y = 1.0 - Math.floor(i / vertsX) / (vertsY - 1);
			mesh.texa[i * 2    ] = (Math.floor(x * 32767 * uvScale) - 1) % 32767;
			mesh.texa[i * 2 + 1] = (Math.floor(y * 32767 * uvScale) - 1) % 32767;
		}
		for (let i = 0; i < (vertsX - 1) * (vertsY - 1); ++i) {
			let x = i % (vertsX - 1);
			let y = Math.floor(i / (vertsY - 1));
			mesh.inda[i * 6    ] = y * vertsX + x;
			mesh.inda[i * 6 + 1] = y * vertsX + x + 1;
			mesh.inda[i * 6 + 2] = (y + 1) * vertsX + x;
			mesh.inda[i * 6 + 3] = y * vertsX + x + 1;
			mesh.inda[i * 6 + 4] = (y + 1) * vertsX + x + 1;
			mesh.inda[i * 6 + 5] = (y + 1) * vertsX + x;
		}

		return mesh;
	}

	static make_uv_sphere(radius = 1.0, widthSegments = 32, heightSegments = 16, stretchUV = true, uvScale = 1.0): TRawMesh {

		let mesh: TRawMesh = {};
		mesh.scalePos = 1.0;
		mesh.scaleTex = 1.0;
		mesh.name = "";
		mesh.hasNext = false;

		// Pack positions to (-1, 1) range
		mesh.scalePos = radius;
		mesh.scaleTex = uvScale;
		let inv = (1 / mesh.scalePos) * 32767;
		let pi2 = Math.PI * 2;

		let widthVerts = widthSegments + 1;
		let heightVerts = heightSegments + 1;
		mesh.posa = new Int16Array(widthVerts * heightVerts * 4);
		mesh.nora = new Int16Array(widthVerts * heightVerts * 2);
		mesh.texa = new Int16Array(widthVerts * heightVerts * 2);
		mesh.inda = new Uint32Array(widthSegments * heightSegments * 6 - widthSegments * 6);

		let nor = vec4_create();
		let pos = 0;
		for (let y = 0; y < heightVerts; ++y) {
			let v = y / heightSegments;
			let vFlip = 1.0 - v;
			if (!stretchUV) vFlip /= 2;
			let uOff = y == 0 ? 0.5 / widthSegments : y == heightSegments ? -0.5 / widthSegments : 0.0;
			for (let x = 0; x < widthVerts; ++x) {
				let u = x / widthSegments;
				let uPI2 = u * pi2;
				let vPI  = v * Math.PI;
				let vPIsin = Math.sin(vPI);
				let vx = -radius * Math.cos(uPI2) * vPIsin;
				let vy =  radius * Math.sin(uPI2) * vPIsin;
				let vz = -radius * Math.cos(vPI);
				let i4 = pos * 4;
				let i2 = pos * 2;
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
		let heightSegments1 = heightSegments - 1;
		for (let y = 0; y < heightSegments; ++y) {
			for (let x = 0; x < widthSegments; ++x) {
				let x1 = x + 1;
				let y1 = y + 1;
				let a = y  * widthVerts + x1;
				let b = y  * widthVerts + x;
				let c = y1 * widthVerts + x;
				let d = y1 * widthVerts + x1;
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

type TRawMesh = {
	posa?: Int16Array;
	nora?: Int16Array;
	texa?: Int16Array;
	inda?: Uint32Array;
	scalePos?: f32;
	scaleTex?:f32;
	name?: string;
	hasNext?: bool;
}
