
class Base {

	static uiEnabled = true;
	static isDragging = false;
	static isResizing = false;
	static dragAsset: TAsset = null;
	static dragSwatch: TSwatchColor = null;
	static dragFile: string = null;
	static dragFileIcon: ImageRaw = null;
	static dragTint = 0xffffffff;
	static dragSize = -1;
	static dragRect: TRect = null;
	static dragOffX = 0.0;
	static dragOffY = 0.0;
	static dragStart = 0.0;
	static dropX = 0.0;
	static dropY = 0.0;
	static font: FontRaw = null;
	static theme: Theme;
	static colorWheel: ImageRaw;
	static colorWheelGradient: ImageRaw;
	static uiBox: Zui;
	static uiMenu: Zui;
	static defaultElementW = 100;
	static defaultElementH = 28;
	static defaultFontSize = 13;
	static resHandle = new Handle();
	static bitsHandle = new Handle();
	static dropPaths: string[] = [];
	static appx = 0;
	static appy = 0;
	static lastWindowWidth = 0;
	static lastWindowHeight = 0;
	///if (is_paint || is_sculpt)
	static dragMaterial: SlotMaterialRaw = null;
	static dragLayer: SlotLayerRaw = null;
	///end

	static pipeCopy: PipelineStateRaw;
	static pipeCopy8: PipelineStateRaw;
	static pipeCopy128: PipelineStateRaw;
	static pipeCopyBGRA: PipelineStateRaw;
	static pipeCopyRGB: PipelineStateRaw = null;
	///if (is_paint || is_sculpt)
	static pipeMerge: PipelineStateRaw = null;
	static pipeMergeR: PipelineStateRaw = null;
	static pipeMergeG: PipelineStateRaw = null;
	static pipeMergeB: PipelineStateRaw = null;
	static pipeMergeA: PipelineStateRaw = null;
	static pipeInvert8: PipelineStateRaw;
	static pipeApplyMask: PipelineStateRaw;
	static pipeMergeMask: PipelineStateRaw;
	static pipeColorIdToMask: PipelineStateRaw;
	static tex0: TextureUnit;
	static tex1: TextureUnit;
	static texmask: TextureUnit;
	static texa: TextureUnit;
	static opac: ConstantLocation;
	static blending: ConstantLocation;
	static tex0Mask: TextureUnit;
	static texaMask: TextureUnit;
	static tex0MergeMask: TextureUnit;
	static texaMergeMask: TextureUnit;
	static texColorId: TextureUnit;
	static texpaintColorId: TextureUnit;
	static opacMergeMask: ConstantLocation;
	static blendingMergeMask: ConstantLocation;
	static tempMaskImage: ImageRaw = null;
	///end
	///if is_lab
	static pipeCopyR: PipelineStateRaw;
	static pipeCopyG: PipelineStateRaw;
	static pipeCopyB: PipelineStateRaw;
	static pipeCopyA: PipelineStateRaw;
	static pipeCopyATex: TextureUnit;
	static pipeInpaintPreview: PipelineStateRaw;
	static tex0InpaintPreview: TextureUnit;
	static texaInpaintPreview: TextureUnit;
	///end
	static tempImage: ImageRaw = null;
	static expa: ImageRaw = null;
	static expb: ImageRaw = null;
	static expc: ImageRaw = null;
	static pipeCursor: PipelineStateRaw;
	static cursorVP: ConstantLocation;
	static cursorInvVP: ConstantLocation;
	static cursorMouse: ConstantLocation;
	static cursorTexStep: ConstantLocation;
	static cursorRadius: ConstantLocation;
	static cursorCameraRight: ConstantLocation;
	static cursorTint: ConstantLocation;
	static cursorTex: TextureUnit;
	static cursorGbufferD: TextureUnit;

	///if (is_paint || is_sculpt)
	static defaultBase = 0.5;
	static defaultRough = 0.4;
	///if (krom_android || krom_ios)
	static maxLayers = 18;
	///else
	static maxLayers = 255;
	///end
	///end
	static defaultFov = 0.69;

	constructor() {
		Base.lastWindowWidth = System.width;
		Base.lastWindowHeight = System.height;

		System.notifyOnDropFiles((dropPath: string) => {
			///if krom_linux
			dropPath = decodeURIComponent(dropPath);
			///end
			dropPath = trim_end(dropPath);
			Base.dropPaths.push(dropPath);
		});

		System.notifyOnApplicationState(
			() => { // Foreground
				Context.raw.foregroundEvent = true;
				Context.raw.lastPaintX = -1;
				Context.raw.lastPaintY = -1;
			},
			() => {}, // Resume
			() => {}, // Pause
			() => { // Background
				// Release keys after alt-tab / win-tab
				Keyboard.upListener(KeyCode.Alt);
				Keyboard.upListener(KeyCode.Win);
			},
			() => { // Shutdown
				///if (krom_android || krom_ios)
				Project.projectSave();
				///end
			}
		);

		Krom.setSaveAndQuitCallback(Base.saveAndQuitCallback);

		Data.getFont("font.ttf", (f: FontRaw) => {
			Data.getImage("color_wheel.k", (imageColorWheel: ImageRaw) => {
				Data.getImage("color_wheel_gradient.k", (imageColorWheelGradient: ImageRaw) => {

					Base.font = f;
					Config.loadTheme(Config.raw.theme, false);
					Base.defaultElementW = Base.theme.ELEMENT_W;
					Base.defaultFontSize = Base.theme.FONT_SIZE;
					Translator.loadTranslations(Config.raw.locale);
					UIFiles.filename = tr("untitled");
					///if (krom_android || krom_ios)
					System.title = tr("untitled");
					///end

					// Baked font for fast startup
					if (Config.raw.locale == "en") {
						Base.font.font_ = Krom.g2_font_13(Base.font.blob);
						Base.font.fontGlyphs = Graphics2.fontGlyphs;
					}
					else Font.init(Base.font);

					Base.colorWheel = imageColorWheel;
					Base.colorWheelGradient = imageColorWheelGradient;
					Nodes.enumTexts = Base.enumTexts;
					Nodes.tr = tr;
					Base.uiBox = new Zui({ theme: Base.theme, font: f, scaleFactor: Config.raw.window_scale, color_wheel: Base.colorWheel, black_white_gradient: Base.colorWheelGradient });
					Base.uiMenu = new Zui({ theme: Base.theme, font: f, scaleFactor: Config.raw.window_scale, color_wheel: Base.colorWheel, black_white_gradient: Base.colorWheelGradient });
					Base.defaultElementH = Base.uiMenu.t.ELEMENT_H;

					// Init plugins
					if (Config.raw.plugins != null) {
						for (let plugin of Config.raw.plugins) {
							Plugin.start(plugin);
						}
					}

					Args.parse();

					new Camera();
					new UIBase();
					new UINodes();
					new UIView2D();

					///if is_lab
					RandomNode.setSeed(Math.floor(Time.time() * 4294967295));
					///end

					App.notifyOnUpdate(Base.update);
					App.notifyOnRender2D(UIView2D.render);
					App.notifyOnUpdate(UIView2D.update);
					///if (is_paint || is_sculpt)
					App.notifyOnRender2D(UIBase.renderCursor);
					///end
					App.notifyOnUpdate(UINodes.update);
					App.notifyOnRender2D(UINodes.render);
					App.notifyOnUpdate(UIBase.update);
					App.notifyOnRender2D(UIBase.render);
					App.notifyOnUpdate(Camera.update);
					App.notifyOnRender2D(Base.render);

					///if (is_paint || is_sculpt)
					Base.appx = UIToolbar.toolbarw;
					///end
					///if is_lab
					Base.appx = 0;
					///end

					Base.appy = UIHeader.headerh;
					if (Config.raw.layout[LayoutSize.LayoutHeader] == 1) Base.appy += UIHeader.headerh;
					let cam = Scene.camera;
					cam.data.fov = Math.floor(cam.data.fov * 100) / 100;
					CameraObject.buildProjection(cam);

					Args.run();

					///if (krom_android || krom_ios)
					let hasProjects = Config.raw.recent_projects.length > 0;
					///else
					let hasProjects = true;
					///end

					if (Config.raw.splash_screen && hasProjects) {
						BoxProjects.show();
					}
				});
			});
		});
	}

	static saveAndQuitCallback = (save: bool) => {
		Base.saveWindowRect();
		if (save) Project.projectSave(true);
		else System.stop();
	}

	///if (is_paint || is_sculpt)
	static w = (): i32 => {
		// Drawing material preview
		if (Context.raw.materialPreview) {
			return UtilRender.materialPreviewSize;
		}

		// Drawing decal preview
		if (Context.raw.decalPreview) {
			return UtilRender.decalPreviewSize;
		}

		let res = 0;
		if (Config.raw.layout == null) {
			let sidebarw = UIBase.defaultSidebarW;
			res = System.width - sidebarw - UIToolbar.defaultToolbarW;
		}
		else if (UINodes.show || UIView2D.show) {
			res = System.width - Config.raw.layout[LayoutSize.LayoutSidebarW] - Config.raw.layout[LayoutSize.LayoutNodesW] - UIToolbar.toolbarw;
		}
		else if (UIBase.show) {
			res = System.width - Config.raw.layout[LayoutSize.LayoutSidebarW] - UIToolbar.toolbarw;
		}
		else { // Distract free
			res = System.width;
		}
		if (Context.raw.viewIndex > -1) {
			res = Math.floor(res / 2);
		}
		if (Context.raw.paint2dView) {
			res = UIView2D.ww;
		}

		return res > 0 ? res : 1; // App was minimized, force render path resize
	}

	static h = (): i32 => {
		// Drawing material preview
		if (Context.raw.materialPreview) {
			return UtilRender.materialPreviewSize;
		}

		// Drawing decal preview
		if (Context.raw.decalPreview) {
			return UtilRender.decalPreviewSize;
		}

		let res = System.height;

		if (Config.raw.layout == null) {
			res -= UIHeader.defaultHeaderH * 2 + UIStatus.defaultStatusH;

			///if (krom_android || krom_ios)
			let layoutHeader = 0;
			///else
			let layoutHeader = 1;
			///end
			if (layoutHeader == 0) {
				res += UIHeader.headerh;
			}
		}
		else if (UIBase.show && res > 0) {
			let statush = Config.raw.layout[LayoutSize.LayoutStatusH];
			res -= Math.floor(UIHeader.defaultHeaderH * 2 * Config.raw.window_scale) + statush;

			if (Config.raw.layout[LayoutSize.LayoutHeader] == 0) {
				res += UIHeader.headerh;
			}
		}

		return res > 0 ? res : 1; // App was minimized, force render path resize
	}
	///end

	///if is_lab
	static w = (): i32 => {
		let res = 0;
		if (UINodes == null) {
			res = System.width;
		}
		else if (UINodes.show || UIView2D.show) {
			res = System.width - Config.raw.layout[LayoutSize.LayoutNodesW];
		}
		else { // Distract free
			res = System.width;
		}

		return res > 0 ? res : 1; // App was minimized, force render path resize
	}

	static h = (): i32 => {
		let res = System.height;
		if (UIBase == null) {
			res -= UIHeader.defaultHeaderH * 2 + UIStatus.defaultStatusH;
		}
		else if (res > 0) {
			let statush = Config.raw.layout[LayoutSize.LayoutStatusH];
			res -= Math.floor(UIHeader.defaultHeaderH * 2 * Config.raw.window_scale) + statush;
		}

		return res > 0 ? res : 1; // App was minimized, force render path resize
	}
	///end

