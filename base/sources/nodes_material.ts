
let nodes_material_categories: string[] = [
	_tr("Input"),
	_tr("Texture"),
	_tr("Color"),
	_tr("Vector"),
	_tr("Converter"),
	_tr("Group")
];

let nodes_material_input: ui_node_t[] = [
	{
		id: 0,
		name: _tr("Attribute"),
		type: "ATTRIBUTE",
		x: 0,
		y: 0,
		color: 0xffb34f5a,
		inputs: [],
		outputs: [
			{
				id: 0,
				node_id: 0,
				name: _tr("Color"),
				type: "RGBA",
				color: 0xffc7c729,
				default_value: f32_array_create_xyzw(0.8, 0.8, 0.8, 1.0),
				min: 0.0,
				max: 1.0,
				precision: 100,
				display: 0
			},
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
			},
			{
				id: 0,
				node_id: 0,
				name: _tr("Fac"),
				type: "VALUE",
				color: 0xffa1a1a1,
				default_value: f32_array_create_x(0.0),
				min: 0.0,
				max: 1.0,
				precision: 100,
				display: 0
			}
		],
		buttons: [
			{
				name: _tr("Name"),
				type: "STRING",
				output: -1,
				default_value: f32_array_create_x(0),
				data: null,
				min: 0.0,
				max: 1.0,
				precision: 100,
				height: 0
			}
		],
		width: 0
	},
	{
		id: 0,
		name: _tr("Camera Data"),
		type: "CAMERA",
		x: 0,
		y: 0,
		color: 0xffb34f5a,
		inputs: [],
		outputs: [
			{
				id: 0,
				node_id: 0,
				name: _tr("View Vector"),
				type: "VECTOR",
				color: 0xff6363c7,
				default_value: f32_array_create_xyz(0.0, 0.0, 0.0),
				min: 0.0,
				max: 1.0,
				precision: 100,
				display: 0
			},
			{
				id: 0,
				node_id: 0,
				name: _tr("View Z Depth"),
				type: "VALUE",
				color: 0xffa1a1a1,
				default_value: f32_array_create_x(0.0),
				min: 0.0,
				max: 1.0,
				precision: 100,
				display: 0
			},
			{
				id: 0,
				node_id: 0,
				name: _tr("View Distance"),
				type: "VALUE",
				color: 0xffa1a1a1,
				default_value: f32_array_create_x(0.0),
				min: 0.0,
				max: 1.0,
				precision: 100,
				display: 0
			}
		],
		buttons: [],
		width: 0
	},
	{
		id: 0,
		name: _tr("Fresnel"),
		type: "FRESNEL",
		x: 0,
		y: 0,
		color: 0xffb34f5a,
		inputs: [
			{
				id: 0,
				node_id: 0,
				name: _tr("IOR"),
				type: "VALUE",
				color: 0xffa1a1a1,
				default_value: f32_array_create_x(1.45),
				min: 0.0,
				max: 3.0,
				precision: 100,
				display: 0
			},
			{
				id: 0,
				node_id: 0,
				name: _tr("Normal"),
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
				name: _tr("Fac"),
				type: "VALUE",
				color: 0xffa1a1a1,
				default_value: f32_array_create_x(0.0),
				min: 0.0,
				max: 1.0,
				precision: 100,
				display: 0
			}
		],
		buttons: [],
		width: 0
	},
	{
		id: 0,
		name: _tr("Geometry"),
		type: "NEW_GEOMETRY",
		x: 0,
		y: 0,
		color: 0xffb34f5a,
		inputs: [],
		outputs: [
			{
				id: 0,
				node_id: 0,
				name: _tr("Position"),
				type: "VECTOR",
				color: 0xff6363c7,
				default_value: f32_array_create_xyz(0.0, 0.0, 0.0),
				min: 0.0,
				max: 1.0,
				precision: 100,
				display: 0
			},
			{
				id: 0,
				node_id: 0,
				name: _tr("Normal"),
				type: "VECTOR",
				color: 0xff6363c7,
				default_value: f32_array_create_xyz(0.0, 0.0, 0.0),
				min: 0.0,
				max: 1.0,
				precision: 100,
				display: 0
			},
			{
				id: 0,
				node_id: 0,
				name: _tr("Tangent"),
				type: "VECTOR",
				color: 0xff6363c7,
				default_value: f32_array_create_xyz(0.0, 0.0, 0.0),
				min: 0.0,
				max: 1.0,
				precision: 100,
				display: 0
			},
			{
				id: 0,
				node_id: 0,
				name: _tr("True Normal"),
				type: "VECTOR",
				color: 0xff6363c7,
				default_value: f32_array_create_xyz(0.0, 0.0, 0.0),
				min: 0.0,
				max: 1.0,
				precision: 100,
				display: 0
			},
			{
				id: 0,
				node_id: 0,
				name: _tr("Incoming"),
				type: "VECTOR",
				color: 0xff6363c7,
				default_value: f32_array_create_xyz(0.0, 0.0, 0.0),
				min: 0.0,
				max: 1.0,
				precision: 100,
				display: 0
			},
			{
				id: 0,
				node_id: 0,
				name: _tr("Parametric"),
				type: "VECTOR",
				color: 0xff6363c7,
				default_value: f32_array_create_xyz(0.0, 0.0, 0.0),
				min: 0.0,
				max: 1.0,
				precision: 100,
				display: 0
			},
			{
				id: 0,
				node_id: 0,
				name: _tr("Backfacing"),
				type: "VALUE",
				color: 0xffa1a1a1,
				default_value: f32_array_create_x(0.0),
				min: 0.0,
				max: 1.0,
				precision: 100,
				display: 0
			},
			{
				id: 0,
				node_id: 0,
				name: _tr("Pointiness"),
				type: "VALUE",
				color: 0xffa1a1a1,
				default_value: f32_array_create_x(0.0),
				min: 0.0,
				max: 1.0,
				precision: 100,
				display: 0
			},
			{
				id: 0,
				node_id: 0,
				name: _tr("Random Per Island"),
				type: "VALUE",
				color: 0xffa1a1a1,
				default_value: f32_array_create_x(0.0),
				min: 0.0,
				max: 1.0,
				precision: 100,
				display: 0
			}
		],
		buttons: [],
		width: 0
	},
	{
		id: 0,
		name: _tr("Layer"),
		type: "LAYER", // extension
		x: 0,
		y: 0,
		color: 0xff4982a0,
		inputs: [],
		outputs: [
			{
				id: 0,
				node_id: 0,
				name: _tr("Base Color"),
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
				name: _tr("Opacity"),
				type: "VALUE",
				color: 0xffa1a1a1,
				default_value: f32_array_create_x(1.0),
				min: 0.0,
				max: 1.0,
				precision: 100,
				display: 0
			},
			{
				id: 0,
				node_id: 0,
				name: _tr("Occlusion"),
				type: "VALUE",
				color: 0xffa1a1a1,
				default_value: f32_array_create_x(1.0),
				min: 0.0,
				max: 1.0,
				precision: 100,
				display: 0
			},
			{
				id: 0,
				node_id: 0,
				name: _tr("Roughness"),
				type: "VALUE",
				color: 0xffa1a1a1,
				default_value: f32_array_create_x(1.0),
				min: 0.0,
				max: 1.0,
				precision: 100,
				display: 0
			},
			{
				id: 0,
				node_id: 0,
				name: _tr("Metallic"),
				type: "VALUE",
				color: 0xffa1a1a1,
				default_value: f32_array_create_x(1.0),
				min: 0.0,
				max: 1.0,
				precision: 100,
				display: 0
			},
			{
				id: 0,
				node_id: 0,
				name: _tr("Normal Map"),
				type: "VECTOR",
				color: -10238109,
				default_value: f32_array_create_xyz(0.5, 0.5, 1.0),
				min: 0.0,
				max: 1.0,
				precision: 100,
				display: 0
			},
			{
				id: 0,
				node_id: 0,
				name: _tr("Emission"),
				type: "VALUE",
				color: 0xffa1a1a1,
				default_value: f32_array_create_x(1.0),
				min: 0.0,
				max: 1.0,
				precision: 100,
				display: 0
			},
			{
				id: 0,
				node_id: 0,
				name: _tr("Height"),
				type: "VALUE",
				color: 0xffa1a1a1,
				default_value: f32_array_create_x(1.0),
				min: 0.0,
				max: 1.0,
				precision: 100,
				display: 0
			},
			{
				id: 0,
				node_id: 0,
				name: _tr("Subsurface"),
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
				name: _tr("Layer"),
				type: "ENUM",
				output: -1,
				default_value: f32_array_create_x(0),
				data: u8_array_create_from_string(""),
				min: 0.0,
				max: 1.0,
				precision: 100,
				height: 0
			}
		],
		width: 0
	},
	{
		id: 0,
		name: _tr("Layer Mask"),
		type: "LAYER_MASK", // extension
		x: 0,
		y: 0,
		color: 0xff4982a0,
		inputs: [],
		outputs: [
			{
				id: 0,
				node_id: 0,
				name: _tr("Value"),
				type: "VALUE",
				color: 0xffa1a1a1,
				default_value: f32_array_create_x(0.0),
				min: 0.0,
				max: 1.0,
				precision: 100,
				display: 0
			}
		],
		buttons: [
			{
				name: _tr("Layer"),
				type: "ENUM",
				output: -1,
				default_value: f32_array_create_x(0),
				data: u8_array_create_from_string(""),
				min: 0.0,
				max: 1.0,
				precision: 100,
				height: 0
			}
		],
		width: 0
	},
	{
		id: 0,
		name: _tr("Layer Weight"),
		type: "LAYER_WEIGHT",
		x: 0,
		y: 0,
		color: 0xffb34f5a,
		inputs: [
			{
				id: 0,
				node_id: 0,
				name: _tr("Blend"),
				type: "VALUE",
				color: 0xffa1a1a1,
				default_value: f32_array_create_x(0.5),
				min: 0.0,
				max: 1.0,
				precision: 100,
				display: 0
			},
			{
				id: 0,
				node_id: 0,
				name: _tr("Normal"),
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
				name: _tr("Fresnel"),
				type: "VALUE",
				color: 0xffa1a1a1,
				default_value: f32_array_create_x(0.0),
				min: 0.0,
				max: 1.0,
				precision: 100,
				display: 0
			},
			{
				id: 0,
				node_id: 0,
				name: _tr("Facing"),
				type: "VALUE",
				color: 0xffa1a1a1,
				default_value: f32_array_create_x(0.0),
				min: 0.0,
				max: 1.0,
				precision: 100,
				display: 0
			}
		],
		buttons: [],
		width: 0
	},
	{
		id: 0,
		name: _tr("Material"),
		type: "MATERIAL", // extension
		x: 0,
		y: 0,
		color: 0xff4982a0,
		inputs: [],
		outputs: [
			{
				id: 0,
				node_id: 0,
				name: _tr("Base Color"),
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
				name: _tr("Opacity"),
				type: "VALUE",
				color: 0xffa1a1a1,
				default_value: f32_array_create_x(1.0),
				min: 0.0,
				max: 1.0,
				precision: 100,
				display: 0
			},
			{
				id: 0,
				node_id: 0,
				name: _tr("Occlusion"),
				type: "VALUE",
				color: 0xffa1a1a1,
				default_value: f32_array_create_x(1.0),
				min: 0.0,
				max: 1.0,
				precision: 100,
				display: 0
			},
			{
				id: 0,
				node_id: 0,
				name: _tr("Roughness"),
				type: "VALUE",
				color: 0xffa1a1a1,
				default_value: f32_array_create_x(1.0),
				min: 0.0,
				max: 1.0,
				precision: 100,
				display: 0
			},
			{
				id: 0,
				node_id: 0,
				name: _tr("Metallic"),
				type: "VALUE",
				color: 0xffa1a1a1,
				default_value: f32_array_create_x(1.0),
				min: 0.0,
				max: 1.0,
				precision: 100,
				display: 0
			},
			{
				id: 0,
				node_id: 0,
				name: _tr("Normal Map"),
				type: "VECTOR",
				color: -10238109,
				default_value: f32_array_create_xyz(0.5, 0.5, 1.0),
				min: 0.0,
				max: 1.0,
				precision: 100,
				display: 0
			},
			{
				id: 0,
				node_id: 0,
				name: _tr("Emission"),
				type: "VALUE",
				color: 0xffa1a1a1,
				default_value: f32_array_create_x(1.0),
				min: 0.0,
				max: 1.0,
				precision: 100,
				display: 0
			},
			{
				id: 0,
				node_id: 0,
				name: _tr("Height"),
				type: "VALUE",
				color: 0xffa1a1a1,
				default_value: f32_array_create_x(1.0),
				min: 0.0,
				max: 1.0,
				precision: 100,
				display: 0
			},
			{
				id: 0,
				node_id: 0,
				name: _tr("Subsurface"),
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
				name: _tr("Material"),
				type: "ENUM",
				output: -1,
				default_value: f32_array_create_x(0),
				data: u8_array_create_from_string(""),
				min: 0.0,
				max: 1.0,
				precision: 100,
				height: 0
			}
		],
		width: 0
	},
	{
		id: 0,
		name: _tr("Object Info"),
		type: "OBJECT_INFO",
		x: 0,
		y: 0,
		color: 0xffb34f5a,
		inputs: [],
		outputs: [
			{
				id: 0,
				node_id: 0,
				name: _tr("Location"),
				type: "VECTOR",
				color: 0xff6363c7,
				default_value: f32_array_create_xyz(0.0, 0.0, 0.0),
				min: 0.0,
				max: 1.0,
				precision: 100,
				display: 0
			},
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
				name: _tr("Object Index"),
				type: "VALUE",
				color: 0xffa1a1a1,
				default_value: f32_array_create_x(0.0),
				min: 0.0,
				max: 1.0,
				precision: 100,
				display: 0
			},
			{
				id: 0,
				node_id: 0,
				name: _tr("Material Index"),
				type: "VALUE",
				color: 0xffa1a1a1,
				default_value: f32_array_create_x(0.0),
				min: 0.0,
				max: 1.0,
				precision: 100,
				display: 0
			},
			{
				id: 0,
				node_id: 0,
				name: _tr("Random"),
				type: "VALUE",
				color: 0xffa1a1a1,
				default_value: f32_array_create_x(0.0),
				min: 0.0,
				max: 1.0,
				precision: 100,
				display: 0
			}
		],
		buttons: [],
		width: 0
	},
	{
		id: 0,
		name: _tr("Picker"),
		type: "PICKER", // extension
		x: 0,
		y: 0,
		color: 0xff4982a0,
		inputs: [],
		outputs: [
			{
				id: 0,
				node_id: 0,
				name: _tr("Base Color"),
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
				name: _tr("Opacity"),
				type: "VALUE",
				color: 0xffa1a1a1,
				default_value: f32_array_create_x(1.0),
				min: 0.0,
				max: 1.0,
				precision: 100,
				display: 0
			},
			{
				id: 0,
				node_id: 0,
				name: _tr("Occlusion"),
				type: "VALUE",
				color: 0xffa1a1a1,
				default_value: f32_array_create_x(1.0),
				min: 0.0,
				max: 1.0,
				precision: 100,
				display: 0
			},
			{
				id: 0,
				node_id: 0,
				name: _tr("Roughness"),
				type: "VALUE",
				color: 0xffa1a1a1,
				default_value: f32_array_create_x(1.0),
				min: 0.0,
				max: 1.0,
				precision: 100,
				display: 0
			},
			{
				id: 0,
				node_id: 0,
				name: _tr("Metallic"),
				type: "VALUE",
				color: 0xffa1a1a1,
				default_value: f32_array_create_x(1.0),
				min: 0.0,
				max: 1.0,
				precision: 100,
				display: 0
			},
			{
				id: 0,
				node_id: 0,
				name: _tr("Normal Map"),
				type: "VECTOR",
				color: -10238109,
				default_value: f32_array_create_xyz(0.5, 0.5, 1.0),
				min: 0.0,
				max: 1.0,
				precision: 100,
				display: 0
			},
			{
				id: 0,
				node_id: 0,
				name: _tr("Emission"),
				type: "VALUE",
				color: 0xffa1a1a1,
				default_value: f32_array_create_x(1.0),
				min: 0.0,
				max: 1.0,
				precision: 100,
				display: 0
			},
			{
				id: 0,
				node_id: 0,
				name: _tr("Height"),
				type: "VALUE",
				color: 0xffa1a1a1,
				default_value: f32_array_create_x(1.0),
				min: 0.0,
				max: 1.0,
				precision: 100,
				display: 0
			},
			{
				id: 0,
				node_id: 0,
				name: _tr("Subsurface"),
				type: "VALUE",
				color: 0xffa1a1a1,
				default_value: f32_array_create_x(1.0),
				min: 0.0,
				max: 1.0,
				precision: 100,
				display: 0
			}
		],
		buttons: [],
		width: 0
	},
	{
		id: 0,
		name: _tr("RGB"),
		type: "RGB",
		x: 0,
		y: 0,
		color: 0xffb34f5a,
		inputs: [],
		outputs: [
			{
				id: 0,
				node_id: 0,
				name: _tr("Color"),
				type: "RGBA",
				color: 0xffc7c729,
				default_value: f32_array_create_xyzw(0.5, 0.5, 0.5, 1.0),
				min: 0.0,
				max: 1.0,
				precision: 100,
				display: 0
			}
		],
		buttons: [
			{
				name: _tr("default_value"),
				type: "RGBA",
				output: 0,
				default_value: f32_array_create_x(0),
				data: null,
				min: 0.0,
				max: 1.0,
				precision: 100,
				height: 0
			}
		],
		width: 0
	},
	{
		id: 0,
		name: _tr("Script"),
		type: "SCRIPT_CPU", // extension
		x: 0,
		y: 0,
		color: 0xffb34f5a,
		inputs: [],
		outputs: [
			{
				id: 0,
				node_id: 0,
				name: _tr("Value"),
				type: "VALUE",
				color: 0xffa1a1a1,
				default_value: f32_array_create_x(0.5),
				min: 0.0,
				max: 1.0,
				precision: 100,
				display: 0
			}
		],
		buttons: [
			{
				name: " ",
				type: "STRING",
				output: -1,
				default_value: f32_array_create_x(0), // "",
				data: null,
				min: 0.0,
				max: 1.0,
				precision: 100,
				height: 0
			}
		],
		width: 0
	},
	{
		id: 0,
		name: _tr("Shader"),
		type: "SHADER_GPU", // extension
		x: 0,
		y: 0,
		color: 0xffb34f5a,
		inputs: [],
		outputs: [
			{
				id: 0,
				node_id: 0,
				name: _tr("Value"),
				type: "VALUE",
				color: 0xffa1a1a1,
				default_value: f32_array_create_x(0.5),
				min: 0.0,
				max: 1.0,
				precision: 100,
				display: 0
			}
		],
		buttons: [
			{
				name: " ",
				type: "STRING",
				output: -1,
				default_value: f32_array_create_x(0), // "",
				data: null,
				min: 0.0,
				max: 1.0,
				precision: 100,
				height: 0
			}
		],
		width: 0
	},
	{
		id: 0,
		name: _tr("Tangent"),
		type: "TANGENT",
		x: 0,
		y: 0,
		color: 0xffb34f5a,
		inputs: [],
		outputs: [
			{
				id: 0,
				node_id: 0,
				name: _tr("Tangent"),
				type: "VECTOR",
				color: 0xff6363c7,
				default_value: f32_array_create_xyz(0.0, 0.0, 0.0),
				min: 0.0,
				max: 1.0,
				precision: 100,
				display: 0
			}
		],
		buttons: [],
		width: 0
	},
	{
		id: 0,
		name: _tr("Texture Coordinate"),
		type: "TEX_COORD",
		x: 0,
		y: 0,
		color: 0xffb34f5a,
		inputs: [],
		outputs: [
			{
				id: 0,
				node_id: 0,
				name: _tr("Generated"),
				type: "VECTOR",
				color: 0xff6363c7,
				default_value: f32_array_create_xyz(0.0, 0.0, 0.0),
				min: 0.0,
				max: 1.0,
				precision: 100,
				display: 0
			},
			{
				id: 0,
				node_id: 0,
				name: _tr("Normal"),
				type: "VECTOR",
				color: 0xff6363c7,
				default_value: f32_array_create_xyz(0.0, 0.0, 0.0),
				min: 0.0,
				max: 1.0,
				precision: 100,
				display: 0
			},
			{
				id: 0,
				node_id: 0,
				name: _tr("UV"),
				type: "VECTOR",
				color: 0xff6363c7,
				default_value: f32_array_create_xyz(0.0, 0.0, 0.0),
				min: 0.0,
				max: 1.0,
				precision: 100,
				display: 0
			},
			{
				id: 0,
				node_id: 0,
				name: _tr("Object"),
				type: "VECTOR",
				color: 0xff6363c7,
				default_value: f32_array_create_xyz(0.0, 0.0, 0.0),
				min: 0.0,
				max: 1.0,
				precision: 100,
				display: 0
			},
			{
				id: 0,
				node_id: 0,
				name: _tr("Camera"),
				type: "VECTOR",
				color: 0xff6363c7,
				default_value: f32_array_create_xyz(0.0, 0.0, 0.0),
				min: 0.0,
				max: 1.0,
				precision: 100,
				display: 0
			},
			{
				id: 0,
				node_id: 0,
				name: _tr("Window"),
				type: "VECTOR",
				color: 0xff6363c7,
				default_value: f32_array_create_xyz(0.0, 0.0, 0.0),
				min: 0.0,
				max: 1.0,
				precision: 100,
				display: 0
			},
			{
				id: 0,
				node_id: 0,
				name: _tr("Reflection"),
				type: "VECTOR",
				color: 0xff6363c7,
				default_value: f32_array_create_xyz(0.0, 0.0, 0.0),
				min: 0.0,
				max: 1.0,
				precision: 100,
				display: 0
			}
		],
		buttons: []
	},
	{
		id: 0,
		name: _tr("UV Map"),
		type: "UVMAP",
		x: 0,
		y: 0,
		color: 0xffb34f5a,
		inputs: [],
		outputs: [
			{
				id: 0,
				node_id: 0,
				name: _tr("UV"),
				type: "VECTOR",
				color: 0xff6363c7,
				default_value: f32_array_create_xyz(0.0, 0.0, 0.0),
				min: 0.0,
				max: 1.0,
				precision: 100,
				display: 0
			}
		],
		buttons: [],
		width: 0
	},
	{
		id: 0,
		name: _tr("Value"),
		type: "VALUE",
		x: 0,
		y: 0,
		color: 0xffb34f5a,
		inputs: [],
		outputs: [
			{
				id: 0,
				node_id: 0,
				name: _tr("Value"),
				type: "VALUE",
				color: 0xffa1a1a1,
				default_value: f32_array_create_x(0.5),
				min: 0.0,
				max: 1.0,
				precision: 100,
				display: 0
			}
		],
		buttons: [
			{
				name: _tr("default_value"),
				type: "VALUE",
				output: 0,
				default_value: f32_array_create_x(0),
				data: null,
				min: 0.0,
				max: 10.0,
				precision: 100,
				height: 0
			}
		],
		width: 0
	},
	{
		id: 0,
		name: _tr("Vertex Color"),
		type: "VERTEX_COLOR",
		x: 0,
		y: 0,
		color: 0xffb34f5a,
		inputs: [],
		outputs: [
			{
				id: 0,
				node_id: 0,
				name: _tr("Color"),
				type: "RGBA",
				color: 0xffc7c729,
				default_value: f32_array_create_xyzw(0.8, 0.8, 0.8, 1.0),
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
				default_value: f32_array_create_x(0.0),
				min: 0.0,
				max: 1.0,
				precision: 100,
				display: 0
			}
		],
		buttons: [],
		width: 0
	},
	{
		id: 0,
		name: _tr("Wireframe"),
		type: "WIREFRAME",
		x: 0,
		y: 0,
		color: 0xffb34f5a,
		inputs: [
			{
				id: 0,
				node_id: 0,
				name: _tr("Size"),
				type: "VALUE",
				color: 0xffa1a1a1,
				default_value: f32_array_create_x(0.01),
				min: 0.0,
				max: 0.1,
				precision: 100,
				display: 0
			},
		],
		outputs: [
			{
				id: 0,
				node_id: 0,
				name: _tr("Fac"),
				type: "VALUE",
				color: 0xffa1a1a1,
				default_value: f32_array_create_x(0.0),
				min: 0.0,
				max: 1.0,
				precision: 100,
				display: 0
			}
		],
		buttons: [
			{
				name: _tr("Pixel Size"),
				type: "BOOL",
				output: 0,
				default_value: f32_array_create_x(0),
				data: null,
				min: 0.0,
				max: 1.0,
				precision: 100,
				height: 0
			}
		],
		width: 0
	},
];

