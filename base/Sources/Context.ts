/// <reference path='./ContextFormat.ts'/>

class Context {

	static raw: TContext = new TContext(); //{};

	static use_deferred = (): bool => {
		///if is_paint
		return Context.raw.render_mode != render_mode_t.FORWARD && (Context.raw.viewport_mode == viewport_mode_t.LIT || Context.raw.viewport_mode == viewport_mode_t.PATH_TRACE) && Context.raw.tool != workspace_tool_t.COLORID;
		///end

		///if (is_sculpt || is_lab)
		return Context.raw.render_mode != render_mode_t.FORWARD && (Context.raw.viewport_mode == viewport_mode_t.LIT || Context.raw.viewport_mode == viewport_mode_t.PATH_TRACE);
		///end
	}

	///if (is_paint || is_sculpt)
	static select_material = (i: i32) => {
		if (Project.materials.length <= i) return;
		Context.set_material(Project.materials[i]);
	}

	static set_material = (m: SlotMaterialRaw) => {
		if (Project.materials.indexOf(m) == -1) return;
		Context.raw.material = m;
		MakeMaterial.parse_paint_material();
		UIBase.hwnds[tab_area_t.SIDEBAR1].redraws = 2;
		UIHeader.header_handle.redraws = 2;
		UINodes.hwnd.redraws = 2;
		UINodes.group_stack = [];

		let decal: bool = Context.raw.tool == workspace_tool_t.DECAL || Context.raw.tool == workspace_tool_t.TEXT;
		if (decal) {
			let _next = () => {
				UtilRender.make_decal_preview();
			}
			Base.notify_on_next_frame(_next);
		}
	}

	static select_brush = (i: i32) => {
		if (Project.brushes.length <= i) return;
		Context.set_brush(Project.brushes[i]);
	}

	static set_brush = (b: SlotBrushRaw) => {
		if (Project.brushes.indexOf(b) == -1) return;
		Context.raw.brush = b;
		MakeMaterial.parse_brush();
		UIBase.hwnds[tab_area_t.SIDEBAR1].redraws = 2;
		UINodes.hwnd.redraws = 2;
	}

	static select_font = (i: i32) => {
		if (Project.fonts.length <= i) return;
		Context.set_font(Project.fonts[i]);
	}

	static set_font = (f: SlotFontRaw) => {
		if (Project.fonts.indexOf(f) == -1) return;
		Context.raw.font = f;
		UtilRender.make_text_preview();
		UtilRender.make_decal_preview();
		UIBase.hwnds[tab_area_t.STATUS].redraws = 2;
		UIView2D.hwnd.redraws = 2;
	}

	static select_layer = (i: i32) => {
		if (Project.layers.length <= i) return;
		Context.set_layer(Project.layers[i]);
	}

	static set_layer = (l: SlotLayerRaw) => {
		if (l == Context.raw.layer) return;
		Context.raw.layer = l;
		UIHeader.header_handle.redraws = 2;

		let current: image_t = _g2_current;
		if (current != null) g2_end();

		Base.set_object_mask();
		MakeMaterial.parse_mesh_material();
		MakeMaterial.parse_paint_material();

		if (current != null) g2_begin(current);

		UIBase.hwnds[tab_area_t.SIDEBAR0].redraws = 2;
		UIView2D.hwnd.redraws = 2;
	}
	///end

	static select_tool = (i: i32) => {
		Context.raw.tool = i;
		MakeMaterial.parse_paint_material();
		MakeMaterial.parse_mesh_material();
		Context.raw.ddirty = 3;
		let _viewportMode: viewport_mode_t = Context.raw.viewport_mode;
		Context.raw.viewport_mode = -1 as viewport_mode_t;
		Context.set_viewport_mode(_viewportMode);

		///if (is_paint || is_sculpt)
		Context.init_tool();
		UIHeader.header_handle.redraws = 2;
		UIToolbar.toolbar_handle.redraws = 2;
		///end
	}

	///if (is_paint || is_sculpt)
	static init_tool = () => {
		let decal: bool = Context.raw.tool == workspace_tool_t.DECAL || Context.raw.tool == workspace_tool_t.TEXT;
		if (decal) {
			if (Context.raw.tool == workspace_tool_t.TEXT) {
				UtilRender.make_text_preview();
			}
			UtilRender.make_decal_preview();
		}

		else if (Context.raw.tool == workspace_tool_t.PARTICLE) {
			UtilParticle.init_particle();
			MakeMaterial.parse_particle_material();
		}

		else if (Context.raw.tool == workspace_tool_t.BAKE) {
			///if (krom_direct3d12 || krom_vulkan || krom_metal)
			// Bake in lit mode for now
			if (Context.raw.viewport_mode == viewport_mode_t.PATH_TRACE) {
				Context.raw.viewport_mode = viewport_mode_t.LIT;
			}
			///end
		}

		else if (Context.raw.tool == workspace_tool_t.MATERIAL) {
			Base.update_fill_layers();
			Context.main_object().skip_context = null;
		}

		///if krom_ios
		// No hover on iPad, decals are painted by pen release
		Config.raw.brush_live = decal;
		///end
	}
	///end

