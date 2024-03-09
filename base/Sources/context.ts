/// <reference path='./ContextFormat.ts'/>

let context_raw: context_t = new context_t(); //{};

function context_use_deferred(): bool {
	///if is_paint
	return context_raw.render_mode != render_mode_t.FORWARD && (context_raw.viewport_mode == viewport_mode_t.LIT || context_raw.viewport_mode == viewport_mode_t.PATH_TRACE) && context_raw.tool != workspace_tool_t.COLORID;
	///end

	///if (is_sculpt || is_lab)
	return context_raw.render_mode != render_mode_t.FORWARD && (context_raw.viewport_mode == viewport_mode_t.LIT || context_raw.viewport_mode == viewport_mode_t.PATH_TRACE);
	///end
}

///if (is_paint || is_sculpt)
function context_select_material(i: i32) {
	if (project_materials.length <= i) return;
	context_set_material(project_materials[i]);
}

function context_set_material(m: SlotMaterialRaw) {
	if (project_materials.indexOf(m) == -1) return;
	context_raw.material = m;
	MakeMaterial.parse_paint_material();
	UIBase.hwnds[tab_area_t.SIDEBAR1].redraws = 2;
	UIHeader.header_handle.redraws = 2;
	UINodes.hwnd.redraws = 2;
	UINodes.group_stack = [];

	let decal: bool = context_raw.tool == workspace_tool_t.DECAL || context_raw.tool == workspace_tool_t.TEXT;
	if (decal) {
		let _next = function() {
			UtilRender.make_decal_preview();
		}
		base_notify_on_next_frame(_next);
	}
}

function context_select_brush(i: i32) {
	if (project_brushes.length <= i) return;
	context_set_brush(project_brushes[i]);
}

function context_set_brush(b: SlotBrushRaw) {
	if (project_brushes.indexOf(b) == -1) return;
	context_raw.brush = b;
	MakeMaterial.parse_brush();
	UIBase.hwnds[tab_area_t.SIDEBAR1].redraws = 2;
	UINodes.hwnd.redraws = 2;
}

function context_select_font(i: i32) {
	if (project_fonts.length <= i) return;
	context_set_font(project_fonts[i]);
}

function context_set_font(f: SlotFontRaw) {
	if (project_fonts.indexOf(f) == -1) return;
	context_raw.font = f;
	UtilRender.make_text_preview();
	UtilRender.make_decal_preview();
	UIBase.hwnds[tab_area_t.STATUS].redraws = 2;
	UIView2D.hwnd.redraws = 2;
}

function context_select_layer(i: i32) {
	if (project_layers.length <= i) return;
	context_set_layer(project_layers[i]);
}

function context_set_layer(l: SlotLayerRaw) {
	if (l == context_raw.layer) return;
	context_raw.layer = l;
	UIHeader.header_handle.redraws = 2;

	let current: image_t = _g2_current;
	if (current != null) g2_end();

	base_set_object_mask();
	MakeMaterial.parse_mesh_material();
	MakeMaterial.parse_paint_material();

	if (current != null) g2_begin(current);

	UIBase.hwnds[tab_area_t.SIDEBAR0].redraws = 2;
	UIView2D.hwnd.redraws = 2;
}
///end

function context_select_tool(i: i32) {
	context_raw.tool = i;
	MakeMaterial.parse_paint_material();
	MakeMaterial.parse_mesh_material();
	context_raw.ddirty = 3;
	let _viewport_mode: viewport_mode_t = context_raw.viewport_mode;
	context_raw.viewport_mode = -1 as viewport_mode_t;
	context_set_viewport_mode(_viewport_mode);

	///if (is_paint || is_sculpt)
	context_init_tool();
	UIHeader.header_handle.redraws = 2;
	UIToolbar.toolbar_handle.redraws = 2;
	///end
}

///if (is_paint || is_sculpt)
function context_init_tool() {
	let decal: bool = context_raw.tool == workspace_tool_t.DECAL || context_raw.tool == workspace_tool_t.TEXT;
	if (decal) {
		if (context_raw.tool == workspace_tool_t.TEXT) {
			UtilRender.make_text_preview();
		}
		UtilRender.make_decal_preview();
	}

	else if (context_raw.tool == workspace_tool_t.PARTICLE) {
		UtilParticle.init_particle();
		MakeMaterial.parse_particle_material();
	}

	else if (context_raw.tool == workspace_tool_t.BAKE) {
		///if (krom_direct3d12 || krom_vulkan || krom_metal)
		// Bake in lit mode for now
		if (context_raw.viewport_mode == viewport_mode_t.PATH_TRACE) {
			context_raw.viewport_mode = viewport_mode_t.LIT;
		}
		///end
	}

	else if (context_raw.tool == workspace_tool_t.MATERIAL) {
		base_update_fill_layers();
		context_main_object().skip_context = null;
	}

	///if krom_ios
	// No hover on iPad, decals are painted by pen release
	config_raw.brush_live = decal;
	///end
}
///end

