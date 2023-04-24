package arm;

class Strings {
	public static function error0(): String { return tr("Error: .arm file expected"); }
	public static function error1(): String { return tr("Error: Unknown asset format"); }
	public static function error2(): String { return tr("Error: Could not locate texture"); }
	public static function error3(): String { return tr("Error: Failed to read mesh data"); }
	public static function error5(): String { return tr("Error: Check internet connection to access the cloud"); }
	public static function info0(): String { return tr("Info: Asset already imported"); }

	#if kha_direct3d11
	public static inline var graphics_api = "Direct3D11";
	#elseif kha_direct3d12
	public static inline var graphics_api = "Direct3D12";
	#elseif kha_metal
	public static inline var graphics_api = "Metal";
	#elseif kha_vulkan
	public static inline var graphics_api = "Vulkan";
	#else
	public static inline var graphics_api = "OpenGL";
	#end
}
