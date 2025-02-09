
type node_shader_context_t = {
	vert?: node_shader_t;
	frag?: node_shader_t;
	data?: shader_context_t;
	allow_vcols?: bool;
	material?: material_t;
	constants?: shader_const_t[];
	tunits?: tex_unit_t[];
};

function node_shader_context_create(material: material_t, props: shader_context_t): node_shader_context_t {
	let raw: node_shader_context_t = {};
	raw.material = material;

	let vertex_elements_default: vertex_element_t[] = [
		{
			name: "pos",
			data: "short4norm"
		},
		{
			name: "nor",
			data: "short2norm"
		}
	];

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
		fragment_shader: "",
		vertex_shader: "",
		vertex_elements: props.vertex_elements != null ? props.vertex_elements : vertex_elements_default,
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

function node_shader_context_add_elem(raw: node_shader_context_t, name: string, data_type: string) {
	for (let i: i32 = 0; i < raw.data.vertex_elements.length; ++i) {
		let e: vertex_element_t = raw.data.vertex_elements[i];
		if (e.name == name) {
			return;
		}
	}
	let elem: vertex_element_t = { name: name, data: data_type };
	array_push(raw.data.vertex_elements, elem);
}

function node_shader_context_is_elem(raw: node_shader_context_t, name: string): bool {
	for (let i: i32 = 0; i < raw.data.vertex_elements.length; ++i) {
		let elem: vertex_element_t = raw.data.vertex_elements[i];
		if (elem.name == name) {
			return true;
		}
	}
	return false;
}

function node_shader_context_get_elem(raw: node_shader_context_t, name: string): vertex_element_t {
	for (let i: i32 = 0; i < raw.data.vertex_elements.length; ++i) {
		let elem: vertex_element_t = raw.data.vertex_elements[i];
		if (elem.name == name) {
			return elem;
		}
	}
	return null;
}

function node_shader_context_add_constant(raw: node_shader_context_t, ctype: string, name: string, link: string = null) {
	for (let i: i32 = 0; i < raw.constants.length; ++i) {
		let c: shader_const_t = raw.constants[i];
		if (c.name == name) {
			return;
		}
	}

	let c: shader_const_t = { name: name, type: ctype };
	if (link != null) {
		c.link = link;
	}
	array_push(raw.constants, c);
}

function node_shader_context_add_texture_unit(raw: node_shader_context_t, ctype: string, name: string, link: string = null) {
	for (let i: i32 = 0; i < raw.tunits.length; ++i) {
		let c: tex_unit_t = raw.tunits[i];
		if (c.name == name) {
			return;
		}
	}

	let c: tex_unit_t = { name: name };
	if (link != null) {
		c.link = link;
	}
	array_push(raw.tunits, c);
}

function node_shader_context_make_vert(raw: node_shader_context_t): node_shader_t {
	raw.data.vertex_shader = raw.material.name + "_" + raw.data.name + ".vert";
	raw.vert = node_shader_create(raw, "vert");
	return raw.vert;
}

function node_shader_context_make_frag(raw: node_shader_context_t): node_shader_t {
	raw.data.fragment_shader = raw.material.name + "_" + raw.data.name + ".frag";
	raw.frag = node_shader_create(raw, "frag");
	return raw.frag;
}

type material_t = {
	name?: string;
	canvas?: ui_node_canvas_t;
};
