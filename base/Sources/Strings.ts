
class Strings {
	static error0 = (): string => { return tr("Error: .arm file expected"); }
	static error1 = (): string => { return tr("Error: Unknown asset format"); }
	static error2 = (): string => { return tr("Error: Could not locate texture"); }
	static error3 = (): string => { return tr("Error: Failed to read mesh data"); }
	static error5 = (): string => { return tr("Error: Check internet connection to access the cloud"); }
	static info0 = (): string => { return tr("Info: Asset already imported"); }

	static get graphics_api(): string {
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
}