function context_select_paint_object(o: mesh_object_t) {
	///if (is_paint || is_sculpt)
	UIHeader.header_handle.redraws = 2;
	for (let p of project_paint_objects) p.skip_context = "paint";
	context_raw.paint_object = o;

	let mask: i32 = SlotLayer.get_object_mask(context_raw.layer);
	if (context_layer_filter_used()) mask = context_raw.layer_filter;

	if (context_raw.merged_object == null || mask > 0) {
		context_raw.paint_object.skip_context = "";
	}
	UtilUV.uvmap_cached = false;
	UtilUV.trianglemap_cached = false;
	UtilUV.dilatemap_cached = false;
	///end

	///if is_lab
	context_raw.paint_object = o;
	///end
}

function context_main_object(): mesh_object_t {
	///if (is_paint || is_sculpt)
	for (let po of project_paint_objects) if (po.base.children.length > 0) return po;
	return project_paint_objects[0];
	///end

	///if is_lab
	return project_paint_objects[0];
	///end
}

function context_layer_filter_used(): bool {
	///if (is_paint || is_sculpt)
	return context_raw.layer_filter > 0 && context_raw.layer_filter <= project_paint_objects.length;
	///end

	///if is_lab
	return true;
	///end
}

function context_object_mask_used(): bool {
	///if (is_paint || is_sculpt)
	return SlotLayer.get_object_mask(context_raw.layer) > 0 && SlotLayer.get_object_mask(context_raw.layer) <= project_paint_objects.length;
	///end

	///if is_lab
	return false;
	///end
}

function context_in_viewport(): bool {
	return context_raw.paint_vec.x < 1 && context_raw.paint_vec.x > 0 &&
			context_raw.paint_vec.y < 1 && context_raw.paint_vec.y > 0;
}

function context_in_paint_area(): bool {
	///if (is_paint || is_sculpt)
	let right: i32 = app_w();
	if (UIView2D.show) right += UIView2D.ww;
	return mouse_view_x() > 0 && mouse_view_x() < right &&
			mouse_view_y() > 0 && mouse_view_y() < app_h();
	///end

	///if is_lab
	return context_in_viewport();
	///end
}

function context_in_layers(): bool {
	return zui_get_hovered_tab_name() == tr("Layers");
}

function context_in_materials(): bool {
	return zui_get_hovered_tab_name() == tr("Materials");
}

///if (is_paint || is_sculpt)
function context_in_2d_view(type: view_2d_type_t = view_2d_type_t.LAYER): bool {
	return UIView2D.show && UIView2D.type == type &&
			mouse_x > UIView2D.wx && mouse_x < UIView2D.wx + UIView2D.ww &&
			mouse_y > UIView2D.wy && mouse_y < UIView2D.wy + UIView2D.wh;
}
///end

function context_in_nodes(): bool {
	return UINodes.show &&
			mouse_x > UINodes.wx && mouse_x < UINodes.wx + UINodes.ww &&
			mouse_y > UINodes.wy && mouse_y < UINodes.wy + UINodes.wh;
}

function context_in_swatches(): bool {
	return zui_get_hovered_tab_name() == tr("Swatches");
}

function context_in_browser(): bool {
	return zui_get_hovered_tab_name() == tr("Browser");
}

function context_get_area_type(): area_type_t {
	if (context_in_viewport()) return area_type_t.VIEWPORT;
	if (context_in_nodes()) return area_type_t.NODES;
	if (context_in_browser()) return area_type_t.BROWSER;
	///if (is_paint || is_sculpt)
	if (context_in_2d_view()) return area_type_t.VIEW2D;
	if (context_in_layers()) return area_type_t.LAYERS;
	if (context_in_materials()) return area_type_t.MATERIALS;
	///end
	return -1 as area_type_t;
}

function context_set_viewport_mode(mode: viewport_mode_t) {
	if (mode == context_raw.viewport_mode) return;

	context_raw.viewport_mode = mode;
	if (context_use_deferred()) {
		render_path_commands = RenderPathDeferred.commands;
	}
	else {
		render_path_commands = RenderPathForward.commands;
	}
	let _workspace: i32 = UIHeader.worktab.position;
	UIHeader.worktab.position = 0;
	MakeMaterial.parse_mesh_material();
	UIHeader.worktab.position = _workspace;
}

function context_load_envmap() {
	if (!context_raw.envmap_loaded) {
		// TODO: Unable to share texture for both radiance and envmap - reload image
		context_raw.envmap_loaded = true;
		data_cached_images.delete("World_radiance.k");
	}
	world_data_load_envmap(scene_world);
	if (context_raw.saved_envmap == null) context_raw.saved_envmap = scene_world._.envmap;
}