	static x = (): i32 => {
		///if (is_paint || is_sculpt)
		return Context.raw.viewIndex == 1 ? Base.appx + Base.w() : Base.appx;
		///end
		///if is_lab
		return Base.appx;
		///end
	}

	static y = (): i32 => {
		return Base.appy;
	}

	static onResize = () => {
		if (System.width == 0 || System.height == 0) return;

		let ratioW = System.width / Base.lastWindowWidth;
		Base.lastWindowWidth = System.width;
		let ratioH = System.height / Base.lastWindowHeight;
		Base.lastWindowHeight = System.height;

		Config.raw.layout[LayoutSize.LayoutNodesW] = Math.floor(Config.raw.layout[LayoutSize.LayoutNodesW] * ratioW);
		///if (is_paint || is_sculpt)
		Config.raw.layout[LayoutSize.LayoutSidebarH0] = Math.floor(Config.raw.layout[LayoutSize.LayoutSidebarH0] * ratioH);
		Config.raw.layout[LayoutSize.LayoutSidebarH1] = System.height - Config.raw.layout[LayoutSize.LayoutSidebarH0];
		///end

		Base.resize();

		///if (krom_linux || krom_darwin)
		Base.saveWindowRect();
		///end
	}

	static saveWindowRect = () => {
		///if (krom_windows || krom_linux || krom_darwin)
		Config.raw.window_w = System.width;
		Config.raw.window_h = System.height;
		Config.raw.window_x = System.x;
		Config.raw.window_y = System.y;
		Config.save();
		///end
	}

	static resize = () => {
		if (System.width == 0 || System.height == 0) return;

		let cam = Scene.camera;
		if (cam.data.ortho != null) {
			cam.data.ortho[2] = -2 * (App.h() / App.w());
			cam.data.ortho[3] =  2 * (App.h() / App.w());
		}
		CameraObject.buildProjection(cam);

		if (Context.raw.cameraType == CameraType.CameraOrthographic) {
			Viewport.updateCameraType(Context.raw.cameraType);
		}

		Context.raw.ddirty = 2;

		if (UIBase.show) {
			///if (is_paint || is_sculpt)
			Base.appx = UIToolbar.toolbarw;
			///end
			///if is_lab
			Base.appx = 0;
			///end
			Base.appy = UIHeader.headerh * 2;
			if (Config.raw.layout[LayoutSize.LayoutHeader] == 0) {
				Base.appy -= UIHeader.headerh;
			}
		}
		else {
			Base.appx = 0;
			Base.appy = 0;
		}

		if (UINodes.grid != null) {
			let _grid = UINodes.grid;
			let _next = () => {
				Image.unload(_grid);
			}
			Base.notifyOnNextFrame(_next);
			UINodes.grid = null;
		}

		Base.redrawUI();
	}

	static redrawUI = () => {
		UIHeader.headerHandle.redraws = 2;
		UIBase.hwnds[TabArea.TabStatus].redraws = 2;
		UIMenubar.menuHandle.redraws = 2;
		UIMenubar.workspaceHandle.redraws = 2;
		UINodes.hwnd.redraws = 2;
		UIBox.hwnd.redraws = 2;
		UIView2D.hwnd.redraws = 2;
		if (Context.raw.ddirty < 0) Context.raw.ddirty = 0; // Redraw viewport
		///if (is_paint || is_sculpt)
		UIBase.hwnds[TabArea.TabSidebar0].redraws = 2;
		UIBase.hwnds[TabArea.TabSidebar1].redraws = 2;
		UIToolbar.toolbarHandle.redraws = 2;
		if (Context.raw.splitView) Context.raw.ddirty = 1;
		///end
	}

	static update = () => {
		if (Mouse.movementX != 0 || Mouse.movementY != 0) {
			Krom.setMouseCursor(0); // Arrow
		}

		///if (is_paint || is_sculpt)
		let hasDrag = Base.dragAsset != null || Base.dragMaterial != null || Base.dragLayer != null || Base.dragFile != null || Base.dragSwatch != null;
		///end
		///if is_lab
		let hasDrag = Base.dragAsset != null || Base.dragFile != null || Base.dragSwatch != null;
		///end

		if (Config.raw.touch_ui) {
			// Touch and hold to activate dragging
			if (Base.dragStart < 0.2) {
				if (hasDrag && Mouse.down()) Base.dragStart += Time.realDelta;
				else Base.dragStart = 0;
				hasDrag = false;
			}
			if (Mouse.released()) {
				Base.dragStart = 0;
			}
			let moved = Math.abs(Mouse.movementX) > 1 && Math.abs(Mouse.movementY) > 1;
			if ((Mouse.released() || moved) && !hasDrag) {
				Base.dragAsset = null;
				Base.dragSwatch = null;
				Base.dragFile = null;
				Base.dragFileIcon = null;
				Base.isDragging = false;
				///if (is_paint || is_sculpt)
				Base.dragMaterial = null;
				Base.dragLayer = null;
				///end
			}
			// Disable touch scrolling while dragging is active
			Zui.touchScroll = !Base.isDragging;
		}

		if (hasDrag && (Mouse.movementX != 0 || Mouse.movementY != 0)) {
			Base.isDragging = true;
		}
		if (Mouse.released() && hasDrag) {
			if (Base.dragAsset != null) {
				if (Context.inNodes()) { // Create image texture
					UINodes.acceptAssetDrag(Project.assets.indexOf(Base.dragAsset));
				}
				else if (Context.inViewport()) {
					if (Base.dragAsset.file.toLowerCase().endsWith(".hdr")) {
						let image = Project.getImage(Base.dragAsset);
						ImportEnvmap.run(Base.dragAsset.file, image);
					}
				}
				///if (is_paint || is_sculpt)
				else if (Context.inLayers() || Context.in2dView()) { // Create mask
					Base.createImageMask(Base.dragAsset);
				}
				///end
				Base.dragAsset = null;
			}
			else if (Base.dragSwatch != null) {
				if (Context.inNodes()) { // Create RGB node
					UINodes.acceptSwatchDrag(Base.dragSwatch);
				}
				else if (Context.inSwatches()) {
					TabSwatches.acceptSwatchDrag(Base.dragSwatch);
				}
				///if (is_paint || is_sculpt)
				else if (Context.inMaterials()) {
					TabMaterials.acceptSwatchDrag(Base.dragSwatch);
				}
				else if (Context.inViewport()) {
					let color = Base.dragSwatch.base;
					color = color_set_ab(color, Base.dragSwatch.opacity * 255);
					Base.createColorLayer(color, Base.dragSwatch.occlusion, Base.dragSwatch.roughness, Base.dragSwatch.metallic);
				}
				else if (Context.inLayers() && TabLayers.canDropNewLayer(Context.raw.dragDestination)) {
					let color = Base.dragSwatch.base;
					color = color_set_ab(color, Base.dragSwatch.opacity * 255);
					Base.createColorLayer(color, Base.dragSwatch.occlusion, Base.dragSwatch.roughness, Base.dragSwatch.metallic, Context.raw.dragDestination);
				}
				///end

				Base.dragSwatch = null;
			}
			else if (Base.dragFile != null) {
				if (!Context.inBrowser()) {
					Base.dropX = Mouse.x;
					Base.dropY = Mouse.y;

					///if (is_paint || is_sculpt)
					let materialCount = Project.materials.length;
					ImportAsset.run(Base.dragFile, Base.dropX, Base.dropY, true, true, () => {
						// Asset was material
						if (Project.materials.length > materialCount) {
							Base.dragMaterial = Context.raw.material;
							Base.materialDropped();
						}
					});
					///end

					///if is_lab
					ImportAsset.run(Base.dragFile, Base.dropX, Base.dropY);
					///end
				}
				Base.dragFile = null;
				Base.dragFileIcon = null;
			}
			///if (is_paint || is_sculpt)
			else if (Base.dragMaterial != null) {
				Base.materialDropped();
			}
			else if (Base.dragLayer != null) {
				if (Context.inNodes()) {
					UINodes.acceptLayerDrag(Project.layers.indexOf(Base.dragLayer));
				}
				else if (Context.inLayers() && Base.isDragging) {
					SlotLayer.move(Base.dragLayer, Context.raw.dragDestination);
					MakeMaterial.parseMeshMaterial();
				}
				Base.dragLayer = null;
			}
			///end

			Krom.setMouseCursor(0); // Arrow
			Base.isDragging = false;
		}
		if (Context.raw.colorPickerCallback != null && (Mouse.released() || Mouse.released("right"))) {
			Context.raw.colorPickerCallback = null;
			Context.selectTool(Context.raw.colorPickerPreviousTool);
		}

		Base.handleDropPaths();

		///if (is_paint || is_sculpt)
		///if krom_windows
		let isPicker = Context.raw.tool == WorkspaceTool.ToolPicker || Context.raw.tool == WorkspaceTool.ToolMaterial;
		let decal = Context.raw.tool == WorkspaceTool.ToolDecal || Context.raw.tool == WorkspaceTool.ToolText;
		Zui.alwaysRedrawWindow = !Context.raw.cacheDraws ||
			UIMenu.show ||
			UIBox.show ||
			Base.isDragging ||
			isPicker ||
			decal ||
			UIView2D.show ||
			!Config.raw.brush_3d ||
			Context.raw.frame < 3;
		///end
		///end

		if (Zui.alwaysRedrawWindow && Context.raw.ddirty < 0) Context.raw.ddirty = 0;
	}

	///if (is_paint || is_sculpt)
	static materialDropped = () => {
		// Material drag and dropped onto viewport or layers tab
		if (Context.inViewport()) {
			let uvType = Keyboard.down("control") ? UVType.UVProject : UVType.UVMap;
			let decalMat = uvType == UVType.UVProject ? UtilRender.getDecalMat() : null;
			Base.createFillLayer(uvType, decalMat);
		}
		if (Context.inLayers() && TabLayers.canDropNewLayer(Context.raw.dragDestination)) {
			let uvType = Keyboard.down("control") ? UVType.UVProject : UVType.UVMap;
			let decalMat = uvType == UVType.UVProject ? UtilRender.getDecalMat() : null;
			Base.createFillLayer(uvType, decalMat, Context.raw.dragDestination);
		}
		else if (Context.inNodes()) {
			UINodes.acceptMaterialDrag(Project.materials.indexOf(Base.dragMaterial));
		}
		Base.dragMaterial = null;
	}
	///end

	static handleDropPaths = () => {
		if (Base.dropPaths.length > 0) {
			///if (krom_linux || krom_darwin)
			let wait = !Mouse.moved; // Mouse coords not updated during drag
			///else
			let wait = false;
			///end
			if (!wait) {
				Base.dropX = Mouse.x;
				Base.dropY = Mouse.y;
				let dropPath = Base.dropPaths.shift();
				ImportAsset.run(dropPath, Base.dropX, Base.dropY);
			}
		}
	}

	///if (is_paint || is_sculpt)
	static getDragBackground = (): TRect => {
		let icons = Res.get("icons.k");
		if (Base.dragLayer != null && !SlotLayer.isGroup(Base.dragLayer) && Base.dragLayer.fill_layer == null) {
			return Res.tile50(icons, 4, 1);
		}
		return null;
	}
	///end

