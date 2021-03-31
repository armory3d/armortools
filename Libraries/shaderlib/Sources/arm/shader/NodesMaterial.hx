package arm.shader;

import haxe.Json;
import zui.Zui;
import zui.Id;
import zui.Nodes;
import arm.Project;

class NodesMaterial {

	// Mark strings as localizable
	public static inline function _tr(s: String) { return s; }

	public static var categories = [_tr("Input"), _tr("Texture"), _tr("Color"), _tr("Vector"), _tr("Converter"), _tr("Group")];

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
					},
					{
						id: 0,
						node_id: 0,
						name: _tr("Random Per Island"),
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
						name: _tr("Color"),
						type: "RGBA",
						color: 0xffc7c729,
						default_value: f32([0.0, 0.0, 0.0, 1.0])
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
						default_value: f32([0.8, 0.8, 0.8, 1.0])
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
				buttons: []
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
						default_value: 0.01,
						max: 0.1
					},
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
				buttons: [
					{
						name: _tr("Pixel Size"),
						type: "BOOL",
						default_value: false,
						output: 0
					}
				]
			},
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
						name: _tr("Mortar"),
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
						default_value: 1.0,
						min: 0.0,
						max: 2.0
					},
					{
						id: 0,
						node_id: 0,
						name: _tr("Radius"),
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 1.0,
						min: 0.0,
						max: 2.0
					},
					{
						id: 0,
						node_id: 0,
						name: _tr("Offset"),
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 0.0,
						min: -2.0,
						max: 2.0
					}
				],
				outputs: [
					{
						id: 0,
						node_id: 0,
						name: _tr("Value"),
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
						data: [_tr("Linear"), _tr("Diagonal"), _tr("Radial"), _tr("Spherical")],
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
						data: [_tr("linear"), _tr("srgb")]
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
						name: _tr("Height"),
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
						data: [_tr("Intensity"), _tr("Cells")],
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
				name: _tr("Bright/Contrast"),
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
				name: _tr("Hue/Saturation"),
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
						name: _tr("Saturation"),
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 1.0
					},
					{
						id: 0,
						node_id: 0,
						name: _tr("Value"),
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
						default_value: f32([0.0, 0.0, 0.0, 1.0])
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
						data: [_tr("Mix"), _tr("Darken"), _tr("Multiply"), _tr("Burn"), _tr("Lighten"), _tr("Screen"), _tr("Dodge"), _tr("Add"), _tr("Overlay"), _tr("Soft Light"), _tr("Linear Light"), _tr("Difference"), _tr("Subtract"), _tr("Divide"), _tr("Hue"), _tr("Saturation"), _tr("Color"), _tr("Value")],
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
					},
					{
						id: 0,
						node_id: 0,
						name: _tr("Location"),
						type: "VECTOR",
						color: 0xff6363c7,
						default_value: f32([0.0, 0.0, 0.0]),
						display: 1
					},
					{
						id: 0,
						node_id: 0,
						name: _tr("Rotation"),
						type: "VECTOR",
						color: 0xff6363c7,
						default_value: f32([0.0, 0.0, 0.0]),
						max: 360.0,
						display: 1
					},
					{
						id: 0,
						node_id: 0,
						name: _tr("Scale"),
						type: "VECTOR",
						color: 0xff6363c7,
						default_value: f32([1.0, 1.0, 1.0]),
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
						default_value: f32([0.0, 0.0, 0.0])
					}
				],
				buttons: []
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
						data: [_tr("Add"), _tr("Subtract"), _tr("Multiply"), _tr("Divide"), _tr("Power"), _tr("Logarithm"), _tr("Square Root"), _tr("Absolute"), _tr("Minimum"), _tr("Maximum"), _tr("Less Than"), _tr("Greater Than"), _tr("Round"), _tr("Floor"), _tr("Ceil"), _tr("Fract"), _tr("Modulo"), _tr("Sine"), _tr("Cosine"), _tr("Tangent"), _tr("Arcsine"), _tr("Arccosine"), _tr("Arctangent"), _tr("Arctan2")],
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
						data: [_tr("Add"), _tr("Subtract"), _tr("Average"), _tr("Dot Product"), _tr("Cross Product"), _tr("Normalize")],
						default_value: 0,
						output: 0
					}
				]
			}
		],
		[ // Input
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
						name: "arm.shader.NodesMaterial.newGroupButton",
						type: "CUSTOM",
						height: 1
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
		but.data = ui.combo(nhandle.nest(0).nest(1, {position: but.data}), [tr("Linear"), tr("Constant")], tr("Interpolate"));
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

	@:keep
	public static function newGroupButton(ui: Zui, nodes: Nodes, node: TNode) {
		if (node.name == "New Group") {
			for (i in 1...999) {
				node.name = tr("Group") + " " + i;
				var found = false;
				for (g in Project.materialGroups) if (g.canvas.name == node.name) { found = true; break; }
				if (!found) break;
			}
			var canvas: TNodeCanvas = {
				name: node.name,
				nodes: [
					{
						id: 0,
						x: 50,
						y: 200,
						name: _tr("Group Input"),
						type: "GROUP_INPUT",
						inputs: [],
						outputs: [],
						buttons: [
							{
								name: "arm.shader.NodesMaterial.groupInputButton",
								type: "CUSTOM",
								height: 1
							}
						],
						color: 0xff448c6d
					},
					{
						id: 1,
						x: 450,
						y: 200,
						name: _tr("Group Output"),
						type: "GROUP_OUTPUT",
						inputs: [],
						outputs: [],
						buttons: [
							{
								name: "arm.shader.NodesMaterial.groupOutputButton",
								type: "CUSTOM",
								height: 1
							}
						],
						color: 0xff448c6d
					}
				],
				links: []
			};
			Project.materialGroups.push({ canvas: canvas, nodes: new Nodes() });
		}

		var group: TNodeGroup = null;
		for (g in Project.materialGroups) if (g.canvas.name == node.name) { group = g; break; }

		if (ui.button(tr("Nodes"))) {
			arm.ui.UINodes.inst.groupStack.push(group);
		}
	}

	@:keep
	public static function groupInputButton(ui: Zui, nodes: Nodes, node: TNode) {
		addSocketButton(ui, nodes, node, node.outputs);
	}

	@:keep
	public static function groupOutputButton(ui: Zui, nodes: Nodes, node: TNode) {
		addSocketButton(ui, nodes, node, node.inputs);
	}

	static function addSocketButton(ui: Zui, nodes: Nodes, node: TNode, sockets: Array<TNodeSocket>) {
		if (ui.button(tr("Add"))) {
			arm.ui.UIMenu.draw(function(ui: Zui) {
				ui.text(tr("Socket"), Right, ui.t.HIGHLIGHT_COL);
				var groupStack = arm.ui.UINodes.inst.groupStack;
				var c = groupStack[groupStack.length - 1].canvas;
				if (ui.button(tr("RGBA"), Left)) {
					sockets.push(createSocket(nodes, node, null, "RGBA", c));
					syncSockets(node);
				}
				if (ui.button(tr("Vector"), Left)) {
					sockets.push(createSocket(nodes, node, null, "VECTOR", c));
					syncSockets(node);
				}
				if (ui.button(tr("Value"), Left)) {
					sockets.push(createSocket(nodes, node, null, "VALUE", c));
					syncSockets(node);
				}
			}, 4);
		}
	}

	public static function syncSockets(node: TNode) {
		var groupStack = arm.ui.UINodes.inst.groupStack;
		var c = groupStack[groupStack.length - 1].canvas;
		for (m in Project.materials) syncGroupSockets(m.canvas, c.name, node);
		for (g in Project.materialGroups) syncGroupSockets(g.canvas, c.name, node);
	}

	static function syncGroupSockets(canvas: TNodeCanvas, groupName: String, node: TNode) {
		for (n in canvas.nodes) {
			if (n.type == "GROUP" && n.name == groupName) {
				var isInputs = node.name == "Group Input";
				var oldSockets = isInputs ? n.inputs : n.outputs;
				var sockets = Json.parse(Json.stringify(isInputs ? node.outputs : node.inputs));
				isInputs ? n.inputs = sockets : n.outputs = sockets;
				for (s in sockets) s.node_id = n.id;
				var numSockets = sockets.length < oldSockets.length ? sockets.length : oldSockets.length;
				for (i in 0...numSockets) {
					if (sockets[i].type == oldSockets[i].type) {
						sockets[i].default_value = oldSockets[i].default_value;
					}
				}
			}
		}
	}

	public static inline function get_socket_color(type: String): Int {
		return type == "RGBA" ? 0xffc7c729 : type == "VECTOR" ? 0xff6363c7 : 0xffa1a1a1;
	}

	public static inline function get_socket_default_value(type: String): Dynamic {
		return type == "RGBA" ? f32([0.8, 0.8, 0.8, 1.0]) : type == "VECTOR" ? f32([0.0, 0.0, 0.0]) : 0.0;
	}

	public static inline function get_socket_name(type: String): String {
		return type == "RGBA" ? _tr("Color") : type == "VECTOR" ? _tr("Vector") : _tr("Value");
	}

	public static function createSocket(nodes: Nodes, node: TNode, name: String, type: String, canvas: TNodeCanvas, min = 0.0, max = 1.0, default_value: Dynamic = null): TNodeSocket {
		return {
			id: nodes.getSocketId(canvas.nodes),
			node_id: node.id,
			name: name == null ? get_socket_name(type) : name,
			type: type,
			color: get_socket_color(type),
			default_value: default_value == null ? get_socket_default_value(type) : default_value,
			min: min,
			max: max
		}
	}

	public static function getTNode(nodeType: String): TNode {
		for (c in list) for (n in c) if (n.type == nodeType) return n;
		return null;
	}

	public static function createNode(nodeType: String, group: TNodeGroup = null): TNode {
		var n = getTNode(nodeType);
		if (n == null) return null;
		var canvas = group != null ? group.canvas : Context.material.canvas;
		var nodes = group != null ? group.nodes : Context.material.nodes;
		var node = arm.ui.UINodes.makeNode(n, nodes, canvas);
		canvas.nodes.push(node);
		return node;
	}

	static function f32(ar: Array<kha.FastFloat>): kha.arrays.Float32Array {
		var res = new kha.arrays.Float32Array(ar.length);
		for (i in 0...ar.length) res[i] = ar[i];
		return res;
	}
}
