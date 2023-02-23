package;

class Main {

	public static inline var title = "ArmorLab";
	public static var version = "0.1";

	public static function main() {
		#if arm_snapshot
		MainBase.embed(["placeholder.k"]);
		#if (kha_direct3d12 || kha_vulkan)
		MainBase.embedRaytrace();
		#end
		#else
		MainBase.kickstart();
		#end
	}
}
