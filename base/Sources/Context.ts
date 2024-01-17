/// <reference path='./ContextFormat.ts'/>

class Context {

	static raw: TContext = new TContext(); //{};

	static useDeferred = (): bool => {
		///if is_paint
		return Context.raw.renderMode != RenderMode.RenderForward && (Context.raw.viewportMode == ViewportMode.ViewLit || Context.raw.viewportMode == ViewportMode.ViewPathTrace) && Context.raw.tool != WorkspaceTool.ToolColorId;
		///end

		///if (is_sculpt || is_lab)
		return Context.raw.renderMode != RenderMode.RenderForward && (Context.raw.viewportMode == ViewportMode.ViewLit || Context.raw.viewportMode == ViewportMode.ViewPathTrace);
		///end
	}

	///if (is_paint || is_sculpt)
	static selectMaterial = (i: i32) => {
		if (Project.materials.length <= i) return;
		Context.setMaterial(Project.materials[i]);
	}

	static setMaterial = (m: SlotMaterial) => {
		if (Project.materials.indexOf(m) == -1) return;
		Context.raw.material = m;
		MakeMaterial.parsePaintMaterial();
		UIBase.inst.hwnds[TabArea.TabSidebar1].redraws = 2;
		UIHeader.inst.headerHandle.redraws = 2;
		UINodes.inst.hwnd.redraws = 2;
		UINodes.inst.groupStack = [];

		let decal = Context.raw.tool == WorkspaceTool.ToolDecal || Context.raw.tool == WorkspaceTool.ToolText;
		if (decal) {
			let _next = () => {
				UtilRender.makeDecalPreview();
			}
			Base.notifyOnNextFrame(_next);
		}
	}

	static selectBrush = (i: i32) => {
		if (Project.brushes.length <= i) return;
		Context.setBrush(Project.brushes[i]);
	}

	static setBrush = (b: SlotBrush) => {
		if (Project.brushes.indexOf(b) == -1) return;
		Context.raw.brush = b;
		MakeMaterial.parseBrush();
		UIBase.inst.hwnds[TabArea.TabSidebar1].redraws = 2;
		UINodes.inst.hwnd.redraws = 2;
	}

	static selectFont = (i: i32) => {
		if (Project.fonts.length <= i) return;
		Context.setFont(Project.fonts[i]);
	}

	static setFont = (f: SlotFont) => {
		if (Project.fonts.indexOf(f) == -1) return;
		Context.raw.font = f;
		UtilRender.makeTextPreview();
		UtilRender.makeDecalPreview();
		UIBase.inst.hwnds[TabArea.TabStatus].redraws = 2;
		UIView2D.inst.hwnd.redraws = 2;
	}

	static selectLayer = (i: i32) => {
		if (Project.layers.length <= i) return;
		Context.setLayer(Project.layers[i]);
	}

	static setLayer = (l: SlotLayer) => {
		if (l == Context.raw.layer) return;
		Context.raw.layer = l;
		UIHeader.inst.headerHandle.redraws = 2;

		let current = Graphics2.current;
		if (current != null) current.end();

		Base.setObjectMask();
		MakeMaterial.parseMeshMaterial();
		MakeMaterial.parsePaintMaterial();

		if (current != null) current.begin(false);

		UIBase.inst.hwnds[TabArea.TabSidebar0].redraws = 2;
		UIView2D.inst.hwnd.redraws = 2;
	}
	///end

	static selectTool = (i: i32) => {
		Context.raw.tool = i;
		MakeMaterial.parsePaintMaterial();
		MakeMaterial.parseMeshMaterial();
		Context.raw.ddirty = 3;
		let _viewportMode = Context.raw.viewportMode;
		Context.raw.viewportMode = -1 as ViewportMode;
		Context.setViewportMode(_viewportMode);

		///if (is_paint || is_sculpt)
		Context.initTool();
		UIHeader.inst.headerHandle.redraws = 2;
		UIToolbar.inst.toolbarHandle.redraws = 2;
		///end
	}

