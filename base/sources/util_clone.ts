
function util_clone_f32_array(f32a: f32_array_t): f32_array_t {
	if (f32a == null) {
		return null;
	}
	return f32_array_create_from_array(f32a);
}

function util_clone_u32_array(u32a: u32_array_t): u32_array_t {
	if (u32a == null) {
		return null;
	}
	return u32_array_create_from_array(u32a);
}

function util_clone_u8_array(u8a: u8_array_t): u8_array_t {
	if (u8a == null) {
		return null;
	}
	return u8_array_create_from_array(u8a);
}

function util_clone_string_array(a: string[]): string[] {
	if (a == null) {
		return null;
	}
	let r: string[] = [];
	for (let i: i32 = 0; i < a.length; ++i) {
		let s: string = a[i];
		array_push(r, s);
	}
	return r;
}

function util_clone_bool_array(a: bool[]): bool[] {
	if (a == null) {
		return null;
	}
	let r: bool[] = [];
	for (let i: i32 = 0; i < a.length; ++i) {
		let s: bool = a[i];
		array_push(r, s);
	}
	return r;
}

function util_clone_canvas_sockets(sockets: ui_node_socket_t[]): ui_node_socket_t[] {
	if (sockets == null) {
		return null;
	}
	let r: ui_node_socket_t[] = [];
	for (let i: i32 = 0; i < sockets.length; ++i) {
		let s: ui_node_socket_t = {};
		s.id = sockets[i].id;;
		s.node_id = sockets[i].node_id;
		s.name = sockets[i].name;
		s.type = sockets[i].type;
		s.color = sockets[i].color;
		s.default_value = util_clone_f32_array(sockets[i].default_value);
		s.min = sockets[i].min;
		s.max = sockets[i].max;
		s.precision = sockets[i].precision;
		s.display = sockets[i].display;
		array_push(r, s);
	}
	return r;
}

function util_clone_canvas_buttons(buttons: ui_node_button_t[]): ui_node_button_t[] {
	if (buttons == null) {
		return null;
	}
	let r: ui_node_button_t[] = [];
	for (let i: i32 = 0; i < buttons.length; ++i) {
		let b: ui_node_button_t = {};
		b.name = buttons[i].name;
		b.type = buttons[i].type;
		b.output = buttons[i].output;
		b.default_value = util_clone_f32_array(buttons[i].default_value);
		b.data = util_clone_u8_array(buttons[i].data);
		b.min = buttons[i].min;
		b.max = buttons[i].max;
		b.precision = buttons[i].precision;
		b.height = buttons[i].height;
		array_push(r, b);
	}
	return r;
}

function util_clone_canvas_node(n: ui_node_t): ui_node_t {
	if (n == null) {
		return null;
	}
	let r: ui_node_t = {};
	r.id = n.id;
	r.name = n.name;
	r.type = n.type;
	r.x = n.x;
	r.y = n.y;
	r.color = n.color;
	r.inputs = util_clone_canvas_sockets(n.inputs);
	r.outputs = util_clone_canvas_sockets(n.outputs);
	r.buttons = util_clone_canvas_buttons(n.buttons);
	r.width = n.width;
	return r;
}

function util_clone_canvas_nodes(nodes: ui_node_t[]): ui_node_t[] {
	if (nodes == null) {
		return null;
	}
	let r: ui_node_t[] = [];
	for (let i: i32 = 0; i < nodes.length; ++i) {
		let n: ui_node_t = util_clone_canvas_node(nodes[i]);
		array_push(r, n);
	}
	return r;
}

function util_clone_canvas_links(links: ui_node_link_t[]): ui_node_link_t[] {
	if (links == null) {
		return null;
	}
	let r: ui_node_link_t[] = [];
	for (let i: i32 = 0; i < links.length; ++i) {
		let l: ui_node_link_t = {};
		l.id = links[i].id;
		l.from_id = links[i].from_id;
		l.from_socket = links[i].from_socket;
		l.to_id = links[i].to_id;
		l.to_socket = links[i].to_socket;
		array_push(r, l);
	}
	return r;
}

