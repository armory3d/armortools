package arm.sys;

import haxe.macro.Context;

class BuildMacros {

	#if krom_android
	macro public static function readDirectory(path: String): ExprOf<Array<String>> {
		return Context.makeExpr(sys.FileSystem.readDirectory(path), Context.currentPos());
	}
	#end

	macro public static function date(): ExprOf<String> {
		return Context.makeExpr(Date.now().toString(), Context.currentPos());
	}

	macro public static function sha(): ExprOf<String> {
		var proc = new sys.io.Process("git", ["log", "--pretty=format:'%h'", "-n", "1"]);
		return Context.makeExpr(proc.stdout.readLine(), Context.currentPos());
	}
}
