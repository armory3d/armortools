package arm.logic;

import zui.Zui.Nodes;
import zui.Zui.TNode;
import arm.logic.LogicNode;
import arm.Translator._tr;

@:keep
class RandomNode extends LogicNode {

	public function new() {
		super();
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

	public static var def: TNode = {
		id: 0,
		name: _tr("Random"),
		type: "RandomNode",
		x: 0,
		y: 0,
		color: 0xffb34f5a,
		inputs: [
			{
				id: 0,
				node_id: 0,
				name: _tr("Min"),
				type: "VALUE",
				color: 0xffa1a1a1,
				default_value: 0.0
			},
			{
				id: 0,
				node_id: 0,
				name: _tr("Max"),
				type: "VALUE",
				color: 0xffa1a1a1,
				default_value: 1.0
			}
		],
		outputs: [
			{
				id: 0,
				node_id: 0,
				name: _tr("Value"),
				type: "VALUE",
				color: 0xffa1a1a1,
				default_value: 0.5
			}
		],
		buttons: []
	};
}
