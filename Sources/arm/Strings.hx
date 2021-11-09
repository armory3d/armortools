package arm;

class Strings {
	public static function error0(): String { return tr("Error: .arm file expected"); }
	public static function error1(): String { return tr("Error: Unknown asset format"); }
	public static function error2(): String { return tr("Error: Could not locate texture"); }
	public static function error3(): String { return tr("Error: Failed to read mesh data"); }
	// public static function error4(): String { return tr("Error: Mesh has no UVs, generating defaults"); }
	public static function error5(): String { return tr("Error: Check internet connection to access the cloud"); }
	public static function info0(): String { return tr("Info: Asset already imported"); }
}
