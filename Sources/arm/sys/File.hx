package arm.sys;

import haxe.io.Bytes;
using StringTools;

class File {

	#if krom_windows
	static inline var cmd_dir = "dir /b";
	static inline var cmd_copy = "copy";
	#else
	static inline var cmd_dir = "ls";
	static inline var cmd_copy = "cp";
	#end

	public static function readDirectory(path:String):Array<String> {
		var save = Path.data() + Path.sep + "dir.txt";
		Krom.sysCommand(cmd_dir + ' "' + path + '" > "' + save + '"');
		var str = Bytes.ofData(Krom.loadBlob(save)).toString();
		var ar = str.split("\n");
		var files:Array<String> = [];
		for (file in ar) if (file.length > 0) files.push(file.rtrim());
		return files;
	}

	public static function copy(srcPath:String, dstPath:String) {
		Krom.sysCommand(cmd_copy + ' ' + srcPath + ' ' + dstPath);
	}
}
