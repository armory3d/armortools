
let _tab_meshes_draw_i: i32;

function tab_meshes_draw(htab: ui_handle_t) {
	let statush: i32 = config_raw.layout[layout_size_t.STATUS_H];
	if (ui_tab(htab, tr("Meshes")) && statush > ui_statusbar_default_h * UI_SCALE()) {

		ui_begin_sticky();

		if (config_raw.touch_ui) {
			let row: f32[] = [1 / 4, 1 / 4];
			ui_row(row);
		}
		else {
			let row: f32[] = [1 / 14, 1 / 14];
			ui_row(row);
		}

		if (ui_button(tr("Import"))) {
			ui_menu_draw(function () {
				if (ui_menu_button(tr("Replace Existing"), map_get(config_keymap, "file_import_assets"))) {
					project_import_mesh(true);
				}
				if (ui_menu_button(tr("Append File"))) {
					project_append_mesh();
				}

				// project_fetch_default_meshes();
				// if (ui_menu_button(tr("Append Shape"))) {
				// 	ui_menu_draw(function () {
				// 		for (let i: i32 = 0; i < project_mesh_list.length; ++i) {
				// 			if (ui_menu_button(project_mesh_list[i])) {
				// 				tab_meshes_append_shape(project_mesh_list[i]);
				// 			}
				// 		}
				// 	});
				// }
			});
		}
		if (ui.is_hovered) ui_tooltip(tr("Import mesh file"));


		if (ui_button(tr("Edit"))) {

			ui_menu_draw(function () {

				if (ui_menu_button(tr("Flip Normals"))) {
					util_mesh_flip_normals();
					context_raw.ddirty = 2;
				}

				if (ui_menu_button(tr("Calculate Normals"))) {
					ui_menu_draw(function () {
						if (ui_menu_button(tr("Smooth"))) {
							util_mesh_calc_normals(true);
							context_raw.ddirty = 2;
						}
						if (ui_menu_button(tr("Flat"))) {
							util_mesh_calc_normals(false);
							context_raw.ddirty = 2;
						}
					});
				}

				if (ui_menu_button(tr("Geometry to Origin"))) {
					util_mesh_to_origin();
					context_raw.ddirty = 2;
				}

				if (ui_menu_button(tr("Apply Displacement"))) {
					util_mesh_apply_displacement(project_layers[0].texpaint_pack);

					util_mesh_calc_normals();
					context_raw.ddirty = 2;
				}

				if (ui_menu_button(tr("Rotate"))) {
					ui_menu_draw(function () {
						if (ui_menu_button(tr("Rotate X"))) {
							util_mesh_swap_axis(1, 2);
							context_raw.ddirty = 2;
						}

						if (ui_menu_button(tr("Rotate Y"))) {
							util_mesh_swap_axis(2, 0);
							context_raw.ddirty = 2;
						}

						if (ui_menu_button(tr("Rotate Z"))) {
							util_mesh_swap_axis(0, 1);
							context_raw.ddirty = 2;
						}
					});
				}

				if (ui_menu_button(tr("UV Unwrap"))) {
					let f: string = "uv_unwrap.js";
					if (array_index_of(config_raw.plugins, f) == -1) {
						config_enable_plugin(f);
					}
					plugin_uv_unwrap_button();
				}
			});
		}

		ui_end_sticky();

		for (let i: i32 = 0; i < project_paint_objects.length; ++i) {
			let o: mesh_object_t = project_paint_objects[i];
			let h: ui_handle_t = ui_handle(__ID__);
			h.selected = o.base.visible;
			o.base.visible = ui_check(h, o.base.name);

			if (ui.is_hovered && ui.input_released_r) {
				_tab_meshes_draw_i = i;

				ui_menu_draw(function () {
					let i: i32 = _tab_meshes_draw_i;
					let o: mesh_object_t = project_paint_objects[i];

					if (ui_menu_button(tr("Export"))) {
						context_raw.export_mesh_index = i + 1;
						box_export_show_mesh();
					}
					if (project_paint_objects.length > 1 && ui_menu_button(tr("Delete"))) {
						array_remove(project_paint_objects, o);
						while (o.base.children.length > 0) {
							let child: object_t = o.base.children[0];
							object_set_parent(child, null);
							if (project_paint_objects[0].base != child) {
								object_set_parent(child, project_paint_objects[0].base);
							}
							if (o.base.children.length == 0) {
								project_paint_objects[0].base.transform.scale = vec4_clone(o.base.transform.scale);
								transform_build_matrix(project_paint_objects[0].base.transform);
							}
						}
						data_delete_mesh(o.data._.handle);
						mesh_object_remove(o);
						context_raw.paint_object = context_main_object();
						util_mesh_merge();
						context_raw.ddirty = 2;
					}


					context_raw.selected_object = o.base;
					let h: ui_handle_t = ui_handle(__ID__);
					// h.selected = context_raw.selected_object.visible;
					// context_raw.selected_object.visible = ui_check(h, "Visible");
					// if (h.changed) {
						// Rebuild full vb for path-tracing
						// util_mesh_merge();
					// }

					let t: transform_t = context_raw.selected_object.transform;
					let rot: vec4_t = quat_get_euler(t.rot);
					rot = vec4_mult(rot, 180 / 3.141592);
					let f: f32 = 0.0;
					let changed: bool = false;

					ui_row4();
					ui_text("Loc");

					h = ui_handle(__ID__);
					h.text = f32_to_string(t.loc.x);
					f = parse_float(ui_text_input(h, "X"));
					if (h.changed) {
						changed = true;
						t.loc.x = f;
					}

					h = ui_handle(__ID__);
					h.text = f32_to_string(t.loc.y);
					f = parse_float(ui_text_input(h, "Y"));
					if (h.changed) {
						changed = true;
						t.loc.y = f;
					}

					h = ui_handle(__ID__);
					h.text = f32_to_string(t.loc.z);
					f = parse_float(ui_text_input(h, "Z"));
					if (h.changed) {
						changed = true;
						t.loc.z = f;
					}

					ui_row4();
					ui_text("Rot");

					h = ui_handle(__ID__);
					h.text = f32_to_string(rot.x);
					f = parse_float(ui_text_input(h, "X"));
					if (h.changed) {
						changed = true;
						rot.x = f;
					}

					h = ui_handle(__ID__);
					h.text = f32_to_string(rot.y);
					f = parse_float(ui_text_input(h, "Y"));
					if (h.changed) {
						changed = true;
						rot.y = f;
					}

					h = ui_handle(__ID__);
					h.text = f32_to_string(rot.z);
					f = parse_float(ui_text_input(h, "Z"));
					if (h.changed) {
						changed = true;
						rot.z = f;
					}

					ui_row4();
					ui_text("Scale");

					h = ui_handle(__ID__);
					h.text = f32_to_string(t.scale.x);
					f = parse_float(ui_text_input(h, "X"));
					if (h.changed) {
						changed = true;
						t.scale.x = f;
					}

					h = ui_handle(__ID__);
					h.text = f32_to_string(t.scale.y);
					f = parse_float(ui_text_input(h, "Y"));
					if (h.changed) {
						changed = true;
						t.scale.y = f;
					}

					h = ui_handle(__ID__);
					h.text = f32_to_string(t.scale.z);
					f = parse_float(ui_text_input(h, "Z"));
					if (h.changed) {
						changed = true;
						t.scale.z = f;
					}

					ui_row4();
					ui_text("Dim");

					h = ui_handle(__ID__);
					h.text = f32_to_string(t.dim.x);
					f = parse_float(ui_text_input(h, "X"));
					if (h.changed) {
						changed = true;
						t.dim.x = f;
					}

					h = ui_handle(__ID__);
					h.text = f32_to_string(t.dim.y);
					f = parse_float(ui_text_input(h, "Y"));
					if (h.changed) {
						changed = true;
						t.dim.y = f;
					}

					h = ui_handle(__ID__);
					h.text = f32_to_string(t.dim.z);
					f = parse_float(ui_text_input(h, "Z"));
					if (h.changed) {
						changed = true;
						t.dim.z = f;
					}

					if (changed) {
						rot = vec4_mult(rot, 3.141592 / 180);
						context_raw.selected_object.transform.rot = quat_from_euler(rot.x, rot.y, rot.z);
						transform_build_matrix(context_raw.selected_object.transform);
						transform_compute_dim(context_raw.selected_object.transform);

						let pb: physics_body_t = map_get(physics_body_object_map, context_raw.selected_object.uid);
						if (pb != null) {
							physics_body_sync_transform(pb);
						}
					}

					let pb: physics_body_t = map_get(physics_body_object_map, context_raw.selected_object.uid);
					let hshape: ui_handle_t = ui_handle(__ID__);
					let shape_combo: string[] = [
						tr("None"),
						tr("Box"),
						tr("Sphere"),
						tr("Convex Hull"),
						tr("Terrain"),
						tr("Mesh"),
					];
					hshape.position = pb != null ? pb.shape + 1 : 0;
					ui_combo(hshape, shape_combo, tr("Shape"), true);

					let hdynamic: ui_handle_t = ui_handle(__ID__);
					hdynamic.selected = pb != null ? pb.mass > 0 : false;
					ui_check(hdynamic, "Dynamic");

					if (hshape.changed || hdynamic.changed) {
						sim_remove_body(context_raw.selected_object.uid);
						if (hshape.position > 0) {
							sim_add_body(context_raw.selected_object, hshape.position - 1, hdynamic.selected ? 1.0 : 0.0);
						}
					}

					ui_text("Script", ui_align_t.LEFT, ui.ops.theme.SEPARATOR_COL);

					let script: string = map_get(sim_object_script_map, context_raw.selected_object);
					if (script == null) {
						script = "";
					}

					let hscript: ui_handle_t = ui_handle(__ID__);
					hscript.text = script;

					let _font: draw_font_t = ui.ops.font;
					let _font_size: i32 = ui.font_size;
					let fmono: draw_font_t = data_get_font("font_mono.ttf");
					ui_set_font(ui, fmono);
					ui.font_size = math_floor(15 * UI_SCALE());
					ui_text_area_coloring = tab_scripts_get_text_coloring();
					ui_text_area(hscript);
					ui_text_area_coloring = null;
					ui_set_font(ui, _font);
					ui.font_size = _font_size;

					script = hscript.text;
					map_set(sim_object_script_map, context_raw.selected_object, script);


				});
			}
			if (h.changed) {
				let visibles: mesh_object_t[] = [];
				for (let i: i32 = 0; i < project_paint_objects.length; ++i) {
					let p: mesh_object_t = project_paint_objects[i];
					if (p.base.visible) {
						array_push(visibles, p);
					}
				}
				util_mesh_merge(visibles);
				context_raw.ddirty = 2;
			}
		}
	}
}

