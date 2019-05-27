package arm;

import haxe.macro.Context;

class Macro {

	macro public static function buildDate():ExprOf<String> {
		return Context.makeExpr(Date.now().toString(), Context.currentPos());
	}

	macro public static function buildSha():ExprOf<String> {
		var proc = new sys.io.Process('git', ['log', "--pretty=format:'%h'", '-n', '1']);
		try {
			return Context.makeExpr(proc.stdout.readLine(), Context.currentPos());
		}
		catch (e:Dynamic) {
			throw "- use 'git clone --recursive https://github.com/armory3d/armorpaint' to compile armorpaint";
		}
	}
}
