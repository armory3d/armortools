
let plugin = plugin_create();

let category_name = "My Nodes";
let node_name = "Hello World";
let node_type = "HELLO_WORLD";

// Create new node category
let node_list = [
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
				default_value__f32: [1.0],
				min__f32: 0.0,
				max__f32: 5.0,
				precision__f32: 100.0,
				display: 0
			}
		],
		outputs: [
			{
				id: 0,
				node_id: 0,
				name: "Color",
				type: "RGBA",
				color: 0xffc7c729,
				default_value__f32: [0.8, 0.8, 0.8, 1.0],
				min__f32: 0.0,
				max__f32: 1.0,
				precision__f32: 100.0,
				display: 0
			},
			{
				id: 1,
				node_id: 0,
				name: "Fac",
				type: "VALUE",
				color: 0xffa1a1a1,
				default_value__f32: [1.0],
				min__f32: 0.0,
				max__f32: 1.0,
				precision__f32: 100.0,
				display: 0
			}
		],
		buttons: [],
		width: 0
	}
];

nodes_material_category_add(category_name, armpack_encode(node_list));

// Node shader
parser_material_custom_nodes_set(node_type, function(node, socket_name) {
	let frag = parser_material_frag_get();
	let scale = parser_material_parse_value_input(node, 0);
	let my_out = "my_out";

	node_shader_write(frag,
		"float " + my_out + " = cos(sin(tex_coord.x * 200.0 * " + scale + ") + cos(tex_coord.y * 200.0 * " + scale + "));"
	);

	if (socket_name == "Color") {
		return "vec3(" + my_out + ", " + my_out + ", " + my_out + ")";
	}
	else if (socket_name == "Fac") {
		return my_out;
	}
});

// Cleanup
plugin_notify_on_delete(plugin, function() {
	parser_material_custom_nodes_delete(node_type);
	nodes_material_category_remove(category_name);
});