	static getDragImage = (): ImageRaw => {
		Base.dragTint = 0xffffffff;
		Base.dragSize = -1;
		Base.dragRect = null;
		if (Base.dragAsset != null) {
			return Project.getImage(Base.dragAsset);
		}
		if (Base.dragSwatch != null) {
			Base.dragTint = Base.dragSwatch.base;
			Base.dragSize = 26;
			return TabSwatches.empty;
		}
		if (Base.dragFile != null) {
			if (Base.dragFileIcon != null) return Base.dragFileIcon;
			let icons = Res.get("icons.k");
			Base.dragRect = Base.dragFile.indexOf(".") > 0 ? Res.tile50(icons, 3, 1) : Res.tile50(icons, 2, 1);
			Base.dragTint = UIBase.ui.t.HIGHLIGHT_COL;
			return icons;
		}

		///if is_paint
		if (Base.dragMaterial != null) {
			return Base.dragMaterial.imageIcon;
		}
		if (Base.dragLayer != null && SlotLayer.isGroup(Base.dragLayer)) {
			let icons = Res.get("icons.k");
			let folderClosed = Res.tile50(icons, 2, 1);
			let folderOpen = Res.tile50(icons, 8, 1);
			Base.dragRect = Base.dragLayer.show_panel ? folderOpen : folderClosed;
			Base.dragTint = UIBase.ui.t.LABEL_COL - 0x00202020;
			return icons;
		}
		if (Base.dragLayer != null && SlotLayer.isMask(Base.dragLayer) && Base.dragLayer.fill_layer == null) {
			TabLayers.makeMaskPreviewRgba32(Base.dragLayer);
			return Context.raw.maskPreviewRgba32;
		}
		if (Base.dragLayer != null) {
			return Base.dragLayer.fill_layer != null ? Base.dragLayer.fill_layer.imageIcon : Base.dragLayer.texpaint_preview;
		}
		///end

		return null;
	}

	static render = (g: Graphics2Raw) => {
		if (System.width == 0 || System.height == 0) return;

		if (Context.raw.frame == 2) {
			///if (is_paint || is_sculpt)
			UtilRender.makeMaterialPreview();
			UIBase.hwnds[TabArea.TabSidebar1].redraws = 2;
			///end

			MakeMaterial.parseMeshMaterial();
			MakeMaterial.parsePaintMaterial();
			Context.raw.ddirty = 0;

			///if (is_paint || is_sculpt)
			if (History.undoLayers == null) {
				History.undoLayers = [];
				for (let i = 0; i < Config.raw.undo_steps; ++i) {
					let l = SlotLayer.create("_undo" + History.undoLayers.length);
					History.undoLayers.push(l);
				}
			}
			///end

			// Default workspace
			if (Config.raw.workspace != 0) {
				UIHeader.worktab.position = Config.raw.workspace;
				UIMenubar.workspaceHandle.redraws = 2;
				UIHeader.worktab.changed = true;
			}

			// Default camera controls
			Context.raw.cameraControls = Config.raw.camera_controls;

			///if is_lab
			Base.notifyOnNextFrame(() => {
				Base.notifyOnNextFrame(() => {
					TabMeshes.setDefaultMesh(".Sphere");
				});
			});
			///end

			///if is_sculpt
			Base.notifyOnNextFrame(() => {
				Base.notifyOnNextFrame(() => {
					Context.raw.projectType = ProjectModel.ModelSphere;
					Project.projectNew();
				});
			});
			///end
		}
		else if (Context.raw.frame == 3) {
			Context.raw.ddirty = 3;
		}
		Context.raw.frame++;

		if (Base.isDragging) {
			Krom.setMouseCursor(1); // Hand
			let img = Base.getDragImage();

			///if (is_paint || is_sculpt)
			let scaleFactor = UIBase.ui.SCALE();
			///end
			///if is_lab
			let scaleFactor = Base.uiBox.SCALE();
			///end

			let size = (Base.dragSize == -1 ? 50 : Base.dragSize) * scaleFactor;
			let ratio = size / img.width;
			let h = img.height * ratio;

			///if (is_lab || krom_direct3d11 || krom_direct3d12 || krom_metal || krom_vulkan)
			let inv = 0;
			///else
			let inv = (Base.dragMaterial != null || (Base.dragLayer != null && Base.dragLayer.fill_layer != null)) ? h : 0;
			///end

			g.color = Base.dragTint;

			///if (is_paint || is_sculpt)
			let bgRect = Base.getDragBackground();
			if (bgRect != null) {
				Graphics2.drawScaledSubImage(Res.get("icons.k"), bgRect.x, bgRect.y, bgRect.w, bgRect.h, Mouse.x + Base.dragOffX, Mouse.y + Base.dragOffY + inv, size, h - inv * 2);
			}
			///end

			Base.dragRect == null ?
				Graphics2.drawScaledImage(img, Mouse.x + Base.dragOffX, Mouse.y + Base.dragOffY + inv, size, h - inv * 2) :
				Graphics2.drawScaledSubImage(img, Base.dragRect.x, Base.dragRect.y, Base.dragRect.w, Base.dragRect.h, Mouse.x + Base.dragOffX, Mouse.y + Base.dragOffY + inv, size, h - inv * 2);
			g.color = 0xffffffff;
		}

		let usingMenu = UIMenu.show && Mouse.y > UIHeader.headerh;
		Base.uiEnabled = !UIBox.show && !usingMenu && !Base.isComboSelected();
		if (UIBox.show) UIBox.render(g);
		if (UIMenu.show) UIMenu.render(g);

		// Save last pos for continuos paint
		Context.raw.lastPaintVecX = Context.raw.paintVec.x;
		Context.raw.lastPaintVecY = Context.raw.paintVec.y;

		///if (krom_android || krom_ios)
		// No mouse move events for touch, re-init last paint position on touch start
		if (!Mouse.down()) {
			Context.raw.lastPaintX = -1;
			Context.raw.lastPaintY = -1;
		}
		///end
	}

	static enumTexts = (nodeType: string): string[] => {
		///if (is_paint || is_sculpt)
		if (nodeType == "TEX_IMAGE") {
			return Project.assetNames.length > 0 ? Project.assetNames : [""];
		}
		if (nodeType == "LAYER" || nodeType == "LAYER_MASK") {
			let layerNames: string[] = [];
			for (let l of Project.layers) layerNames.push(l.name);
			return layerNames;
		}
		if (nodeType == "MATERIAL") {
			let materialNames: string[] = [];
			for (let m of Project.materials) materialNames.push(m.canvas.name);
			return materialNames;
		}
		///end

		///if is_lab
		if (nodeType == "ImageTextureNode") {
			return Project.assetNames.length > 0 ? Project.assetNames : [""];
		}
		///end

		return null;
	}

	static getAssetIndex = (fileName: string): i32 => {
		let i = Project.assetNames.indexOf(fileName);
		return i >= 0 ? i : 0;
	}

	static notifyOnNextFrame = (f: ()=>void) => {
		let _render = (_: any) => {
			App.notifyOnInit(() => {
				let _update = () => {
					App.notifyOnInit(f);
					App.removeUpdate(_update);
				}
				App.notifyOnUpdate(_update);
			});
			App.removeRender(_render);
		}
		App.notifyOnRender(_render);
	}

	static toggleFullscreen = () => {
		if (System.mode == WindowMode.Windowed) {
			///if (krom_windows || krom_linux || krom_darwin)
			Config.raw.window_w = System.width;
			Config.raw.window_h = System.height;
			Config.raw.window_x = System.x;
			Config.raw.window_y = System.y;
			///end
			System.mode = WindowMode.Fullscreen;
		}
		else {
			System.mode = WindowMode.Windowed;
			System.resize(Config.raw.window_w, Config.raw.window_h);
			System.move(Config.raw.window_x, Config.raw.window_y);
		}
	}

	static isScrolling = (): bool => {
		for (let ui of Base.getUIs()) if (ui.isScrolling) return true;
		return false;
	}

	static isComboSelected = (): bool => {
		for (let ui of Base.getUIs()) if (ui.comboSelectedHandle_ptr != null) return true;
		return false;
	}

	static getUIs = (): Zui[] => {
		return [Base.uiBox, Base.uiMenu, UIBase.ui, UINodes.ui, UIView2D.ui];
	}

	static isDecalLayer = (): bool => {
		///if is_paint
		let isPaint = Context.raw.tool != WorkspaceTool.ToolMaterial && Context.raw.tool != WorkspaceTool.ToolBake;
		return isPaint && Context.raw.layer.fill_layer != null && Context.raw.layer.uvType == UVType.UVProject;
		///end

		///if (is_sculpt || is_lab)
		return false;
		///end
	}

	static redrawStatus = () => {
		UIBase.hwnds[TabArea.TabStatus].redraws = 2;
	}

	static redrawConsole = () => {
		let statush = Config.raw.layout[LayoutSize.LayoutStatusH];
		if (UIBase.ui != null && statush > UIStatus.defaultStatusH * UIBase.ui.SCALE()) {
			UIBase.hwnds[TabArea.TabStatus].redraws = 2;
		}
	}

	static initLayout = () => {
		let show2d = (UINodes != null && UINodes.show) || (UIView2D != null && UIView2D.show);

		let raw = Config.raw;
		raw.layout = [
			///if (is_paint || is_sculpt)
			Math.floor(UIBase.defaultSidebarW * raw.window_scale), // LayoutSidebarW
			Math.floor(System.height / 2), // LayoutSidebarH0
			Math.floor(System.height / 2), // LayoutSidebarH1
			///end

			///if krom_ios
			show2d ? Math.floor((App.w() + raw.layout[LayoutSize.LayoutNodesW]) * 0.473) : Math.floor(App.w() * 0.473), // LayoutNodesW
			///elseif krom_android
			show2d ? Math.floor((App.w() + raw.layout[LayoutSize.LayoutNodesW]) * 0.473) : Math.floor(App.w() * 0.473),
			///else
			show2d ? Math.floor((App.w() + raw.layout[LayoutSize.LayoutNodesW]) * 0.515) : Math.floor(App.w() * 0.515), // Align with ui header controls
			///end

			Math.floor(App.h() / 2), // LayoutNodesH
			Math.floor(UIStatus.defaultStatusH * raw.window_scale), // LayoutStatusH

			///if (krom_android || krom_ios)
			0, // LayoutHeader
			///else
			1,
			///end
		];

		raw.layout_tabs = [
			///if (is_paint || is_sculpt)
			0,
			0,
			///end
			0
		];
	}

	static initConfig = () => {
		let raw = Config.raw;
		raw.recent_projects = [];
		raw.bookmarks = [];
		raw.plugins = [];
		///if (krom_android || krom_ios)
		raw.keymap = "touch.json";
		///else
		raw.keymap = "default.json";
		///end
		raw.theme = "default.json";
		raw.server = "https://armorpaint.fra1.digitaloceanspaces.com";
		raw.undo_steps = 4;
		raw.pressure_radius = true;
		raw.pressure_sensitivity = 1.0;
		raw.camera_zoom_speed = 1.0;
		raw.camera_pan_speed = 1.0;
		raw.camera_rotation_speed = 1.0;
		raw.zoom_direction = ZoomDirection.ZoomVertical;
		///if (is_paint || is_sculpt)
		raw.displace_strength = 0.0;
		///else
		raw.displace_strength = 1.0;
		///end
		raw.wrap_mouse = false;
		///if is_paint
		raw.workspace = SpaceType.Space3D;
		///end
		///if is_sculpt
		raw.workspace = SpaceType.Space3D;
		///end
		///if is_lab
		raw.workspace = SpaceType.Space2D;
		///end
		///if (krom_android || krom_ios)
		raw.camera_controls = CameraControls.ControlsRotate;
		///else
		raw.camera_controls = CameraControls.ControlsOrbit;
		///end
		raw.layer_res = TextureRes.Res2048;
		///if (krom_android || krom_ios)
		raw.touch_ui = true;
		raw.splash_screen = true;
		///else
		raw.touch_ui = false;
		raw.splash_screen = false;
		///end
		///if (is_paint || is_sculpt)
		raw.node_preview = true;
		///else
		raw.node_preview = false;
		///end

		///if (is_paint || is_sculpt)
		raw.pressure_hardness = true;
		raw.pressure_angle = false;
		raw.pressure_opacity = false;
		///if (krom_vulkan || krom_ios)
		raw.material_live = false;
		///else
		raw.material_live = true;
		///end
		raw.brush_3d = true;
		raw.brush_depth_reject = true;
		raw.brush_angle_reject = true;
		raw.brush_live = false;
		raw.show_asset_names = false;
		///end

		///if is_paint
		raw.dilate = DilateType.DilateInstant;
		raw.dilate_radius = 2;
		///end

		///if is_lab
		raw.gpu_inference = true;
		///end
	}

