package arm;

@:expose
class Plugin {

	public static function keep() {}

	public static function test() {
		UITrait.inst.customText = "hello world";
	}
}