	///if (is_paint || is_sculpt)
	static initTool = () => {
		let decal = Context.raw.tool == WorkspaceTool.ToolDecal || Context.raw.tool == WorkspaceTool.ToolText;
		if (decal) {
			if (Context.raw.tool == WorkspaceTool.ToolText) {
				UtilRender.makeTextPreview();
			}
			UtilRender.makeDecalPreview();
		}

		else if (Context.raw.tool == WorkspaceTool.ToolParticle) {
			UtilParticle.initParticle();
			MakeMaterial.parseParticleMaterial();
		}

		else if (Context.raw.tool == WorkspaceTool.ToolBake) {
			///if (krom_direct3d12 || krom_vulkan || krom_metal)
			// Bake in lit mode for now
			if (Context.raw.viewportMode == ViewportMode.ViewPathTrace) {
				Context.raw.viewportMode = ViewportMode.ViewLit;
			}
			///end
		}

		else if (Context.raw.tool == WorkspaceTool.ToolMaterial) {
			Base.updateFillLayers();
			Context.mainObject().skip_context = null;
		}

		///if krom_ios
		// No hover on iPad, decals are painted by pen release
		Config.raw.brush_live = decal;
		///end
	}
	///end

	static selectPaintObject = (o: MeshObject) => {
		///if (is_paint || is_sculpt)
		UIHeader.inst.headerHandle.redraws = 2;
		for (let p of Project.paintObjects) p.skip_context = "paint";
		Context.raw.paintObject = o;

		let mask = Context.raw.layer.getObjectMask();
		if (Context.layerFilterUsed()) mask = Context.raw.layerFilter;

		if (Context.raw.mergedObject == null || mask > 0) {
			Context.raw.paintObject.skip_context = "";
		}
		UtilUV.uvmapCached = false;
		UtilUV.trianglemapCached = false;
		UtilUV.dilatemapCached = false;
		///end

		///if is_lab
		Context.raw.paintObject = o;
		///end
	}

	static mainObject = (): MeshObject => {
		///if (is_paint || is_sculpt)
		for (let po of Project.paintObjects) if (po.children.length > 0) return po;
		return Project.paintObjects[0];
		///end

		///if is_lab
		return Project.paintObjects[0];
		///end
	}

	static layerFilterUsed = (): bool => {
		///if (is_paint || is_sculpt)
		return Context.raw.layerFilter > 0 && Context.raw.layerFilter <= Project.paintObjects.length;
		///end

		///if is_lab
		return true;
		///end
	}

	static objectMaskUsed = (): bool => {
		return Context.raw.layer.getObjectMask() > 0 && Context.raw.layer.getObjectMask() <= Project.paintObjects.length;
	}

	static inViewport = (): bool => {
		return Context.raw.paintVec.x < 1 && Context.raw.paintVec.x > 0 &&
			   Context.raw.paintVec.y < 1 && Context.raw.paintVec.y > 0;
	}

	static inPaintArea = (): bool => {
		///if (is_paint || is_sculpt)
		let mouse = Input.getMouse();
		let right = App.w();
		if (UIView2D.inst.show) right += UIView2D.inst.ww;
		return mouse.viewX > 0 && mouse.viewX < right &&
			   mouse.viewY > 0 && mouse.viewY < App.h();
		///end

		///if is_lab
		return Context.inViewport();
		///end
	}

	static inLayers = (): bool => {
		return UIBase.inst.ui.getHoveredTabName() == tr("Layers");
	}

	static inMaterials = (): bool => {
		return UIBase.inst.ui.getHoveredTabName() == tr("Materials");
	}

	///if (is_paint || is_sculpt)
	static in2dView = (type = View2DType.View2DLayer): bool => {
		let mouse = Input.getMouse();
		return UIView2D.inst.show && UIView2D.inst.type == type &&
			   mouse.x > UIView2D.inst.wx && mouse.x < UIView2D.inst.wx + UIView2D.inst.ww &&
			   mouse.y > UIView2D.inst.wy && mouse.y < UIView2D.inst.wy + UIView2D.inst.wh;
	}
	///end

