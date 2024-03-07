
class TabMeshes {

	static draw = (htab: zui_handle_t) => {
		let ui: zui_t = UIBase.ui;
		let statush: i32 = Config.raw.layout[layout_size_t.STATUS_H];
		if (zui_tab(htab, tr("Meshes")) && statush > UIStatus.default_status_h * zui_SCALE(ui)) {

			zui_begin_sticky();

			///if (is_paint || is_sculpt)
			if (Config.raw.touch_ui) {
				zui_row([1 / 6, 1 / 6, 1 / 6, 1 / 6, 1 / 6, 1 / 6]);
			}
			else {
				zui_row([1 / 14, 1 / 9, 1 / 9, 1 / 9, 1 / 9, 1 / 14]);
			}
			///end

			///if is_lab
			if (Config.raw.touch_ui) {
				zui_row([1 / 7, 1 / 7, 1 / 7, 1 / 7, 1 / 7, 1 / 7, 1 / 7]);
			}
			else {
				zui_row([1 / 14, 1 / 9, 1 / 9, 1 / 9, 1 / 9, 1 / 9, 1 / 14]);
			}
			///end

			if (zui_button(tr("Import"))) {
				UIMenu.draw((ui: zui_t) => {
					if (UIMenu.menu_button(ui, tr("Replace Existing"), `${Config.keymap.file_import_assets}`)) {
						Project.import_mesh(true);
					}
					if (UIMenu.menu_button(ui, tr("Append"))) {
						Project.import_mesh(false);
					}
				}, 2);
			}
			if (ui.is_hovered) zui_tooltip(tr("Import mesh file"));

			///if is_lab
			if (zui_button(tr("Set Default"))) {
				UIMenu.draw((ui: zui_t) => {
					if (UIMenu.menu_button(ui, tr("Cube"))) TabMeshes.set_default_mesh(".Cube");
					if (UIMenu.menu_button(ui, tr("Plane"))) TabMeshes.set_default_mesh(".Plane");
					if (UIMenu.menu_button(ui, tr("Sphere"))) TabMeshes.set_default_mesh(".Sphere");
					if (UIMenu.menu_button(ui, tr("Cylinder"))) TabMeshes.set_default_mesh(".Cylinder");
				}, 4);
			}
			///end

			if (zui_button(tr("Flip Normals"))) {
				UtilMesh.flip_normals();
				Context.raw.ddirty = 2;
			}

			if (zui_button(tr("Calculate Normals"))) {
				UIMenu.draw((ui: zui_t) => {
					if (UIMenu.menu_button(ui, tr("Smooth"))) { UtilMesh.calc_normals(true); Context.raw.ddirty = 2; }
					if (UIMenu.menu_button(ui, tr("Flat"))) { UtilMesh.calc_normals(false); Context.raw.ddirty = 2; }
				}, 2);
			}

			if (zui_button(tr("Geometry to Origin"))) {
				UtilMesh.to_origin();
				Context.raw.ddirty = 2;
			}

			if (zui_button(tr("Apply Displacement"))) {
				///if is_paint
				UtilMesh.apply_displacement(Project.layers[0].texpaint_pack);
				///end
				///if is_lab
				let displace_strength: f32 = Config.raw.displace_strength > 0 ? Config.raw.displace_strength : 1.0;
				let uv_scale: f32 = scene_meshes[0].data.scale_tex * Context.raw.brush_scale;
				UtilMesh.apply_displacement(BrushOutputNode.inst.texpaint_pack, 0.05 * displace_strength, uv_scale);
				///end

				UtilMesh.calc_normals();
				Context.raw.ddirty = 2;
			}

			if (zui_button(tr("Rotate"))) {
				UIMenu.draw((ui: zui_t) => {
					if (UIMenu.menu_button(ui, tr("Rotate X"))) {
						UtilMesh.swap_axis(1, 2);
						Context.raw.ddirty = 2;
					}

					if (UIMenu.menu_button(ui, tr("Rotate Y"))) {
						UtilMesh.swap_axis(2, 0);
						Context.raw.ddirty = 2;
					}

					if (UIMenu.menu_button(ui, tr("Rotate Z"))) {
						UtilMesh.swap_axis(0, 1);
						Context.raw.ddirty = 2;
					}
				}, 3);
			}

			zui_end_sticky();

			for (let i: i32 = 0; i < Project.paint_objects.length; ++i) {
				let o: mesh_object_t = Project.paint_objects[i];
				let h: zui_handle_t = zui_handle("tabmeshes_0");
				h.selected = o.base.visible;
				o.base.visible = zui_check(h, o.base.name);
				if (ui.is_hovered && ui.input_released_r) {
					UIMenu.draw((ui: zui_t) => {
						if (UIMenu.menu_button(ui, tr("Export"))) {
							Context.raw.export_mesh_index = i + 1;
							BoxExport.show_mesh();
						}
						if (Project.paint_objects.length > 1 && UIMenu.menu_button(ui, tr("Delete"))) {
							array_remove(Project.paint_objects, o);
							while (o.base.children.length > 0) {
								let child: object_t = o.base.children[0];
								object_set_parent(child, null);
								if (Project.paint_objects[0].base != child) {
									object_set_parent(child, Project.paint_objects[0].base);
								}
								if (o.base.children.length == 0) {
									vec4_set_from(Project.paint_objects[0].base.transform.scale, o.base.transform.scale);
									transform_build_matrix(Project.paint_objects[0].base.transform);
								}
							}
							data_delete_mesh(o.data._.handle);
							mesh_object_remove(o);
							Context.raw.paint_object = Context.main_object();
							UtilMesh.merge_mesh();
							Context.raw.ddirty = 2;
						}
					}, Project.paint_objects.length > 1 ? 2 : 1);
				}
				if (h.changed) {
					let visibles: mesh_object_t[] = [];
					for (let p of Project.paint_objects) if (p.base.visible) visibles.push(p);
					UtilMesh.merge_mesh(visibles);
					Context.raw.ddirty = 2;
				}
			}
		}
	}

	///if is_lab
	static set_default_mesh = (name: string) => {
		let mo: mesh_object_t = null;
		if (name == ".Plane" || name == ".Sphere") {
			let res: i32 = Config.raw.rp_supersample > 1.0 ? 2048 : 1024;
			let mesh: any = name == ".Plane" ? Geom.make_plane(1, 1, res, res) : Geom.make_uv_sphere(1.0, res, Math.floor(res / 2), false, 2.0);
			let raw: any = {
				name: "Tessellated",
				vertex_arrays: [
					{ values: mesh.posa, attrib: "pos", data: "short4norm" },
					{ values: mesh.nora, attrib: "nor", data: "short2norm" },
					{ values: mesh.texa, attrib: "tex", data: "short2norm" }
				],
				index_arrays: [
					{ values: mesh.inda, material: 0 }
				],
				scale_pos: mesh.scalePos,
				scale_tex: mesh.scaleTex
			};
			let md: mesh_data_t = mesh_data_create(raw);
			mo = mesh_object_create(md, Context.raw.paint_object.materials);
			array_remove(scene_meshes, mo);
			mo.base.name = "Tessellated";
		}
		else {
			mo = scene_get_child(name).ext;
		}

		mo.base.visible = true;
		Context.raw.ddirty = 2;
		Context.raw.paint_object = mo;
		Project.paint_objects[0] = mo;
		if (UIHeader.worktab.position == space_type_t.SPACE3D) {
			scene_meshes = [mo];
		}

		///if (krom_direct3d12 || krom_vulkan || krom_metal)
		RenderPathRaytrace.ready = false;
		///end
	}
	///end
}
