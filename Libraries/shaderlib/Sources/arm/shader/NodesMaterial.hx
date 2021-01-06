package arm.shader;

import zui.Zui;
import zui.Id;
import zui.Nodes;
import arm.Enums;

class NodesMaterial {

	// Mark strings as localizable
	public static inline function _tr(s: String) { return s; }

	public static var categories = [_tr("Input"), _tr("Texture"), _tr("Color"), _tr("Vector"), _tr("Converter")];

	public static var list: Array<Array<TNode>> = [
		[ // Input
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
						default_value: f32([0.8, 0.8, 0.8, 1.0])
					},
					{
						id: 0,
						node_id: 0,
						name: _tr("Vector"),
						type: "VECTOR",
						color: 0xff6363c7,
						default_value: f32([0.0, 0.0, 0.0])
					},
					{
						id: 0,
						node_id: 0,
						name: _tr("Fac"),
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 0.0
					}
				],
				buttons: [
					{
						name: _tr("Name"),
						type: "STRING"
					}
				]
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
						default_value: f32([0.0, 0.0, 0.0])
					},
					{
						id: 0,
						node_id: 0,
						name: _tr("View Z Depth"),
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 0.0
					},
					{
						id: 0,
						node_id: 0,
						name: _tr("View Distance"),
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 0.0
					}
				],
				buttons: []
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
						default_value: 1.45,
						min: 0,
						max: 3
					},
					{
						id: 0,
						node_id: 0,
						name: _tr("Normal"),
						type: "VECTOR",
						color: 0xff6363c7,
						default_value: f32([0.0, 0.0, 0.0])
					}
				],
				outputs: [
					{
						id: 0,
						node_id: 0,
						name: _tr("Fac"),
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 0.0
					}
				],
				buttons: []
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
						default_value: f32([0.0, 0.0, 0.0])
					},
					{
						id: 0,
						node_id: 0,
						name: _tr("Normal"),
						type: "VECTOR",
						color: 0xff6363c7,
						default_value: f32([0.0, 0.0, 0.0])
					},
					{
						id: 0,
						node_id: 0,
						name: _tr("Tangent"),
						type: "VECTOR",
						color: 0xff6363c7,
						default_value: f32([0.0, 0.0, 0.0])
					},
					{
						id: 0,
						node_id: 0,
						name: _tr("True Normal"),
						type: "VECTOR",
						color: 0xff6363c7,
						default_value: f32([0.0, 0.0, 0.0])
					},
					{
						id: 0,
						node_id: 0,
						name: _tr("Incoming"),
						type: "VECTOR",
						color: 0xff6363c7,
						default_value: f32([0.0, 0.0, 0.0])
					},
					{
						id: 0,
						node_id: 0,
						name: _tr("Parametric"),
						type: "VECTOR",
						color: 0xff6363c7,
						default_value: f32([0.0, 0.0, 0.0])
					},
					{
						id: 0,
						node_id: 0,
						name: _tr("Backfacing"),
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 0.0
					},
					{
						id: 0,
						node_id: 0,
						name: _tr("Pointiness"),
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 0.0
					}
				],
				buttons: []
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
						default_value: f32([0.0, 0.0, 0.0, 1.0])
					},
					{
						id: 0,
						node_id: 0,
						name: _tr("Opacity"),
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 1.0
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
						default_value: 1.0
					},
					{
						id: 0,
						node_id: 0,
						name: _tr("Normal Map"),
						type: "VECTOR",
						color: -10238109,
						default_value: f32([0.5, 0.5, 1.0])
					},
					{
						id: 0,
						node_id: 0,
						name: _tr("Emission"),
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 1.0
					},
					{
						id: 0,
						node_id: 0,
						name: _tr("Height"),
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 1.0
					},
					{
						id: 0,
						node_id: 0,
						name: _tr("Subsurface"),
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 1.0
					}
				],
				buttons: [
					{
						name: _tr("Layer"),
						type: "ENUM",
						default_value: 0,
						data: ""
					}
				]
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
						default_value: 0.0
					}
				],
				buttons: [
					{
						name: _tr("Layer"),
						type: "ENUM",
						default_value: 0,
						data: ""
					}
				]
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
						default_value: 0.5
					},
					{
						id: 0,
						node_id: 0,
						name: _tr("Normal"),
						type: "VECTOR",
						color: 0xff6363c7,
						default_value: f32([0.0, 0.0, 0.0])
					}
				],
				outputs: [
					{
						id: 0,
						node_id: 0,
						name: _tr("Fresnel"),
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 0.0
					},
					{
						id: 0,
						node_id: 0,
						name: _tr("Facing"),
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 0.0
					}
				],
				buttons: []
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
						default_value: f32([0.0, 0.0, 0.0, 1.0])
					},
					{
						id: 0,
						node_id: 0,
						name: _tr("Opacity"),
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 1.0
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
						default_value: 1.0
					},
					{
						id: 0,
						node_id: 0,
						name: _tr("Normal Map"),
						type: "VECTOR",
						color: -10238109,
						default_value: f32([0.5, 0.5, 1.0])
					},
					{
						id: 0,
						node_id: 0,
						name: _tr("Emission"),
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 1.0
					},
					{
						id: 0,
						node_id: 0,
						name: _tr("Height"),
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 1.0
					},
					{
						id: 0,
						node_id: 0,
						name: _tr("Subsurface"),
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 1.0
					}
				],
				buttons: [
					{
						name: _tr("Material"),
						type: "ENUM",
						default_value: 0,
						data: ""
					}
				]
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
						default_value: f32([0.0, 0.0, 0.0])
					},
					{
						id: 0,
						node_id: 0,
						name: _tr("Object Index"),
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 0.0
					},
					{
						id: 0,
						node_id: 0,
						name: _tr("Material Index"),
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 0.0
					},
					{
						id: 0,
						node_id: 0,
						name: _tr("Random"),
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 0.0
					}
				],
				buttons: []
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
						default_value: f32([0.5, 0.5, 0.5, 1.0])
					}
				],
				buttons: [
					{
						name: _tr("default_value"),
						type: "RGBA",
						output: 0
					}
				]
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
						default_value: 0.5
					}
				],
				buttons: [
					{
						name: " ",
						type: "STRING",
						default_value: ""
					}
				]
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
						default_value: 0.5
					}
				],
				buttons: [
					{
						name: " ",
						type: "STRING",
						default_value: ""
					}
				]
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
						default_value: f32([0.0, 0.0, 0.0])
					}
				],
				buttons: []
			},
			{
				id: 0,
				name: _tr("Texture Coord"),
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
						default_value: f32([0.0, 0.0, 0.0])
					},
					{
						id: 0,
						node_id: 0,
						name: _tr("Normal"),
						type: "VECTOR",
						color: 0xff6363c7,
						default_value: f32([0.0, 0.0, 0.0])
					},
					{
						id: 0,
						node_id: 0,
						name: _tr("UV"),
						type: "VECTOR",
						color: 0xff6363c7,
						default_value: f32([0.0, 0.0, 0.0])
					},
					{
						id: 0,
						node_id: 0,
						name: _tr("Object"),
						type: "VECTOR",
						color: 0xff6363c7,
						default_value: f32([0.0, 0.0, 0.0])
					},
					{
						id: 0,
						node_id: 0,
						name: _tr("Camera"),
						type: "VECTOR",
						color: 0xff6363c7,
						default_value: f32([0.0, 0.0, 0.0])
					},
					{
						id: 0,
						node_id: 0,
						name: _tr("Window"),
						type: "VECTOR",
						color: 0xff6363c7,
						default_value: f32([0.0, 0.0, 0.0])
					},
					{
						id: 0,
						node_id: 0,
						name: _tr("Reflection"),
						type: "VECTOR",
						color: 0xff6363c7,
						default_value: f32([0.0, 0.0, 0.0])
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
						default_value: f32([0.0, 0.0, 0.0])
					}
				],
				buttons: []
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
						default_value: 0.5
					}
				],
				buttons: [
					{
						name: _tr("default_value"),
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
		// 				default_value: f32([0.8, 0.8, 0.8, 1.0])
		// 			},
		// 			{
		// 				id: 0,
		// 				node_id: 0,
		// 				name: _tr("Opacity"),
		// 				type: "VALUE",
		// 				color: 0xffa1a1a1,
		// 				default_value: 1.0
		// 			},
		// 			{
		// 				id: 0,
		// 				node_id: 0,
		// 				name: _tr("Occlusion"),
		// 				type: "VALUE",
		// 				color: 0xffa1a1a1,
		// 				default_value: 1.0
		// 			},
		// 			{
		// 				id: 0,
		// 				node_id: 0,
		// 				name: _tr("Roughness"),
		// 				type: "VALUE",
		// 				color: 0xffa1a1a1,
		// 				default_value: 0.1
		// 			},
		// 			{
		// 				id: 0,
		// 				node_id: 0,
		// 				name: _tr("Metallic"),
		// 				type: "VALUE",
		// 				color: 0xffa1a1a1,
		// 				default_value: 0.0
		// 			},
		// 			{
		// 				id: 0,
		// 				node_id: 0,
		// 				name: _tr("Normal Map"),
		// 				type: "VECTOR",
		// 				color: -10238109,
		// 				default_value: f32([0.5, 0.5, 1.0])
		// 			},
		// 			{
		// 				id: 0,
		// 				node_id: 0,
		// 				name: _tr("Emission"),
		// 				type: "VALUE",
		// 				color: 0xffa1a1a1,
		// 				default_value: 0.0
		// 			},
		// 			{
		// 				id: 0,
		// 				node_id: 0,
		// 				name: _tr("Height"),
		// 				type: "VALUE",
		// 				color: 0xffa1a1a1,
		// 				default_value: 0.0
		// 			},
		// 			{
		// 				id: 0,
		// 				node_id: 0,
		// 				name: _tr("Subsurface"),
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
						default_value: f32([0.0, 0.0, 0.0])
					},
					{
						id: 0,
						node_id: 0,
						name: _tr("Color 1"),
						type: "RGBA",
						color: 0xffc7c729,
						default_value: f32([0.8, 0.8, 0.8])
					},
					{
						id: 0,
						node_id: 0,
						name: _tr("Color 2"),
						type: "RGBA",
						color: 0xffc7c729,
						default_value: f32([0.2, 0.2, 0.2])
					},
					{
						id: 0,
						node_id: 0,
						name: _tr("Color 3"),
						type: "RGBA",
						color: 0xffc7c729,
						default_value: f32([0.0, 0.0, 0.0])
					},
					{
						id: 0,
						node_id: 0,
						name: _tr("Scale"),
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
						name: _tr("Color"),
						type: "RGBA",
						color: 0xffc7c729,
						default_value: f32([0.8, 0.8, 0.8, 1.0])
					},
					{
						id: 0,
						node_id: 0,
						name: _tr("Fac"),
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 1.0
					}
				],
				buttons: []
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
						default_value: f32([0.0, 0.0, 0.0])
					},
					{
						id: 0,
						node_id: 0,
						name: _tr("Color 1"),
						type: "RGBA",
						color: 0xffc7c729,
						default_value: f32([0.8, 0.8, 0.8])
					},
					{
						id: 0,
						node_id: 0,
						name: _tr("Color 2"),
						type: "RGBA",
						color: 0xffc7c729,
						default_value: f32([0.2, 0.2, 0.2])
					},
					{
						id: 0,
						node_id: 0,
						name: _tr("Scale"),
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
						name: _tr("Color"),
						type: "RGBA",
						color: 0xffc7c729,
						default_value: f32([0.8, 0.8, 0.8, 1.0])
					},
					{
						id: 0,
						node_id: 0,
						name: _tr("Fac"),
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 1.0
					}
				],
				buttons: []
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
						default_value: f32([0.8, 0.8, 0.8, 1.0])
					},
					{
						id: 0,
						node_id: 0,
						name: _tr("Fac"),
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 1.0
					}
				],
				buttons: [
					{
						name: _tr("gradient_type"),
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
						name: _tr("File"),
						type: "ENUM",
						default_value: 0,
						data: ""
					},
					{
						name: _tr("Color Space"),
						type: "ENUM",
						default_value: 0,
						data: ["linear", "srgb"]
					}
				]
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
						default_value: f32([0.0, 0.0, 0.0])
					},
					{
						id: 0,
						node_id: 0,
						name: _tr("Scale"),
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
						name: _tr("Color"),
						type: "RGBA",
						color: 0xffc7c729,
						default_value: f32([0.8, 0.8, 0.8, 1.0])
					},
					{
						id: 0,
						node_id: 0,
						name: _tr("Fac"),
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 1.0
					}
				],
				buttons: []
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
						default_value: f32([0.0, 0.0, 0.0])
					},
					{
						id: 0,
						node_id: 0,
						name: _tr("Scale"),
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
						name: _tr("Color"),
						type: "RGBA",
						color: 0xffc7c729,
						default_value: f32([0.8, 0.8, 0.8, 1.0])
					},
					{
						id: 0,
						node_id: 0,
						name: _tr("Fac"),
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 1.0
					}
				],
				buttons: []
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
						default_value: f32([0.0, 0.0, 0.0])
					},
					{
						id: 0,
						node_id: 0,
						name: _tr("Scale"),
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
						name: _tr("Color"),
						type: "RGBA",
						color: 0xffc7c729,
						default_value: f32([0.8, 0.8, 0.8, 1.0])
					},
					{
						id: 0,
						node_id: 0,
						name: _tr("Fac"),
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 1.0
					}
				],
				buttons: []
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
						default_value: f32([0.0, 0.0, 0.0])
					},
					{
						id: 0,
						node_id: 0,
						name: _tr("Scale"),
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
						name: _tr("Color"),
						type: "RGBA",
						color: 0xffc7c729,
						default_value: f32([0.8, 0.8, 0.8, 1.0])
					},
					{
						id: 0,
						node_id: 0,
						name: _tr("Fac"),
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 1.0
					}
				],
				buttons: [
					{
						name: _tr("coloring"),
						type: "ENUM",
						data: ["Intensity", "Cells"],
						default_value: 0,
						output: 0
					}
				]
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
						default_value: f32([0.0, 0.0, 0.0])
					},
					{
						id: 0,
						node_id: 0,
						name: _tr("Scale"),
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
						name: _tr("Color"),
						type: "RGBA",
						color: 0xffc7c729,
						default_value: f32([0.8, 0.8, 0.8, 1.0])
					},
					{
						id: 0,
						node_id: 0,
						name: _tr("Fac"),
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
						default_value: f32([0.8, 0.8, 0.8, 1.0])
					},
					{
						id: 0,
						node_id: 0,
						name: _tr("Angle"),
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 0.0,
						min: 0.0,
						max: 360.0
					},
					{
						id: 0,
						node_id: 0,
						name: _tr("Mask"),
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 0.5,
						min: 0.0,
						max: 1.0
					}
				],
				outputs: [
					{
						id: 0,
						node_id: 0,
						name: _tr("Color"),
						type: "RGBA",
						color: 0xffc7c729,
						default_value: f32([0.8, 0.8, 0.8, 1.0])
					}
				],
				buttons: []
			},
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
						default_value: f32([0.8, 0.8, 0.8, 1.0])
					},
					{
						id: 0,
						node_id: 0,
						name: _tr("Strength"),
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 0.5
					}
				],
				outputs: [
					{
						id: 0,
						node_id: 0,
						name: _tr("Color"),
						type: "RGBA",
						color: 0xffc7c729,
						default_value: f32([0.8, 0.8, 0.8, 1.0])
					}
				],
				buttons: []
			},
			{
				id: 0,
				name: _tr("BrightContrast"),
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
						default_value: f32([0.8, 0.8, 0.8, 1.0])
					},
					{
						id: 0,
						node_id: 0,
						name: _tr("Bright"),
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 0.0
					},
					{
						id: 0,
						node_id: 0,
						name: _tr("Contrast"),
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 0.0
					}
				],
				outputs: [
					{
						id: 0,
						node_id: 0,
						name: _tr("Color"),
						type: "RGBA",
						color: 0xffc7c729,
						default_value: f32([0.8, 0.8, 0.8, 1.0])
					}
				],
				buttons: []
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
						default_value: f32([0.8, 0.8, 0.8, 1.0])
					},
					{
						id: 0,
						node_id: 0,
						name: _tr("Gamma"),
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 1.0
					}
				],
				outputs: [
					{
						id: 0,
						node_id: 0,
						name: _tr("Color"),
						type: "RGBA",
						color: 0xffc7c729,
						default_value: f32([0.8, 0.8, 0.8, 1.0])
					}
				],
				buttons: []
			},
			{
				id: 0,
				name: _tr("HueSatVal"),
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
						default_value: 0.5
					},
					{
						id: 0,
						node_id: 0,
						name: _tr("Sat"),
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 1.0
					},
					{
						id: 0,
						node_id: 0,
						name: _tr("Val"),
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 1.0
					},
					{
						id: 0,
						node_id: 0,
						name: _tr("Fac"),
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 1.0
					},
					{
						id: 0,
						node_id: 0,
						name: _tr("Color"),
						type: "RGBA",
						color: 0xffc7c729,
						default_value: f32([0.8, 0.8, 0.8, 1.0])
					}
				],
				outputs: [
					{
						id: 0,
						node_id: 0,
						name: _tr("Color"),
						type: "RGBA",
						color: 0xffc7c729,
						default_value: f32([0.8, 0.8, 0.8, 1.0])
					}
				],
				buttons: []
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
						default_value: 1.0
					},
					{
						id: 0,
						node_id: 0,
						name: _tr("Color"),
						type: "RGBA",
						color: 0xffc7c729,
						default_value: f32([0.8, 0.8, 0.8, 1.0])
					}
				],
				outputs: [
					{
						id: 0,
						node_id: 0,
						name: _tr("Color"),
						type: "RGBA",
						color: 0xffc7c729,
						default_value: f32([0.8, 0.8, 0.8, 1.0])
					}
				],
				buttons: []
			},
			{
				id: 0,
				name: _tr("MixRGB"),
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
						default_value: 0.5
					},
					{
						id: 0,
						node_id: 0,
						name: _tr("Color1"),
						type: "RGBA",
						color: 0xffc7c729,
						default_value: f32([0.5, 0.5, 0.5, 1.0])
					},
					{
						id: 0,
						node_id: 0,
						name: _tr("Color2"),
						type: "RGBA",
						color: 0xffc7c729,
						default_value: f32([0.5, 0.5, 0.5, 1.0])
					}
				],
				outputs: [
					{
						id: 0,
						node_id: 0,
						name: _tr("Color"),
						type: "RGBA",
						color: 0xffc7c729,
						default_value: f32([0.8, 0.8, 0.8, 1.0])
					}
				],
				buttons: [
					{
						name: _tr("blend_type"),
						type: "ENUM",
						data: ["Mix", "Darken", "Multiply", "Burn", "Lighten", "Screen", "Dodge", "Add", "Overlay", "Soft Light", "Linear Light", "Difference", "Subtract", "Divide", "Hue", "Saturation", "Color", "Value"],
						default_value: 0,
						output: 0
					},
					{
						name: _tr("use_clamp"),
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
						default_value: 1.0
					},
					{
						id: 0,
						node_id: 0,
						name: _tr("Distance"),
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 0.0
					},
					{
						id: 0,
						node_id: 0,
						name: _tr("Height"),
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 1.0
					},
					{
						id: 0,
						node_id: 0,
						name: _tr("Normal"),
						type: "VECTOR",
						color: 0xff6363c7,
						default_value: f32([0.0, 0.0, 0.0])
					}
				],
				outputs: [
					{
						id: 0,
						node_id: 0,
						// name: _tr("Normal"),
						name: _tr("Normal Map"),
						type: "VECTOR",
						color: -10238109,
						default_value: f32([0.0, 0.0, 0.0])
					}
				],
				buttons: []
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
						default_value: f32([0.0, 0.0, 0.0])
					}
				],
				outputs: [
					{
						id: 0,
						node_id: 0,
						name: _tr("Vector"),
						type: "VECTOR",
						color: 0xff6363c7,
						default_value: f32([0.0, 0.0, 0.0])
					}
				],
				buttons: [
					{
						name: _tr("Location"),
						type: "VECTOR",
						default_value: f32([0.0, 0.0, 0.0]),
						output: 0
					},
					{
						name: _tr("Rotation"),
						type: "VECTOR",
						default_value: f32([0.0, 0.0, 0.0]),
						output: 0,
						max: 360.0
					},
					{
						name: _tr("Scale"),
						type: "VECTOR",
						default_value: f32([1.0, 1.0, 1.0]),
						output: 0
					}
				]
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
						default_value: f32([0.0, 0.0, 0.0])
					}
				],
				outputs: [
					{
						id: 0,
						node_id: 0,
						name: _tr("Normal"),
						type: "VECTOR",
						color: 0xff6363c7,
						default_value: f32([0.0, 0.0, 0.0])
					},
					{
						id: 0,
						node_id: 0,
						name: _tr("Dot"),
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 1.0
					}
				],
				buttons: [
					{
						name: _tr("Vector"),
						type: "VECTOR",
						default_value: f32([0.0, 0.0, 0.0]),
						output: 0
					}
				]
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
						default_value: 1.0
					},
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
						name: _tr("Vector"),
						type: "VECTOR",
						color: 0xff6363c7,
						default_value: f32([0.0, 0.0, 0.0])
					}
				],
				buttons: [
					{
						name: "arm.shader.NodesMaterial.vectorCurvesButton",
						type: "CUSTOM",
						default_value: [[f32([0.0, 0.0]), f32([0.0, 0.0])], [f32([0.0, 0.0]), f32([0.0, 0.0])], [f32([0.0, 0.0]), f32([0.0, 0.0])]],
						output: 0,
						height: 8.5
					}
				]
			}
		],
		[ // Converter
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
						default_value: 0.5
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
						default_value: 0.0
					}
				],
				buttons: [
					{
						name: "arm.shader.NodesMaterial.colorRampButton",
						type: "CUSTOM",
						default_value: [f32([1.0, 1.0, 1.0, 1.0, 0.0])],
						data: 0,
						output: 0,
						height: 4.5
					}
				]
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
						default_value: 0.0
					},
					{
						id: 0,
						node_id: 0,
						name: _tr("S"),
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 0.0
					},
					{
						id: 0,
						node_id: 0,
						name: _tr("V"),
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 0.0
					}
				],
				outputs: [
					{
						id: 0,
						node_id: 0,
						name: _tr("Color"),
						type: "RGBA",
						color: 0xffc7c729,
						default_value: f32([0.8, 0.8, 0.8, 1.0])
					}
				],
				buttons: []
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
						default_value: 0.0
					},
					{
						id: 0,
						node_id: 0,
						name: _tr("G"),
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 0.0
					},
					{
						id: 0,
						node_id: 0,
						name: _tr("B"),
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 0.0
					}
				],
				outputs: [
					{
						id: 0,
						node_id: 0,
						name: _tr("Color"),
						type: "RGBA",
						color: 0xffc7c729,
						default_value: f32([0.8, 0.8, 0.8, 1.0])
					}
				],
				buttons: []
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
						default_value: 0.0
					},
					{
						id: 0,
						node_id: 0,
						name: _tr("Y"),
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 0.0
					},
					{
						id: 0,
						node_id: 0,
						name: _tr("Z"),
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 0.0
					}
				],
				outputs: [
					{
						id: 0,
						node_id: 0,
						name: _tr("Vector"),
						type: "VECTOR",
						color: 0xff6363c7,
						default_value: f32([0.0, 0.0, 0.0])
					}
				],
				buttons: []
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
						default_value: 0.5
					},
					{
						id: 0,
						node_id: 0,
						name: _tr("Value"),
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 0.5
					}
				],
				outputs: [
					{
						id: 0,
						node_id: 0,
						name: _tr("Value"),
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 0.0
					}
				],
				buttons: [
					{
						name: _tr("operation"),
						type: "ENUM",
						data: ["Add", "Subtract", "Multiply", "Divide", "Power", "Logarithm", "Square Root", "Absolute", "Minimum", "Maximum", "Less Than", "Greater Than", "Round", "Floor", "Ceil", "Fract", "Modulo", "Sine", "Cosine", "Tangent", "Arcsine", "Arccosine", "Arctangent", "Arctan2"],
						default_value: 0,
						output: 0
					},
					{
						name: _tr("use_clamp"),
						type: "BOOL",
						default_value: false,
						output: 0
					}
				]
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
						default_value: f32([0.0, 0.0, 0.0, 0.0])
					}
				],
				outputs: [
					{
						id: 0,
						node_id: 0,
						name: _tr("Val"),
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 0.0
					}
				],
				buttons: []
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
						default_value: f32([0.5, 0.5, 0.5, 1.0])
					}
				],
				outputs: [
					{
						id: 0,
						node_id: 0,
						name: _tr("H"),
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 0.0
					},
					{
						id: 0,
						node_id: 0,
						name: _tr("S"),
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 0.0
					},
					{
						id: 0,
						node_id: 0,
						name: _tr("V"),
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 0.0
					}
				],
				buttons: []
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
						default_value: f32([0.8, 0.8, 0.8, 1.0])
					}
				],
				outputs: [
					{
						id: 0,
						node_id: 0,
						name: _tr("R"),
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 0.0
					},
					{
						id: 0,
						node_id: 0,
						name: _tr("G"),
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 0.0
					},
					{
						id: 0,
						node_id: 0,
						name: _tr("B"),
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 0.0
					}
				],
				buttons: []
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
						default_value: f32([0.0, 0.0, 0.0])
					}
				],
				outputs: [
					{
						id: 0,
						node_id: 0,
						name: _tr("X"),
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 0.0
					},
					{
						id: 0,
						node_id: 0,
						name: _tr("Y"),
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 0.0
					},
					{
						id: 0,
						node_id: 0,
						name: _tr("Z"),
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 0.0
					}
				],
				buttons: []
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
						default_value: f32([0.0, 0.0, 0.0])
					},
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
						name: _tr("Vector"),
						type: "VECTOR",
						color: 0xff6363c7,
						default_value: f32([0.0, 0.0, 0.0])
					},
					{
						id: 0,
						node_id: 0,
						name: _tr("Value"),
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 0.0
					}
				],
				buttons: [
					{
						name: _tr("operation"),
						type: "ENUM",
						data: ["Add", "Subtract", "Average", "Dot Product", "Cross Product", "Normalize"],
						default_value: 0,
						output: 0
					}
				]
			}
		]
	];

	@:keep
	@:access(zui.Zui)
	public static function vectorCurvesButton(ui: Zui, nodes: Nodes, node: TNode) {
		var but = node.buttons[0];
		var nhandle = Id.handle().nest(node.id);
		ui.row([1 / 3, 1 / 3, 1 / 3]);
		ui.radio(nhandle.nest(0).nest(1), 0, "X");
		ui.radio(nhandle.nest(0).nest(1), 1, "Y");
		ui.radio(nhandle.nest(0).nest(1), 2, "Z");
		// Preview
		var axis = nhandle.nest(0).nest(1).position;
		var val: Array<kha.arrays.Float32Array> = but.default_value[axis]; // [ [[x, y], [x, y], ..], [[x, y]], ..]
		var num = val.length;
		// for (i in 0...num) { ui.line(); }
		ui._y += nodes.LINE_H() * 5;
		// Edit
		ui.row([1 / 5, 1 / 5, 3 / 5]);
		if (ui.button("+")) {
			var f32 = new kha.arrays.Float32Array(2);
			f32[0] = 0; f32[1] = 0;
			val.push(f32);
		}
		if (ui.button("-")) {
			if (val.length > 2) { val.pop(); }
		}
		var i = Std.int(ui.slider(nhandle.nest(0).nest(2).nest(axis, {position: 0}), "Index", 0, num - 1, false, 1, true, Left));
		ui.row([1 / 2, 1 / 2]);
		nhandle.nest(0).nest(3).value = val[i][0];
		nhandle.nest(0).nest(4).value = val[i][1];
		val[i][0] = ui.slider(nhandle.nest(0).nest(3, {value: 0}), "X", -1, 1, true, 100, true, Left);
		val[i][1] = ui.slider(nhandle.nest(0).nest(4, {value: 0}), "Y", -1, 1, true, 100, true, Left);
	}

	@:keep
	@:access(zui.Zui)
	public static function colorRampButton(ui: Zui, nodes: Nodes, node: TNode) {
		var but = node.buttons[0];
		var nhandle = Id.handle().nest(node.id);
		var nx = ui._x;
		var ny = ui._y;

		// Preview
		var vals: Array<kha.arrays.Float32Array> = but.default_value; // [[r, g, b, a, pos], ..]
		var sw = ui._w / nodes.SCALE();
		for (val in vals) {
			var pos = val[4];
			var col = kha.Color.fromFloats(val[0], val[1], val[2]);
			ui.fill(pos * sw, 0, (1.0 - pos) * sw, nodes.LINE_H() - 2 * nodes.SCALE(), col);
		}
		ui._y += nodes.LINE_H();
		// Edit
		var ihandle = nhandle.nest(0).nest(2);
		ui.row([1 / 4, 1 / 4, 2 / 4]);
		if (ui.button("+")) {
			var last = vals[vals.length - 1];
			var f32 = new kha.arrays.Float32Array(5);
			f32[0] = last[0]; f32[1] = last[1]; f32[2] = last[2]; f32[3] = last[3]; f32[4] = 1.0;
			vals.push(f32);
			ihandle.value += 1;
		}
		if (ui.button("-") && vals.length > 1) {
			vals.pop();
			ihandle.value -= 1;
		}
		but.data = ui.combo(nhandle.nest(0).nest(1, {position: but.data}), ["Linear", "Constant"], "Interpolate");
		ui.row([1 / 2, 1 / 2]);
		var i = Std.int(ui.slider(ihandle, "Index", 0, vals.length - 1, false, 1, true, Left));
		var val = vals[i];
		nhandle.nest(0).nest(3).value = val[4];
		val[4] = ui.slider(nhandle.nest(0).nest(3), "Pos", 0, 1, true, 100, true, Left);
		var chandle = nhandle.nest(0).nest(4);
		chandle.color = kha.Color.fromFloats(val[0], val[1], val[2]);
		if (ui.text("", Right, chandle.color) == Started) {
			var rx = nx + ui._w - nodes.p(37);
			var ry = ny - nodes.p(5);
			nodes._inputStarted = ui.inputStarted = false;
			nodes.rgbaPopup(ui, chandle, val, Std.int(rx), Std.int(ry + ui.ELEMENT_H()));
		}
		val[0] = chandle.color.R;
		val[1] = chandle.color.G;
		val[2] = chandle.color.B;
	}

	public static function createNode(nodeType: String): TNode {
		for (c in list) {
			for (n in c) {
				if (n.type == nodeType) {
					var isScene = arm.ui.UIHeader.inst.worktab.position == SpaceRender;
					var material = isScene ? Context.materialScene : Context.material;
					var canvas = material.canvas;
					var nodes = material.nodes;
					var node = arm.ui.UINodes.makeNode(n, nodes, canvas);
					canvas.nodes.push(node);
					return node;
				}
			}
		}
		return null;
	}

	static function f32(ar: Array<kha.FastFloat>): kha.arrays.Float32Array {
		var res = new kha.arrays.Float32Array(ar.length);
		for (i in 0...ar.length) res[i] = ar[i];
		return res;
	}
}