function tab_meshes_append_shape(mesh_name: string) {
	let blob: buffer_t = iron_load_blob(data_path() + "meshes/" + mesh_name + ".arm");
	let raw: scene_t = armpack_decode(blob);
	util_mesh_pack_uvs(raw.mesh_datas[0].vertex_arrays[2].values);
	let md: mesh_data_t = mesh_data_create(raw.mesh_datas[0]);
	md._.handle = md.name;
	let mo: mesh_object_t = scene_add_mesh_object(md, project_paint_objects[0].material);
	mo.base.name = md.name;
	let o: obj_t = {};
	o._ = { _gc: raw };
	mo.base.raw = o;
	map_set(data_cached_meshes, md._.handle, md);
	array_push(project_paint_objects, mo);
	// tab_scene_import_mesh_done();
	// sys_notify_on_next_frame(function(mo: mesh_object_t) {
	// 	tab_scene_select_object(mo);
	// }, mo);
}

////

let tab_scene_line_counter: i32 = 0;
let _tab_scene_paint_object_length: i32 = 1;

function tab_scene_select_object(mo: mesh_object_t) {
	if (mo == null) {
		return;
	}

	context_raw.selected_object = mo.base;

	if (mo.base.ext_type != "mesh_object_t") {
		return;
	}

	context_raw.paint_object = mo;
	if (context_raw.merged_object != null) {
		context_raw.merged_object.base.visible = false;
	}
	context_select_paint_object(mo);
}

