node_shader_t *node_shader_create(node_shader_context_t *context) {
	node_shader_t *raw     = GC_ALLOC_INIT(node_shader_t, {0});
	raw->context           = context;
	raw->ins               = any_array_create_from_raw((any[]){}, 0);
	raw->outs              = any_array_create_from_raw((any[]){}, 0);
	raw->frag_out          = "float4";
	raw->consts            = any_array_create_from_raw((any[]){}, 0);
	raw->textures          = any_array_create_from_raw((any[]){}, 0);
	raw->functions         = any_map_create();

	raw->vert              = "";
	raw->vert_end          = "";
	raw->vert_normal       = "";
	raw->vert_attribs      = "";
	raw->vert_write_normal = 0;

	raw->frag              = "";
	raw->frag_end          = "";
	raw->frag_normal       = "";
	raw->frag_attribs      = "";
	raw->frag_write_normal = 0;

	return raw;
}

void node_shader_add_in(node_shader_t *raw, string_t *s) {
	any_array_push(raw->ins, s);
}

void node_shader_add_out(node_shader_t *raw, string_t *s) {
	any_array_push(raw->outs, s);
}

void node_shader_add_constant(node_shader_t *raw, string_t *s, string_t *link) {
	// inp: float4
	if (char_ptr_array_index_of(raw->consts, s) == -1) {
		string_t_array_t *ar    = string_split(s, ": ");
		string_t         *uname = ar->buffer[0];
		string_t         *utype = ar->buffer[1];

		////
		if (string_equals(utype, "float2"))
			utype = "vec2";
		if (string_equals(utype, "float3"))
			utype = "vec3";
		if (string_equals(utype, "float4"))
			utype = "vec4";
		if (string_equals(utype, "float3x3"))
			utype = "mat3";
		if (string_equals(utype, "float4x4"))
			utype = "mat4";
		////

		any_array_push(raw->consts, s);
		node_shader_context_add_constant(raw->context, utype, uname, link);
	}
}

void node_shader_add_texture(node_shader_t *raw, string_t *name, string_t *link) {
	if (char_ptr_array_index_of(raw->textures, name) == -1) {
		any_array_push(raw->textures, name);
		node_shader_context_add_texture_unit(raw->context, name, link);
	}
}

void node_shader_add_function(node_shader_t *raw, string_t *s) {
	string_t *fname = string_split(s, "(")->buffer[0];
	if (any_map_get(raw->functions, fname) != null) {
		return;
	}
	any_map_set(raw->functions, fname, s);
}

void node_shader_write_vert(node_shader_t *raw, string_t *s) {
	if (raw->vert_write_normal > 0) {
		raw->vert_normal = string_join(raw->vert_normal, string_join(s, "\n"));
	}
	else {
		raw->vert = string_join(raw->vert, string_join(s, "\n"));
	}
}

void node_shader_write_end_vert(node_shader_t *raw, string_t *s) {
	raw->vert_end = string_join(raw->vert_end, string_join(s, "\n"));
}

void node_shader_write_attrib_vert(node_shader_t *raw, string_t *s) {
	raw->vert_attribs = string_join(raw->vert_attribs, string_join(s, "\n"));
}

void node_shader_write_frag(node_shader_t *raw, string_t *s) {
	if (raw->frag_write_normal > 0) {
		raw->frag_normal = string_join(raw->frag_normal, string_join(s, "\n"));
	}
	else {
		raw->frag = string_join(raw->frag, string_join(s, "\n"));
	}
}

void node_shader_write_attrib_frag(node_shader_t *raw, string_t *s) {
	raw->frag_attribs = string_join(raw->frag_attribs, string_join(s, "\n"));
}

string_t *node_shader_data_size(node_shader_t *raw, string_t *data) {
	if (string_equals(data, "float1")) {
		return "1";
	}
	else if (string_equals(data, "float2") || string_equals(data, "short2norm")) {
		return "2";
	}
	else if (string_equals(data, "float3")) {
		return "3";
	}
	else { // float4 || short4norm
		return "4";
	}
}

