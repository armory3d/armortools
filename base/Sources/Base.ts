
class Base {

	static uiEnabled = true;
	static isDragging = false;
	static isResizing = false;
	static dragAsset: TAsset = null;
	static dragSwatch: TSwatchColor = null;
	static dragFile: string = null;
	static dragFileIcon: image_t = null;
	static dragTint = 0xffffffff;
	static dragSize = -1;
	static dragRect: TRect = null;
	static dragOffX = 0.0;
	static dragOffY = 0.0;
	static dragStart = 0.0;
	static dropX = 0.0;
	static dropY = 0.0;
	static font: g2_font_t = null;
	static theme: theme_t;
	static colorWheel: image_t;
	static colorWheelGradient: image_t;
	static uiBox: zui_t;
	static uiMenu: zui_t;
	static defaultElementW = 100;
	static defaultElementH = 28;
	static defaultFontSize = 13;
	static resHandle = zui_handle_create();
	static bitsHandle = zui_handle_create();
	static dropPaths: string[] = [];
	static appx = 0;
	static appy = 0;
	static lastWindowWidth = 0;
	static lastWindowHeight = 0;
	///if (is_paint || is_sculpt)
	static dragMaterial: SlotMaterialRaw = null;
	static dragLayer: SlotLayerRaw = null;
	///end

	static pipeCopy: pipeline_t;
	static pipeCopy8: pipeline_t;
	static pipeCopy128: pipeline_t;
	static pipeCopyBGRA: pipeline_t;
	static pipeCopyRGB: pipeline_t = null;
	///if (is_paint || is_sculpt)
	static pipeMerge: pipeline_t = null;
	static pipeMergeR: pipeline_t = null;
	static pipeMergeG: pipeline_t = null;
	static pipeMergeB: pipeline_t = null;
	static pipeMergeA: pipeline_t = null;
	static pipeInvert8: pipeline_t;
	static pipeApplyMask: pipeline_t;
	static pipeMergeMask: pipeline_t;
	static pipeColorIdToMask: pipeline_t;
	static tex0: kinc_tex_unit_t;
	static tex1: kinc_tex_unit_t;
	static texmask: kinc_tex_unit_t;
	static texa: kinc_tex_unit_t;
	static opac: kinc_const_loc_t;
	static blending: kinc_const_loc_t;
	static tex0Mask: kinc_tex_unit_t;
	static texaMask: kinc_tex_unit_t;
	static tex0MergeMask: kinc_tex_unit_t;
	static texaMergeMask: kinc_tex_unit_t;
	static texColorId: kinc_tex_unit_t;
	static texpaintColorId: kinc_tex_unit_t;
	static opacMergeMask: kinc_const_loc_t;
	static blendingMergeMask: kinc_const_loc_t;
	static tempMaskImage: image_t = null;
	///end
	///if is_lab
	static pipeCopyR: pipeline_t;
	static pipeCopyG: pipeline_t;
	static pipeCopyB: pipeline_t;
	static pipeCopyA: pipeline_t;
	static pipeCopyATex: kinc_tex_unit_t;
	static pipeInpaintPreview: pipeline_t;
	static tex0InpaintPreview: kinc_tex_unit_t;
	static texaInpaintPreview: kinc_tex_unit_t;
	///end
	static tempImage: image_t = null;
	static expa: image_t = null;
	static expb: image_t = null;
	static expc: image_t = null;
	static pipeCursor: pipeline_t;
	static cursorVP: kinc_const_loc_t;
	static cursorInvVP: kinc_const_loc_t;
	static cursorMouse: kinc_const_loc_t;
	static cursorTexStep: kinc_const_loc_t;
	static cursorRadius: kinc_const_loc_t;
	static cursorCameraRight: kinc_const_loc_t;
	static cursorTint: kinc_const_loc_t;
	static cursorTex: kinc_tex_unit_t;
	static cursorGbufferD: kinc_tex_unit_t;

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
		Base.lastWindowWidth = sys_width();
		Base.lastWindowHeight = sys_height();

		sys_notify_on_drop_files((dropPath: string) => {
			///if krom_linux
			dropPath = decodeURIComponent(dropPath);
			///end
			dropPath = trim_end(dropPath);
			Base.dropPaths.push(dropPath);
		});

		sys_notify_on_app_state(
			() => { // Foreground
				Context.raw.foregroundEvent = true;
				Context.raw.lastPaintX = -1;
				Context.raw.lastPaintY = -1;
			},
			() => {}, // Resume
			() => {}, // Pause
			() => { // Background
				// Release keys after alt-tab / win-tab
				keyboard_up_listener(key_code_t.ALT);
				keyboard_up_listener(key_code_t.WIN);
			},
			() => { // Shutdown
				///if (krom_android || krom_ios)
				Project.projectSave();
				///end
			}
		);

		krom_set_save_and_quit_callback(Base.saveAndQuitCallback);

