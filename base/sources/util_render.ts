
let util_render_material_preview_size: i32 = 256;
let util_render_decal_preview_size: i32 = 512;
let util_render_layer_preview_size: i32 = 200;
let util_render_font_preview_size: i32 = 200;
let util_render_screen_aligned_full_vb: vertex_buffer_t = null;
let util_render_screen_aligned_full_ib: index_buffer_t = null;

function util_render_make_material_preview() {
	context_raw.material_preview = true;

	let sphere: mesh_object_t = scene_get_child(".Sphere").ext;
	sphere.base.visible = true;
	let meshes: mesh_object_t[] = scene_meshes;
	scene_meshes = [sphere];
	let painto: mesh_object_t = context_raw.paint_object;
	context_raw.paint_object = sphere;

	sphere.materials[0] = project_materials[0].data;
	context_raw.material.preview_ready = true;

	context_raw.saved_camera = mat4_clone(scene_camera.base.transform.local);
	let m: mat4_t = mat4_create(
		0.9146286343879498, -0.0032648027153306235, 0.404281837254303, 0.4659988049397712,
		0.404295023959927, 0.007367569133732468, -0.9145989516155143, -1.0687517188018691,
		0.000007410128652369705, 0.9999675337275382, 0.008058532943908717, 0.015935682577325486,
		0, 0, 0, 1
	);
	transform_set_matrix(scene_camera.base.transform, m);
	let saved_fov: f32 = scene_camera.data.fov;
	scene_camera.data.fov = 0.92;
	viewport_update_camera_type(camera_type_t.PERSPECTIVE);


	let _light_strength: f32 = uniforms_light_strength;
	uniforms_light_strength = 0;

	let probe: world_data_t = scene_world;
	let _probe_strength: f32 = probe.strength;
	probe.strength = 7;
	let _envmap_angle: f32 = context_raw.envmap_angle;
	context_raw.envmap_angle = 6.0;
	let _brush_scale: f32 = context_raw.brush_scale;
	context_raw.brush_scale = 1.5;
	let _brush_nodes_scale: f32 = context_raw.brush_nodes_scale;
	context_raw.brush_nodes_scale = 1.0;

	scene_world._.envmap = context_raw.preview_envmap;
	// No resize
	_render_path_last_w = util_render_material_preview_size;
	_render_path_last_h = util_render_material_preview_size;
	camera_object_build_proj(scene_camera);
	camera_object_build_mat(scene_camera);

	make_material_parse_mesh_preview_material();
	let _commands: ()=>void = render_path_commands;
	render_path_commands = render_path_preview_commands_preview;
	render_path_render_frame();
	render_path_commands = _commands;

	context_raw.material_preview = false;
	_render_path_last_w = app_w();
	_render_path_last_h = app_h();

	// Restore
	sphere.base.visible = false;
	scene_meshes = meshes;
	context_raw.paint_object = painto;

	transform_set_matrix(scene_camera.base.transform, context_raw.saved_camera);
	viewport_update_camera_type(context_raw.camera_type);
	scene_camera.data.fov = saved_fov;
	camera_object_build_proj(scene_camera);
	camera_object_build_mat(scene_camera);

	uniforms_light_strength = _light_strength;
	probe.strength = _probe_strength;
	context_raw.envmap_angle = _envmap_angle;
	context_raw.brush_scale = _brush_scale;
	context_raw.brush_nodes_scale = _brush_nodes_scale;
	scene_world._.envmap = context_raw.show_envmap ? context_raw.saved_envmap : context_raw.empty_envmap;
	make_material_parse_mesh_material();
	context_raw.ddirty = 0;
}

