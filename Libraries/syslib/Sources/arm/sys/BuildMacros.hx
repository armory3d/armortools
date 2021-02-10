package arm.sys;

import haxe.macro.Context;

class BuildMacros {

	#if krom_android
	macro public static function readDirectory(path: String): ExprOf<Array<String>> {
		return Context.makeExpr(sys.FileSystem.readDirectory(path), Context.currentPos());
	}
	#end
}
