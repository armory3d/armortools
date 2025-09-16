
type node_shader_t = {
	context?: node_shader_context_t;
	ins?: string[];
	outs?: string[];
	frag_out?: string;
	consts?: string[];
	textures?: string[];
	functions?: map_t<string, string>;

	vert?: string;
	vert_end?: string;
	vert_normal?: string;
	vert_attribs?: string;
	vert_write_normal?: i32;

	frag?: string;
	frag_end?: string;
	frag_normal?: string;
	frag_attribs?: string;
	frag_write_normal?: i32;

	// References
	vert_n?: bool;
	frag_bposition?: bool;
	frag_wposition?: bool;
	frag_mposition?: bool;
	frag_vposition?: bool;
	frag_wvpposition?: bool;
	frag_ndcpos?: bool;
	frag_wtangent?: bool;
	frag_vvec?: bool;
	frag_vvec_cam?: bool;
	frag_n?: bool;
	frag_nattr?: bool;
	frag_dotnv?: bool;
};

function node_shader_create(context: node_shader_context_t): node_shader_t {
	let raw: node_shader_t = {};
	raw.context = context;
	raw.ins = [];
	raw.outs = [];
	raw.frag_out = "float4";
	raw.consts = [];
	raw.textures = [];
	raw.functions = map_create();

	raw.vert = "";
	raw.vert_end = "";
	raw.vert_normal = "";
	raw.vert_attribs = "";
	raw.vert_write_normal = 0;

	raw.frag = "";
	raw.frag_end = "";
	raw.frag_normal = "";
	raw.frag_attribs = "";
	raw.frag_write_normal = 0;

	return raw;
}

function node_shader_add_in(raw: node_shader_t, s: string) {
	array_push(raw.ins, s);
}

function node_shader_add_out(raw: node_shader_t, s: string) {
	array_push(raw.outs, s);
}

function node_shader_add_constant(raw: node_shader_t, s: string, link: string = null) {
	// inp: float4
	if (array_index_of(raw.consts, s) == -1) {
		let ar: string[] = string_split(s, ": ");
		let uname: string = ar[0];
		let utype: string = ar[1];

		////
		if (utype == "float2") utype = "vec2";
		if (utype == "float3") utype = "vec3";
		if (utype == "float4") utype = "vec4";
		if (utype == "float3x3") utype = "mat3";
		if (utype == "float4x4") utype = "mat4";
		////

		array_push(raw.consts, s);
		node_shader_context_add_constant(raw.context, utype, uname, link);
	}
}

function node_shader_add_texture(raw: node_shader_t, name: string, link: string = null) {
	if (array_index_of(raw.textures, name) == -1) {
		array_push(raw.textures, name);
		node_shader_context_add_texture_unit(raw.context, name, link);
	}
}

function node_shader_add_function(raw: node_shader_t, s: string) {
	let fname: string = string_split(s, "(")[0];
	if (map_get(raw.functions, fname) != null) {
		return;
	}
	map_set(raw.functions, fname, s);
}

function node_shader_write_vert(raw: node_shader_t, s: string) {
	if (raw.vert_write_normal > 0) {
		raw.vert_normal += s + "\n";
	}
	else {
		raw.vert += s + "\n";
	}
}

function node_shader_write_end_vert(raw: node_shader_t, s: string) {
	raw.vert_end += s + "\n";
}

function node_shader_write_attrib_vert(raw: node_shader_t, s: string) {
	raw.vert_attribs += s + "\n";
}

function node_shader_write_frag(raw: node_shader_t, s: string) {
	if (raw.frag_write_normal > 0) {
		raw.frag_normal += s + "\n";
	}
	else {
		raw.frag += s + "\n";
	}
}

function node_shader_write_attrib_frag(raw: node_shader_t, s: string) {
	raw.frag_attribs += s + "\n";
}

function node_shader_data_size(raw: node_shader_t, data: string): string {
	if (data == "float1") {
		return "1";
	}
	else if (data == "float2" || data == "short2norm") {
		return "2";
	}
	else if (data == "float3") {
		return "3";
	}
	else { // float4 || short4norm
		return "4";
	}
}

function node_shader_vstruct_to_vsin(raw: node_shader_t) {
	let vs: vertex_element_t[] = raw.context.data.vertex_elements;
	for (let i: i32 = 0; i < vs.length; ++i) {
		let e: vertex_element_t = vs[i];
		node_shader_add_in(raw, "" + e.name + ": " + "float" + node_shader_data_size(raw, e.data));
	}
}

