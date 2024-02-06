
class ExportMesh {

	static run = (path: string, paintObjects: mesh_object_t[] = null, applyDisplacement = false) => {
		if (paintObjects == null) paintObjects = Project.paintObjects;
		if (Context.raw.exportMeshFormat == MeshFormat.FormatObj) ExportObj.run(path, paintObjects, applyDisplacement);
		else ExportArm.runMesh(path, paintObjects);
	}
}
