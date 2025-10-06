
// else if (node.type == "CURVE_RGB") { // RGB Curves
// 	let fac: string = parser_material_parse_value_input(node.inputs[0]);
// 	let vec: string = parser_material_parse_vector_input(node.inputs[1]);
// 	let curves: f32_array_t = node.buttons[0].default_value;
// 	let name: string = parser_material_node_name(node);
// 	// mapping.curves[0].points[0].handle_type
// 	let vc0: string = parser_material_vector_curve(name + "0", vec + ".x", curves[0]);
// 	let vc1: string = parser_material_vector_curve(name + "1", vec + ".y", curves[1]);
// 	let vc2: string = parser_material_vector_curve(name + "2", vec + ".z", curves[2]);
// 	let vc3a: string = parser_material_vector_curve(name + "3a", vec + ".x", curves[3]);
// 	let vc3b: string = parser_material_vector_curve(name + "3b", vec + ".y", curves[3]);
// 	let vc3c: string = parser_material_vector_curve(name + "3c", vec + ".z", curves[3]);
// 	return "(sqrt(float3(" + vc0 + ", " + vc1 + ", " + vc2 + ") * float3(" + vc3a + ", " + vc3b + ", " + vc3c + ")) * " + fac + ")";
// }
