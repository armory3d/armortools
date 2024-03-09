
function strings_error0(): string {
	return tr("Error: .arm file expected");
}

function strings_error1(): string {
	return tr("Error: Unknown asset format");
}

function strings_error2(): string {
	return tr("Error: Could not locate texture");
}

function strings_error3(): string {
	return tr("Error: Failed to read mesh data");
}

function strings_error5(): string {
	return tr("Error: Check internet connection to access the cloud");
}

function strings_info0(): string {
	return tr("Info: Asset already imported");
}

function strings_graphics_api(): string {
	///if krom_direct3d11
	return "Direct3D11";
	///elseif krom_direct3d12
	return "Direct3D12";
	///elseif krom_metal
	return "Metal";
	///elseif krom_vulkan
	return "Vulkan";
	///else
	return "OpenGL";
	///end
}
