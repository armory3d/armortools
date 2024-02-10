
class UIHeader {

	static defaultHeaderH = 28;
	static headerh = UIHeader.defaultHeaderH;
	static headerHandle = zui_handle_create({ layout: Layout.Horizontal });
	static worktab = zui_handle_create();

	constructor() {
	}

	static renderUI = () => {
		let ui = UIBase.ui;
		if (Config.raw.touch_ui) {
			UIHeader.headerh = UIHeader.defaultHeaderH + 6;
		}
		else {
			UIHeader.headerh = UIHeader.defaultHeaderH;
		}
		UIHeader.headerh = Math.floor(UIHeader.headerh * zui_SCALE(ui));

		if (Config.raw.layout[LayoutSize.LayoutHeader] == 0) return;

		let nodesw = (UINodes.show || UIView2D.show) ? Config.raw.layout[LayoutSize.LayoutNodesW] : 0;
		///if is_lab
		let ww = sys_width() - nodesw;
		///else
		let ww = sys_width() - UIToolbar.toolbarw - Config.raw.layout[LayoutSize.LayoutSidebarW] - nodesw;
		///end

		if (zui_window(UIHeader.headerHandle, app_x(), UIHeader.headerh, ww, UIHeader.headerh)) {
			ui._y += 2;
			UIHeader.drawToolProperties(ui);
		}
	}

	///if is_paint

