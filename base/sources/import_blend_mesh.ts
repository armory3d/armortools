
function import_blend_mesh_run(path: string, replace_existing: bool = true) {
	let b: buffer_t = data_get_blob(path);

	data_delete_blob(path);
}