function util_render_make_decal_preview() {
	let current: image_t = _g2_current;
	let g2_in_use: bool = _g2_in_use;
	if (g2_in_use) g2_end();

	if (context_raw.decal_image == null) {
		context_raw.decal_image = image_create_render_target(util_render_decal_preview_size, util_render_decal_preview_size);
	}
	context_raw.decal_preview = true;

	let plane: mesh_object_t = scene_get_child(".Plane").ext;
	plane.base.transform.scale = vec4_create(1, 1, 1);
	plane.base.transform.rot = quat_from_euler(-math_pi() / 2, 0, 0);
	transform_build_matrix(plane.base.transform);
	plane.base.visible = true;
	let meshes: mesh_object_t[] = scene_meshes;
	scene_meshes = [plane];
	let painto: mesh_object_t = context_raw.paint_object;
	context_raw.paint_object = plane;

	context_raw.saved_camera = mat4_clone(scene_camera.base.transform.local);
	let m: mat4_t = mat4_identity();
	m = mat4_translate(m, 0, 0, 1);
	transform_set_matrix(scene_camera.base.transform, m);
	let saved_fov: f32 = scene_camera.data.fov;
	scene_camera.data.fov = 0.92;
	viewport_update_camera_type(camera_type_t.PERSPECTIVE);
	let _light_strength: f32 = uniforms_light_strength;
	uniforms_light_strength = 0.0;
	scene_world._.envmap = context_raw.preview_envmap;

	// No resize
	_render_path_last_w = util_render_decal_preview_size;
	_render_path_last_h = util_render_decal_preview_size;
	camera_object_build_proj(scene_camera);
	camera_object_build_mat(scene_camera);

	make_material_parse_mesh_preview_material();
	let _commands: ()=>void = render_path_commands;
	render_path_commands = render_path_preview_commands_decal;
	render_path_render_frame();
	render_path_commands = _commands;

	context_raw.decal_preview = false;
	_render_path_last_w = app_w();
	_render_path_last_h = app_h();

	// Restore
	plane.base.visible = false;
	scene_meshes = meshes;
	context_raw.paint_object = painto;

	transform_set_matrix(scene_camera.base.transform, context_raw.saved_camera);
	scene_camera.data.fov = saved_fov;
	viewport_update_camera_type(context_raw.camera_type);
	camera_object_build_proj(scene_camera);
	camera_object_build_mat(scene_camera);

	uniforms_light_strength = _light_strength;

	scene_world._.envmap = context_raw.show_envmap ? context_raw.saved_envmap : context_raw.empty_envmap;

	make_material_parse_mesh_material();
	context_raw.ddirty = 1; // Refresh depth for decal paint

	if (g2_in_use) g2_begin(current);
}

function util_render_make_text_preview() {
	let current: image_t = _g2_current;
	let g2_in_use: bool = _g2_in_use;
	if (g2_in_use) g2_end();

	let text: string = context_raw.text_tool_text;
	let font: g2_font_t = context_raw.font.font;
	let font_size: i32 = util_render_font_preview_size;
	let text_w: i32 = math_floor(g2_font_width(font, font_size, text));
	let text_h: i32 = math_floor(g2_font_height(font, font_size));
	let tex_w: i32 = text_w + 32;
	if (tex_w < 512) {
		tex_w = 512;
	}
	if (context_raw.text_tool_image != null && context_raw.text_tool_image.width < tex_w) {
		image_unload(context_raw.text_tool_image);
		context_raw.text_tool_image = null;
	}
	if (context_raw.text_tool_image == null) {
		///if arm_metal
		context_raw.text_tool_image = image_create_render_target(tex_w, tex_w, tex_format_t.RGBA32);
		///else
		context_raw.text_tool_image = image_create_render_target(tex_w, tex_w, tex_format_t.R8);
		///end
	}
	g2_begin(context_raw.text_tool_image);
	g2_clear(0xff000000);
	g2_set_font(font);
	g2_set_font_size(font_size);
	g2_set_color(0xffffffff);
	g2_draw_string(text, tex_w / 2 - text_w / 2, tex_w / 2 - text_h / 2);
	g2_end();

	if (g2_in_use) g2_begin(current);
}

function util_render_make_font_preview() {
	let current: image_t = _g2_current;
	let g2_in_use: bool = _g2_in_use;
	if (g2_in_use) g2_end();

	let text: string = "Abg";
	let font: g2_font_t = context_raw.font.font;
	let font_size: i32 = util_render_font_preview_size;
	let text_w: i32 = math_floor(g2_font_width(font, font_size, text)) + 8;
	let text_h: i32 = math_floor(g2_font_height(font, font_size)) + 8;
	let tex_w: i32 = text_w + 32;
	if (context_raw.font.image == null) {
		context_raw.font.image = image_create_render_target(tex_w, tex_w, tex_format_t.RGBA32);
	}
	g2_begin(context_raw.font.image);
	g2_clear(0x00000000);
	g2_set_font(font);
	g2_set_font_size(font_size);
	g2_set_color(0xffffffff);
	g2_draw_string(text, tex_w / 2 - text_w / 2, tex_w / 2 - text_h / 2);
	g2_end();
	context_raw.font.preview_ready = true;

	if (g2_in_use) g2_begin(current);
}

