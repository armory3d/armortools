package arm.util;

import haxe.macro.Context;

class BuildMacros {

	macro public static function date():ExprOf<String> {
		return Context.makeExpr(Date.now().toString(), Context.currentPos());
	}

	macro public static function sha():ExprOf<String> {
		var proc = new sys.io.Process("git", ["log", "--pretty=format:'%h'", "-n", "1"]);
		return Context.makeExpr(proc.stdout.readLine(), Context.currentPos());
	}
}
