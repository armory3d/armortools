
class UIHeader {

	static default_header_h: i32 = 28;
	static headerh: i32 = UIHeader.default_header_h;
	static header_handle: zui_handle_t = zui_handle_create({ layout: zui_layout_t.HORIZONTAL });
	static worktab: zui_handle_t = zui_handle_create();

	constructor() {
	}

	static render_ui = () => {
		let ui: zui_t = UIBase.ui;
		if (Config.raw.touch_ui) {
			UIHeader.headerh = UIHeader.default_header_h + 6;
		}
		else {
			UIHeader.headerh = UIHeader.default_header_h;
		}
		UIHeader.headerh = Math.floor(UIHeader.headerh * zui_SCALE(ui));

		if (Config.raw.layout[layout_size_t.HEADER] == 0) return;

		let nodesw: i32 = (UINodes.show || UIView2D.show) ? Config.raw.layout[layout_size_t.NODES_W] : 0;
		///if is_lab
		let ww: i32 = sys_width() - nodesw;
		///else
		let ww: i32 = sys_width() - UIToolbar.toolbar_w - Config.raw.layout[layout_size_t.SIDEBAR_W] - nodesw;
		///end

		if (zui_window(UIHeader.header_handle, app_x(), UIHeader.headerh, ww, UIHeader.headerh)) {
			ui._y += 2;
			UIHeader.draw_tool_properties(ui);
		}
	}

	///if is_paint

