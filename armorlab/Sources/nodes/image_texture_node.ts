
type image_texture_node_t = {
	base?: logic_node_t;
	file?: string;
	color_space?: string;
};

function image_texture_node_create(): image_texture_node_t {
	let n: image_texture_node_t = {};
	n.base = logic_node_create();
	n.base.get_as_image = image_texture_node_get_as_image;
	n.base.get_cached_image = image_texture_node_get_cached_image;
	return n;
}

function image_texture_node_get_as_image(self: image_texture_node_t, from: i32, done: (img: image_t)=>void) {
	let index = array_index_of(project_asset_names, self.file);
	let asset = project_assets[index];
	done(project_get_image(asset));
}

function image_texture_node_get_cached_image(self: image_texture_node_t): image_t {
	let image: image_t;
	self.base.get_as_image(self, 0, function (img: image_t) {
		image = img;
	});
	return image;
}

let image_texture_node_def: zui_node_t = {
	id: 0,
	name: _tr("Image Texture"),
	type: "image_texture_node",
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
			default_value: new f32_array_t([0.0, 0.0, 0.0])
		}
	],
	outputs: [
		{
			id: 0,
			node_id: 0,
			name: _tr("Color"),
			type: "RGBA",
			color: 0xffc7c729,
			default_value: new f32_array_t([0.0, 0.0, 0.0, 1.0])
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