void node_shader_vstruct_to_vsin(node_shader_t *raw) {
	vertex_element_t_array_t *vs = raw->context->data->vertex_elements;
	for (i32 i = 0; i < vs->length; ++i) {
		vertex_element_t *e = vs->buffer[i];
		node_shader_add_in(raw, string_join(string_join(string_join(string_join("", e->name), ": "), "float"), node_shader_data_size(raw, e->data)));
	}
}

string_t *node_shader_get(node_shader_t *raw) {
	node_shader_vstruct_to_vsin(raw);

	string_t *s = "";

	s           = string_join(s, "struct vert_in {\n");
	for (i32 i = 0; i < raw->ins->length; ++i) {
		string_t *a = raw->ins->buffer[i];
		s           = string_join(s, string_join(string_join("\t", a), ";\n"));
	}
	s = string_join(s, "}\n\n");

	s = string_join(s, "struct vert_out {\n");
	s = string_join(s, "\tpos: float4;\n");
	for (i32 i = 0; i < raw->outs->length; ++i) {
		string_t *a = raw->outs->buffer[i];
		s           = string_join(s, string_join(string_join("\t", a), ";\n"));
	}
	if (raw->consts->length == 0) {
		s = string_join(s, "\tempty: float4;\n");
	}
	s = string_join(s, "}\n\n");

	s = string_join(s, "#[set(everything)]\n");
	s = string_join(s, "const constants: {\n");
	for (i32 i = 0; i < raw->consts->length; ++i) {
		string_t *a = raw->consts->buffer[i];
		s           = string_join(s, string_join(string_join("\t", a), ";\n"));
	}
	if (raw->consts->length == 0) {
		s = string_join(s, "\tempty: float4;\n");
	}
	s = string_join(s, "};\n\n");

	if (raw->textures->length > 0) {
		s = string_join(s, "#[set(everything)]\n");
		s = string_join(s, "const sampler_linear: sampler;\n\n");
	}

	for (i32 i = 0; i < raw->textures->length; ++i) {
		string_t *a = raw->textures->buffer[i];
		s           = string_join(s, "#[set(everything)]\n");
		s           = string_join(s, string_join(string_join("const ", a), ": tex2d;\n"));
	}

	string_t_array_t *keys = map_keys(raw->functions);
	for (i32 i = 0; i < keys->length; ++i) {
		string_t *f = any_map_get(raw->functions, keys->buffer[i]);
		s           = string_join(s, string_join(f, "\n"));
	}
	s = string_join(s, "\n");

	s = string_join(s, "fun kong_vert(input: vert_in): vert_out {\n");
	s = string_join(s, "\tvar output: vert_out;\n\n");
	s = string_join(s, raw->vert_attribs);
	s = string_join(s, raw->vert_normal);
	s = string_join(s, raw->vert);
	s = string_join(s, raw->vert_end);
	if (raw->consts->length == 0) {
		s = string_join(s, "\toutput.empty = constants.empty;\n");
	}
	s = string_join(s, "\n\treturn output;\n");
	s = string_join(s, "}\n\n");

	s = string_join(s, string_join(string_join("fun kong_frag(input: vert_out): ", raw->frag_out), " {\n"));
	s = string_join(s, string_join(string_join("\tvar output: ", raw->frag_out), ";\n\n"));
	s = string_join(s, raw->frag_attribs);
	s = string_join(s, raw->frag_normal);
	s = string_join(s, raw->frag);
	s = string_join(s, raw->frag_end);
	s = string_join(s, "\n\treturn output;\n");
	s = string_join(s, "}\n\n");

	s = string_join(s, "#[pipe]\n");
	s = string_join(s, "struct pipe {\n");
	s = string_join(s, "\tvertex = kong_vert;\n");
	s = string_join(s, "\tfragment = kong_frag;\n");
	s = string_join(s, "}\n");

	return s;
}

