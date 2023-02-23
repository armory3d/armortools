package;

class Main {

	public static inline var title = "ArmorPaint";
	public static var version = "0.9";

	public static function main() {
		#if arm_snapshot
		MainBase.embed(["default_material.arm"]);
		#if (kha_direct3d12 || kha_vulkan)
		MainBase.embedRaytrace();
		MainBase.embedRaytraceBake();
		#end
		#else
		MainBase.kickstart();
		#end
	}
}
