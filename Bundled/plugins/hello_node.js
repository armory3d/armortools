
let plugin = new arm.Plugin();

let categoryName = "Custom";
let nodeName = "Hello World";
let nodeType = "HELLO_WORLD";

// Create new node category
let categories = arm.NodesMaterial.categories;
categories.push("Custom");

// Create new node
let nodes = [
	{
		id: 0,
		name: nodeName,
		type: nodeType,
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
			}
		],
		buttons: []
	}
];
arm.NodesMaterial.list.push(nodes);

// Node shader
arm.Material.customNodes.set(nodeType, function() {
	return "vec3(1.0, 0.0, 0.0)";
});

// Cleanup
plugin.delete = function() {
	arm.Material.customNodes.delete(nodeType);
	arm.NodesMaterial.list.splice(arm.NodesMaterial.list.indexOf(nodes), 1);
	categories.splice(categories.indexOf(categoryName), 1);
};
