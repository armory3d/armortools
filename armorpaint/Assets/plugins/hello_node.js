
let plugin = Plugin.create();

let categoryName = "My Nodes";
let nodeName = "Hello World";
let nodeType = "HELLO_WORLD";

// Create new node category
let categories = NodesMaterial.categories;
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
NodesMaterial.list.push(nodes);

// Node shader
ParserMaterial.customNodes.set(nodeType, function(node, socket) {
	let frag = ParserMaterial.frag;
	let scale = ParserMaterial.parse_value_input(node.inputs[0]);
	let my_out = ParserMaterial.node_name(node) + "_out";

	NodeShader.write(frag, `
		float ${my_out} = cos(sin(texCoord.x * 200.0 * ${scale}) + cos(texCoord.y * 200.0 * ${scale}));
	`);

	if (socket.name == "Color") {
		return `vec3(${my_out}, ${my_out}, ${my_out})`;
	}
	else if (socket.name == "Fac") {
		return my_out;
	}
});

// Cleanup
plugin.delete = function() {
	ParserMaterial.customNodes.delete(nodeType);
	NodesMaterial.list.splice(NodesMaterial.list.indexOf(nodes), 1);
	categories.splice(categories.indexOf(categoryName), 1);
};
