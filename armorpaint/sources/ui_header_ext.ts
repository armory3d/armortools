
let _ui_header_draw_tool_properties_h: ui_handle_t;

function ui_header_draw_tool_properties(ui: ui_t) {
	if (context_raw.tool == workspace_tool_t.COLORID) {
		ui_text(tr("Picked Color"));
		if (context_raw.colorid_picked) {
			let rt: render_target_t = map_get(render_path_render_targets, "texpaint_colorid");
			_ui_image(rt._image, 0xffffffff, 64);
		}
		ui.enabled = context_raw.colorid_picked;
		if (ui_button(tr("Clear"))) {
			context_raw.colorid_picked = false;
			ui_toolbar_handle.redraws = 1;
		}
		ui.enabled = true;
		ui_text(tr("Color ID Map"));
		if (project_asset_names.length > 0) {
			let cid: i32 = ui_combo(context_raw.colorid_handle, base_enum_texts("TEX_IMAGE"), tr("Color ID"));
			if (context_raw.colorid_handle.changed) {
				context_raw.ddirty = 2;
				context_raw.colorid_picked = false;
				ui_toolbar_handle.redraws = 1;
			}
			_ui_image(project_get_image(project_assets[cid]));
			if (ui.is_hovered) {
				_ui_tooltip_image(project_get_image(project_assets[cid]), 256);
			}
		}
		if (ui_button(tr("Import"))) {
			ui_files_show(string_array_join(path_texture_formats, ","), false, true, function (path: string) {
				import_asset_run(path, -1.0, -1.0, true, false);

				context_raw.colorid_handle.position = project_asset_names.length - 1;
				for (let i: i32 = 0; i < project_assets.length; ++i) {
					let a: asset_t = project_assets[i];
					// Already imported
					if (a.file == path) {
						context_raw.colorid_handle.position = array_index_of(project_assets, a);
					}
				}
				context_raw.ddirty = 2;
				context_raw.colorid_picked = false;
				ui_toolbar_handle.redraws = 1;
				ui_base_hwnds[2].redraws = 2;
			});
		}
		ui.enabled = context_raw.colorid_picked;
		if (ui_button(tr("To Mask"))) {
			if (slot_layer_is_mask(context_raw.layer)) {
				context_set_layer(context_raw.layer.parent);
			}
			let m: slot_layer_t = layers_new_mask(false, context_raw.layer);
			app_notify_on_next_frame(function (m: slot_layer_t) {
				g4_begin(m.texpaint);
				g4_set_pipeline(pipes_colorid_to_mask);
				let rt: render_target_t = map_get(render_path_render_targets, "texpaint_colorid");
				g4_set_tex(pipes_texpaint_colorid, rt._image);
				g4_set_tex(pipes_tex_colorid, project_get_image(project_assets[context_raw.colorid_handle.position]));
				g4_set_vertex_buffer(const_data_screen_aligned_vb);
				g4_set_index_buffer(const_data_screen_aligned_ib);
				g4_draw();
				g4_end();
				context_raw.colorid_picked = false;
				ui_toolbar_handle.redraws = 1;
				ui_header_handle.redraws = 1;
				context_raw.layer_preview_dirty = true;
				layers_update_fill_layers();
			}, m);
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

		let h: ui_handle_t = ui_handle(__ID__);
		let color: color_t = 0xffffffff;
		color = color_set_rb(color, base_r_picked * 255);
		color = color_set_gb(color, base_g_picked * 255);
		color = color_set_bb(color, base_b_picked * 255);
		h.color = color;
		let state: ui_state_t = ui_text("", 0, h.color);
		if (state == ui_state_t.STARTED) {
			let uix: i32 = ui._x;
			let uiy: i32 = ui._y;
			base_drag_off_x = -(mouse_x - uix - ui._window_x - 3);
			base_drag_off_y = -(mouse_y - uiy - ui._window_y + 1);
			base_drag_swatch = project_clone_swatch(context_raw.picked_color);
		}
		if (ui.is_hovered) {
			ui_tooltip(tr("Drag and drop picked color to swatches, materials, layers or to the node editor"));
		}
		if (ui.is_hovered && ui.input_released) {
			_ui_header_draw_tool_properties_h = h;
			ui_menu_draw(function (ui: ui_t) {
				ui_fill(0, 0, ui._w / ui_SCALE(ui), ui.ops.theme.ELEMENT_H * 9, ui.ops.theme.SEPARATOR_COL);
				ui.changed = false;
				ui_color_wheel(_ui_header_draw_tool_properties_h, false, -1, 10 * ui.ops.theme.ELEMENT_H * ui_SCALE(ui), false);
				if (ui.changed) {
					ui_menu_keep_open = true;
				}
			});
		}
		if (ui_button(tr("Add Swatch"))) {
			let new_swatch: swatch_color_t = project_clone_swatch(context_raw.picked_color);
			context_set_swatch(new_swatch);
			array_push(project_raw.swatches, new_swatch);
			ui_base_hwnds[2].redraws = 1;
		}
		if (ui.is_hovered) {
			ui_tooltip(tr("Add picked color to swatches"));
		}

		ui_text(tr("Base") + " (" + base_r_picked + "," + base_g_picked + "," + base_b_picked + ")");
		ui_text(tr("Normal") + " (" + normal_r_picked + "," + normal_g_picked + "," + normal_b_picked + ")");
		ui_text(tr("Occlusion") + " (" + occlusion_picked + ")");
		ui_text(tr("Roughness") + " (" + roughness_picked + ")");
		ui_text(tr("Metallic") + " (" + metallic_picked + ")");
		ui_text(tr("Height") + " (" + height_picked + ")");
		ui_text(tr("Opacity") + " (" + opacity_picked + ")");
		let h_select_mat: ui_handle_t = ui_handle(__ID__);
		if (h_select_mat.init) {
			h_select_mat.selected = context_raw.picker_select_material;
		}
		context_raw.picker_select_material = ui_check(h_select_mat, tr("Select Material"));
		let picker_mask_combo: string[] = [tr("None"), tr("Material")];
		ui_combo(context_raw.picker_mask_handle, picker_mask_combo, tr("Mask"), true);
		if (context_raw.picker_mask_handle.changed) {
			make_material_parse_paint_material();
		}
	}
	else if (context_raw.tool == workspace_tool_t.BAKE) {
		ui.changed = false;

		let baking: bool = context_raw.pdirty > 0;
		let rt_bake: bool = render_path_paint_is_rt_bake();
		if (baking && ui_button(tr("Stop"))) {
			context_raw.pdirty = 0;
			context_raw.rdirty = 2;
		}

		if (!baking && ui_button(tr("Bake"))) {
			context_raw.pdirty = rt_bake ? context_raw.bake_samples : 1;
			context_raw.rdirty = 3;
			app_notify_on_next_frame(function () {
				context_raw.layer_preview_dirty = true;
			});
			ui_base_hwnds[0].redraws = 2;
			history_push_undo = true;
			render_path_raytrace_bake_current_sample = 0;
		}

		let bake_handle: ui_handle_t = ui_handle(__ID__);
		if (bake_handle.init) {
			bake_handle.position = context_raw.bake_type;
		}
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
		if (iron_raytrace_supported()) {
			array_push(bakes, tr("Lightmap"));
			array_push(bakes, tr("Bent Normal"));
			array_push(bakes, tr("Thickness"));
		}
		else {
			array_shift(bakes); // Remove AO
		}

		context_raw.bake_type = ui_combo(bake_handle, bakes, tr("Bake"));

		if (!iron_raytrace_supported()) {
			context_raw.bake_type += 1; // Offset for removed AO
		}

		if (rt_bake) {
			let samples_handle: ui_handle_t = ui_handle(__ID__);
			if (samples_handle.init) {
				samples_handle.value = context_raw.bake_samples;
			}
			context_raw.bake_samples = math_floor(ui_slider(samples_handle, tr("Samples"), 1, 512, true, 1));
		}

		if (context_raw.bake_type == bake_type_t.NORMAL_OBJECT ||
			context_raw.bake_type == bake_type_t.POSITION ||
			context_raw.bake_type == bake_type_t.BENT_NORMAL
		) {
			let bake_up_axis_handle: ui_handle_t = ui_handle(__ID__);
			if (bake_up_axis_handle.init) {
				bake_up_axis_handle.position = context_raw.bake_up_axis;
			}
			let bake_up_axis_combo: string[] = [tr("Z"), tr("Y")];
			context_raw.bake_up_axis = ui_combo(bake_up_axis_handle, bake_up_axis_combo, tr("Up Axis"), true);
		}
		if (context_raw.bake_type == bake_type_t.AO || context_raw.bake_type == bake_type_t.CURVATURE) {
			let bake_axis_handle: ui_handle_t = ui_handle(__ID__);
			if (bake_axis_handle.init) {
				bake_axis_handle.position = context_raw.bake_axis;
			}
			let bake_axis_combo: string[] = [tr("XYZ"), tr("X"), tr("Y"), tr("Z"), tr("-X"), tr("-Y"), tr("-Z")];
			context_raw.bake_axis = ui_combo(bake_axis_handle, bake_axis_combo, tr("Axis"), true);
		}
		if (context_raw.bake_type == bake_type_t.AO) {
			let strength_handle: ui_handle_t = ui_handle(__ID__);
			if (strength_handle.init) {
				strength_handle.value = context_raw.bake_ao_strength;
			}
			context_raw.bake_ao_strength = ui_slider(strength_handle, tr("Strength"), 0.0, 2.0, true);
			let radius_handle: ui_handle_t = ui_handle(__ID__);
			if (radius_handle.init) {
				radius_handle.value = context_raw.bake_ao_radius;
			}
			context_raw.bake_ao_radius = ui_slider(radius_handle, tr("Radius"), 0.0, 2.0, true);
			let offset_handle: ui_handle_t = ui_handle(__ID__);
			if (offset_handle.init) {
				offset_handle.value = context_raw.bake_ao_offset;
			}
			context_raw.bake_ao_offset = ui_slider(offset_handle, tr("Offset"), 0.0, 2.0, true);
		}
		if (rt_bake) {
			let progress: f32 = render_path_raytrace_bake_current_sample / context_raw.bake_samples;
			if (progress > 1.0) progress = 1.0;
			// Progress bar
			g2_set_color(ui.ops.theme.SEPARATOR_COL);
			ui_draw_rect(true, ui._x + 1, ui._y, ui._w - 2, ui_ELEMENT_H(ui));
			g2_set_color(ui.ops.theme.HIGHLIGHT_COL);
			ui_draw_rect(true, ui._x + 1, ui._y, (ui._w - 2) * progress, ui_ELEMENT_H(ui));
			g2_set_color(0xffffffff);
			ui_text(tr("Samples") + ": " + render_path_raytrace_bake_current_sample);
			ui_text(tr("Rays/pixel") + ": " + render_path_raytrace_bake_rays_pix);
			ui_text(tr("Rays/second") + ": " + render_path_raytrace_bake_rays_sec);
		}
		if (context_raw.bake_type == bake_type_t.CURVATURE) {
			let strength_handle: ui_handle_t = ui_handle(__ID__);
			if (strength_handle.init) {
				strength_handle.value = context_raw.bake_curv_strength;
			}
			context_raw.bake_curv_strength = ui_slider(strength_handle, tr("Strength"), 0.0, 2.0, true);
			let radius_handle: ui_handle_t = ui_handle(__ID__);
			if (radius_handle.init) {
				radius_handle.value = context_raw.bake_curv_radius;
			}
			context_raw.bake_curv_radius = ui_slider(radius_handle, tr("Radius"), 0.0, 2.0, true);
			let offset_handle: ui_handle_t = ui_handle(__ID__);
			if (offset_handle.init) {
				offset_handle.value = context_raw.bake_curv_offset;
			}
			context_raw.bake_curv_offset = ui_slider(offset_handle, tr("Offset"), -2.0, 2.0, true);
			let smooth_handle: ui_handle_t = ui_handle(__ID__);
			if (smooth_handle.init) {
				smooth_handle.value = context_raw.bake_curv_smooth;
			}
			context_raw.bake_curv_smooth = math_floor(ui_slider(smooth_handle, tr("Smooth"), 0, 5, false, 1));
		}
		if (context_raw.bake_type == bake_type_t.NORMAL || context_raw.bake_type == bake_type_t.HEIGHT || context_raw.bake_type == bake_type_t.DERIVATIVE) {
			let ar: string[] = [];
			for (let i: i32 = 0; i < project_paint_objects.length; ++i) {
				let p: mesh_object_t = project_paint_objects[i];
				array_push(ar, p.base.name);
			}
			let poly_handle: ui_handle_t = ui_handle(__ID__);
			if (poly_handle.init) {
				poly_handle.position = context_raw.bake_high_poly;
			}
			context_raw.bake_high_poly = ui_combo(poly_handle, ar, tr("High Poly"));
		}
		if (ui.changed) {
			make_material_parse_paint_material();
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

		let decal: bool = context_is_decal();
		let decal_mask: bool = context_is_decal_mask();
		if (context_raw.tool != workspace_tool_t.FILL) {
			if (decal_mask) {
				context_raw.brush_decal_mask_radius = ui_slider(context_raw.brush_decal_mask_radius_handle, tr("Radius"), 0.01, 2.0, true);
				if (ui.is_hovered) {
					let vars: map_t<string, string> = map_create();
					map_set(vars, "brush_radius", map_get(config_keymap, "brush_radius"));
					map_set(vars, "brush_radius_decrease", map_get(config_keymap, "brush_radius_decrease"));
					map_set(vars, "brush_radius_increase", map_get(config_keymap, "brush_radius_increase"));
					ui_tooltip(tr("Hold {brush_radius} and move mouse to the left or press {brush_radius_decrease} to decrease the radius\nHold {brush_radius} and move mouse to the right or press {brush_radius_increase} to increase the radius", vars));
				}
			}
			else {
				context_raw.brush_radius = ui_slider(context_raw.brush_radius_handle, tr("Radius"), 0.01, 2.0, true);
				if (ui.is_hovered) {
					let vars: map_t<string, string> = map_create();
					map_set(vars, "brush_radius", map_get(config_keymap, "brush_radius"));
					map_set(vars, "brush_radius_decrease", map_get(config_keymap, "brush_radius_decrease"));
					map_set(vars, "brush_radius_increase", map_get(config_keymap, "brush_radius_increase"));
					ui_tooltip(tr("Hold {brush_radius} and move mouse to the left or press {brush_radius_decrease} to decrease the radius\nHold {brush_radius} and move mouse to the right or press {brush_radius_increase} to increase the radius", vars));
				}
			}
		}

		if (context_raw.tool == workspace_tool_t.DECAL || context_raw.tool == workspace_tool_t.TEXT) {
			context_raw.brush_scale_x = ui_slider(context_raw.brush_scale_x_handle, tr("Scale X"), 0.01, 2.0, true);
		}

		if (context_raw.tool == workspace_tool_t.BRUSH  ||
			context_raw.tool == workspace_tool_t.FILL   ||
			context_raw.tool == workspace_tool_t.DECAL  ||
			context_raw.tool == workspace_tool_t.TEXT) {
			let brush_scale_handle: ui_handle_t = ui_handle(__ID__);
			if (brush_scale_handle.init) {
				brush_scale_handle.value = context_raw.brush_scale;
			}
			context_raw.brush_scale = ui_slider(brush_scale_handle, tr("UV Scale"), 0.01, 5.0, true);
			if (brush_scale_handle.changed) {
				if (context_raw.tool == workspace_tool_t.DECAL || context_raw.tool == workspace_tool_t.TEXT) {
					let current: image_t = _g2_current;
					g2_end();
					util_render_make_decal_preview();
					g2_begin(current);
				}
			}

			context_raw.brush_angle = ui_slider(context_raw.brush_angle_handle, tr("Angle"), 0.0, 360.0, true, 1);
			if (ui.is_hovered) {
				let vars: map_t<string, string> = map_create();
				map_set(vars, "brush_angle", map_get(config_keymap, "brush_angle"));
				ui_tooltip(tr("Hold {brush_angle} and move mouse to the left to decrease the angle\nHold {brush_angle} and move mouse to the right to increase the angle", vars));
			}

			if (context_raw.brush_angle_handle.changed) {
				make_material_parse_paint_material();
			}
		}

		context_raw.brush_opacity = ui_slider(context_raw.brush_opacity_handle, tr("Opacity"), 0.0, 1.0, true);
		if (ui.is_hovered) {
			let vars: map_t<string, string> = map_create();
			map_set(vars, "brush_opacity", map_get(config_keymap, "brush_opacity"));
			ui_tooltip(tr("Hold {brush_opacity} and move mouse to the left to decrease the opacity\nHold {brush_opacity} and move mouse to the right to increase the opacity", vars));
		}

		if (context_raw.tool == workspace_tool_t.BRUSH || context_raw.tool == workspace_tool_t.ERASER || context_raw.tool == workspace_tool_t.CLONE || decal_mask) {
			let h: ui_handle_t = ui_handle(__ID__);
			if (h.init) {
				h.value = context_raw.brush_hardness;
			}
			context_raw.brush_hardness = ui_slider(h, tr("Hardness"), 0.0, 1.0, true);
		}

		if (context_raw.tool != workspace_tool_t.ERASER) {
			let brush_blending_handle: ui_handle_t = ui_handle(__ID__);
			if (brush_blending_handle.init) {
				brush_blending_handle.value = context_raw.brush_blending;
			}
			let brush_blending_combo: string[] = [
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
			];
			context_raw.brush_blending = ui_combo(brush_blending_handle, brush_blending_combo, tr("Blending"));
			if (brush_blending_handle.changed) {
				make_material_parse_paint_material();
			}
		}

		if (context_raw.tool == workspace_tool_t.BRUSH || context_raw.tool == workspace_tool_t.FILL) {
			let paint_handle: ui_handle_t = ui_handle(__ID__);
			let texcoord_combo: string[] = [tr("UV Map"), tr("Triplanar"), tr("Project")];
			context_raw.brush_paint = ui_combo(paint_handle, texcoord_combo, tr("TexCoord"));
			if (paint_handle.changed) {
				make_material_parse_paint_material();
			}
		}
		if (context_raw.tool == workspace_tool_t.TEXT) {
			let h: ui_handle_t = ui_handle(__ID__);
			h.text = context_raw.text_tool_text;
			let w: i32 = ui._w;
			if (ui.text_selected_handle == h || ui.submit_text_handle == h) {
				ui._w *= 3;
			}

			context_raw.text_tool_text = ui_text_input(h, "", ui_align_t.LEFT, true, true);
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
			let fill_mode_combo: string[] = [tr("Object"), tr("Face"), tr("Angle"), tr("UV Island")];
			ui_combo(context_raw.fill_type_handle, fill_mode_combo, tr("Fill Mode"));
			if (context_raw.fill_type_handle.changed) {
				if (context_raw.fill_type_handle.position == fill_type_t.FACE) {
					let current: image_t = _g2_current;
					g2_end();
					// cache_uv_map();
					util_uv_cache_triangle_map();
					g2_begin(current);
					// wireframe_handle.selected = draw_wireframe = true;
				}
				make_material_parse_paint_material();
				make_material_parse_mesh_material();
			}
		}
		else {
			let _w: i32 = ui._w;
			let sc: f32 = ui_SCALE(ui);
			let touch_header: bool = (config_raw.touch_ui && config_raw.layout[layout_size_t.HEADER] == 1);
			if (touch_header) {
				ui._x -= 4 * sc;
			}
			ui._w = math_floor((touch_header ? 54 : 60) * sc);

			let xray_handle: ui_handle_t = ui_handle(__ID__);
			if (xray_handle.init) {
				xray_handle.selected = context_raw.xray;
			}
			context_raw.xray = ui_check(xray_handle, tr("X-Ray"));
			if (xray_handle.changed) {
				make_material_parse_paint_material();
			}

			let sym_x_handle: ui_handle_t = ui_handle(__ID__);
			if (sym_x_handle.init) {
				sym_x_handle.selected = false;
			}
			let sym_y_handle: ui_handle_t = ui_handle(__ID__);
			if (sym_y_handle.init) {
				sym_y_handle.selected = false;
			}
			let sym_z_handle: ui_handle_t = ui_handle(__ID__);
			if (sym_z_handle.init) {
				sym_z_handle.selected = false;
			}

			if (config_raw.layout[layout_size_t.HEADER] == 1) {
				if (config_raw.touch_ui) {
					ui._w = math_floor(19 * sc);
					context_raw.sym_x = ui_check(sym_x_handle, "");
					ui._x -= 4 * sc;
					context_raw.sym_y = ui_check(sym_y_handle, "");
					ui._x -= 4 * sc;
					context_raw.sym_z = ui_check(sym_z_handle, "");
					ui._x -= 4 * sc;
					ui._w = math_floor(40 * sc);
					let x: string = tr("X");
					let y: string = tr("Y");
					let z: string = tr("Z");
					ui_text(x + y + z);
				}
				else {
					ui._w = math_floor(56 * sc);
					ui_text(tr("Symmetry"));
					ui._w = math_floor(25 * sc);
					context_raw.sym_x = ui_check(sym_x_handle, tr("X"));
					context_raw.sym_y = ui_check(sym_y_handle, tr("Y"));
					context_raw.sym_z = ui_check(sym_z_handle, tr("Z"));
				}
				ui._w = _w;
			}
			else {
				// Popup
				ui._w = _w;
				context_raw.sym_x = ui_check(sym_x_handle, tr("Symmetry") + " " + tr("X"));
				context_raw.sym_y = ui_check(sym_y_handle, tr("Symmetry") + " " + tr("Y"));
				context_raw.sym_z = ui_check(sym_z_handle, tr("Symmetry") + " " + tr("Z"));
			}

			if (sym_x_handle.changed || sym_y_handle.changed || sym_z_handle.changed) {
				make_material_parse_paint_material();
			}
		}
	}
}
