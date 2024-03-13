
let ui_header_default_h: i32 = 28;
let ui_header_h: i32 = ui_header_default_h;
let ui_header_handle: zui_handle_t = zui_handle_create({ layout: zui_layout_t.HORIZONTAL });
let ui_header_worktab: zui_handle_t = zui_handle_create();

function ui_header_init() {

}

function ui_header_render_ui() {
	let ui: zui_t = ui_base_ui;
	if (config_raw.touch_ui) {
		ui_header_h = ui_header_default_h + 6;
	}
	else {
		ui_header_h = ui_header_default_h;
	}
	ui_header_h = math_floor(ui_header_h * zui_SCALE(ui));

	if (config_raw.layout[layout_size_t.HEADER] == 0) return;

	let nodesw: i32 = (ui_nodes_show || ui_view2d_show) ? config_raw.layout[layout_size_t.NODES_W] : 0;
	///if is_lab
	let ww: i32 = sys_width() - nodesw;
	///else
	let ww: i32 = sys_width() - ui_toolbar_w - config_raw.layout[layout_size_t.SIDEBAR_W] - nodesw;
	///end

	if (zui_window(ui_header_handle, app_x(), ui_header_h, ww, ui_header_h)) {
		ui._y += 2;
		ui_header_draw_tool_properties(ui);
	}
}

///if is_paint

