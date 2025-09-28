
let nodes_material_categories: string[] = [
	_tr("Input"),
	_tr("Texture"),
	_tr("Color"),
	_tr("Vector"),
	_tr("Converter"),
	// _tr("Neural"),
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
		width: 0,
		flags: 0
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
		width: 0,
		flags: 0
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
		width: 0,
		flags: 0
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
		width: 0,
		flags: 0
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
		width: 0,
		flags: 0
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
		width: 0,
		flags: 0
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
		width: 0,
		flags: 0
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
		width: 0,
		flags: 0
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
		width: 0,
		flags: 0
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
		width: 0,
		flags: 0
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
		width: 0,
		flags: 0
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
		width: 0,
		flags: 0
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
		width: 0,
		flags: 0
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
		width: 0,
		flags: 0
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
		width: 0,
		flags: 0
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
		width: 0,
		flags: 0
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
		width: 0,
		flags: 0
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
		width: 0,
		flags: 0
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
//		width: 0,
// 		flags: 0
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
		width: 0,
		flags: 0
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
		width: 0,
		flags: 0
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
		width: 0,
		flags: 0
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
		width: 0,
		flags: 0
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
		width: 0,
		flags: 0
	},
	{
		id: 0,
		name: _tr("Text Texture"),
		type: "TEX_TEXT", // extension
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
				name: "text",
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
		width: 0,
		flags: 0
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
		width: 0,
		flags: 0
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
		width: 0,
		flags: 0
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
		width: 0,
		flags: 0
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
		width: 0,
		flags: 0
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
		width: 0,
		flags: 0
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
		width: 0,
		flags: 0
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
		width: 0,
		flags: 0
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
		width: 0,
		flags: 0
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
		width: 0,
		flags: 0
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
		width: 0,
		flags: 0
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
		width: 0,
		flags: 0
	},
	{
		id: 0,
		name: _tr("Quantize"),
		type: "QUANTIZE", // extension
		x: 0,
		y: 0,
		color: 0xff448c6d,
		inputs: [
			{
				id: 0,
				node_id: 0,
				name: _tr("Strength"),
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
		width: 0,
		flags: 0
	},
	{
		id: 0,
		name: _tr("Replace Color"),
		type: "REPLACECOL", // extension
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
				default_value: f32_array_create_xyzw(0.8, 0.8, 0.8, 1.0)
			},
			{
				id: 0,
				node_id: 0,
				name: _tr("Old Color"),
				type: "RGBA",
				color: 0xffc7c729,
				default_value: f32_array_create_xyzw(0.8, 0.8, 0.8, 1.0)
			},
			{
				id: 0,
				node_id: 0,
				name: _tr("New Color"),
				type: "RGBA",
				color: 0xffc7c729,
				default_value: f32_array_create_xyzw(0.8, 0.8, 0.8, 1.0)
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
				name: _tr("Color"),
				type: "RGBA",
				color: 0xffc7c729,
				default_value: f32_array_create_xyzw(0.8, 0.8, 0.8, 1.0)
			}
		],
		buttons: []
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
		width: 0,
		flags: 0
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
		width: 0,
		flags: 0
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
		width: 0,
		flags: 0
	},
	{
		id: 0,
		name: _tr("Mix Normal Map"),
		type: "MIX_NORMAL_MAP", // extension
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
		width: 0,
		flags: 0
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
		width: 0,
		flags: 0
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
		width: 0,
		flags: 0
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
		width: 0,
		flags: 0
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
		width: 0,
		flags: 0
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
		width: 0,
		flags: 0
	},
	{
		id: 0,
		name: _tr("Color Mask"),
		type: "COLMASK", // extension
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
		width: 0,
		flags: 0
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
		width: 0,
		flags: 0
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
		width: 0,
		flags: 0
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
		width: 0,
		flags: 0
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
		width: 0,
		flags: 0
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
		width: 0,
		flags: 0
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
		width: 0,
		flags: 0
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
		width: 0,
		flags: 0
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
		width: 0,
		flags: 0
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
		width: 0,
		flags: 0
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
				display: 1
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
		width: 0,
		flags: 0
	}
];

let nodes_material_neural: ui_node_t[] = [
	{
		id: 0,
		name: _tr("Text to Photo"),
		type: "text_to_photo_node",
		x: 0,
		y: 0,
		color: 0xff4982a0,
		inputs: [],
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
			}
		],
		buttons: [
			{
				name: _tr("tiling"),
				type: "BOOL",
				output: 0,
				default_value: f32_array_create_x(0),
				data: null,
				min: 0.0,
				max: 1.0,
				precision: 100,
				height: 0
			},
			{
				name: "text_to_photo_node_button",
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
		width: 0,
		flags: 0
	},
	{
		id: 0,
		name: _tr("Inpaint"),
		type: "inpaint_node",
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
				default_value: f32_array_create_xyzw(1.0, 1.0, 1.0, 1.0),
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
			}
		],
		buttons: [
			{
				name: _tr("auto"),
				type: "BOOL",
				output: 0,
				default_value: f32_array_create_x(1),
				data: null,
				min: 0.0,
				max: 1.0,
				precision: 100,
				height: 0
			},
			{
				name: "inpaint_node_button",
				type: "CUSTOM",
				output: -1,
				default_value: f32_array_create_x(0),
				data: null,
				min: 0.0,
				max: 1.0,
				precision: 100,
				height: 0
			}
		],
		width: 0,
		flags: 0
	},
	{
		id: 0,
		name: _tr("Photo to PBR"),
		type: "photo_to_pbr_node",
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
				default_value: f32_array_create_x(0.0),
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
		width: 0,
		flags: 0
	},
	{
		id: 0,
		name: _tr("Tiling"),
		type: "tiling_node",
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
				default_value: f32_array_create_xyzw(0.0, 0.0, 0.0, 1.0),
				min: 0.0,
				max: 1.0,
				precision: 100,
				display: 0
			}
		],
		buttons: [
			{
				name: _tr("auto"),
				type: "BOOL",
				output: 0,
				default_value: f32_array_create_x(1),
				data: null,
				min: 0.0,
				max: 1.0,
				precision: 100,
				height: 0
			},
			{
				name: "tiling_node_button",
				type: "CUSTOM",
				output: -1,
				default_value: f32_array_create_x(0),
				data: null,
				min: 0.0,
				max: 1.0,
				precision: 100,
				height: 0
			}
		],
		width: 0,
		flags: 0
	},
	{
		id: 0,
		name: _tr("Upscale"),
		type: "upscale_node",
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
				default_value: f32_array_create_xyzw(0.0, 0.0, 0.0, 1.0),
				min: 0.0,
				max: 1.0,
				precision: 100,
				display: 0
			}
		],
		buttons: [],
		width: 0,
		flags: 0
	},
	{
		id: 0,
		name: _tr("Variance"),
		type: "variance_node",
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
				default_value: f32_array_create_xyzw(0.0, 0.0, 0.0, 1.0),
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
				default_value: f32_array_create_xyzw(0.0, 0.0, 0.0, 1.0),
				min: 0.0,
				max: 1.0,
				precision: 100,
				display: 0
			}
		],
		buttons: [
			{
				name: "variance_node_button",
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
		width: 0,
		flags: 0
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
		width: 0,
		flags: 0
	}
];

type node_list_t = ui_node_t[];

let nodes_material_list: node_list_t[] = [
	nodes_material_input,
	nodes_material_texture,
	nodes_material_color,
	nodes_material_vector,
	nodes_material_converter,
	// nodes_material_neural,
	nodes_material_group
];
