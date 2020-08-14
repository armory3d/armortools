package arm.shader;

import zui.Nodes;
import iron.data.SceneFormat;
import arm.shader.NodeShaderData;

class NodeShaderContext {
	public var vert: NodeShader;
	public var frag: NodeShader;
	public var geom: NodeShader;
	public var tesc: NodeShader;
	public var tese: NodeShader;
	public var data: TShaderContext;
	public var allow_vcols = false;
	var material: TMaterial;
	var constants: Array<TShaderConstant>;
	var tunits: Array<TTextureUnit>;

	public function new(material: TMaterial, props: Dynamic) {
		this.material = material;
		data = {
			name: props.name,
			depth_write: props.depth_write,
			compare_mode: props.compare_mode,
			cull_mode: props.cull_mode,
			blend_source: props.blend_source,
			blend_destination: props.blend_destination,
			blend_operation: props.blend_operation,
			alpha_blend_source: props.alpha_blend_source,
			alpha_blend_destination: props.alpha_blend_destination,
			alpha_blend_operation: props.alpha_blend_operation,
			fragment_shader: '',
			vertex_shader: '',
			vertex_elements: Reflect.hasField(props, 'vertex_elements') ? props.vertex_elements : [ {name: "pos", data: 'short4norm'}, {name: "nor", data: 'short2norm'}],
			color_attachments: props.color_attachments,
			depth_attachment: props.depth_attachment
		};

		if (props.color_writes_red != null)
			data.color_writes_red = props.color_writes_red;
		if (props.color_writes_green != null)
			data.color_writes_green = props.color_writes_green;
		if (props.color_writes_blue != null)
			data.color_writes_blue = props.color_writes_blue;
		if (props.color_writes_alpha != null)
			data.color_writes_alpha = props.color_writes_alpha;

		tunits = data.texture_units = [];
		constants = data.constants = [];
	}

	public function add_elem(name: String, data_type: String) {
		for (e in data.vertex_elements) {
			if (e.name == name) return;
		}
		var elem: TVertexElement = { name: name, data: data_type };
		data.vertex_elements.push(elem);
	}

	public function is_elem(name: String): Bool {
		for (elem in data.vertex_elements)
			if (elem.name == name)
				return true;
		return false;
	}

	public function get_elem(name: String): TVertexElement {
		for (elem in data.vertex_elements) {
			#if cpp
			if (Reflect.field(elem, "name") == name)
			#else
			if (elem.name == name)
			#end {
				return elem;
			}
		}
		return null;
	}

	public function add_constant(ctype: String, name: String, link: String = null) {
		for (c in constants)
			if (c.name == name)
				return;

		var c:TShaderConstant = { name: name, type: ctype };
		if (link != null)
			c.link = link;
		constants.push(c);
	}

	public function add_texture_unit(ctype: String, name: String, link: String = null, is_image = false) {
		for (c in tunits) {
			if (c.name == name) {
				return;
			}
		}

		var c: TTextureUnit = { name: name };
		if (link != null) {
			c.link = link;
		}
		if (is_image) {
			c.is_image = is_image;
		}
		tunits.push(c);
	}

	public function make_vert(): NodeShader {
		data.vertex_shader = material.name + '_' + data.name + '.vert';
		vert = new NodeShader(this, 'vert');
		return vert;
	}

	public function make_frag(): NodeShader {
		data.fragment_shader = material.name + '_' + data.name + '.frag';
		frag = new NodeShader(this, 'frag');
		return frag;
	}
}