	static drawToolProperties = (ui: zui_t) => {
		if (Context.raw.tool == WorkspaceTool.ToolColorId) {
			zui_text(tr("Picked Color"));
			if (Context.raw.colorIdPicked) {
				zui_image(render_path_render_targets.get("texpaint_colorid").image, 0xffffffff, 64);
			}
			ui.enabled = Context.raw.colorIdPicked;
			if (zui_button(tr("Clear"))) {
				Context.raw.colorIdPicked = false;
				UIToolbar.toolbarHandle.redraws = 1;
			}
			ui.enabled = true;
			zui_text(tr("Color ID Map"));
			if (Project.assetNames.length > 0) {
				let cid = zui_combo(Context.raw.colorIdHandle, Base.enumTexts("TEX_IMAGE"), tr("Color ID"));
				if (Context.raw.colorIdHandle.changed) {
					Context.raw.ddirty = 2;
					Context.raw.colorIdPicked = false;
					UIToolbar.toolbarHandle.redraws = 1;
				}
				zui_image(Project.getImage(Project.assets[cid]));
				if (ui.is_hovered) zui_tooltip_image(Project.getImage(Project.assets[cid]), 256);
			}
			if (zui_button(tr("Import"))) {
				UIFiles.show(Path.textureFormats.join(","), false, true, (path: string) => {
					ImportAsset.run(path, -1.0, -1.0, true, false);

					Context.raw.colorIdHandle.position = Project.assetNames.length - 1;
					for (let a of Project.assets) {
						// Already imported
						if (a.file == path) Context.raw.colorIdHandle.position = Project.assets.indexOf(a);
					}
					Context.raw.ddirty = 2;
					Context.raw.colorIdPicked = false;
					UIToolbar.toolbarHandle.redraws = 1;
					UIBase.hwnds[2].redraws = 2;
				});
			}
			ui.enabled = Context.raw.colorIdPicked;
			if (zui_button(tr("To Mask"))) {
				if (SlotLayer.isMask(Context.raw.layer)) Context.setLayer(Context.raw.layer.parent);
				let m = Base.newMask(false, Context.raw.layer);
				let _next = () => {
					if (Base.pipeMerge == null) Base.makePipe();
					if (const_data_screen_aligned_vb == null) const_data_create_screen_aligned_data();
					g4_begin(m.texpaint);
					g4_set_pipeline(Base.pipeColorIdToMask);
					g4_set_tex(Base.texpaintColorId,render_path_render_targets.get("texpaint_colorid").image);
					g4_set_tex(Base.texColorId, Project.getImage(Project.assets[Context.raw.colorIdHandle.position]));
					g4_set_vertex_buffer(const_data_screen_aligned_vb);
					g4_set_index_buffer(const_data_screen_aligned_ib);
					g4_draw();
					g4_end();
					Context.raw.colorIdPicked = false;
					UIToolbar.toolbarHandle.redraws = 1;
					UIHeader.headerHandle.redraws = 1;
					Context.raw.layerPreviewDirty = true;
					Base.updateFillLayers();
				}
				Base.notifyOnNextFrame(_next);
				History.newWhiteMask();
			}
			ui.enabled = true;
		}
		else if (Context.raw.tool == WorkspaceTool.ToolPicker || Context.raw.tool == WorkspaceTool.ToolMaterial) {
			let baseRPicked = Math.round(color_get_rb(Context.raw.pickedColor.base) / 255 * 10) / 10;
			let baseGPicked = Math.round(color_get_gb(Context.raw.pickedColor.base) / 255 * 10) / 10;
			let baseBPicked = Math.round(color_get_bb(Context.raw.pickedColor.base) / 255 * 10) / 10;
			let normalRPicked = Math.round(color_get_rb(Context.raw.pickedColor.normal) / 255 * 10) / 10;
			let normalGPicked = Math.round(color_get_gb(Context.raw.pickedColor.normal) / 255 * 10) / 10;
			let normalBPicked = Math.round(color_get_bb(Context.raw.pickedColor.normal) / 255 * 10) / 10;
			let occlusionPicked = Math.round(Context.raw.pickedColor.occlusion * 100) / 100;
			let roughnessPicked = Math.round(Context.raw.pickedColor.roughness * 100) / 100;
			let metallicPicked = Math.round(Context.raw.pickedColor.metallic * 100) / 100;
			let heightPicked = Math.round(Context.raw.pickedColor.height * 100) / 100;
			let opacityPicked = Math.round(Context.raw.pickedColor.opacity * 100) / 100;

			let h = zui_handle("uiheader_0");
			let color: Color = 0xffffffff;
			color = color_set_rb(color, baseRPicked * 255);
			color = color_set_gb(color, baseGPicked * 255);
			color = color_set_bb(color, baseBPicked * 255);
			h.color = color;
			let state = zui_text("", 0, h.color);
			if (state == State.Started) {
				let uix = ui._x;
				let uiy = ui._y;
				Base.dragOffX = -(mouse_x - uix - ui._window_x - 3);
				Base.dragOffY = -(mouse_y - uiy - ui._window_y + 1);
				Base.dragSwatch = Project.cloneSwatch(Context.raw.pickedColor);
			}
			if (ui.is_hovered) zui_tooltip(tr("Drag and drop picked color to swatches, materials, layers or to the node editor"));
			if (ui.is_hovered && ui.input_released) {
				UIMenu.draw((ui: zui_t) => {
					zui_fill(0, 0, ui._w / zui_SCALE(ui), ui.t.ELEMENT_H * 9, ui.t.SEPARATOR_COL);
					ui.changed = false;
					zui_color_wheel(h, false, null, 10 * ui.t.ELEMENT_H * zui_SCALE(ui), false);
					if (ui.changed) UIMenu.keepOpen = true;
				}, 10);
			}
			if (zui_button(tr("Add Swatch"))) {
				let newSwatch = Project.cloneSwatch(Context.raw.pickedColor);
				Context.setSwatch(newSwatch);
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
			Context.raw.pickerSelectMaterial = zui_check(zui_handle("uiheader_1", { selected: Context.raw.pickerSelectMaterial }), tr("Select Material"));
			zui_combo(Context.raw.pickerMaskHandle, [tr("None"), tr("Material")], tr("Mask"), true);
			if (Context.raw.pickerMaskHandle.changed) {
				MakeMaterial.parsePaintMaterial();
			}
		}
		else if (Context.raw.tool == WorkspaceTool.ToolBake) {
			ui.changed = false;

			///if (krom_direct3d12 || krom_vulkan || krom_metal)
			let baking = Context.raw.pdirty > 0;
			let rtBake = Context.raw.bakeType == BakeType.BakeAO || Context.raw.bakeType == BakeType.BakeLightmap || Context.raw.bakeType == BakeType.BakeBentNormal || Context.raw.bakeType == BakeType.BakeThickness;
			if (baking && zui_button(tr("Stop"))) {
				Context.raw.pdirty = 0;
				Context.raw.rdirty = 2;
			}
			///else
			let baking = false;
			let rtBake = false;
			///end

			if (!baking && zui_button(tr("Bake"))) {
				Context.raw.pdirty = rtBake ? Context.raw.bakeSamples : 1;
				Context.raw.rdirty = 3;
				Base.notifyOnNextFrame(() => {
					Context.raw.layerPreviewDirty = true;
				});
				UIBase.hwnds[0].redraws = 2;
				History.pushUndo = true;
				///if (krom_direct3d12 || krom_vulkan || krom_metal)
				RenderPathRaytraceBake.currentSample = 0;
				///end
			}

			let bakeHandle = zui_handle("uiheader_2", { position: Context.raw.bakeType });
			let bakes = [
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

			Context.raw.bakeType = zui_combo(bakeHandle, bakes, tr("Bake"));

			///if (krom_direct3d12 || krom_vulkan || krom_metal)
			if (!krom_raytrace_supported()) {
				Context.raw.bakeType += 1; // Offset for removed AO
			}
			///end

			///if (krom_direct3d12 || krom_vulkan || krom_metal)
			if (rtBake) {
				let samplesHandle = zui_handle("uiheader_3", { value: Context.raw.bakeSamples });
				Context.raw.bakeSamples = Math.floor(zui_slider(samplesHandle, tr("Samples"), 1, 512, true, 1));
			}
			///end

			if (Context.raw.bakeType == BakeType.BakeNormalObject || Context.raw.bakeType == BakeType.BakePosition || Context.raw.bakeType == BakeType.BakeBentNormal) {
				let bakeUpAxisHandle = zui_handle("uiheader_4", { position: Context.raw.bakeUpAxis });
				Context.raw.bakeUpAxis = zui_combo(bakeUpAxisHandle, [tr("Z"), tr("Y")], tr("Up Axis"), true);
			}
			if (Context.raw.bakeType == BakeType.BakeAO || Context.raw.bakeType == BakeType.BakeCurvature) {
				let bakeAxisHandle = zui_handle("uiheader_5", { position: Context.raw.bakeAxis });
				Context.raw.bakeAxis = zui_combo(bakeAxisHandle, [tr("XYZ"), tr("X"), tr("Y"), tr("Z"), tr("-X"), tr("-Y"), tr("-Z")], tr("Axis"), true);
			}
			if (Context.raw.bakeType == BakeType.BakeAO) {
				let strengthHandle = zui_handle("uiheader_6", { value: Context.raw.bakeAoStrength });
				Context.raw.bakeAoStrength = zui_slider(strengthHandle, tr("Strength"), 0.0, 2.0, true);
				let radiusHandle = zui_handle("uiheader_7", { value: Context.raw.bakeAoRadius });
				Context.raw.bakeAoRadius = zui_slider(radiusHandle, tr("Radius"), 0.0, 2.0, true);
				let offsetHandle = zui_handle("uiheader_8", { value: Context.raw.bakeAoOffset });
				Context.raw.bakeAoOffset = zui_slider(offsetHandle, tr("Offset"), 0.0, 2.0, true);
			}
			///if (krom_direct3d12 || krom_vulkan || krom_metal)
			if (rtBake) {
				let progress = RenderPathRaytraceBake.currentSample / Context.raw.bakeSamples;
				if (progress > 1.0) progress = 1.0;
				// Progress bar
				g2_set_color(ui.t.SEPARATOR_COL);
				zui_draw_rect(true, ui._x + 1, ui._y, ui._w - 2, zui_ELEMENT_H(ui));
				g2_set_color(ui.t.HIGHLIGHT_COL);
				zui_draw_rect(true, ui._x + 1, ui._y, (ui._w - 2) * progress, zui_ELEMENT_H(ui));
				g2_set_color(0xffffffff);
				zui_text(tr("Samples") + ": " + RenderPathRaytraceBake.currentSample);
				zui_text(tr("Rays/pixel" + ": ") + RenderPathRaytraceBake.raysPix);
				zui_text(tr("Rays/second" + ": ") + RenderPathRaytraceBake.raysSec);
			}
			///end
			if (Context.raw.bakeType == BakeType.BakeCurvature) {
				let strengthHandle = zui_handle("uiheader_9", { value: Context.raw.bakeCurvStrength });
				Context.raw.bakeCurvStrength = zui_slider(strengthHandle, tr("Strength"), 0.0, 2.0, true);
				let radiusHandle = zui_handle("uiheader_10", { value: Context.raw.bakeCurvRadius });
				Context.raw.bakeCurvRadius = zui_slider(radiusHandle, tr("Radius"), 0.0, 2.0, true);
				let offsetHandle = zui_handle("uiheader_11", { value: Context.raw.bakeCurvOffset });
				Context.raw.bakeCurvOffset = zui_slider(offsetHandle, tr("Offset"), -2.0, 2.0, true);
				let smoothHandle = zui_handle("uiheader_12", { value: Context.raw.bakeCurvSmooth });
				Context.raw.bakeCurvSmooth = Math.floor(zui_slider(smoothHandle, tr("Smooth"), 0, 5, false, 1));
			}
			if (Context.raw.bakeType == BakeType.BakeNormal || Context.raw.bakeType == BakeType.BakeHeight || Context.raw.bakeType == BakeType.BakeDerivative) {
				let ar = [];
				for (let p of Project.paintObjects) ar.push(p.base.name);
				let polyHandle = zui_handle("uiheader_13", { position: Context.raw.bakeHighPoly });
				Context.raw.bakeHighPoly = zui_combo(polyHandle, ar, tr("High Poly"));
			}
			if (ui.changed) {
				MakeMaterial.parsePaintMaterial();
			}
		}
		else if (Context.raw.tool == WorkspaceTool.ToolBrush ||
				 Context.raw.tool == WorkspaceTool.ToolEraser ||
				 Context.raw.tool == WorkspaceTool.ToolFill ||
				 Context.raw.tool == WorkspaceTool.ToolDecal ||
				 Context.raw.tool == WorkspaceTool.ToolText ||
				 Context.raw.tool == WorkspaceTool.ToolClone ||
				 Context.raw.tool == WorkspaceTool.ToolBlur ||
				 Context.raw.tool == WorkspaceTool.ToolSmudge ||
				 Context.raw.tool == WorkspaceTool.ToolParticle) {

			let decal = Context.raw.tool == WorkspaceTool.ToolDecal || Context.raw.tool == WorkspaceTool.ToolText;
			let decalMask = decal && Operator.shortcut(Config.keymap.decal_mask, ShortcutType.ShortcutDown);
			if (Context.raw.tool != WorkspaceTool.ToolFill) {
				if (decalMask) {
					Context.raw.brushDecalMaskRadius = zui_slider(Context.raw.brushDecalMaskRadiusHandle, tr("Radius"), 0.01, 2.0, true);
					if (ui.is_hovered) zui_tooltip(tr("Hold {brush_radius} and move mouse to the left or press {brush_radius_decrease} to decrease the radius\nHold {brush_radius} and move mouse to the right or press {brush_radius_increase} to increase the radius", new Map([["brush_radius", Config.keymap.brush_radius], ["brush_radius_decrease", Config.keymap.brush_radius_decrease], ["brush_radius_increase", Config.keymap.brush_radius_increase]])));
				}
				else {
					Context.raw.brushRadius = zui_slider(Context.raw.brushRadiusHandle, tr("Radius"), 0.01, 2.0, true);
					if (ui.is_hovered) zui_tooltip(tr("Hold {brush_radius} and move mouse to the left or press {brush_radius_decrease} to decrease the radius\nHold {brush_radius} and move mouse to the right or press {brush_radius_increase} to increase the radius", new Map([["brush_radius", Config.keymap.brush_radius], ["brush_radius_decrease", Config.keymap.brush_radius_decrease], ["brush_radius_increase", Config.keymap.brush_radius_increase]])));
				}
			}

			if (Context.raw.tool == WorkspaceTool.ToolDecal || Context.raw.tool == WorkspaceTool.ToolText) {
				Context.raw.brushScaleX = zui_slider(Context.raw.brushScaleXHandle, tr("Scale X"), 0.01, 2.0, true);
			}

			if (Context.raw.tool == WorkspaceTool.ToolBrush  ||
				Context.raw.tool == WorkspaceTool.ToolFill   ||
				Context.raw.tool == WorkspaceTool.ToolDecal  ||
				Context.raw.tool == WorkspaceTool.ToolText) {
				let brushScaleHandle = zui_handle("uiheader_14", { value: Context.raw.brushScale });
				Context.raw.brushScale = zui_slider(brushScaleHandle, tr("UV Scale"), 0.01, 5.0, true);
				if (brushScaleHandle.changed) {
					if (Context.raw.tool == WorkspaceTool.ToolDecal || Context.raw.tool == WorkspaceTool.ToolText) {
						let current = _g2_current;
						g2_end();
						UtilRender.makeDecalPreview();
						g2_begin(current, false);
					}
				}

				Context.raw.brushAngle = zui_slider(Context.raw.brushAngleHandle, tr("Angle"), 0.0, 360.0, true, 1);
				if (ui.is_hovered) zui_tooltip(tr("Hold {brush_angle} and move mouse to the left to decrease the angle\nHold {brush_angle} and move mouse to the right to increase the angle", new Map([["brush_angle", Config.keymap.brush_angle]])));

				if (Context.raw.brushAngleHandle.changed) {
					MakeMaterial.parsePaintMaterial();
				}
			}

			Context.raw.brushOpacity = zui_slider(Context.raw.brushOpacityHandle, tr("Opacity"), 0.0, 1.0, true);
			if (ui.is_hovered) zui_tooltip(tr("Hold {brush_opacity} and move mouse to the left to decrease the opacity\nHold {brush_opacity} and move mouse to the right to increase the opacity", new Map([["brush_opacity", Config.keymap.brush_opacity]])));

			if (Context.raw.tool == WorkspaceTool.ToolBrush || Context.raw.tool == WorkspaceTool.ToolEraser || Context.raw.tool == WorkspaceTool.ToolClone || decalMask) {
				Context.raw.brushHardness = zui_slider(zui_handle("uiheader_15", { value: Context.raw.brushHardness }), tr("Hardness"), 0.0, 1.0, true);
			}

			if (Context.raw.tool != WorkspaceTool.ToolEraser) {
				let brushBlendingHandle = zui_handle("uiheader_16", { value: Context.raw.brushBlending });
				Context.raw.brushBlending = zui_combo(brushBlendingHandle, [
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
					MakeMaterial.parsePaintMaterial();
				}
			}

			if (Context.raw.tool == WorkspaceTool.ToolBrush || Context.raw.tool == WorkspaceTool.ToolFill) {
				let paintHandle = zui_handle("uiheader_17");
				Context.raw.brushPaint = zui_combo(paintHandle, [tr("UV Map"), tr("Triplanar"), tr("Project")], tr("TexCoord"));
				if (paintHandle.changed) {
					MakeMaterial.parsePaintMaterial();
				}
			}
			if (Context.raw.tool == WorkspaceTool.ToolText) {
				let h = zui_handle("uiheader_18");
				h.text = Context.raw.textToolText;
				let w = ui._w;
				if (ui.text_selected_handle_ptr == h.ptr || ui.submit_text_handle_ptr == h.ptr) {
					ui._w *= 3;
				}

				Context.raw.textToolText = zui_text_input(h, "", Align.Left, true, true);
				ui._w = w;

				if (h.changed) {
					let current = _g2_current;
					g2_end();
					UtilRender.makeTextPreview();
					UtilRender.makeDecalPreview();
					g2_begin(current, false);
				}
			}

			if (Context.raw.tool == WorkspaceTool.ToolFill) {
				zui_combo(Context.raw.fillTypeHandle, [tr("Object"), tr("Face"), tr("Angle"), tr("UV Island")], tr("Fill Mode"));
				if (Context.raw.fillTypeHandle.changed) {
					if (Context.raw.fillTypeHandle.position == FillType.FillFace) {
						let current = _g2_current;
						g2_end();
						// UtilUV.cacheUVMap();
						UtilUV.cacheTriangleMap();
						g2_begin(current, false);
						// wireframeHandle.selected = drawWireframe = true;
					}
					MakeMaterial.parsePaintMaterial();
					MakeMaterial.parseMeshMaterial();
				}
			}
			else {
				let _w = ui._w;
				let sc = zui_SCALE(ui);
				let touchHeader = (Config.raw.touch_ui && Config.raw.layout[LayoutSize.LayoutHeader] == 1);
				if (touchHeader) ui._x -= 4 * sc;
				ui._w = Math.floor((touchHeader ? 54 : 60) * sc);

				let xrayHandle = zui_handle("uiheader_19", { selected: Context.raw.xray });
				Context.raw.xray = zui_check(xrayHandle, tr("X-Ray"));
				if (xrayHandle.changed) {
					MakeMaterial.parsePaintMaterial();
				}

				let symXHandle = zui_handle("uiheader_20", { selected: false });
				let symYHandle = zui_handle("uiheader_21", { selected: false });
				let symZHandle = zui_handle("uiheader_22", { selected: false });

				if (Config.raw.layout[LayoutSize.LayoutHeader] == 1) {
					if (Config.raw.touch_ui) {
						ui._w = Math.floor(19 * sc);
						Context.raw.symX = zui_check(symXHandle, "");
						ui._x -= 4 * sc;
						Context.raw.symY = zui_check(symYHandle, "");
						ui._x -= 4 * sc;
						Context.raw.symZ = zui_check(symZHandle, "");
						ui._x -= 4 * sc;
						ui._w = Math.floor(40 * sc);
						zui_text(tr("X") + tr("Y") + tr("Z"));
					}
					else {
						ui._w = Math.floor(56 * sc);
						zui_text(tr("Symmetry"));
						ui._w = Math.floor(25 * sc);
						Context.raw.symX = zui_check(symXHandle, tr("X"));
						Context.raw.symY = zui_check(symYHandle, tr("Y"));
						Context.raw.symZ = zui_check(symZHandle, tr("Z"));
					}
					ui._w = _w;
				}
				else {
					// Popup
					ui._w = _w;
					Context.raw.symX = zui_check(symXHandle, tr("Symmetry") + " " + tr("X"));
					Context.raw.symY = zui_check(symYHandle, tr("Symmetry") + " " + tr("Y"));
					Context.raw.symZ = zui_check(symZHandle, tr("Symmetry") + " " + tr("Z"));
				}

				if (symXHandle.changed || symYHandle.changed || symZHandle.changed) {
					MakeMaterial.parsePaintMaterial();
				}
			}

			///if arm_physics
			if (Context.raw.tool == WorkspaceTool.ToolParticle) {
				ui._x += 10 * zui_SCALE(ui);
				let physHandle = zui_handle("uiheader_23", { selected: false });
				Context.raw.particlePhysics = zui_check(physHandle, tr("Physics"));
				if (physHandle.changed) {
					UtilParticle.initParticlePhysics();
					MakeMaterial.parsePaintMaterial();
				}
			}
			///end
		}
	}

	///end

	///if is_sculpt
	static drawToolProperties = (ui: zui_t) => {
		if (Context.raw.tool == WorkspaceTool.ToolBrush) {
			Context.raw.brushRadius = zui_slider(Context.raw.brushRadiusHandle, tr("Radius"), 0.01, 2.0, true);
			if (ui.is_hovered) zui_tooltip(tr("Hold {brush_radius} and move mouse to the left or press {brush_radius_decrease} to decrease the radius\nHold {brush_radius} and move mouse to the right or press {brush_radius_increase} to increase the radius", new Map([["brush_radius", Config.keymap.brush_radius], ["brush_radius_decrease", Config.keymap.brush_radius_decrease], ["brush_radius_increase", Config.keymap.brush_radius_increase]])));
		}
	}
	///end

	///if is_lab
	static drawToolProperties = (ui: zui_t) => {
		if (Context.raw.tool == WorkspaceTool.ToolPicker) {

		}
		else if (Context.raw.tool == WorkspaceTool.ToolEraser ||
				 Context.raw.tool == WorkspaceTool.ToolClone  ||
				 Context.raw.tool == WorkspaceTool.ToolBlur   ||
				 Context.raw.tool == WorkspaceTool.ToolSmudge) {

			let nodes = UINodes.getNodes();
			let canvas = UINodes.getCanvas(true);
			let inpaint = nodes.nodesSelectedId.length > 0 && zui_get_node(canvas.nodes, nodes.nodesSelectedId[0]).type == "InpaintNode";
			if (inpaint) {
				Context.raw.brushRadius = zui_slider(Context.raw.brushRadiusHandle, tr("Radius"), 0.01, 2.0, true);
				if (ui.is_hovered) zui_tooltip(tr("Hold {brush_radius} and move mouse to the left or press {brush_radius_decrease} to decrease the radius\nHold {brush_radius} and move mouse to the right or press {brush_radius_increase} to increase the radius", new Map([["brush_radius", Config.keymap.brush_radius], ["brush_radius_decrease", Config.keymap.brush_radius_decrease], ["brush_radius_increase", Config.keymap.brush_radius_increase]])));
			}
		}
	}
	///end
}