	static inNodes = (): bool => {
		let mouse = Input.getMouse();
		return UINodes.inst.show &&
			   mouse.x > UINodes.inst.wx && mouse.x < UINodes.inst.wx + UINodes.inst.ww &&
			   mouse.y > UINodes.inst.wy && mouse.y < UINodes.inst.wy + UINodes.inst.wh;
	}

	static inSwatches = (): bool => {
		return UIBase.inst.ui.getHoveredTabName() == tr("Swatches");
	}

	static inBrowser = (): bool => {
		return UIBase.inst.ui.getHoveredTabName() == tr("Browser");
	}

	static getAreaType = (): AreaType => {
		if (Context.inViewport()) return AreaType.AreaViewport;
		if (Context.inNodes()) return AreaType.AreaNodes;
		if (Context.inBrowser()) return AreaType.AreaBrowser;
		///if (is_paint || is_sculpt)
		if (Context.in2dView()) return AreaType.Area2DView;
		if (Context.inLayers()) return AreaType.AreaLayers;
		if (Context.inMaterials()) return AreaType.AreaMaterials;
		///end
		return -1 as AreaType;
	}

	static setViewportMode = (mode: ViewportMode) => {
		if (mode == Context.raw.viewportMode) return;

		Context.raw.viewportMode = mode;
		if (Context.useDeferred()) {
			RenderPath.active.commands = RenderPathDeferred.commands;
		}
		else {
			if (RenderPathForward.path == null) {
				RenderPathForward.init(RenderPath.active);
			}
			RenderPath.active.commands = RenderPathForward.commands;
		}
		let _workspace = UIHeader.inst.worktab.position;
		UIHeader.inst.worktab.position = 0;
		MakeMaterial.parseMeshMaterial();
		UIHeader.inst.worktab.position = _workspace;
	}

	static loadEnvmap = () => {
		if (!Context.raw.envmapLoaded) {
			// TODO: Unable to share texture for both radiance and envmap - reload image
			Context.raw.envmapLoaded = true;
			Data.cachedImages.delete("World_radiance.k");
		}
		Scene.active.world.loadEnvmap((_) => {});
		if (Context.raw.savedEnvmap == null) Context.raw.savedEnvmap = Scene.active.world.envmap;
	}

	static updateEnvmap = () => {
		if (Context.raw.showEnvmap) {
			Scene.active.world.envmap = Context.raw.showEnvmapBlur ? Scene.active.world.probe.radianceMipmaps[0] : Context.raw.savedEnvmap;
		}
		else {
			Scene.active.world.envmap = Context.raw.emptyEnvmap;
		}
	}

	// @:keep
	static setViewportShader = (viewportShader: (ns: NodeShader)=>string) => {
		Context.raw.viewportShader = viewportShader;
		Context.setRenderPath();
	}

	static setRenderPath = () => {
		if (Context.raw.renderMode == RenderMode.RenderForward || Context.raw.viewportShader != null) {
			if (RenderPathForward.path == null) {
				RenderPathForward.init(RenderPath.active);
			}
			RenderPath.active.commands = RenderPathForward.commands;
		}
		else {
			RenderPath.active.commands = RenderPathDeferred.commands;
		}
		App.notifyOnInit(() => {
			MakeMaterial.parseMeshMaterial();
		});
	}

	static enableImportPlugin = (file: string): bool => {
		// Return plugin name suitable for importing the specified file
		if (BoxPreferences.filesPlugin == null) {
			BoxPreferences.fetchPlugins();
		}
		let ext = file.substr(file.lastIndexOf(".") + 1);
		for (let f of BoxPreferences.filesPlugin) {
			if (f.startsWith("import_") && f.indexOf(ext) >= 0) {
				Config.enablePlugin(f);
				Console.info(f + " " + tr("plugin enabled"));
				return true;
			}
		}
		return false;
	}

