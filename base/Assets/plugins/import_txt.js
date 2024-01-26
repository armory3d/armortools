
let import_txt = function(path, done) {
	Data.getBlob(path, function(b) {
		var filename = path.split('\\').pop().split('/').pop();
		try {
			UIBox.showMessage(filename, System.bufferToString(b), true);
			UIBox.clickToHide = false;
			Data.deleteBlob(path);
		}
		catch(e) {
			console.error(e);
		}
	});
}

let plugin = Plugin.create();
let formats = Path.textureFormats;
let importers = Path.textureImporters;
formats.push("txt");
importers.set("txt", import_txt);

plugin.delete = function() {
	formats.splice(formats.indexOf("txt"), 1);
	importers.delete("txt");
};
