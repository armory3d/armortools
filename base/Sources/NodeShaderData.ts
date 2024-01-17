
class NodeShaderData {
	material: TMaterial;

	constructor(material: TMaterial) {
		this.material = material;
	}

	add_context = (props: any): NodeShaderContext => {
		return new NodeShaderContext(this.material, props);
	}
}

type TMaterial = {
	name: string;
	canvas: TNodeCanvas;
}
