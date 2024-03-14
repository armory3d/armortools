
class PhotoToPBRNode extends LogicNode {

	static temp: image_t = null;
	static images: image_t[] = null;
	static model_names = ["base", "occlusion", "roughness", "metallic", "normal", "height"];

	static cached_source: image_t = null;
	static border_w = 64;
	static tile_w = 2048;
	static tile_with_border_w = PhotoToPBRNode.tile_w + PhotoToPBRNode.border_w * 2;

	constructor() {
		super();

		if (PhotoToPBRNode.temp == null) {
			PhotoToPBRNode.temp = image_create_render_target(PhotoToPBRNode.tile_with_border_w, PhotoToPBRNode.tile_with_border_w);
		}

		PhotoToPBRNode.init();
	}

	static init = () => {
		if (PhotoToPBRNode.images == null) {
			PhotoToPBRNode.images = [];
			for (let i = 0; i < PhotoToPBRNode.model_names.length; ++i) {
				PhotoToPBRNode.images.push(image_create_render_target(config_get_texture_res_x(), config_get_texture_res_y()));
			}
		}
	}

	override get_as_image = (from: i32, done: (img: image_t)=>void) => {
		let getSource = (done: (img: image_t)=>void) => {
			if (PhotoToPBRNode.cached_source != null) done(PhotoToPBRNode.cached_source);
			else this.inputs[0].get_as_image(done);
		}

		getSource((source: image_t) => {
			PhotoToPBRNode.cached_source = source;

			console_progress(tr("Processing") + " - " + tr("Photo to PBR"));
			base_notify_on_next_frame(() => {
				let tile_floats: Float32Array[] = [];
				let tiles_x = math_floor(config_get_texture_res_x() / PhotoToPBRNode.tile_w);
				let tiles_y = math_floor(config_get_texture_res_y() / PhotoToPBRNode.tile_w);
				let num_tiles = tiles_x * tiles_y;
				for (let i = 0; i < num_tiles; ++i) {
					let x = i % tiles_x;
					let y = math_floor(i / tiles_x);

					g2_begin(PhotoToPBRNode.temp);
					g2_draw_scaled_image(source, PhotoToPBRNode.border_w - x * PhotoToPBRNode.tile_w, PhotoToPBRNode.border_w - y * PhotoToPBRNode.tile_w, -config_get_texture_res_x(), config_get_texture_res_y());
					g2_draw_scaled_image(source, PhotoToPBRNode.border_w - x * PhotoToPBRNode.tile_w, PhotoToPBRNode.border_w - y * PhotoToPBRNode.tile_w, config_get_texture_res_x(), -config_get_texture_res_y());
					g2_draw_scaled_image(source, PhotoToPBRNode.border_w - x * PhotoToPBRNode.tile_w, PhotoToPBRNode.border_w - y * PhotoToPBRNode.tile_w, -config_get_texture_res_x(), -config_get_texture_res_y());
					g2_draw_scaled_image(source, PhotoToPBRNode.border_w - x * PhotoToPBRNode.tile_w + PhotoToPBRNode.tile_w, PhotoToPBRNode.border_w - y * PhotoToPBRNode.tile_w + PhotoToPBRNode.tile_w, config_get_texture_res_x(), config_get_texture_res_y());
					g2_draw_scaled_image(source, PhotoToPBRNode.border_w - x * PhotoToPBRNode.tile_w + PhotoToPBRNode.tile_w, PhotoToPBRNode.border_w - y * PhotoToPBRNode.tile_w + PhotoToPBRNode.tile_w, -config_get_texture_res_x(), config_get_texture_res_y());
					g2_draw_scaled_image(source, PhotoToPBRNode.border_w - x * PhotoToPBRNode.tile_w + PhotoToPBRNode.tile_w, PhotoToPBRNode.border_w - y * PhotoToPBRNode.tile_w + PhotoToPBRNode.tile_w, config_get_texture_res_x(), -config_get_texture_res_y());
					g2_draw_scaled_image(source, PhotoToPBRNode.border_w - x * PhotoToPBRNode.tile_w, PhotoToPBRNode.border_w - y * PhotoToPBRNode.tile_w, config_get_texture_res_x(), config_get_texture_res_y());
					g2_end();

					let bytes_img = image_get_pixels(PhotoToPBRNode.temp);
					let u8a = new Uint8Array(bytes_img);
					let f32a = new Float32Array(3 * PhotoToPBRNode.tile_with_border_w * PhotoToPBRNode.tile_with_border_w);
					for (let i = 0; i < (PhotoToPBRNode.tile_with_border_w * PhotoToPBRNode.tile_with_border_w); ++i) {
						f32a[i                                        ] = (u8a[i * 4    ] / 255 - 0.5) / 0.5;
						f32a[i + PhotoToPBRNode.tile_with_border_w * PhotoToPBRNode.tile_with_border_w    ] = (u8a[i * 4 + 1] / 255 - 0.5) / 0.5;
						f32a[i + PhotoToPBRNode.tile_with_border_w * PhotoToPBRNode.tile_with_border_w * 2] = (u8a[i * 4 + 2] / 255 - 0.5) / 0.5;
					}

					let model_blob: ArrayBuffer = data_get_blob("models/photo_to_" + PhotoToPBRNode.model_names[from] + ".quant.onnx");
					let buf = krom_ml_inference(model_blob, [f32a.buffer], null, null, config_raw.gpu_inference);
					let ar = new Float32Array(buf);
					u8a = new Uint8Array(4 * PhotoToPBRNode.tile_w * PhotoToPBRNode.tile_w);
					let offset_g = (from == channel_type_t.BASE_COLOR || from == channel_type_t.NORMAL_MAP) ? PhotoToPBRNode.tile_with_border_w * PhotoToPBRNode.tile_with_border_w : 0;
					let offset_b = (from == channel_type_t.BASE_COLOR || from == channel_type_t.NORMAL_MAP) ? PhotoToPBRNode.tile_with_border_w * PhotoToPBRNode.tile_with_border_w * 2 : 0;
					for (let i = 0; i < (PhotoToPBRNode.tile_w * PhotoToPBRNode.tile_w); ++i) {
						let x = PhotoToPBRNode.border_w + i % PhotoToPBRNode.tile_w;
						let y = PhotoToPBRNode.border_w + math_floor(i / PhotoToPBRNode.tile_w);
						u8a[i * 4    ] = math_floor((ar[y * PhotoToPBRNode.tile_with_border_w + x          ] * 0.5 + 0.5) * 255);
						u8a[i * 4 + 1] = math_floor((ar[y * PhotoToPBRNode.tile_with_border_w + x + offset_g] * 0.5 + 0.5) * 255);
						u8a[i * 4 + 2] = math_floor((ar[y * PhotoToPBRNode.tile_with_border_w + x + offset_b] * 0.5 + 0.5) * 255);
						u8a[i * 4 + 3] = 255;
					}
					tile_floats.push(ar);

					// Use border pixels to blend seams
					if (i > 0) {
						if (x > 0) {
							let ar = tile_floats[i - 1];
							for (let yy = 0; yy < PhotoToPBRNode.tile_w; ++yy) {
								for (let xx = 0; xx < PhotoToPBRNode.border_w; ++xx) {
									let i = yy * PhotoToPBRNode.tile_w + xx;
									let a = u8a[i * 4];
									let b = u8a[i * 4 + 1];
									let c = u8a[i * 4 + 2];

									let aa = math_floor((ar[(PhotoToPBRNode.border_w + yy) * PhotoToPBRNode.tile_with_border_w + PhotoToPBRNode.border_w + PhotoToPBRNode.tile_w + xx          ] * 0.5 + 0.5) * 255);
									let bb = math_floor((ar[(PhotoToPBRNode.border_w + yy) * PhotoToPBRNode.tile_with_border_w + PhotoToPBRNode.border_w + PhotoToPBRNode.tile_w + xx + offset_g] * 0.5 + 0.5) * 255);
									let cc = math_floor((ar[(PhotoToPBRNode.border_w + yy) * PhotoToPBRNode.tile_with_border_w + PhotoToPBRNode.border_w + PhotoToPBRNode.tile_w + xx + offset_b] * 0.5 + 0.5) * 255);

									let f = xx / PhotoToPBRNode.border_w;
									let invf = 1.0 - f;
									a = math_floor(a * f + aa * invf);
									b = math_floor(b * f + bb * invf);
									c = math_floor(c * f + cc * invf);

									u8a[i * 4    ] = a;
									u8a[i * 4 + 1] = b;
									u8a[i * 4 + 2] = c;
								}
							}
						}
						if (y > 0) {
							let ar = tile_floats[i - tiles_x];
							for (let xx = 0; xx < PhotoToPBRNode.tile_w; ++xx) {
								for (let yy = 0; yy < PhotoToPBRNode.border_w; ++yy) {
									let i = yy * PhotoToPBRNode.tile_w + xx;
									let a = u8a[i * 4];
									let b = u8a[i * 4 + 1];
									let c = u8a[i * 4 + 2];

									let aa = math_floor((ar[(PhotoToPBRNode.border_w + PhotoToPBRNode.tile_w + yy) * PhotoToPBRNode.tile_with_border_w + PhotoToPBRNode.border_w + xx          ] * 0.5 + 0.5) * 255);
									let bb = math_floor((ar[(PhotoToPBRNode.border_w + PhotoToPBRNode.tile_w + yy) * PhotoToPBRNode.tile_with_border_w + PhotoToPBRNode.border_w + xx + offset_g] * 0.5 + 0.5) * 255);
									let cc = math_floor((ar[(PhotoToPBRNode.border_w + PhotoToPBRNode.tile_w + yy) * PhotoToPBRNode.tile_with_border_w + PhotoToPBRNode.border_w + xx + offset_b] * 0.5 + 0.5) * 255);

									let f = yy / PhotoToPBRNode.border_w;
									let invf = 1.0 - f;
									a = math_floor(a * f + aa * invf);
									b = math_floor(b * f + bb * invf);
									c = math_floor(c * f + cc * invf);

									u8a[i * 4    ] = a;
									u8a[i * 4 + 1] = b;
									u8a[i * 4 + 2] = c;
								}
							}
						}
					}

					///if (krom_metal || krom_vulkan)
					if (from == channel_type_t.BASE_COLOR) PhotoToPBRNode.bgra_swap(u8a.buffer);
					///end

					let temp2 = image_from_bytes(u8a.buffer, PhotoToPBRNode.tile_w, PhotoToPBRNode.tile_w);
					g2_begin(PhotoToPBRNode.images[from]);
					g2_draw_image(temp2, x * PhotoToPBRNode.tile_w, y * PhotoToPBRNode.tile_w);
					g2_end();
					base_notify_on_next_frame(() => {
						image_unload(temp2);
					});
				}

				done(PhotoToPBRNode.images[from]);
			});
		});
	}

