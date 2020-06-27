
let plugin = new arm.Plugin();

let categoryName = "MixNormalMap";
let nodeName = "MixNormalMap";
let nodeType = "MixNode";

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
				name: "Base NormalMap",
				type: "VECTOR",
				color: 0xff6363c7,
				default_value: [0.0, 0.0, 0.0, 1.0]
			},
			{
				id: 0,
				node_id: 0,
				name: "Detail NormalMap",
				type: "VECTOR",
				color: 0xff6363c7,
				default_value: [0.0, 0.0, 0.0, 1.0]
			}
		],
		outputs: [
			{
				id: 0,
				node_id: 0,
				name: "MixNormalMap",
				type: "VECTOR",
				color: 0xff6363c7,
				default_value: [0.0, 0.0, 0.0, 1.0]
			}
		],
		buttons: []
	}
];
arm.NodesMaterial.list.push(nodes);

// Node shader
arm.Material.customNodes.set(nodeType, function(node) {
	let frag = arm.Material.frag;
	let BaseNormal = arm.Material.parse_vector_input(node.inputs[0]);
	let DetailNornal = arm.Material.parse_vector_input(node.inputs[1]);

	frag.write(`
		vec3 my_input1 = ${BaseNormal};
		vec3 my_input2 = ${DetailNornal};
		vec3 t = my_input1*vec3(2,2,2) + vec3(-1,-1,0);
		vec3 u = my_input2*vec3(-2,-2,2) + vec3(1,1,-1);
		vec3 r = normalize(t*dot(t,u)-u*t.z);
		
	`);
		return `vec3(r.x, r.y, r.z)*0.5 + 0.5`;
});

// Cleanup
plugin.delete = function() {
	arm.Material.customNodes.delete(nodeType);
	arm.NodesMaterial.list.splice(arm.NodesMaterial.list.indexOf(nodes), 1);
	categories.splice(categories.indexOf(categoryName), 1);
};