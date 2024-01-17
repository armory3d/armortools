
class GeomPlane {

	posa: Int16Array = null;
	nora: Int16Array = null;
	texa: Int16Array = null;
	inda: Uint32Array = null;
	scalePos = 1.0;
	scaleTex = 1.0;
	name = "";
	hasNext = false;

	constructor(sizeX = 1.0, sizeY = 1.0, vertsX = 2, vertsY = 2, uvScale = 1.0) {
		// Pack positions to (-1, 1) range
		let halfX = sizeX / 2;
		let halfY = sizeY / 2;
		this.scalePos = Math.max(halfX, halfY);
		let inv = (1 / this.scalePos) * 32767;

		this.posa = new Int16Array(vertsX * vertsY * 4);
		this.nora = new Int16Array(vertsX * vertsY * 2);
		this.texa = new Int16Array(vertsX * vertsY * 2);
		this.inda = new Uint32Array((vertsX - 1) * (vertsY - 1) * 6);
		let stepX = sizeX / (vertsX - 1);
		let stepY = sizeY / (vertsY - 1);
		for (let i = 0; i < vertsX * vertsY; ++i) {
			let x = (i % vertsX) * stepX - halfX;
			let y = Math.floor(i / vertsX) * stepY - halfY;
			this.posa[i * 4    ] = Math.floor(x * inv);
			this.posa[i * 4 + 1] = Math.floor(y * inv);
			this.posa[i * 4 + 2] = 0;
			this.nora[i * 2    ] = 0;
			this.nora[i * 2 + 1] = 0;
			this.posa[i * 4 + 3] = 32767;
			x = (i % vertsX) / (vertsX - 1);
			y = 1.0 - Math.floor(i / vertsX) / (vertsY - 1);
			this.texa[i * 2    ] = (Math.floor(x * 32767 * uvScale) - 1) % 32767;
			this.texa[i * 2 + 1] = (Math.floor(y * 32767 * uvScale) - 1) % 32767;
		}
		for (let i = 0; i < (vertsX - 1) * (vertsY - 1); ++i) {
			let x = i % (vertsX - 1);
			let y = Math.floor(i / (vertsY - 1));
			this.inda[i * 6    ] = y * vertsX + x;
			this.inda[i * 6 + 1] = y * vertsX + x + 1;
			this.inda[i * 6 + 2] = (y + 1) * vertsX + x;
			this.inda[i * 6 + 3] = y * vertsX + x + 1;
			this.inda[i * 6 + 4] = (y + 1) * vertsX + x + 1;
			this.inda[i * 6 + 5] = (y + 1) * vertsX + x;
		}
	}
}
