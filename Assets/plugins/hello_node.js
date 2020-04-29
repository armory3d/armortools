
let plugin = new arm.Plugin();

let categoryName = "My Nodes";
let nodeName = "Hello World";
let nodeType = "HELLO_WORLD";

// Create new node category
let categories = arm.NodesMaterial.categories;
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
				default_value: [0.8, 0.8, 0.8, 1.0]
			}
		],
		buttons: []
	}
];
arm.NodesMaterial.list.push(nodes);

// Node shader
arm.Material.customNodes.set(nodeType, function(node) {
	let frag = arm.Material.frag;
	let scale = arm.Material.parse_value_input(node.inputs[0]);

	frag.write(`
		float my_out = cos(sin(texCoord.x * 200.0 * ${scale}) + cos(texCoord.y * 200.0 * ${scale}));
	`);
	return `vec3(my_out, my_out, my_out)`;
});

// Cleanup
plugin.delete = function() {
	arm.Material.customNodes.delete(nodeType);
	arm.NodesMaterial.list.splice(arm.NodesMaterial.list.indexOf(nodes), 1);
	categories.splice(categories.indexOf(categoryName), 1);
};