function context_update_envmap() {
	if (context_raw.show_envmap) {
		scene_world._.envmap = context_raw.show_envmap_blur ? scene_world._.radiance_mipmaps[0] : context_raw.saved_envmap;
	}
	else {
		scene_world._.envmap = context_raw.empty_envmap;
	}
}

function context_set_viewport_shader(viewportShader: (ns: NodeShaderRaw)=>string) {
	context_raw.viewport_shader = viewportShader;
	context_set_render_path();
}

function context_set_render_path() {
	if (context_raw.render_mode == render_mode_t.FORWARD || context_raw.viewport_shader != null) {
		render_path_commands = RenderPathForward.commands;
	}
	else {
		render_path_commands = RenderPathDeferred.commands;
	}
	app_notify_on_init(function() {
		MakeMaterial.parse_mesh_material();
	});
}

function context_enable_import_plugin(file: string): bool {
	// Return plugin name suitable for importing the specified file
	if (BoxPreferences.files_plugin == null) {
		BoxPreferences.fetch_plugins();
	}
	let ext: string = file.substr(file.lastIndexOf(".") + 1);
	for (let f of BoxPreferences.files_plugin) {
		if (f.startsWith("import_") && f.indexOf(ext) >= 0) {
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

///if is_lab
function context_run_brush(from: i32) {
	let left: f32 = 0.0;
	let right: f32 = 1.0;

	// First time init
	if (context_raw.last_paint_x < 0 || context_raw.last_paint_y < 0) {
		context_raw.last_paint_vec_x = context_raw.paint_vec.x;
		context_raw.last_paint_vec_y = context_raw.paint_vec.y;
	}

	let nodes: zui_nodes_t = UINodes.get_nodes();
	let canvas: zui_node_canvas_t = UINodes.get_canvas(true);
	let inpaint: bool = nodes.nodes_selected_id.length > 0 && zui_get_node(canvas.nodes, nodes.nodes_selected_id[0]).type == "InpaintNode";

	// Paint bounds
	if (inpaint &&
		context_raw.paint_vec.x > left &&
		context_raw.paint_vec.x < right &&
		context_raw.paint_vec.y > 0 &&
		context_raw.paint_vec.y < 1 &&
		!base_is_dragging &&
		!base_is_resizing &&
		!base_is_scrolling() &&
		!base_is_combo_selected()) {

		let down: bool = mouse_down() || pen_down();

		// Prevent painting the same spot
		let same_spot: bool = context_raw.paint_vec.x == context_raw.last_paint_x && context_raw.paint_vec.y == context_raw.last_paint_y;
		if (down && same_spot) {
			context_raw.painted++;
		}
		else {
			context_raw.painted = 0;
		}
		context_raw.last_paint_x = context_raw.paint_vec.x;
		context_raw.last_paint_y = context_raw.paint_vec.y;

		if (context_raw.painted == 0) {
			context_parse_brush_inputs();
		}

		if (context_raw.painted <= 1) {
			context_raw.pdirty = 1;
			context_raw.rdirty = 2;
		}
	}
}

function context_parse_brush_inputs() {
	if (!context_raw.registered) {
		context_raw.registered = true;
		app_notify_on_update(context_update);
	}

	context_raw.paint_vec = context_raw.coords;
}

function context_update() {
	let paint_x: f32 = mouse_view_x() / app_w();
	let paint_y: f32 = mouse_view_y() / app_h();
	if (mouse_started()) {
		context_raw.start_x = mouse_view_x() / app_w();
		context_raw.start_y = mouse_view_y() / app_h();
	}

	if (pen_down()) {
		paint_x = pen_view_x() / app_w();
		paint_y = pen_view_y() / app_h();
	}
	if (pen_started()) {
		context_raw.start_x = pen_view_x() / app_w();
		context_raw.start_y = pen_view_y() / app_h();
	}

	if (operator_shortcut(config_keymap.brush_ruler + "+" + config_keymap.action_paint, shortcut_type_t.DOWN)) {
		if (context_raw.lock_x) paint_x = context_raw.start_x;
		if (context_raw.lock_y) paint_y = context_raw.start_y;
	}

	context_raw.coords.x = paint_x;
	context_raw.coords.y = paint_y;

	if (context_raw.lock_begin) {
		let dx: i32 = Math.abs(context_raw.lock_start_x - mouse_view_x());
		let dy: i32 = Math.abs(context_raw.lock_start_y - mouse_view_y());
		if (dx > 1 || dy > 1) {
			context_raw.lock_begin = false;
			dx > dy ? context_raw.lock_y = true : context_raw.lock_x = true;
		}
	}

	if (keyboard_started(config_keymap.brush_ruler)) {
		context_raw.lock_start_x = mouse_view_x();
		context_raw.lock_start_y = mouse_view_y();
		context_raw.lock_begin = true;
	}
	else if (keyboard_released(config_keymap.brush_ruler)) {
		context_raw.lock_x = context_raw.lock_y = context_raw.lock_begin = false;
	}

	context_parse_brush_inputs();
}
///end
