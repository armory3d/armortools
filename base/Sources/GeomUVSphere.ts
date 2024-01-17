
class GeomUVSphere {

	posa: Int16Array = null;
	nora: Int16Array = null;
	texa: Int16Array = null;
	inda: Uint32Array = null;
	scalePos = 1.0;
	scaleTex = 1.0;
	name = "";
	hasNext = false;

	constructor(radius = 1.0, widthSegments = 32, heightSegments = 16, stretchUV = true, uvScale = 1.0) {
		// Pack positions to (-1, 1) range
		this.scalePos = radius;
		this.scaleTex = uvScale;
		let inv = (1 / this.scalePos) * 32767;
		let pi2 = Math.PI * 2;

		let widthVerts = widthSegments + 1;
		let heightVerts = heightSegments + 1;
		this.posa = new Int16Array(widthVerts * heightVerts * 4);
		this.nora = new Int16Array(widthVerts * heightVerts * 2);
		this.texa = new Int16Array(widthVerts * heightVerts * 2);
		this.inda = new Uint32Array(widthSegments * heightSegments * 6 - widthSegments * 6);

		let nor = new Vec4();
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
				this.posa[i4    ] = Math.floor(vx * inv);
				this.posa[i4 + 1] = Math.floor(vy * inv);
				this.posa[i4 + 2] = Math.floor(vz * inv);
				nor.set(vx, vy, vz).normalize();
				this.posa[i4 + 3] = Math.floor(nor.z * 32767);
				this.nora[i2    ] = Math.floor(nor.x * 32767);
				this.nora[i2 + 1] = Math.floor(nor.y * 32767);
				this.texa[i2    ] = (Math.floor((u + uOff) * 32767) - 1) % 32767;
				this.texa[i2 + 1] = (Math.floor(vFlip      * 32767) - 1) % 32767;
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
					this.inda[pos++] = a;
					this.inda[pos++] = b;
					this.inda[pos++] = d;
				}
				if (y < heightSegments1) {
					this.inda[pos++] = b;
					this.inda[pos++] = c;
					this.inda[pos++] = d;
				}
			}
		}
	}
}