		data_get_font("font.ttf", (f: g2_font_t) => {
			data_get_image("color_wheel.k", (imageColorWheel: image_t) => {
				data_get_image("color_wheel_gradient.k", (imageColorWheelGradient: image_t) => {

					Base.font = f;
					Config.loadTheme(Config.raw.theme, false);
					Base.defaultElementW = Base.theme.ELEMENT_W;
					Base.defaultFontSize = Base.theme.FONT_SIZE;
					Translator.loadTranslations(Config.raw.locale);
					UIFiles.filename = tr("untitled");
					///if (krom_android || krom_ios)
					sys_title_set(tr("untitled"));
					///end

					// Baked font for fast startup
					if (Config.raw.locale == "en") {
						Base.font.font_ = krom_g2_font_13(Base.font.blob);
						Base.font.glyphs = _g2_font_glyphs;
					}
					else g2_font_init(Base.font);

					Base.colorWheel = imageColorWheel;
					Base.colorWheelGradient = imageColorWheelGradient;
					zui_set_enum_texts(Base.enumTexts);
					zui_tr = tr;
					Base.uiBox = zui_create({ theme: Base.theme, font: f, scaleFactor: Config.raw.window_scale, color_wheel: Base.colorWheel, black_white_gradient: Base.colorWheelGradient });
					Base.uiMenu = zui_create({ theme: Base.theme, font: f, scaleFactor: Config.raw.window_scale, color_wheel: Base.colorWheel, black_white_gradient: Base.colorWheelGradient });
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
					RandomNode.setSeed(Math.floor(time_time() * 4294967295));
					///end

					app_notify_on_update(Base.update);
					app_notify_on_render_2d(UIView2D.render);
					app_notify_on_update(UIView2D.update);
					///if (is_paint || is_sculpt)
					app_notify_on_render_2d(UIBase.renderCursor);
					///end
					app_notify_on_update(UINodes.update);
					app_notify_on_render_2d(UINodes.render);
					app_notify_on_update(UIBase.update);
					app_notify_on_render_2d(UIBase.render);
					app_notify_on_update(Camera.update);
					app_notify_on_render_2d(Base.render);

					///if (is_paint || is_sculpt)
					Base.appx = UIToolbar.toolbarw;
					///end
					///if is_lab
					Base.appx = 0;
					///end

					Base.appy = UIHeader.headerh;
					if (Config.raw.layout[LayoutSize.LayoutHeader] == 1) Base.appy += UIHeader.headerh;
					let cam = scene_camera;
					cam.data.fov = Math.floor(cam.data.fov * 100) / 100;
					camera_object_build_proj(cam);

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
		else sys_stop();
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
			res = sys_width() - sidebarw - UIToolbar.defaultToolbarW;
		}
		else if (UINodes.show || UIView2D.show) {
			res = sys_width() - Config.raw.layout[LayoutSize.LayoutSidebarW] - Config.raw.layout[LayoutSize.LayoutNodesW] - UIToolbar.toolbarw;
		}
		else if (UIBase.show) {
			res = sys_width() - Config.raw.layout[LayoutSize.LayoutSidebarW] - UIToolbar.toolbarw;
		}
		else { // Distract free
			res = sys_width();
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

		let res = sys_height();

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
			res = sys_width();
		}
		else if (UINodes.show || UIView2D.show) {
			res = sys_width() - Config.raw.layout[LayoutSize.LayoutNodesW];
		}
		else { // Distract free
			res = sys_width();
		}

		return res > 0 ? res : 1; // App was minimized, force render path resize
	}

	static h = (): i32 => {
		let res = sys_height();
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
		if (sys_width() == 0 || sys_height() == 0) return;

		let ratioW = sys_width() / Base.lastWindowWidth;
		Base.lastWindowWidth = sys_width();
		let ratioH = sys_height() / Base.lastWindowHeight;
		Base.lastWindowHeight = sys_height();

		Config.raw.layout[LayoutSize.LayoutNodesW] = Math.floor(Config.raw.layout[LayoutSize.LayoutNodesW] * ratioW);
		///if (is_paint || is_sculpt)
		Config.raw.layout[LayoutSize.LayoutSidebarH0] = Math.floor(Config.raw.layout[LayoutSize.LayoutSidebarH0] * ratioH);
		Config.raw.layout[LayoutSize.LayoutSidebarH1] = sys_height() - Config.raw.layout[LayoutSize.LayoutSidebarH0];
		///end

		Base.resize();

		///if (krom_linux || krom_darwin)
		Base.saveWindowRect();
		///end
	}

	static saveWindowRect = () => {
		///if (krom_windows || krom_linux || krom_darwin)
		Config.raw.window_w = sys_width();
		Config.raw.window_h = sys_height();
		Config.raw.window_x = sys_x();
		Config.raw.window_y = sys_y();
		Config.save();
		///end
	}

	static resize = () => {
		if (sys_width() == 0 || sys_height() == 0) return;

		let cam = scene_camera;
		if (cam.data.ortho != null) {
			cam.data.ortho[2] = -2 * (app_h() / app_w());
			cam.data.ortho[3] =  2 * (app_h() / app_w());
		}
		camera_object_build_proj(cam);

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
				image_unload(_grid);
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
		if (mouse_movement_x != 0 || mouse_movement_y != 0) {
			krom_set_mouse_cursor(0); // Arrow
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
				if (hasDrag && mouse_down()) Base.dragStart += time_real_delta();
				else Base.dragStart = 0;
				hasDrag = false;
			}
			if (mouse_released()) {
				Base.dragStart = 0;
			}
			let moved = Math.abs(mouse_movement_x) > 1 && Math.abs(mouse_movement_y) > 1;
			if ((mouse_released() || moved) && !hasDrag) {
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
			zui_set_touch_scroll(!Base.isDragging);
		}

		if (hasDrag && (mouse_movement_x != 0 || mouse_movement_y != 0)) {
			Base.isDragging = true;
		}
		if (mouse_released() && hasDrag) {
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
					Base.dropX = mouse_x;
					Base.dropY = mouse_y;

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

			krom_set_mouse_cursor(0); // Arrow
			Base.isDragging = false;
		}
		if (Context.raw.colorPickerCallback != null && (mouse_released() || mouse_released("right"))) {
			Context.raw.colorPickerCallback = null;
			Context.selectTool(Context.raw.colorPickerPreviousTool);
		}

		Base.handleDropPaths();

		///if (is_paint || is_sculpt)
		///if krom_windows
		let isPicker = Context.raw.tool == WorkspaceTool.ToolPicker || Context.raw.tool == WorkspaceTool.ToolMaterial;
		let decal = Context.raw.tool == WorkspaceTool.ToolDecal || Context.raw.tool == WorkspaceTool.ToolText;
		zui_set_always_redraw_window(!Context.raw.cacheDraws ||
			UIMenu.show ||
			UIBox.show ||
			Base.isDragging ||
			isPicker ||
			decal ||
			UIView2D.show ||
			!Config.raw.brush_3d ||
			Context.raw.frame < 3);
		///end
		///end

		if (zui_always_redraw_window() && Context.raw.ddirty < 0) Context.raw.ddirty = 0;
	}

	///if (is_paint || is_sculpt)
	static materialDropped = () => {
		// Material drag and dropped onto viewport or layers tab
		if (Context.inViewport()) {
			let uvType = keyboard_down("control") ? UVType.UVProject : UVType.UVMap;
			let decalMat = uvType == UVType.UVProject ? UtilRender.getDecalMat() : null;
			Base.createFillLayer(uvType, decalMat);
		}
		if (Context.inLayers() && TabLayers.canDropNewLayer(Context.raw.dragDestination)) {
			let uvType = keyboard_down("control") ? UVType.UVProject : UVType.UVMap;
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
			let wait = !mouse_moved; // Mouse coords not updated during drag
			///else
			let wait = false;
			///end
			if (!wait) {
				Base.dropX = mouse_x;
				Base.dropY = mouse_y;
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

	static getDragImage = (): image_t => {
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

	static render = () => {
		if (sys_width() == 0 || sys_height() == 0) return;

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
			krom_set_mouse_cursor(1); // Hand
			let img = Base.getDragImage();

			///if (is_paint || is_sculpt)
			let scaleFactor = zui_SCALE(UIBase.ui);
			///end
			///if is_lab
			let scaleFactor = zui_SCALE(Base.uiBox);
			///end

			let size = (Base.dragSize == -1 ? 50 : Base.dragSize) * scaleFactor;
			let ratio = size / img.width;
			let h = img.height * ratio;

			///if (is_lab || krom_direct3d11 || krom_direct3d12 || krom_metal || krom_vulkan)
			let inv = 0;
			///else
			let inv = (Base.dragMaterial != null || (Base.dragLayer != null && Base.dragLayer.fill_layer != null)) ? h : 0;
			///end

			g2_set_color(Base.dragTint);

			///if (is_paint || is_sculpt)
			let bgRect = Base.getDragBackground();
			if (bgRect != null) {
				g2_draw_scaled_sub_image(Res.get("icons.k"), bgRect.x, bgRect.y, bgRect.w, bgRect.h, mouse_x + Base.dragOffX, mouse_y + Base.dragOffY + inv, size, h - inv * 2);
			}
			///end

			Base.dragRect == null ?
				g2_draw_scaled_image(img, mouse_x + Base.dragOffX, mouse_y + Base.dragOffY + inv, size, h - inv * 2) :
				g2_draw_scaled_sub_image(img, Base.dragRect.x, Base.dragRect.y, Base.dragRect.w, Base.dragRect.h, mouse_x + Base.dragOffX, mouse_y + Base.dragOffY + inv, size, h - inv * 2);
			g2_set_color(0xffffffff);
		}

		let usingMenu = UIMenu.show && mouse_y > UIHeader.headerh;
		Base.uiEnabled = !UIBox.show && !usingMenu && !Base.isComboSelected();
		if (UIBox.show) UIBox.render();
		if (UIMenu.show) UIMenu.render();

		// Save last pos for continuos paint
		Context.raw.lastPaintVecX = Context.raw.paintVec.x;
		Context.raw.lastPaintVecY = Context.raw.paintVec.y;

		///if (krom_android || krom_ios)
		// No mouse move events for touch, re-init last paint position on touch start
		if (!mouse_down()) {
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
		let _render = () => {
			app_notify_on_init(() => {
				let _update = () => {
					app_notify_on_init(f);
					app_remove_update(_update);
				}
				app_notify_on_update(_update);
			});
			app_remove_render(_render);
		}
		app_notify_on_render(_render);
	}

	static toggleFullscreen = () => {
		if (sys_mode() == window_mode_t.WINDOWED) {
			///if (krom_windows || krom_linux || krom_darwin)
			Config.raw.window_w = sys_width();
			Config.raw.window_h = sys_height();
			Config.raw.window_x = sys_x();
			Config.raw.window_y = sys_y();
			///end
			sys_mode_set(window_mode_t.FULLSCREEN);
		}
		else {
			sys_mode_set(window_mode_t.WINDOWED);
			sys_resize(Config.raw.window_w, Config.raw.window_h);
			sys_move(Config.raw.window_x, Config.raw.window_y);
		}
	}

	static isScrolling = (): bool => {
		for (let ui of Base.getUIs()) if (ui.is_scrolling) return true;
		return false;
	}

	static isComboSelected = (): bool => {
		for (let ui of Base.getUIs()) if (ui.combo_selected_handle_ptr != null) return true;
		return false;
	}

	static getUIs = (): zui_t[] => {
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
		if (UIBase.ui != null && statush > UIStatus.defaultStatusH * zui_SCALE(UIBase.ui)) {
			UIBase.hwnds[TabArea.TabStatus].redraws = 2;
		}
	}

	static initLayout = () => {
		let show2d = (UINodes != null && UINodes.show) || (UIView2D != null && UIView2D.show);

		let raw = Config.raw;
		raw.layout = [
			///if (is_paint || is_sculpt)
			Math.floor(UIBase.defaultSidebarW * raw.window_scale), // LayoutSidebarW
			Math.floor(sys_height() / 2), // LayoutSidebarH0
			Math.floor(sys_height() / 2), // LayoutSidebarH1
			///end

			///if krom_ios
			show2d ? Math.floor((app_w() + raw.layout[LayoutSize.LayoutNodesW]) * 0.473) : Math.floor(app_w() * 0.473), // LayoutNodesW
			///elseif krom_android
			show2d ? Math.floor((app_w() + raw.layout[LayoutSize.LayoutNodesW]) * 0.473) : Math.floor(app_w() * 0.473),
			///else
			show2d ? Math.floor((app_w() + raw.layout[LayoutSize.LayoutNodesW]) * 0.515) : Math.floor(app_w() * 0.515), // Align with ui header controls
			///end

			Math.floor(app_h() / 2), // LayoutNodesH
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
		let texpaint = render_path_render_targets.get("texpaint").image;
		let texpaint_nor = render_path_render_targets.get("texpaint_nor").image;
		let texpaint_pack = render_path_render_targets.get("texpaint_pack").image;
		g2_begin(texpaint, false);
		g2_draw_scaled_image(Res.get("placeholder.k"), 0, 0, Config.getTextureResX(), Config.getTextureResY()); // Base
		g2_end();
		g4_begin(texpaint_nor);
		g4_clear(color_from_floats(0.5, 0.5, 1.0, 0.0)); // Nor
		g4_end();
		g4_begin(texpaint_pack);
		g4_clear(color_from_floats(1.0, 0.4, 0.0, 0.0)); // Occ, rough, met
		g4_end();
		let texpaint_nor_empty = render_path_render_targets.get("texpaint_nor_empty").image;
		let texpaint_pack_empty = render_path_render_targets.get("texpaint_pack_empty").image;
		g4_begin(texpaint_nor_empty);
		g4_clear(color_from_floats(0.5, 0.5, 1.0, 0.0)); // Nor
		g4_end();
		g4_begin(texpaint_pack_empty);
		g4_clear(color_from_floats(1.0, 0.4, 0.0, 0.0)); // Occ, rough, met
		g4_end();
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
		let rts = render_path_render_targets;
		let _texpaint_blend0 = rts.get("texpaint_blend0").image;
		Base.notifyOnNextFrame(() => {
			image_unload(_texpaint_blend0);
		});
		rts.get("texpaint_blend0").width = Config.getTextureResX();
		rts.get("texpaint_blend0").height = Config.getTextureResY();
		rts.get("texpaint_blend0").image = image_create_render_target(Config.getTextureResX(), Config.getTextureResY(), tex_format_t.R8);
		let _texpaint_blend1 = rts.get("texpaint_blend1").image;
		Base.notifyOnNextFrame(() => {
			image_unload(_texpaint_blend1);
		});
		rts.get("texpaint_blend1").width = Config.getTextureResX();
		rts.get("texpaint_blend1").height = Config.getTextureResY();
		rts.get("texpaint_blend1").image = image_create_render_target(Config.getTextureResX(), Config.getTextureResY(), tex_format_t.R8);
		Context.raw.brushBlendDirty = true;
		if (rts.get("texpaint_blur") != null) {
			let _texpaint_blur = rts.get("texpaint_blur").image;
			Base.notifyOnNextFrame(() => {
				image_unload(_texpaint_blur);
			});
			let sizeX = Math.floor(Config.getTextureResX() * 0.95);
			let sizeY = Math.floor(Config.getTextureResY() * 0.95);
			rts.get("texpaint_blur").width = sizeX;
			rts.get("texpaint_blur").height = sizeY;
			rts.get("texpaint_blur").image = image_create_render_target(sizeX, sizeY);
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

	static makeMergePipe = (red: bool, green: bool, blue: bool, alpha: bool): pipeline_t => {
		let pipe = g4_pipeline_create();
		pipe.vertex_shader = sys_get_shader("pass.vert");
		pipe.fragment_shader = sys_get_shader("layer_merge.frag");
		let vs = g4_vertex_struct_create();
		g4_vertex_struct_add(vs, "pos", vertex_data_t.F32_2X);
		pipe.input_layout = [vs];
		pipe.color_write_masks_red = [red];
		pipe.color_write_masks_green = [green];
		pipe.color_write_masks_blue = [blue];
		pipe.color_write_masks_alpha = [alpha];
		g4_pipeline_compile(pipe);
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
		Base.tex0 =g4_pipeline_get_tex_unit( Base.pipeMerge, "tex0"); // Always binding texpaint.a for blending
		Base.tex1 =g4_pipeline_get_tex_unit( Base.pipeMerge, "tex1");
		Base.texmask =g4_pipeline_get_tex_unit( Base.pipeMerge, "texmask");
		Base.texa =g4_pipeline_get_tex_unit( Base.pipeMerge, "texa");
		Base.opac =g4_pipeline_get_const_loc( Base.pipeMerge, "opac");
		Base.blending =g4_pipeline_get_const_loc( Base.pipeMerge, "blending");
		///end

		{
			Base.pipeCopy = g4_pipeline_create();
			Base.pipeCopy.vertex_shader = sys_get_shader("layer_view.vert");
			Base.pipeCopy.fragment_shader = sys_get_shader("layer_copy.frag");
			let vs = g4_vertex_struct_create();
			g4_vertex_struct_add(vs, "pos", vertex_data_t.F32_3X);
			g4_vertex_struct_add(vs, "tex", vertex_data_t.F32_2X);
			g4_vertex_struct_add(vs, "col", vertex_data_t.U8_4X_NORM);
			Base.pipeCopy.input_layout = [vs];
			g4_pipeline_compile(Base.pipeCopy);
		}

		{
			Base.pipeCopyBGRA = g4_pipeline_create();
			Base.pipeCopyBGRA.vertex_shader = sys_get_shader("layer_view.vert");
			Base.pipeCopyBGRA.fragment_shader = sys_get_shader("layer_copy_bgra.frag");
			let vs = g4_vertex_struct_create();
			g4_vertex_struct_add(vs, "pos", vertex_data_t.F32_3X);
			g4_vertex_struct_add(vs, "tex", vertex_data_t.F32_2X);
			g4_vertex_struct_add(vs, "col", vertex_data_t.U8_4X_NORM);
			Base.pipeCopyBGRA.input_layout = [vs];
			g4_pipeline_compile(Base.pipeCopyBGRA);
		}

		///if (krom_metal || krom_vulkan || krom_direct3d12)
		{
			Base.pipeCopy8 = g4_pipeline_create();
			Base.pipeCopy8.vertex_shader = sys_get_shader("layer_view.vert");
			Base.pipeCopy8.fragment_shader = sys_get_shader("layer_copy.frag");
			let vs = g4_vertex_struct_create();
			g4_vertex_struct_add(vs, "pos", vertex_data_t.F32_3X);
			g4_vertex_struct_add(vs, "tex", vertex_data_t.F32_2X);
			g4_vertex_struct_add(vs, "col", vertex_data_t.U8_4X_NORM);
			Base.pipeCopy8.input_layout = [vs];
			Base.pipeCopy8.color_attachment_count = 1;
			Base.pipeCopy8.color_attachments[0] = tex_format_t.R8;
			g4_pipeline_compile(Base.pipeCopy8);
		}

		{
			Base.pipeCopy128 = g4_pipeline_create();
			Base.pipeCopy128.vertex_shader = sys_get_shader("layer_view.vert");
			Base.pipeCopy128.fragment_shader = sys_get_shader("layer_copy.frag");
			let vs = g4_vertex_struct_create();
			g4_vertex_struct_add(vs, "pos", vertex_data_t.F32_3X);
			g4_vertex_struct_add(vs, "tex", vertex_data_t.F32_2X);
			g4_vertex_struct_add(vs, "col", vertex_data_t.U8_4X_NORM);
			Base.pipeCopy128.input_layout = [vs];
			Base.pipeCopy128.color_attachment_count = 1;
			Base.pipeCopy128.color_attachments[0] = tex_format_t.RGBA128;
			g4_pipeline_compile(Base.pipeCopy128);
		}
		///else
		Base.pipeCopy8 = Base.pipeCopy;
		Base.pipeCopy128 = Base.pipeCopy;
		///end

		///if (is_paint || is_sculpt)
		{
			Base.pipeInvert8 = g4_pipeline_create();
			Base.pipeInvert8.vertex_shader = sys_get_shader("layer_view.vert");
			Base.pipeInvert8.fragment_shader = sys_get_shader("layer_invert.frag");
			let vs = g4_vertex_struct_create();
			g4_vertex_struct_add(vs, "pos", vertex_data_t.F32_3X);
			g4_vertex_struct_add(vs, "tex", vertex_data_t.F32_2X);
			g4_vertex_struct_add(vs, "col", vertex_data_t.U8_4X_NORM);
			Base.pipeInvert8.input_layout = [vs];
			Base.pipeInvert8.color_attachment_count = 1;
			Base.pipeInvert8.color_attachments[0] = tex_format_t.R8;
			g4_pipeline_compile(Base.pipeInvert8);
		}

		{
			Base.pipeApplyMask = g4_pipeline_create();
			Base.pipeApplyMask.vertex_shader = sys_get_shader("pass.vert");
			Base.pipeApplyMask.fragment_shader = sys_get_shader("mask_apply.frag");
			let vs = g4_vertex_struct_create();
			g4_vertex_struct_add(vs, "pos", vertex_data_t.F32_2X);
			Base.pipeApplyMask.input_layout = [vs];
			g4_pipeline_compile(Base.pipeApplyMask);
			Base.tex0Mask = g4_pipeline_get_tex_unit(Base.pipeApplyMask, "tex0");
			Base.texaMask = g4_pipeline_get_tex_unit(Base.pipeApplyMask, "texa");
		}

		{
			Base.pipeMergeMask = g4_pipeline_create();
			Base.pipeMergeMask.vertex_shader = sys_get_shader("pass.vert");
			Base.pipeMergeMask.fragment_shader = sys_get_shader("mask_merge.frag");
			let vs = g4_vertex_struct_create();
			g4_vertex_struct_add(vs, "pos", vertex_data_t.F32_2X);
			Base.pipeMergeMask.input_layout = [vs];
			g4_pipeline_compile(Base.pipeMergeMask);
			Base.tex0MergeMask = g4_pipeline_get_tex_unit(Base.pipeMergeMask, "tex0");
			Base.texaMergeMask = g4_pipeline_get_tex_unit(Base.pipeMergeMask, "texa");
			Base.opacMergeMask = g4_pipeline_get_const_loc(Base.pipeMergeMask, "opac");
			Base.blendingMergeMask = g4_pipeline_get_const_loc(Base.pipeMergeMask, "blending");
		}

		{
			Base.pipeColorIdToMask = g4_pipeline_create();
			Base.pipeColorIdToMask.vertex_shader = sys_get_shader("pass.vert");
			Base.pipeColorIdToMask.fragment_shader = sys_get_shader("mask_colorid.frag");
			let vs = g4_vertex_struct_create();
			g4_vertex_struct_add(vs, "pos", vertex_data_t.F32_2X);
			Base.pipeColorIdToMask.input_layout = [vs];
			g4_pipeline_compile(Base.pipeColorIdToMask);
			Base.texpaintColorId = g4_pipeline_get_tex_unit(Base.pipeColorIdToMask, "texpaint_colorid");
			Base.texColorId = g4_pipeline_get_tex_unit(Base.pipeColorIdToMask, "texcolorid");
		}
		///end

		///if is_lab
		{
			Base.pipeCopyR = g4_pipeline_create();
			Base.pipeCopyR.vertex_shader = sys_get_shader("layer_view.vert");
			Base.pipeCopyR.fragment_shader = sys_get_shader("layer_copy.frag");
			let vs = g4_vertex_struct_create();
			g4_vertex_struct_add(vs, "pos", vertex_data_t.F32_3X);
			g4_vertex_struct_add(vs, "tex", vertex_data_t.F32_2X);
			g4_vertex_struct_add(vs, "col", vertex_data_t.U8_4X_NORM);
			Base.pipeCopyR.input_layout = [vs];
			Base.pipeCopyR.color_write_masks_green = [false];
			Base.pipeCopyR.color_write_masks_blue = [false];
			Base.pipeCopyR.color_write_masks_alpha = [false];
			g4_pipeline_compile(Base.pipeCopyR);
		}

		{
			Base.pipeCopyG = g4_pipeline_create();
			Base.pipeCopyG.vertex_shader = sys_get_shader("layer_view.vert");
			Base.pipeCopyG.fragment_shader = sys_get_shader("layer_copy.frag");
			let vs = g4_vertex_struct_create();
			g4_vertex_struct_add(vs, "pos", vertex_data_t.F32_3X);
			g4_vertex_struct_add(vs, "tex", vertex_data_t.F32_2X);
			g4_vertex_struct_add(vs, "col", vertex_data_t.U8_4X_NORM);
			Base.pipeCopyG.input_layout = [vs];
			Base.pipeCopyG.color_write_masks_red = [false];
			Base.pipeCopyG.color_write_masks_blue = [false];
			Base.pipeCopyG.color_write_masks_alpha = [false];
			g4_pipeline_compile(Base.pipeCopyG);
		}

		{
			Base.pipeCopyB = g4_pipeline_create();
			Base.pipeCopyB.vertex_shader = sys_get_shader("layer_view.vert");
			Base.pipeCopyB.fragment_shader = sys_get_shader("layer_copy.frag");
			let vs = g4_vertex_struct_create();
			g4_vertex_struct_add(vs, "pos", vertex_data_t.F32_3X);
			g4_vertex_struct_add(vs, "tex", vertex_data_t.F32_2X);
			g4_vertex_struct_add(vs, "col", vertex_data_t.U8_4X_NORM);
			Base.pipeCopyB.input_layout = [vs];
			Base.pipeCopyB.color_write_masks_red = [false];
			Base.pipeCopyB.color_write_masks_green = [false];
			Base.pipeCopyB.color_write_masks_alpha = [false];
			g4_pipeline_compile(Base.pipeCopyB);
		}

		{
			Base.pipeInpaintPreview = g4_pipeline_create();
			Base.pipeInpaintPreview.vertex_shader = sys_get_shader("pass.vert");
			Base.pipeInpaintPreview.fragment_shader = sys_get_shader("inpaint_preview.frag");
			let vs = g4_vertex_struct_create();
			g4_vertex_struct_add(vs, "pos", vertex_data_t.F32_2X);
			Base.pipeInpaintPreview.input_layout = [vs];
			g4_pipeline_compile(Base.pipeInpaintPreview);
			Base.tex0InpaintPreview = g4_pipeline_get_tex_unit(Base.pipeInpaintPreview, "tex0");
			Base.texaInpaintPreview = g4_pipeline_get_tex_unit(Base.pipeInpaintPreview, "texa");
		}
		///end
	}

	static makePipeCopyRGB = () => {
		Base.pipeCopyRGB = g4_pipeline_create();
		Base.pipeCopyRGB.vertex_shader = sys_get_shader("layer_view.vert");
		Base.pipeCopyRGB.fragment_shader = sys_get_shader("layer_copy.frag");
		let vs = g4_vertex_struct_create();
		g4_vertex_struct_add(vs, "pos", vertex_data_t.F32_3X);
		g4_vertex_struct_add(vs, "tex", vertex_data_t.F32_2X);
		g4_vertex_struct_add(vs, "col", vertex_data_t.U8_4X_NORM);
		Base.pipeCopyRGB.input_layout = [vs];
		Base.pipeCopyRGB.color_write_masks_alpha = [false];
		g4_pipeline_compile(Base.pipeCopyRGB);
	}

	///if is_lab
	static makePipeCopyA = () => {
		Base.pipeCopyA = g4_pipeline_create();
		Base.pipeCopyA.vertex_shader = sys_get_shader("pass.vert");
		Base.pipeCopyA.fragment_shader = sys_get_shader("layer_copy_rrrr.frag");
		let vs = g4_vertex_struct_create();
		g4_vertex_struct_add(vs, "pos", vertex_data_t.F32_2X);
		Base.pipeCopyA.input_layout = [vs];
		Base.pipeCopyA.color_write_masks_red = [false];
		Base.pipeCopyA.color_write_masks_green = [false];
		Base.pipeCopyA.color_write_masks_blue = [false];
		g4_pipeline_compile(Base.pipeCopyA);
		Base.pipeCopyATex = g4_pipeline_get_tex_unit(Base.pipeCopyA, "tex");
	}
	///end

	static makeCursorPipe = () => {
		Base.pipeCursor = g4_pipeline_create();
		Base.pipeCursor.vertex_shader = sys_get_shader("cursor.vert");
		Base.pipeCursor.fragment_shader = sys_get_shader("cursor.frag");
		let vs = g4_vertex_struct_create();
		///if (krom_metal || krom_vulkan)
		g4_vertex_struct_add(vs, "tex", vertex_data_t.I16_2X_NORM);
		///else
		g4_vertex_struct_add(vs, "pos", vertex_data_t.I16_4X_NORM);
		g4_vertex_struct_add(vs, "nor", vertex_data_t.I16_2X_NORM);
		g4_vertex_struct_add(vs, "tex", vertex_data_t.I16_2X_NORM);
		///end
		Base.pipeCursor.input_layout = [vs];
		Base.pipeCursor.blend_source = blend_factor_t.SOURCE_ALPHA;
		Base.pipeCursor.blend_dest = blend_factor_t.INV_SOURCE_ALPHA;
		Base.pipeCursor.depth_write = false;
		Base.pipeCursor.depth_mode = compare_mode_t.ALWAYS;
		g4_pipeline_compile(Base.pipeCursor);
		Base.cursorVP = g4_pipeline_get_const_loc(Base.pipeCursor, "VP");
		Base.cursorInvVP = g4_pipeline_get_const_loc(Base.pipeCursor, "invVP");
		Base.cursorMouse = g4_pipeline_get_const_loc(Base.pipeCursor, "mouse");
		Base.cursorTexStep = g4_pipeline_get_const_loc(Base.pipeCursor, "texStep");
		Base.cursorRadius = g4_pipeline_get_const_loc(Base.pipeCursor, "radius");
		Base.cursorCameraRight = g4_pipeline_get_const_loc(Base.pipeCursor, "cameraRight");
		Base.cursorTint = g4_pipeline_get_const_loc(Base.pipeCursor, "tint");
		Base.cursorGbufferD = g4_pipeline_get_tex_unit(Base.pipeCursor, "gbufferD");
		Base.cursorTex = g4_pipeline_get_tex_unit(Base.pipeCursor, "tex");
	}

	static makeTempImg = () => {
		///if (is_paint || is_sculpt)
		let l = Project.layers[0];
		///end
		///if is_lab
		let l = BrushOutputNode.inst;
		///end

		if (Base.tempImage != null && (Base.tempImage.width != l.texpaint.width || Base.tempImage.height != l.texpaint.height || Base.tempImage.format != l.texpaint.format)) {
			let _temptex0 = render_path_render_targets.get("temptex0");
			Base.notifyOnNextFrame(() => {
				render_target_unload(_temptex0);
			});
			render_path_render_targets.delete("temptex0");
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

			let t = render_target_create();
			t.name = "temptex0";
			t.width = l.texpaint.width;
			t.height = l.texpaint.height;
			t.format = format;
			let rt = render_path_create_render_target(t);
			Base.tempImage = rt.image;
		}
	}

	///if (is_paint || is_sculpt)
	static makeTempMaskImg = () => {
		if (Base.tempMaskImage != null && (Base.tempMaskImage.width != Config.getTextureResX() || Base.tempMaskImage.height != Config.getTextureResY())) {
			let _tempMaskImage = Base.tempMaskImage;
			Base.notifyOnNextFrame(() => {
				image_unload(_tempMaskImage);
			});
			Base.tempMaskImage = null;
		}
		if (Base.tempMaskImage == null) {
			Base.tempMaskImage = image_create_render_target(Config.getTextureResX(), Config.getTextureResY(), tex_format_t.R8);
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
				image_unload(_expa);
				image_unload(_expb);
				image_unload(_expc);
			});
			Base.expa = null;
			Base.expb = null;
			Base.expc = null;
			render_path_render_targets.delete("expa");
			render_path_render_targets.delete("expb");
			render_path_render_targets.delete("expc");
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
				let t = render_target_create();
				t.name = "expa";
				t.width = l.texpaint.width;
				t.height = l.texpaint.height;
				t.format = format;
				let rt = render_path_create_render_target(t);
				Base.expa = rt.image;
			}

			{
				let t = render_target_create();
				t.name = "expb";
				t.width = l.texpaint.width;
				t.height = l.texpaint.height;
				t.format = format;
				let rt = render_path_create_render_target(t);
				Base.expb = rt.image;
			}

			{
				let t = render_target_create();
				t.name = "expc";
				t.width = l.texpaint.width;
				t.height = l.texpaint.height;
				t.format = format;
				let rt = render_path_create_render_target(t);
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
		if (const_data_screen_aligned_vb == null) const_data_create_screen_aligned_data();

		g2_begin(Base.tempImage, false); // Copy to temp
		g2_set_pipeline(Base.pipeCopy);
		g2_draw_image(l0.texpaint, 0, 0);
		g2_set_pipeline(null);
		g2_end();

		let empty = render_path_render_targets.get("empty_white").image;
		let mask = empty;
		let l1masks =  use_mask ? SlotLayer.getMasks(l1) : null;
		if (l1masks != null) {
			// for (let i = 1; i < l1masks.length - 1; ++i) {
			// 	mergeLayer(l1masks[i + 1], l1masks[i]);
			// }
			mask = l1masks[0].texpaint;
		}

		if (SlotLayer.isMask(l1)) {
			g4_begin(l0.texpaint);
			g4_set_pipeline(Base.pipeMergeMask);
			g4_set_tex(Base.tex0MergeMask, l1.texpaint);
			g4_set_tex(Base.texaMergeMask, Base.tempImage);
			g4_set_float(Base.opacMergeMask, SlotLayer.getOpacity(l1));
			g4_set_int(Base.blendingMergeMask, l1.blending);
			g4_set_vertex_buffer(const_data_screen_aligned_vb);
			g4_set_index_buffer(const_data_screen_aligned_ib);
			g4_draw();
			g4_end();
		}

		if (SlotLayer.isLayer(l1)) {
			if (l1.paintBase) {
				g4_begin(l0.texpaint);
				g4_set_pipeline(Base.pipeMerge);
				g4_set_tex(Base.tex0, l1.texpaint);
				g4_set_tex(Base.tex1, empty);
				g4_set_tex(Base.texmask, mask);
				g4_set_tex(Base.texa, Base.tempImage);
				g4_set_float(Base.opac, SlotLayer.getOpacity(l1));
				g4_set_int(Base.blending, l1.blending);
				g4_set_vertex_buffer(const_data_screen_aligned_vb);
				g4_set_index_buffer(const_data_screen_aligned_ib);
				g4_draw();
				g4_end();
			}

			///if is_paint
			g2_begin(Base.tempImage, false);
			g2_set_pipeline(Base.pipeCopy);
			g2_draw_image(l0.texpaint_nor, 0, 0);
			g2_set_pipeline(null);
			g2_end();

			if (l1.paintNor) {
				g4_begin(l0.texpaint_nor);
				g4_set_pipeline(Base.pipeMerge);
				g4_set_tex(Base.tex0, l1.texpaint);
				g4_set_tex(Base.tex1, l1.texpaint_nor);
				g4_set_tex(Base.texmask, mask);
				g4_set_tex(Base.texa, Base.tempImage);
				g4_set_float(Base.opac, SlotLayer.getOpacity(l1));
				g4_set_int(Base.blending, l1.paintNorBlend ? -2 : -1);
				g4_set_vertex_buffer(const_data_screen_aligned_vb);
				g4_set_index_buffer(const_data_screen_aligned_ib);
				g4_draw();
				g4_end();
			}

			g2_begin(Base.tempImage, false);
			g2_set_pipeline(Base.pipeCopy);
			g2_draw_image(l0.texpaint_pack, 0, 0);
			g2_set_pipeline(null);
			g2_end();

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
		if (const_data_screen_aligned_vb == null) const_data_create_screen_aligned_data();
		let empty = render_path_render_targets.get("empty_white").image;

		// Clear export layer
		g4_begin(Base.expa);
		g4_clear(color_from_floats(0.0, 0.0, 0.0, 0.0));
		g4_end();
		g4_begin(Base.expb);
		g4_clear(color_from_floats(0.5, 0.5, 1.0, 0.0));
		g4_end();
		g4_begin(Base.expc);
		g4_clear(color_from_floats(1.0, 0.0, 0.0, 0.0));
		g4_end();

		// Flatten layers
		for (let l1 of layers) {
			if (!SlotLayer.isVisible(l1)) continue;
			if (!SlotLayer.isLayer(l1)) continue;

			let mask = empty;
			let l1masks = SlotLayer.getMasks(l1);
			if (l1masks != null) {
				if (l1masks.length > 1) {
					Base.makeTempMaskImg();
					g2_begin(Base.tempMaskImage, true, 0x00000000);
					g2_end();
					let l1: any = { texpaint: Base.tempMaskImage };
					for (let i = 0; i < l1masks.length; ++i) {
						Base.mergeLayer(l1, l1masks[i]);
					}
					mask = Base.tempMaskImage;
				}
				else mask = l1masks[0].texpaint;
			}

			if (l1.paintBase) {
				g2_begin(Base.tempImage, false); // Copy to temp
				g2_set_pipeline(Base.pipeCopy);
				g2_draw_image(Base.expa, 0, 0);
				g2_set_pipeline(null);
				g2_end();

				g4_begin(Base.expa);
				g4_set_pipeline(Base.pipeMerge);
				g4_set_tex(Base.tex0, l1.texpaint);
				g4_set_tex(Base.tex1, empty);
				g4_set_tex(Base.texmask, mask);
				g4_set_tex(Base.texa, Base.tempImage);
				g4_set_float(Base.opac, SlotLayer.getOpacity(l1));
				g4_set_int(Base.blending, layers.length > 1 ? l1.blending : 0);
				g4_set_vertex_buffer(const_data_screen_aligned_vb);
				g4_set_index_buffer(const_data_screen_aligned_ib);
				g4_draw();
				g4_end();
			}

			///if is_paint
			if (l1.paintNor) {
				g2_begin(Base.tempImage, false);
				g2_set_pipeline(Base.pipeCopy);
				g2_draw_image(Base.expb, 0, 0);
				g2_set_pipeline(null);
				g2_end();

				g4_begin(Base.expb);
				g4_set_pipeline(Base.pipeMerge);
				g4_set_tex(Base.tex0, l1.texpaint);
				g4_set_tex(Base.tex1, l1.texpaint_nor);
				g4_set_tex(Base.texmask, mask);
				g4_set_tex(Base.texa, Base.tempImage);
				g4_set_float(Base.opac, SlotLayer.getOpacity(l1));
				g4_set_int(Base.blending, l1.paintNorBlend ? -2 : -1);
				g4_set_vertex_buffer(const_data_screen_aligned_vb);
				g4_set_index_buffer(const_data_screen_aligned_ib);
				g4_draw();
				g4_end();
			}

			if (l1.paintOcc || l1.paintRough || l1.paintMet || l1.paintHeight) {
				g2_begin(Base.tempImage, false);
				g2_set_pipeline(Base.pipeCopy);
				g2_draw_image(Base.expc, 0, 0);
				g2_set_pipeline(null);
				g2_end();

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
		g2_begin(Base.expa, false);
		g2_end();
		g2_begin(Base.expb, false);
		g2_end();
		g2_begin(Base.expc, false);
		g2_end();
		///end

		let l0 = { texpaint: Base.expa, texpaint_nor: Base.expb, texpaint_pack: Base.expc };

		// Merge height map into normal map
		if (heightToNormal && MakeMaterial.heightUsed) {

			g2_begin(Base.tempImage, false);
			g2_set_pipeline(Base.pipeCopy);
			g2_draw_image(l0.texpaint_nor, 0, 0);
			g2_set_pipeline(null);
			g2_end();

			g4_begin(l0.texpaint_nor);
			g4_set_pipeline(Base.pipeMerge);
			g4_set_tex(Base.tex0, Base.tempImage);
			g4_set_tex(Base.tex1, l0.texpaint_pack);
			g4_set_tex(Base.texmask, empty);
			g4_set_tex(Base.texa, empty);
			g4_set_float(Base.opac, 1.0);
			g4_set_int(Base.blending, -4);
			g4_set_vertex_buffer(const_data_screen_aligned_vb);
			g4_set_index_buffer(const_data_screen_aligned_ib);
			g4_draw();
			g4_end();
		}

		return l0;
	}

	static applyMask = (l: SlotLayerRaw, m: SlotLayerRaw) => {
		if (!SlotLayer.isLayer(l) || !SlotLayer.isMask(m)) return;

		if (Base.pipeMerge == null) Base.makePipe();
		Base.makeTempImg();

		// Copy layer to temp
		g2_begin(Base.tempImage, false);
		g2_set_pipeline(Base.pipeCopy);
		g2_draw_image(l.texpaint, 0, 0);
		g2_set_pipeline(null);
		g2_end();

		// Apply mask
		if (const_data_screen_aligned_vb == null) const_data_create_screen_aligned_data();
		g4_begin(l.texpaint);
		g4_set_pipeline(Base.pipeApplyMask);
		g4_set_tex(Base.tex0Mask, Base.tempImage);
		g4_set_tex(Base.texaMask, m.texpaint);
		g4_set_vertex_buffer(const_data_screen_aligned_vb);
		g4_set_index_buffer(const_data_screen_aligned_ib);
		g4_draw();
		g4_end();
	}

	static commandsMergePack = (pipe: pipeline_t, i0: image_t, i1: image_t, i1pack: image_t, i1maskOpacity: f32, i1texmask: image_t, i1blending = -1) => {
		g4_begin(i0);
		g4_set_pipeline(pipe);
		g4_set_tex(Base.tex0, i1);
		g4_set_tex(Base.tex1, i1pack);
		g4_set_tex(Base.texmask, i1texmask);
		g4_set_tex(Base.texa, Base.tempImage);
		g4_set_float(Base.opac, i1maskOpacity);
		g4_set_int(Base.blending, i1blending);
		g4_set_vertex_buffer(const_data_screen_aligned_vb);
		g4_set_index_buffer(const_data_screen_aligned_ib);
		g4_draw();
		g4_end();
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
		let current: image_t = null;

		///if is_paint
		if (Context.raw.tool == WorkspaceTool.ToolMaterial) {
			if (RenderPathPaint.liveLayer == null) {
				RenderPathPaint.liveLayer = SlotLayer.create("_live");
			}

			current = _g2_current;
			if (current != null) g2_end();

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

			if (current != null) g2_begin(current, false);
			return;
		}
		///end

		let hasFillLayer = false;
		let hasFillMask = false;
		for (let l of Project.layers) if (SlotLayer.isLayer(l) && l.fill_layer == Context.raw.material) hasFillLayer = true;
		for (let l of Project.layers) if (SlotLayer.isMask(l) && l.fill_layer == Context.raw.material) hasFillMask = true;

		if (hasFillLayer || hasFillMask) {
			current = _g2_current;
			if (current != null) g2_end();
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
			if (current != null) g2_begin(current, false);
			Context.raw.layer = _layer;
			Base.setObjectMask();
			Context.raw.tool = _tool;
			Context.raw.fillTypeHandle.position = _fillType;
			MakeMaterial.parsePaintMaterial(false);
		}
	}

	static updateFillLayer = (parsePaint = true) => {
		let current = _g2_current;
		if (current != null) g2_end();

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
		if (current != null) g2_begin(current, false);
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
		if (clear) app_notify_on_init(() => { SlotLayer.clear(l); });
		Context.raw.layerPreviewDirty = true;
		return l;
	}

	static newMask = (clear = true, parent: SlotLayerRaw, position = -1): SlotLayerRaw => {
		if (Project.layers.length > Base.maxLayers) return null;
		let l = SlotLayer.create("", LayerSlotType.SlotMask, parent);
		if (position == -1) position = Project.layers.indexOf(parent);
		Project.layers.splice(position, 0, l);
		Context.setLayer(l);
		if (clear) app_notify_on_init(() => { SlotLayer.clear(l); });
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

	static createFillLayer = (uvType = UVType.UVMap, decalMat: mat4_t = null, position = -1) => {
		let _init = () => {
			let l = Base.newLayer(false, position);
			History.newLayer();
			l.uvType = uvType;
			if (decalMat != null) l.decalMat = decalMat;
			l.objectMask = Context.raw.layerFilter;
			History.toFillLayer();
			SlotLayer.toFillLayer(l);
		}
		app_notify_on_init(_init);
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
		app_notify_on_init(_init);
	}

	static onLayersResized = () => {
		app_notify_on_init(() => {
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
			let node = zui_get_node(canvas.nodes, nodes.nodesSelectedId[0]);
			let brushNode = ParserLogic.getLogicNode(node);
			if (brushNode != null && brushNode.getCachedImage() != null) {
				texpaint = brushNode.getCachedImage();
				texpaint_nor = render_path_render_targets.get("texpaint_nor_empty").image;
				texpaint_pack = render_path_render_targets.get("texpaint_pack_empty").image;
			}
		}

		return { texpaint: texpaint, texpaint_nor: texpaint_nor, texpaint_pack: texpaint_pack };
	}

	static onLayersResized = () => {
		image_unload(BrushOutputNode.inst.texpaint);
		BrushOutputNode.inst.texpaint = render_path_render_targets.get("texpaint").image = image_create_render_target(Config.getTextureResX(), Config.getTextureResY());
		image_unload(BrushOutputNode.inst.texpaint_nor);
		BrushOutputNode.inst.texpaint_nor = render_path_render_targets.get("texpaint_nor").image = image_create_render_target(Config.getTextureResX(), Config.getTextureResY());
		image_unload(BrushOutputNode.inst.texpaint_pack);
		BrushOutputNode.inst.texpaint_pack = render_path_render_targets.get("texpaint_pack").image = image_create_render_target(Config.getTextureResX(), Config.getTextureResY());

		if (InpaintNode.image != null) {
			image_unload(InpaintNode.image);
			InpaintNode.image = null;
			image_unload(InpaintNode.mask);
			InpaintNode.mask = null;
			InpaintNode.init();
		}

		if (PhotoToPBRNode.images != null) {
			for (let image of PhotoToPBRNode.images) image_unload(image);
			PhotoToPBRNode.images = null;
			PhotoToPBRNode.init();
		}

		if (TilingNode.image != null) {
			image_unload(TilingNode.image);
			TilingNode.image = null;
			TilingNode.init();
		}

		image_unload(render_path_render_targets.get("texpaint_blend0").image);
		render_path_render_targets.get("texpaint_blend0").image = image_create_render_target(Config.getTextureResX(), Config.getTextureResY(), tex_format_t.R8);
		image_unload(render_path_render_targets.get("texpaint_blend1").image);
		render_path_render_targets.get("texpaint_blend1").image = image_create_render_target(Config.getTextureResX(), Config.getTextureResY(), tex_format_t.R8);

		if (render_path_render_targets.get("texpaint_node") != null) {
			render_path_render_targets.delete("texpaint_node");
		}
		if (render_path_render_targets.get("texpaint_node_target") != null) {
			render_path_render_targets.delete("texpaint_node_target");
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
