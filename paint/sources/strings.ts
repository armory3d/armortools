
function strings_arm_file_expected(): string {
	return tr("Error: .arm file expected");
}

function strings_unknown_asset_format(): string {
	return tr("Error: Unknown asset format");
}

function strings_could_not_locate_texture(): string {
	return tr("Error: Could not locate texture");
}

function strings_failed_to_read_mesh_data(): string {
	return tr("Error: Failed to read mesh data");
}

function strings_check_internet_connection(): string {
	return tr("Error: Check internet connection to access the cloud");
}

function strings_asset_already_imported(): string {
	return tr("Info: Asset already imported");
}

function strings_graphics_api(): string {
	/// if arm_direct3d12
	return "Direct3D12";
	/// elseif arm_metal
	return "Metal";
	/// elseif arm_vulkan
	return "Vulkan";
	/// else
	return "WebGPU";
	/// end
}
