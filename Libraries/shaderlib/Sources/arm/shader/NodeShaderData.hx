package arm.shader;

import zui.Nodes;
import iron.data.SceneFormat;

class NodeShaderData {
	var material: TMaterial;

	public function new(material: TMaterial) {
		this.material = material;
	}

	public function add_context(props: Dynamic): NodeShaderContext {
		return new NodeShaderContext(material, props);
	}
}

typedef TMaterial = {
	var name: String;
	var canvas: TNodeCanvas;
}
