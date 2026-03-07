char *strings_arm_file_expected() {
	return tr("Error: .arm file expected", NULL);
}

char *strings_unknown_asset_format() {
	return tr("Error: Unknown asset format", NULL);
}

char *strings_could_not_locate_texture() {
	return tr("Error: Could not locate texture", NULL);
}

char *strings_failed_to_read_mesh_data() {
	return tr("Error: Failed to read mesh data", NULL);
}

char *strings_check_internet_connection() {
	return tr("Error: Check internet connection to access the cloud", NULL);
}

char *strings_asset_already_imported() {
	return tr("Info: Asset already imported", NULL);
}

char *strings_graphics_api() {
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
