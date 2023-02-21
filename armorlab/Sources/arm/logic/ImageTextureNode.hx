package arm.logic;

import arm.logic.LogicNode;

@:keep
class ImageTextureNode extends LogicNode {

	public var file: String;
	public var color_space: String;

	public function new(tree: LogicTree) {
		super(tree);
	}

	override function getAsImage(from: Int, done: kha.Image->Void) {
		var index = Project.assetNames.indexOf(file);
		var asset = Project.assets[index];
		done(Project.getImage(asset));
	}

	override public function getCachedImage(): kha.Image {
		var image: kha.Image;
		getAsImage(0, function(img: kha.Image) { image = img; });
		return image;
	}
}