function util_clone_canvas(c: ui_node_canvas_t): ui_node_canvas_t {
	if (c == null) {
		return null;
	}
	let r: ui_node_canvas_t = {};
	r.name = c.name;
	r.nodes = util_clone_canvas_nodes(c.nodes);
	r.links = util_clone_canvas_links(c.links);
	return r;
}

function util_clone_vertex_elements(elems: vertex_element_t[]): vertex_element_t[] {
	if (elems == null) {
		return null;
	}
	let r: vertex_element_t[] = [];
	for (let i: i32 = 0; i < elems.length; ++i) {
		let e: vertex_element_t = {};
		e.name = elems[i].name;
		e.data = elems[i].data;
		array_push(r, e);
	}
	return r;
}

function util_clone_shader_consts(consts: shader_const_t[]): shader_const_t[] {
	if (consts == null) {
		return null;
	}
	let r: shader_const_t[] = [];
	for (let i: i32 = 0; i < consts.length; ++i) {
		let s: shader_const_t = {};
		s.name = consts[i].name;
		s.type = consts[i].type;
		s.link = consts[i].link;
		array_push(r, s);
	}
	return r;
}

function util_clone_tex_units(units: tex_unit_t[]): tex_unit_t[] {
	if (units == null) {
		return null;
	}
	let r: tex_unit_t[] = [];
	for (let i: i32 = 0; i < units.length; ++i) {
		let u: tex_unit_t = {};
		u.name = units[i].name;
		u.link = units[i].link;
		array_push(r, u);
	}
	return r;
}

function util_clone_shader_contexts(contexts: shader_context_t[]): shader_context_t[] {
	if (contexts == null) {
		return null;
	}
	let r: shader_context_t[] = [];
	for (let i: i32 = 0; i < contexts.length; ++i) {
		let c: shader_context_t = {};
		c.name = contexts[i].name;
		c.depth_write = contexts[i].depth_write;
		c.compare_mode = contexts[i].compare_mode;
		c.cull_mode = contexts[i].cull_mode;
		c.vertex_shader = contexts[i].vertex_shader;
		c.fragment_shader = contexts[i].fragment_shader;
		c.shader_from_source = contexts[i].shader_from_source;
		c.blend_source = contexts[i].blend_source;
		c.blend_destination = contexts[i].blend_destination;
		c.blend_operation = contexts[i].blend_operation;
		c.alpha_blend_source = contexts[i].alpha_blend_source;
		c.alpha_blend_destination = contexts[i].alpha_blend_destination;
		c.alpha_blend_operation = contexts[i].alpha_blend_operation;
		c.color_writes_red = util_clone_bool_array(contexts[i].color_writes_red);
		c.color_writes_green = util_clone_bool_array(contexts[i].color_writes_green);
		c.color_writes_blue = util_clone_bool_array(contexts[i].color_writes_blue);
		c.color_writes_alpha = util_clone_bool_array(contexts[i].color_writes_alpha);
		c.color_attachments = util_clone_string_array(contexts[i].color_attachments);
		c.depth_attachment = contexts[i].depth_attachment;
		c.vertex_elements = util_clone_vertex_elements(contexts[i].vertex_elements);
		c.constants = util_clone_shader_consts(contexts[i].constants);
		c.texture_units = util_clone_tex_units(contexts[i].texture_units);
		array_push(r, c);
	}
	return r;
}

function util_clone_shader_data(s: shader_data_t): shader_data_t {
	if (s == null) {
		return null;
	}
	let r: shader_data_t = {};
	r.name = s.name;
	r.contexts = util_clone_shader_contexts(s.contexts);
	return r;
}

function util_clone_bind_constants(consts: bind_const_t[]): bind_const_t[] {
	if (consts == null) {
		return null;
	}
	let r: bind_const_t[] = [];
	for (let i: i32 = 0; i < consts.length; ++i) {
		let c: bind_const_t = {};
		c.name = consts[i].name;
		c.vec = util_clone_f32_array(consts[i].vec);
		array_push(r, c);
	}
	return r;
}

