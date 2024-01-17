
class ImportFbx {

	static run = (path: string, replaceExisting = true) => {
		Data.getBlob(path, (b: ArrayBuffer) => {

			///if arm_fbx
			ParserFbx.parseTransform = Context.raw.parseTransform;
			ParserFbx.parseVCols = Context.raw.parseVCols;
			let obj = new ParserFbx(b);
			replaceExisting ? ImportMesh.makeMesh(obj, path) : ImportMesh.addMesh(obj);
			while (obj.next()) {
				ImportMesh.addMesh(obj);
			}
			Data.deleteBlob(path);
			///end

		});
	}
}
