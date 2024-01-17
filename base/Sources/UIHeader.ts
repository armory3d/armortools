
class UIHeader {

	static inst: UIHeader;

	static defaultHeaderH = 28;
	static headerh = UIHeader.defaultHeaderH;
	headerHandle = new Handle({ layout: Layout.Horizontal });
	worktab = new Handle();

	constructor() {
		UIHeader.inst = this;
	}

	renderUI = (g: Graphics2) => {
		let ui = UIBase.inst.ui;
		if (Config.raw.touch_ui) {
			UIHeader.headerh = UIHeader.defaultHeaderH + 6;
		}
		else {
			UIHeader.headerh = UIHeader.defaultHeaderH;
		}
		UIHeader.headerh = Math.floor(UIHeader.headerh * ui.SCALE());

		if (Config.raw.layout[LayoutSize.LayoutHeader] == 0) return;

		let nodesw = (UINodes.inst.show || UIView2D.inst.show) ? Config.raw.layout[LayoutSize.LayoutNodesW] : 0;
		///if is_lab
		let ww = System.width - nodesw;
		///else
		let ww = System.width - UIToolbar.inst.toolbarw - Config.raw.layout[LayoutSize.LayoutSidebarW] - nodesw;
		///end

		if (ui.window(this.headerHandle, App.x(), UIHeader.headerh, ww, UIHeader.headerh)) {
			ui._y += 2;
			this.drawToolProperties(ui);
		}
	}

	///if is_paint

