package arm.shader;

import zui.Zui.Nodes;
import zui.Zui.TNodeCanvas;
import iron.SceneFormat;

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