	///if (krom_metal || krom_vulkan)
	static bgra_swap = (buffer: ArrayBuffer) => {
		let u8a = new Uint8Array(buffer);
		for (let i = 0; i < math_floor(buffer.byteLength / 4); ++i) {
			let r = u8a[i * 4];
			u8a[i * 4] = u8a[i * 4 + 2];
			u8a[i * 4 + 2] = r;
		}
		return buffer;
	}
	///end

	static def: zui_node_t = {
		id: 0,
		name: _tr("Photo to PBR"),
		type: "PhotoToPBRNode",
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
				name: _tr("Base Color"),
				type: "RGBA",
				color: 0xffc7c729,
				default_value: new Float32Array([0.0, 0.0, 0.0, 1.0])
			},
			{
				id: 0,
				node_id: 0,
				name: _tr("Occlusion"),
				type: "VALUE",
				color: 0xffa1a1a1,
				default_value: 1.0
			},
			{
				id: 0,
				node_id: 0,
				name: _tr("Roughness"),
				type: "VALUE",
				color: 0xffa1a1a1,
				default_value: 1.0
			},
			{
				id: 0,
				node_id: 0,
				name: _tr("Metallic"),
				type: "VALUE",
				color: 0xffa1a1a1,
				default_value: 0.0
			},
			{
				id: 0,
				node_id: 0,
				name: _tr("Normal Map"),
				type: "VECTOR",
				color: 0xffc7c729,
				default_value: new Float32Array([0.0, 0.0, 0.0, 1.0])
			},
			{
				id: 0,
				node_id: 0,
				name: _tr("Height"),
				type: "VALUE",
				color: 0xffa1a1a1,
				default_value: 1.0
			}
		],
		buttons: []
	};
}