	static draw_tool_properties = (ui: zui_t) => {
		if (Context.raw.tool == workspace_tool_t.COLORID) {
			zui_text(tr("Picked Color"));
			if (Context.raw.colorid_picked) {
				zui_image(render_path_render_targets.get("texpaint_colorid")._image, 0xffffffff, 64);
			}
			ui.enabled = Context.raw.colorid_picked;
			if (zui_button(tr("Clear"))) {
				Context.raw.colorid_picked = false;
				UIToolbar.toolbar_handle.redraws = 1;
			}
			ui.enabled = true;
			zui_text(tr("Color ID Map"));
			if (Project.asset_names.length > 0) {
				let cid: i32 = zui_combo(Context.raw.colorid_handle, Base.enum_texts("TEX_IMAGE"), tr("Color ID"));
				if (Context.raw.colorid_handle.changed) {
					Context.raw.ddirty = 2;
					Context.raw.colorid_picked = false;
					UIToolbar.toolbar_handle.redraws = 1;
				}
				zui_image(Project.get_image(Project.assets[cid]));
				if (ui.is_hovered) zui_tooltip_image(Project.get_image(Project.assets[cid]), 256);
			}
			if (zui_button(tr("Import"))) {
				UIFiles.show(Path.texture_formats.join(","), false, true, (path: string) => {
					ImportAsset.run(path, -1.0, -1.0, true, false);

					Context.raw.colorid_handle.position = Project.asset_names.length - 1;
					for (let a of Project.assets) {
						// Already imported
						if (a.file == path) Context.raw.colorid_handle.position = Project.assets.indexOf(a);
					}
					Context.raw.ddirty = 2;
					Context.raw.colorid_picked = false;
					UIToolbar.toolbar_handle.redraws = 1;
					UIBase.hwnds[2].redraws = 2;
				});
			}
			ui.enabled = Context.raw.colorid_picked;
			if (zui_button(tr("To Mask"))) {
				if (SlotLayer.is_mask(Context.raw.layer)) Context.set_layer(Context.raw.layer.parent);
				let m: SlotLayerRaw = Base.new_mask(false, Context.raw.layer);
				let _next = () => {
					if (Base.pipe_merge == null) Base.make_pipe();
					if (const_data_screen_aligned_vb == null) const_data_create_screen_aligned_data();
					g4_begin(m.texpaint);
					g4_set_pipeline(Base.pipe_colorid_to_mask);
					g4_set_tex(Base.texpaint_colorid,render_path_render_targets.get("texpaint_colorid")._image);
					g4_set_tex(Base.tex_colorid, Project.get_image(Project.assets[Context.raw.colorid_handle.position]));
					g4_set_vertex_buffer(const_data_screen_aligned_vb);
					g4_set_index_buffer(const_data_screen_aligned_ib);
					g4_draw();
					g4_end();
					Context.raw.colorid_picked = false;
					UIToolbar.toolbar_handle.redraws = 1;
					UIHeader.header_handle.redraws = 1;
					Context.raw.layer_preview_dirty = true;
					Base.update_fill_layers();
				}
				Base.notify_on_next_frame(_next);
				History.new_white_mask();
			}
			ui.enabled = true;
		}
		else if (Context.raw.tool == workspace_tool_t.PICKER || Context.raw.tool == workspace_tool_t.MATERIAL) {
			let baseRPicked: f32 = Math.round(color_get_rb(Context.raw.picked_color.base) / 255 * 10) / 10;
			let baseGPicked: f32 = Math.round(color_get_gb(Context.raw.picked_color.base) / 255 * 10) / 10;
			let baseBPicked: f32 = Math.round(color_get_bb(Context.raw.picked_color.base) / 255 * 10) / 10;
			let normalRPicked: f32 = Math.round(color_get_rb(Context.raw.picked_color.normal) / 255 * 10) / 10;
			let normalGPicked: f32 = Math.round(color_get_gb(Context.raw.picked_color.normal) / 255 * 10) / 10;
			let normalBPicked: f32 = Math.round(color_get_bb(Context.raw.picked_color.normal) / 255 * 10) / 10;
			let occlusionPicked: f32 = Math.round(Context.raw.picked_color.occlusion * 100) / 100;
			let roughnessPicked: f32 = Math.round(Context.raw.picked_color.roughness * 100) / 100;
			let metallicPicked: f32 = Math.round(Context.raw.picked_color.metallic * 100) / 100;
			let heightPicked: f32 = Math.round(Context.raw.picked_color.height * 100) / 100;
			let opacityPicked: f32 = Math.round(Context.raw.picked_color.opacity * 100) / 100;

			let h: zui_handle_t = zui_handle("uiheader_0");
			let color: color_t = 0xffffffff;
			color = color_set_rb(color, baseRPicked * 255);
			color = color_set_gb(color, baseGPicked * 255);
			color = color_set_bb(color, baseBPicked * 255);
			h.color = color;
			let state: zui_state_t = zui_text("", 0, h.color);
			if (state == zui_state_t.STARTED) {
				let uix: i32 = ui._x;
				let uiy: i32 = ui._y;
				Base.drag_off_x = -(mouse_x - uix - ui._window_x - 3);
				Base.drag_off_y = -(mouse_y - uiy - ui._window_y + 1);
				Base.drag_swatch = Project.clone_swatch(Context.raw.picked_color);
			}
			if (ui.is_hovered) zui_tooltip(tr("Drag and drop picked color to swatches, materials, layers or to the node editor"));
			if (ui.is_hovered && ui.input_released) {
				UIMenu.draw((ui: zui_t) => {
					zui_fill(0, 0, ui._w / zui_SCALE(ui), ui.t.ELEMENT_H * 9, ui.t.SEPARATOR_COL);
					ui.changed = false;
					zui_color_wheel(h, false, null, 10 * ui.t.ELEMENT_H * zui_SCALE(ui), false);
					if (ui.changed) UIMenu.keep_open = true;
				}, 10);
			}
			if (zui_button(tr("Add Swatch"))) {
				let newSwatch: swatch_color_t = Project.clone_swatch(Context.raw.picked_color);
				Context.set_swatch(newSwatch);
				Project.raw.swatches.push(newSwatch);
				UIBase.hwnds[2].redraws = 1;
			}
			if (ui.is_hovered) zui_tooltip(tr("Add picked color to swatches"));

			zui_text(tr("Base") + ` (${baseRPicked},${baseGPicked},${baseBPicked})`);
			zui_text(tr("Normal") + ` (${normalRPicked},${normalGPicked},${normalBPicked})`);
			zui_text(tr("Occlusion") + ` (${occlusionPicked})`);
			zui_text(tr("Roughness") + ` (${roughnessPicked})`);
			zui_text(tr("Metallic") + ` (${metallicPicked})`);
			zui_text(tr("Height") + ` (${heightPicked})`);
			zui_text(tr("Opacity") + ` (${opacityPicked})`);
			Context.raw.picker_select_material = zui_check(zui_handle("uiheader_1", { selected: Context.raw.picker_select_material }), tr("Select Material"));
			zui_combo(Context.raw.picker_mask_handle, [tr("None"), tr("Material")], tr("Mask"), true);
			if (Context.raw.picker_mask_handle.changed) {
				MakeMaterial.parse_paint_material();
			}
		}
		else if (Context.raw.tool == workspace_tool_t.BAKE) {
			ui.changed = false;

			///if (krom_direct3d12 || krom_vulkan || krom_metal)
			let baking: bool = Context.raw.pdirty > 0;
			let rtBake: bool = Context.raw.bake_type == bake_type_t.AO || Context.raw.bake_type == bake_type_t.LIGHTMAP || Context.raw.bake_type == bake_type_t.BENT_NORMAL || Context.raw.bake_type == bake_type_t.THICKNESS;
			if (baking && zui_button(tr("Stop"))) {
				Context.raw.pdirty = 0;
				Context.raw.rdirty = 2;
			}
			///else
			let baking: bool = false;
			let rtBake: bool = false;
			///end

			if (!baking && zui_button(tr("Bake"))) {
				Context.raw.pdirty = rtBake ? Context.raw.bake_samples : 1;
				Context.raw.rdirty = 3;
				Base.notify_on_next_frame(() => {
					Context.raw.layer_preview_dirty = true;
				});
				UIBase.hwnds[0].redraws = 2;
				History.push_undo = true;
				///if (krom_direct3d12 || krom_vulkan || krom_metal)
				RenderPathRaytraceBake.current_sample = 0;
				///end
			}

			let bakeHandle: zui_handle_t = zui_handle("uiheader_2", { position: Context.raw.bake_type });
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

			Context.raw.bake_type = zui_combo(bakeHandle, bakes, tr("Bake"));

			///if (krom_direct3d12 || krom_vulkan || krom_metal)
			if (!krom_raytrace_supported()) {
				Context.raw.bake_type += 1; // Offset for removed AO
			}
			///end

			///if (krom_direct3d12 || krom_vulkan || krom_metal)
			if (rtBake) {
				let samplesHandle: zui_handle_t = zui_handle("uiheader_3", { value: Context.raw.bake_samples });
				Context.raw.bake_samples = Math.floor(zui_slider(samplesHandle, tr("Samples"), 1, 512, true, 1));
			}
			///end

			if (Context.raw.bake_type == bake_type_t.NORMAL_OBJECT || Context.raw.bake_type == bake_type_t.POSITION || Context.raw.bake_type == bake_type_t.BENT_NORMAL) {
				let bakeUpAxisHandle: zui_handle_t = zui_handle("uiheader_4", { position: Context.raw.bake_up_axis });
				Context.raw.bake_up_axis = zui_combo(bakeUpAxisHandle, [tr("Z"), tr("Y")], tr("Up Axis"), true);
			}
			if (Context.raw.bake_type == bake_type_t.AO || Context.raw.bake_type == bake_type_t.CURVATURE) {
				let bakeAxisHandle: zui_handle_t = zui_handle("uiheader_5", { position: Context.raw.bake_axis });
				Context.raw.bake_axis = zui_combo(bakeAxisHandle, [tr("XYZ"), tr("X"), tr("Y"), tr("Z"), tr("-X"), tr("-Y"), tr("-Z")], tr("Axis"), true);
			}
			if (Context.raw.bake_type == bake_type_t.AO) {
				let strengthHandle: zui_handle_t = zui_handle("uiheader_6", { value: Context.raw.bake_ao_strength });
				Context.raw.bake_ao_strength = zui_slider(strengthHandle, tr("Strength"), 0.0, 2.0, true);
				let radiusHandle: zui_handle_t = zui_handle("uiheader_7", { value: Context.raw.bake_ao_radius });
				Context.raw.bake_ao_radius = zui_slider(radiusHandle, tr("Radius"), 0.0, 2.0, true);
				let offsetHandle: zui_handle_t = zui_handle("uiheader_8", { value: Context.raw.bake_ao_offset });
				Context.raw.bake_ao_offset = zui_slider(offsetHandle, tr("Offset"), 0.0, 2.0, true);
			}
			///if (krom_direct3d12 || krom_vulkan || krom_metal)
			if (rtBake) {
				let progress: f32 = RenderPathRaytraceBake.current_sample / Context.raw.bake_samples;
				if (progress > 1.0) progress = 1.0;
				// Progress bar
				g2_set_color(ui.t.SEPARATOR_COL);
				zui_draw_rect(true, ui._x + 1, ui._y, ui._w - 2, zui_ELEMENT_H(ui));
				g2_set_color(ui.t.HIGHLIGHT_COL);
				zui_draw_rect(true, ui._x + 1, ui._y, (ui._w - 2) * progress, zui_ELEMENT_H(ui));
				g2_set_color(0xffffffff);
				zui_text(tr("Samples") + ": " + RenderPathRaytraceBake.current_sample);
				zui_text(tr("Rays/pixel" + ": ") + RenderPathRaytraceBake.rays_pix);
				zui_text(tr("Rays/second" + ": ") + RenderPathRaytraceBake.rays_sec);
			}
			///end
			if (Context.raw.bake_type == bake_type_t.CURVATURE) {
				let strengthHandle: zui_handle_t = zui_handle("uiheader_9", { value: Context.raw.bake_curv_strength });
				Context.raw.bake_curv_strength = zui_slider(strengthHandle, tr("Strength"), 0.0, 2.0, true);
				let radiusHandle: zui_handle_t = zui_handle("uiheader_10", { value: Context.raw.bake_curv_radius });
				Context.raw.bake_curv_radius = zui_slider(radiusHandle, tr("Radius"), 0.0, 2.0, true);
				let offsetHandle: zui_handle_t = zui_handle("uiheader_11", { value: Context.raw.bake_curv_offset });
				Context.raw.bake_curv_offset = zui_slider(offsetHandle, tr("Offset"), -2.0, 2.0, true);
				let smoothHandle: zui_handle_t = zui_handle("uiheader_12", { value: Context.raw.bake_curv_smooth });
				Context.raw.bake_curv_smooth = Math.floor(zui_slider(smoothHandle, tr("Smooth"), 0, 5, false, 1));
			}
			if (Context.raw.bake_type == bake_type_t.NORMAL || Context.raw.bake_type == bake_type_t.HEIGHT || Context.raw.bake_type == bake_type_t.DERIVATIVE) {
				let ar: string[] = [];
				for (let p of Project.paint_objects) ar.push(p.base.name);
				let polyHandle: zui_handle_t = zui_handle("uiheader_13", { position: Context.raw.bake_high_poly });
				Context.raw.bake_high_poly = zui_combo(polyHandle, ar, tr("High Poly"));
			}
			if (ui.changed) {
				MakeMaterial.parse_paint_material();
			}
		}
		else if (Context.raw.tool == workspace_tool_t.BRUSH ||
				 Context.raw.tool == workspace_tool_t.ERASER ||
				 Context.raw.tool == workspace_tool_t.FILL ||
				 Context.raw.tool == workspace_tool_t.DECAL ||
				 Context.raw.tool == workspace_tool_t.TEXT ||
				 Context.raw.tool == workspace_tool_t.CLONE ||
				 Context.raw.tool == workspace_tool_t.BLUR ||
				 Context.raw.tool == workspace_tool_t.SMUDGE ||
				 Context.raw.tool == workspace_tool_t.PARTICLE) {

			let decal: bool = Context.raw.tool == workspace_tool_t.DECAL || Context.raw.tool == workspace_tool_t.TEXT;
			let decalMask: bool = decal && Operator.shortcut(Config.keymap.decal_mask, ShortcutType.ShortcutDown);
			if (Context.raw.tool != workspace_tool_t.FILL) {
				if (decalMask) {
					Context.raw.brush_decal_mask_radius = zui_slider(Context.raw.brush_decal_mask_radius_handle, tr("Radius"), 0.01, 2.0, true);
					if (ui.is_hovered) zui_tooltip(tr("Hold {brush_radius} and move mouse to the left or press {brush_radius_decrease} to decrease the radius\nHold {brush_radius} and move mouse to the right or press {brush_radius_increase} to increase the radius", new Map([["brush_radius", Config.keymap.brush_radius], ["brush_radius_decrease", Config.keymap.brush_radius_decrease], ["brush_radius_increase", Config.keymap.brush_radius_increase]])));
				}
				else {
					Context.raw.brush_radius = zui_slider(Context.raw.brush_radius_handle, tr("Radius"), 0.01, 2.0, true);
					if (ui.is_hovered) zui_tooltip(tr("Hold {brush_radius} and move mouse to the left or press {brush_radius_decrease} to decrease the radius\nHold {brush_radius} and move mouse to the right or press {brush_radius_increase} to increase the radius", new Map([["brush_radius", Config.keymap.brush_radius], ["brush_radius_decrease", Config.keymap.brush_radius_decrease], ["brush_radius_increase", Config.keymap.brush_radius_increase]])));
				}
			}

			if (Context.raw.tool == workspace_tool_t.DECAL || Context.raw.tool == workspace_tool_t.TEXT) {
				Context.raw.brush_scale_x = zui_slider(Context.raw.brush_scale_x_handle, tr("Scale X"), 0.01, 2.0, true);
			}

			if (Context.raw.tool == workspace_tool_t.BRUSH  ||
				Context.raw.tool == workspace_tool_t.FILL   ||
				Context.raw.tool == workspace_tool_t.DECAL  ||
				Context.raw.tool == workspace_tool_t.TEXT) {
				let brushScaleHandle: zui_handle_t = zui_handle("uiheader_14", { value: Context.raw.brush_scale });
				Context.raw.brush_scale = zui_slider(brushScaleHandle, tr("UV Scale"), 0.01, 5.0, true);
				if (brushScaleHandle.changed) {
					if (Context.raw.tool == workspace_tool_t.DECAL || Context.raw.tool == workspace_tool_t.TEXT) {
						let current: image_t = _g2_current;
						g2_end();
						UtilRender.make_decal_preview();
						g2_begin(current);
					}
				}

				Context.raw.brush_angle = zui_slider(Context.raw.brush_angle_handle, tr("Angle"), 0.0, 360.0, true, 1);
				if (ui.is_hovered) zui_tooltip(tr("Hold {brush_angle} and move mouse to the left to decrease the angle\nHold {brush_angle} and move mouse to the right to increase the angle", new Map([["brush_angle", Config.keymap.brush_angle]])));

				if (Context.raw.brush_angle_handle.changed) {
					MakeMaterial.parse_paint_material();
				}
			}

			Context.raw.brush_opacity = zui_slider(Context.raw.brush_opacity_handle, tr("Opacity"), 0.0, 1.0, true);
			if (ui.is_hovered) zui_tooltip(tr("Hold {brush_opacity} and move mouse to the left to decrease the opacity\nHold {brush_opacity} and move mouse to the right to increase the opacity", new Map([["brush_opacity", Config.keymap.brush_opacity]])));

			if (Context.raw.tool == workspace_tool_t.BRUSH || Context.raw.tool == workspace_tool_t.ERASER || Context.raw.tool == workspace_tool_t.CLONE || decalMask) {
				Context.raw.brush_hardness = zui_slider(zui_handle("uiheader_15", { value: Context.raw.brush_hardness }), tr("Hardness"), 0.0, 1.0, true);
			}

			if (Context.raw.tool != workspace_tool_t.ERASER) {
				let brushBlendingHandle: zui_handle_t = zui_handle("uiheader_16", { value: Context.raw.brush_blending });
				Context.raw.brush_blending = zui_combo(brushBlendingHandle, [
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
				if (brushBlendingHandle.changed) {
					MakeMaterial.parse_paint_material();
				}
			}

			if (Context.raw.tool == workspace_tool_t.BRUSH || Context.raw.tool == workspace_tool_t.FILL) {
				let paintHandle: zui_handle_t = zui_handle("uiheader_17");
				Context.raw.brush_paint = zui_combo(paintHandle, [tr("UV Map"), tr("Triplanar"), tr("Project")], tr("TexCoord"));
				if (paintHandle.changed) {
					MakeMaterial.parse_paint_material();
				}
			}
			if (Context.raw.tool == workspace_tool_t.TEXT) {
				let h: zui_handle_t = zui_handle("uiheader_18");
				h.text = Context.raw.text_tool_text;
				let w: i32 = ui._w;
				if (ui.text_selected_handle_ptr == h.ptr || ui.submit_text_handle_ptr == h.ptr) {
					ui._w *= 3;
				}

				Context.raw.text_tool_text = zui_text_input(h, "", zui_align_t.LEFT, true, true);
				ui._w = w;

				if (h.changed) {
					let current: image_t = _g2_current;
					g2_end();
					UtilRender.make_text_preview();
					UtilRender.make_decal_preview();
					g2_begin(current);
				}
			}

			if (Context.raw.tool == workspace_tool_t.FILL) {
				zui_combo(Context.raw.fill_type_handle, [tr("Object"), tr("Face"), tr("Angle"), tr("UV Island")], tr("Fill Mode"));
				if (Context.raw.fill_type_handle.changed) {
					if (Context.raw.fill_type_handle.position == fill_type_t.FACE) {
						let current: image_t = _g2_current;
						g2_end();
						// UtilUV.cacheUVMap();
						UtilUV.cache_triangle_map();
						g2_begin(current);
						// wireframeHandle.selected = drawWireframe = true;
					}
					MakeMaterial.parse_paint_material();
					MakeMaterial.parse_mesh_material();
				}
			}
			else {
				let _w: i32 = ui._w;
				let sc: f32 = zui_SCALE(ui);
				let touchHeader: bool = (Config.raw.touch_ui && Config.raw.layout[layout_size_t.HEADER] == 1);
				if (touchHeader) ui._x -= 4 * sc;
				ui._w = Math.floor((touchHeader ? 54 : 60) * sc);

				let xrayHandle: zui_handle_t = zui_handle("uiheader_19", { selected: Context.raw.xray });
				Context.raw.xray = zui_check(xrayHandle, tr("X-Ray"));
				if (xrayHandle.changed) {
					MakeMaterial.parse_paint_material();
				}

				let symXHandle: zui_handle_t = zui_handle("uiheader_20", { selected: false });
				let symYHandle: zui_handle_t = zui_handle("uiheader_21", { selected: false });
				let symZHandle: zui_handle_t = zui_handle("uiheader_22", { selected: false });

				if (Config.raw.layout[layout_size_t.HEADER] == 1) {
					if (Config.raw.touch_ui) {
						ui._w = Math.floor(19 * sc);
						Context.raw.sym_x = zui_check(symXHandle, "");
						ui._x -= 4 * sc;
						Context.raw.sym_y = zui_check(symYHandle, "");
						ui._x -= 4 * sc;
						Context.raw.sym_z = zui_check(symZHandle, "");
						ui._x -= 4 * sc;
						ui._w = Math.floor(40 * sc);
						zui_text(tr("X") + tr("Y") + tr("Z"));
					}
					else {
						ui._w = Math.floor(56 * sc);
						zui_text(tr("Symmetry"));
						ui._w = Math.floor(25 * sc);
						Context.raw.sym_x = zui_check(symXHandle, tr("X"));
						Context.raw.sym_y = zui_check(symYHandle, tr("Y"));
						Context.raw.sym_z = zui_check(symZHandle, tr("Z"));
					}
					ui._w = _w;
				}
				else {
					// Popup
					ui._w = _w;
					Context.raw.sym_x = zui_check(symXHandle, tr("Symmetry") + " " + tr("X"));
					Context.raw.sym_y = zui_check(symYHandle, tr("Symmetry") + " " + tr("Y"));
					Context.raw.sym_z = zui_check(symZHandle, tr("Symmetry") + " " + tr("Z"));
				}

				if (symXHandle.changed || symYHandle.changed || symZHandle.changed) {
					MakeMaterial.parse_paint_material();
				}
			}

			///if arm_physics
			if (Context.raw.tool == workspace_tool_t.PARTICLE) {
				ui._x += 10 * zui_SCALE(ui);
				let physHandle: zui_handle_t = zui_handle("uiheader_23", { selected: false });
				Context.raw.particle_physics = zui_check(physHandle, tr("Physics"));
				if (physHandle.changed) {
					UtilParticle.init_particle_physics();
					MakeMaterial.parse_paint_material();
				}
			}
			///end
		}
	}

	///end

	///if is_sculpt
	static draw_tool_properties = (ui: zui_t) => {
		if (Context.raw.tool == workspace_tool_t.BRUSH) {
			Context.raw.brush_radius = zui_slider(Context.raw.brush_radius_handle, tr("Radius"), 0.01, 2.0, true);
			if (ui.is_hovered) zui_tooltip(tr("Hold {brush_radius} and move mouse to the left or press {brush_radius_decrease} to decrease the radius\nHold {brush_radius} and move mouse to the right or press {brush_radius_increase} to increase the radius", new Map([["brush_radius", Config.keymap.brush_radius], ["brush_radius_decrease", Config.keymap.brush_radius_decrease], ["brush_radius_increase", Config.keymap.brush_radius_increase]])));
		}
	}
	///end

	///if is_lab
	static draw_tool_properties = (ui: zui_t) => {
		if (Context.raw.tool == workspace_tool_t.PICKER) {

		}
		else if (Context.raw.tool == workspace_tool_t.ERASER ||
				 Context.raw.tool == workspace_tool_t.CLONE  ||
				 Context.raw.tool == workspace_tool_t.BLUR   ||
				 Context.raw.tool == workspace_tool_t.SMUDGE) {

			let nodes: zui_nodes_t = UINodes.get_nodes();
			let canvas: zui_node_canvas_t = UINodes.get_canvas(true);
			let inpaint: bool = nodes.nodesSelectedId.length > 0 && zui_get_node(canvas.nodes, nodes.nodesSelectedId[0]).type == "InpaintNode";
			if (inpaint) {
				Context.raw.brush_radius = zui_slider(Context.raw.brush_radius_handle, tr("Radius"), 0.01, 2.0, true);
				if (ui.is_hovered) zui_tooltip(tr("Hold {brush_radius} and move mouse to the left or press {brush_radius_decrease} to decrease the radius\nHold {brush_radius} and move mouse to the right or press {brush_radius_increase} to increase the radius", new Map([["brush_radius", Config.keymap.brush_radius], ["brush_radius_decrease", Config.keymap.brush_radius_decrease], ["brush_radius_increase", Config.keymap.brush_radius_increase]])));
			}
		}
	}
	///end
}
