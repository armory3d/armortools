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

	static var cloud: Map<String, Array<String>> = null;
	static var cloudSizes: Map<String, Int> = null;

	#if krom_android
	static var internal: Map<String, Array<String>> = null; // .apk contents
	#end

	public static function readDirectory(path: String, foldersOnly = false): Array<String> {
		if (path.startsWith("cloud")) {
			if (cloud == null) initCloud();
			var files = cloud.get(path.replace("\\", "/"));
			return files != null ? files : [];
		}
		#if krom_android
		path = path.replace("//", "/");
		if (internal == null) {
			internal = [];
			internal.set("/data/plugins", BuildMacros.readDirectory("krom/data/plugins"));
			internal.set("/data/export_presets", BuildMacros.readDirectory("krom/data/export_presets"));
			internal.set("/data/keymap_presets", BuildMacros.readDirectory("krom/data/keymap_presets"));
			internal.set("/data/locale", BuildMacros.readDirectory("krom/data/locale"));
			internal.set("/data/meshes", BuildMacros.readDirectory("krom/data/meshes"));
			internal.set("/data/themes", BuildMacros.readDirectory("krom/data/themes"));
		}
		if (internal.exists(path)) return internal.get(path);
		#end
		return Krom.readDirectory(path, foldersOnly).split("\n");
	}

	public static function createDirectory(path: String) {
		#if krom_windows
		Krom.sysCommand("mkdir " + path); // -p by default
		#else
		Krom.sysCommand("mkdir -p " + path);
		#end
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

	public static function download(url: String, dstPath: String, size: Null<Int> = null) {
		#if krom_windows
		if (size == null) {
			Krom.sysCommand('powershell -c "Invoke-WebRequest -Uri ' + url + " -OutFile '" + dstPath + "'");
		}
		else {
			Krom.httpRequest(url, size, function(ab: js.lib.ArrayBuffer) {
				if (ab != null) Krom.fileSaveBytes(dstPath, ab);
			});
		}
		#else
		Krom.sysCommand("curl -L " + url + " -o " + dstPath);
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

	public static function cacheCloud(path: String): String {
		var dest = Krom.getFilesLocation() + Path.sep + path;
		if (!File.exists(dest)) {
			var fileDir = dest.substr(0, dest.lastIndexOf(Path.sep));
			File.createDirectory(fileDir);
			#if krom_windows
			path = path.replace("\\", "/");
			#end
			var url = Config.raw.server + "/" + path;
			File.download(url, dest, cloudSizes.get(path));
			if (!File.exists(dest)) {
				Log.error(Strings.error5());
				return null;
			}
		}
		#if krom_darwin
		return dest;
		#else
		return Path.workingDir() + Path.sep + path;
		#end
	}

	static function initCloud() {
		cloud = [];
		cloudSizes = [];
		var files: Array<String> = [];
		var sizes: Array<Int> = [];
		var bytes = File.downloadBytes(Config.raw.server);
		var dataPath = Path.data().startsWith(Path.workingDir()) ? Path.data() : Path.workingDir() + Path.sep + Path.data();
		if (!File.exists(dataPath + Path.sep + "download.bin")) {
			cloud.set("cloud", []);
			Log.error(Strings.error5());
			return;
		}
		for (e in Xml.parse(bytes.toString()).firstElement().elementsNamed("Contents")) {
			for (k in e.elementsNamed("Key")) {
				files.push(k.firstChild().nodeValue);
			}
			for (k in e.elementsNamed("Size")) {
				sizes.push(Std.parseInt(k.firstChild().nodeValue));
			}
		}
		for (file in files) {
			if (Path.isFolder(file)) {
				cloud.set(file.substr(0, file.length - 1), []);
			}
		}
		for (i in 0...files.length) {
			var file = files[i];
			var nested = file.indexOf("/") != file.lastIndexOf("/");
			if (nested) {
				var delim = Path.isFolder(file) ? file.substr(0, file.length - 1).lastIndexOf("/") : file.lastIndexOf("/");
				var parent = file.substr(0, delim);
				var child = Path.isFolder(file) ? file.substring(delim + 1, file.length - 1)  : file.substr(delim + 1);
				cloud.get(parent).push(child);
				if (!Path.isFolder(file)) {
					cloudSizes.set(file, sizes[i]);
				}
			}
		}
	}
}