function tab_scene_sort() {
	let scene: object_t = _scene_root.children[0];
	array_sort(scene.children, function (pa: any_ptr, pb: any_ptr): i32 {
		let a: object_t = DEREFERENCE(pa);
		let b: object_t = DEREFERENCE(pb);
		return strcmp(a.name, b.name);
	});
}

function tab_scene_import_mesh_done() {
	let count: i32 = project_paint_objects.length - _tab_scene_paint_object_length;
	_tab_scene_paint_object_length = project_paint_objects.length;

	for (let i: i32 = 0; i < count; ++i) {
		let mo: mesh_object_t = project_paint_objects[project_paint_objects.length - 1 - i];
		object_set_parent(mo.base, null);
		tab_scene_select_object(mo);
	}

	sys_notify_on_next_frame(function() {
		util_mesh_merge();
		tab_scene_select_object(context_raw.selected_object.ext);
		tab_scene_sort();
	});
}

function tab_scene_draw_list(list_handle: ui_handle_t, current_object: object_t) {
	if (char_at(current_object.name, 0) == ".") {
		return; // Hidden
	}

	let b: bool = false;

	// Highlight every other line
	if (tab_scene_line_counter % 2 == 0) {
		draw_set_color(ui.ops.theme.SEPARATOR_COL);
		draw_filled_rect(0, ui._y, ui._window_w, UI_ELEMENT_H());
		draw_set_color(0xffffffff);
	}

	// Highlight selected line
	if (current_object == context_raw.selected_object) {
		draw_set_color(0xff205d9c);
		draw_filled_rect(0, ui._y, ui._window_w, UI_ELEMENT_H());
		draw_set_color(0xffffffff);
	}

	if (current_object.children.length > 0) {
		let row: f32[] = [1 / 13, 12 / 13];
		ui_row(row);
		let h: ui_handle_t = ui_nest(list_handle, tab_scene_line_counter);
		if (h.init) {
			h.selected = true;
		}
		b = ui_panel(h, "", true, false);
		ui_text(current_object.name);
	}
	else {
		ui._x += 18; // Sign offset

		// Draw line that shows parent relations
		draw_set_color(ui.ops.theme.BUTTON_COL);
		draw_line(ui._x - 10, ui._y + UI_ELEMENT_H() / 2, ui._x, ui._y + UI_ELEMENT_H() / 2);
		draw_set_color(0xffffffff);

		ui_text(current_object.name);
		ui._x -= 18;
	}

	tab_scene_line_counter++;
	// Undo applied offset for row drawing caused by end_element()
	ui._y -= UI_ELEMENT_OFFSET();

	if (ui.is_released) {
		tab_scene_select_object(current_object.ext);
	}

	if (ui.is_hovered && ui.input_released_r) {
		tab_scene_select_object(current_object.ext);

		ui_menu_draw(function () {
			if (ui_menu_button(tr("Duplicate"))) {
				sim_duplicate();
			}
			if (ui_menu_button(tr("Delete"))) {
				sim_delete();
			}
		});
	}

	if (b) {
		let current_y: i32 = ui._y;
		for (let i: i32 = 0; i < current_object.children.length; ++i) {
			let child: object_t = current_object.children[i];
			ui._x += 8;
			tab_scene_draw_list(list_handle, child);
			ui._x -= 8;
		}

		// Draw line that shows parent relations
		draw_set_color(ui.ops.theme.BUTTON_COL);
		draw_line(ui._x + 14, current_y, ui._x + 14, ui._y - UI_ELEMENT_H() / 2);
		draw_set_color(0xffffffff);
	}
}

function tab_scene_draw(htab: ui_handle_t) {
	if (ui_tab(htab, tr("Scene"))) {

		tab_scene_line_counter = 0;

		let scene: object_t = _scene_root.children[0];
		for (let i: i32 = 0; i < scene.children.length; ++i) {
			let c: object_t = scene.children[i];
			tab_scene_draw_list(ui_handle(__ID__), c);
		}

		// Select object with arrow keys
		if (ui.is_key_pressed && ui.key_code == key_code_t.DOWN) {
			let i: i32 = array_index_of(project_paint_objects, context_raw.selected_object.ext);
			if (i < project_paint_objects.length - 1) {
				tab_scene_select_object(project_paint_objects[i + 1]);
			}
		}
		if (ui.is_key_pressed && ui.key_code == key_code_t.UP) {
			let i: i32 = array_index_of(project_paint_objects, context_raw.selected_object.ext);
			if (i > 1) {
				tab_scene_select_object(project_paint_objects[i - 1]);
			}
		}
	}
}
