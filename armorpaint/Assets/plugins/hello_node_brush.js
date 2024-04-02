
let plugin = plugin_create();

let category_name = "My Nodes";
let node_name = "Hello World";
let node_type = "HELLO_WORLD";

// Create new node category
let categories = nodes_brush_categories;
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
				default_value: 1.0,
				min: 0.0,
				max: 5.0
			}
		],
		outputs: [
			{
				id: 0,
				node_id: 0,
				name: "Value",
				type: "VALUE",
				color: 0xffc7c729,
				default_value: 1.0
			}
		],
		buttons: []
	}
];
nodes_brush_list.push(nodes);

// Brush node
parser_logic_custom_nodes.set(node_type, function(node, from) {
	return Math.sin(time_time() * node.inputs[0].get(0));
});

// Cleanup
plugin.delete = function() {
	parser_logic_custom_nodes.delete(node_type);
	nodes_brush_list.splice(nodes_brush_list.indexOf(nodes), 1);
	categories.splice(categories.indexOf(category_name), 1);
};