// let nodes_material_output: ui_node_t[] = [
// 	{
// 		id: 0,
// 		name: _tr("Material Output"),
// 		type: "OUTPUT_MATERIAL_PBR",
// 		x: 0,
// 		y: 0,
// 		color: 0xffb34f5a,
// 		inputs: [
// 			{
// 				id: 0,
// 				node_id: 0,
// 				name: _tr("Base Color"),
// 				type: "RGBA",
// 				color: 0xffc7c729,
// 				default_value: f32_array_create_xyzw(0.8, 0.8, 0.8, 1.0),
//				min: 0.0,
//				max: 1.0,
//				precision: 100,
//				display: 0
// 			},
// 			{
// 				id: 0,
// 				node_id: 0,
// 				name: _tr("Opacity"),
// 				type: "VALUE",
// 				color: 0xffa1a1a1,
// 				default_value: f32_array_create_x(1.0),
//				min: 0.0,
//				max: 1.0,
//				precision: 100,
//				display: 0
// 			},
// 			{
// 				id: 0,
// 				node_id: 0,
// 				name: _tr("Occlusion"),
// 				type: "VALUE",
// 				color: 0xffa1a1a1,
// 				default_value: f32_array_create_x(1.0),
//				min: 0.0,
//				max: 1.0,
//				precision: 100,
//				display: 0
// 			},
// 			{
// 				id: 0,
// 				node_id: 0,
// 				name: _tr("Roughness"),
// 				type: "VALUE",
// 				color: 0xffa1a1a1,
// 				default_value: f32_array_create_x(0.1),
//				min: 0.0,
//				max: 1.0,
//				precision: 100,
//				display: 0
// 			},
// 			{
// 				id: 0,
// 				node_id: 0,
// 				name: _tr("Metallic"),
// 				type: "VALUE",
// 				color: 0xffa1a1a1,
// 				default_value: f32_array_create_x(0.0),
//				min: 0.0,
//				max: 1.0,
//				precision: 100,
//				display: 0
// 			},
// 			{
// 				id: 0,
// 				node_id: 0,
// 				name: _tr("Normal Map"),
// 				type: "VECTOR",
// 				color: -10238109,
// 				default_value: f32_array_create_xyz(0.5, 0.5, 1.0),
//				min: 0.0,
//				max: 1.0,
//				precision: 100,
//				display: 0
// 			},
// 			{
// 				id: 0,
// 				node_id: 0,
// 				name: _tr("Emission"),
// 				type: "VALUE",
// 				color: 0xffa1a1a1,
// 				default_value: f32_array_create_x(0.0),
//				min: 0.0,
//				max: 1.0,
//				precision: 100,
//				display: 0
// 			},
// 			{
// 				id: 0,
// 				node_id: 0,
// 				name: _tr("Height"),
// 				type: "VALUE",
// 				color: 0xffa1a1a1,
// 				default_value: f32_array_create_x(0.0),
//				min: 0.0,
//				max: 1.0,
//				precision: 100,
//				display: 0
// 			},
// 			{
// 				id: 0,
// 				node_id: 0,
// 				name: _tr("Subsurface"),
// 				type: "VALUE",
// 				color: 0xffa1a1a1,
// 				default_value: f32_array_create_x(0.0),
//				min: 0.0,
//				max: 1.0,
//				precision: 100,
//				display: 0
// 			}
// 		],
// 		outputs: [],
// 		buttons: [],
//		width: 0
// 	}
// ];

