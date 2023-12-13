package arm.nodes;

import zui.Zui.Nodes;
import iron.System;
import arm.LogicNode;
import arm.ParserLogic.f32;
import arm.Translator._tr;

@:keep
class ImageTextureNode extends LogicNode {

	public var file: String;
	public var color_space: String;

	public function new() {
		super();
	}

	override function getAsImage(from: Int, done: Image->Void) {
		var index = Project.assetNames.indexOf(file);
		var asset = Project.assets[index];
		done(Project.getImage(asset));
	}

	override public function getCachedImage(): Image {
		var image: Image;
		getAsImage(0, function(img: Image) { image = img; });
		return image;
	}

	public static var def: zui.Zui.TNode = {
		id: 0,
		name: _tr("Image Texture"),
		type: "ImageTextureNode",
		x: 0,
		y: 0,
		color: 0xff4982a0,
		inputs: [
			{
				id: 0,
				node_id: 0,
				name: _tr("Vector"),
				type: "VECTOR",
				color: 0xff6363c7,
				default_value: f32([0.0, 0.0, 0.0])
			}
		],
		outputs: [
			{
				id: 0,
				node_id: 0,
				name: _tr("Color"),
				type: "RGBA",
				color: 0xffc7c729,
				default_value: f32([0.0, 0.0, 0.0, 1.0])
			},
			{
				id: 0,
				node_id: 0,
				name: _tr("Alpha"),
				type: "VALUE",
				color: 0xffa1a1a1,
				default_value: 1.0
			}
		],
		buttons: [
			{
				name: _tr("file"),
				type: "ENUM",
				default_value: 0,
				data: ""
			},
			{
				name: _tr("color_space"),
				type: "ENUM",
				default_value: 0,
				data: ["linear", "srgb"]
			}
		]
	};
}
