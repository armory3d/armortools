
class TilingNode extends LogicNode {

	result: Image = null;
	static image: Image = null;
	static prompt = "";
	static strength = 0.5;
	static auto = true;

	constructor() {
		super();
		TilingNode.init();
	}

	static init = () => {
		if (TilingNode.image == null) {
			TilingNode.image = Image.createRenderTarget(Config.getTextureResX(), Config.getTextureResY());
		}
	}

	static buttons = (ui: Zui, nodes: Nodes, node: TNode) => {
		TilingNode.auto = node.buttons[0].default_value == 0 ? false : true;
		if (!TilingNode.auto) {
			TilingNode.strength = ui.slider(Zui.handle("tilingnode_0", {value: TilingNode.strength}), tr("strength"), 0, 1, true);
			TilingNode.prompt = ui.textArea(Zui.handle("tilingnode_1"), Align.Left, true, tr("prompt"), true);
			node.buttons[1].height = 1 + TilingNode.prompt.split("\n").length;
		}
		else node.buttons[1].height = 0;
	}

	override getAsImage = (from: i32, done: (img: Image)=>void) => {
		this.inputs[0].getAsImage((source: Image) => {
			TilingNode.image.g2.begin(false);
			TilingNode.image.g2.drawScaledImage(source, 0, 0, Config.getTextureResX(), Config.getTextureResY());
			TilingNode.image.g2.end();

			Console.progress(tr("Processing") + " - " + tr("Tiling"));
			Base.notifyOnNextFrame(() => {
				let _done = (image: Image) => {
					this.result = image;
					done(image);
				}
				TilingNode.auto ? InpaintNode.texsynthInpaint(TilingNode.image, true, null, _done) : TilingNode.sdTiling(TilingNode.image, -1, _done);
			});
		});
	}

	override getCachedImage = (): Image => {
		return this.result;
	}

	static sdTiling = (image: Image, seed: i32/* = -1*/, done: (img: Image)=>void) => {
		TextToPhotoNode.tiling = false;
		let tile = Image.createRenderTarget(512, 512);
		tile.g2.begin(false);
		tile.g2.drawScaledImage(image, -256, -256, 512, 512);
		tile.g2.drawScaledImage(image, 256, -256, 512, 512);
		tile.g2.drawScaledImage(image, -256, 256, 512, 512);
		tile.g2.drawScaledImage(image, 256, 256, 512, 512);
		tile.g2.end();

		let u8a = new Uint8Array(512 * 512);
		for (let i = 0; i < 512 * 512; ++i) {
			let x = i % 512;
			let y = Math.floor(i / 512);
			let l = y < 256 ? y : (511 - y);
			u8a[i] = (x > 256 - l && x < 256 + l) ? 0 : 255;
		}
		// for (let i = 0; i < 512 * 512; ++i) u8a[i] = 255;
		// for (let x = (256 - 32); x < (256 + 32); ++x) {
		// 	for (let y = 0; y < 512; ++y) {
		// 		u8a[y * 512 + x] = 0;
		// 	}
		// }
		// for (let x = 0; x < 512; ++x) {
		// 	for (let y = (256 - 32); y < 256 + 32); ++y) {
		// 		u8a[y * 512 + x] = 0;
		// 	}
		// }
		let mask = Image.fromBytes(u8a.buffer, 512, 512, TextureFormat.R8);

		InpaintNode.prompt = TilingNode.prompt;
		InpaintNode.strength = TilingNode.strength;
		if (seed >= 0) RandomNode.setSeed(seed);
		InpaintNode.sdInpaint(tile, mask, done);
	}

	static def: TNode = {
		id: 0,
		name: _tr("Tiling"),
		type: "TilingNode",
		x: 0,
		y: 0,
		color: 0xff4982a0,
		inputs: [
			{
				id: 0,
				node_id: 0,
				name: _tr("Color"),
				type: "RGBA",
				color: 0xffc7c729,
				default_value: new Float32Array([0.0, 0.0, 0.0, 1.0])
			}
		],
		outputs: [
			{
				id: 0,
				node_id: 0,
				name: _tr("Color"),
				type: "RGBA",
				color: 0xffc7c729,
				default_value: new Float32Array([0.0, 0.0, 0.0, 1.0])
			}
		],
		buttons: [
			{
				name: _tr("auto"),
				type: "BOOL",
				default_value: true,
				output: 0
			},
			{
				name: "TilingNode.buttons",
				type: "CUSTOM",
				height: 0
			}
		]
	};
}
