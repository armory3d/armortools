
class TabMeshes {

	static draw = (htab: zui_handle_t) => {
		let ui = UIBase.ui;
		let statush = Config.raw.layout[LayoutSize.LayoutStatusH];
		if (zui_tab(htab, tr("Meshes")) && statush > UIStatus.defaultStatusH * zui_SCALE(ui)) {

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
					if (UIMenu.menuButton(ui, tr("Replace Existing"), `${Config.keymap.file_import_assets}`)) {
						Project.importMesh(true);
					}
					if (UIMenu.menuButton(ui, tr("Append"))) {
						Project.importMesh(false);
					}
				}, 2);
			}
			if (ui.is_hovered) zui_tooltip(tr("Import mesh file"));

			///if is_lab
			if (zui_button(tr("Set Default"))) {
				UIMenu.draw((ui: zui_t) => {
					if (UIMenu.menuButton(ui, tr("Cube"))) TabMeshes.setDefaultMesh(".Cube");
					if (UIMenu.menuButton(ui, tr("Plane"))) TabMeshes.setDefaultMesh(".Plane");
					if (UIMenu.menuButton(ui, tr("Sphere"))) TabMeshes.setDefaultMesh(".Sphere");
					if (UIMenu.menuButton(ui, tr("Cylinder"))) TabMeshes.setDefaultMesh(".Cylinder");
				}, 4);
			}
			///end

			if (zui_button(tr("Flip Normals"))) {
				UtilMesh.flipNormals();
				Context.raw.ddirty = 2;
			}

			if (zui_button(tr("Calculate Normals"))) {
				UIMenu.draw((ui: zui_t) => {
					if (UIMenu.menuButton(ui, tr("Smooth"))) { UtilMesh.calcNormals(true); Context.raw.ddirty = 2; }
					if (UIMenu.menuButton(ui, tr("Flat"))) { UtilMesh.calcNormals(false); Context.raw.ddirty = 2; }
				}, 2);
			}

			if (zui_button(tr("Geometry to Origin"))) {
				UtilMesh.toOrigin();
				Context.raw.ddirty = 2;
			}

			if (zui_button(tr("Apply Displacement"))) {
				///if is_paint
				UtilMesh.applyDisplacement(Project.layers[0].texpaint_pack);
				///end
				///if is_lab
				let displace_strength = Config.raw.displace_strength > 0 ? Config.raw.displace_strength : 1.0;
				let uv_scale = scene_meshes[0].data.scale_tex * Context.raw.brushScale;
				UtilMesh.applyDisplacement(BrushOutputNode.inst.texpaint_pack, 0.05 * displace_strength, uv_scale);
				///end

				UtilMesh.calcNormals();
				Context.raw.ddirty = 2;
			}

			if (zui_button(tr("Rotate"))) {
				UIMenu.draw((ui: zui_t) => {
					if (UIMenu.menuButton(ui, tr("Rotate X"))) {
						UtilMesh.swapAxis(1, 2);
						Context.raw.ddirty = 2;
					}

					if (UIMenu.menuButton(ui, tr("Rotate Y"))) {
						UtilMesh.swapAxis(2, 0);
						Context.raw.ddirty = 2;
					}

					if (UIMenu.menuButton(ui, tr("Rotate Z"))) {
						UtilMesh.swapAxis(0, 1);
						Context.raw.ddirty = 2;
					}
				}, 3);
			}

			zui_end_sticky();

			for (let i = 0; i < Project.paintObjects.length; ++i) {
				let o = Project.paintObjects[i];
				let h = zui_handle("tabmeshes_0");
				h.selected = o.base.visible;
				o.base.visible = zui_check(h, o.base.name);
				if (ui.is_hovered && ui.input_released_r) {
					UIMenu.draw((ui: zui_t) => {
						if (UIMenu.menuButton(ui, tr("Export"))) {
							Context.raw.exportMeshIndex = i + 1;
							BoxExport.showMesh();
						}
						if (Project.paintObjects.length > 1 && UIMenu.menuButton(ui, tr("Delete"))) {
							array_remove(Project.paintObjects, o);
							while (o.base.children.length > 0) {
								let child = o.base.children[0];
								object_set_parent(child, null);
								if (Project.paintObjects[0].base != child) {
									object_set_parent(child, Project.paintObjects[0].base);
								}
								if (o.base.children.length == 0) {
									vec4_set_from(Project.paintObjects[0].base.transform.scale, o.base.transform.scale);
									transform_build_matrix(Project.paintObjects[0].base.transform);
								}
							}
							data_delete_mesh(o.data._handle);
							mesh_object_remove(o);
							Context.raw.paintObject = Context.mainObject();
							UtilMesh.mergeMesh();
							Context.raw.ddirty = 2;
						}
					}, Project.paintObjects.length > 1 ? 2 : 1);
				}
				if (h.changed) {
					let visibles: mesh_object_t[] = [];
					for (let p of Project.paintObjects) if (p.base.visible) visibles.push(p);
					UtilMesh.mergeMesh(visibles);
					Context.raw.ddirty = 2;
				}
			}
		}
	}

	///if is_lab
	static setDefaultMesh = (name: string) => {
		let mo: mesh_object_t = null;
		if (name == ".Plane" || name == ".Sphere") {
			let res = Config.raw.rp_supersample > 1.0 ? 2048 : 1024;
			let mesh: any = name == ".Plane" ? Geom.make_plane(1, 1, res, res) : Geom.make_uv_sphere(1.0, res, Math.floor(res / 2), false, 2.0);
			let raw = {
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
			let md: mesh_data_t;
			mesh_data_create(raw, (_md: mesh_data_t) => { md = _md; });
			mo = mesh_object_create(md, Context.raw.paintObject.materials);
			array_remove(scene_meshes, mo);
			mo.base.name = "Tessellated";
		}
		else {
			mo = scene_get_child(name).ext;
		}

		mo.base.visible = true;
		Context.raw.ddirty = 2;
		Context.raw.paintObject = mo;
		Project.paintObjects[0] = mo;
		if (UIHeader.worktab.position == SpaceType.Space3D) {
			scene_meshes = [mo];
		}

		///if (krom_direct3d12 || krom_vulkan || krom_metal)
		RenderPathRaytrace.ready = false;
		///end
	}
	///end
}
