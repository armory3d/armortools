
let import_txt = function(path, done) {
	iron.Data.getBlob(path, function(b) {
		var filename = path.split('\\').pop().split('/').pop();
		try {
			arm.UIBox.showMessage(filename, b.toString(), true);
			arm.UIBox.clickToHide = false;
			iron.Data.deleteBlob(path);
		}
		catch(e) {
			console.error(e);
		}
	});
}

let plugin = new arm.Plugin();
let formats = arm.Path.textureFormats;
let importers = arm.Path.textureImporters;
formats.push("txt");
importers.h["txt"] = import_txt;

plugin.delete = function() {
	formats.splice(formats.indexOf("txt"), 1);
	importers.h["txt"] = null;
};
