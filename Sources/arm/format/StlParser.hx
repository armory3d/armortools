package arm.format;

class StlParser {

	public var posa:kha.arrays.Int16Array = null;
	public var nora:kha.arrays.Int16Array = null;
	public var texa:kha.arrays.Int16Array = null;
	public var inda:kha.arrays.Uint32Array = null;
	
	public var scalePos = 1.0;
	public var scaleTex = 1.0;
	public var name = "";
	public var hasNext = false;
	public var pos = 0;

	public function new(blob:kha.Blob) {
		var bytes = blob.bytes;
		var input = new haxe.io.BytesInput(bytes);
		var header = input.read(80);
		if (header.getString(0, 5) == "solid") {
			return; // ascii not supported
		}
		var faces = input.readInt32();
		var posTemp = new kha.arrays.Float32Array(faces * 9);
		var norTemp = new kha.arrays.Float32Array(faces * 3);
		for (i in 0...faces) {
			var i3 = i * 3;
			norTemp[i3    ] = input.readFloat();
			norTemp[i3 + 1] = input.readFloat();
			norTemp[i3 + 2] = input.readFloat();
			var i9 = i * 9;
			posTemp[i9    ] = input.readFloat();
			posTemp[i9 + 1] = input.readFloat();
			posTemp[i9 + 2] = input.readFloat();
			posTemp[i9 + 3] = input.readFloat();
			posTemp[i9 + 4] = input.readFloat();
			posTemp[i9 + 5] = input.readFloat();
			posTemp[i9 + 6] = input.readFloat();
			posTemp[i9 + 7] = input.readFloat();
			posTemp[i9 + 8] = input.readFloat();
			input.readInt16(); // attribute
		}

		scalePos = 0.0;
		for (i in 0...posTemp.length) {
			var f = Math.abs(posTemp[i]);
			if (scalePos < f) scalePos = f;
		}
		var inv = 32767 * (1 / scalePos);

		var verts = Std.int(posTemp.length / 3);
		posa = new kha.arrays.Int16Array(verts * 4);
		nora = new kha.arrays.Int16Array(verts * 2);
		inda = new kha.arrays.Uint32Array(verts);
		for (i in 0...verts) {
			posa[i * 4    ] = Std.int( posTemp[i * 3    ] * inv);
			posa[i * 4 + 1] = Std.int(-posTemp[i * 3 + 2] * inv);
			posa[i * 4 + 2] = Std.int( posTemp[i * 3 + 1] * inv);
			nora[i * 2    ] = Std.int( norTemp[i    ] * 32767);
			nora[i * 2 + 1] = Std.int(-norTemp[i + 2] * 32767);
			posa[i * 4 + 3] = Std.int( norTemp[i + 1] * 32767);
			inda[i] = i;
		}
	}
}