function ui_header_draw_tool_properties(ui: zui_t) {
	if (context_raw.tool == workspace_tool_t.COLORID) {
		zui_text(tr("Picked Color"));
		if (context_raw.colorid_picked) {
			zui_image(render_path_render_targets.get("texpaint_colorid")._image, 0xffffffff, 64);
		}
		ui.enabled = context_raw.colorid_picked;
		if (zui_button(tr("Clear"))) {
			context_raw.colorid_picked = false;
			ui_toolbar_handle.redraws = 1;
		}
		ui.enabled = true;
		zui_text(tr("Color ID Map"));
		if (project_asset_names.length > 0) {
			let cid: i32 = zui_combo(context_raw.colorid_handle, base_enum_texts("TEX_IMAGE"), tr("Color ID"));
			if (context_raw.colorid_handle.changed) {
				context_raw.ddirty = 2;
				context_raw.colorid_picked = false;
				ui_toolbar_handle.redraws = 1;
			}
			zui_image(project_get_image(project_assets[cid]));
			if (ui.is_hovered) zui_tooltip_image(project_get_image(project_assets[cid]), 256);
		}
		if (zui_button(tr("Import"))) {
			ui_files_show(path_texture_formats.join(","), false, true, function (path: string) {
				import_asset_run(path, -1.0, -1.0, true, false);

				context_raw.colorid_handle.position = project_asset_names.length - 1;
				for (let a of project_assets) {
					// Already imported
					if (a.file == path) context_raw.colorid_handle.position = project_assets.indexOf(a);
				}
				context_raw.ddirty = 2;
				context_raw.colorid_picked = false;
				ui_toolbar_handle.redraws = 1;
				ui_base_hwnds[2].redraws = 2;
			});
		}
		ui.enabled = context_raw.colorid_picked;
		if (zui_button(tr("To Mask"))) {
			if (SlotLayer.slot_layer_is_mask(context_raw.layer)) context_set_layer(context_raw.layer.parent);
			let m: SlotLayerRaw = base_new_mask(false, context_raw.layer);
			let _next = function () {
				if (base_pipe_merge == null) base_make_pipe();
				if (const_data_screen_aligned_vb == null) const_data_create_screen_aligned_data();
				g4_begin(m.texpaint);
				g4_set_pipeline(base_pipe_colorid_to_mask);
				g4_set_tex(base_texpaint_colorid,render_path_render_targets.get("texpaint_colorid")._image);
				g4_set_tex(base_tex_colorid, project_get_image(project_assets[context_raw.colorid_handle.position]));
				g4_set_vertex_buffer(const_data_screen_aligned_vb);
				g4_set_index_buffer(const_data_screen_aligned_ib);
				g4_draw();
				g4_end();
				context_raw.colorid_picked = false;
				ui_toolbar_handle.redraws = 1;
				ui_header_handle.redraws = 1;
				context_raw.layer_preview_dirty = true;
				base_update_fill_layers();
			}
			base_notify_on_next_frame(_next);
			history_new_white_mask();
		}
		ui.enabled = true;
	}
	else if (context_raw.tool == workspace_tool_t.PICKER || context_raw.tool == workspace_tool_t.MATERIAL) {
		let base_r_picked: f32 = math_round(color_get_rb(context_raw.picked_color.base) / 255 * 10) / 10;
		let base_g_picked: f32 = math_round(color_get_gb(context_raw.picked_color.base) / 255 * 10) / 10;
		let base_b_picked: f32 = math_round(color_get_bb(context_raw.picked_color.base) / 255 * 10) / 10;
		let normal_r_picked: f32 = math_round(color_get_rb(context_raw.picked_color.normal) / 255 * 10) / 10;
		let normal_g_picked: f32 = math_round(color_get_gb(context_raw.picked_color.normal) / 255 * 10) / 10;
		let normal_b_picked: f32 = math_round(color_get_bb(context_raw.picked_color.normal) / 255 * 10) / 10;
		let occlusion_picked: f32 = math_round(context_raw.picked_color.occlusion * 100) / 100;
		let roughness_picked: f32 = math_round(context_raw.picked_color.roughness * 100) / 100;
		let metallic_picked: f32 = math_round(context_raw.picked_color.metallic * 100) / 100;
		let height_picked: f32 = math_round(context_raw.picked_color.height * 100) / 100;
		let opacity_picked: f32 = math_round(context_raw.picked_color.opacity * 100) / 100;

		let h: zui_handle_t = zui_handle("uiheader_0");
		let color: color_t = 0xffffffff;
		color = color_set_rb(color, base_r_picked * 255);
		color = color_set_gb(color, base_g_picked * 255);
		color = color_set_bb(color, base_b_picked * 255);
		h.color = color;
		let state: zui_state_t = zui_text("", 0, h.color);
		if (state == zui_state_t.STARTED) {
			let uix: i32 = ui._x;
			let uiy: i32 = ui._y;
			base_drag_off_x = -(mouse_x - uix - ui._window_x - 3);
			base_drag_off_y = -(mouse_y - uiy - ui._window_y + 1);
			base_drag_swatch = project_clone_swatch(context_raw.picked_color);
		}
		if (ui.is_hovered) zui_tooltip(tr("Drag and drop picked color to swatches, materials, layers or to the node editor"));
		if (ui.is_hovered && ui.input_released) {
			ui_menu_draw(function (ui: zui_t) {
				zui_fill(0, 0, ui._w / zui_SCALE(ui), ui.t.ELEMENT_H * 9, ui.t.SEPARATOR_COL);
				ui.changed = false;
				zui_color_wheel(h, false, null, 10 * ui.t.ELEMENT_H * zui_SCALE(ui), false);
				if (ui.changed) ui_menu_keep_open = true;
			}, 10);
		}
		if (zui_button(tr("Add Swatch"))) {
			let new_swatch: swatch_color_t = project_clone_swatch(context_raw.picked_color);
			context_set_swatch(new_swatch);
			project_raw.swatches.push(new_swatch);
			ui_base_hwnds[2].redraws = 1;
		}
		if (ui.is_hovered) zui_tooltip(tr("Add picked color to swatches"));

		zui_text(tr("Base") + ` (${base_r_picked},${base_g_picked},${base_b_picked})`);
		zui_text(tr("Normal") + ` (${normal_r_picked},${normal_g_picked},${normal_b_picked})`);
		zui_text(tr("Occlusion") + ` (${occlusion_picked})`);
		zui_text(tr("Roughness") + ` (${roughness_picked})`);
		zui_text(tr("Metallic") + ` (${metallic_picked})`);
		zui_text(tr("Height") + ` (${height_picked})`);
		zui_text(tr("Opacity") + ` (${opacity_picked})`);
		context_raw.picker_select_material = zui_check(zui_handle("uiheader_1", { selected: context_raw.picker_select_material }), tr("Select Material"));
		zui_combo(context_raw.picker_mask_handle, [tr("None"), tr("Material")], tr("Mask"), true);
		if (context_raw.picker_mask_handle.changed) {
			MakeMaterial.make_material_parse_paint_material();
		}
	}
	else if (context_raw.tool == workspace_tool_t.BAKE) {
		ui.changed = false;

		///if (krom_direct3d12 || krom_vulkan || krom_metal)
		let baking: bool = context_raw.pdirty > 0;
		let rt_bake: bool = context_raw.bake_type == bake_type_t.AO || context_raw.bake_type == bake_type_t.LIGHTMAP || context_raw.bake_type == bake_type_t.BENT_NORMAL || context_raw.bake_type == bake_type_t.THICKNESS;
		if (baking && zui_button(tr("Stop"))) {
			context_raw.pdirty = 0;
			context_raw.rdirty = 2;
		}
		///else
		let baking: bool = false;
		let rt_bake: bool = false;
		///end

		if (!baking && zui_button(tr("Bake"))) {
			context_raw.pdirty = rt_bake ? context_raw.bake_samples : 1;
			context_raw.rdirty = 3;
			base_notify_on_next_frame(function () {
				context_raw.layer_preview_dirty = true;
			});
			ui_base_hwnds[0].redraws = 2;
			history_push_undo = true;
			///if (krom_direct3d12 || krom_vulkan || krom_metal)
			render_path_raytrace_bake_current_sample = 0;
			///end
		}

		let bake_handle: zui_handle_t = zui_handle("uiheader_2", { position: context_raw.bake_type });
		let bakes: string[] = [
			tr("AO"),
			tr("Curvature"),
			tr("Normal"),
			tr("Object Normal"),
			tr("Height"),
			tr("Derivative"),
			tr("Position"),
			tr("TexCoord"),
			tr("Material ID"),
			tr("Object ID"),
			tr("Vertex Color"),
		];
		///if (krom_direct3d12 || krom_vulkan || krom_metal)
		if (krom_raytrace_supported()) {
			bakes.push(tr("Lightmap"));
			bakes.push(tr("Bent Normal"));
			bakes.push(tr("Thickness"));
		}
		else {
			bakes.shift(); // Remove AO
		}
		///end

		context_raw.bake_type = zui_combo(bake_handle, bakes, tr("Bake"));

		///if (krom_direct3d12 || krom_vulkan || krom_metal)
		if (!krom_raytrace_supported()) {
			context_raw.bake_type += 1; // Offset for removed AO
		}
		///end

		///if (krom_direct3d12 || krom_vulkan || krom_metal)
		if (rt_bake) {
			let samples_handle: zui_handle_t = zui_handle("uiheader_3", { value: context_raw.bake_samples });
			context_raw.bake_samples = math_floor(zui_slider(samples_handle, tr("Samples"), 1, 512, true, 1));
		}
		///end

		if (context_raw.bake_type == bake_type_t.NORMAL_OBJECT || context_raw.bake_type == bake_type_t.POSITION || context_raw.bake_type == bake_type_t.BENT_NORMAL) {
			let bake_up_axis_handle: zui_handle_t = zui_handle("uiheader_4", { position: context_raw.bake_up_axis });
			context_raw.bake_up_axis = zui_combo(bake_up_axis_handle, [tr("Z"), tr("Y")], tr("Up Axis"), true);
		}
		if (context_raw.bake_type == bake_type_t.AO || context_raw.bake_type == bake_type_t.CURVATURE) {
			let bake_axis_handle: zui_handle_t = zui_handle("uiheader_5", { position: context_raw.bake_axis });
			context_raw.bake_axis = zui_combo(bake_axis_handle, [tr("XYZ"), tr("X"), tr("Y"), tr("Z"), tr("-X"), tr("-Y"), tr("-Z")], tr("Axis"), true);
		}
		if (context_raw.bake_type == bake_type_t.AO) {
			let strength_handle: zui_handle_t = zui_handle("uiheader_6", { value: context_raw.bake_ao_strength });
			context_raw.bake_ao_strength = zui_slider(strength_handle, tr("Strength"), 0.0, 2.0, true);
			let radius_handle: zui_handle_t = zui_handle("uiheader_7", { value: context_raw.bake_ao_radius });
			context_raw.bake_ao_radius = zui_slider(radius_handle, tr("Radius"), 0.0, 2.0, true);
			let offset_handle: zui_handle_t = zui_handle("uiheader_8", { value: context_raw.bake_ao_offset });
			context_raw.bake_ao_offset = zui_slider(offset_handle, tr("Offset"), 0.0, 2.0, true);
		}
		///if (krom_direct3d12 || krom_vulkan || krom_metal)
		if (rt_bake) {
			let progress: f32 = render_path_raytrace_bake_current_sample / context_raw.bake_samples;
			if (progress > 1.0) progress = 1.0;
			// Progress bar
			g2_set_color(ui.t.SEPARATOR_COL);
			zui_draw_rect(true, ui._x + 1, ui._y, ui._w - 2, zui_ELEMENT_H(ui));
			g2_set_color(ui.t.HIGHLIGHT_COL);
			zui_draw_rect(true, ui._x + 1, ui._y, (ui._w - 2) * progress, zui_ELEMENT_H(ui));
			g2_set_color(0xffffffff);
			zui_text(tr("Samples") + ": " + render_path_raytrace_bake_current_sample);
			zui_text(tr("Rays/pixel" + ": ") + render_path_raytrace_bake_rays_pix);
			zui_text(tr("Rays/second" + ": ") + render_path_raytrace_bake_rays_sec);
		}
		///end
		if (context_raw.bake_type == bake_type_t.CURVATURE) {
			let strength_handle: zui_handle_t = zui_handle("uiheader_9", { value: context_raw.bake_curv_strength });
			context_raw.bake_curv_strength = zui_slider(strength_handle, tr("Strength"), 0.0, 2.0, true);
			let radius_handle: zui_handle_t = zui_handle("uiheader_10", { value: context_raw.bake_curv_radius });
			context_raw.bake_curv_radius = zui_slider(radius_handle, tr("Radius"), 0.0, 2.0, true);
			let offset_handle: zui_handle_t = zui_handle("uiheader_11", { value: context_raw.bake_curv_offset });
			context_raw.bake_curv_offset = zui_slider(offset_handle, tr("Offset"), -2.0, 2.0, true);
			let smooth_handle: zui_handle_t = zui_handle("uiheader_12", { value: context_raw.bake_curv_smooth });
			context_raw.bake_curv_smooth = math_floor(zui_slider(smooth_handle, tr("Smooth"), 0, 5, false, 1));
		}
		if (context_raw.bake_type == bake_type_t.NORMAL || context_raw.bake_type == bake_type_t.HEIGHT || context_raw.bake_type == bake_type_t.DERIVATIVE) {
			let ar: string[] = [];
			for (let p of project_paint_objects) ar.push(p.base.name);
			let poly_handle: zui_handle_t = zui_handle("uiheader_13", { position: context_raw.bake_high_poly });
			context_raw.bake_high_poly = zui_combo(poly_handle, ar, tr("High Poly"));
		}
		if (ui.changed) {
			MakeMaterial.make_material_parse_paint_material();
		}
	}
	else if (context_raw.tool == workspace_tool_t.BRUSH ||
				context_raw.tool == workspace_tool_t.ERASER ||
				context_raw.tool == workspace_tool_t.FILL ||
				context_raw.tool == workspace_tool_t.DECAL ||
				context_raw.tool == workspace_tool_t.TEXT ||
				context_raw.tool == workspace_tool_t.CLONE ||
				context_raw.tool == workspace_tool_t.BLUR ||
				context_raw.tool == workspace_tool_t.SMUDGE ||
				context_raw.tool == workspace_tool_t.PARTICLE) {

		let decal: bool = context_raw.tool == workspace_tool_t.DECAL || context_raw.tool == workspace_tool_t.TEXT;
		let decal_mask: bool = decal && operator_shortcut(config_keymap.decal_mask, shortcut_type_t.DOWN);
		if (context_raw.tool != workspace_tool_t.FILL) {
			if (decal_mask) {
				context_raw.brush_decal_mask_radius = zui_slider(context_raw.brush_decal_mask_radius_handle, tr("Radius"), 0.01, 2.0, true);
				if (ui.is_hovered) zui_tooltip(tr("Hold {brush_radius} and move mouse to the left or press {brush_radius_decrease} to decrease the radius\nHold {brush_radius} and move mouse to the right or press {brush_radius_increase} to increase the radius", new Map([["brush_radius", config_keymap.brush_radius], ["brush_radius_decrease", config_keymap.brush_radius_decrease], ["brush_radius_increase", config_keymap.brush_radius_increase]])));
			}
			else {
				context_raw.brush_radius = zui_slider(context_raw.brush_radius_handle, tr("Radius"), 0.01, 2.0, true);
				if (ui.is_hovered) zui_tooltip(tr("Hold {brush_radius} and move mouse to the left or press {brush_radius_decrease} to decrease the radius\nHold {brush_radius} and move mouse to the right or press {brush_radius_increase} to increase the radius", new Map([["brush_radius", config_keymap.brush_radius], ["brush_radius_decrease", config_keymap.brush_radius_decrease], ["brush_radius_increase", config_keymap.brush_radius_increase]])));
			}
		}

		if (context_raw.tool == workspace_tool_t.DECAL || context_raw.tool == workspace_tool_t.TEXT) {
			context_raw.brush_scale_x = zui_slider(context_raw.brush_scale_x_handle, tr("Scale X"), 0.01, 2.0, true);
		}

		if (context_raw.tool == workspace_tool_t.BRUSH  ||
			context_raw.tool == workspace_tool_t.FILL   ||
			context_raw.tool == workspace_tool_t.DECAL  ||
			context_raw.tool == workspace_tool_t.TEXT) {
			let brush_scale_handle: zui_handle_t = zui_handle("uiheader_14", { value: context_raw.brush_scale });
			context_raw.brush_scale = zui_slider(brush_scale_handle, tr("UV Scale"), 0.01, 5.0, true);
			if (brush_scale_handle.changed) {
				if (context_raw.tool == workspace_tool_t.DECAL || context_raw.tool == workspace_tool_t.TEXT) {
					let current: image_t = _g2_current;
					g2_end();
					util_render_make_decal_preview();
					g2_begin(current);
				}
			}

			context_raw.brush_angle = zui_slider(context_raw.brush_angle_handle, tr("Angle"), 0.0, 360.0, true, 1);
			if (ui.is_hovered) zui_tooltip(tr("Hold {brush_angle} and move mouse to the left to decrease the angle\nHold {brush_angle} and move mouse to the right to increase the angle", new Map([["brush_angle", config_keymap.brush_angle]])));

			if (context_raw.brush_angle_handle.changed) {
				MakeMaterial.make_material_parse_paint_material();
			}
		}

		context_raw.brush_opacity = zui_slider(context_raw.brush_opacity_handle, tr("Opacity"), 0.0, 1.0, true);
		if (ui.is_hovered) zui_tooltip(tr("Hold {brush_opacity} and move mouse to the left to decrease the opacity\nHold {brush_opacity} and move mouse to the right to increase the opacity", new Map([["brush_opacity", config_keymap.brush_opacity]])));

		if (context_raw.tool == workspace_tool_t.BRUSH || context_raw.tool == workspace_tool_t.ERASER || context_raw.tool == workspace_tool_t.CLONE || decal_mask) {
			context_raw.brush_hardness = zui_slider(zui_handle("uiheader_15", { value: context_raw.brush_hardness }), tr("Hardness"), 0.0, 1.0, true);
		}

		if (context_raw.tool != workspace_tool_t.ERASER) {
			let brush_blending_handle: zui_handle_t = zui_handle("uiheader_16", { value: context_raw.brush_blending });
			context_raw.brush_blending = zui_combo(brush_blending_handle, [
				tr("Mix"),
				tr("Darken"),
				tr("Multiply"),
				tr("Burn"),
				tr("Lighten"),
				tr("Screen"),
				tr("Dodge"),
				tr("Add"),
				tr("Overlay"),
				tr("Soft Light"),
				tr("Linear Light"),
				tr("Difference"),
				tr("Subtract"),
				tr("Divide"),
				tr("Hue"),
				tr("Saturation"),
				tr("Color"),
				tr("Value"),
			], tr("Blending"));
			if (brush_blending_handle.changed) {
				MakeMaterial.make_material_parse_paint_material();
			}
		}

		if (context_raw.tool == workspace_tool_t.BRUSH || context_raw.tool == workspace_tool_t.FILL) {
			let paint_handle: zui_handle_t = zui_handle("uiheader_17");
			context_raw.brush_paint = zui_combo(paint_handle, [tr("UV Map"), tr("Triplanar"), tr("Project")], tr("TexCoord"));
			if (paint_handle.changed) {
				MakeMaterial.make_material_parse_paint_material();
			}
		}
		if (context_raw.tool == workspace_tool_t.TEXT) {
			let h: zui_handle_t = zui_handle("uiheader_18");
			h.text = context_raw.text_tool_text;
			let w: i32 = ui._w;
			if (ui.text_selected_handle_ptr == h.ptr || ui.submit_text_handle_ptr == h.ptr) {
				ui._w *= 3;
			}

			context_raw.text_tool_text = zui_text_input(h, "", zui_align_t.LEFT, true, true);
			ui._w = w;

			if (h.changed) {
				let current: image_t = _g2_current;
				g2_end();
				util_render_make_text_preview();
				util_render_make_decal_preview();
				g2_begin(current);
			}
		}

		if (context_raw.tool == workspace_tool_t.FILL) {
			zui_combo(context_raw.fill_type_handle, [tr("Object"), tr("Face"), tr("Angle"), tr("UV Island")], tr("Fill Mode"));
			if (context_raw.fill_type_handle.changed) {
				if (context_raw.fill_type_handle.position == fill_type_t.FACE) {
					let current: image_t = _g2_current;
					g2_end();
					// cacheUVMap();
					util_uv_cache_triangle_map();
					g2_begin(current);
					// wireframeHandle.selected = drawWireframe = true;
				}
				MakeMaterial.make_material_parse_paint_material();
				MakeMaterial.make_material_parse_mesh_material();
			}
		}
		else {
			let _w: i32 = ui._w;
			let sc: f32 = zui_SCALE(ui);
			let touch_header: bool = (config_raw.touch_ui && config_raw.layout[layout_size_t.HEADER] == 1);
			if (touch_header) ui._x -= 4 * sc;
			ui._w = math_floor((touch_header ? 54 : 60) * sc);

			let xray_handle: zui_handle_t = zui_handle("uiheader_19", { selected: context_raw.xray });
			context_raw.xray = zui_check(xray_handle, tr("X-Ray"));
			if (xray_handle.changed) {
				MakeMaterial.make_material_parse_paint_material();
			}

			let sym_x_handle: zui_handle_t = zui_handle("uiheader_20", { selected: false });
			let sym_y_handle: zui_handle_t = zui_handle("uiheader_21", { selected: false });
			let sym_z_handle: zui_handle_t = zui_handle("uiheader_22", { selected: false });

			if (config_raw.layout[layout_size_t.HEADER] == 1) {
				if (config_raw.touch_ui) {
					ui._w = math_floor(19 * sc);
					context_raw.sym_x = zui_check(sym_x_handle, "");
					ui._x -= 4 * sc;
					context_raw.sym_y = zui_check(sym_y_handle, "");
					ui._x -= 4 * sc;
					context_raw.sym_z = zui_check(sym_z_handle, "");
					ui._x -= 4 * sc;
					ui._w = math_floor(40 * sc);
					zui_text(tr("X") + tr("Y") + tr("Z"));
				}
				else {
					ui._w = math_floor(56 * sc);
					zui_text(tr("Symmetry"));
					ui._w = math_floor(25 * sc);
					context_raw.sym_x = zui_check(sym_x_handle, tr("X"));
					context_raw.sym_y = zui_check(sym_y_handle, tr("Y"));
					context_raw.sym_z = zui_check(sym_z_handle, tr("Z"));
				}
				ui._w = _w;
			}
			else {
				// Popup
				ui._w = _w;
				context_raw.sym_x = zui_check(sym_x_handle, tr("Symmetry") + " " + tr("X"));
				context_raw.sym_y = zui_check(sym_y_handle, tr("Symmetry") + " " + tr("Y"));
				context_raw.sym_z = zui_check(sym_z_handle, tr("Symmetry") + " " + tr("Z"));
			}

			if (sym_x_handle.changed || sym_y_handle.changed || sym_z_handle.changed) {
				MakeMaterial.make_material_parse_paint_material();
			}
		}

		///if arm_physics
		if (context_raw.tool == workspace_tool_t.PARTICLE) {
			ui._x += 10 * zui_SCALE(ui);
			let phys_handle: zui_handle_t = zui_handle("uiheader_23", { selected: false });
			context_raw.particle_physics = zui_check(phys_handle, tr("Physics"));
			if (phys_handle.changed) {
				util_particle_init_physics();
				MakeMaterial.make_material_parse_paint_material();
			}
		}
		///end
	}
}

