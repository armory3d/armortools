package arm.sys;

import haxe.io.Bytes;

class File {

	#if krom_windows
	static inline var cmd_mkdir = "mkdir";
	static inline var cmd_copy = "copy";
	#else
	static inline var cmd_mkdir = "mkdir -p";
	static inline var cmd_copy = "cp";
	#end

	static var cloud: Map<String, Array<String>> = null;
	static var cloudSizes: Map<String, Int> = null;

	#if krom_android
	static var internal: Map<String, Array<String>> = null; // .apk contents
	#end

	public static function readDirectory(path: String, foldersOnly = false): Array<String> {
		if (path.startsWith("cloud")) {
			var files = cloud != null ? cloud.get(path.replace("\\", "/")) : null;
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
		Krom.sysCommand(cmd_mkdir + ' "' + path + '"');
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

	public static function loadUrl(url: String) {
		Krom.loadUrl(url);
	}

	public static function delete(path: String) {
		Krom.deleteFile(path);
	}

	public static function exists(path: String): Bool {
		return Krom.fileExists(path);
	}

	public static function download(url: String, dstPath: String, done: Void->Void, size = 0) {
		#if (krom_windows || krom_darwin || krom_ios || krom_android)
		Krom.httpRequest(url, size, function(ab: js.lib.ArrayBuffer) {
			if (ab != null) Krom.fileSaveBytes(dstPath, ab);
			done();
		});
		#elseif krom_linux
		Krom.sysCommand('wget -O "' + dstPath + '" "' + url + '"');
		done();
		#else
		Krom.sysCommand('curl -L ' + url + ' -o "' + dstPath + '"');
		done();
		#end
	}

	public static function downloadBytes(url: String, done: Bytes->Void) {
		var save = (Path.isProtected() ? Krom.savePath() : Path.data() + Path.sep) + "download.bin";
		File.download(url, save, function() {
			var bytes: Bytes = null;
			try {
				bytes = Bytes.ofData(Krom.loadBlob(save));
			}
			catch (e: Dynamic) {}
			done(bytes);
		});
	}

	public static function cacheCloud(path: String, done: String->Void) {
		#if krom_ios
		var path2 = path.replace("/", "_"); // Cache everything into root folder
		#else
		var path2 = path;
		#end
		var dest = (Path.isProtected() ? Krom.savePath() : Krom.getFilesLocation() + Path.sep) + path2;
		if (File.exists(dest)) {
			#if (krom_darwin || krom_ios)
			done(dest);
			#else
			done((Path.isProtected() ? Krom.savePath() : Path.workingDir() + Path.sep) + path);
			#end
			return;
		}

		var fileDir = dest.substr(0, dest.lastIndexOf(Path.sep));
		if (File.readDirectory(fileDir)[0] == "") {
			File.createDirectory(fileDir);
		}
		#if krom_windows
		path = path.replace("\\", "/");
		#end
		var url = Config.raw.server + "/" + path;
		File.download(url, dest, function() {
			if (!File.exists(dest)) {
				Console.error(Strings.error5());
				done(null);
				return;
			}
			#if (krom_darwin || krom_ios)
			done(dest);
			#else
			done((Path.isProtected() ? Krom.savePath() : Path.workingDir() + Path.sep) + path);
			#end
		}, cloudSizes.get(path));
	}

	static function initCloudBytes(done: Void->Void, append = "") {
		File.downloadBytes(Config.raw.server + "/?list-type=2" + append, function(bytes: Bytes) {
			if (bytes == null) {
				cloud.set("cloud", []);
				Console.error(Strings.error5());
				return;
			}
			var files: Array<String> = [];
			var sizes: Array<Int> = [];
			var xml = Xml.parse(bytes.toString());
			for (e in xml.firstElement().elementsNamed("Contents")) {
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

			var isTruncated = xml.firstElement().elementsNamed("IsTruncated").next().firstChild().nodeValue == "true";
			if (isTruncated) {
				initCloudBytes(done, "&start-after=" + xml.firstElement().elementsNamed("NextContinuationToken").next().firstChild().nodeValue);
			}
			else done();
		});
	}

	static function initCloud(done: Void->Void) {
		cloud = [];
		cloudSizes = [];
		initCloudBytes(done);
	}
}
