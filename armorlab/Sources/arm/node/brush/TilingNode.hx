package arm.node.brush;

@:keep
class TilingNode extends LogicNode {

	static var image: kha.Image = null;
	var result: kha.Image = null;
	static var prompt = "";
	static var strength = 0.5;
	static var auto = true;

	public function new(tree: LogicTree) {
		super(tree);

		init();
	}

	public static function init() {
		if (image == null) {
			image = kha.Image.createRenderTarget(Config.getTextureResX(), Config.getTextureResY());
		}
	}

	public static function buttons(ui: zui.Zui, nodes: zui.Nodes, node: zui.Nodes.TNode) {
		auto = node.buttons[0].default_value;
		if (!auto) {
			strength = ui.slider(zui.Id.handle({value: strength}), tr("strength"), 0, 1, true);
			prompt = zui.Ext.textArea(ui, zui.Id.handle(), true, tr("prompt"), true);
			node.buttons[1].height = 1 + prompt.split("\n").length;
		}
		else node.buttons[1].height = 0;
	}

	override function get(from: Int, done: Dynamic->Void) {
		inputs[0].get(function(source: Dynamic) {
			if (!Std.isOfType(source, kha.Image)) { done(null); return; }

			image.g2.begin(false);
			image.g2.drawScaledImage(source, 0, 0, Config.getTextureResX(), Config.getTextureResY());
			image.g2.end();

			result = auto ? InpaintNode.texsynthInpaint(image, true) : sdTiling(image);
			done(result);
		});
	}

	override public function getImage(): kha.Image {
		return result;
	}

	public static function sdTiling(image: kha.Image, seed = -1): kha.Image {
		@:privateAccess TextToPhotoNode.tiling = false;
		var tile = kha.Image.createRenderTarget(512, 512);
		tile.g2.begin(false);
		tile.g2.drawScaledImage(image, -256, -256, 512, 512);
		tile.g2.drawScaledImage(image, 256, -256, 512, 512);
		tile.g2.drawScaledImage(image, -256, 256, 512, 512);
		tile.g2.drawScaledImage(image, 256, 256, 512, 512);
		tile.g2.end();

		var bytes = haxe.io.Bytes.alloc(512 * 512);
		for (i in 0...512 * 512) {
			var x = i % 512;
			var y = Std.int(i / 512);
			var l = y < 256 ? y : (511 - y);
			bytes.set(i, (x > 256 - l && x < 256 + l) ? 0 : 255);
		}
		// for (i in 0...512 * 512) bytes.set(i, 255);
		// for (x in (256 - 32)...(256 + 32)) {
		// 	for (y in 0...512) {
		// 		bytes.set(y * 512 + x, 0);
		// 	}
		// }
		// for (x in 0...512) {
		// 	for (y in (256 - 32)...(256 + 32)) {
		// 		bytes.set(y * 512 + x, 0);
		// 	}
		// }
		var mask = kha.Image.fromBytes(bytes, 512, 512, kha.graphics4.TextureFormat.L8);

		@:privateAccess InpaintNode.prompt = prompt;
		@:privateAccess InpaintNode.strength = strength;
		if (seed >= 0) RandomNode.setSeed(seed);
		return InpaintNode.sdInpaint(tile, mask);
	}
}
