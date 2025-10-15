
type context_t = {
	texture?: asset_t;
	paint_object?: mesh_object_t;
	merged_object?: mesh_object_t;
	merged_object_is_atlas?: bool;
	ddirty?: i32;
	pdirty?: i32;
	rdirty?: i32;
	brush_blend_dirty?: bool;
	split_view?: bool;
	view_index?: i32;
	view_index_last?: i32;
	swatch?: swatch_color_t;
	picked_color?: swatch_color_t;
	color_picker_callback?: (sc: swatch_color_t) => void;
	default_irradiance?: f32_array_t;
	default_radiance?: gpu_texture_t;
	default_radiance_mipmaps?: gpu_texture_t[];
	saved_envmap?: gpu_texture_t;
	empty_envmap?: gpu_texture_t;
	preview_envmap?: gpu_texture_t;
	envmap_loaded?: bool;
	show_envmap?: bool;
	show_envmap_handle?: ui_handle_t;
	show_envmap_blur?: bool;
	show_envmap_blur_handle?: ui_handle_t;
	envmap_angle?: f32;
	light_angle?: f32;
	cull_backfaces?: bool;
	texture_filter?: bool;
	format_type?: texture_ldr_format_t;
	format_quality?: f32;
	layers_destination?: export_destination_t;
	split_by?: split_type_t;
	parse_transform?: bool;
	parse_vcols?: bool;
	select_time?: f32;
	viewport_mode?: viewport_mode_t;
	render_mode?: render_mode_t;
	viewport_shader?: any; // JSValue * -> (ns: node_shader_t)=>void;
	hscale_was_changed?: bool;
	export_mesh_format?: mesh_format_t;
	export_mesh_index?: i32;
	pack_assets_on_export?: bool;
	paint_vec?: vec4_t;
	last_paint_x?: f32;
	last_paint_y?: f32;
	foreground_event?: bool;
	painted?: i32;
	brush_time?: f32;
	clone_start_x?: f32;
	clone_start_y?: f32;
	clone_delta_x?: f32;
	clone_delta_y?: f32;
	show_compass?: bool;
	project_type?: i32;
	project_aspect_ratio?: i32;
	project_objects?: mesh_object_t[];
	last_paint_vec_x?: f32;
	last_paint_vec_y?: f32;
	prev_paint_vec_x?: f32;
	prev_paint_vec_y?: f32;
	frame?: i32;
	paint2d_view?: bool;
	brush_locked?: bool;
	camera_type?: camera_type_t;
	cam_handle?: ui_handle_t;
	fov_handle?: ui_handle_t;
	undo_handle?: ui_handle_t;
	hssao?: ui_handle_t;
	hbloom?: ui_handle_t;
	hsupersample?: ui_handle_t;
	texture_export_path?: string;
	last_status_position?: i32;
	camera_controls?: camera_controls_t;
	pen_painting_only?: bool;
	material?: slot_material_t;
	layer?: slot_layer_t;
	brush?: slot_brush_t;
	font?: slot_font_t;
	tool?: tool_type_t;
	layer_preview_dirty?: bool;
	layers_preview_dirty?: bool;
	node_preview_socket_map?: map_t<i32, i32>;
	node_preview_map?: map_t<i32, gpu_texture_t>;
	node_preview_name?: string;
	node_previews?: map_t<string, gpu_texture_t>;
	node_previews_used?: string[]; selected_node_preview : bool;
	mask_preview_rgba32?: gpu_texture_t;
	mask_preview_last?: slot_layer_t;
	colorid_picked?: bool;
	material_preview?: bool;
	saved_camera?: mat4_t;
	color_picker_previous_tool?: tool_type_t;
	materialid_picked?: i32;
	uvx_picked?: f32;
	uvy_picked?: f32;
	picker_select_material?: bool;
	picker_mask_handle?: ui_handle_t;
	pick_pos_nor_tex?: bool;
	posx_picked?: f32;
	posy_picked?: f32;
	posz_picked?: f32;
	norx_picked?: f32;
	nory_picked?: f32;
	norz_picked?: f32;
	draw_wireframe?: bool;
	wireframe_handle?: ui_handle_t;
	draw_texels?: bool;
	texels_handle?: ui_handle_t;
	colorid_handle?: ui_handle_t;
	layers_export?: export_mode_t;
	decal_image?: gpu_texture_t;
	decal_preview?: bool;
	decal_x?: f32;
	decal_y?: f32;
	// cache_draws?: bool;
	write_icon_on_export?: bool;
	text_tool_image?: gpu_texture_t;
	text_tool_text?: string;
	particle_material?: material_data_t;
	layer_filter?: i32;
	brush_output_node_inst?: brush_output_node_t;
	run_brush?: (self: any, i: i32)  => void;
	parse_brush_inputs?: (self: any) => void;
	gizmo?: object_t;
	gizmo_translate_x?: object_t;
	gizmo_translate_y?: object_t;
	gizmo_translate_z?: object_t;
	gizmo_scale_x?: object_t;
	gizmo_scale_y?: object_t;
	gizmo_scale_z?: object_t;
	gizmo_rotate_x?: object_t;
	gizmo_rotate_y?: object_t;
	gizmo_rotate_z?: object_t;
	gizmo_started?: bool;
	gizmo_offset?: f32;
	gizmo_drag?: f32;
	gizmo_drag_last?: f32;
	translate_x?: bool;
	translate_y?: bool;
	translate_z?: bool;
	scale_x?: bool;
	scale_y?: bool;
	scale_z?: bool;
	rotate_x?: bool;
	rotate_y?: bool;
	rotate_z?: bool;
	brush_nodes_radius?: f32;
	brush_nodes_opacity?: f32;
	brush_mask_image?: gpu_texture_t;
	brush_mask_image_is_alpha?: bool;
	brush_stencil_image?: gpu_texture_t;
	brush_stencil_image_is_alpha?: bool;
	brush_stencil_x?: f32;
	brush_stencil_y?: f32;
	brush_stencil_scale?: f32;
	brush_stencil_scaling?: bool;
	brush_stencil_angle?: f32;
	brush_stencil_rotating?: bool;
	brush_nodes_scale?: f32;
	brush_nodes_angle?: f32;
	brush_nodes_hardness?: f32;
	brush_directional?: bool;
	brush_radius?: f32;
	brush_radius_handle?: ui_handle_t;
	brush_scale_x?: f32;
	brush_decal_mask_radius?: f32;
	brush_decal_mask_radius_handle?: ui_handle_t;
	brush_scale_x_handle?: ui_handle_t;
	brush_blending?: blend_type_t;
	brush_opacity?: f32;
	brush_opacity_handle?: ui_handle_t;
	brush_scale?: f32;
	brush_angle?: f32;
	brush_angle_handle?: ui_handle_t;
	brush_hardness?: f32;
	brush_lazy_radius?: f32;
	brush_lazy_step?: f32;
	brush_lazy_x?: f32;
	brush_lazy_y?: f32;
	brush_paint?: uv_type_t;
	brush_angle_reject_dot?: f32;
	bake_type?: bake_type_t;
	bake_axis?: bake_axis_t;
	bake_up_axis?: bake_up_axis_t;
	bake_samples?: i32;
	bake_ao_strength?: f32;
	bake_ao_radius?: f32;
	bake_ao_offset?: f32;
	bake_curv_strength?: f32;
	bake_curv_radius?: f32;
	bake_curv_offset?: f32;
	bake_curv_smooth?: i32;
	bake_high_poly?: i32;
	xray?: bool;
	sym_x?: bool;
	sym_y?: bool;
	sym_z?: bool;
	fill_type_handle?: ui_handle_t;
	paint2d?: bool;
	maximized_sidebar_width?: i32;
	drag_dest?: i32;
	coords?: vec4_t;
	start_x?: f32;
	start_y?: f32;
	lock_begin?: bool;
	lock_x?: bool;
	lock_y?: bool;
	lock_start_x?: f32;
	lock_start_y?: f32;
	registered?: bool;
	selected_object?: object_t;
	particle_hit_x?: f32;
	particle_hit_y?: f32;
	particle_hit_z?: f32;
	last_particle_hit_x?: f32;
	last_particle_hit_y?: f32;
	last_particle_hit_z?: f32;
	particle_timer?: tween_anim_t;
	paint_body?: physics_body_t;
}

