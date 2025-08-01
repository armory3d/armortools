
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
				name: "Value",
				type: "VALUE",
				color: 0xffc7c729,
				default_value__f32: [1.0],
				min__f32: 0.0,
				max__f32: 5.0,
				precision__f32: 100.0,
				display: 0
			}
		],
		buttons: [],
		width: 0
	}
];

nodes_brush_category_add(category_name, armpack_encode(node_list));

// Brush node
parser_logic_custom_nodes_set(node_type, function(node, from) {
	return Math.sin(sys_time() * node.inputs[0].get(0));
});

// Cleanup
plugin_notify_on_delete(plugin, function() {
	parser_logic_custom_nodes_delete(node_type);
	nodes_brush_category_remove(category_name);
});