	static setSwatch = (s: TSwatchColor) => {
		Context.raw.swatch = s;
	}

	///if is_lab
	static runBrush = (from: i32) => {
		let left = 0.0;
		let right = 1.0;

		// First time init
		if (Context.raw.lastPaintX < 0 || Context.raw.lastPaintY < 0) {
			Context.raw.lastPaintVecX = Context.raw.paintVec.x;
			Context.raw.lastPaintVecY = Context.raw.paintVec.y;
		}

		let nodes = UINodes.inst.getNodes();
		let canvas = UINodes.inst.getCanvas(true);
		let inpaint = nodes.nodesSelectedId.length > 0 && nodes.getNode(canvas.nodes, nodes.nodesSelectedId[0]).type == "InpaintNode";

		// Paint bounds
		if (inpaint &&
			Context.raw.paintVec.x > left &&
			Context.raw.paintVec.x < right &&
			Context.raw.paintVec.y > 0 &&
			Context.raw.paintVec.y < 1 &&
			!Base.isDragging &&
			!Base.isResizing &&
			!Base.isScrolling() &&
			!Base.isComboSelected()) {

			let down = Input.getMouse().down() || Input.getPen().down();

			// Prevent painting the same spot
			let sameSpot = Context.raw.paintVec.x == Context.raw.lastPaintX && Context.raw.paintVec.y == Context.raw.lastPaintY;
			if (down && sameSpot) {
				Context.raw.painted++;
			}
			else {
				Context.raw.painted = 0;
			}
			Context.raw.lastPaintX = Context.raw.paintVec.x;
			Context.raw.lastPaintY = Context.raw.paintVec.y;

			if (Context.raw.painted == 0) {
				Context.parseBrushInputs();
			}

			if (Context.raw.painted <= 1) {
				Context.raw.pdirty = 1;
				Context.raw.rdirty = 2;
			}
		}
	}

	static parseBrushInputs = () => {
		if (!Context.raw.registered) {
			Context.raw.registered = true;
			App.notifyOnUpdate(Context.update);
		}

		Context.raw.paintVec = Context.raw.coords;
	}

	static update = () => {
		let mouse = Input.getMouse();
		let paintX = mouse.viewX / App.w();
		let paintY = mouse.viewY / App.h();
		if (mouse.started()) {
			Context.raw.startX = mouse.viewX / App.w();
			Context.raw.startY = mouse.viewY / App.h();
		}

		let pen = Input.getPen();
		if (pen.down()) {
			paintX = pen.viewX / App.w();
			paintY = pen.viewY / App.h();
		}
		if (pen.started()) {
			Context.raw.startX = pen.viewX / App.w();
			Context.raw.startY = pen.viewY / App.h();
		}

		if (Operator.shortcut(Config.keymap.brush_ruler + "+" + Config.keymap.action_paint, ShortcutType.ShortcutDown)) {
			if (Context.raw.lockX) paintX = Context.raw.startX;
			if (Context.raw.lockY) paintY = Context.raw.startY;
		}

		Context.raw.coords.x = paintX;
		Context.raw.coords.y = paintY;

		if (Context.raw.lockBegin) {
			let dx = Math.abs(Context.raw.lockStartX - mouse.viewX);
			let dy = Math.abs(Context.raw.lockStartY - mouse.viewY);
			if (dx > 1 || dy > 1) {
				Context.raw.lockBegin = false;
				dx > dy ? Context.raw.lockY = true : Context.raw.lockX = true;
			}
		}

		let kb = Input.getKeyboard();
		if (kb.started(Config.keymap.brush_ruler)) {
			Context.raw.lockStartX = mouse.viewX;
			Context.raw.lockStartY = mouse.viewY;
			Context.raw.lockBegin = true;
		}
		else if (kb.released(Config.keymap.brush_ruler)) {
			Context.raw.lockX = Context.raw.lockY = Context.raw.lockBegin = false;
		}

		Context.parseBrushInputs();
	}
	///end
}