let nodes_material_texture: ui_node_t[] = [
	{
		id: 0,
		name: _tr("Brick Texture"),
		type: "TEX_BRICK",
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
			},
			{
				id: 0,
				node_id: 0,
				name: _tr("Color 1"),
				type: "RGBA",
				color: 0xffc7c729,
				default_value: f32_array_create_xyz(0.8, 0.8, 0.8),
				min: 0.0,
				max: 1.0,
				precision: 100,
				display: 0
			},
			{
				id: 0,
				node_id: 0,
				name: _tr("Color 2"),
				type: "RGBA",
				color: 0xffc7c729,
				default_value: f32_array_create_xyz(0.2, 0.2, 0.2),
				min: 0.0,
				max: 1.0,
				precision: 100,
				display: 0
			},
			{
				id: 0,
				node_id: 0,
				name: _tr("Mortar"),
				type: "RGBA",
				color: 0xffc7c729,
				default_value: f32_array_create_xyz(0.0, 0.0, 0.0),
				min: 0.0,
				max: 1.0,
				precision: 100,
				display: 0
			},
			{
				id: 0,
				node_id: 0,
				name: _tr("Scale"),
				type: "VALUE",
				color: 0xffa1a1a1,
				default_value: f32_array_create_x(5.0),
				min: 0.0,
				max: 10.0,
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
				default_value: f32_array_create_xyzw(0.8, 0.8, 0.8, 1.0),
				min: 0.0,
				max: 1.0,
				precision: 100,
				display: 0
			},
			{
				id: 0,
				node_id: 0,
				name: _tr("Fac"),
				type: "VALUE",
				color: 0xffa1a1a1,
				default_value: f32_array_create_x(1.0),
				min: 0.0,
				max: 1.0,
				precision: 100,
				display: 0
			}
		],
		buttons: [],
		width: 0
	},
	{
		id: 0,
		name: _tr("Checker Texture"),
		type: "TEX_CHECKER",
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
			},
			{
				id: 0,
				node_id: 0,
				name: _tr("Color 1"),
				type: "RGBA",
				color: 0xffc7c729,
				default_value: f32_array_create_xyz(0.8, 0.8, 0.8),
				min: 0.0,
				max: 1.0,
				precision: 100,
				display: 0
			},
			{
				id: 0,
				node_id: 0,
				name: _tr("Color 2"),
				type: "RGBA",
				color: 0xffc7c729,
				default_value: f32_array_create_xyz(0.2, 0.2, 0.2),
				min: 0.0,
				max: 1.0,
				precision: 100,
				display: 0
			},
			{
				id: 0,
				node_id: 0,
				name: _tr("Scale"),
				type: "VALUE",
				color: 0xffa1a1a1,
				default_value: f32_array_create_x(5.0),
				min: 0.0,
				max: 10.0,
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
				default_value: f32_array_create_xyzw(0.8, 0.8, 0.8, 1.0),
				min: 0.0,
				max: 1.0,
				precision: 100,
				display: 0
			},
			{
				id: 0,
				node_id: 0,
				name: _tr("Fac"),
				type: "VALUE",
				color: 0xffa1a1a1,
				default_value: f32_array_create_x(1.0),
				min: 0.0,
				max: 1.0,
				precision: 100,
				display: 0
			}
		],
		buttons: [],
		width: 0
	},
	{
		id: 0,
		name: _tr("Curvature Bake"),
		type: "BAKE_CURVATURE",
		x: 0,
		y: 0,
		color: 0xff4982a0,
		inputs: [
			{
				id: 0,
				node_id: 0,
				name: _tr("Strength"),
				type: "VALUE",
				color: 0xffa1a1a1,
				default_value: f32_array_create_x(1.0),
				min: 0.0,
				max: 2.0,
				precision: 100,
				display: 0
			},
			{
				id: 0,
				node_id: 0,
				name: _tr("Radius"),
				type: "VALUE",
				color: 0xffa1a1a1,
				default_value: f32_array_create_x(1.0),
				min: 0.0,
				max: 2.0,
				precision: 100,
				display: 0
			},
			{
				id: 0,
				node_id: 0,
				name: _tr("Offset"),
				type: "VALUE",
				color: 0xffa1a1a1,
				default_value: f32_array_create_x(0.0),
				min: -2.0,
				max: 2.0,
				precision: 100,
				display: 0
			}
		],
		outputs: [
			{
				id: 0,
				node_id: 0,
				name: _tr("Value"),
				type: "VALUE",
				color: 0xffa1a1a1,
				default_value: f32_array_create_x(1.0),
				min: 0.0,
				max: 1.0,
				precision: 100,
				display: 0
			}
		],
		buttons: [],
		width: 0
	},
	{
		id: 0,
		name: _tr("Gradient Texture"),
		type: "TEX_GRADIENT",
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
				default_value: f32_array_create_xyzw(0.8, 0.8, 0.8, 1.0),
				min: 0.0,
				max: 1.0,
				precision: 100,
				display: 0
			},
			{
				id: 0,
				node_id: 0,
				name: _tr("Fac"),
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
				name: _tr("gradient_type"),
				type: "ENUM",
				output: 0,
				default_value: f32_array_create_x(0),
				data: u8_array_create_from_string(_tr("Linear") + "\n" + _tr("Diagonal") + "\n" + _tr("Radial") + "\n" + _tr("Spherical")),
				min: 0.0,
				max: 1.0,
				precision: 100,
				height: 0
			}
		],
		width: 0
	},
	{
		id: 0,
		name: _tr("Image Texture"),
		type: "TEX_IMAGE",
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
				name: _tr("File"),
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
				name: _tr("Color Space"),
				type: "ENUM",
				output: -1,
				default_value: f32_array_create_x(0),
				data: u8_array_create_from_string(_tr("Auto") + "\n" + _tr("Linear") + "\n" + _tr("sRGB") + "\n" + _tr("DirectX Normal Map")),
				min: 0.0,
				max: 1.0,
				precision: 100,
				height: 0
			}
		],
		width: 0
	},
	{
		id: 0,
		name: _tr("Magic Texture"),
		type: "TEX_MAGIC",
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
			},
			{
				id: 0,
				node_id: 0,
				name: _tr("Scale"),
				type: "VALUE",
				color: 0xffa1a1a1,
				default_value: f32_array_create_x(5.0),
				min: 0.0,
				max: 10.0,
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
				default_value: f32_array_create_xyzw(0.8, 0.8, 0.8, 1.0),
				min: 0.0,
				max: 1.0,
				precision: 100,
				display: 0
			},
			{
				id: 0,
				node_id: 0,
				name: _tr("Fac"),
				type: "VALUE",
				color: 0xffa1a1a1,
				default_value: f32_array_create_x(1.0),
				min: 0.0,
				max: 1.0,
				precision: 100,
				display: 0
			}
		],
		buttons: [],
		width: 0
	},
	{
		id: 0,
		name: _tr("Musgrave Texture"),
		type: "TEX_MUSGRAVE",
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
			},
			{
				id: 0,
				node_id: 0,
				name: _tr("Scale"),
				type: "VALUE",
				color: 0xffa1a1a1,
				default_value: f32_array_create_x(5.0),
				min: 0.0,
				max: 10.0,
				precision: 100,
				display: 0
			}
		],
		outputs: [
			{
				id: 0,
				node_id: 0,
				name: _tr("Height"),
				type: "VALUE",
				color: 0xffa1a1a1,
				default_value: f32_array_create_x(1.0),
				min: 0.0,
				max: 1.0,
				precision: 100,
				display: 0
			}
		],
		buttons: [],
		width: 0
	},
	{
		id: 0,
		name: _tr("Noise Texture"),
		type: "TEX_NOISE",
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
			},
			{
				id: 0,
				node_id: 0,
				name: _tr("Scale"),
				type: "VALUE",
				color: 0xffa1a1a1,
				default_value: f32_array_create_x(5.0),
				min: 0.0,
				max: 10.0,
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
				default_value: f32_array_create_xyzw(0.8, 0.8, 0.8, 1.0),
				min: 0.0,
				max: 1.0,
				precision: 100,
				display: 0
			},
			{
				id: 0,
				node_id: 0,
				name: _tr("Fac"),
				type: "VALUE",
				color: 0xffa1a1a1,
				default_value: f32_array_create_x(1.0),
				min: 0.0,
				max: 1.0,
				precision: 100,
				display: 0
			}
		],
		buttons: [],
		width: 0
	},
	{
		id: 0,
		name: _tr("Voronoi Texture"),
		type: "TEX_VORONOI",
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
			},
			{
				id: 0,
				node_id: 0,
				name: _tr("Scale"),
				type: "VALUE",
				color: 0xffa1a1a1,
				default_value: f32_array_create_x(5.0),
				min: 0.0,
				max: 10.0,
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
				default_value: f32_array_create_xyzw(0.8, 0.8, 0.8, 1.0),
				min: 0.0,
				max: 1.0,
				precision: 100,
				display: 0
			},
			{
				id: 0,
				node_id: 0,
				name: _tr("Fac"),
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
				name: _tr("coloring"),
				type: "ENUM",
				output: 0,
				default_value: f32_array_create_x(0),
				data: u8_array_create_from_string(_tr("Intensity") + "\n" + _tr("Cells")),
				min: 0.0,
				max: 1.0,
				precision: 100,
				height: 0
			}
		],
		width: 0
	},
	{
		id: 0,
		name: _tr("Wave Texture"),
		type: "TEX_WAVE",
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
			},
			{
				id: 0,
				node_id: 0,
				name: _tr("Scale"),
				type: "VALUE",
				color: 0xffa1a1a1,
				default_value: f32_array_create_x(5.0),
				min: 0.0,
				max: 10.0,
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
				default_value: f32_array_create_xyzw(0.8, 0.8, 0.8, 1.0),
				min: 0.0,
				max: 1.0,
				precision: 100,
				display: 0
			},
			{
				id: 0,
				node_id: 0,
				name: _tr("Fac"),
				type: "VALUE",
				color: 0xffa1a1a1,
				default_value: f32_array_create_x(1.0),
				min: 0.0,
				max: 1.0,
				precision: 100,
				display: 0
			}
		],
		buttons: [],
		width: 0
	}
];