function util_render_make_brush_preview() {
	if (render_path_paint_live_layer_locked) {
		return;
	}

	let current: image_t = _g2_current;
	let g2_in_use: bool = _g2_in_use;
	if (g2_in_use) g2_end();

	context_raw.material_preview = true;

	// Prepare layers
	if (render_path_paint_live_layer == null) {
		render_path_paint_live_layer = slot_layer_create("_live");
	}

	let l: slot_layer_t = render_path_paint_live_layer;
	slot_layer_clear(l);

	if (context_raw.brush.image == null) {
		context_raw.brush.image = image_create_render_target(util_render_material_preview_size, util_render_material_preview_size);
		context_raw.brush.image_icon = image_create_render_target(50, 50);
	}

	let _material: slot_material_t = context_raw.material;
	context_raw.material = slot_material_create();
	let _tool: workspace_tool_t = context_raw.tool;
	context_raw.tool = workspace_tool_t.BRUSH;

	let _layer: slot_layer_t = context_raw.layer;
	if (slot_layer_is_mask(context_raw.layer)) {
		context_raw.layer = context_raw.layer.parent;
	}

	let _fill_layer: slot_material_t = context_raw.layer.fill_layer;
	context_raw.layer.fill_layer = null;

	render_path_paint_use_live_layer(true);
	make_material_parse_paint_material(false);

	let hid: i32 = history_undo_i - 1 < 0 ? config_raw.undo_steps - 1 : history_undo_i - 1;
	map_set(render_path_render_targets, "texpaint_undo" + hid, map_get(render_path_render_targets, "empty_black"));

	// Set plane mesh
	let painto: mesh_object_t = context_raw.paint_object;
	let visibles: bool[] = [];
	for (let i: i32 = 0; i < project_paint_objects.length; ++i) {
		let p: mesh_object_t = project_paint_objects[i];
		array_push(visibles, p.base.visible);
		p.base.visible = false;
	}
	let merged_object_visible: bool = false;
	if (context_raw.merged_object != null) {
		merged_object_visible = context_raw.merged_object.base.visible;
		context_raw.merged_object.base.visible = false;
	}

	let cam: camera_object_t = scene_camera;
	context_raw.saved_camera = mat4_clone(cam.base.transform.local);
	let saved_fov: f32 = cam.data.fov;
	viewport_update_camera_type(camera_type_t.PERSPECTIVE);
	let m: mat4_t = mat4_identity();
	m = mat4_translate(m, 0, 0, 0.5);
	transform_set_matrix(cam.base.transform, m);
	cam.data.fov = 0.92;
	camera_object_build_proj(cam);
	camera_object_build_mat(cam);
	m = mat4_inv(scene_camera.vp);

	let planeo: mesh_object_t = scene_get_child(".Plane").ext;
	planeo.base.visible = true;
	context_raw.paint_object = planeo;

	let v: vec4_t = vec4_create();
	v = vec4_create(m.m00, m.m01, m.m02);
	let sx: f32 = vec4_len(v);
	planeo.base.transform.rot = quat_from_euler(-math_pi() / 2, 0, 0);
	planeo.base.transform.scale = vec4_create(sx, 1.0, sx);
	planeo.base.transform.loc = vec4_create(m.m30, -m.m31, 0.0);
	transform_build_matrix(planeo.base.transform);

	render_path_paint_live_layer_drawn = 0;
	render_path_base_draw_gbuffer();

	// Paint brush preview
	let _brush_radius: f32 = context_raw.brush_radius;
	let _brush_opacity: f32 = context_raw.brush_opacity;
	let _brush_hardness: f32 = context_raw.brush_hardness;
	context_raw.brush_radius = 0.33;
	context_raw.brush_opacity = 1.0;
	context_raw.brush_hardness = 0.8;
	let _x: f32 = context_raw.paint_vec.x;
	let _y: f32 = context_raw.paint_vec.y;
	let _last_x: f32 = context_raw.last_paint_vec_x;
	let _last_y: f32 = context_raw.last_paint_vec_y;
	let _pdirty: i32 = context_raw.pdirty;
	context_raw.pdirty = 2;

	let points_x: f32[] = [
		0.2, 0.2,
		0.35, 0.5,
		0.5, 0.5,
		0.65, 0.8,
		0.8, 0.8
	];
	let points_y: f32[] = [
		0.5, 0.5,
		0.35 - 0.04, 0.2 - 0.08,
		0.4 + 0.015, 0.6 + 0.03,
		0.45 - 0.025, 0.3 - 0.05,
		0.5 + 0.025, 0.7 + 0.05
	];
	for (let i: i32 = 1; i < points_x.length; ++i) {
		context_raw.last_paint_vec_x = points_x[i - 1];
		context_raw.last_paint_vec_y = points_y[i - 1];
		context_raw.paint_vec.x = points_x[i];
		context_raw.paint_vec.y = points_y[i];
		render_path_paint_commands_paint(false);
	}

	context_raw.brush_radius = _brush_radius;
	context_raw.brush_opacity = _brush_opacity;
	context_raw.brush_hardness = _brush_hardness;
	context_raw.paint_vec.x = _x;
	context_raw.paint_vec.y = _y;
	context_raw.last_paint_vec_x = _last_x;
	context_raw.last_paint_vec_y = _last_y;
	context_raw.prev_paint_vec_x = -1;
	context_raw.prev_paint_vec_y = -1;
	context_raw.pdirty = _pdirty;
	render_path_paint_use_live_layer(false);
	context_raw.layer.fill_layer = _fill_layer;
	context_raw.layer = _layer;
	context_raw.material = _material;
	context_raw.tool = _tool;
	app_notify_on_init(function () {
		make_material_parse_paint_material(false);
	});

	// Restore paint mesh
	context_raw.material_preview = false;
	planeo.base.visible = false;
	for (let i: i32 = 0; i < project_paint_objects.length; ++i) {
		project_paint_objects[i].base.visible = visibles[i];
	}
	if (context_raw.merged_object != null) {
		context_raw.merged_object.base.visible = merged_object_visible;
	}
	context_raw.paint_object = painto;
	transform_set_matrix(scene_camera.base.transform, context_raw.saved_camera);
	scene_camera.data.fov = saved_fov;
	viewport_update_camera_type(context_raw.camera_type);
	camera_object_build_proj(scene_camera);
	camera_object_build_mat(scene_camera);

	// Scale layer down to to image preview
	l = render_path_paint_live_layer;
	let target: image_t = context_raw.brush.image;
	g2_begin(target);
	g2_clear(0x00000000);
	g2_set_pipeline(pipes_copy);
	g2_draw_scaled_image(l.texpaint, 0, 0, target.width, target.height);
	g2_set_pipeline(null);
	g2_end();

	// Scale image preview down to to icon
	let texpreview: render_target_t = map_get(render_path_render_targets, "texpreview");
	texpreview._image = context_raw.brush.image;
	let texpreview_icon: render_target_t = map_get(render_path_render_targets, "texpreview_icon");
	texpreview_icon._image = context_raw.brush.image_icon;
	render_path_set_target("texpreview_icon");
	render_path_bind_target("texpreview", "tex");
	render_path_draw_shader("shader_datas/supersample_resolve/supersample_resolve");

	context_raw.brush.preview_ready = true;
	context_raw.brush_blend_dirty = true;

	if (g2_in_use) g2_begin(current);
}