	static initLayers = () => {
		///if (is_paint || is_sculpt)
		SlotLayer.clear(Project.layers[0], color_from_floats(Base.defaultBase, Base.defaultBase, Base.defaultBase, 1.0));
		///end

		///if is_lab
		let texpaint = RenderPath.renderTargets.get("texpaint").image;
		let texpaint_nor = RenderPath.renderTargets.get("texpaint_nor").image;
		let texpaint_pack = RenderPath.renderTargets.get("texpaint_pack").image;
		Graphics2.begin(texpaint.g2, false);
		Graphics2.drawScaledImage(Res.get("placeholder.k"), 0, 0, Config.getTextureResX(), Config.getTextureResY()); // Base
		Graphics2.end(texpaint.g2);
		Graphics4.begin(texpaint_nor.g4);
		Graphics4.clear(color_from_floats(0.5, 0.5, 1.0, 0.0)); // Nor
		Graphics4.end();
		Graphics4.begin(texpaint_pack.g4);
		Graphics4.clear(color_from_floats(1.0, 0.4, 0.0, 0.0)); // Occ, rough, met
		Graphics4.end();
		let texpaint_nor_empty = RenderPath.renderTargets.get("texpaint_nor_empty").image;
		let texpaint_pack_empty = RenderPath.renderTargets.get("texpaint_pack_empty").image;
		Graphics4.begin(texpaint_nor_empty.g4);
		Graphics4.clear(color_from_floats(0.5, 0.5, 1.0, 0.0)); // Nor
		Graphics4.end();
		Graphics4.begin(texpaint_pack_empty.g4);
		Graphics4.clear(color_from_floats(1.0, 0.4, 0.0, 0.0)); // Occ, rough, met
		Graphics4.end();
		///end
	}

	///if (is_paint || is_sculpt)
	static resizeLayers = () => {
		let C = Config.raw;
		if (Base.resHandle.position >= Math.floor(TextureRes.Res16384)) { // Save memory for >=16k
			C.undo_steps = 1;
			if (Context.raw.undoHandle != null) {
				Context.raw.undoHandle.value = C.undo_steps;
			}
			while (History.undoLayers.length > C.undo_steps) {
				let l = History.undoLayers.pop();
				Base.notifyOnNextFrame(() => {
					SlotLayer.unload(l);
				});
			}
		}
		for (let l of Project.layers) SlotLayer.resizeAndSetBits(l);
		for (let l of History.undoLayers) SlotLayer.resizeAndSetBits(l);
		let rts = RenderPath.renderTargets;
		let _texpaint_blend0 = rts.get("texpaint_blend0").image;
		Base.notifyOnNextFrame(() => {
			Image.unload(_texpaint_blend0);
		});
		rts.get("texpaint_blend0").width = Config.getTextureResX();
		rts.get("texpaint_blend0").height = Config.getTextureResY();
		rts.get("texpaint_blend0").image = Image.createRenderTarget(Config.getTextureResX(), Config.getTextureResY(), TextureFormat.R8);
		let _texpaint_blend1 = rts.get("texpaint_blend1").image;
		Base.notifyOnNextFrame(() => {
			Image.unload(_texpaint_blend1);
		});
		rts.get("texpaint_blend1").width = Config.getTextureResX();
		rts.get("texpaint_blend1").height = Config.getTextureResY();
		rts.get("texpaint_blend1").image = Image.createRenderTarget(Config.getTextureResX(), Config.getTextureResY(), TextureFormat.R8);
		Context.raw.brushBlendDirty = true;
		if (rts.get("texpaint_blur") != null) {
			let _texpaint_blur = rts.get("texpaint_blur").image;
			Base.notifyOnNextFrame(() => {
				Image.unload(_texpaint_blur);
			});
			let sizeX = Math.floor(Config.getTextureResX() * 0.95);
			let sizeY = Math.floor(Config.getTextureResY() * 0.95);
			rts.get("texpaint_blur").width = sizeX;
			rts.get("texpaint_blur").height = sizeY;
			rts.get("texpaint_blur").image = Image.createRenderTarget(sizeX, sizeY);
		}
		if (RenderPathPaint.liveLayer != null) SlotLayer.resizeAndSetBits(RenderPathPaint.liveLayer);
		///if (krom_direct3d12 || krom_vulkan || krom_metal)
		RenderPathRaytrace.ready = false; // Rebuild baketex
		///end
		Context.raw.ddirty = 2;
	}

	static setLayerBits = () => {
		for (let l of Project.layers) SlotLayer.resizeAndSetBits(l);
		for (let l of History.undoLayers) SlotLayer.resizeAndSetBits(l);
	}

	static makeMergePipe = (red: bool, green: bool, blue: bool, alpha: bool): PipelineStateRaw => {
		let pipe = PipelineState.create();
		pipe.vertexShader = System.getShader("pass.vert");
		pipe.fragmentShader = System.getShader("layer_merge.frag");
		let vs = VertexStructure.create();
		VertexStructure.add(vs, "pos", VertexData.F32_2X);
		pipe.inputLayout = [vs];
		pipe.colorWriteMasksRed = [red];
		pipe.colorWriteMasksGreen = [green];
		pipe.colorWriteMasksBlue = [blue];
		pipe.colorWriteMasksAlpha = [alpha];
		PipelineState.compile(pipe);
		return pipe;
	}
	///end

	static makePipe = () => {
		///if (is_paint || is_sculpt)
		Base.pipeMerge = Base.makeMergePipe(true, true, true, true);
		Base.pipeMergeR = Base.makeMergePipe(true, false, false, false);
		Base.pipeMergeG = Base.makeMergePipe(false, true, false, false);
		Base.pipeMergeB = Base.makeMergePipe(false, false, true, false);
		Base.pipeMergeA = Base.makeMergePipe(false, false, false, true);
		Base.tex0 =PipelineState.getTextureUnit( Base.pipeMerge, "tex0"); // Always binding texpaint.a for blending
		Base.tex1 =PipelineState.getTextureUnit( Base.pipeMerge, "tex1");
		Base.texmask =PipelineState.getTextureUnit( Base.pipeMerge, "texmask");
		Base.texa =PipelineState.getTextureUnit( Base.pipeMerge, "texa");
		Base.opac =PipelineState.getConstantLocation( Base.pipeMerge, "opac");
		Base.blending =PipelineState.getConstantLocation( Base.pipeMerge, "blending");
		///end

		{
			Base.pipeCopy = PipelineState.create();
			Base.pipeCopy.vertexShader = System.getShader("layer_view.vert");
			Base.pipeCopy.fragmentShader = System.getShader("layer_copy.frag");
			let vs = VertexStructure.create();
			VertexStructure.add(vs, "pos", VertexData.F32_3X);
			VertexStructure.add(vs, "tex", VertexData.F32_2X);
			VertexStructure.add(vs, "col", VertexData.U8_4X_Normalized);
			Base.pipeCopy.inputLayout = [vs];
			PipelineState.compile(Base.pipeCopy);
		}

		{
			Base.pipeCopyBGRA = PipelineState.create();
			Base.pipeCopyBGRA.vertexShader = System.getShader("layer_view.vert");
			Base.pipeCopyBGRA.fragmentShader = System.getShader("layer_copy_bgra.frag");
			let vs = VertexStructure.create();
			VertexStructure.add(vs, "pos", VertexData.F32_3X);
			VertexStructure.add(vs, "tex", VertexData.F32_2X);
			VertexStructure.add(vs, "col", VertexData.U8_4X_Normalized);
			Base.pipeCopyBGRA.inputLayout = [vs];
			PipelineState.compile(Base.pipeCopyBGRA);
		}

		///if (krom_metal || krom_vulkan || krom_direct3d12)
		{
			Base.pipeCopy8 = PipelineState.create();
			Base.pipeCopy8.vertexShader = System.getShader("layer_view.vert");
			Base.pipeCopy8.fragmentShader = System.getShader("layer_copy.frag");
			let vs = VertexStructure.create();
			VertexStructure.add(vs, "pos", VertexData.F32_3X);
			VertexStructure.add(vs, "tex", VertexData.F32_2X);
			VertexStructure.add(vs, "col", VertexData.U8_4X_Normalized);
			Base.pipeCopy8.inputLayout = [vs];
			Base.pipeCopy8.colorAttachmentCount = 1;
			Base.pipeCopy8.colorAttachments[0] = TextureFormat.R8;
			PipelineState.compile(Base.pipeCopy8);
		}

		{
			Base.pipeCopy128 = PipelineState.create();
			Base.pipeCopy128.vertexShader = System.getShader("layer_view.vert");
			Base.pipeCopy128.fragmentShader = System.getShader("layer_copy.frag");
			let vs = VertexStructure.create();
			VertexStructure.add(vs, "pos", VertexData.F32_3X);
			VertexStructure.add(vs, "tex", VertexData.F32_2X);
			VertexStructure.add(vs, "col", VertexData.U8_4X_Normalized);
			Base.pipeCopy128.inputLayout = [vs];
			Base.pipeCopy128.colorAttachmentCount = 1;
			Base.pipeCopy128.colorAttachments[0] = TextureFormat.RGBA128;
			PipelineState.compile(Base.pipeCopy128);
		}
		///else
		Base.pipeCopy8 = Base.pipeCopy;
		Base.pipeCopy128 = Base.pipeCopy;
		///end

		///if (is_paint || is_sculpt)
		{
			Base.pipeInvert8 = PipelineState.create();
			Base.pipeInvert8.vertexShader = System.getShader("layer_view.vert");
			Base.pipeInvert8.fragmentShader = System.getShader("layer_invert.frag");
			let vs = VertexStructure.create();
			VertexStructure.add(vs, "pos", VertexData.F32_3X);
			VertexStructure.add(vs, "tex", VertexData.F32_2X);
			VertexStructure.add(vs, "col", VertexData.U8_4X_Normalized);
			Base.pipeInvert8.inputLayout = [vs];
			Base.pipeInvert8.colorAttachmentCount = 1;
			Base.pipeInvert8.colorAttachments[0] = TextureFormat.R8;
			PipelineState.compile(Base.pipeInvert8);
		}

		{
			Base.pipeApplyMask = PipelineState.create();
			Base.pipeApplyMask.vertexShader = System.getShader("pass.vert");
			Base.pipeApplyMask.fragmentShader = System.getShader("mask_apply.frag");
			let vs = VertexStructure.create();
			VertexStructure.add(vs, "pos", VertexData.F32_2X);
			Base.pipeApplyMask.inputLayout = [vs];
			PipelineState.compile(Base.pipeApplyMask);
			Base.tex0Mask = PipelineState.getTextureUnit(Base.pipeApplyMask, "tex0");
			Base.texaMask = PipelineState.getTextureUnit(Base.pipeApplyMask, "texa");
		}

		{
			Base.pipeMergeMask = PipelineState.create();
			Base.pipeMergeMask.vertexShader = System.getShader("pass.vert");
			Base.pipeMergeMask.fragmentShader = System.getShader("mask_merge.frag");
			let vs = VertexStructure.create();
			VertexStructure.add(vs, "pos", VertexData.F32_2X);
			Base.pipeMergeMask.inputLayout = [vs];
			PipelineState.compile(Base.pipeMergeMask);
			Base.tex0MergeMask = PipelineState.getTextureUnit(Base.pipeMergeMask, "tex0");
			Base.texaMergeMask = PipelineState.getTextureUnit(Base.pipeMergeMask, "texa");
			Base.opacMergeMask = PipelineState.getConstantLocation(Base.pipeMergeMask, "opac");
			Base.blendingMergeMask = PipelineState.getConstantLocation(Base.pipeMergeMask, "blending");
		}

		{
			Base.pipeColorIdToMask = PipelineState.create();
			Base.pipeColorIdToMask.vertexShader = System.getShader("pass.vert");
			Base.pipeColorIdToMask.fragmentShader = System.getShader("mask_colorid.frag");
			let vs = VertexStructure.create();
			VertexStructure.add(vs, "pos", VertexData.F32_2X);
			Base.pipeColorIdToMask.inputLayout = [vs];
			PipelineState.compile(Base.pipeColorIdToMask);
			Base.texpaintColorId = PipelineState.getTextureUnit(Base.pipeColorIdToMask, "texpaint_colorid");
			Base.texColorId = PipelineState.getTextureUnit(Base.pipeColorIdToMask, "texcolorid");
		}
		///end

		///if is_lab
		{
			Base.pipeCopyR = PipelineState.create();
			Base.pipeCopyR.vertexShader = System.getShader("layer_view.vert");
			Base.pipeCopyR.fragmentShader = System.getShader("layer_copy.frag");
			let vs = VertexStructure.create();
			VertexStructure.add(vs, "pos", VertexData.F32_3X);
			VertexStructure.add(vs, "tex", VertexData.F32_2X);
			VertexStructure.add(vs, "col", VertexData.U8_4X_Normalized);
			Base.pipeCopyR.inputLayout = [vs];
			Base.pipeCopyR.colorWriteMasksGreen = [false];
			Base.pipeCopyR.colorWriteMasksBlue = [false];
			Base.pipeCopyR.colorWriteMasksAlpha = [false];
			PipelineState.compile(Base.pipeCopyR);
		}

		{
			Base.pipeCopyG = PipelineState.create();
			Base.pipeCopyG.vertexShader = System.getShader("layer_view.vert");
			Base.pipeCopyG.fragmentShader = System.getShader("layer_copy.frag");
			let vs = VertexStructure.create();
			VertexStructure.add(vs, "pos", VertexData.F32_3X);
			VertexStructure.add(vs, "tex", VertexData.F32_2X);
			VertexStructure.add(vs, "col", VertexData.U8_4X_Normalized);
			Base.pipeCopyG.inputLayout = [vs];
			Base.pipeCopyG.colorWriteMasksRed = [false];
			Base.pipeCopyG.colorWriteMasksBlue = [false];
			Base.pipeCopyG.colorWriteMasksAlpha = [false];
			PipelineState.compile(Base.pipeCopyG);
		}

		{
			Base.pipeCopyB = PipelineState.create();
			Base.pipeCopyB.vertexShader = System.getShader("layer_view.vert");
			Base.pipeCopyB.fragmentShader = System.getShader("layer_copy.frag");
			let vs = VertexStructure.create();
			VertexStructure.add(vs, "pos", VertexData.F32_3X);
			VertexStructure.add(vs, "tex", VertexData.F32_2X);
			VertexStructure.add(vs, "col", VertexData.U8_4X_Normalized);
			Base.pipeCopyB.inputLayout = [vs];
			Base.pipeCopyB.colorWriteMasksRed = [false];
			Base.pipeCopyB.colorWriteMasksGreen = [false];
			Base.pipeCopyB.colorWriteMasksAlpha = [false];
			PipelineState.compile(Base.pipeCopyB);
		}

		{
			Base.pipeInpaintPreview = PipelineState.create();
			Base.pipeInpaintPreview.vertexShader = System.getShader("pass.vert");
			Base.pipeInpaintPreview.fragmentShader = System.getShader("inpaint_preview.frag");
			let vs = VertexStructure.create();
			VertexStructure.add(vs, "pos", VertexData.F32_2X);
			Base.pipeInpaintPreview.inputLayout = [vs];
			PipelineState.compile(Base.pipeInpaintPreview);
			Base.tex0InpaintPreview = PipelineState.getTextureUnit(Base.pipeInpaintPreview, "tex0");
			Base.texaInpaintPreview = PipelineState.getTextureUnit(Base.pipeInpaintPreview, "texa");
		}
		///end
	}