let nodes_material_color: ui_node_t[] = [
	{
		id: 0,
		name: _tr("Blur"),
		type: "BLUR", // extension
		x: 0,
		y: 0,
		color: 0xff448c6d,
		inputs: [
			{
				id: 0,
				node_id: 0,
				name: _tr("Color"),
				type: "RGBA",
				color: 0xffc7c729,
				default_value: f32_array_create_xyzw(0.8, 0.8, 0.8, 1.0),
				min: 0.0,
				max: 1.0,
				precision: 100,
				display: 0
			},
			{
				id: 0,
				node_id: 0,
				name: _tr("Strength"),
				type: "VALUE",
				color: 0xffa1a1a1,
				default_value: f32_array_create_x(0.5),
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
				default_value: f32_array_create_xyzw(0.8, 0.8, 0.8, 1.0),
				min: 0.0,
				max: 1.0,
				precision: 100,
				display: 0
			}
		],
		buttons: [],
		width: 0
	},
	{
		id: 0,
		name: _tr("Brightness/Contrast"),
		type: "BRIGHTCONTRAST",
		x: 0,
		y: 0,
		color: 0xff448c6d,
		inputs: [
			{
				id: 0,
				node_id: 0,
				name: _tr("Color"),
				type: "RGBA",
				color: 0xffc7c729,
				default_value: f32_array_create_xyzw(0.8, 0.8, 0.8, 1.0),
				min: 0.0,
				max: 1.0,
				precision: 100,
				display: 0
			},
			{
				id: 0,
				node_id: 0,
				name: _tr("Bright"),
				type: "VALUE",
				color: 0xffa1a1a1,
				default_value: f32_array_create_x(0.0),
				min: 0.0,
				max: 1.0,
				precision: 100,
				display: 0
			},
			{
				id: 0,
				node_id: 0,
				name: _tr("Contrast"),
				type: "VALUE",
				color: 0xffa1a1a1,
				default_value: f32_array_create_x(0.0),
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
				default_value: f32_array_create_xyzw(0.8, 0.8, 0.8, 1.0),
				min: 0.0,
				max: 1.0,
				precision: 100,
				display: 0
			}
		],
		buttons: [],
		width: 0
	},
	{
		id: 0,
		name: _tr("Gamma"),
		type: "GAMMA",
		x: 0,
		y: 0,
		color: 0xff448c6d,
		inputs: [
			{
				id: 0,
				node_id: 0,
				name: _tr("Color"),
				type: "RGBA",
				color: 0xffc7c729,
				default_value: f32_array_create_xyzw(0.8, 0.8, 0.8, 1.0),
				min: 0.0,
				max: 1.0,
				precision: 100,
				display: 0
			},
			{
				id: 0,
				node_id: 0,
				name: _tr("Gamma"),
				type: "VALUE",
				color: 0xffa1a1a1,
				default_value: f32_array_create_x(1.0),
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
				default_value: f32_array_create_xyzw(0.8, 0.8, 0.8, 1.0),
				min: 0.0,
				max: 1.0,
				precision: 100,
				display: 0
			}
		],
		buttons: [],
		width: 0
	},
	{
		id: 0,
		name: _tr("Hue/Saturation/Value"),
		type: "HUE_SAT",
		x: 0,
		y: 0,
		color: 0xff448c6d,
		inputs: [
			{
				id: 0,
				node_id: 0,
				name: _tr("Hue"),
				type: "VALUE",
				color: 0xffa1a1a1,
				default_value: f32_array_create_x(0.5),
				min: 0.0,
				max: 1.0,
				precision: 100,
				display: 0
			},
			{
				id: 0,
				node_id: 0,
				name: _tr("Saturation"),
				type: "VALUE",
				color: 0xffa1a1a1,
				default_value: f32_array_create_x(1.0),
				min: 0.0,
				max: 1.0,
				precision: 100,
				display: 0
			},
			{
				id: 0,
				node_id: 0,
				name: _tr("Value"),
				type: "VALUE",
				color: 0xffa1a1a1,
				default_value: f32_array_create_x(1.0),
				min: 0.0,
				max: 1.0,
				precision: 100,
				display: 0
			},
			{
				id: 0,
				node_id: 0,
				name: _tr("Fac"),
				type: "VALUE",
				color: 0xffa1a1a1,
				default_value: f32_array_create_x(1.0),
				min: 0.0,
				max: 1.0,
				precision: 100,
				display: 0
			},
			{
				id: 0,
				node_id: 0,
				name: _tr("Color"),
				type: "RGBA",
				color: 0xffc7c729,
				default_value: f32_array_create_xyzw(0.8, 0.8, 0.8, 1.0),
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
				default_value: f32_array_create_xyzw(0.8, 0.8, 0.8, 1.0),
				min: 0.0,
				max: 1.0,
				precision: 100,
				display: 0
			}
		],
		buttons: [],
		width: 0
	},
	{
		id: 0,
		name: _tr("Invert"),
		type: "INVERT",
		x: 0,
		y: 0,
		color: 0xff448c6d,
		inputs: [
			{
				id: 0,
				node_id: 0,
				name: _tr("Fac"),
				type: "VALUE",
				color: 0xffa1a1a1,
				default_value: f32_array_create_x(1.0),
				min: 0.0,
				max: 1.0,
				precision: 100,
				display: 0
			},
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
			}
		],
		outputs: [
			{
				id: 0,
				node_id: 0,
				name: _tr("Color"),
				type: "RGBA",
				color: 0xffc7c729,
				default_value: f32_array_create_xyzw(0.8, 0.8, 0.8, 1.0),
				min: 0.0,
				max: 1.0,
				precision: 100,
				display: 0
			}
		],
		buttons: [],
		width: 0
	},
	{
		id: 0,
		name: _tr("Mix Color"),
		type: "MIX_RGB",
		x: 0,
		y: 0,
		color: 0xff448c6d,
		inputs: [
			{
				id: 0,
				node_id: 0,
				name: _tr("Fac"),
				type: "VALUE",
				color: 0xffa1a1a1,
				default_value: f32_array_create_x(0.5),
				min: 0.0,
				max: 1.0,
				precision: 100,
				display: 0
			},
			{
				id: 0,
				node_id: 0,
				name: _tr("Color 1"),
				type: "RGBA",
				color: 0xffc7c729,
				default_value: f32_array_create_xyzw(0.5, 0.5, 0.5, 1.0),
				min: 0.0,
				max: 1.0,
				precision: 100,
				display: 0
			},
			{
				id: 0,
				node_id: 0,
				name: _tr("Color 2"),
				type: "RGBA",
				color: 0xffc7c729,
				default_value: f32_array_create_xyzw(0.5, 0.5, 0.5, 1.0),
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
				default_value: f32_array_create_xyzw(0.8, 0.8, 0.8, 1.0),
				min: 0.0,
				max: 1.0,
				precision: 100,
				display: 0
			}
		],
		buttons: [
			{
				name: _tr("blend_type"),
				type: "ENUM",
				output: 0,
				default_value: f32_array_create_x(0),
				data: u8_array_create_from_string(_tr("Mix") + "\n" + _tr("Darken") + "\n" + _tr("Multiply") + "\n" + _tr("Burn") + "\n" + _tr("Lighten") + "\n" + _tr("Screen") + "\n" + _tr("Dodge") + "\n" + _tr("Add") + "\n" + _tr("Overlay") + "\n" + _tr("Soft Light") + "\n" + _tr("Linear Light") + "\n" + _tr("Difference") + "\n" + _tr("Subtract") + "\n" + _tr("Divide") + "\n" + _tr("Hue") + "\n" + _tr("Saturation") + "\n" + _tr("Color") + "\n" + _tr("Value")),
				min: 0.0,
				max: 1.0,
				precision: 100,
				height: 0
			},
			{
				name: _tr("use_clamp"),
				type: "BOOL",
				output: 0,
				default_value: f32_array_create_x(0),
				data: null,
				min: 0.0,
				max: 1.0,
				precision: 100,
				height: 0
			}
		],
		width: 0
	},
	{
		id: 0,
		name: _tr("Quantize"),
		type: "QUANTIZE",
		x: 0,
		y: 0,
		color: 0xff448c6d,
		inputs: [
			{
				id: 0,
				node_id: 0,
				name: _tr("Stength"),
				type: "VALUE",
				color: 0xffa1a1a1,
				default_value: f32_array_create_x(0.1),
				min: 0.0,
				max: 1.0,
				precision: 100,
				display: 0
			},
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
			}
		],
		outputs: [
			{
				id: 0,
				node_id: 0,
				name: _tr("Color"),
				type: "RGBA",
				color: 0xffc7c729,
				default_value: f32_array_create_xyzw(0.8, 0.8, 0.8, 1.0),
				min: 0.0,
				max: 1.0,
				precision: 100,
				display: 0
			}
		],
		buttons: [],
		width: 0
	},
	{
		id: 0,
		name: _tr("Warp"),
		type: "DIRECT_WARP", // extension
		x: 0,
		y: 0,
		color: 0xff448c6d,
		inputs: [
			{
				id: 0,
				node_id: 0,
				name: _tr("Color"),
				type: "RGBA",
				color: 0xffc7c729,
				default_value: f32_array_create_xyzw(0.8, 0.8, 0.8, 1.0),
				min: 0.0,
				max: 1.0,
				precision: 100,
				display: 0
			},
			{
				id: 0,
				node_id: 0,
				name: _tr("Angle"),
				type: "VALUE",
				color: 0xffa1a1a1,
				default_value: f32_array_create_x(0.0),
				min: 0.0,
				max: 360.0,
				precision: 100,
				display: 0
			},
			{
				id: 0,
				node_id: 0,
				name: _tr("Mask"),
				type: "VALUE",
				color: 0xffa1a1a1,
				default_value: f32_array_create_x(0.5),
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
				default_value: f32_array_create_xyzw(0.8, 0.8, 0.8, 1.0),
				min: 0.0,
				max: 1.0,
				precision: 100,
				display: 0
			}
		],
		buttons: [],
		width: 0
	}
];