function node_shader_get(raw: node_shader_t): string {

	node_shader_vstruct_to_vsin(raw);

	let s: string = "";

	s += "struct vert_in {\n";
	for (let i: i32 = 0; i < raw.ins.length; ++i) {
		let a: string = raw.ins[i];
		s += "\t" + a + ";\n";
	}
	s += "}\n\n";

	s += "struct vert_out {\n";
	s += "\tpos: float4;\n";
	for (let i: i32 = 0; i < raw.outs.length; ++i) {
		let a: string = raw.outs[i];
		s += "\t" + a + ";\n";
	}
	if (raw.consts.length == 0) {
		s += "\tempty: float4;\n";
	}
	s += "}\n\n";

	s += "#[set(everything)]\n";
	s += "const constants: {\n";
	for (let i: i32 = 0; i < raw.consts.length; ++i) {
		let a: string = raw.consts[i];
		s += "\t" + a + ";\n";
	}
	if (raw.consts.length == 0) {
		s += "\tempty: float4;\n";
	}
	s += "};\n\n";

	if (raw.textures.length > 0) {
		s += "#[set(everything)]\n";
		s += "const sampler_linear: sampler;\n\n";
	}

	for (let i: i32 = 0; i < raw.textures.length; ++i) {
		let a: string = raw.textures[i];
		s += "#[set(everything)]\n";
		s += "const " + a + ": tex2d;\n";
	}

	let keys: string[] = map_keys(raw.functions);
	for (let i: i32 = 0; i < keys.length; ++i) {
		let f: string = map_get(raw.functions, keys[i]);
		s += f + "\n";
	}
	s += "\n";

	s += "fun kong_vert(input: vert_in): vert_out {\n";
	s += "\tvar output: vert_out;\n\n";
	s += raw.vert_attribs;
	s += raw.vert_normal;
	s += raw.vert;
	s += raw.vert_end;
	s += "\toutput.pos.z = (output.pos.z + output.pos.w) * 0.5;\n"; ////
	if (raw.consts.length == 0) {
		s += "\toutput.empty = constants.empty;\n";
	}
	s += "\n\treturn output;\n";
	s += "}\n\n";

	s += "fun kong_frag(input: vert_out): " + raw.frag_out + " {\n";
	s += "\tvar output: " + raw.frag_out + ";\n\n";
	s += raw.frag_attribs;
	s += raw.frag_normal;
	s += raw.frag;
	s += raw.frag_end;
	s += "\n\treturn output;\n";
	s += "}\n\n";

	s += "#[pipe]\n";
	s += "struct pipe {\n";
	s += "\tvertex = kong_vert;\n";
	s += "\tfragment = kong_frag;\n";
	s += "}\n";

	return s;
}


type material_t = {
	name?: string;
	canvas?: ui_node_canvas_t;
};

type node_shader_context_t = {
	kong?: node_shader_t;
	data?: shader_context_t;
	allow_vcols?: bool;
	material?: material_t;
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
		alpha_blend_source: props.alpha_blend_source,
		alpha_blend_destination: props.alpha_blend_destination,
		fragment_shader: "",
		vertex_shader: "",
		vertex_elements: props.vertex_elements != null ? props.vertex_elements : vertex_elements_default,
		color_attachments: props.color_attachments,
		depth_attachment: props.depth_attachment
	};

	let rw: shader_context_t = raw.data;
	rw._ = {};

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

	raw.data.texture_units = [];
	raw.data.constants = [];
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
	for (let i: i32 = 0; i < raw.data.constants.length; ++i) {
		let c: shader_const_t = raw.data.constants[i];
		if (c.name == name) {
			return;
		}
	}

	let c: shader_const_t = { name: name, type: ctype };
	if (link != null) {
		c.link = link;
	}
	let consts: shader_const_t[] = raw.data.constants;
	array_push(consts, c);
}

function node_shader_context_add_texture_unit(raw: node_shader_context_t, name: string, link: string = null) {
	for (let i: i32 = 0; i < raw.data.texture_units.length; ++i) {
		let c: tex_unit_t = raw.data.texture_units[i];
		if (c.name == name) {
			return;
		}
	}

	let c: tex_unit_t = { name: name, link: link };
	array_push(raw.data.texture_units, c);
}

function node_shader_context_make_kong(raw: node_shader_context_t): node_shader_t {
	raw.data.vertex_shader = raw.material.name + "_" + raw.data.name + ".vert";
	raw.data.fragment_shader = raw.material.name + "_" + raw.data.name + ".frag";
	raw.kong = node_shader_create(raw);
	return raw.kong;
}