let context_raw: context_t;

function context_create(): context_t {
	let c: context_t          = {};
	c.merged_object_is_atlas  = false; // Only objects referenced by atlas are merged
	c.ddirty                  = 0;     // depth
	c.pdirty                  = 0;     // paint
	c.rdirty                  = 0;     // render
	c.brush_blend_dirty       = true;
	c.split_view              = false;
	c.view_index              = -1;
	c.view_index_last         = -1;
	c.picked_color            = make_swatch();
	c.envmap_loaded           = false;
	c.show_envmap             = false;
	c.show_envmap_handle      = ui_handle_create();
	c.show_envmap_blur        = false;
	c.show_envmap_blur_handle = ui_handle_create();
	c.envmap_angle            = 0.0;
	c.light_angle             = 0.0;
	c.cull_backfaces          = true;
	c.texture_filter          = true;
	c.format_type             = texture_ldr_format_t.PNG;
	c.format_quality          = 100.0;
	c.layers_destination      = export_destination_t.DISK;
	c.split_by                = split_type_t.OBJECT;
	c.parse_transform         = true;
	c.parse_vcols             = false;
	c.select_time             = 0.0;
	c.viewport_mode           = config_raw.viewport_mode == 0 ? viewport_mode_t.LIT : viewport_mode_t.PATH_TRACE;
	/// if (arm_android || arm_ios)
	c.render_mode = render_mode_t.FORWARD;
	/// else
	c.render_mode = render_mode_t.DEFERRED;
	/// end
	c.hscale_was_changed      = false;
	c.export_mesh_format      = mesh_format_t.OBJ;
	c.export_mesh_index       = 0;
	c.pack_assets_on_export   = true;
	c.paint_vec               = vec4_create();
	c.last_paint_x            = -1.0;
	c.last_paint_y            = -1.0;
	c.foreground_event        = false;
	c.painted                 = 0;
	c.brush_time              = 0.0;
	c.clone_start_x           = -1.0;
	c.clone_start_y           = -1.0;
	c.clone_delta_x           = 0.0;
	c.clone_delta_y           = 0.0;
	c.show_compass            = true;
	c.project_aspect_ratio    = 0; // 1:1, 2:1, 1:2
	c.last_paint_vec_x        = -1.0;
	c.last_paint_vec_y        = -1.0;
	c.prev_paint_vec_x        = -1.0;
	c.prev_paint_vec_y        = -1.0;
	c.frame                   = 0;
	c.paint2d_view            = false;
	c.brush_locked            = false;
	c.camera_type             = camera_type_t.PERSPECTIVE;
	c.cam_handle              = ui_handle_create();
	c.hssao                   = ui_handle_create();
	c.hbloom                  = ui_handle_create();
	c.hsupersample            = ui_handle_create();
	c.hssao.b                 = config_raw.rp_ssao;
	c.hbloom.b                = config_raw.rp_bloom;
	c.hsupersample.i          = config_get_super_sample_quality(config_raw.rp_supersample);
	c.texture_export_path     = "";
	c.last_status_position    = 0;
	c.camera_controls         = camera_controls_t.ORBIT;
	c.pen_painting_only       = false; // Reject painting with finger when using pen
	c.layer_preview_dirty     = true;
	c.layers_preview_dirty    = false;
	c.node_preview_name       = "";
	c.node_preview_socket_map = map_create();
	c.node_preview_map        = map_create();
	c.selected_node_preview   = true;
	c.colorid_picked          = false;
	c.material_preview        = false; // Drawing material previews
	c.saved_camera            = mat4_identity();
	c.materialid_picked       = 0;
	c.uvx_picked              = 0.0;
	c.uvy_picked              = 0.0;
	c.picker_select_material  = false;
	c.picker_mask_handle      = ui_handle_create();
	c.pick_pos_nor_tex        = false;
	c.posx_picked             = 0.0;
	c.posy_picked             = 0.0;
	c.posz_picked             = 0.0;
	c.norx_picked             = 0.0;
	c.nory_picked             = 0.0;
	c.norz_picked             = 0.0;
	c.draw_wireframe          = false;
	c.wireframe_handle        = ui_handle_create();
	c.draw_texels             = false;
	c.texels_handle           = ui_handle_create();
	c.colorid_handle          = ui_handle_create();
	c.layers_export           = export_mode_t.VISIBLE;
	c.decal_preview           = false;
	c.decal_x                 = 0.0;
	c.decal_y                 = 0.0;
	// c.cache_draws = false;
	c.write_icon_on_export             = false;
	c.particle_hit_x                   = 0.0;
	c.particle_hit_y                   = 0.0;
	c.particle_hit_z                   = 0.0;
	c.last_particle_hit_x              = 0.0;
	c.last_particle_hit_y              = 0.0;
	c.last_particle_hit_z              = 0.0;
	c.layer_filter                     = 0;
	c.gizmo_started                    = false;
	c.gizmo_offset                     = 0.0;
	c.gizmo_drag                       = 0.0;
	c.gizmo_drag_last                  = 0.0;
	c.translate_x                      = false;
	c.translate_y                      = false;
	c.translate_z                      = false;
	c.scale_x                          = false;
	c.scale_y                          = false;
	c.scale_z                          = false;
	c.rotate_x                         = false;
	c.rotate_y                         = false;
	c.rotate_z                         = false;
	c.brush_nodes_radius               = 1.0;
	c.brush_nodes_opacity              = 1.0;
	c.brush_mask_image_is_alpha        = false;
	c.brush_stencil_image_is_alpha     = false;
	c.brush_stencil_x                  = 0.02;
	c.brush_stencil_y                  = 0.02;
	c.brush_stencil_scale              = 0.9;
	c.brush_stencil_scaling            = false;
	c.brush_stencil_angle              = 0.0;
	c.brush_stencil_rotating           = false;
	c.brush_nodes_scale                = 1.0;
	c.brush_nodes_angle                = 0.0;
	c.brush_nodes_hardness             = 1.0;
	c.brush_directional                = false;
	c.brush_radius_handle              = ui_handle_create();
	c.brush_scale_x                    = 1.0;
	c.brush_decal_mask_radius          = 0.5;
	c.brush_decal_mask_radius_handle   = ui_handle_create();
	c.brush_decal_mask_radius_handle.f = 0.5;
	c.brush_scale_x_handle             = ui_handle_create();
	c.brush_scale_x_handle.f           = 1.0;
	c.brush_blending                   = blend_type_t.MIX;
	c.brush_opacity                    = 1.0;
	c.brush_opacity_handle             = ui_handle_create();
	c.brush_opacity_handle.f           = 1.0;
	c.brush_scale                      = 1.0;
	c.brush_angle                      = 0.0;
	c.brush_angle_handle               = ui_handle_create();
	c.brush_angle_handle.f             = 0.0;
	c.brush_lazy_radius                = 0.0;
	c.brush_lazy_step                  = 0.0;
	c.brush_lazy_x                     = 0.0;
	c.brush_lazy_y                     = 0.0;
	c.brush_paint                      = uv_type_t.UVMAP;
	c.brush_angle_reject_dot           = 0.5;
	c.bake_type                        = bake_type_t.CURVATURE;
	c.bake_axis                        = bake_axis_t.XYZ;
	c.bake_up_axis                     = bake_up_axis_t.Z;
	c.bake_samples                     = 128;
	c.bake_ao_strength                 = 1.0;
	c.bake_ao_radius                   = 1.0;
	c.bake_ao_offset                   = 1.0;
	c.bake_curv_strength               = 1.0;
	c.bake_curv_radius                 = 1.0;
	c.bake_curv_offset                 = 0.0;
	c.bake_curv_smooth                 = 1;
	c.bake_high_poly                   = 0;
	c.xray                             = false;
	c.sym_x                            = false;
	c.sym_y                            = false;
	c.sym_z                            = false;
	c.fill_type_handle                 = ui_handle_create();
	c.paint2d                          = false;
	c.maximized_sidebar_width          = 0;
	c.drag_dest                        = 0;
	return c;
}

