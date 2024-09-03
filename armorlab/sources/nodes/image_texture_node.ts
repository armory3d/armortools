
type image_texture_node_t = {
	base?: logic_node_t;
	file?: string;
	color_space?: string;
};

function image_texture_node_create(arg: any): image_texture_node_t {
	let n: image_texture_node_t = {};
	n.base = logic_node_create();
	n.base.get_as_image = image_texture_node_get_as_image;
	n.base.get_cached_image = image_texture_node_get_cached_image;
	return n;
}

function image_texture_node_get_as_image(self: image_texture_node_t, from: i32): image_t {
	let index: i32 = array_index_of(project_asset_names, self.file);
	let asset: asset_t = project_assets[index];
	return project_get_image(asset);
}

function image_texture_node_get_cached_image(self: image_texture_node_t): image_t {
	let image: image_t = self.base.get_as_image(self, 0);
	return image;
}

let image_texture_node_def: ui_node_t = {
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
			default_value: f32_array_create_xyz(0.0, 0.0, 0.0),
			min: 0.0,
			max: 1.0,
			precision: 100,
			display: 0
		}
	],
	outputs: [
		{
			id: 0,
			node_id: 0,
			name: _tr("Color"),
			type: "RGBA",
			color: 0xffc7c729,
			default_value: f32_array_create_xyzw(0.0, 0.0, 0.0, 1.0),
			min: 0.0,
			max: 1.0,
			precision: 100,
			display: 0
		},
		{
			id: 0,
			node_id: 0,
			name: _tr("Alpha"),
			type: "VALUE",
			color: 0xffa1a1a1,
			default_value: f32_array_create_x(1.0),
			min: 0.0,
			max: 1.0,
			precision: 100,
			display: 0
		}
	],
	buttons: [
		{
			name: _tr("file"),
			type: "ENUM",
			output: -1,
			default_value: f32_array_create_x(0),
			data: u8_array_create_from_string(""),
			min: 0.0,
			max: 1.0,
			precision: 100,
			height: 0
		},
		{
			name: _tr("color_space"),
			type: "ENUM",
			output: -1,
			default_value: f32_array_create_x(0),
			data: u8_array_create_from_string("linear\nsrgb"),
			min: 0.0,
			max: 1.0,
			precision: 100,
			height: 0
		}
	],
	width: 0
};