	static makePipeCopyRGB = () => {
		Base.pipeCopyRGB = PipelineState.create();
		Base.pipeCopyRGB.vertexShader = System.getShader("layer_view.vert");
		Base.pipeCopyRGB.fragmentShader = System.getShader("layer_copy.frag");
		let vs = VertexStructure.create();
		VertexStructure.add(vs, "pos", VertexData.F32_3X);
		VertexStructure.add(vs, "tex", VertexData.F32_2X);
		VertexStructure.add(vs, "col", VertexData.U8_4X_Normalized);
		Base.pipeCopyRGB.inputLayout = [vs];
		Base.pipeCopyRGB.colorWriteMasksAlpha = [false];
		PipelineState.compile(Base.pipeCopyRGB);
	}

	///if is_lab
	static makePipeCopyA = () => {
		Base.pipeCopyA = PipelineState.create();
		Base.pipeCopyA.vertexShader = System.getShader("pass.vert");
		Base.pipeCopyA.fragmentShader = System.getShader("layer_copy_rrrr.frag");
		let vs = VertexStructure.create();
		VertexStructure.add(vs, "pos", VertexData.F32_2X);
		Base.pipeCopyA.inputLayout = [vs];
		Base.pipeCopyA.colorWriteMasksRed = [false];
		Base.pipeCopyA.colorWriteMasksGreen = [false];
		Base.pipeCopyA.colorWriteMasksBlue = [false];
		PipelineState.compile(Base.pipeCopyA);
		Base.pipeCopyATex = PipelineState.getTextureUnit(Base.pipeCopyA, "tex");
	}
	///end

	static makeCursorPipe = () => {
		Base.pipeCursor = PipelineState.create();
		Base.pipeCursor.vertexShader = System.getShader("cursor.vert");
		Base.pipeCursor.fragmentShader = System.getShader("cursor.frag");
		let vs = VertexStructure.create();
		///if (krom_metal || krom_vulkan)
		VertexStructure.add(vs, "tex", VertexData.I16_2X_Normalized);
		///else
		VertexStructure.add(vs, "pos", VertexData.I16_4X_Normalized);
		VertexStructure.add(vs, "nor", VertexData.I16_2X_Normalized);
		VertexStructure.add(vs, "tex", VertexData.I16_2X_Normalized);
		///end
		Base.pipeCursor.inputLayout = [vs];
		Base.pipeCursor.blendSource = BlendingFactor.SourceAlpha;
		Base.pipeCursor.blendDestination = BlendingFactor.InverseSourceAlpha;
		Base.pipeCursor.depthWrite = false;
		Base.pipeCursor.depthMode = CompareMode.Always;
		PipelineState.compile(Base.pipeCursor);
		Base.cursorVP = PipelineState.getConstantLocation(Base.pipeCursor, "VP");
		Base.cursorInvVP = PipelineState.getConstantLocation(Base.pipeCursor, "invVP");
		Base.cursorMouse = PipelineState.getConstantLocation(Base.pipeCursor, "mouse");
		Base.cursorTexStep = PipelineState.getConstantLocation(Base.pipeCursor, "texStep");
		Base.cursorRadius = PipelineState.getConstantLocation(Base.pipeCursor, "radius");
		Base.cursorCameraRight = PipelineState.getConstantLocation(Base.pipeCursor, "cameraRight");
		Base.cursorTint = PipelineState.getConstantLocation(Base.pipeCursor, "tint");
		Base.cursorGbufferD = PipelineState.getTextureUnit(Base.pipeCursor, "gbufferD");
		Base.cursorTex = PipelineState.getTextureUnit(Base.pipeCursor, "tex");
	}

	static makeTempImg = () => {
		///if (is_paint || is_sculpt)
		let l = Project.layers[0];
		///end
		///if is_lab
		let l = BrushOutputNode.inst;
		///end

		if (Base.tempImage != null && (Base.tempImage.width != l.texpaint.width || Base.tempImage.height != l.texpaint.height || Base.tempImage.format != l.texpaint.format)) {
			let _temptex0 = RenderPath.renderTargets.get("temptex0");
			Base.notifyOnNextFrame(() => {
				RenderTarget.unload(_temptex0);
			});
			RenderPath.renderTargets.delete("temptex0");
			Base.tempImage = null;
		}
		if (Base.tempImage == null) {
			///if (is_paint || is_sculpt)
			let format = Base.bitsHandle.position == TextureBits.Bits8  ? "RGBA32" :
					 	 Base.bitsHandle.position == TextureBits.Bits16 ? "RGBA64" :
					 										  			  "RGBA128";
			///end
			///if is_lab
			let format = "RGBA32";
			///end

			let t = RenderTarget.create();
			t.name = "temptex0";
			t.width = l.texpaint.width;
			t.height = l.texpaint.height;
			t.format = format;
			let rt = RenderPath.createRenderTarget(t);
			Base.tempImage = rt.image;
		}
	}

	///if (is_paint || is_sculpt)
	static makeTempMaskImg = () => {
		if (Base.tempMaskImage != null && (Base.tempMaskImage.width != Config.getTextureResX() || Base.tempMaskImage.height != Config.getTextureResY())) {
			let _tempMaskImage = Base.tempMaskImage;
			Base.notifyOnNextFrame(() => {
				Image.unload(_tempMaskImage);
			});
			Base.tempMaskImage = null;
		}
		if (Base.tempMaskImage == null) {
			Base.tempMaskImage = Image.createRenderTarget(Config.getTextureResX(), Config.getTextureResY(), TextureFormat.R8);
		}
	}
	///end

	static makeExportImg = () => {
		///if (is_paint || is_sculpt)
		let l = Project.layers[0];
		///end
		///if is_lab
		let l = BrushOutputNode.inst;
		///end

		if (Base.expa != null && (Base.expa.width != l.texpaint.width || Base.expa.height != l.texpaint.height || Base.expa.format != l.texpaint.format)) {
			let _expa = Base.expa;
			let _expb = Base.expb;
			let _expc = Base.expc;
			Base.notifyOnNextFrame(() => {
				Image.unload(_expa);
				Image.unload(_expb);
				Image.unload(_expc);
			});
			Base.expa = null;
			Base.expb = null;
			Base.expc = null;
			RenderPath.renderTargets.delete("expa");
			RenderPath.renderTargets.delete("expb");
			RenderPath.renderTargets.delete("expc");
		}
		if (Base.expa == null) {
			///if (is_paint || is_sculpt)
			let format = Base.bitsHandle.position == TextureBits.Bits8  ? "RGBA32" :
					 	 Base.bitsHandle.position == TextureBits.Bits16 ? "RGBA64" :
					 										  			  "RGBA128";
			///end
			///if is_lab
			let format = "RGBA32";
			///end

			{
				let t = RenderTarget.create();
				t.name = "expa";
				t.width = l.texpaint.width;
				t.height = l.texpaint.height;
				t.format = format;
				let rt = RenderPath.createRenderTarget(t);
				Base.expa = rt.image;
			}

			{
				let t = RenderTarget.create();
				t.name = "expb";
				t.width = l.texpaint.width;
				t.height = l.texpaint.height;
				t.format = format;
				let rt = RenderPath.createRenderTarget(t);
				Base.expb = rt.image;
			}

			{
				let t = RenderTarget.create();
				t.name = "expc";
				t.width = l.texpaint.width;
				t.height = l.texpaint.height;
				t.format = format;
				let rt = RenderPath.createRenderTarget(t);
				Base.expc = rt.image;
			}
		}
	}

