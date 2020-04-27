
let plugin = new arm.Plugin();

let categoryName = "My Nodes";
let nodeName = "Hello World";
let nodeType = "HELLO_WORLD";

// Create new node category
let categories = arm.NodesBrush.categories;
categories.push(categoryName);

// Create new node
let nodes = [
	{
		id: 0,
		name: nodeName,
		type: nodeType,
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
arm.NodesBrush.list.push(nodes);

// Brush node
arm.Brush.customNodes.set(nodeType, function(node, from) {
	return Math.sin(arm.Scheduler.time() * node.inputs[0].get(0));
});

// Cleanup
plugin.delete = function() {
	arm.Brush.customNodes.delete(nodeType);
	arm.NodesBrush.list.splice(arm.NodesBrush.list.indexOf(nodes), 1);
	categories.splice(categories.indexOf(categoryName), 1);
};