let nodes_material_vector: ui_node_t[] = [
	{
		id: 0,
		name: _tr("Bump"),
		type: "BUMP",
		x: 0,
		y: 0,
		color: 0xff522c99,
		inputs: [
			{
				id: 0,
				node_id: 0,
				name: _tr("Strength"),
				type: "VALUE",
				color: 0xffa1a1a1,
				default_value: f32_array_create_x(1.0),
				min: 0.0,
				max: 1.0,
				precision: 100,
				display: 0
			},
			{
				id: 0,
				node_id: 0,
				name: _tr("Distance"),
				type: "VALUE",
				color: 0xffa1a1a1,
				default_value: f32_array_create_x(0.0),
				min: 0.0,
				max: 1.0,
				precision: 100,
				display: 0
			},
			{
				id: 0,
				node_id: 0,
				name: _tr("Height"),
				type: "VALUE",
				color: 0xffa1a1a1,
				default_value: f32_array_create_x(1.0),
				min: 0.0,
				max: 1.0,
				precision: 100,
				display: 0
			},
			{
				id: 0,
				node_id: 0,
				name: _tr("Normal"),
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
				name: _tr("Normal Map"),
				type: "VECTOR",
				color: -10238109,
				default_value: f32_array_create_xyz(0.0, 0.0, 0.0),
				min: 0.0,
				max: 1.0,
				precision: 100,
				display: 0
			}
		],
		buttons: [],
		width: 0
	},
	{
		id: 0,
		name: _tr("Mapping"),
		type: "MAPPING",
		x: 0,
		y: 0,
		color: 0xff522c99,
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
			},
			{
				id: 0,
				node_id: 0,
				name: _tr("Location"),
				type: "VECTOR",
				color: 0xff6363c7,
				default_value: f32_array_create_xyz(0.0, 0.0, 0.0),
				min: 0.0,
				max: 1.0,
				precision: 100,
				display: 1
			},
			{
				id: 0,
				node_id: 0,
				name: _tr("Rotation"),
				type: "VECTOR",
				color: 0xff6363c7,
				default_value: f32_array_create_xyz(0.0, 0.0, 0.0),
				min: 0.0,
				max: 360.0,
				precision: 100,
				display: 1
			},
			{
				id: 0,
				node_id: 0,
				name: _tr("Scale"),
				type: "VECTOR",
				color: 0xff6363c7,
				default_value: f32_array_create_xyz(1.0, 1.0, 1.0),
				min: 0.0,
				max: 1.0,
				precision: 100,
				display: 1
			}
		],
		outputs: [
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
		buttons: [],
		width: 0
	},
	{
		id: 0,
		name: _tr("Mix Normal Map"),
		type: "MIX_NORMAL_MAP",
		x: 0,
		y: 0,
		color: 0xff522c99,
		inputs: [
			{
				id: 0,
				node_id: 0,
				name: _tr("Normal Map 1"),
				type: "VECTOR",
				color: -10238109,
				default_value: f32_array_create_xyz(0.5, 0.5, 1.0),
				min: 0.0,
				max: 1.0,
				precision: 100,
				display: 0
			},
			{
				id: 0,
				node_id: 0,
				name: _tr("Normal Map 2"),
				type: "VECTOR",
				color: -10238109,
				default_value: f32_array_create_xyz(0.5, 0.5, 1.0),
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
				name: _tr("Normal Map"),
				type: "VECTOR",
				color: -10238109,
				default_value: f32_array_create_xyz(0.5, 0.5, 1.0),
				min: 0.0,
				max: 1.0,
				precision: 100,
				display: 0
			}
		],
		buttons: [
			{
				name: _tr("blend_type"),
				type: "ENUM",
				output: 0,
				default_value: f32_array_create_x(0),
				data: u8_array_create_from_string(_tr("Partial Derivative") + "\n" + _tr("Whiteout") + "\n" + _tr("Reoriented")),
				min: 0.0,
				max: 1.0,
				precision: 100,
				height: 0
			}
		],
		width: 0
	},
	{
		id: 0,
		name: _tr("Normal"),
		type: "NORMAL",
		x: 0,
		y: 0,
		color: 0xff522c99,
		inputs: [
			{
				id: 0,
				node_id: 0,
				name: _tr("Normal"),
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
				name: _tr("Normal"),
				type: "VECTOR",
				color: 0xff6363c7,
				default_value: f32_array_create_xyz(0.0, 0.0, 0.0),
				min: 0.0,
				max: 1.0,
				precision: 100,
				display: 0
			},
			{
				id: 0,
				node_id: 0,
				name: _tr("Dot"),
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
				name: _tr("Vector"),
				type: "VECTOR",
				output: 0,
				default_value: f32_array_create_xyz(0.0, 0.0, 0.0),
				data: null,
				min: 0.0,
				max: 1.0,
				precision: 100,
				height: 0
			}
		],
		width: 0
	},
	{
		id: 0,
		name: _tr("Normal Map"),
		type: "NORMAL_MAP",
		x: 0,
		y: 0,
		color: 0xff522c99,
		inputs: [
			{
				id: 0,
				node_id: 0,
				name: _tr("Stength"),
				type: "VALUE",
				color: 0xffa1a1a1,
				default_value: f32_array_create_x(1.0),
				min: 0.0,
				max: 2.0,
				precision: 100,
				display: 0
			},
			{
				id: 0,
				node_id: 0,
				name: _tr("Normal Map"),
				type: "VECTOR",
				color: -10238109,
				default_value: f32_array_create_xyz(0.5, 0.5, 1.0),
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
				name: _tr("Normal Map"),
				type: "VECTOR",
				color: -10238109,
				default_value: f32_array_create_xyz(0.5, 0.5, 1.0),
				min: 0.0,
				max: 1.0,
				precision: 100,
				display: 0
			}
		],
		buttons: [],
		width: 0
	},
	{
		id: 0,
		name: _tr("Vector Curves"),
		type: "CURVE_VEC",
		x: 0,
		y: 0,
		color: 0xff522c99,
		inputs: [
			{
				id: 0,
				node_id: 0,
				name: _tr("Fac"),
				type: "VALUE",
				color: 0xffa1a1a1,
				default_value: f32_array_create_x(1.0),
				min: 0.0,
				max: 1.0,
				precision: 100,
				display: 0
			},
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
		buttons: [
			{
				name: "nodes_material_vector_curves_button",
				type: "CUSTOM",
				output: 0,
				default_value: f32_array_create(96 + 3), // x - [0, 32], y - [33, 64], z - [65, 96], x_len, y_len, z_len
				data: null,
				min: 0.0,
				max: 1.0,
				precision: 100,
				height: 8.5
			}
		],
		width: 0
	}
];