	///if (is_paint || is_sculpt)
	static duplicateLayer = (l: SlotLayerRaw) => {
		if (!SlotLayer.isGroup(l)) {
			let newLayer = SlotLayer.duplicate(l);
			Context.setLayer(newLayer);
			let masks = SlotLayer.getMasks(l, false);
			if (masks != null) {
				for (let m of masks) {
					m = SlotLayer.duplicate(m);
					m.parent = newLayer;
					array_remove(Project.layers, m);
					Project.layers.splice(Project.layers.indexOf(newLayer), 0, m);
				}
			}
			Context.setLayer(newLayer);
		}
		else {
			let newGroup = Base.newGroup();
			array_remove(Project.layers, newGroup);
			Project.layers.splice(Project.layers.indexOf(l) + 1, 0, newGroup);
			// group.show_panel = true;
			for (let c of SlotLayer.getChildren(l)) {
				let masks = SlotLayer.getMasks(c, false);
				let newLayer = SlotLayer.duplicate(c);
				newLayer.parent = newGroup;
				array_remove(Project.layers, newLayer);
				Project.layers.splice(Project.layers.indexOf(newGroup), 0, newLayer);
				if (masks != null) {
					for (let m of masks) {
						let newMask = SlotLayer.duplicate(m);
						newMask.parent = newLayer;
						array_remove(Project.layers, newMask);
						Project.layers.splice(Project.layers.indexOf(newLayer), 0, newMask);
					}
				}
			}
			let groupMasks = SlotLayer.getMasks(l);
			if (groupMasks != null) {
				for (let m of groupMasks) {
					let newMask = SlotLayer.duplicate(m);
					newMask.parent = newGroup;
					array_remove(Project.layers, newMask);
					Project.layers.splice(Project.layers.indexOf(newGroup), 0, newMask);
				}
			}
			Context.setLayer(newGroup);
		}
	}

	static applyMasks = (l: SlotLayerRaw) => {
		let masks = SlotLayer.getMasks(l);

		if (masks != null) {
			for (let i = 0; i < masks.length - 1; ++i) {
				Base.mergeLayer(masks[i + 1], masks[i]);
				SlotLayer.delete(masks[i]);
			}
			SlotLayer.applyMask(masks[masks.length - 1]);
			Context.raw.layerPreviewDirty = true;
		}
	}

	static mergeDown = () => {
		let l1 = Context.raw.layer;

		if (SlotLayer.isGroup(l1)) {
			l1 = Base.mergeGroup(l1);
		}
		else if (SlotLayer.hasMasks(l1)) { // It is a layer
			Base.applyMasks(l1);
			Context.setLayer(l1);
		}

		let l0 = Project.layers[Project.layers.indexOf(l1) - 1];

		if (SlotLayer.isGroup(l0)) {
			l0 = Base.mergeGroup(l0);
		}
		else if (SlotLayer.hasMasks(l0)) { // It is a layer
			Base.applyMasks(l0);
			Context.setLayer(l0);
		}

		Base.mergeLayer(l0, l1);
		SlotLayer.delete(l1);
		Context.setLayer(l0);
		Context.raw.layerPreviewDirty = true;
	}

	static mergeGroup = (l: SlotLayerRaw) => {
		if (!SlotLayer.isGroup(l)) return null;

		let children = SlotLayer.getChildren(l);

		if (children.length == 1 && SlotLayer.hasMasks(children[0], false)) {
			Base.applyMasks(children[0]);
		}

		for (let i = 0; i < children.length - 1; ++i) {
			Context.setLayer(children[children.length - 1 - i]);
			History.mergeLayers();
			Base.mergeDown();
		}

		// Now apply the group masks
		let masks = SlotLayer.getMasks(l);
		if (masks != null) {
			for (let i = 0; i < masks.length - 1; ++i) {
				Base.mergeLayer(masks[i + 1], masks[i]);
				SlotLayer.delete(masks[i]);
			}
			Base.applyMask(children[0], masks[masks.length - 1]);
		}

		children[0].parent = null;
		children[0].name = l.name;
		if (children[0].fill_layer != null) SlotLayer.toPaintLayer(children[0]);
		SlotLayer.delete(l);
		return children[0];
	}

	static mergeLayer = (l0 : SlotLayerRaw, l1: SlotLayerRaw, use_mask = false) => {
		if (!l1.visible || SlotLayer.isGroup(l1)) return;

		if (Base.pipeMerge == null) Base.makePipe();
		Base.makeTempImg();
		if (ConstData.screenAlignedVB == null) ConstData.createScreenAlignedData();

		Graphics2.begin(Base.tempImage.g2, false); // Copy to temp
		Base.tempImage.g2.pipeline = Base.pipeCopy;
		Graphics2.drawImage(l0.texpaint, 0, 0);
		Base.tempImage.g2.pipeline = null;
		Graphics2.end(Base.tempImage.g2);

		let empty = RenderPath.renderTargets.get("empty_white").image;
		let mask = empty;
		let l1masks =  use_mask ? SlotLayer.getMasks(l1) : null;
		if (l1masks != null) {
			// for (let i = 1; i < l1masks.length - 1; ++i) {
			// 	mergeLayer(l1masks[i + 1], l1masks[i]);
			// }
			mask = l1masks[0].texpaint;
		}

		if (SlotLayer.isMask(l1)) {
			Graphics4.begin(l0.texpaint.g4);
			Graphics4.setPipeline(Base.pipeMergeMask);
			Graphics4.setTexture(Base.tex0MergeMask, l1.texpaint);
			Graphics4.setTexture(Base.texaMergeMask, Base.tempImage);
			Graphics4.setFloat(Base.opacMergeMask, SlotLayer.getOpacity(l1));
			Graphics4.setInt(Base.blendingMergeMask, l1.blending);
			Graphics4.setVertexBuffer(ConstData.screenAlignedVB);
			Graphics4.setIndexBuffer(ConstData.screenAlignedIB);
			Graphics4.drawIndexedVertices();
			Graphics4.end();
		}

		if (SlotLayer.isLayer(l1)) {
			if (l1.paintBase) {
				Graphics4.begin(l0.texpaint.g4);
				Graphics4.setPipeline(Base.pipeMerge);
				Graphics4.setTexture(Base.tex0, l1.texpaint);
				Graphics4.setTexture(Base.tex1, empty);
				Graphics4.setTexture(Base.texmask, mask);
				Graphics4.setTexture(Base.texa, Base.tempImage);
				Graphics4.setFloat(Base.opac, SlotLayer.getOpacity(l1));
				Graphics4.setInt(Base.blending, l1.blending);
				Graphics4.setVertexBuffer(ConstData.screenAlignedVB);
				Graphics4.setIndexBuffer(ConstData.screenAlignedIB);
				Graphics4.drawIndexedVertices();
				Graphics4.end();
			}

			///if is_paint
			Graphics2.begin(Base.tempImage.g2, false);
			Base.tempImage.g2.pipeline = Base.pipeCopy;
			Graphics2.drawImage(l0.texpaint_nor, 0, 0);
			Base.tempImage.g2.pipeline = null;
			Graphics2.end(Base.tempImage.g2);

			if (l1.paintNor) {
				Graphics4.begin(l0.texpaint_nor.g4);
				Graphics4.setPipeline(Base.pipeMerge);
				Graphics4.setTexture(Base.tex0, l1.texpaint);
				Graphics4.setTexture(Base.tex1, l1.texpaint_nor);
				Graphics4.setTexture(Base.texmask, mask);
				Graphics4.setTexture(Base.texa, Base.tempImage);
				Graphics4.setFloat(Base.opac, SlotLayer.getOpacity(l1));
				Graphics4.setInt(Base.blending, l1.paintNorBlend ? -2 : -1);
				Graphics4.setVertexBuffer(ConstData.screenAlignedVB);
				Graphics4.setIndexBuffer(ConstData.screenAlignedIB);
				Graphics4.drawIndexedVertices();
				Graphics4.end();
			}

			Graphics2.begin(Base.tempImage.g2, false);
			Base.tempImage.g2.pipeline = Base.pipeCopy;
			Graphics2.drawImage(l0.texpaint_pack, 0, 0);
			Base.tempImage.g2.pipeline = null;
			Graphics2.end(Base.tempImage.g2);

			if (l1.paintOcc || l1.paintRough || l1.paintMet || l1.paintHeight) {
				if (l1.paintOcc && l1.paintRough && l1.paintMet && l1.paintHeight) {
					Base.commandsMergePack(Base.pipeMerge, l0.texpaint_pack, l1.texpaint, l1.texpaint_pack, SlotLayer.getOpacity(l1), mask, l1.paintHeightBlend ? -3 : -1);
				}
				else {
					if (l1.paintOcc) Base.commandsMergePack(Base.pipeMergeR, l0.texpaint_pack, l1.texpaint, l1.texpaint_pack, SlotLayer.getOpacity(l1), mask);
					if (l1.paintRough) Base.commandsMergePack(Base.pipeMergeG, l0.texpaint_pack, l1.texpaint, l1.texpaint_pack, SlotLayer.getOpacity(l1), mask);
					if (l1.paintMet) Base.commandsMergePack(Base.pipeMergeB, l0.texpaint_pack, l1.texpaint, l1.texpaint_pack, SlotLayer.getOpacity(l1), mask);
				}
			}
			///end
		}
	}

