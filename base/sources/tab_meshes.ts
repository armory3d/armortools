
let _tab_meshes_draw_i: i32;

function tab_meshes_draw(htab: ui_handle_t) {
	let ui: ui_t = ui_base_ui;
	let statush: i32 = config_raw.layout[layout_size_t.STATUS_H];
	if (ui_tab(htab, tr("Meshes")) && statush > ui_status_default_status_h * ui_SCALE(ui)) {

		ui_begin_sticky();

		///if (is_paint || is_sculpt)
		if (config_raw.touch_ui) {
			ui_row6();
		}
		else {
			let row: f32[] = [1 / 14, 1 / 9, 1 / 9, 1 / 9, 1 / 9, 1 / 14];
			ui_row(row);
		}
		///end

		///if is_lab
		if (config_raw.touch_ui) {
			ui_row7();
		}
		else {
			let row: f32[] = [1 / 14, 1 / 9, 1 / 9, 1 / 9, 1 / 9, 1 / 9, 1 / 14];
			ui_row(row);
		}
		///end

		if (ui_button(tr("Import"))) {
			ui_menu_draw(function (ui: ui_t) {
				if (ui_menu_button(ui, tr("Replace Existing"), map_get(config_keymap, "file_import_assets"))) {
					project_import_mesh(true);
				}
				if (ui_menu_button(ui, tr("Append"))) {
					project_append_mesh();
				}
			});
		}
		if (ui.is_hovered) ui_tooltip(tr("Import mesh file"));

		///if is_lab
		if (ui_button(tr("Set Default"))) {
			ui_menu_draw(function (ui: ui_t) {
				if (ui_menu_button(ui, tr("Cube"))) {
					tab_meshes_set_default_mesh(".Cube");
				}
				if (ui_menu_button(ui, tr("Plane"))) {
					tab_meshes_set_default_mesh(".Plane");
				}
				if (ui_menu_button(ui, tr("Sphere"))) {
					tab_meshes_set_default_mesh(".Sphere");
				}
				if (ui_menu_button(ui, tr("Cylinder"))) {
					tab_meshes_set_default_mesh(".Cylinder");
				}
			});
		}
		///end

		if (ui_button(tr("Flip Normals"))) {
			util_mesh_flip_normals();
			context_raw.ddirty = 2;
		}

		if (ui_button(tr("Calculate Normals"))) {
			ui_menu_draw(function (ui: ui_t) {
				if (ui_menu_button(ui, tr("Smooth"))) {
					util_mesh_calc_normals(true);
					context_raw.ddirty = 2;
				}
				if (ui_menu_button(ui, tr("Flat"))) {
					util_mesh_calc_normals(false);
					context_raw.ddirty = 2;
				}
			});
		}

		if (ui_button(tr("Geometry to Origin"))) {
			util_mesh_to_origin();
			context_raw.ddirty = 2;
		}

		if (ui_button(tr("Apply Displacement"))) {
			///if is_paint
			util_mesh_apply_displacement(project_layers[0].texpaint_pack);
			///end
			///if is_lab
			let displace_strength: f32 = config_raw.displace_strength > 0 ? config_raw.displace_strength : 1.0;
			let uv_scale: f32 = scene_meshes[0].data.scale_tex * context_raw.brush_scale;
			util_mesh_apply_displacement(context_raw.brush_output_node_inst.texpaint_pack, 0.05 * displace_strength, uv_scale);
			///end

			util_mesh_calc_normals();
			context_raw.ddirty = 2;
		}

		if (ui_button(tr("Rotate"))) {
			ui_menu_draw(function (ui: ui_t) {
				if (ui_menu_button(ui, tr("Rotate X"))) {
					util_mesh_swap_axis(1, 2);
					context_raw.ddirty = 2;
				}

				if (ui_menu_button(ui, tr("Rotate Y"))) {
					util_mesh_swap_axis(2, 0);
					context_raw.ddirty = 2;
				}

				if (ui_menu_button(ui, tr("Rotate Z"))) {
					util_mesh_swap_axis(0, 1);
					context_raw.ddirty = 2;
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

				ui_menu_draw(function (ui: ui_t) {
					let i: i32 = _tab_meshes_draw_i;
					let o: mesh_object_t = project_paint_objects[i];

					if (ui_menu_button(ui, tr("Export"))) {
						context_raw.export_mesh_index = i + 1;
						box_export_show_mesh();
					}
					if (project_paint_objects.length > 1 && ui_menu_button(ui, tr("Delete"))) {
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

function tab_meshes_set_default_mesh(name: string) {
	let mo: mesh_object_t = null;
	if (name == ".Plane" || name == ".Sphere") {
		let res: i32 = config_raw.rp_supersample > 1.0 ? 2048 : 1024;
		let mesh: raw_mesh_t = name == ".Plane" ? geom_make_plane(1, 1, res, res) : geom_make_uv_sphere(1.0, res, math_floor(res / 2), false, 2.0);
		let raw: mesh_data_t = {
			name: "Tessellated",
			vertex_arrays: [
				{ values: mesh.posa, attrib: "pos", data: "short4norm" },
				{ values: mesh.nora, attrib: "nor", data: "short2norm" },
				{ values: mesh.texa, attrib: "tex", data: "short2norm" }
			],
			index_arrays: [
				{ values: mesh.inda, material: 0 }
			],
			scale_pos: mesh.scale_pos,
			scale_tex: mesh.scale_tex
		};
		let md: mesh_data_t = mesh_data_create(raw);
		mo = mesh_object_create(md, context_raw.paint_object.materials);
		array_remove(scene_meshes, mo);
		mo.base.name = "Tessellated";
	}
	else {
		mo = scene_get_child(name).ext;
	}

	mo.base.visible = true;
	context_raw.ddirty = 2;
	context_raw.paint_object = mo;
	project_paint_objects[0] = mo;
	if (ui_header_worktab.position == space_type_t.SPACE3D) {
		scene_meshes = [mo];
	}

	render_path_raytrace_ready = false;
}