let nodes_material_converter: ui_node_t[] = [
	{
		id: 0,
		name: _tr("Clamp"),
		type: "CLAMP",
		x: 0,
		y: 0,
		color: 0xff62676d,
		inputs: [
			{
				id: 0,
				node_id: 0,
				name: _tr("Value"),
				type: "VALUE",
				color: 0xffa1a1a1,
				default_value: f32_array_create_x(0.5),
				min: 0.0,
				max: 1.0,
				precision: 100,
				display: 0
			},
			{
				id: 0,
				node_id: 0,
				name: _tr("Min"),
				type: "VALUE",
				color: 0xffa1a1a1,
				default_value: f32_array_create_x(0.0),
				min: 0.0,
				max: 1.0,
				precision: 100,
				display: 0
			},
			{
				id: 0,
				node_id: 0,
				name: _tr("Max"),
				type: "VALUE",
				color: 0xffa1a1a1,
				default_value: f32_array_create_x(1.0),
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
				name: _tr("Value"),
				type: "VALUE",
				color: 0xffa1a1a1,
				default_value: f32_array_create_x(0.0),
				min: 0.0,
				max: 1.0,
				precision: 100,
				display: 0
			}
		],
		buttons: [
			{
				name: _tr("operation"),
				type: "ENUM",
				output: 0,
				default_value: f32_array_create_x(0),
				data: u8_array_create_from_string(_tr("Min Max") + "\n" + _tr("Range")),
				min: 0.0,
				max: 1.0,
				precision: 100,
				height: 0
			}
		],
		width: 0
	},
	{
		id: 0,
		name: _tr("Color Ramp"),
		type: "VALTORGB",
		x: 0,
		y: 0,
		color: 0xff62676d,
		inputs: [
			{
				id: 0,
				node_id: 0,
				name: _tr("Fac"),
				type: "VALUE",
				color: 0xffa1a1a1,
				default_value: f32_array_create_x(0.5),
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
				default_value: f32_array_create_x(0.0),
				min: 0.0,
				max: 1.0,
				precision: 100,
				display: 0
			}
		],
		buttons: [
			{
				name: "nodes_material_color_ramp_button",
				type: "CUSTOM",
				output: 0,
				default_value: f32_array_create_xyzwv(1.0, 1.0, 1.0, 1.0, 0.0),
				data: u8_array_create(1),
				min: 0.0,
				max: 1.0,
				precision: 100,
				height: 4.5
			}
		],
		width: 0
	},
	{
		id: 0,
		name: _tr("Color Mask"),
		type: "COLMASK",
		x: 0,
		y: 0,
		color: 0xff62676d,
		inputs: [
			{
				id: 0,
				node_id: 0,
				name: _tr("Color"),
				type: "RGBA",
				color: 0xffc7c729,
				default_value: f32_array_create_xyzw(0.8, 0.8, 0.8, 1.0),
				min: 0.0,
				max: 1.0,
				precision: 100,
				display: 0
			},
			{
				id: 0,
				node_id: 0,
				name: _tr("Mask Color"),
				type: "RGBA",
				color: 0xffc7c729,
				default_value: f32_array_create_xyzw(0.8, 0.8, 0.8, 1.0),
				min: 0.0,
				max: 1.0,
				precision: 100,
				display: 0
			},
			{
				id: 0,
				node_id: 0,
				name: _tr("Radius"),
				type: "VALUE",
				color: 0xffa1a1a1,
				default_value: f32_array_create_x(0.1),
				min: 0.0,
				max: 1.74,
				precision: 100,
				display: 0
			},
			{
				id: 0,
				node_id: 0,
				name: _tr("Fuzziness"),
				type: "VALUE",
				color: 0xffa1a1a1,
				default_value: f32_array_create_x(0.0),
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
				name: _tr("Mask"),
				type: "VALUE",
				color: 0xffa1a1a1,
				default_value: f32_array_create_x(0.0),
				min: 0.0,
				max: 1.0,
				precision: 100,
				display: 0
			}
		],
		buttons: [],
		width: 0
	},
	{
		id: 0,
		name: _tr("Combine HSV"),
		type: "COMBHSV",
		x: 0,
		y: 0,
		color: 0xff62676d,
		inputs: [
			{
				id: 0,
				node_id: 0,
				name: _tr("H"),
				type: "VALUE",
				color: 0xffa1a1a1,
				default_value: f32_array_create_x(0.0),
				min: 0.0,
				max: 1.0,
				precision: 100,
				display: 0
			},
			{
				id: 0,
				node_id: 0,
				name: _tr("S"),
				type: "VALUE",
				color: 0xffa1a1a1,
				default_value: f32_array_create_x(0.0),
				min: 0.0,
				max: 1.0,
				precision: 100,
				display: 0
			},
			{
				id: 0,
				node_id: 0,
				name: _tr("V"),
				type: "VALUE",
				color: 0xffa1a1a1,
				default_value: f32_array_create_x(0.0),
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
				default_value: f32_array_create_xyzw(0.8, 0.8, 0.8, 1.0),
				min: 0.0,
				max: 1.0,
				precision: 100,
				display: 0
			}
		],
		buttons: [],
		width: 0
	},
	{
		id: 0,
		name: _tr("Combine RGB"),
		type: "COMBRGB",
		x: 0,
		y: 0,
		color: 0xff62676d,
		inputs: [
			{
				id: 0,
				node_id: 0,
				name: _tr("R"),
				type: "VALUE",
				color: 0xffa1a1a1,
				default_value: f32_array_create_x(0.0),
				min: 0.0,
				max: 1.0,
				precision: 100,
				display: 0
			},
			{
				id: 0,
				node_id: 0,
				name: _tr("G"),
				type: "VALUE",
				color: 0xffa1a1a1,
				default_value: f32_array_create_x(0.0),
				min: 0.0,
				max: 1.0,
				precision: 100,
				display: 0
			},
			{
				id: 0,
				node_id: 0,
				name: _tr("B"),
				type: "VALUE",
				color: 0xffa1a1a1,
				default_value: f32_array_create_x(0.0),
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
				default_value: f32_array_create_xyzw(0.8, 0.8, 0.8, 1.0),
				min: 0.0,
				max: 1.0,
				precision: 100,
				display: 0
			}
		],
		buttons: [],
		width: 0
	},
	{
		id: 0,
		name: _tr("Combine XYZ"),
		type: "COMBXYZ",
		x: 0,
		y: 0,
		color: 0xff62676d,
		inputs: [
			{
				id: 0,
				node_id: 0,
				name: _tr("X"),
				type: "VALUE",
				color: 0xffa1a1a1,
				default_value: f32_array_create_x(0.0),
				min: 0.0,
				max: 1.0,
				precision: 100,
				display: 0
			},
			{
				id: 0,
				node_id: 0,
				name: _tr("Y"),
				type: "VALUE",
				color: 0xffa1a1a1,
				default_value: f32_array_create_x(0.0),
				min: 0.0,
				max: 1.0,
				precision: 100,
				display: 0
			},
			{
				id: 0,
				node_id: 0,
				name: _tr("Z"),
				type: "VALUE",
				color: 0xffa1a1a1,
				default_value: f32_array_create_x(0.0),
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
		buttons: [],
		width: 0
	},
	{
		id: 0,
		name: _tr("Map Range"),
		type: "MAPRANGE",
		x: 0,
		y: 0,
		color: 0xff62676d,
		inputs: [
			{
				id: 0,
				node_id: 0,
				name: _tr("Value"),
				type: "VALUE",
				color: 0xffa1a1a1,
				default_value: f32_array_create_x(0.5),
				min: 0.0,
				max: 1.0,
				precision: 100,
				display: 0
			},
			{
				id: 0,
				node_id: 0,
				name: _tr("From Min"),
				type: "VALUE",
				color: 0xffa1a1a1,
				default_value: f32_array_create_x(0.0),
				min: 0.0,
				max: 1.0,
				precision: 100,
				display: 0
			},
			{
				id: 0,
				node_id: 0,
				name: _tr("From Max"),
				type: "VALUE",
				color: 0xffa1a1a1,
				default_value: f32_array_create_x(1.0),
				min: 0.0,
				max: 1.0,
				precision: 100,
				display: 0
			},
			{
				id: 0,
				node_id: 0,
				name: _tr("To Min"),
				type: "VALUE",
				color: 0xffa1a1a1,
				default_value: f32_array_create_x(0.0),
				min: 0.0,
				max: 1.0,
				precision: 100,
				display: 0
			},
			{
				id: 0,
				node_id: 0,
				name: _tr("To Max"),
				type: "VALUE",
				color: 0xffa1a1a1,
				default_value: f32_array_create_x(1.0),
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
				name: _tr("Value"),
				type: "VALUE",
				color: 0xffa1a1a1,
				default_value: f32_array_create_x(0.0),
				min: 0.0,
				max: 1.0,
				precision: 100,
				display: 0
			}
		],
		buttons: [
			{
				name: _tr("use_clamp"),
				type: "BOOL",
				output: 0,
				default_value: f32_array_create_x(0),
				data: null,
				min: 0.0,
				max: 1.0,
				precision: 100,
				height: 0
			}
		],
		width: 0
	},
	{
		id: 0,
		name: _tr("Math"),
		type: "MATH",
		x: 0,
		y: 0,
		color: 0xff62676d,
		inputs: [
			{
				id: 0,
				node_id: 0,
				name: _tr("Value"),
				type: "VALUE",
				color: 0xffa1a1a1,
				default_value: f32_array_create_x(0.5),
				min: 0.0,
				max: 1.0,
				precision: 100,
				display: 0
			},
			{
				id: 0,
				node_id: 0,
				name: _tr("Value"),
				type: "VALUE",
				color: 0xffa1a1a1,
				default_value: f32_array_create_x(0.5),
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
				name: _tr("Value"),
				type: "VALUE",
				color: 0xffa1a1a1,
				default_value: f32_array_create_x(0.0),
				min: 0.0,
				max: 1.0,
				precision: 100,
				display: 0
			}
		],
		buttons: [
			{
				name: _tr("operation"),
				type: "ENUM",
				output: 0,
				default_value: f32_array_create_x(0),
				data: u8_array_create_from_string(_tr("Add") + "\n" + _tr("Subtract") + "\n" + _tr("Multiply") + "\n" + _tr("Divide") + "\n" + _tr("Power") + "\n" + _tr("Logarithm") + "\n" + _tr("Square Root") + "\n" + _tr("Inverse Square Root") + "\n" + _tr("Absolute") + "\n" + _tr("Exponent") + "\n" + _tr("Minimum") + "\n" + _tr("Maximum") + "\n" + _tr("Less Than") + "\n" + _tr("Greater Than") + "\n" + _tr("Sign") + "\n" + _tr("Round") + "\n" + _tr("Floor") + "\n" + _tr("Ceil") + "\n" + _tr("Truncate") + "\n" + _tr("Fraction") + "\n" + _tr("Modulo") + "\n" + _tr("Snap") + "\n" + _tr("Ping-Pong") + "\n" + _tr("Sine") + "\n" + _tr("Cosine") + "\n" + _tr("Tangent") + "\n" + _tr("Arcsine") + "\n" + _tr("Arccosine") + "\n" + _tr("Arctangent") + "\n" + _tr("Arctan2") + "\n" + _tr("Hyperbolic Sine") + "\n" + _tr("Hyperbolic Cosine") + "\n" + _tr("Hyperbolic Tangent") + "\n" + _tr("To Radians") + "\n" + _tr("To Degrees")),
				min: 0.0,
				max: 1.0,
				precision: 100,
				height: 0
			},
			{
				name: _tr("use_clamp"),
				type: "BOOL",
				output: 0,
				default_value: f32_array_create_x(0),
				data: null,
				min: 0.0,
				max: 1.0,
				precision: 100,
				height: 0
			}
		],
		width: 0
	},
	{
		id: 0,
		name: _tr("RGB to BW"),
		type: "RGBTOBW",
		x: 0,
		y: 0,
		color: 0xff62676d,
		inputs: [
			{
				id: 0,
				node_id: 0,
				name: _tr("Color"),
				type: "RGBA",
				color: 0xffc7c729,
				default_value: f32_array_create_xyzw(0.0, 0.0, 0.0, 0.0),
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
				name: _tr("Val"),
				type: "VALUE",
				color: 0xffa1a1a1,
				default_value: f32_array_create_x(0.0),
				min: 0.0,
				max: 1.0,
				precision: 100,
				display: 0
			}
		],
		buttons: [],
		width: 0
	},
	{
		id: 0,
		name: _tr("Separate HSV"),
		type: "SEPHSV",
		x: 0,
		y: 0,
		color: 0xff62676d,
		inputs: [
			{
				id: 0,
				node_id: 0,
				name: _tr("Color"),
				type: "RGBA",
				color: 0xffc7c729,
				default_value: f32_array_create_xyzw(0.5, 0.5, 0.5, 1.0),
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
				name: _tr("H"),
				type: "VALUE",
				color: 0xffa1a1a1,
				default_value: f32_array_create_x(0.0),
				min: 0.0,
				max: 1.0,
				precision: 100,
				display: 0
			},
			{
				id: 0,
				node_id: 0,
				name: _tr("S"),
				type: "VALUE",
				color: 0xffa1a1a1,
				default_value: f32_array_create_x(0.0),
				min: 0.0,
				max: 1.0,
				precision: 100,
				display: 0
			},
			{
				id: 0,
				node_id: 0,
				name: _tr("V"),
				type: "VALUE",
				color: 0xffa1a1a1,
				default_value: f32_array_create_x(0.0),
				min: 0.0,
				max: 1.0,
				precision: 100,
				display: 0
			}
		],
		buttons: [],
		width: 0
	},
	{
		id: 0,
		name: _tr("Separate RGB"),
		type: "SEPRGB",
		x: 0,
		y: 0,
		color: 0xff62676d,
		inputs: [
			{
				id: 0,
				node_id: 0,
				name: _tr("Color"),
				type: "RGBA",
				color: 0xffc7c729,
				default_value: f32_array_create_xyzw(0.8, 0.8, 0.8, 1.0),
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
				name: _tr("R"),
				type: "VALUE",
				color: 0xffa1a1a1,
				default_value: f32_array_create_x(0.0),
				min: 0.0,
				max: 1.0,
				precision: 100,
				display: 0
			},
			{
				id: 0,
				node_id: 0,
				name: _tr("G"),
				type: "VALUE",
				color: 0xffa1a1a1,
				default_value: f32_array_create_x(0.0),
				min: 0.0,
				max: 1.0,
				precision: 100,
				display: 0
			},
			{
				id: 0,
				node_id: 0,
				name: _tr("B"),
				type: "VALUE",
				color: 0xffa1a1a1,
				default_value: f32_array_create_x(0.0),
				min: 0.0,
				max: 1.0,
				precision: 100,
				display: 0
			}
		],
		buttons: [],
		width: 0
	},
	{
		id: 0,
		name: _tr("Separate XYZ"),
		type: "SEPXYZ",
		x: 0,
		y: 0,
		color: 0xff62676d,
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
				name: _tr("X"),
				type: "VALUE",
				color: 0xffa1a1a1,
				default_value: f32_array_create_x(0.0),
				min: 0.0,
				max: 1.0,
				precision: 100,
				display: 0
			},
			{
				id: 0,
				node_id: 0,
				name: _tr("Y"),
				type: "VALUE",
				color: 0xffa1a1a1,
				default_value: f32_array_create_x(0.0),
				min: 0.0,
				max: 1.0,
				precision: 100,
				display: 0
			},
			{
				id: 0,
				node_id: 0,
				name: _tr("Z"),
				type: "VALUE",
				color: 0xffa1a1a1,
				default_value: f32_array_create_x(0.0),
				min: 0.0,
				max: 1.0,
				precision: 100,
				display: 0
			}
		],
		buttons: [],
		width: 0
	},
	{
		id: 0,
		name: _tr("Vector Math"),
		type: "VECT_MATH",
		x: 0,
		y: 0,
		color: 0xff62676d,
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
			},
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
				name: _tr("Vector"),
				type: "VECTOR",
				color: 0xff6363c7,
				default_value: f32_array_create_xyz(0.0, 0.0, 0.0),
				min: 0.0,
				max: 1.0,
				precision: 100,
				display: 0
			},
			{
				id: 0,
				node_id: 0,
				name: _tr("Value"),
				type: "VALUE",
				color: 0xffa1a1a1,
				default_value: f32_array_create_x(0.0),
				min: 0.0,
				max: 1.0,
				precision: 100,
				display: 0
			}
		],
		buttons: [
			{
				name: _tr("operation"),
				type: "ENUM",
				output: 0,
				default_value: f32_array_create_x(0),
				data: u8_array_create_from_string(_tr("Add") + "\n" + _tr("Subtract") + "\n" + _tr("Multiply") + "\n" + _tr("Divide") + "\n" + _tr("Average") + "\n" + _tr("Cross Product") + "\n" + _tr("Project") + "\n" + _tr("Reflect") + "\n" + _tr("Dot Product") + "\n" + _tr("Distance") + "\n" + _tr("Length") + "\n" + _tr("Scale") + "\n" + _tr("Normalize") + "\n" + _tr("Absolute") + "\n" + _tr("Minimum") + "\n" + _tr("Maximum") + "\n" + _tr("Floor") + "\n" + _tr("Ceil") + "\n" + _tr("Fraction") + "\n" + _tr("Modulo") + "\n" + _tr("Snap") + "\n" + _tr("Sine") + "\n" + _tr("Cosine") + "\n" + _tr("Tangent")),
				min: 0.0,
				max: 1.0,
				precision: 100,
				height: 0
			}
		],
		width: 0
	}
];

