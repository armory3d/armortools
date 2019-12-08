package arm.sys;

import haxe.io.Bytes;
using StringTools;

class File {

	#if krom_windows
	static inline var cmd_dir = "dir /b";
	static inline var cmd_copy = "copy";
	static inline var cmd_del = "del /f";
	#else
	static inline var cmd_dir = "ls";
	static inline var cmd_copy = "cp";
	static inline var cmd_del = "rm";
	#end

	public static function readDirectory(path: String): Array<String> {
		var save = Path.data() + Path.sep + "dir.txt";
		Krom.sysCommand(cmd_dir + ' "' + path + '" > "' + save + '"');
		var str = Bytes.ofData(Krom.loadBlob(save)).toString();
		var ar = str.split("\n");
		var files: Array<String> = [];
		for (file in ar) if (file.length > 0) files.push(file.rtrim());
		return files;
	}

	public static function createDirectory(path: String) {
		Krom.sysCommand("mkdir " + path);
	}

	public static function copy(srcPath: String, dstPath: String) {
		Krom.sysCommand(cmd_copy + ' "' + srcPath + '" "' + dstPath + '"');
	}

	public static function start(path: String) {
		#if krom_windows
		Krom.sysCommand('start "" "' + path + '"');
		#elseif krom_linux
		Krom.sysCommand('xdg-open "' + path + '"');
		#else
		Krom.sysCommand('open "' + path + '"');
		#end
	}

	public static function explorer(url: String) {
		#if krom_windows
		Krom.sysCommand('explorer "' + url + '"');
		#elseif krom_linux
		Krom.sysCommand('xdg-open "' + url + '"');
		#else
		Krom.sysCommand('open "' + url + '"');
		#end
	}

	public static function delete(path: String) {
		Krom.sysCommand(cmd_del + ' "' + path + '"');
	}

	public static function exists(path: String): Bool {
		#if krom_windows
		var exists = Krom.sysCommand('IF EXIST "' + path + '" EXIT /b 1');
		#else
		var exists = 1;
		// { test -e file && echo 1 || echo 0 }
		#end
		return exists == 1;
	}

	public static function download(url: String, dstPath: String) {
		#if krom_windows
		Krom.sysCommand('powershell -c "Invoke-WebRequest -Uri ' + url + " -OutFile '" + dstPath + "'");
		#else
		Krom.sysCommand("curl " + url + " -o " + dstPath);
		#end
	}

	public static function downloadBytes(url: String): Bytes {
		var save = Path.data() + Path.sep + "download.bin";
		download(url, save);
		return Bytes.ofData(Krom.loadBlob(save));
	}
}
