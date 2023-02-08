package arm.node.brush;

@:keep
class ImageTextureNode extends LogicNode {

	public var file: String;
	public var color_space: String;

	public function new(tree: LogicTree) {
		super(tree);
	}

	override function get(from: Int, done: Dynamic->Void) {
		var index = Project.assetNames.indexOf(file);
		var asset = Project.assets[index];
		done(Project.getImage(asset));
	}

	override public function getImage(): kha.Image {
		var image: kha.Image;
		get(0, function(img: kha.Image) { image = img; });
		return image;
	}
}
