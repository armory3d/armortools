
type node_shader_t = {
	context?: node_shader_context_t;
	ins?: string[];
	outs?: string[];
	frag_out?: string;
	shared_samplers?: string[];
	uniforms?: string[];
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
	vert_wposition?: bool;
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
	raw.shared_samplers = [];
	raw.uniforms = [];
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

function node_shader_add_uniform(raw: node_shader_t, s: string, link: string = null) {
	let ar: string[] = string_split(s, " ");
	// layout(RGBA8) sampler2D tex
	let utype: string = ar[ar.length - 2];
	let uname: string = ar[ar.length - 1];
	if (starts_with(utype, "sampler")) {
		node_shader_context_add_texture_unit(raw.context, utype, uname, link);
	}
	else {
		if (ar[0] == "float" && string_index_of(ar[1], "[") >= 0) {
			ar[0] = "floats";
			ar[1] = string_split(ar[1], "[")[0];
		}
		else if (ar[0] == "vec4" && string_index_of(ar[1], "[") >= 0) {
			ar[0] = "floats";
			ar[1] = string_split(ar[1], "[")[0];
		}
		node_shader_context_add_constant(raw.context, ar[0], ar[1], link);
	}
	if (array_index_of(raw.uniforms, s) == -1) {
		array_push(raw.uniforms, s);
	}
}

function node_shader_add_shared_sampler(raw: node_shader_t, s: string) {
	if (array_index_of(raw.shared_samplers, s) == -1) {
		array_push(raw.shared_samplers, s);
		let ar: string[] = string_split(s, " ");
		// layout(RGBA8) sampler2D tex
		let utype: string = ar[ar.length - 2];
		let uname: string = ar[ar.length - 1];
		node_shader_context_add_texture_unit(raw.context, utype, uname, null);
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

	let shared_sampler: string = "shared_sampler";
	// if (raw.shared_samplers.length > 0) {
	// 	shared_sampler = string_split(raw.shared_samplers[0], " ")[1] + "_sampler";
	// }

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
	s += "}\n\n";

	s += "#[set(everything)] \n";
	s += "const constants: { \n";
	for (let i: i32 = 0; i < raw.uniforms.length; ++i) {
		let a: string = raw.uniforms[i];
		s += "\t" + a + ";\n";
	}
	s += "};\n\n";

	// for (let i: i32 = 0; i < raw.shared_samplers.length; ++i) {
	// 	let a: string = raw.shared_samplers[i];
	// 	s += "uniform " + a + ";\n";
	// }

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

	iron_file_save_bytes("C:/users/lubos/desktop/test.txt", sys_string_to_buffer(s), 0);

	return s;
}