	static flatten = (heightToNormal = false, layers: SlotLayerRaw[] = null): any => {
		if (layers == null) layers = Project.layers;
		Base.makeTempImg();
		Base.makeExportImg();
		if (Base.pipeMerge == null) Base.makePipe();
		if (ConstData.screenAlignedVB == null) ConstData.createScreenAlignedData();
		let empty = RenderPath.renderTargets.get("empty_white").image;

		// Clear export layer
		Graphics4.begin(Base.expa.g4);
		Graphics4.clear(color_from_floats(0.0, 0.0, 0.0, 0.0));
		Graphics4.end();
		Graphics4.begin(Base.expb.g4);
		Graphics4.clear(color_from_floats(0.5, 0.5, 1.0, 0.0));
		Graphics4.end();
		Graphics4.begin(Base.expc.g4);
		Graphics4.clear(color_from_floats(1.0, 0.0, 0.0, 0.0));
		Graphics4.end();

		// Flatten layers
		for (let l1 of layers) {
			if (!SlotLayer.isVisible(l1)) continue;
			if (!SlotLayer.isLayer(l1)) continue;

			let mask = empty;
			let l1masks = SlotLayer.getMasks(l1);
			if (l1masks != null) {
				if (l1masks.length > 1) {
					Base.makeTempMaskImg();
					Graphics2.begin(Base.tempMaskImage.g2, true, 0x00000000);
					Graphics2.end(Base.tempMaskImage.g2);
					let l1: any = { texpaint: Base.tempMaskImage };
					for (let i = 0; i < l1masks.length; ++i) {
						Base.mergeLayer(l1, l1masks[i]);
					}
					mask = Base.tempMaskImage;
				}
				else mask = l1masks[0].texpaint;
			}

			if (l1.paintBase) {
				Graphics2.begin(Base.tempImage.g2, false); // Copy to temp
				Base.tempImage.g2.pipeline = Base.pipeCopy;
				Graphics2.drawImage(Base.expa, 0, 0);
				Base.tempImage.g2.pipeline = null;
				Graphics2.end(Base.tempImage.g2);

				Graphics4.begin(Base.expa.g4);
				Graphics4.setPipeline(Base.pipeMerge);
				Graphics4.setTexture(Base.tex0, l1.texpaint);
				Graphics4.setTexture(Base.tex1, empty);
				Graphics4.setTexture(Base.texmask, mask);
				Graphics4.setTexture(Base.texa, Base.tempImage);
				Graphics4.setFloat(Base.opac, SlotLayer.getOpacity(l1));
				Graphics4.setInt(Base.blending, layers.length > 1 ? l1.blending : 0);
				Graphics4.setVertexBuffer(ConstData.screenAlignedVB);
				Graphics4.setIndexBuffer(ConstData.screenAlignedIB);
				Graphics4.drawIndexedVertices();
				Graphics4.end();
			}

			///if is_paint
			if (l1.paintNor) {
				Graphics2.begin(Base.tempImage.g2, false);
				Base.tempImage.g2.pipeline = Base.pipeCopy;
				Graphics2.drawImage(Base.expb, 0, 0);
				Base.tempImage.g2.pipeline = null;
				Graphics2.end(Base.tempImage.g2);

				Graphics4.begin(Base.expb.g4);
				Graphics4.setPipeline(Base.pipeMerge);
				Graphics4.setTexture(Base.tex0, l1.texpaint);
				Graphics4.setTexture(Base.tex1, l1.texpaint_nor);
				Graphics4.setTexture(Base.texmask, mask);
				Graphics4.setTexture(Base.texa, Base.tempImage);
				Graphics4.setFloat(Base.opac, SlotLayer.getOpacity(l1));
				Graphics4.setInt(Base.blending, l1.paintNorBlend ? -2 : -1);
				Graphics4.setVertexBuffer(ConstData.screenAlignedVB);
				Graphics4.setIndexBuffer(ConstData.screenAlignedIB);
				Graphics4.drawIndexedVertices();
				Graphics4.end();
			}

			if (l1.paintOcc || l1.paintRough || l1.paintMet || l1.paintHeight) {
				Graphics2.begin(Base.tempImage.g2, false);
				Base.tempImage.g2.pipeline = Base.pipeCopy;
				Graphics2.drawImage(Base.expc, 0, 0);
				Base.tempImage.g2.pipeline = null;
				Graphics2.end(Base.tempImage.g2);

				if (l1.paintOcc && l1.paintRough && l1.paintMet && l1.paintHeight) {
					Base.commandsMergePack(Base.pipeMerge, Base.expc, l1.texpaint, l1.texpaint_pack, SlotLayer.getOpacity(l1), mask, l1.paintHeightBlend ? -3 : -1);
				}
				else {
					if (l1.paintOcc) Base.commandsMergePack(Base.pipeMergeR, Base.expc, l1.texpaint, l1.texpaint_pack, SlotLayer.getOpacity(l1), mask);
					if (l1.paintRough) Base.commandsMergePack(Base.pipeMergeG, Base.expc, l1.texpaint, l1.texpaint_pack, SlotLayer.getOpacity(l1), mask);
					if (l1.paintMet) Base.commandsMergePack(Base.pipeMergeB, Base.expc, l1.texpaint, l1.texpaint_pack, SlotLayer.getOpacity(l1), mask);
				}
			}
			///end
		}

		///if krom_metal
		// Flush command list
		Graphics2.begin(Base.expa.g2, false);
		Graphics2.end(Base.expa.g2);
		Graphics2.begin(Base.expb.g2, false);
		Graphics2.end(Base.expb.g2);
		Graphics2.begin(Base.expc.g2, false);
		Graphics2.end(Base.expc.g2);
		///end

		let l0 = { texpaint: Base.expa, texpaint_nor: Base.expb, texpaint_pack: Base.expc };

		// Merge height map into normal map
		if (heightToNormal && MakeMaterial.heightUsed) {

			Graphics2.begin(Base.tempImage.g2, false);
			Base.tempImage.g2.pipeline = Base.pipeCopy;
			Graphics2.drawImage(l0.texpaint_nor, 0, 0);
			Base.tempImage.g2.pipeline = null;
			Graphics2.end(Base.tempImage.g2);

			Graphics4.begin(l0.texpaint_nor.g4);
			Graphics4.setPipeline(Base.pipeMerge);
			Graphics4.setTexture(Base.tex0, Base.tempImage);
			Graphics4.setTexture(Base.tex1, l0.texpaint_pack);
			Graphics4.setTexture(Base.texmask, empty);
			Graphics4.setTexture(Base.texa, empty);
			Graphics4.setFloat(Base.opac, 1.0);
			Graphics4.setInt(Base.blending, -4);
			Graphics4.setVertexBuffer(ConstData.screenAlignedVB);
			Graphics4.setIndexBuffer(ConstData.screenAlignedIB);
			Graphics4.drawIndexedVertices();
			Graphics4.end();
		}

		return l0;
	}

	static applyMask = (l: SlotLayerRaw, m: SlotLayerRaw) => {
		if (!SlotLayer.isLayer(l) || !SlotLayer.isMask(m)) return;

		if (Base.pipeMerge == null) Base.makePipe();
		Base.makeTempImg();

		// Copy layer to temp
		Graphics2.begin(Base.tempImage.g2, false);
		Base.tempImage.g2.pipeline = Base.pipeCopy;
		Graphics2.drawImage(l.texpaint, 0, 0);
		Base.tempImage.g2.pipeline = null;
		Graphics2.end(Base.tempImage.g2);

		// Apply mask
		if (ConstData.screenAlignedVB == null) ConstData.createScreenAlignedData();
		Graphics4.begin(l.texpaint.g4);
		Graphics4.setPipeline(Base.pipeApplyMask);
		Graphics4.setTexture(Base.tex0Mask, Base.tempImage);
		Graphics4.setTexture(Base.texaMask, m.texpaint);
		Graphics4.setVertexBuffer(ConstData.screenAlignedVB);
		Graphics4.setIndexBuffer(ConstData.screenAlignedIB);
		Graphics4.drawIndexedVertices();
		Graphics4.end();
	}

	static commandsMergePack = (pipe: PipelineStateRaw, i0: ImageRaw, i1: ImageRaw, i1pack: ImageRaw, i1maskOpacity: f32, i1texmask: ImageRaw, i1blending = -1) => {
		Graphics4.begin(i0.g4);
		Graphics4.setPipeline(pipe);
		Graphics4.setTexture(Base.tex0, i1);
		Graphics4.setTexture(Base.tex1, i1pack);
		Graphics4.setTexture(Base.texmask, i1texmask);
		Graphics4.setTexture(Base.texa, Base.tempImage);
		Graphics4.setFloat(Base.opac, i1maskOpacity);
		Graphics4.setInt(Base.blending, i1blending);
		Graphics4.setVertexBuffer(ConstData.screenAlignedVB);
		Graphics4.setIndexBuffer(ConstData.screenAlignedIB);
		Graphics4.drawIndexedVertices();
		Graphics4.end();
	}

	static isFillMaterial = (): bool => {
		///if is_paint
		if (Context.raw.tool == WorkspaceTool.ToolMaterial) return true;
		///end

		let m = Context.raw.material;
		for (let l of Project.layers) if (l.fill_layer == m) return true;
		return false;
	}

	static updateFillLayers = () => {
		let _layer = Context.raw.layer;
		let _tool = Context.raw.tool;
		let _fillType = Context.raw.fillTypeHandle.position;
		let current: Graphics2Raw = null;

		///if is_paint
		if (Context.raw.tool == WorkspaceTool.ToolMaterial) {
			if (RenderPathPaint.liveLayer == null) {
				RenderPathPaint.liveLayer = SlotLayer.create("_live");
			}

			current = Graphics2.current;
			if (current != null) Graphics2.end(current);

			Context.raw.tool = WorkspaceTool.ToolFill;
			Context.raw.fillTypeHandle.position = FillType.FillObject;
			MakeMaterial.parsePaintMaterial(false);
			Context.raw.pdirty = 1;
			RenderPathPaint.useLiveLayer(true);
			RenderPathPaint.commandsPaint(false);
			RenderPathPaint.dilate(true, true);
			RenderPathPaint.useLiveLayer(false);
			Context.raw.tool = _tool;
			Context.raw.fillTypeHandle.position = _fillType;
			Context.raw.pdirty = 0;
			Context.raw.rdirty = 2;

			if (current != null) Graphics2.begin(current, false);
			return;
		}
		///end

		let hasFillLayer = false;
		let hasFillMask = false;
		for (let l of Project.layers) if (SlotLayer.isLayer(l) && l.fill_layer == Context.raw.material) hasFillLayer = true;
		for (let l of Project.layers) if (SlotLayer.isMask(l) && l.fill_layer == Context.raw.material) hasFillMask = true;

		if (hasFillLayer || hasFillMask) {
			current = Graphics2.current;
			if (current != null) Graphics2.end(current);
			Context.raw.pdirty = 1;
			Context.raw.tool = WorkspaceTool.ToolFill;
			Context.raw.fillTypeHandle.position = FillType.FillObject;

			if (hasFillLayer) {
				let first = true;
				for (let l of Project.layers) {
					if (SlotLayer.isLayer(l) && l.fill_layer == Context.raw.material) {
						Context.raw.layer = l;
						if (first) {
							first = false;
							MakeMaterial.parsePaintMaterial(false);
						}
						Base.setObjectMask();
						SlotLayer.clear(l);
						RenderPathPaint.commandsPaint(false);
						RenderPathPaint.dilate(true, true);
					}
				}
			}
			if (hasFillMask) {
				let first = true;
				for (let l of Project.layers) {
					if (SlotLayer.isMask(l) && l.fill_layer == Context.raw.material) {
						Context.raw.layer = l;
						if (first) {
							first = false;
							MakeMaterial.parsePaintMaterial(false);
						}
						Base.setObjectMask();
						SlotLayer.clear(l);
						RenderPathPaint.commandsPaint(false);
						RenderPathPaint.dilate(true, true);
					}
				}
			}

			Context.raw.pdirty = 0;
			Context.raw.ddirty = 2;
			Context.raw.rdirty = 2;
			Context.raw.layersPreviewDirty = true; // Repaint all layer previews as multiple layers might have changed.
			if (current != null) Graphics2.begin(current, false);
			Context.raw.layer = _layer;
			Base.setObjectMask();
			Context.raw.tool = _tool;
			Context.raw.fillTypeHandle.position = _fillType;
			MakeMaterial.parsePaintMaterial(false);
		}
	}