	static select_paint_object = (o: mesh_object_t) => {
		///if (is_paint || is_sculpt)
		UIHeader.header_handle.redraws = 2;
		for (let p of Project.paint_objects) p.skip_context = "paint";
		Context.raw.paint_object = o;

		let mask: i32 = SlotLayer.get_object_mask(Context.raw.layer);
		if (Context.layer_filter_used()) mask = Context.raw.layer_filter;

		if (Context.raw.merged_object == null || mask > 0) {
			Context.raw.paint_object.skip_context = "";
		}
		UtilUV.uvmap_cached = false;
		UtilUV.trianglemap_cached = false;
		UtilUV.dilatemap_cached = false;
		///end

		///if is_lab
		Context.raw.paint_object = o;
		///end
	}

	static main_object = (): mesh_object_t => {
		///if (is_paint || is_sculpt)
		for (let po of Project.paint_objects) if (po.base.children.length > 0) return po;
		return Project.paint_objects[0];
		///end

		///if is_lab
		return Project.paint_objects[0];
		///end
	}

	static layer_filter_used = (): bool => {
		///if (is_paint || is_sculpt)
		return Context.raw.layer_filter > 0 && Context.raw.layer_filter <= Project.paint_objects.length;
		///end

		///if is_lab
		return true;
		///end
	}

	static object_mask_used = (): bool => {
		///if (is_paint || is_sculpt)
		return SlotLayer.get_object_mask(Context.raw.layer) > 0 && SlotLayer.get_object_mask(Context.raw.layer) <= Project.paint_objects.length;
		///end

		///if is_lab
		return false;
		///end
	}

	static in_viewport = (): bool => {
		return Context.raw.paint_vec.x < 1 && Context.raw.paint_vec.x > 0 &&
			   Context.raw.paint_vec.y < 1 && Context.raw.paint_vec.y > 0;
	}

	static in_paint_area = (): bool => {
		///if (is_paint || is_sculpt)
		let right: i32 = app_w();
		if (UIView2D.show) right += UIView2D.ww;
		return mouse_view_x() > 0 && mouse_view_x() < right &&
			   mouse_view_y() > 0 && mouse_view_y() < app_h();
		///end

		///if is_lab
		return Context.in_viewport();
		///end
	}

	static in_layers = (): bool => {
		return zui_get_hovered_tab_name() == tr("Layers");
	}

	static in_materials = (): bool => {
		return zui_get_hovered_tab_name() == tr("Materials");
	}

	///if (is_paint || is_sculpt)
	static in_2d_view = (type: view_2d_type_t = view_2d_type_t.LAYER): bool => {
		return UIView2D.show && UIView2D.type == type &&
			   mouse_x > UIView2D.wx && mouse_x < UIView2D.wx + UIView2D.ww &&
			   mouse_y > UIView2D.wy && mouse_y < UIView2D.wy + UIView2D.wh;
	}
	///end

	static in_nodes = (): bool => {
		return UINodes.show &&
			   mouse_x > UINodes.wx && mouse_x < UINodes.wx + UINodes.ww &&
			   mouse_y > UINodes.wy && mouse_y < UINodes.wy + UINodes.wh;
	}

	static in_swatches = (): bool => {
		return zui_get_hovered_tab_name() == tr("Swatches");
	}

	static in_browser = (): bool => {
		return zui_get_hovered_tab_name() == tr("Browser");
	}

	static get_area_type = (): area_type_t => {
		if (Context.in_viewport()) return area_type_t.VIEWPORT;
		if (Context.in_nodes()) return area_type_t.NODES;
		if (Context.in_browser()) return area_type_t.BROWSER;
		///if (is_paint || is_sculpt)
		if (Context.in_2d_view()) return area_type_t.VIEW2D;
		if (Context.in_layers()) return area_type_t.LAYERS;
		if (Context.in_materials()) return area_type_t.MATERIALS;
		///end
		return -1 as area_type_t;
	}

