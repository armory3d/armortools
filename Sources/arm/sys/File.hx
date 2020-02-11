package arm.sys;

import haxe.io.Bytes;
using StringTools;

class File {

	#if krom_windows
	static inline var cmd_dir = "dir /b";
	static inline var cmd_dir_nofile = "dir /b /ad";
	static inline var cmd_copy = "copy";
	static inline var cmd_del = "del /f";
	#else
	static inline var cmd_dir = "ls";
	static inline var cmd_dir_nofile = "ls";
	static inline var cmd_copy = "cp";
	static inline var cmd_del = "rm";
	#end

	public static function readDirectory(path: String, foldersOnly = false): Array<String> {
		#if (krom_windows || krom_linux || krom_android)
		return Krom.readDirectory(path, foldersOnly).split("\n");
		#else
		var save = Path.data() + Path.sep + "tmp.txt";
		Krom.sysCommand(foldersOnly ? cmd_dir_nofile : cmd_dir + ' "' + path + '" > "' + save + '"');
		var str = Bytes.ofData(Krom.loadBlob(save)).toString();
		var ar = str.split("\n");
		var files: Array<String> = [];
		for (file in ar) if (file.length > 0) files.push(file.rtrim());
		return files;
		#end
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
		#elseif krom_android
		Krom.loadUrl(url);
		#else
		Krom.sysCommand('open "' + url + '"');
		#end
	}

	public static function delete(path: String) {
		Krom.sysCommand(cmd_del + ' "' + path + '"');
	}

	public static function exists(path: String): Bool {
		var slash = path.replace("\\", "/").lastIndexOf("/");
		var dir = path.substr(0, slash);
		var file = path.substr(slash + 1);
		return readDirectory(dir).indexOf(file) >= 0;
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
		try {
			return Bytes.ofData(Krom.loadBlob(save));
		}
		catch (e: Dynamic) {
			return null;
		}
	}
}