function context_init() {
	context_raw                            = context_create();
	context_raw.tool                       = tool_type_t.BRUSH;
	context_raw.color_picker_previous_tool = tool_type_t.BRUSH;
	context_raw.brush_radius               = 0.5;
	context_raw.brush_radius_handle.f      = 0.5;
	context_raw.brush_hardness             = 0.8;
}

function context_use_deferred(): bool {
	return context_raw.render_mode != render_mode_t.FORWARD &&
	       (context_raw.viewport_mode == viewport_mode_t.LIT || context_raw.viewport_mode == viewport_mode_t.PATH_TRACE) &&
	       context_raw.tool != tool_type_t.COLORID;
}

function context_select_material(i: i32) {
	if (project_materials.length <= i) {
		return;
	}
	context_set_material(project_materials[i]);
}

function context_set_material(m: slot_material_t) {
	if (array_index_of(project_materials, m) == -1) {
		return;
	}
	context_raw.material = m;
	make_material_parse_paint_material();
	ui_base_hwnds[tab_area_t.SIDEBAR1].redraws = 2;
	ui_header_handle.redraws                   = 2;
	ui_nodes_hwnd.redraws                      = 2;
	ui_nodes_group_stack                       = [];

	let decal: bool = context_is_decal();
	if (decal) {
		sys_notify_on_next_frame(util_render_make_decal_preview);
	}
}

