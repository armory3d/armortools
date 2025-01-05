
function import_blend_material_run(path: string) {
	let b: buffer_t = data_get_blob(path);

	data_delete_blob(path);
}
