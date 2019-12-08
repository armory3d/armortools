package arm.node;

import zui.Nodes;

class NodesMaterial {

	public static var categories = ["Input", "Texture", "Color", "Vector", "Converter"];

	public static var list: Array<Array<TNode>> = [
		[ // Input
			{
				id: 0,
				name: "Attribute",
				type: "ATTRIBUTE",
				x: 0,
				y: 0,
				color: 0xffb34f5a,
				inputs: [],
				outputs: [
					{
						id: 0,
						node_id: 0,
						name: "Color",
						type: "RGBA",
						color: 0xffc7c729,
						default_value: [0.8, 0.8, 0.8, 1.0]
					},
					{
						id: 0,
						node_id: 0,
						name: "Vector",
						type: "VECTOR",
						color: 0xff6363c7,
						default_value: [0.0, 0.0, 0.0]
					},
					{
						id: 0,
						node_id: 0,
						name: "Fac",
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 0.0
					}
				],
				buttons: [
					{
						name: "Name",
						type: "STRING"
					}
				]
			},
			{
				id: 0,
				name: "Camera Data",
				type: "CAMERA",
				x: 0,
				y: 0,
				color: 0xffb34f5a,
				inputs: [],
				outputs: [
					{
						id: 0,
						node_id: 0,
						name: "View Vector",
						type: "VECTOR",
						color: 0xff6363c7,
						default_value: [0.0, 0.0, 0.0]
					},
					{
						id: 0,
						node_id: 0,
						name: "View Z Depth",
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 0.0
					},
					{
						id: 0,
						node_id: 0,
						name: "View Distance",
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 0.0
					}
				],
				buttons: []
			},
			{
				id: 0,
				name: "Layer",
				type: "LAYER", // extension
				x: 0,
				y: 0,
				color: 0xff4982a0,
				inputs: [
					{
						id: 0,
						node_id: 0,
						name: "Vector",
						type: "VECTOR",
						color: 0xff6363c7,
						default_value: [0.0, 0.0, 0.0]
					}
				],
				outputs: [
					{
						id: 0,
						node_id: 0,
						name: "Base Color",
						type: "RGBA",
						color: 0xffc7c729,
						default_value: [0.0, 0.0, 0.0, 1.0]
					},
					{
						id: 0,
						node_id: 0,
						name: "Opacity",
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 1.0
					},
					{
						id: 0,
						node_id: 0,
						name: "Occlusion",
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 1.0
					},
					{
						id: 0,
						node_id: 0,
						name: "Roughness",
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 1.0
					},
					{
						id: 0,
						node_id: 0,
						name: "Metallic",
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 1.0
					},
					{
						id: 0,
						node_id: 0,
						name: "Normal Map",
						type: "VECTOR",
						color: -10238109,
						default_value: [0.5, 0.5, 1.0]
					},
					{
						id: 0,
						node_id: 0,
						name: "Emission",
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 1.0
					},
					{
						id: 0,
						node_id: 0,
						name: "Height",
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 1.0
					},
					{
						id: 0,
						node_id: 0,
						name: "Subsurface",
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 1.0
					}
				],
				buttons: [
					{
						name: "Layer",
						type: "ENUM",
						default_value: 0,
						data: ""
					}
				]
			},
			{
				id: 0,
				name: "Layer Mask",
				type: "LAYER_MASK", // extension
				x: 0,
				y: 0,
				color: 0xff4982a0,
				inputs: [
					{
						id: 0,
						node_id: 0,
						name: "Vector",
						type: "VECTOR",
						color: 0xff6363c7,
						default_value: [0.0, 0.0, 0.0]
					}
				],
				outputs: [
					{
						id: 0,
						node_id: 0,
						name: "Value",
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 0.0
					}
				],
				buttons: [
					{
						name: "Layer",
						type: "ENUM",
						default_value: 0,
						data: ""
					}
				]
			},
			{
				id: 0,
				name: "Material",
				type: "MATERIAL", // extension
				x: 0,
				y: 0,
				color: 0xff4982a0,
				inputs: [],
				outputs: [
					{
						id: 0,
						node_id: 0,
						name: "Base Color",
						type: "RGBA",
						color: 0xffc7c729,
						default_value: [0.0, 0.0, 0.0, 1.0]
					},
					{
						id: 0,
						node_id: 0,
						name: "Opacity",
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 1.0
					},
					{
						id: 0,
						node_id: 0,
						name: "Occlusion",
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 1.0
					},
					{
						id: 0,
						node_id: 0,
						name: "Roughness",
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 1.0
					},
					{
						id: 0,
						node_id: 0,
						name: "Metallic",
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 1.0
					},
					{
						id: 0,
						node_id: 0,
						name: "Normal Map",
						type: "VECTOR",
						color: -10238109,
						default_value: [0.5, 0.5, 1.0]
					},
					{
						id: 0,
						node_id: 0,
						name: "Emission",
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 1.0
					},
					{
						id: 0,
						node_id: 0,
						name: "Height",
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 1.0
					},
					{
						id: 0,
						node_id: 0,
						name: "Subsurface",
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 1.0
					}
				],
				buttons: [
					{
						name: "Material",
						type: "ENUM",
						default_value: 0,
						data: ""
					}
				]
			},
			{
				id: 0,
				name: "Fresnel",
				type: "FRESNEL",
				x: 0,
				y: 0,
				color: 0xffb34f5a,
				inputs: [
					{
						id: 0,
						node_id: 0,
						name: "IOR",
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 1.45,
						min: 0,
						max: 3
					},
					{
						id: 0,
						node_id: 0,
						name: "Normal",
						type: "VECTOR",
						color: 0xff6363c7,
						default_value: [0.0, 0.0, 0.0]
					}
				],
				outputs: [
					{
						id: 0,
						node_id: 0,
						name: "Fac",
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 0.0
					}
				],
				buttons: []
			},
			{
				id: 0,
				name: "Geometry",
				type: "NEW_GEOMETRY",
				x: 0,
				y: 0,
				color: 0xffb34f5a,
				inputs: [],
				outputs: [
					{
						id: 0,
						node_id: 0,
						name: "Position",
						type: "VECTOR",
						color: 0xff6363c7,
						default_value: [0.0, 0.0, 0.0]
					},
					{
						id: 0,
						node_id: 0,
						name: "Normal",
						type: "VECTOR",
						color: 0xff6363c7,
						default_value: [0.0, 0.0, 0.0]
					},
					{
						id: 0,
						node_id: 0,
						name: "Tangent",
						type: "VECTOR",
						color: 0xff6363c7,
						default_value: [0.0, 0.0, 0.0]
					},
					{
						id: 0,
						node_id: 0,
						name: "True Normal",
						type: "VECTOR",
						color: 0xff6363c7,
						default_value: [0.0, 0.0, 0.0]
					},
					{
						id: 0,
						node_id: 0,
						name: "Incoming",
						type: "VECTOR",
						color: 0xff6363c7,
						default_value: [0.0, 0.0, 0.0]
					},
					{
						id: 0,
						node_id: 0,
						name: "Parametric",
						type: "VECTOR",
						color: 0xff6363c7,
						default_value: [0.0, 0.0, 0.0]
					},
					{
						id: 0,
						node_id: 0,
						name: "Backfacing",
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 0.0
					},
					{
						id: 0,
						node_id: 0,
						name: "Pointiness",
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 0.0
					}
				],
				buttons: []
			},
			{
				id: 0,
				name: "Layer Weight",
				type: "LAYER_WEIGHT",
				x: 0,
				y: 0,
				color: 0xffb34f5a,
				inputs: [
					{
						id: 0,
						node_id: 0,
						name: "Blend",
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 0.5
					},
					{
						id: 0,
						node_id: 0,
						name: "Normal",
						type: "VECTOR",
						color: 0xff6363c7,
						default_value: [0.0, 0.0, 0.0]
					}
				],
				outputs: [
					{
						id: 0,
						node_id: 0,
						name: "Fresnel",
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 0.0
					},
					{
						id: 0,
						node_id: 0,
						name: "Facing",
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 0.0
					}
				],
				buttons: []
			},
			{
				id: 0,
				name: "Object Info",
				type: "OBJECT_INFO",
				x: 0,
				y: 0,
				color: 0xffb34f5a,
				inputs: [],
				outputs: [
					{
						id: 0,
						node_id: 0,
						name: "Location",
						type: "VECTOR",
						color: 0xff6363c7,
						default_value: [0.0, 0.0, 0.0]
					},
					{
						id: 0,
						node_id: 0,
						name: "Object Index",
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 0.0
					},
					{
						id: 0,
						node_id: 0,
						name: "Material Index",
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 0.0
					},
					{
						id: 0,
						node_id: 0,
						name: "Random",
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 0.0
					}
				],
				buttons: []
			},
			{
				id: 0,
				name: "RGB",
				type: "RGB",
				x: 0,
				y: 0,
				color: 0xffb34f5a,
				inputs: [],
				outputs: [
					{
						id: 0,
						node_id: 0,
						name: "Color",
						type: "RGBA",
						color: 0xffc7c729,
						default_value: [0.5, 0.5, 0.5, 1.0]
					}
				],
				buttons: [
					{
						name: "default_value",
						type: "RGBA",
						output: 0
					}
				]
			},
			{
				id: 0,
				name: "Tangent",
				type: "TANGENT",
				x: 0,
				y: 0,
				color: 0xffb34f5a,
				inputs: [],
				outputs: [
					{
						id: 0,
						node_id: 0,
						name: "Tangent",
						type: "VECTOR",
						color: 0xff6363c7,
						default_value: [0.0, 0.0, 0.0]
					}
				],
				buttons: []
			},
			{
				id: 0,
				name: "Texture Coord",
				type: "TEX_COORD",
				x: 0,
				y: 0,
				color: 0xffb34f5a,
				inputs: [],
				outputs: [
					{
						id: 0,
						node_id: 0,
						name: "Generated",
						type: "VECTOR",
						color: 0xff6363c7,
						default_value: [0.0, 0.0, 0.0]
					},
					{
						id: 0,
						node_id: 0,
						name: "Normal",
						type: "VECTOR",
						color: 0xff6363c7,
						default_value: [0.0, 0.0, 0.0]
					},
					{
						id: 0,
						node_id: 0,
						name: "UV",
						type: "VECTOR",
						color: 0xff6363c7,
						default_value: [0.0, 0.0, 0.0]
					},
					{
						id: 0,
						node_id: 0,
						name: "Object",
						type: "VECTOR",
						color: 0xff6363c7,
						default_value: [0.0, 0.0, 0.0]
					},
					{
						id: 0,
						node_id: 0,
						name: "Camera",
						type: "VECTOR",
						color: 0xff6363c7,
						default_value: [0.0, 0.0, 0.0]
					},
					{
						id: 0,
						node_id: 0,
						name: "Window",
						type: "VECTOR",
						color: 0xff6363c7,
						default_value: [0.0, 0.0, 0.0]
					},
					{
						id: 0,
						node_id: 0,
						name: "Reflection",
						type: "VECTOR",
						color: 0xff6363c7,
						default_value: [0.0, 0.0, 0.0]
					}
				],
				buttons: []
			},
			{
				id: 0,
				name: "UV Map",
				type: "UVMAP",
				x: 0,
				y: 0,
				color: 0xffb34f5a,
				inputs: [],
				outputs: [
					{
						id: 0,
						node_id: 0,
						name: "UV",
						type: "VECTOR",
						color: 0xff6363c7,
						default_value: [0.0, 0.0, 0.0]
					}
				],
				buttons: []
			},
			{
				id: 0,
				name: "Value",
				type: "VALUE",
				x: 0,
				y: 0,
				color: 0xffb34f5a,
				inputs: [],
				outputs: [
					{
						id: 0,
						node_id: 0,
						name: "Value",
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 0.5
					}
				],
				buttons: [
					{
						name: "default_value",
						type: "VALUE",
						output: 0,
						min: 0.0,
						max: 10.0
					}
				]
			}
		],
		// [ // Output
		// 	{
		// 		id: 0,
		// 		name: "Material Output",
		// 		type: "OUTPUT_MATERIAL_PBR",
		// 		x: 0,
		// 		y: 0,
		// 		color: 0xffb34f5a,
		// 		inputs: [
		// 			{
		// 				id: 0,
		// 				node_id: 0,
		// 				name: "Base Color",
		// 				type: "RGBA",
		// 				color: 0xffc7c729,
		// 				default_value: [0.8, 0.8, 0.8, 1.0]
		// 			},
		// 			{
		// 				id: 0,
		// 				node_id: 0,
		// 				name: "Opacity",
		// 				type: "VALUE",
		// 				color: 0xffa1a1a1,
		// 				default_value: 1.0
		// 			},
		// 			{
		// 				id: 0,
		// 				node_id: 0,
		// 				name: "Occlusion",
		// 				type: "VALUE",
		// 				color: 0xffa1a1a1,
		// 				default_value: 1.0
		// 			},
		// 			{
		// 				id: 0,
		// 				node_id: 0,
		// 				name: "Roughness",
		// 				type: "VALUE",
		// 				color: 0xffa1a1a1,
		// 				default_value: 0.1
		// 			},
		// 			{
		// 				id: 0,
		// 				node_id: 0,
		// 				name: "Metallic",
		// 				type: "VALUE",
		// 				color: 0xffa1a1a1,
		// 				default_value: 0.0
		// 			},
		// 			{
		// 				id: 0,
		// 				node_id: 0,
		// 				name: "Normal Map",
		// 				type: "VECTOR",
		// 				color: -10238109,
		// 				default_value: [0.5, 0.5, 1.0]
		// 			},
		// 			{
		// 				id: 0,
		// 				node_id: 0,
		// 				name: "Emission",
		// 				type: "VALUE",
		// 				color: 0xffa1a1a1,
		// 				default_value: 0.0
		// 			},
		// 			{
		// 				id: 0,
		// 				node_id: 0,
		// 				name: "Height",
		// 				type: "VALUE",
		// 				color: 0xffa1a1a1,
		// 				default_value: 0.0
		// 			},
		// 			{
		// 				id: 0,
		// 				node_id: 0,
		// 				name: "Subsurface",
		// 				type: "VALUE",
		// 				color: 0xffa1a1a1,
		// 				default_value: 0.0
		// 			}
		// 		],
		// 		outputs: [],
		// 		buttons: []
		// 	}
		// ],
		[ // Texture
			{
				id: 0,
				name: "Brick Texture",
				type: "TEX_BRICK",
				x: 0,
				y: 0,
				color: 0xff4982a0,
				inputs: [
					{
						id: 0,
						node_id: 0,
						name: "Vector",
						type: "VECTOR",
						color: 0xff6363c7,
						default_value: [0.0, 0.0, 0.0]
					},
					{
						id: 0,
						node_id: 0,
						name: "Color 1",
						type: "RGB",
						color: 0xffc7c729,
						default_value: [0.8, 0.8, 0.8]
					},
					{
						id: 0,
						node_id: 0,
						name: "Color 2",
						type: "RGB",
						color: 0xffc7c729,
						default_value: [0.2, 0.2, 0.2]
					},
					{
						id: 0,
						node_id: 0,
						name: "Color 3",
						type: "RGB",
						color: 0xffc7c729,
						default_value: [0.0, 0.0, 0.0]
					},
					{
						id: 0,
						node_id: 0,
						name: "Scale",
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 5.0,
						min: 0.0,
						max: 10.0
					}
				],
				outputs: [
					{
						id: 0,
						node_id: 0,
						name: "Color",
						type: "RGBA",
						color: 0xffc7c729,
						default_value: [0.8, 0.8, 0.8, 1.0]
					},
					{
						id: 0,
						node_id: 0,
						name: "Fac",
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 1.0
					}
				],
				buttons: []
			},
			{
				id: 0,
				name: "Checker Texture",
				type: "TEX_CHECKER",
				x: 0,
				y: 0,
				color: 0xff4982a0,
				inputs: [
					{
						id: 0,
						node_id: 0,
						name: "Vector",
						type: "VECTOR",
						color: 0xff6363c7,
						default_value: [0.0, 0.0, 0.0]
					},
					{
						id: 0,
						node_id: 0,
						name: "Color 1",
						type: "RGB",
						color: 0xffc7c729,
						default_value: [0.8, 0.8, 0.8]
					},
					{
						id: 0,
						node_id: 0,
						name: "Color 2",
						type: "RGB",
						color: 0xffc7c729,
						default_value: [0.2, 0.2, 0.2]
					},
					{
						id: 0,
						node_id: 0,
						name: "Scale",
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 5.0,
						min: 0.0,
						max: 10.0
					}
				],
				outputs: [
					{
						id: 0,
						node_id: 0,
						name: "Color",
						type: "RGBA",
						color: 0xffc7c729,
						default_value: [0.8, 0.8, 0.8, 1.0]
					},
					{
						id: 0,
						node_id: 0,
						name: "Fac",
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 1.0
					}
				],
				buttons: []
			},
			{
				id: 0,
				name: "Gradient Texture",
				type: "TEX_GRADIENT",
				x: 0,
				y: 0,
				color: 0xff4982a0,
				inputs: [
					{
						id: 0,
						node_id: 0,
						name: "Vector",
						type: "VECTOR",
						color: 0xff6363c7,
						default_value: [0.0, 0.0, 0.0]
					}
				],
				outputs: [
					{
						id: 0,
						node_id: 0,
						name: "Color",
						type: "RGBA",
						color: 0xffc7c729,
						default_value: [0.8, 0.8, 0.8, 1.0]
					},
					{
						id: 0,
						node_id: 0,
						name: "Fac",
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 1.0
					}
				],
				buttons: [
					{
						name: "gradient_type",
						type: "ENUM",
						// data: ["Linear", "Quadratic", "Easing", "Diagonal", "Radial", "Quadratic Sphere", "Spherical"],
						data: ["Linear", "Diagonal", "Radial", "Spherical"],
						default_value: 0,
						output: 0
					}
				]
			},
			{
				id: 0,
				name: "Image Texture",
				type: "TEX_IMAGE",
				x: 0,
				y: 0,
				color: 0xff4982a0,
				inputs: [
					{
						id: 0,
						node_id: 0,
						name: "Vector",
						type: "VECTOR",
						color: 0xff6363c7,
						default_value: [0.0, 0.0, 0.0]
					}
				],
				outputs: [
					{
						id: 0,
						node_id: 0,
						name: "Color",
						type: "RGBA",
						color: 0xffc7c729,
						default_value: [0.0, 0.0, 0.0, 1.0]
					},
					{
						id: 0,
						node_id: 0,
						name: "Alpha",
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 1.0
					}
				],
				buttons: [
					{
						name: "File",
						type: "ENUM",
						default_value: 0,
						data: ""
					},
					{
						name: "Color Space",
						type: "ENUM",
						default_value: 0,
						data: ["linear", "srgb"]
					}
				]
			},
			{
				id: 0,
				name: "Magic Texture",
				type: "TEX_MAGIC",
				x: 0,
				y: 0,
				color: 0xff4982a0,
				inputs: [
					{
						id: 0,
						node_id: 0,
						name: "Vector",
						type: "VECTOR",
						color: 0xff6363c7,
						default_value: [0.0, 0.0, 0.0]
					},
					{
						id: 0,
						node_id: 0,
						name: "Scale",
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 5.0,
						min: 0.0,
						max: 10.0
					}
				],
				outputs: [
					{
						id: 0,
						node_id: 0,
						name: "Color",
						type: "RGBA",
						color: 0xffc7c729,
						default_value: [0.8, 0.8, 0.8, 1.0]
					},
					{
						id: 0,
						node_id: 0,
						name: "Fac",
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 1.0
					}
				],
				buttons: []
			},
			{
				id: 0,
				name: "Musgrave Texture",
				type: "TEX_MUSGRAVE",
				x: 0,
				y: 0,
				color: 0xff4982a0,
				inputs: [
					{
						id: 0,
						node_id: 0,
						name: "Vector",
						type: "VECTOR",
						color: 0xff6363c7,
						default_value: [0.0, 0.0, 0.0]
					},
					{
						id: 0,
						node_id: 0,
						name: "Scale",
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 5.0,
						min: 0.0,
						max: 10.0
					}
				],
				outputs: [
					{
						id: 0,
						node_id: 0,
						name: "Color",
						type: "RGBA",
						color: 0xffc7c729,
						default_value: [0.8, 0.8, 0.8, 1.0]
					},
					{
						id: 0,
						node_id: 0,
						name: "Fac",
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 1.0
					}
				],
				buttons: []
			},
			{
				id: 0,
				name: "Noise Texture",
				type: "TEX_NOISE",
				x: 0,
				y: 0,
				color: 0xff4982a0,
				inputs: [
					{
						id: 0,
						node_id: 0,
						name: "Vector",
						type: "VECTOR",
						color: 0xff6363c7,
						default_value: [0.0, 0.0, 0.0]
					},
					{
						id: 0,
						node_id: 0,
						name: "Scale",
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 5.0,
						min: 0.0,
						max: 10.0
					}
				],
				outputs: [
					{
						id: 0,
						node_id: 0,
						name: "Color",
						type: "RGBA",
						color: 0xffc7c729,
						default_value: [0.8, 0.8, 0.8, 1.0]
					},
					{
						id: 0,
						node_id: 0,
						name: "Fac",
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 1.0
					}
				],
				buttons: []
			},
			{
				id: 0,
				name: "Voronoi Texture",
				type: "TEX_VORONOI",
				x: 0,
				y: 0,
				color: 0xff4982a0,
				inputs: [
					{
						id: 0,
						node_id: 0,
						name: "Vector",
						type: "VECTOR",
						color: 0xff6363c7,
						default_value: [0.0, 0.0, 0.0]
					},
					{
						id: 0,
						node_id: 0,
						name: "Scale",
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 5.0,
						min: 0.0,
						max: 10.0
					}
				],
				outputs: [
					{
						id: 0,
						node_id: 0,
						name: "Color",
						type: "RGBA",
						color: 0xffc7c729,
						default_value: [0.8, 0.8, 0.8, 1.0]
					},
					{
						id: 0,
						node_id: 0,
						name: "Fac",
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 1.0
					}
				],
				buttons: [
					{
						name: "coloring",
						type: "ENUM",
						data: ["Intensity", "Cells"],
						default_value: 0,
						output: 0
					}
				]
			},
			{
				id: 0,
				name: "Wave Texture",
				type: "TEX_WAVE",
				x: 0,
				y: 0,
				color: 0xff4982a0,
				inputs: [
					{
						id: 0,
						node_id: 0,
						name: "Vector",
						type: "VECTOR",
						color: 0xff6363c7,
						default_value: [0.0, 0.0, 0.0]
					},
					{
						id: 0,
						node_id: 0,
						name: "Scale",
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 5.0,
						min: 0.0,
						max: 10.0
					}
				],
				outputs: [
					{
						id: 0,
						node_id: 0,
						name: "Color",
						type: "RGBA",
						color: 0xffc7c729,
						default_value: [0.8, 0.8, 0.8, 1.0]
					},
					{
						id: 0,
						node_id: 0,
						name: "Fac",
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 1.0
					}
				],
				buttons: []
			}
		],
		[ // Color
			{
				id: 0,
				name: "BrightContrast",
				type: "BRIGHTCONTRAST",
				x: 0,
				y: 0,
				color: 0xff448c6d,
				inputs: [
					{
						id: 0,
						node_id: 0,
						name: "Color",
						type: "RGBA",
						color: 0xffc7c729,
						default_value: [0.8, 0.8, 0.8, 1.0]
					},
					{
						id: 0,
						node_id: 0,
						name: "Bright",
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 0.0
					},
					{
						id: 0,
						node_id: 0,
						name: "Contrast",
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 0.0
					}
				],
				outputs: [
					{
						id: 0,
						node_id: 0,
						name: "Color",
						type: "RGBA",
						color: 0xffc7c729,
						default_value: [0.8, 0.8, 0.8, 1.0]
					}
				],
				buttons: []
			},
			{
				id: 0,
				name: "Gamma",
				type: "GAMMA",
				x: 0,
				y: 0,
				color: 0xff448c6d,
				inputs: [
					{
						id: 0,
						node_id: 0,
						name: "Color",
						type: "RGBA",
						color: 0xffc7c729,
						default_value: [0.8, 0.8, 0.8, 1.0]
					},
					{
						id: 0,
						node_id: 0,
						name: "Gamma",
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 1.0
					}
				],
				outputs: [
					{
						id: 0,
						node_id: 0,
						name: "Color",
						type: "RGBA",
						color: 0xffc7c729,
						default_value: [0.8, 0.8, 0.8, 1.0]
					}
				],
				buttons: []
			},
			{
				id: 0,
				name: "Blur (Image)",
				type: "BLUR", // extension
				x: 0,
				y: 0,
				color: 0xff448c6d,
				inputs: [
					{
						id: 0,
						node_id: 0,
						name: "Color",
						type: "RGBA",
						color: 0xffc7c729,
						default_value: [0.8, 0.8, 0.8, 1.0]
					},
					{
						id: 0,
						node_id: 0,
						name: "Strength",
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 0.5
					}
				],
				outputs: [
					{
						id: 0,
						node_id: 0,
						name: "Color",
						type: "RGBA",
						color: 0xffc7c729,
						default_value: [0.8, 0.8, 0.8, 1.0]
					}
				],
				buttons: []
			},
			{
				id: 0,
				name: "HueSatVal",
				type: "HUE_SAT",
				x: 0,
				y: 0,
				color: 0xff448c6d,
				inputs: [
					{
						id: 0,
						node_id: 0,
						name: "Hue",
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 0.5
					},
					{
						id: 0,
						node_id: 0,
						name: "Sat",
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 1.0
					},
					{
						id: 0,
						node_id: 0,
						name: "Val",
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 1.0
					},
					{
						id: 0,
						node_id: 0,
						name: "Fac",
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 1.0
					},
					{
						id: 0,
						node_id: 0,
						name: "Color",
						type: "RGBA",
						color: 0xffc7c729,
						default_value: [0.8, 0.8, 0.8, 1.0]
					}
				],
				outputs: [
					{
						id: 0,
						node_id: 0,
						name: "Color",
						type: "RGBA",
						color: 0xffc7c729,
						default_value: [0.8, 0.8, 0.8, 1.0]
					}
				],
				buttons: []
			},
			{
				id: 0,
				name: "Invert",
				type: "INVERT",
				x: 0,
				y: 0,
				color: 0xff448c6d,
				inputs: [
					{
						id: 0,
						node_id: 0,
						name: "Fac",
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 1.0
					},
					{
						id: 0,
						node_id: 0,
						name: "Color",
						type: "RGBA",
						color: 0xffc7c729,
						default_value: [0.8, 0.8, 0.8, 1.0]
					}
				],
				outputs: [
					{
						id: 0,
						node_id: 0,
						name: "Color",
						type: "RGBA",
						color: 0xffc7c729,
						default_value: [0.8, 0.8, 0.8, 1.0]
					}
				],
				buttons: []
			},
			{
				id: 0,
				name: "MixRGB",
				type: "MIX_RGB",
				x: 0,
				y: 0,
				color: 0xff448c6d,
				inputs: [
					{
						id: 0,
						node_id: 0,
						name: "Fac",
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 0.5
					},
					{
						id: 0,
						node_id: 0,
						name: "Color1",
						type: "RGBA",
						color: 0xffc7c729,
						default_value: [0.5, 0.5, 0.5, 1.0]
					},
					{
						id: 0,
						node_id: 0,
						name: "Color2",
						type: "RGBA",
						color: 0xffc7c729,
						default_value: [0.5, 0.5, 0.5, 1.0]
					}
				],
				outputs: [
					{
						id: 0,
						node_id: 0,
						name: "Color",
						type: "RGBA",
						color: 0xffc7c729,
						default_value: [0.8, 0.8, 0.8, 1.0]
					}
				],
				buttons: [
					{
						name: "blend_type",
						type: "ENUM",
						data: ["Mix", "Darken", "Multiply", "Burn", "Lighten", "Screen", "Dodge", "Add", "Overlay", "Soft Light", "Linear Light", "Difference", "Subtract", "Divide", "Hue", "Saturation", "Color", "Value"],
						default_value: 0,
						output: 0
					},
					{
						name: "use_clamp",
						type: "BOOL",
						default_value: false,
						output: 0
					}
				]
			}
		],
		[ // Vector
			{
				id: 0,
				name: "Bump",
				type: "BUMP",
				x: 0,
				y: 0,
				color: 0xff522c99,
				inputs: [
					{
						id: 0,
						node_id: 0,
						name: "Strength",
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 1.0
					},
					{
						id: 0,
						node_id: 0,
						name: "Distance",
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 0.0
					},
					{
						id: 0,
						node_id: 0,
						name: "Height",
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 1.0
					},
					{
						id: 0,
						node_id: 0,
						name: "Normal",
						type: "VECTOR",
						color: 0xff6363c7,
						default_value: [0.0, 0.0, 0.0]
					}
				],
				outputs: [
					{
						id: 0,
						node_id: 0,
						// name: "Normal",
						name: "Normal Map",
						type: "VECTOR",
						color: -10238109,
						default_value: [0.0, 0.0, 0.0]
					}
				],
				buttons: []
			},
			{
				id: 0,
				name: "Mapping",
				type: "MAPPING",
				x: 0,
				y: 0,
				color: 0xff522c99,
				inputs: [
					{
						id: 0,
						node_id: 0,
						name: "Vector",
						type: "VECTOR",
						color: 0xff6363c7,
						default_value: [0.0, 0.0, 0.0]
					}
				],
				outputs: [
					{
						id: 0,
						node_id: 0,
						name: "Vector",
						type: "VECTOR",
						color: 0xff6363c7,
						default_value: [0.0, 0.0, 0.0]
					}
				],
				buttons: [
					{
						name: "Location",
						type: "VECTOR",
						default_value: [0.0, 0.0, 0.0],
						output: 0
					},
					{
						name: "Rotation",
						type: "VECTOR",
						default_value: [0.0, 0.0, 0.0],
						output: 0
					},
					{
						name: "Scale",
						type: "VECTOR",
						default_value: [1.0, 1.0, 1.0],
						output: 0
					}
				]
			},
			{
				id: 0,
				name: "Normal",
				type: "NORMAL",
				x: 0,
				y: 0,
				color: 0xff522c99,
				inputs: [
					{
						id: 0,
						node_id: 0,
						name: "Normal",
						type: "VECTOR",
						color: 0xff6363c7,
						default_value: [0.0, 0.0, 0.0]
					}
				],
				outputs: [
					{
						id: 0,
						node_id: 0,
						name: "Normal",
						type: "VECTOR",
						color: 0xff6363c7,
						default_value: [0.0, 0.0, 0.0]
					},
					{
						id: 0,
						node_id: 0,
						name: "Dot",
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 1.0
					}
				],
				buttons: [
					{
						name: "Vector",
						type: "VECTOR",
						default_value: [0.0, 0.0, 0.0],
						output: 0
					}
				]
			},
			{
				id: 0,
				name: "Vector Curves",
				type: "CURVE_VEC",
				x: 0,
				y: 0,
				color: 0xff522c99,
				inputs: [
					{
						id: 0,
						node_id: 0,
						name: "Fac",
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 1.0
					},
					{
						id: 0,
						node_id: 0,
						name: "Vector",
						type: "VECTOR",
						color: 0xff6363c7,
						default_value: [0.0, 0.0, 0.0]
					}
				],
				outputs: [
					{
						id: 0,
						node_id: 0,
						name: "Vector",
						type: "VECTOR",
						color: 0xff6363c7,
						default_value: [0.0, 0.0, 0.0]
					}
				],
				buttons: [
					{
						name: "Vector",
						type: "CURVES",
						default_value: [[0.0, 0.0, 0.0, 0.0], [0.0, 0.0, 0.0, 0.0], [0.0, 0.0, 0.0, 0.0]],
						output: 0
					}
				]
			}
		],
		[ // Converter
			{
				id: 0,
				name: "Color Ramp",
				type: "VALTORGB",
				x: 0,
				y: 0,
				color: 0xff62676d,
				inputs: [
					{
						id: 0,
						node_id: 0,
						name: "Fac",
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 0.5
					}
				],
				outputs: [
					{
						id: 0,
						node_id: 0,
						name: "Color",
						type: "RGBA",
						color: 0xffc7c729,
						default_value: [0.0, 0.0, 0.0, 1.0]
					},
					{
						id: 0,
						node_id: 0,
						name: "Alpha",
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 0.0
					}
				],
				buttons: [
					{
						name: "Ramp",
						type: "RAMP",
						default_value: [[1.0, 1.0, 1.0, 1.0, 0.0]],
						data: 0,
						output: 0
					}
				]
			},
			{
				id: 0,
				name: "Combine HSV",
				type: "COMBHSV",
				x: 0,
				y: 0,
				color: 0xff62676d,
				inputs: [
					{
						id: 0,
						node_id: 0,
						name: "H",
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 0.0
					},
					{
						id: 0,
						node_id: 0,
						name: "S",
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 0.0
					},
					{
						id: 0,
						node_id: 0,
						name: "V",
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 0.0
					}
				],
				outputs: [
					{
						id: 0,
						node_id: 0,
						name: "Color",
						type: "RGBA",
						color: 0xffc7c729,
						default_value: [0.8, 0.8, 0.8, 1.0]
					}
				],
				buttons: []
			},
			{
				id: 0,
				name: "Combine RGB",
				type: "COMBRGB",
				x: 0,
				y: 0,
				color: 0xff62676d,
				inputs: [
					{
						id: 0,
						node_id: 0,
						name: "R",
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 0.0
					},
					{
						id: 0,
						node_id: 0,
						name: "G",
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 0.0
					},
					{
						id: 0,
						node_id: 0,
						name: "B",
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 0.0
					}
				],
				outputs: [
					{
						id: 0,
						node_id: 0,
						name: "Color",
						type: "RGBA",
						color: 0xffc7c729,
						default_value: [0.8, 0.8, 0.8, 1.0]
					}
				],
				buttons: []
			},
			{
				id: 0,
				name: "Combine XYZ",
				type: "COMBXYZ",
				x: 0,
				y: 0,
				color: 0xff62676d,
				inputs: [
					{
						id: 0,
						node_id: 0,
						name: "X",
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 0.0
					},
					{
						id: 0,
						node_id: 0,
						name: "Y",
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 0.0
					},
					{
						id: 0,
						node_id: 0,
						name: "Z",
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 0.0
					}
				],
				outputs: [
					{
						id: 0,
						node_id: 0,
						name: "Vector",
						type: "VECTOR",
						color: 0xff6363c7,
						default_value: [0.0, 0.0, 0.0]
					}
				],
				buttons: []
			},
			{
				id: 0,
				name: "Math",
				type: "MATH",
				x: 0,
				y: 0,
				color: 0xff62676d,
				inputs: [
					{
						id: 0,
						node_id: 0,
						name: "Value",
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 0.5
					},
					{
						id: 0,
						node_id: 0,
						name: "Value",
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 0.5
					}
				],
				outputs: [
					{
						id: 0,
						node_id: 0,
						name: "Value",
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 0.0
					}
				],
				buttons: [
					{
						name: "operation",
						type: "ENUM",
						data: ["Add", "Subtract", "Multiply", "Divide", "Power", "Logarithm", "Square Root", "Absolute", "Minimum", "Maximum", "Less Than", "Greater Than", "Round", "Floor", "Ceil", "Fract", "Modulo", "Sine", "Cosine", "Tangent", "Arcsine", "Arccosine", "Arctangent", "Arctan2"],
						default_value: 0,
						output: 0
					},
					{
						name: "use_clamp",
						type: "BOOL",
						default_value: false,
						output: 0
					}
				]
			},
			{
				id: 0,
				name: "RGB to BW",
				type: "RGBTOBW",
				x: 0,
				y: 0,
				color: 0xff62676d,
				inputs: [
					{
						id: 0,
						node_id: 0,
						name: "Color",
						type: "RGBA",
						color: 0xffc7c729,
						default_value: [0.0, 0.0, 0.0, 0.0]
					}
				],
				outputs: [
					{
						id: 0,
						node_id: 0,
						name: "Val",
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 0.0
					}
				],
				buttons: []
			},
			{
				id: 0,
				name: "Separate HSV",
				type: "SEPHSV",
				x: 0,
				y: 0,
				color: 0xff62676d,
				inputs: [
					{
						id: 0,
						node_id: 0,
						name: "Color",
						type: "RGBA",
						color: 0xffc7c729,
						default_value: [0.5, 0.5, 0.5, 1.0]
					}
				],
				outputs: [
					{
						id: 0,
						node_id: 0,
						name: "H",
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 0.0
					},
					{
						id: 0,
						node_id: 0,
						name: "S",
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 0.0
					},
					{
						id: 0,
						node_id: 0,
						name: "V",
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 0.0
					}
				],
				buttons: []
			},
			{
				id: 0,
				name: "Separate RGB",
				type: "SEPRGB",
				x: 0,
				y: 0,
				color: 0xff62676d,
				inputs: [
					{
						id: 0,
						node_id: 0,
						name: "Color",
						type: "RGBA",
						color: 0xffc7c729,
						default_value: [0.8, 0.8, 0.8, 1.0]
					}
				],
				outputs: [
					{
						id: 0,
						node_id: 0,
						name: "R",
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 0.0
					},
					{
						id: 0,
						node_id: 0,
						name: "G",
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 0.0
					},
					{
						id: 0,
						node_id: 0,
						name: "B",
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 0.0
					}
				],
				buttons: []
			},
			{
				id: 0,
				name: "Separate XYZ",
				type: "SEPXYZ",
				x: 0,
				y: 0,
				color: 0xff62676d,
				inputs: [
					{
						id: 0,
						node_id: 0,
						name: "Vector",
						type: "VECTOR",
						color: 0xff6363c7,
						default_value: [0.0, 0.0, 0.0]
					}
				],
				outputs: [
					{
						id: 0,
						node_id: 0,
						name: "X",
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 0.0
					},
					{
						id: 0,
						node_id: 0,
						name: "Y",
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 0.0
					},
					{
						id: 0,
						node_id: 0,
						name: "Z",
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 0.0
					}
				],
				buttons: []
			},
			{
				id: 0,
				name: "Vector Math",
				type: "VECT_MATH",
				x: 0,
				y: 0,
				color: 0xff62676d,
				inputs: [
					{
						id: 0,
						node_id: 0,
						name: "Vector",
						type: "VECTOR",
						color: 0xff6363c7,
						default_value: [0.0, 0.0, 0.0]
					},
					{
						id: 0,
						node_id: 0,
						name: "Vector",
						type: "VECTOR",
						color: 0xff6363c7,
						default_value: [0.0, 0.0, 0.0]
					}
				],
				outputs: [
					{
						id: 0,
						node_id: 0,
						name: "Vector",
						type: "VECTOR",
						color: 0xff6363c7,
						default_value: [0.0, 0.0, 0.0]
					},
					{
						id: 0,
						node_id: 0,
						name: "Value",
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 0.0
					}
				],
				buttons: [
					{
						name: "operation",
						type: "ENUM",
						data: ["Add", "Subtract", "Average", "Dot Product", "Cross Product", "Normalize"],
						default_value: 0,
						output: 0
					}
				]
			}
		]
	];

	public static function createNode(nodeType: String): TNode {
		for (c in list) {
			for (n in c) {
				if (n.type == nodeType) {
					var canvas = Context.material.canvas;
					var nodes = Context.material.nodes;
					var node = arm.ui.UINodes.makeNode(n, nodes, canvas);
					canvas.nodes.push(node);
					return node;
				}
			}
		}
		return null;
	}
}