function context_select_brush(i: i32) {
	if (project_brushes.length <= i) {
		return;
	}
	context_set_brush(project_brushes[i]);
}

function context_set_brush(b: slot_brush_t) {
	if (array_index_of(project_brushes, b) == -1) {
		return;
	}
	context_raw.brush = b;
	make_material_parse_brush();
	ui_base_hwnds[tab_area_t.SIDEBAR1].redraws = 2;
	ui_nodes_hwnd.redraws                      = 2;
}

function context_select_font(i: i32) {
	if (project_fonts.length <= i) {
		return;
	}
	context_set_font(project_fonts[i]);
}

function context_set_font(f: slot_font_t) {
	if (array_index_of(project_fonts, f) == -1) {
		return;
	}
	context_raw.font = f;
	util_render_make_text_preview();
	util_render_make_decal_preview();
	ui_base_hwnds[tab_area_t.STATUS].redraws = 2;
	ui_view2d_hwnd.redraws                   = 2;
}

function context_select_layer(i: i32) {
	if (project_layers.length <= i) {
		return;
	}
	context_set_layer(project_layers[i]);
}

function context_set_layer(l: slot_layer_t) {
	if (l == context_raw.layer) {
		return;
	}
	context_raw.layer        = l;
	ui_header_handle.redraws = 2;

	let current: gpu_texture_t = _draw_current;
	let in_use: bool           = gpu_in_use;
	if (in_use)
		draw_end();

	layers_set_object_mask();
	make_material_parse_mesh_material();
	make_material_parse_paint_material();

	if (in_use)
		draw_begin(current);

	ui_base_hwnds[tab_area_t.SIDEBAR0].redraws = 2;
	ui_view2d_hwnd.redraws                     = 2;
}