///end

///if is_sculpt
function ui_header_draw_tool_properties(ui: zui_t) {
	if (context_raw.tool == workspace_tool_t.BRUSH) {
		context_raw.brush_radius = zui_slider(context_raw.brush_radius_handle, tr("Radius"), 0.01, 2.0, true);
		if (ui.is_hovered) zui_tooltip(tr("Hold {brush_radius} and move mouse to the left or press {brush_radius_decrease} to decrease the radius\nHold {brush_radius} and move mouse to the right or press {brush_radius_increase} to increase the radius", new Map([["brush_radius", config_keymap.brush_radius], ["brush_radius_decrease", config_keymap.brush_radius_decrease], ["brush_radius_increase", config_keymap.brush_radius_increase]])));
	}
}
///end

///if is_lab
function ui_header_draw_tool_properties(ui: zui_t) {
	if (context_raw.tool == workspace_tool_t.PICKER) {

	}
	else if (context_raw.tool == workspace_tool_t.ERASER ||
				context_raw.tool == workspace_tool_t.CLONE  ||
				context_raw.tool == workspace_tool_t.BLUR   ||
				context_raw.tool == workspace_tool_t.SMUDGE) {

		let nodes: zui_nodes_t = ui_nodes_get_nodes();
		let canvas: zui_node_canvas_t = ui_nodes_get_canvas(true);
		let inpaint: bool = nodes.nodes_selected_id.length > 0 && zui_get_node(canvas.nodes, nodes.nodes_selected_id[0]).type == "InpaintNode";
		if (inpaint) {
			context_raw.brush_radius = zui_slider(context_raw.brush_radius_handle, tr("Radius"), 0.01, 2.0, true);
			if (ui.is_hovered) zui_tooltip(tr("Hold {brush_radius} and move mouse to the left or press {brush_radius_decrease} to decrease the radius\nHold {brush_radius} and move mouse to the right or press {brush_radius_increase} to increase the radius", new Map([["brush_radius", config_keymap.brush_radius], ["brush_radius_decrease", config_keymap.brush_radius_decrease], ["brush_radius_increase", config_keymap.brush_radius_increase]])));
		}
	}
}
///end
