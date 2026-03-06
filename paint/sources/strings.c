string_t *strings_arm_file_expected() {
	return tr("Error: .arm file expected", null);
}

string_t *strings_unknown_asset_format() {
	return tr("Error: Unknown asset format", null);
}

string_t *strings_could_not_locate_texture() {
	return tr("Error: Could not locate texture", null);
}

string_t *strings_failed_to_read_mesh_data() {
	return tr("Error: Failed to read mesh data", null);
}

string_t *strings_check_internet_connection() {
	return tr("Error: Check internet connection to access the cloud", null);
}

string_t *strings_asset_already_imported() {
	return tr("Info: Asset already imported", null);
}

string_t *strings_graphics_api() {
	#ifdef IRON_DIRECT3D12
	return "Direct3D12";
	#elif defined(IRON_METAL)
	return "Metal";
	#elif defined(IRON_VULKAN)
	return "Vulkan";
	#else
	return "WebGPU";
	#endif
}