function context_select_tool(i: i32) {
	context_raw.tool = i;
	make_material_parse_paint_material();
	make_material_parse_mesh_material();
	context_raw.ddirty                  = 3;
	let _viewport_mode: viewport_mode_t = context_raw.viewport_mode;
	context_raw.viewport_mode           = viewport_mode_t.MINUS_ONE;
	context_set_viewport_mode(_viewport_mode);

	context_init_tool();
	ui_header_handle.redraws  = 2;
	ui_toolbar_handle.redraws = 2;
}

function context_init_tool() {
	let decal: bool = context_is_decal();
	if (decal) {
		if (context_raw.tool == tool_type_t.TEXT) {
			util_render_make_text_preview();
		}
		util_render_make_decal_preview();
	}
	else if (context_raw.tool == tool_type_t.PARTICLE) {
		util_particle_init();
	}
	else if (context_raw.tool == tool_type_t.BAKE) {
		// Bake in lit mode for now
		if (context_raw.viewport_mode == viewport_mode_t.PATH_TRACE) {
			context_raw.viewport_mode = viewport_mode_t.LIT;
		}
	}
	else if (context_raw.tool == tool_type_t.MATERIAL) {
		layers_update_fill_layers();
		context_main_object().skip_context = null;
	}
}

function context_select_paint_object(o: mesh_object_t) {
	ui_header_handle.redraws = 2;
	for (let i: i32 = 0; i < project_paint_objects.length; ++i) {
		let p: mesh_object_t = project_paint_objects[i];
		p.skip_context       = "paint";
	}

	/// if_forge
	context_raw.paint_object.skip_context = "";
	/// end

	context_raw.paint_object = o;

	let mask: i32 = slot_layer_get_object_mask(context_raw.layer);
	if (context_layer_filter_used()) {
		mask = context_raw.layer_filter;
	}

	if (context_raw.merged_object == null || mask > 0) {
		context_raw.paint_object.skip_context = "";
	}
	util_uv_uvmap_cached       = false;
	util_uv_trianglemap_cached = false;
	util_uv_dilatemap_cached   = false;
}

