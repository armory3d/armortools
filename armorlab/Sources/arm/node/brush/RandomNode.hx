package arm.node.brush;

@:keep
class RandomNode extends LogicNode {

	public function new(tree: LogicTree) {
		super(tree);
	}

	override function get(from: Int, done: Dynamic->Void) {
		inputs[0].get(function(min: Float) {
			inputs[1].get(function(max: Float) {
				done(min + getFloat() * (max - min));
			});
		});
	}

	static var a: Int;
	static var b: Int;
	static var c: Int;
	static var d = setSeed(352124);

	public static function setSeed(seed: Int): Int {
		d = seed;
		a = 0x36aef51a;
		b = 0x21d4b3eb;
		c = 0xf2517abf;
		// Immediately skip a few possibly poor results the easy way
		for (i in 0...15) {
			getInt();
		}
		return d;
	}

	public static function getSeed(): Int {
		return d;
	}

	// Courtesy of https://github.com/Kode/Kha/blob/main/Sources/kha/math/Random.hx
	public static function getInt(): Int {
		var t = (a + b | 0) + d | 0;
		d = d + 1 | 0;
		a = b ^ b >>> 9;
		b = c + (c << 3) | 0;
		c = c << 21 | c >>> 11;
		c = c + t | 0;
		return t & 0x7fffffff;
	}

	public static function getFloat(): Float {
		return getInt() / 0x7fffffff;
	}
}