let nodes_material_group: ui_node_t[] = [
	{
		id: 0,
		name: _tr("New Group"),
		type: "GROUP",
		x: 0,
		y: 0,
		color: 0xffb34f5a,
		inputs: [],
		outputs: [],
		buttons: [
			{
				name: "nodes_material_new_group_button",
				type: "CUSTOM",
				output: -1,
				default_value: f32_array_create_x(0),
				data: null,
				min: 0.0,
				max: 1.0,
				precision: 100,
				height: 1
			}
		],
		width: 0
	}
];

type node_list_t = ui_node_t[];

let nodes_material_list: node_list_t[] = [
	nodes_material_input,
	nodes_material_texture,
	nodes_material_color,
	nodes_material_vector,
	nodes_material_converter,
	nodes_material_group
];

function nodes_material_init() {
	ui_nodes_custom_buttons = map_create();
	map_set(ui_nodes_custom_buttons, "nodes_material_vector_curves_button", nodes_material_vector_curves_button);
	map_set(ui_nodes_custom_buttons, "nodes_material_color_ramp_button", nodes_material_color_ramp_button);
	map_set(ui_nodes_custom_buttons, "nodes_material_new_group_button", nodes_material_new_group_button);
	map_set(ui_nodes_custom_buttons, "nodes_material_group_input_button", nodes_material_group_input_button);
	map_set(ui_nodes_custom_buttons, "nodes_material_group_output_button", nodes_material_group_output_button);
}

function nodes_material_vector_curves_button(node_id: i32) {
	let ui: ui_t = ui_nodes_ui;
	let nodes: ui_nodes_t = ui_nodes_get_nodes();
	let node: ui_node_t = ui_get_node(ui_nodes_get_canvas(true).nodes, node_id);

	let but: ui_node_button_t = node.buttons[0];
	let nhandle: ui_handle_t = ui_nest(ui_handle(__ID__), node.id);
	let row: f32[] = [1 / 3, 1 / 3, 1 / 3];
	ui_row(row);
	ui_radio(ui_nest(ui_nest(nhandle, 0), 1), 0, "X");
	ui_radio(ui_nest(ui_nest(nhandle, 0), 1), 1, "Y");
	ui_radio(ui_nest(ui_nest(nhandle, 0), 1), 2, "Z");

	// Preview
	let axis: i32 = ui_nest(ui_nest(nhandle, 0), 1).position;
	let val: f32[] = but.default_value;
	ui._y += ui_nodes_LINE_H() * 5;

	let num: i32 = val[96 + axis];

	if (num == 0) {
		// Init
		val[96 + 0] = 1;
		val[96 + 1] = 1;
		val[96 + 2] = 1;
	}

	// Edit
	row = [1 / 5, 1 / 5, 3 / 5];
	ui_row(row);
	if (ui_button("+")) {
		val[axis * 32 + num * 2 + 0] = 0.0;
		val[axis * 32 + num * 2 + 1] = 0.0;
		num++;
		val[96 + axis] = num;
	}
	if (ui_button("-")) {
		if (num > 1) {
			num--;
			val[96 + axis] = num;
		}
	}

	let ihandle: ui_handle_t = ui_nest(ui_nest(ui_nest(nhandle, 0), 2), axis);
	if (ihandle.init) {
		ihandle.position = 0;
	}

	let i: i32 = math_floor(ui_slider(ihandle, "Index", 0, num - 1, false, 1, true, ui_align_t.LEFT));
	if (i >= num || i < 0) {
		ihandle.value = i = num - 1; // Stay in bounds
	}

	row = [1 / 2, 1 / 2];
	ui_row(row);
	ui_nest(ui_nest(nhandle, 0), 3).value = val[axis * 32 + i * 2 + 0];
	ui_nest(ui_nest(nhandle, 0), 4).value = val[axis * 32 + i * 2 + 1];

	let h1: ui_handle_t = ui_nest(ui_nest(nhandle, 0), 3);
	if (h1.init) {
		h1.value = 0.0;
	}
	let h2: ui_handle_t = ui_nest(ui_nest(nhandle, 0), 4);
	if (h2.init) {
		h2.value = 0.0;
	}
	val[axis * 32 + i * 2 + 0] = ui_slider(h1, "X", -1, 1, true, 100, true, ui_align_t.LEFT);
	val[axis * 32 + i * 2 + 1] = ui_slider(h2, "Y", -1, 1, true, 100, true, ui_align_t.LEFT);
}

