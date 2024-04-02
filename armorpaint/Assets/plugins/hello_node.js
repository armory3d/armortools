
let plugin = plugin_create();

let category_name = "My Nodes";
let node_name = "Hello World";
let node_type = "HELLO_WORLD";

// Create new node category
let categories = nodes_material_categories;
categories.push(category_name);

// Create new node
let nodes = [
	{
		id: 0,
		name: node_name,
		type: node_type,
		x: 0,
		y: 0,
		color: 0xffb34f5a,
		inputs: [
			{
				id: 0,
				node_id: 0,
				name: "Scale",
				type: "VALUE",
				color: 0xffa1a1a1,
				default_value: 1,
				min: 0.0,
				max: 5.0
			}
		],
		outputs: [
			{
				id: 0,
				node_id: 0,
				name: "Color",
				type: "RGBA",
				color: 0xffc7c729,
				default_value: new Float32Array([0.8, 0.8, 0.8, 1.0])
			},
			{
				id: 1,
				node_id: 0,
				name: "Fac",
				type: "VALUE",
				color: 0xffa1a1a1,
				default_value: 1.0
			}
		],
		buttons: []
	}
];
nodes_material_list.push(nodes);

// Node shader
parser_material_custom_nodes.set(node_type, function(node, socket) {
	let frag = parser_material_frag;
	let scale = parser_material_parse_value_input(node.inputs[0]);
	let my_out = parser_material_node_name(node) + "_out";

	node_shader_write(frag,
		"float " + my_out + " = cos(sin(texCoord.x * 200.0 * " + scale + ") + cos(texCoord.y * 200.0 * " + scale + "));"
	);

	if (socket.name == "Color") {
		return "vec3(" + my_out + ", " + my_out + ", " + my_out + ")";
	}
	else if (socket.name == "Fac") {
		return my_out;
	}
});

// Cleanup
plugin.delete = function() {
	parser_material_custom_nodes.delete(node_type);
	nodes_material_list.splice(nodes_material_list.indexOf(nodes), 1);
	categories.splice(categories.indexOf(category_name), 1);
};