function util_render_make_node_preview(canvas: ui_node_canvas_t, node: ui_node_t, image: image_t, group: ui_node_canvas_t = null, parents: ui_node_t[] = null) {
	let res: parse_node_preview_result_t = make_material_parse_node_preview_material(node, group, parents);
	if (res == null || res.scon == null) {
		return;
	}

	if (util_render_screen_aligned_full_vb == null) {
		util_render_create_screen_aligned_full_data();
	}

	let _scale_world: f32 = context_raw.paint_object.base.transform.scale_world;
	context_raw.paint_object.base.transform.scale_world = 3.0;
	transform_build_matrix(context_raw.paint_object.base.transform);

	g4_begin(image);
	g4_set_pipeline(res.scon._.pipe_state);
	let empty: string[] = [""];
	uniforms_set_context_consts(res.scon, empty);
	uniforms_set_obj_consts(res.scon, context_raw.paint_object.base);
	uniforms_set_material_consts(res.scon, res.mcon);
	g4_set_vertex_buffer(util_render_screen_aligned_full_vb);
	g4_set_index_buffer(util_render_screen_aligned_full_ib);
	g4_draw();
	g4_end();

	context_raw.paint_object.base.transform.scale_world = _scale_world;
	transform_build_matrix(context_raw.paint_object.base.transform);
}

