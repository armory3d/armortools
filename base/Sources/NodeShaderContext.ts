
class NodeShaderContext {
	vert: NodeShader;
	frag: NodeShader;
	data: TShaderContext;
	allow_vcols = false;
	material: TMaterial;
	constants: TShaderConstant[];
	tunits: TTextureUnit[];

	constructor(material: TMaterial, props: any) {
		this.material = material;
		this.data = {
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
			this.data.color_writes_red = props.color_writes_red;
		}
		if (props.color_writes_green != null) {
			this.data.color_writes_green = props.color_writes_green;
		}
		if (props.color_writes_blue != null) {
			this.data.color_writes_blue = props.color_writes_blue;
		}
		if (props.color_writes_alpha != null) {
			this.data.color_writes_alpha = props.color_writes_alpha;
		}

		this.tunits = this.data.texture_units = [];
		this.constants = this.data.constants = [];
	}

	add_elem = (name: string, data_type: string) => {
		for (let e of this.data.vertex_elements) {
			if (e.name == name) return;
		}
		let elem: TVertexElement = { name: name, data: data_type };
		this.data.vertex_elements.push(elem);
	}

	is_elem = (name: string): bool => {
		for (let elem of this.data.vertex_elements) {
			if (elem.name == name) {
				return true;
			}
		}
		return false;
	}

	get_elem = (name: string): TVertexElement => {
		for (let elem of this.data.vertex_elements) {
			if (elem.name == name) {
				return elem;
			}
		}
		return null;
	}

	add_constant = (ctype: string, name: string, link: string = null) => {
		for (let c of this.constants) {
			if (c.name == name) {
				return;
			}
		}

		let c: TShaderConstant = { name: name, type: ctype };
		if (link != null) {
			c.link = link;
		}
		this.constants.push(c);
	}

	add_texture_unit = (ctype: string, name: string, link: string = null, is_image = false) => {
		for (let c of this.tunits) {
			if (c.name == name) {
				return;
			}
		}

		let c: TTextureUnit = { name: name };
		if (link != null) {
			c.link = link;
		}
		if (is_image) {
			c.is_image = is_image;
		}
		this.tunits.push(c);
	}

	make_vert = (): NodeShader => {
		this.data.vertex_shader = this.material.name + '_' + this.data.name + '.vert';
		this.vert = new NodeShader(this, 'vert');
		return this.vert;
	}

	make_frag = (): NodeShader => {
		this.data.fragment_shader = this.material.name + '_' + this.data.name + '.frag';
		this.frag = new NodeShader(this, 'frag');
		return this.frag;
	}
}