	static set_viewport_mode = (mode: viewport_mode_t) => {
		if (mode == Context.raw.viewport_mode) return;

		Context.raw.viewport_mode = mode;
		if (Context.use_deferred()) {
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

	static load_envmap = () => {
		if (!Context.raw.envmap_loaded) {
			// TODO: Unable to share texture for both radiance and envmap - reload image
			Context.raw.envmap_loaded = true;
			data_cached_images.delete("World_radiance.k");
		}
		world_data_load_envmap(scene_world, (_) => {});
		if (Context.raw.saved_envmap == null) Context.raw.saved_envmap = scene_world._.envmap;
	}

	static update_envmap = () => {
		if (Context.raw.show_envmap) {
			scene_world._.envmap = Context.raw.show_envmap_blur ? scene_world._.radiance_mipmaps[0] : Context.raw.saved_envmap;
		}
		else {
			scene_world._.envmap = Context.raw.empty_envmap;
		}
	}

	static set_viewport_shader = (viewportShader: (ns: NodeShaderRaw)=>string) => {
		Context.raw.viewport_shader = viewportShader;
		Context.set_render_path();
	}

	static set_render_path = () => {
		if (Context.raw.render_mode == render_mode_t.FORWARD || Context.raw.viewport_shader != null) {
			render_path_commands = RenderPathForward.commands;
		}
		else {
			render_path_commands = RenderPathDeferred.commands;
		}
		app_notify_on_init(() => {
			MakeMaterial.parse_mesh_material();
		});
	}

	static enable_import_plugin = (file: string): bool => {
		// Return plugin name suitable for importing the specified file
		if (BoxPreferences.files_plugin == null) {
			BoxPreferences.fetch_plugins();
		}
		let ext: string = file.substr(file.lastIndexOf(".") + 1);
		for (let f of BoxPreferences.files_plugin) {
			if (f.startsWith("import_") && f.indexOf(ext) >= 0) {
				Config.enable_plugin(f);
				Console.info(f + " " + tr("plugin enabled"));
				return true;
			}
		}
		return false;
	}

	static set_swatch = (s: swatch_color_t) => {
		Context.raw.swatch = s;
	}

	///if is_lab
	static run_brush = (from: i32) => {
		let left: f32 = 0.0;
		let right: f32 = 1.0;

		// First time init
		if (Context.raw.last_paint_x < 0 || Context.raw.last_paint_y < 0) {
			Context.raw.last_paint_vec_x = Context.raw.paint_vec.x;
			Context.raw.last_paint_vec_y = Context.raw.paint_vec.y;
		}

		let nodes: zui_nodes_t = UINodes.get_nodes();
		let canvas: zui_node_canvas_t = UINodes.get_canvas(true);
		let inpaint: bool = nodes.nodesSelectedId.length > 0 && zui_get_node(canvas.nodes, nodes.nodesSelectedId[0]).type == "InpaintNode";

		// Paint bounds
		if (inpaint &&
			Context.raw.paint_vec.x > left &&
			Context.raw.paint_vec.x < right &&
			Context.raw.paint_vec.y > 0 &&
			Context.raw.paint_vec.y < 1 &&
			!Base.is_dragging &&
			!Base.is_resizing &&
			!Base.is_scrolling() &&
			!Base.is_combo_selected()) {

			let down: bool = mouse_down() || pen_down();

			// Prevent painting the same spot
			let sameSpot: bool = Context.raw.paint_vec.x == Context.raw.last_paint_x && Context.raw.paint_vec.y == Context.raw.last_paint_y;
			if (down && sameSpot) {
				Context.raw.painted++;
			}
			else {
				Context.raw.painted = 0;
			}
			Context.raw.last_paint_x = Context.raw.paint_vec.x;
			Context.raw.last_paint_y = Context.raw.paint_vec.y;

			if (Context.raw.painted == 0) {
				Context.parse_brush_inputs();
			}

			if (Context.raw.painted <= 1) {
				Context.raw.pdirty = 1;
				Context.raw.rdirty = 2;
			}
		}
	}

	static parse_brush_inputs = () => {
		if (!Context.raw.registered) {
			Context.raw.registered = true;
			app_notify_on_update(Context.update);
		}

		Context.raw.paint_vec = Context.raw.coords;
	}

	static update = () => {
		let paintX: f32 = mouse_view_x() / app_w();
		let paintY: f32 = mouse_view_y() / app_h();
		if (mouse_started()) {
			Context.raw.start_x = mouse_view_x() / app_w();
			Context.raw.start_y = mouse_view_y() / app_h();
		}

		if (pen_down()) {
			paintX = pen_view_x() / app_w();
			paintY = pen_view_y() / app_h();
		}
		if (pen_started()) {
			Context.raw.start_x = pen_view_x() / app_w();
			Context.raw.start_y = pen_view_y() / app_h();
		}

		if (Operator.shortcut(Config.keymap.brush_ruler + "+" + Config.keymap.action_paint, ShortcutType.ShortcutDown)) {
			if (Context.raw.lock_x) paintX = Context.raw.start_x;
			if (Context.raw.lock_y) paintY = Context.raw.start_y;
		}

		Context.raw.coords.x = paintX;
		Context.raw.coords.y = paintY;

		if (Context.raw.lock_begin) {
			let dx: i32 = Math.abs(Context.raw.lock_start_x - mouse_view_x());
			let dy: i32 = Math.abs(Context.raw.lock_start_y - mouse_view_y());
			if (dx > 1 || dy > 1) {
				Context.raw.lock_begin = false;
				dx > dy ? Context.raw.lock_y = true : Context.raw.lock_x = true;
			}
		}

		if (keyboard_started(Config.keymap.brush_ruler)) {
			Context.raw.lock_start_x = mouse_view_x();
			Context.raw.lock_start_y = mouse_view_y();
			Context.raw.lock_begin = true;
		}
		else if (keyboard_released(Config.keymap.brush_ruler)) {
			Context.raw.lock_x = Context.raw.lock_y = Context.raw.lock_begin = false;
		}

		Context.parse_brush_inputs();
	}
	///end
}