function util_render_pick_pos_nor_tex() {
	context_raw.pick_pos_nor_tex = true;
	context_raw.pdirty = 1;
	let _tool: workspace_tool_t = context_raw.tool;
	context_raw.tool = workspace_tool_t.PICKER;
	make_material_parse_paint_material();
	if (context_raw.paint2d) {
		render_path_paint_set_plane_mesh();
	}
	render_path_paint_commands_paint(false);
	///if arm_metal
	// Flush command list
	render_path_paint_commands_paint(false);
	///end
	if (context_raw.paint2d) {
		render_path_paint_restore_plane_mesh();
	}
	context_raw.tool = _tool;
	context_raw.pick_pos_nor_tex = false;
	make_material_parse_paint_material();
	context_raw.pdirty = 0;
}

function util_render_get_decal_mat(): mat4_t {
	util_render_pick_pos_nor_tex();
	let decal_mat: mat4_t = mat4_identity();
	let loc: vec4_t = vec4_create(context_raw.posx_picked, context_raw.posy_picked, context_raw.posz_picked);
	let rot: quat_t = quat_from_to(vec4_create(0.0, 0.0, -1.0), vec4_create(context_raw.norx_picked, context_raw.nory_picked, context_raw.norz_picked));
	let scale: vec4_t = vec4_create(context_raw.brush_radius * 0.5, context_raw.brush_radius * 0.5, context_raw.brush_radius * 0.5);
	decal_mat = mat4_compose(loc, rot, scale);
	return decal_mat;
}

function util_render_create_screen_aligned_full_data() {
	// Over-sized triangle
	let data: i16[] = [-math_floor(32767 / 3), -math_floor(32767 / 3), 0, 32767, 0, 0, 0, 0, 0, 0, 0, 0,
						32767,                 -math_floor(32767 / 3), 0, 32767, 0, 0, 0, 0, 0, 0, 0, 0,
					   -math_floor(32767 / 3),  32767,                 0, 32767, 0, 0, 0, 0, 0, 0, 0, 0];
	let indices: u32[] = [0, 1, 2];

	// Mandatory vertex data names and sizes
	let structure: vertex_struct_t = g4_vertex_struct_create();
	g4_vertex_struct_add(structure, "pos", vertex_data_t.I16_4X_NORM);
	g4_vertex_struct_add(structure, "nor", vertex_data_t.I16_2X_NORM);
	g4_vertex_struct_add(structure, "tex", vertex_data_t.I16_2X_NORM);
	g4_vertex_struct_add(structure, "col", vertex_data_t.I16_4X_NORM);
	util_render_screen_aligned_full_vb = g4_vertex_buffer_create(math_floor(data.length / math_floor(g4_vertex_struct_byte_size(structure) / 2)), structure, usage_t.STATIC);
	let vertices: buffer_t = g4_vertex_buffer_lock(util_render_screen_aligned_full_vb);
	for (let i: i32 = 0; i < math_floor((vertices.length) / 2); ++i) {
		buffer_set_i16(vertices, i * 2, data[i]);
	}
	g4_vertex_buffer_unlock(util_render_screen_aligned_full_vb);

	util_render_screen_aligned_full_ib = g4_index_buffer_create(indices.length);
	let id: u32_array_t = g4_index_buffer_lock(util_render_screen_aligned_full_ib);
	for (let i: i32 = 0; i < id.length; ++i) {
		id[i] = indices[i];
	}
	g4_index_buffer_unlock(util_render_screen_aligned_full_ib);
}