node_shader_context_t *node_shader_context_create(material_t *material, shader_context_t *props) {
	node_shader_context_t *raw                        = GC_ALLOC_INIT(node_shader_context_t, {0});
	raw->material                                     = material;

	vertex_element_t_array_t *vertex_elements_default = any_array_create_from_raw(
	    (any[]){
	        GC_ALLOC_INIT(vertex_element_t, {.name = "pos", .data = "short4norm"}),
	        GC_ALLOC_INIT(vertex_element_t, {.name = "nor", .data = "short2norm"}),
	    },
	    2);

	raw->data            = GC_ALLOC_INIT(shader_context_t, {.name                    = props->name,
	                                                        .depth_write             = props->depth_write,
	                                                        .compare_mode            = props->compare_mode,
	                                                        .cull_mode               = props->cull_mode,
	                                                        .blend_source            = props->blend_source,
	                                                        .blend_destination       = props->blend_destination,
	                                                        .alpha_blend_source      = props->alpha_blend_source,
	                                                        .alpha_blend_destination = props->alpha_blend_destination,
	                                                        .fragment_shader         = "",
	                                                        .vertex_shader           = "",
	                                                        .vertex_elements         = props->vertex_elements != null ? props->vertex_elements : vertex_elements_default,
	                                                        .color_attachments       = props->color_attachments,
	                                                        .depth_attachment        = props->depth_attachment});

	shader_context_t *rw = raw->data;
	rw->_                = GC_ALLOC_INIT(shader_context_runtime_t, {0});

	if (props->color_writes_red != null) {
		raw->data->color_writes_red = props->color_writes_red;
	}
	if (props->color_writes_green != null) {
		raw->data->color_writes_green = props->color_writes_green;
	}
	if (props->color_writes_blue != null) {
		raw->data->color_writes_blue = props->color_writes_blue;
	}
	if (props->color_writes_alpha != null) {
		raw->data->color_writes_alpha = props->color_writes_alpha;
	}

	raw->data->texture_units = any_array_create_from_raw((any[]){}, 0);
	raw->data->constants     = i32_array_create_from_raw((i32[]){}, 0);
	return raw;
}

void node_shader_context_add_elem(node_shader_context_t *raw, string_t *name, string_t *data_type) {
	for (i32 i = 0; i < raw->data->vertex_elements->length; ++i) {
		vertex_element_t *e = raw->data->vertex_elements->buffer[i];
		if (string_equals(e->name, name)) {
			return;
		}
	}
	vertex_element_t *elem = GC_ALLOC_INIT(vertex_element_t, {.name = name, .data = data_type});
	any_array_push(raw->data->vertex_elements, elem);
}

bool node_shader_context_is_elem(node_shader_context_t *raw, string_t *name) {
	for (i32 i = 0; i < raw->data->vertex_elements->length; ++i) {
		vertex_element_t *elem = raw->data->vertex_elements->buffer[i];
		if (string_equals(elem->name, name)) {
			return true;
		}
	}
	return false;
}

vertex_element_t *node_shader_context_get_elem(node_shader_context_t *raw, string_t *name) {
	for (i32 i = 0; i < raw->data->vertex_elements->length; ++i) {
		vertex_element_t *elem = raw->data->vertex_elements->buffer[i];
		if (string_equals(elem->name, name)) {
			return elem;
		}
	}
	return null;
}

void node_shader_context_add_constant(node_shader_context_t *raw, string_t *ctype, string_t *name, string_t *link) {
	for (i32 i = 0; i < raw->data->constants->length; ++i) {
		shader_const_t *c = raw->data->constants->buffer[i];
		if (string_equals(c->name, name)) {
			return;
		}
	}

	shader_const_t *c = GC_ALLOC_INIT(shader_const_t, {.name = name, .type = ctype});
	if (link != null) {
		c->link = string_copy(link);
	}
	shader_const_t_array_t *consts = raw->data->constants;
	any_array_push(consts, c);
}

void node_shader_context_add_texture_unit(node_shader_context_t *raw, string_t *name, string_t *link) {
	for (i32 i = 0; i < raw->data->texture_units->length; ++i) {
		tex_unit_t *c = raw->data->texture_units->buffer[i];
		if (string_equals(c->name, name)) {
			return;
		}
	}

	tex_unit_t *c = GC_ALLOC_INIT(tex_unit_t, {.name = name, .link = link});
	any_array_push(raw->data->texture_units, c);
}

node_shader_t *node_shader_context_make_kong(node_shader_context_t *raw) {
	raw->data->vertex_shader   = string_join(string_join(string_join(raw->material->name, "_"), raw->data->name), ".vert");
	raw->data->fragment_shader = string_join(string_join(string_join(raw->material->name, "_"), raw->data->name), ".frag");
	raw->kong                  = node_shader_create(raw);
	return raw->kong;
}
