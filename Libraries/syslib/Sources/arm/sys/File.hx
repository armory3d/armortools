package arm.sys;

import haxe.io.Bytes;

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
		return Krom.readDirectory(path, foldersOnly).split("\n");
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
		#elseif (krom_android || krom_ios)
		Krom.loadUrl(url);
		#else
		Krom.sysCommand('open "' + url + '"');
		#end
	}

	public static function delete(path: String) {
		Krom.sysCommand(cmd_del + ' "' + path + '"');
	}

	public static function exists(path: String): Bool {
		return Krom.fileExists(path);
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