function context_main_object(): mesh_object_t {
	for (let i: i32 = 0; i < project_paint_objects.length; ++i) {
		let po: mesh_object_t = project_paint_objects[i];
		if (po.base.children.length > 0) {
			return po;
		}
	}
	return project_paint_objects[0];
}

function context_layer_filter_used(): bool {
	return context_raw.layer_filter > 0 && context_raw.layer_filter <= project_paint_objects.length;
}

function context_object_mask_used(): bool {
	return slot_layer_get_object_mask(context_raw.layer) > 0 && slot_layer_get_object_mask(context_raw.layer) <= project_paint_objects.length;
}

function context_in_3d_view(): bool {
	return context_raw.paint_vec.x < 1 && context_raw.paint_vec.x > 0 && context_raw.paint_vec.y < 1 && context_raw.paint_vec.y > 0;
}

function context_in_paint_area(): bool {
	return context_in_3d_view() || context_in_2d_view();
}

function context_in_layers(): bool {
	let tab: string = ui_hovered_tab_name();
	return tab == tr("Layers");
}

function context_in_materials(): bool {
	let tab: string = ui_hovered_tab_name();
	return tab == tr("Materials");
}

function context_in_2d_view(type: view_2d_type_t = view_2d_type_t.LAYER): bool {
	return ui_view2d_show && ui_view2d_type == type && mouse_x > ui_view2d_wx && mouse_x < ui_view2d_wx + ui_view2d_ww && mouse_y > ui_view2d_wy &&
	       mouse_y < ui_view2d_wy + ui_view2d_wh;
}

function context_in_nodes(): bool {
	return ui_nodes_show && mouse_x > ui_nodes_wx && mouse_x < ui_nodes_wx + ui_nodes_ww && mouse_y > ui_nodes_wy && mouse_y < ui_nodes_wy + ui_nodes_wh;
}

function context_in_swatches(): bool {
	let tab: string = ui_hovered_tab_name();
	return tab == tr("Swatches");
}

function context_in_browser(): bool {
	let tab: string = ui_hovered_tab_name();
	return tab == tr("Browser");
}

function context_is_picker(): bool {
	return context_raw.tool == tool_type_t.PICKER || context_raw.tool == tool_type_t.MATERIAL;
}

function context_is_decal(): bool {
	return context_raw.tool == tool_type_t.DECAL || context_raw.tool == tool_type_t.TEXT;
}

