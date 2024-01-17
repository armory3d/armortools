
// @:keep
class ImageTextureNode extends LogicNode {

	file: string;
	color_space: string;

	constructor() {
		super();
	}

	override getAsImage = (from: i32, done: (img: Image)=>void) => {
		let index = Project.assetNames.indexOf(file);
		let asset = Project.assets[index];
		done(Project.getImage(asset));
	}

	override getCachedImage = (): Image => {
		let image: Image;
		getAsImage(0, (img: Image) => { image = img; });
		return image;
	}

	static def: zui.Zui.TNode = {
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
				default_value: array_f32([0.0, 0.0, 0.0])
			}
		],
		outputs: [
			{
				id: 0,
				node_id: 0,
				name: _tr("Color"),
				type: "RGBA",
				color: 0xffc7c729,
				default_value: array_f32([0.0, 0.0, 0.0, 1.0])
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