	drawToolProperties = (ui: Zui) => {
		if (Context.raw.tool == WorkspaceTool.ToolColorId) {
			ui.text(tr("Picked Color"));
			if (Context.raw.colorIdPicked) {
				ui.image(RenderPath.active.renderTargets.get("texpaint_colorid").image, 0xffffffff, 64);
			}
			ui.enabled = Context.raw.colorIdPicked;
			if (ui.button(tr("Clear"))) {
				Context.raw.colorIdPicked = false;
				UIToolbar.inst.toolbarHandle.redraws = 1;
			}
			ui.enabled = true;
			ui.text(tr("Color ID Map"));
			if (Project.assetNames.length > 0) {
				let cid = ui.combo(Context.raw.colorIdHandle, Base.enumTexts("TEX_IMAGE"), tr("Color ID"));
				if (Context.raw.colorIdHandle.changed) {
					Context.raw.ddirty = 2;
					Context.raw.colorIdPicked = false;
					UIToolbar.inst.toolbarHandle.redraws = 1;
				}
				ui.image(Project.getImage(Project.assets[cid]));
				if (ui.isHovered) ui.tooltipImage(Project.getImage(Project.assets[cid]), 256);
			}
			if (ui.button(tr("Import"))) {
				UIFiles.show(Path.textureFormats.join(","), false, true, (path: string) => {
					ImportAsset.run(path, -1.0, -1.0, true, false);

					Context.raw.colorIdHandle.position = Project.assetNames.length - 1;
					for (let a of Project.assets) {
						// Already imported
						if (a.file == path) Context.raw.colorIdHandle.position = Project.assets.indexOf(a);
					}
					Context.raw.ddirty = 2;
					Context.raw.colorIdPicked = false;
					UIToolbar.inst.toolbarHandle.redraws = 1;
					UIBase.inst.hwnds[2].redraws = 2;
				});
			}
			ui.enabled = Context.raw.colorIdPicked;
			if (ui.button(tr("To Mask"))) {
				if (Context.raw.layer.isMask()) Context.setLayer(Context.raw.layer.parent);
				let m = Base.newMask(false, Context.raw.layer);
				let _next = () => {
					if (Base.pipeMerge == null) Base.makePipe();
					if (ConstData.screenAlignedVB == null) ConstData.createScreenAlignedData();
					m.texpaint.g4.begin();
					m.texpaint.g4.setPipeline(Base.pipeColorIdToMask);
					m.texpaint.g4.setTexture(Base.texpaintColorId, RenderPath.active.renderTargets.get("texpaint_colorid").image);
					m.texpaint.g4.setTexture(Base.texColorId, Project.getImage(Project.assets[Context.raw.colorIdHandle.position]));
					m.texpaint.g4.setVertexBuffer(ConstData.screenAlignedVB);
					m.texpaint.g4.setIndexBuffer(ConstData.screenAlignedIB);
					m.texpaint.g4.drawIndexedVertices();
					m.texpaint.g4.end();
					Context.raw.colorIdPicked = false;
					UIToolbar.inst.toolbarHandle.redraws = 1;
					UIHeader.inst.headerHandle.redraws = 1;
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

			let h = Zui.handle("uiheader_0");
			let color: Color = 0xffffffff;
			color = color_set_rb(color, baseRPicked * 255);
			color = color_set_gb(color, baseGPicked * 255);
			color = color_set_bb(color, baseBPicked * 255);
			h.color = color;
			let state = ui.text("", 0, h.color);
			if (state == State.Started) {
				let mouse = Input.getMouse();
				let uix = ui._x;
				let uiy = ui._y;
				Base.dragOffX = -(mouse.x - uix - ui._windowX - 3);
				Base.dragOffY = -(mouse.y - uiy - ui._windowY + 1);
				Base.dragSwatch = Project.cloneSwatch(Context.raw.pickedColor);
			}
			if (ui.isHovered) ui.tooltip(tr("Drag and drop picked color to swatches, materials, layers or to the node editor"));
			if (ui.isHovered && ui.inputReleased) {
				UIMenu.draw((ui: Zui) => {
					ui.fill(0, 0, ui._w / ui.SCALE(), ui.t.ELEMENT_H * 9, ui.t.SEPARATOR_COL);
					ui.changed = false;
					ui.colorWheel(h, false, null, 10 * ui.t.ELEMENT_H * ui.SCALE(), false);
					if (ui.changed) UIMenu.keepOpen = true;
				}, 10);
			}
			if (ui.button(tr("Add Swatch"))) {
				let newSwatch = Project.cloneSwatch(Context.raw.pickedColor);
				Context.setSwatch(newSwatch);
				Project.raw.swatches.push(newSwatch);
				UIBase.inst.hwnds[2].redraws = 1;
			}
			if (ui.isHovered) ui.tooltip(tr("Add picked color to swatches"));

			ui.text(tr("Base") + ` (${baseRPicked},${baseGPicked},${baseBPicked})`);
			ui.text(tr("Normal") + ` (${normalRPicked},${normalGPicked},${normalBPicked})`);
			ui.text(tr("Occlusion") + ` (${occlusionPicked})`);
			ui.text(tr("Roughness") + ` (${roughnessPicked})`);
			ui.text(tr("Metallic") + ` (${metallicPicked})`);
			ui.text(tr("Height") + ` (${heightPicked})`);
			ui.text(tr("Opacity") + ` (${opacityPicked})`);
			Context.raw.pickerSelectMaterial = ui.check(Zui.handle("uiheader_1", { selected: Context.raw.pickerSelectMaterial }), tr("Select Material"));
			ui.combo(Context.raw.pickerMaskHandle, [tr("None"), tr("Material")], tr("Mask"), true);
			if (Context.raw.pickerMaskHandle.changed) {
				MakeMaterial.parsePaintMaterial();
			}
		}
		else if (Context.raw.tool == WorkspaceTool.ToolBake) {
			ui.changed = false;

			///if (krom_direct3d12 || krom_vulkan || krom_metal)
			let baking = Context.raw.pdirty > 0;
			let rtBake = Context.raw.bakeType == BakeType.BakeAO || Context.raw.bakeType == BakeType.BakeLightmap || Context.raw.bakeType == BakeType.BakeBentNormal || Context.raw.bakeType == BakeType.BakeThickness;
			if (baking && ui.button(tr("Stop"))) {
				Context.raw.pdirty = 0;
				Context.raw.rdirty = 2;
			}
			///else
			let baking = false;
			let rtBake = false;
			///end

			if (!baking && ui.button(tr("Bake"))) {
				Context.raw.pdirty = rtBake ? Context.raw.bakeSamples : 1;
				Context.raw.rdirty = 3;
				Base.notifyOnNextFrame(() => {
					Context.raw.layerPreviewDirty = true;
				});
				UIBase.inst.hwnds[0].redraws = 2;
				History.pushUndo = true;
				///if (krom_direct3d12 || krom_vulkan || krom_metal)
				RenderPathRaytraceBake.currentSample = 0;
				///end
			}

			let bakeHandle = Zui.handle("uiheader_2", { position: Context.raw.bakeType });
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
			if (Krom.raytraceSupported()) {
				bakes.push(tr("Lightmap"));
				bakes.push(tr("Bent Normal"));
				bakes.push(tr("Thickness"));
			}
			else {
				bakes.shift(); // Remove AO
			}
			///end

			Context.raw.bakeType = ui.combo(bakeHandle, bakes, tr("Bake"));

			///if (krom_direct3d12 || krom_vulkan || krom_metal)
			if (!Krom.raytraceSupported()) {
				Context.raw.bakeType += 1; // Offset for removed AO
			}
			///end

			///if (krom_direct3d12 || krom_vulkan || krom_metal)
			if (rtBake) {
				let samplesHandle = Zui.handle("uiheader_3", { value: Context.raw.bakeSamples });
				Context.raw.bakeSamples = Math.floor(ui.slider(samplesHandle, tr("Samples"), 1, 512, true, 1));
			}
			///end

			if (Context.raw.bakeType == BakeType.BakeNormalObject || Context.raw.bakeType == BakeType.BakePosition || Context.raw.bakeType == BakeType.BakeBentNormal) {
				let bakeUpAxisHandle = Zui.handle("uiheader_4", { position: Context.raw.bakeUpAxis });
				Context.raw.bakeUpAxis = ui.combo(bakeUpAxisHandle, [tr("Z"), tr("Y")], tr("Up Axis"), true);
			}
			if (Context.raw.bakeType == BakeType.BakeAO || Context.raw.bakeType == BakeType.BakeCurvature) {
				let bakeAxisHandle = Zui.handle("uiheader_5", { position: Context.raw.bakeAxis });
				Context.raw.bakeAxis = ui.combo(bakeAxisHandle, [tr("XYZ"), tr("X"), tr("Y"), tr("Z"), tr("-X"), tr("-Y"), tr("-Z")], tr("Axis"), true);
			}
			if (Context.raw.bakeType == BakeType.BakeAO) {
				let strengthHandle = Zui.handle("uiheader_6", { value: Context.raw.bakeAoStrength });
				Context.raw.bakeAoStrength = ui.slider(strengthHandle, tr("Strength"), 0.0, 2.0, true);
				let radiusHandle = Zui.handle("uiheader_7", { value: Context.raw.bakeAoRadius });
				Context.raw.bakeAoRadius = ui.slider(radiusHandle, tr("Radius"), 0.0, 2.0, true);
				let offsetHandle = Zui.handle("uiheader_8", { value: Context.raw.bakeAoOffset });
				Context.raw.bakeAoOffset = ui.slider(offsetHandle, tr("Offset"), 0.0, 2.0, true);
			}
			///if (krom_direct3d12 || krom_vulkan || krom_metal)
			if (rtBake) {
				let progress = RenderPathRaytraceBake.currentSample / Context.raw.bakeSamples;
				if (progress > 1.0) progress = 1.0;
				// Progress bar
				ui.g.color = ui.t.SEPARATOR_COL;
				ui.drawRect(ui.g, true, ui._x + 1, ui._y, ui._w - 2, ui.ELEMENT_H());
				ui.g.color = ui.t.HIGHLIGHT_COL;
				ui.drawRect(ui.g, true, ui._x + 1, ui._y, (ui._w - 2) * progress, ui.ELEMENT_H());
				ui.g.color = 0xffffffff;
				ui.text(tr("Samples") + ": " + RenderPathRaytraceBake.currentSample);
				ui.text(tr("Rays/pixel" + ": ") + RenderPathRaytraceBake.raysPix);
				ui.text(tr("Rays/second" + ": ") + RenderPathRaytraceBake.raysSec);
			}
			///end
			if (Context.raw.bakeType == BakeType.BakeCurvature) {
				let strengthHandle = Zui.handle("uiheader_9", { value: Context.raw.bakeCurvStrength });
				Context.raw.bakeCurvStrength = ui.slider(strengthHandle, tr("Strength"), 0.0, 2.0, true);
				let radiusHandle = Zui.handle("uiheader_10", { value: Context.raw.bakeCurvRadius });
				Context.raw.bakeCurvRadius = ui.slider(radiusHandle, tr("Radius"), 0.0, 2.0, true);
				let offsetHandle = Zui.handle("uiheader_11", { value: Context.raw.bakeCurvOffset });
				Context.raw.bakeCurvOffset = ui.slider(offsetHandle, tr("Offset"), -2.0, 2.0, true);
				let smoothHandle = Zui.handle("uiheader_12", { value: Context.raw.bakeCurvSmooth });
				Context.raw.bakeCurvSmooth = Math.floor(ui.slider(smoothHandle, tr("Smooth"), 0, 5, false, 1));
			}
			if (Context.raw.bakeType == BakeType.BakeNormal || Context.raw.bakeType == BakeType.BakeHeight || Context.raw.bakeType == BakeType.BakeDerivative) {
				let ar = [];
				for (let p of Project.paintObjects) ar.push(p.name);
				let polyHandle = Zui.handle("uiheader_13", { position: Context.raw.bakeHighPoly });
				Context.raw.bakeHighPoly = ui.combo(polyHandle, ar, tr("High Poly"));
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
					Context.raw.brushDecalMaskRadius = ui.slider(Context.raw.brushDecalMaskRadiusHandle, tr("Radius"), 0.01, 2.0, true);
					if (ui.isHovered) ui.tooltip(tr("Hold {brush_radius} and move mouse to the left or press {brush_radius_decrease} to decrease the radius\nHold {brush_radius} and move mouse to the right or press {brush_radius_increase} to increase the radius", new Map([["brush_radius", Config.keymap.brush_radius], ["brush_radius_decrease", Config.keymap.brush_radius_decrease], ["brush_radius_increase", Config.keymap.brush_radius_increase]])));
				}
				else {
					Context.raw.brushRadius = ui.slider(Context.raw.brushRadiusHandle, tr("Radius"), 0.01, 2.0, true);
					if (ui.isHovered) ui.tooltip(tr("Hold {brush_radius} and move mouse to the left or press {brush_radius_decrease} to decrease the radius\nHold {brush_radius} and move mouse to the right or press {brush_radius_increase} to increase the radius", new Map([["brush_radius", Config.keymap.brush_radius], ["brush_radius_decrease", Config.keymap.brush_radius_decrease], ["brush_radius_increase", Config.keymap.brush_radius_increase]])));
				}
			}

			if (Context.raw.tool == WorkspaceTool.ToolDecal || Context.raw.tool == WorkspaceTool.ToolText) {
				Context.raw.brushScaleX = ui.slider(Context.raw.brushScaleXHandle, tr("Scale X"), 0.01, 2.0, true);
			}

			if (Context.raw.tool == WorkspaceTool.ToolBrush  ||
				Context.raw.tool == WorkspaceTool.ToolFill   ||
				Context.raw.tool == WorkspaceTool.ToolDecal  ||
				Context.raw.tool == WorkspaceTool.ToolText) {
				let brushScaleHandle = Zui.handle("uiheader_14", { value: Context.raw.brushScale });
				Context.raw.brushScale = ui.slider(brushScaleHandle, tr("UV Scale"), 0.01, 5.0, true);
				if (brushScaleHandle.changed) {
					if (Context.raw.tool == WorkspaceTool.ToolDecal || Context.raw.tool == WorkspaceTool.ToolText) {
						ui.g.end();
						UtilRender.makeDecalPreview();
						ui.g.begin(false);
					}
				}

				Context.raw.brushAngle = ui.slider(Context.raw.brushAngleHandle, tr("Angle"), 0.0, 360.0, true, 1);
				if (ui.isHovered) ui.tooltip(tr("Hold {brush_angle} and move mouse to the left to decrease the angle\nHold {brush_angle} and move mouse to the right to increase the angle", new Map([["brush_angle", Config.keymap.brush_angle]])));

				if (Context.raw.brushAngleHandle.changed) {
					MakeMaterial.parsePaintMaterial();
				}
			}

			Context.raw.brushOpacity = ui.slider(Context.raw.brushOpacityHandle, tr("Opacity"), 0.0, 1.0, true);
			if (ui.isHovered) ui.tooltip(tr("Hold {brush_opacity} and move mouse to the left to decrease the opacity\nHold {brush_opacity} and move mouse to the right to increase the opacity", new Map([["brush_opacity", Config.keymap.brush_opacity]])));

			if (Context.raw.tool == WorkspaceTool.ToolBrush || Context.raw.tool == WorkspaceTool.ToolEraser || Context.raw.tool == WorkspaceTool.ToolClone || decalMask) {
				Context.raw.brushHardness = ui.slider(Zui.handle("uiheader_15", { value: Context.raw.brushHardness }), tr("Hardness"), 0.0, 1.0, true);
			}

			if (Context.raw.tool != WorkspaceTool.ToolEraser) {
				let brushBlendingHandle = Zui.handle("uiheader_16", { value: Context.raw.brushBlending });
				Context.raw.brushBlending = ui.combo(brushBlendingHandle, [
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
				let paintHandle = Zui.handle("uiheader_17");
				Context.raw.brushPaint = ui.combo(paintHandle, [tr("UV Map"), tr("Triplanar"), tr("Project")], tr("TexCoord"));
				if (paintHandle.changed) {
					MakeMaterial.parsePaintMaterial();
				}
			}
			if (Context.raw.tool == WorkspaceTool.ToolText) {
				let h = Zui.handle("uiheader_18");
				h.text = Context.raw.textToolText;
				let w = ui._w;
				if (ui.textSelectedHandle_ptr == h.ptr || ui.submitTextHandle_ptr == h.ptr) {
					ui._w *= 3;
				}

				Context.raw.textToolText = ui.textInput(h, "", Align.Left, true, true);
				ui._w = w;

				if (h.changed) {
					ui.g.end();
					UtilRender.makeTextPreview();
					UtilRender.makeDecalPreview();
					ui.g.begin(false);
				}
			}

			if (Context.raw.tool == WorkspaceTool.ToolFill) {
				ui.combo(Context.raw.fillTypeHandle, [tr("Object"), tr("Face"), tr("Angle"), tr("UV Island")], tr("Fill Mode"));
				if (Context.raw.fillTypeHandle.changed) {
					if (Context.raw.fillTypeHandle.position == FillType.FillFace) {
						ui.g.end();
						// UtilUV.cacheUVMap();
						UtilUV.cacheTriangleMap();
						ui.g.begin(false);
						// wireframeHandle.selected = drawWireframe = true;
					}
					MakeMaterial.parsePaintMaterial();
					MakeMaterial.parseMeshMaterial();
				}
			}
			else {
				let _w = ui._w;
				let sc = ui.SCALE();
				let touchHeader = (Config.raw.touch_ui && Config.raw.layout[LayoutSize.LayoutHeader] == 1);
				if (touchHeader) ui._x -= 4 * sc;
				ui._w = Math.floor((touchHeader ? 54 : 60) * sc);

				let xrayHandle = Zui.handle("uiheader_19", { selected: Context.raw.xray });
				Context.raw.xray = ui.check(xrayHandle, tr("X-Ray"));
				if (xrayHandle.changed) {
					MakeMaterial.parsePaintMaterial();
				}

				let symXHandle = Zui.handle("uiheader_20", { selected: false });
				let symYHandle = Zui.handle("uiheader_21", { selected: false });
				let symZHandle = Zui.handle("uiheader_22", { selected: false });

				if (Config.raw.layout[LayoutSize.LayoutHeader] == 1) {
					if (Config.raw.touch_ui) {
						ui._w = Math.floor(19 * sc);
						Context.raw.symX = ui.check(symXHandle, "");
						ui._x -= 4 * sc;
						Context.raw.symY = ui.check(symYHandle, "");
						ui._x -= 4 * sc;
						Context.raw.symZ = ui.check(symZHandle, "");
						ui._x -= 4 * sc;
						ui._w = Math.floor(40 * sc);
						ui.text(tr("X") + tr("Y") + tr("Z"));
					}
					else {
						ui._w = Math.floor(56 * sc);
						ui.text(tr("Symmetry"));
						ui._w = Math.floor(25 * sc);
						Context.raw.symX = ui.check(symXHandle, tr("X"));
						Context.raw.symY = ui.check(symYHandle, tr("Y"));
						Context.raw.symZ = ui.check(symZHandle, tr("Z"));
					}
					ui._w = _w;
				}
				else {
					// Popup
					ui._w = _w;
					Context.raw.symX = ui.check(symXHandle, tr("Symmetry") + " " + tr("X"));
					Context.raw.symY = ui.check(symYHandle, tr("Symmetry") + " " + tr("Y"));
					Context.raw.symZ = ui.check(symZHandle, tr("Symmetry") + " " + tr("Z"));
				}

				if (symXHandle.changed || symYHandle.changed || symZHandle.changed) {
					MakeMaterial.parsePaintMaterial();
				}
			}

			///if arm_physics
			if (Context.raw.tool == WorkspaceTool.ToolParticle) {
				ui._x += 10 * ui.SCALE();
				let physHandle = Zui.handle("uiheader_23", { selected: false });
				Context.raw.particlePhysics = ui.check(physHandle, tr("Physics"));
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
	drawToolProperties = (ui: Zui) => {
		if (Context.raw.tool == WorkspaceTool.ToolBrush) {
			Context.raw.brushRadius = ui.slider(Context.raw.brushRadiusHandle, tr("Radius"), 0.01, 2.0, true);
			if (ui.isHovered) ui.tooltip(tr("Hold {brush_radius} and move mouse to the left or press {brush_radius_decrease} to decrease the radius\nHold {brush_radius} and move mouse to the right or press {brush_radius_increase} to increase the radius", new Map([["brush_radius", Config.keymap.brush_radius], ["brush_radius_decrease", Config.keymap.brush_radius_decrease], ["brush_radius_increase", Config.keymap.brush_radius_increase]])));
		}
	}
	///end

	///if is_lab
	drawToolProperties = (ui: Zui) => {
		if (Context.raw.tool == WorkspaceTool.ToolPicker) {

		}
		else if (Context.raw.tool == WorkspaceTool.ToolEraser ||
				 Context.raw.tool == WorkspaceTool.ToolClone  ||
				 Context.raw.tool == WorkspaceTool.ToolBlur   ||
				 Context.raw.tool == WorkspaceTool.ToolSmudge) {

			let nodes = UINodes.inst.getNodes();
			let canvas = UINodes.inst.getCanvas(true);
			let inpaint = nodes.nodesSelectedId.length > 0 && nodes.getNode(canvas.nodes, nodes.nodesSelectedId[0]).type == "InpaintNode";
			if (inpaint) {
				Context.raw.brushRadius = ui.slider(Context.raw.brushRadiusHandle, tr("Radius"), 0.01, 2.0, true);
				if (ui.isHovered) ui.tooltip(tr("Hold {brush_radius} and move mouse to the left or press {brush_radius_decrease} to decrease the radius\nHold {brush_radius} and move mouse to the right or press {brush_radius_increase} to increase the radius", new Map([["brush_radius", Config.keymap.brush_radius], ["brush_radius_decrease", Config.keymap.brush_radius_decrease], ["brush_radius_increase", Config.keymap.brush_radius_increase]])));
			}
		}
	}
	///end
}