function context_is_decal_mask(): bool {
	return context_is_decal() && operator_shortcut(map_get(config_keymap, "decal_mask"), shortcut_type_t.DOWN);
}

function context_is_decal_mask_paint(): bool {
	return context_is_decal() && operator_shortcut(map_get(config_keymap, "decal_mask") + "+" + map_get(config_keymap, "action_paint"), shortcut_type_t.DOWN);
}

function context_is_floating_toolbar(): bool {
	// Header is off -> floating toolbar
	return config_raw.layout[layout_size_t.HEADER] == 0;
}

function context_get_area_type(): area_type_t {
	if (context_in_3d_view()) {
		return area_type_t.VIEW3D;
	}
	if (context_in_nodes()) {
		return area_type_t.NODES;
	}
	if (context_in_browser()) {
		return area_type_t.BROWSER;
	}
	if (context_in_2d_view()) {
		return area_type_t.VIEW2D;
	}
	if (context_in_layers()) {
		return area_type_t.LAYERS;
	}
	if (context_in_materials()) {
		return area_type_t.MATERIALS;
	}
	return area_type_t.MINUS_ONE;
}

function context_set_viewport_mode(mode: viewport_mode_t) {
	if (mode == context_raw.viewport_mode) {
		return;
	}

	context_raw.viewport_mode = mode;
	if (context_use_deferred()) {
		render_path_commands = render_path_deferred_commands;
	}
	else {
		render_path_commands = render_path_forward_commands;
	}
	make_material_parse_mesh_material();

	// Rotate mode is not supported for path tracing yet
	if (context_raw.viewport_mode == viewport_mode_t.PATH_TRACE && context_raw.camera_controls == camera_controls_t.ROTATE) {
		context_raw.camera_controls = camera_controls_t.ORBIT;
		viewport_reset();
	}
	// Bake in lit mode for now
	if (context_raw.viewport_mode == viewport_mode_t.PATH_TRACE && context_raw.tool == tool_type_t.BAKE) {
		context_raw.viewport_mode = viewport_mode_t.LIT;
	}
}

function context_load_envmap() {
	if (!context_raw.envmap_loaded) {
		// TODO: Unable to share texture for both radiance and envmap - reload image
		context_raw.envmap_loaded = true;
		map_delete(data_cached_images, "World_radiance.k");
	}
	world_data_load_envmap(scene_world);
	if (context_raw.saved_envmap == null) {
		context_raw.saved_envmap = scene_world._.envmap;
	}
}

function context_update_envmap() {
	if (context_raw.show_envmap) {
		scene_world._.envmap = context_raw.show_envmap_blur ? scene_world._.radiance_mipmaps[0] : context_raw.saved_envmap;
	}
	else {
		scene_world._.envmap = context_raw.empty_envmap;
	}
}

function context_set_viewport_shader(viewport_shader: any) { // JSValue * -> (ns: node_shader_t)=>void
	context_raw.viewport_shader = viewport_shader;
	context_set_render_path();
}

function context_set_render_path() {
	if (context_raw.render_mode == render_mode_t.FORWARD || context_raw.viewport_shader != null) {
		render_path_commands = render_path_forward_commands;
	}
	else {
		render_path_commands = render_path_deferred_commands;
	}
	sys_notify_on_next_frame(make_material_parse_mesh_material);
}

function context_enable_import_plugin(file: string): bool {
	// Return plugin name suitable for importing the specified file
	if (box_preferences_files_plugin == null) {
		box_preferences_fetch_plugins();
	}
	let ext: string = substring(file, string_last_index_of(file, ".") + 1, file.length);
	for (let i: i32 = 0; i < box_preferences_files_plugin.length; ++i) {
		let f: string = box_preferences_files_plugin[i];
		if (starts_with(f, "import_") && string_index_of(f, ext) >= 0) {
			config_enable_plugin(f);
			console_info(f + " " + tr("plugin enabled"));
			return true;
		}
	}
	return false;
}

function context_set_swatch(s: swatch_color_t) {
	context_raw.swatch = s;
}
