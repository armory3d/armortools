
class NodeShaderContextRaw {
	vert: NodeShaderRaw;
	frag: NodeShaderRaw;
	data: shader_context_t;
	allow_vcols = false;
	material: TMaterial;
	constants: shader_const_t[];
	tunits: tex_unit_t[];
}

class NodeShaderContext {

	static create(material: TMaterial, props: any): NodeShaderContextRaw {
		let raw = new NodeShaderContextRaw();
		raw.material = material;
		raw.data = {
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
			vertex_elements: 'vertex_elements' in props ? props.vertex_elements : [ {name: "pos", data: 'short4norm'}, {name: "nor", data: 'short2norm'}],
			color_attachments: props.color_attachments,
			depth_attachment: props.depth_attachment
		};

		if (props.color_writes_red != null) {
			raw.data.color_writes_red = props.color_writes_red;
		}
		if (props.color_writes_green != null) {
			raw.data.color_writes_green = props.color_writes_green;
		}
		if (props.color_writes_blue != null) {
			raw.data.color_writes_blue = props.color_writes_blue;
		}
		if (props.color_writes_alpha != null) {
			raw.data.color_writes_alpha = props.color_writes_alpha;
		}

		raw.tunits = raw.data.texture_units = [];
		raw.constants = raw.data.constants = [];
		return raw;
	}

	static add_elem = (raw: NodeShaderContextRaw, name: string, data_type: string) => {
		for (let e of raw.data.vertex_elements) {
			if (e.name == name) return;
		}
		let elem: vertex_element_t = { name: name, data: data_type };
		raw.data.vertex_elements.push(elem);
	}

	static is_elem = (raw: NodeShaderContextRaw, name: string): bool => {
		for (let elem of raw.data.vertex_elements) {
			if (elem.name == name) {
				return true;
			}
		}
		return false;
	}

	static get_elem = (raw: NodeShaderContextRaw, name: string): vertex_element_t => {
		for (let elem of raw.data.vertex_elements) {
			if (elem.name == name) {
				return elem;
			}
		}
		return null;
	}

	static add_constant = (raw: NodeShaderContextRaw, ctype: string, name: string, link: string = null) => {
		for (let c of raw.constants) {
			if (c.name == name) {
				return;
			}
		}

		let c: shader_const_t = { name: name, type: ctype };
		if (link != null) {
			c.link = link;
		}
		raw.constants.push(c);
	}

	static add_texture_unit = (raw: NodeShaderContextRaw, ctype: string, name: string, link: string = null, is_image = false) => {
		for (let c of raw.tunits) {
			if (c.name == name) {
				return;
			}
		}

		let c: tex_unit_t = { name: name };
		if (link != null) {
			c.link = link;
		}
		if (is_image) {
			c.is_image = is_image;
		}
		raw.tunits.push(c);
	}

	static make_vert = (raw: NodeShaderContextRaw): NodeShaderRaw => {
		raw.data.vertex_shader = raw.material.name + '_' + raw.data.name + '.vert';
		raw.vert = NodeShader.create(raw, 'vert');
		return raw.vert;
	}

	static make_frag = (raw: NodeShaderContextRaw): NodeShaderRaw => {
		raw.data.fragment_shader = raw.material.name + '_' + raw.data.name + '.frag';
		raw.frag = NodeShader.create(raw, 'frag');
		return raw.frag;
	}
}

type TMaterial = {
	name: string;
	canvas: zui_node_canvas_t;
}