function nodes_material_color_ramp_button(node_id: i32) {
	let ui: ui_t = ui_nodes_ui;
	let nodes: ui_nodes_t = ui_nodes_get_nodes();
	let node: ui_node_t = ui_get_node(ui_nodes_get_canvas(true).nodes, node_id);

	let but: ui_node_button_t = node.buttons[0];
	let nhandle: ui_handle_t = ui_nest(ui_handle(__ID__), node.id);
	let nx: f32 = ui._x;
	let ny: f32 = ui._y;

	// Preview
	let vals: f32[] = but.default_value; // [r, g, b, a, pos, r, g, b, a, pos, ..]
	let sw: f32 = ui._w / ui_nodes_SCALE();
	for (let i: i32 = 0; i < vals.length / 5; ++i) {
		let pos: f32 = vals[i * 5 + 4];
		let col: i32 = color_from_floats(vals[i * 5 + 0], vals[i * 5 + 1], vals[i * 5 + 2], 1.0);
		ui_fill(pos * sw, 0, (1.0 - pos) * sw, ui_nodes_LINE_H() - 2 * ui_nodes_SCALE(), col);
	}
	ui._y += ui_nodes_LINE_H();
	// Edit
	let ihandle: ui_handle_t = ui_nest(ui_nest(nhandle, 0), 2);
	let row: f32[] = [1 / 4, 1 / 4, 2 / 4];
	ui_row(row);
	if (ui_button("+")) {
		array_push(vals, vals[vals.length - 5]); // r
		array_push(vals, vals[vals.length - 5]); // g
		array_push(vals, vals[vals.length - 5]); // b
		array_push(vals, vals[vals.length - 5]); // a
		array_push(vals, 1.0); // pos
		ihandle.value += 1;
	}
	if (ui_button("-") && vals.length > 5) {
		array_pop(vals);
		array_pop(vals);
		array_pop(vals);
		array_pop(vals);
		array_pop(vals);
		ihandle.value -= 1;
	}

	let h: ui_handle_t = ui_nest(ui_nest(nhandle, 0), 1);
	if (h.init) {
		h.position = but.data[0];
	}
	let interpolate_combo: string[] = [tr("Linear"), tr("Constant")];
	but.data[0] = ui_combo(h, interpolate_combo, tr("Interpolate"));

	row = [1 / 2, 1 / 2];
	ui_row(row);
	let i: i32 = math_floor(ui_slider(ihandle, "Index", 0, (vals.length / 5) - 1, false, 1, true, ui_align_t.LEFT));
	if (i >= (vals.length * 5) || i < 0) {
		ihandle.value = i = (vals.length / 5) - 1; // Stay in bounds
	}

	ui_nest(ui_nest(nhandle, 0), 3).value = vals[i * 5 + 4];
	vals[i * 5 + 4] = ui_slider(ui_nest(ui_nest(nhandle, 0), 3), "Pos", 0, 1, true, 100, true, ui_align_t.LEFT);
	if (vals[i * 5 + 4] > 1.0) {
		vals[i * 5 + 4] = 1.0; // Stay in bounds
	}
	else if (vals[i * 5 + 4] < 0.0) {
		vals[i * 5 + 4] = 0.0;
	}

	let chandle: ui_handle_t = ui_nest(ui_nest(nhandle, 0), 4);
	chandle.color = color_from_floats(vals[i * 5 + 0], vals[i * 5 + 1], vals[i * 5 + 2], 1.0);
	if (ui_text("", ui_align_t.RIGHT, chandle.color) == ui_state_t.STARTED) {
		let rx: f32 = nx + ui._w - ui_nodes_p(37);
		let ry: f32 = ny - ui_nodes_p(5);
		nodes._input_started = ui.input_started = false;
		ui_nodes_rgba_popup(chandle, vals.buffer + i * 5, math_floor(rx), math_floor(ry + ui_ELEMENT_H(ui)));
	}
	vals[i * 5 + 0] = color_get_rb(chandle.color) / 255;
	vals[i * 5 + 1] = color_get_gb(chandle.color) / 255;
	vals[i * 5 + 2] = color_get_bb(chandle.color) / 255;
}

function nodes_material_new_group_button(node_id: i32) {
	let ui: ui_t = ui_nodes_ui;
	let nodes: ui_nodes_t = ui_nodes_get_nodes();
	let node: ui_node_t = ui_get_node(ui_nodes_get_canvas(true).nodes, node_id);

	if (node.name == "New Group") {
		for (let i: i32 = 1; i < 999; ++i) {
			node.name = tr("Group") + " " + i;

			let found: bool = false;
			for (let i: i32 = 0; i < project_material_groups.length; ++i) {
				let g: node_group_t = project_material_groups[i];
				let cname: string = g.canvas.name;
				if (cname == node.name) {
					found = true;
					break;
				}
			}
			if (!found) {
				break;
			}
		}

		let canvas: ui_node_canvas_t = {
			name: node.name,
			nodes: [
				{
					id: 0,
					name: _tr("Group Input"),
					type: "GROUP_INPUT",
					x: 50,
					y: 200,
					color: 0xff448c6d,
					inputs: [],
					outputs: [],
					buttons: [
						{
							name: "nodes_material_group_input_button",
							type: "CUSTOM",
							height: 1
						}
					]
				},
				{
					id: 1,
					name: _tr("Group Output"),
					type: "GROUP_OUTPUT",
					x: 450,
					y: 200,
					color: 0xff448c6d,
					inputs: [],
					outputs: [],
					buttons: [
						{
							name: "nodes_material_group_output_button",
							type: "CUSTOM",
							height: 1
						}
					]
				}
			],
			links: []
		};
		let ng: node_group_t = {
			canvas: canvas,
			nodes: ui_nodes_create()
		};
		array_push(project_material_groups, ng);
	}

	let group: node_group_t = null;
	for (let i: i32 = 0; i < project_material_groups.length; ++i) {
		let g: node_group_t = project_material_groups[i];
		let cname: string = g.canvas.name;
		if (cname == node.name) {
			group = g;
			break;
		}
	}

	if (ui_button(tr("Nodes"))) {
		array_push(ui_nodes_group_stack, group);
	}
}

function nodes_material_group_input_button(node_id: i32) {
	let ui: ui_t = ui_nodes_ui;
	let nodes: ui_nodes_t = ui_nodes_get_nodes();
	let node: ui_node_t = ui_get_node(ui_nodes_get_canvas(true).nodes, node_id);

	nodes_material_add_socket_button(ui, nodes, node, node.outputs);
}

function nodes_material_group_output_button(node_id: i32) {
	let ui: ui_t = ui_nodes_ui;
	let nodes: ui_nodes_t = ui_nodes_get_nodes();
	let node: ui_node_t = ui_get_node(ui_nodes_get_canvas(true).nodes, node_id);

	nodes_material_add_socket_button(ui, nodes, node, node.inputs);
}

let _nodes_material_nodes: ui_nodes_t;
let _nodes_material_node: ui_node_t;
let _nodes_material_sockets: ui_node_socket_t[];

function nodes_material_add_socket_button(ui: ui_t, nodes: ui_nodes_t, node: ui_node_t, sockets: ui_node_socket_t[]) {
	if (ui_button(tr("Add"))) {
		_nodes_material_nodes = nodes;
		_nodes_material_node = node;
		_nodes_material_sockets = sockets;
		ui_menu_draw(function (ui: ui_t) {
			let nodes: ui_nodes_t = _nodes_material_nodes;
			let node: ui_node_t = _nodes_material_node;
			let sockets: ui_node_socket_t[] = _nodes_material_sockets;

			let group_stack: node_group_t[] = ui_nodes_group_stack;
			let c: ui_node_canvas_t = group_stack[group_stack.length - 1].canvas;
			if (ui_menu_button(ui, tr("RGBA"))) {
				array_push(sockets, nodes_material_create_socket(nodes, node, null, "RGBA", c));
				nodes_material_sync_sockets(node);
			}
			if (ui_menu_button(ui, tr("Vector"))) {
				array_push(sockets, nodes_material_create_socket(nodes, node, null, "VECTOR", c));
				nodes_material_sync_sockets(node);
			}
			if (ui_menu_button(ui, tr("Value"))) {
				array_push(sockets, nodes_material_create_socket(nodes, node, null, "VALUE", c));
				nodes_material_sync_sockets(node);
			}
		});
	}
}

function nodes_material_sync_sockets(node: ui_node_t) {
	let group_stack: node_group_t[] = ui_nodes_group_stack;
	let c: ui_node_canvas_t = group_stack[group_stack.length - 1].canvas;
	for (let i: i32 = 0; i < project_materials.length; ++i) {
		let m: slot_material_t = project_materials[i];
		nodes_material_sync_group_sockets(m.canvas, c.name, node);
	}
	for (let i: i32 = 0; i < project_material_groups.length; ++i) {
		let g: node_group_t = project_material_groups[i];
		nodes_material_sync_group_sockets(g.canvas, c.name, node);
	}
}

function nodes_material_sync_group_sockets(canvas: ui_node_canvas_t, group_name: string, node: ui_node_t) {
	for (let i: i32 = 0; i < canvas.nodes.length; ++i) {
		let n: ui_node_t = canvas.nodes[i];
		if (n.type == "GROUP" && n.name == group_name) {
			let is_inputs: bool = node.name == "Group Input";
			let old_sockets: ui_node_socket_t[] = is_inputs ? n.inputs : n.outputs;
			let sockets: ui_node_socket_t[] = util_clone_canvas_sockets(is_inputs ? node.outputs : node.inputs);
			if (is_inputs) {
				n.inputs = sockets;
			}
			else {
				n.outputs = sockets;
			}
			for (let i: i32 = 0; i < sockets.length; ++i) {
				let s: ui_node_socket_t = sockets[i];
				s.node_id = n.id;
			}
			let num_sockets: i32 = sockets.length < old_sockets.length ? sockets.length : old_sockets.length;
			for (let i: i32 = 0; i < num_sockets; ++i) {
				if (sockets[i].type == old_sockets[i].type) {
					sockets[i].default_value = old_sockets[i].default_value;
				}
			}
		}
	}
}

function nodes_material_get_socket_color(type: string): i32 {
	return type == "RGBA" ? 0xffc7c729 : type == "VECTOR" ? 0xff6363c7 : 0xffa1a1a1;
}

function nodes_material_get_socket_default_value(type: string): f32_array_t {
	return type == "RGBA" ? f32_array_create_xyzw(0.8, 0.8, 0.8, 1.0) : type == "VECTOR" ? f32_array_create_xyz(0.0, 0.0, 0.0) : f32_array_create_x(0.0);
}

function nodes_material_get_socket_name(type: string): string {
	return type == "RGBA" ? _tr("Color") : type == "VECTOR" ? _tr("Vector") : _tr("Value");
}

function nodes_material_create_socket(nodes: ui_nodes_t, node: ui_node_t, name: string, type: string, canvas: ui_node_canvas_t, min: f32 = 0.0, max: f32 = 1.0, default_value: any = null): ui_node_socket_t {
	let soc: ui_node_socket_t = {
		id: ui_get_socket_id(canvas.nodes),
		node_id: node.id,
		name: name == null ? nodes_material_get_socket_name(type) : name,
		type: type,
		color: nodes_material_get_socket_color(type),
		default_value: default_value == null ? nodes_material_get_socket_default_value(type) : default_value,
		min: min,
		max: max
	};
	return soc;
}

function nodes_material_get_node_t(node_type: string): ui_node_t {
	for (let i: i32 = 0; i < nodes_material_list.length; ++i) {
		let c: ui_node_t[] = nodes_material_list[i];
		for (let i: i32 = 0; i < c.length; ++i) {
			let n: ui_node_t = c[i];
			if (n.type == node_type) {
				return n;
			}
		}
	}
	return null;
}

function nodes_material_create_node(node_type: string, group: node_group_t = null): ui_node_t {
	let n: ui_node_t = nodes_material_get_node_t(node_type);
	if (n == null) {
		return null;
	}
	let canvas: ui_node_canvas_t = group != null ? group.canvas : context_raw.material.canvas;
	let nodes: ui_nodes_t = group != null ? group.nodes : context_raw.material.nodes;
	let node: ui_node_t = ui_nodes_make_node(n, nodes, canvas);
	array_push(canvas.nodes, node);
	return node;
}