	static updateFillLayer = (parsePaint = true) => {
		let current = Graphics2.current;
		if (current != null) Graphics2.end(current);

		let _tool = Context.raw.tool;
		let _fillType = Context.raw.fillTypeHandle.position;
		Context.raw.tool = WorkspaceTool.ToolFill;
		Context.raw.fillTypeHandle.position = FillType.FillObject;
		Context.raw.pdirty = 1;

		SlotLayer.clear(Context.raw.layer);

		if (parsePaint) MakeMaterial.parsePaintMaterial(false);
		RenderPathPaint.commandsPaint(false);
		RenderPathPaint.dilate(true, true);

		Context.raw.rdirty = 2;
		Context.raw.tool = _tool;
		Context.raw.fillTypeHandle.position = _fillType;
		if (current != null) Graphics2.begin(current, false);
	}

	static setObjectMask = () => {
		///if is_sculpt
		return;
		///end

		let ar = [tr("None")];
		for (let p of Project.paintObjects) ar.push(p.base.name);

		let mask = Context.objectMaskUsed() ? SlotLayer.getObjectMask(Context.raw.layer) : 0;
		if (Context.layerFilterUsed()) mask = Context.raw.layerFilter;
		if (mask > 0) {
			if (Context.raw.mergedObject != null) {
				Context.raw.mergedObject.base.visible = false;
			}
			let o = Project.paintObjects[0];
			for (let p of Project.paintObjects) {
				if (p.base.name == ar[mask]) {
					o = p;
					break;
				}
			}
			Context.selectPaintObject(o);
		}
		else {
			let isAtlas = SlotLayer.getObjectMask(Context.raw.layer) > 0 && SlotLayer.getObjectMask(Context.raw.layer) <= Project.paintObjects.length;
			if (Context.raw.mergedObject == null || isAtlas || Context.raw.mergedObjectIsAtlas) {
				let visibles = isAtlas ? Project.getAtlasObjects(SlotLayer.getObjectMask(Context.raw.layer)) : null;
				UtilMesh.mergeMesh(visibles);
			}
			Context.selectPaintObject(Context.mainObject());
			Context.raw.paintObject.skip_context = "paint";
			Context.raw.mergedObject.base.visible = true;
		}
		UtilUV.dilatemapCached = false;
	}

	static newLayer = (clear = true, position = -1): SlotLayerRaw => {
		if (Project.layers.length > Base.maxLayers) return null;
		let l = SlotLayer.create();
		l.objectMask = Context.raw.layerFilter;
		if (position == -1) {
			if (SlotLayer.isMask(Context.raw.layer)) Context.setLayer(Context.raw.layer.parent);
			Project.layers.splice(Project.layers.indexOf(Context.raw.layer) + 1, 0, l);
		}
		else {
			Project.layers.splice(position, 0, l);
		}

		Context.setLayer(l);
		let li = Project.layers.indexOf(Context.raw.layer);
		if (li > 0) {
			let below = Project.layers[li - 1];
			if (SlotLayer.isLayer(below)) {
				Context.raw.layer.parent = below.parent;
			}
		}
		if (clear) App.notifyOnInit(() => { SlotLayer.clear(l); });
		Context.raw.layerPreviewDirty = true;
		return l;
	}

	static newMask = (clear = true, parent: SlotLayerRaw, position = -1): SlotLayerRaw => {
		if (Project.layers.length > Base.maxLayers) return null;
		let l = SlotLayer.create("", LayerSlotType.SlotMask, parent);
		if (position == -1) position = Project.layers.indexOf(parent);
		Project.layers.splice(position, 0, l);
		Context.setLayer(l);
		if (clear) App.notifyOnInit(() => { SlotLayer.clear(l); });
		Context.raw.layerPreviewDirty = true;
		return l;
	}

	static newGroup = (): SlotLayerRaw => {
		if (Project.layers.length > Base.maxLayers) return null;
		let l = SlotLayer.create("", LayerSlotType.SlotGroup);
		Project.layers.push(l);
		Context.setLayer(l);
		return l;
	}

	static createFillLayer = (uvType = UVType.UVMap, decalMat: TMat4 = null, position = -1) => {
		let _init = () => {
			let l = Base.newLayer(false, position);
			History.newLayer();
			l.uvType = uvType;
			if (decalMat != null) l.decalMat = decalMat;
			l.objectMask = Context.raw.layerFilter;
			History.toFillLayer();
			SlotLayer.toFillLayer(l);
		}
		App.notifyOnInit(_init);
	}

	static createImageMask = (asset: TAsset) => {
		let l = Context.raw.layer;
		if (SlotLayer.isMask(l) || SlotLayer.isGroup(l)) {
			return;
		}

		History.newLayer();
		let m = Base.newMask(false, l);
		SlotLayer.clear(m, 0x00000000, Project.getImage(asset));
		Context.raw.layerPreviewDirty = true;
	}

	static createColorLayer = (baseColor: i32, occlusion = 1.0, roughness = Base.defaultRough, metallic = 0.0, position = -1) => {
		let _init = () => {
			let l = Base.newLayer(false, position);
			History.newLayer();
			l.uvType = UVType.UVMap;
			l.objectMask = Context.raw.layerFilter;
			SlotLayer.clear(l, baseColor, null, occlusion, roughness, metallic);
		}
		App.notifyOnInit(_init);
	}

	static onLayersResized = () => {
		App.notifyOnInit(() => {
			Base.resizeLayers();
			let _layer = Context.raw.layer;
			let _material = Context.raw.material;
			for (let l of Project.layers) {
				if (l.fill_layer != null) {
					Context.raw.layer = l;
					Context.raw.material = l.fill_layer;
					Base.updateFillLayer();
				}
			}
			Context.raw.layer = _layer;
			Context.raw.material = _material;
			MakeMaterial.parsePaintMaterial();
		});
		UtilUV.uvmap = null;
		UtilUV.uvmapCached = false;
		UtilUV.trianglemap = null;
		UtilUV.trianglemapCached = false;
		UtilUV.dilatemapCached = false;
		///if (krom_direct3d12 || krom_vulkan || krom_metal)
		RenderPathRaytrace.ready = false;
		///end
	}
	///end

	///if is_lab
	static flatten = (heightToNormal = false): any => {
		let texpaint = BrushOutputNode.inst.texpaint;
		let texpaint_nor = BrushOutputNode.inst.texpaint_nor;
		let texpaint_pack = BrushOutputNode.inst.texpaint_pack;

		let nodes = UINodes.getNodes();
		let canvas = UINodes.getCanvas(true);
		if (nodes.nodesSelectedId.length > 0) {
			let node = nodes.getNode(canvas.nodes, nodes.nodesSelectedId[0]);
			let brushNode = ParserLogic.getLogicNode(node);
			if (brushNode != null && brushNode.getCachedImage() != null) {
				texpaint = brushNode.getCachedImage();
				texpaint_nor = RenderPath.renderTargets.get("texpaint_nor_empty").image;
				texpaint_pack = RenderPath.renderTargets.get("texpaint_pack_empty").image;
			}
		}

		return { texpaint: texpaint, texpaint_nor: texpaint_nor, texpaint_pack: texpaint_pack };
	}

	static onLayersResized = () => {
		Image.unload(BrushOutputNode.inst.texpaint);
		BrushOutputNode.inst.texpaint = RenderPath.renderTargets.get("texpaint").image = Image.createRenderTarget(Config.getTextureResX(), Config.getTextureResY());
		Image.unload(BrushOutputNode.inst.texpaint_nor);
		BrushOutputNode.inst.texpaint_nor = RenderPath.renderTargets.get("texpaint_nor").image = Image.createRenderTarget(Config.getTextureResX(), Config.getTextureResY());
		Image.unload(BrushOutputNode.inst.texpaint_pack);
		BrushOutputNode.inst.texpaint_pack = RenderPath.renderTargets.get("texpaint_pack").image = Image.createRenderTarget(Config.getTextureResX(), Config.getTextureResY());

		if (InpaintNode.image != null) {
			Image.unload(InpaintNode.image);
			InpaintNode.image = null;
			Image.unload(InpaintNode.mask);
			InpaintNode.mask = null;
			InpaintNode.init();
		}

		if (PhotoToPBRNode.images != null) {
			for (let image of PhotoToPBRNode.images) Image.unload(image);
			PhotoToPBRNode.images = null;
			PhotoToPBRNode.init();
		}

		if (TilingNode.image != null) {
			Image.unload(TilingNode.image);
			TilingNode.image = null;
			TilingNode.init();
		}

		Image.unload(RenderPath.renderTargets.get("texpaint_blend0").image);
		RenderPath.renderTargets.get("texpaint_blend0").image = Image.createRenderTarget(Config.getTextureResX(), Config.getTextureResY(), TextureFormat.R8);
		Image.unload(RenderPath.renderTargets.get("texpaint_blend1").image);
		RenderPath.renderTargets.get("texpaint_blend1").image = Image.createRenderTarget(Config.getTextureResX(), Config.getTextureResY(), TextureFormat.R8);

		if (RenderPath.renderTargets.get("texpaint_node") != null) {
			RenderPath.renderTargets.delete("texpaint_node");
		}
		if (RenderPath.renderTargets.get("texpaint_node_target") != null) {
			RenderPath.renderTargets.delete("texpaint_node_target");
		}

		Base.notifyOnNextFrame(() => {
			Base.initLayers();
		});

		///if (krom_direct3d12 || krom_vulkan || krom_metal)
		RenderPathRaytrace.ready = false;
		///end
	}
	///end

	static defaultKeymap = {
		action_paint: "left",
		action_rotate: "alt+left",
		action_pan: "alt+middle",
		action_zoom: "alt+right",
		rotate_light: "shift+middle",
		rotate_envmap: "ctrl+middle",
		set_clone_source: "alt",
		stencil_transform: "ctrl",
		stencil_hide: "z",
		brush_radius: "f",
		brush_radius_decrease: "[",
		brush_radius_increase: "]",
		brush_ruler: "shift",
		file_new: "ctrl+n",
		file_open: "ctrl+o",
		file_open_recent: "ctrl+shift+o",
		file_save: "ctrl+s",
		file_save_as: "ctrl+shift+s",
		file_reimport_mesh: "ctrl+r",
		file_reimport_textures: "ctrl+shift+r",
		file_import_assets: "ctrl+i",
		file_export_textures: "ctrl+e",
		file_export_textures_as: "ctrl+shift+e",
		edit_undo: "ctrl+z",
		edit_redo: "ctrl+shift+z",
		edit_prefs: "ctrl+k",
		view_reset: "0",
		view_front: "1",
		view_back: "ctrl+1",
		view_right: "3",
		view_left: "ctrl+3",
		view_top: "7",
		view_bottom: "ctrl+7",
		view_camera_type: "5",
		view_orbit_left: "4",
		view_orbit_right: "6",
		view_orbit_up: "8",
		view_orbit_down: "2",
		view_orbit_opposite: "9",
		view_zoom_in: "",
		view_zoom_out: "",
		view_distract_free: "f11",
		viewport_mode: "ctrl+m",
		toggle_node_editor: "tab",
		toggle_2d_view: "shift+tab",
		toggle_browser: "`",
		node_search: "space",
		operator_search: "space",

		///if (is_paint || is_sculpt)
		decal_mask: "ctrl",
		select_material: "shift+number",
		select_layer: "alt+number",
		brush_opacity: "shift+f",
		brush_angle: "alt+f",
		tool_brush: "b",
		tool_eraser: "e",
		tool_fill: "g",
		tool_decal: "d",
		tool_text: "t",
		tool_clone: "l",
		tool_blur: "u",
		tool_smudge: "m",
		tool_particle: "p",
		tool_colorid: "c",
		tool_picker: "v",
		tool_bake: "k",
		tool_gizmo: "",
		tool_material: "",
		swap_brush_eraser: "",
		///end
	};
}