function util_clone_bind_textures(texs: bind_tex_t[]): bind_tex_t[] {
	if (texs == null) {
		return null;
	}
	let r: bind_tex_t[] = [];
	for (let i: i32 = 0; i < texs.length; ++i) {
		let t: bind_tex_t = {};
		t.name = texs[i].name;
		t.file = texs[i].file;
		t.u_addressing = texs[i].u_addressing;
		t.v_addressing = texs[i].v_addressing;
		t.min_filter = texs[i].min_filter;
		t.mag_filter = texs[i].mag_filter;
		t.mipmap_filter = texs[i].mipmap_filter;
		t.generate_mipmaps = texs[i].generate_mipmaps;
		t.mipmaps = util_clone_string_array(texs[i].mipmaps);
		array_push(r, t);
	}
	return r;
}

function util_clone_material_contexts(contexts: material_context_t[]): material_context_t[] {
	if (contexts == null) {
		return null;
	}
	let r: material_context_t[] = [];
	for (let i: i32 = 0; i < contexts.length; ++i) {
		let c: material_context_t = {};
		c.name = contexts[i].name;
		c.bind_constants = util_clone_bind_constants(contexts[i].bind_constants);
		c.bind_textures = util_clone_bind_textures(contexts[i].bind_textures);
		array_push(r, c);
	}
	return r;
}

function util_clone_material_data(m: material_data_t): material_data_t {
	if (m == null) {
		return null;
	}
	let r: material_data_t = {};
	r.name = m.name;
	r.shader = m.shader;
	r.contexts = util_clone_material_contexts(m.contexts);
	return r;
}

function util_clone_tracks(tracks: track_t[]): track_t[] {
	if (tracks == null) {
		return null;
	}
	let r: track_t[] = [];
	for (let i: i32 = 0; i < tracks.length; ++i) {
		let t: track_t = {};
		t.target = tracks[i].target;
		t.frames = util_clone_u32_array(tracks[i].frames);
		t.values = util_clone_f32_array(tracks[i].values);
		array_push(r, t);
	}
	return r;
}

function util_clone_anim(a: anim_t): anim_t {
	if (a == null) {
		return null;
	}
	let r: anim_t = {};
	r.object_actions = util_clone_string_array(a.object_actions);
	r.bone_actions = util_clone_string_array(a.bone_actions);
	r.parent_bone = a.parent_bone;
	r.parent_bone_tail = util_clone_f32_array(a.parent_bone_tail);
	r.parent_bone_tail_pose = util_clone_f32_array(a.parent_bone_tail_pose);
	r.parent_bone_connected = a.parent_bone_connected;
	r.tracks = util_clone_tracks(a.tracks);
	r.begin = a.begin;
	r.end = a.end;
	r.has_delta = a.has_delta;
	r.marker_frames = util_clone_u32_array(a.marker_frames);
	r.marker_names = util_clone_string_array(a.marker_names);
	return r;
}

function util_clone_obj(o: obj_t): obj_t {
	if (o == null) {
		return null;
	}
	let r: obj_t = {};
	r.name = o.name;
	r.type = o.type;
	r.data_ref = o.data_ref;
	r.transform = util_clone_f32_array(o.transform);
	r.dimensions = util_clone_f32_array(o.dimensions);
	r.visible = o.visible;
	r.spawn = o.spawn;
	r.anim = util_clone_anim(o.anim);
	r.material_refs = util_clone_string_array(o.material_refs);
	if (o.children != null) {
		r.children = [];
		for (let i: i32 = 0; i < o.children.length; ++i) {
			let c: obj_t = util_clone_obj(o.children[i]);
			array_push(r.children, c);
		}
	}
	return r;
}

function util_clone_swatch_color(s: swatch_color_t): swatch_color_t {
	let r: swatch_color_t = {};
	r.base = s.base;
	r.opacity = s.opacity;
	r.occlusion = s.occlusion;
	r.roughness = s.roughness;
	r.metallic = s.metallic;
	r.normal = s.normal;
	r.emission = s.emission;
	r.height = s.height;
	r.subsurface = s.subsurface;
	return r;
}
