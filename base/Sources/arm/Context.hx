package arm;

import iron.RenderPath;
import iron.object.MeshObject;
import iron.system.Input;
import arm.render.RenderPathDeferred;
import arm.render.RenderPathForward;
import arm.shader.MakeMaterial;
import arm.shader.NodeShader;
import arm.ui.BoxPreferences;
import arm.ui.UIHeader;
import arm.ui.UIBase;
import arm.ui.UINodes;
import arm.ProjectFormat;
import arm.ContextFormat;
#if (is_paint || is_sculpt)
import arm.data.MaterialSlot;
import arm.data.LayerSlot;
import arm.data.BrushSlot;
import arm.data.FontSlot;
import arm.util.UVUtil;
import arm.util.RenderUtil;
import arm.util.ParticleUtil;
import arm.ui.UIToolbar;
import arm.ui.UIView2D;
#end

class Context {

	public static var raw: TContext = {};

	public static function useDeferred(): Bool {
		#if is_paint
		return raw.renderMode != RenderForward && (raw.viewportMode == ViewLit || raw.viewportMode == ViewPathTrace) && raw.tool != ToolColorId;
		#end

		#if (is_sculpt || is_lab)
		return raw.renderMode != RenderForward && (raw.viewportMode == ViewLit || raw.viewportMode == ViewPathTrace);
		#end
	}

	#if (is_paint || is_sculpt)
	public static function selectMaterial(i: Int) {
		if (Project.materials.length <= i) return;
		setMaterial(Project.materials[i]);
	}

	public static function setMaterial(m: MaterialSlot) {
		if (Project.materials.indexOf(m) == -1) return;
		raw.material = m;
		MakeMaterial.parsePaintMaterial();
		UIBase.inst.hwnds[TabSidebar1].redraws = 2;
		UIHeader.inst.headerHandle.redraws = 2;
		UINodes.inst.hwnd.redraws = 2;
		UINodes.inst.groupStack = [];

		var decal = raw.tool == ToolDecal || raw.tool == ToolText;
		if (decal) {
			function _next() {
				RenderUtil.makeDecalPreview();
			}
			App.notifyOnNextFrame(_next);
		}
	}

	public static function selectBrush(i: Int) {
		if (Project.brushes.length <= i) return;
		setBrush(Project.brushes[i]);
	}

	public static function setBrush(b: BrushSlot) {
		if (Project.brushes.indexOf(b) == -1) return;
		raw.brush = b;
		MakeMaterial.parseBrush();
		UIBase.inst.hwnds[TabSidebar1].redraws = 2;
		UINodes.inst.hwnd.redraws = 2;
	}

	public static function selectFont(i: Int) {
		if (Project.fonts.length <= i) return;
		setFont(Project.fonts[i]);
	}

	public static function setFont(f: FontSlot) {
		if (Project.fonts.indexOf(f) == -1) return;
		raw.font = f;
		RenderUtil.makeTextPreview();
		RenderUtil.makeDecalPreview();
		UIBase.inst.hwnds[TabStatus].redraws = 2;
		UIView2D.inst.hwnd.redraws = 2;
	}

	public static function selectLayer(i: Int) {
		if (Project.layers.length <= i) return;
		setLayer(Project.layers[i]);
	}

	public static function setLayer(l: LayerSlot) {
		if (l == raw.layer) return;
		raw.layer = l;
		UIHeader.inst.headerHandle.redraws = 2;

		var current = @:privateAccess kha.graphics2.Graphics.current;
		if (current != null) current.end();

		App.setObjectMask();
		MakeMaterial.parseMeshMaterial();
		MakeMaterial.parsePaintMaterial();

		if (current != null) current.begin(false);

		UIBase.inst.hwnds[TabSidebar0].redraws = 2;
		UIView2D.inst.hwnd.redraws = 2;
	}
	#end

	public static function selectTool(i: Int) {
		raw.tool = i;
		MakeMaterial.parsePaintMaterial();
		MakeMaterial.parseMeshMaterial();
		raw.ddirty = 3;
		var _viewportMode = raw.viewportMode;
		raw.viewportMode = -1;
		setViewportMode(_viewportMode);

		#if (is_paint || is_sculpt)
		initTool();
		UIHeader.inst.headerHandle.redraws = 2;
		UIToolbar.inst.toolbarHandle.redraws = 2;
		#end
	}

	#if (is_paint || is_sculpt)
	public static function initTool() {
		var decal = raw.tool == ToolDecal || raw.tool == ToolText;
		if (decal) {
			if (raw.tool == ToolText) {
				RenderUtil.makeTextPreview();
			}
			RenderUtil.makeDecalPreview();
		}
		if (raw.tool == ToolParticle) {
			ParticleUtil.initParticle();
			MakeMaterial.parseParticleMaterial();
		}

		#if krom_ios
		// No hover on iPad, decals are painted by pen release
		Config.raw.brush_live = decal;
		#end
	}
	#end

	public static function selectPaintObject(o: MeshObject) {
		#if (is_paint || is_sculpt)
		UIHeader.inst.headerHandle.redraws = 2;
		for (p in Project.paintObjects) p.skip_context = "paint";
		raw.paintObject = o;

		var mask = raw.layer.getObjectMask();
		if (layerFilterUsed()) mask = raw.layerFilter;

		if (raw.mergedObject == null || mask > 0) {
			raw.paintObject.skip_context = "";
		}
		UVUtil.uvmapCached = false;
		UVUtil.trianglemapCached = false;
		UVUtil.dilatemapCached = false;
		#end

		#if is_lab
		raw.paintObject = o;
		#end
	}

	public static function mainObject(): MeshObject {
		#if (is_paint || is_sculpt)
		for (po in Project.paintObjects) if (po.children.length > 0) return po;
		return Project.paintObjects[0];
		#end

		#if is_lab
		return Project.paintObjects[0];
		#end
	}

	public static function layerFilterUsed(): Bool {
		#if (is_paint || is_sculpt)
		return raw.layerFilter > 0 && raw.layerFilter <= Project.paintObjects.length;
		#end

		#if is_lab
		return true;
		#end
	}

	public static function objectMaskUsed(): Bool {
		return raw.layer.getObjectMask() > 0 && raw.layer.getObjectMask() <= Project.paintObjects.length;
	}

	public static function inViewport(): Bool {
		return raw.paintVec.x < 1 && raw.paintVec.x > 0 &&
			   raw.paintVec.y < 1 && raw.paintVec.y > 0;
	}

	public static function inPaintArea(): Bool {
		#if (is_paint || is_sculpt)
		var mouse = Input.getMouse();
		var right = iron.App.w();
		if (UIView2D.inst.show) right += UIView2D.inst.ww;
		return mouse.viewX > 0 && mouse.viewX < right &&
			   mouse.viewY > 0 && mouse.viewY < iron.App.h();
		#end

		#if is_lab
		return inViewport();
		#end
	}

	public static function inLayers(): Bool {
		return UIBase.inst.ui.getHoveredTabName() == tr("Layers");
	}

	public static function inMaterials(): Bool {
		return UIBase.inst.ui.getHoveredTabName() == tr("Materials");
	}

	#if (is_paint || is_sculpt)
	public static function in2dView(type = View2DLayer): Bool {
		var mouse = Input.getMouse();
		return UIView2D.inst.show && UIView2D.inst.type == type &&
			   mouse.x > UIView2D.inst.wx && mouse.x < UIView2D.inst.wx + UIView2D.inst.ww &&
			   mouse.y > UIView2D.inst.wy && mouse.y < UIView2D.inst.wy + UIView2D.inst.wh;
	}
	#end

	public static function inNodes(): Bool {
		var mouse = Input.getMouse();
		return UINodes.inst.show &&
			   mouse.x > UINodes.inst.wx && mouse.x < UINodes.inst.wx + UINodes.inst.ww &&
			   mouse.y > UINodes.inst.wy && mouse.y < UINodes.inst.wy + UINodes.inst.wh;
	}

	public static function inSwatches(): Bool {
		return UIBase.inst.ui.getHoveredTabName() == tr("Swatches");
	}

	public static function inBrowser(): Bool {
		return UIBase.inst.ui.getHoveredTabName() == tr("Browser");
	}

	public static function getAreaType(): AreaType {
		if (inViewport()) return AreaViewport;
		if (inNodes()) return AreaNodes;
		if (inBrowser()) return AreaBrowser;
		#if (is_paint || is_sculpt)
		if (in2dView()) return Area2DView;
		if (inLayers()) return AreaLayers;
		if (inMaterials()) return AreaMaterials;
		#end
		return -1;
	}

	public static function setViewportMode(mode: ViewportMode) {
		if (mode == raw.viewportMode) return;

		raw.viewportMode = mode;
		if (useDeferred()) {
			RenderPath.active.commands = RenderPathDeferred.commands;
		}
		else {
			if (RenderPathForward.path == null) {
				RenderPathForward.init(RenderPath.active);
			}
			RenderPath.active.commands = RenderPathForward.commands;
		}
		var _workspace = UIHeader.inst.worktab.position;
		UIHeader.inst.worktab.position = 0;
		MakeMaterial.parseMeshMaterial();
		UIHeader.inst.worktab.position = _workspace;
	}

	public static function loadEnvmap() {
		if (!raw.envmapLoaded) {
			// TODO: Unable to share texture for both radiance and envmap - reload image
			raw.envmapLoaded = true;
			iron.data.Data.cachedImages.remove("World_radiance.k");
		}
		iron.Scene.active.world.loadEnvmap(function(_) {});
		if (raw.savedEnvmap == null) raw.savedEnvmap = iron.Scene.active.world.envmap;
	}

	@:keep
	public static function setViewportShader(viewportShader: NodeShader->String) {
		raw.viewportShader = viewportShader;
		setRenderPath();
	}

	public static function setRenderPath() {
		if (raw.renderMode == RenderForward || raw.viewportShader != null) {
			if (RenderPathForward.path == null) {
				RenderPathForward.init(RenderPath.active);
			}
			RenderPath.active.commands = RenderPathForward.commands;
		}
		else {
			RenderPath.active.commands = RenderPathDeferred.commands;
		}
		iron.App.notifyOnInit(function() {
			MakeMaterial.parseMeshMaterial();
		});
	}

	public static function enableImportPlugin(file: String): Bool {
		// Return plugin name suitable for importing the specified file
		if (BoxPreferences.filesPlugin == null) {
			BoxPreferences.fetchPlugins();
		}
		var ext = file.substr(file.lastIndexOf(".") + 1);
		for (f in BoxPreferences.filesPlugin) {
			if (f.startsWith("import_") && f.indexOf(ext) >= 0) {
				Config.enablePlugin(f);
				Console.info(f + " " + tr("plugin enabled"));
				return true;
			}
		}
		return false;
	}

	public static function setSwatch(s: TSwatchColor) {
		raw.swatch = s;
	}

	#if is_lab
	public static function runBrush(from: Int) {
		var left = 0.0;
		var right = 1.0;

		// First time init
		if (raw.lastPaintX < 0 || raw.lastPaintY < 0) {
			raw.lastPaintVecX = raw.paintVec.x;
			raw.lastPaintVecY = raw.paintVec.y;
		}

		var inpaint = UINodes.inst.getNodes().nodesSelected.length > 0 && UINodes.inst.getNodes().nodesSelected[0].type == "InpaintNode";

		// Paint bounds
		if (inpaint &&
			raw.paintVec.x > left &&
			raw.paintVec.x < right &&
			raw.paintVec.y > 0 &&
			raw.paintVec.y < 1 &&
			!arm.App.isDragging &&
			!arm.App.isResizing &&
			!arm.App.isScrolling() &&
			!arm.App.isComboSelected()) {

			var down = iron.system.Input.getMouse().down() || iron.system.Input.getPen().down();

			// Prevent painting the same spot
			var sameSpot = raw.paintVec.x == raw.lastPaintX && raw.paintVec.y == raw.lastPaintY;
			if (down && sameSpot) {
				raw.painted++;
			}
			else {
				raw.painted = 0;
			}
			raw.lastPaintX = raw.paintVec.x;
			raw.lastPaintY = raw.paintVec.y;

			if (raw.painted == 0) {
				parseBrushInputs();
			}

			if (raw.painted <= 1) {
				raw.pdirty = 1;
				raw.rdirty = 2;
			}
		}
	}

	public static function parseBrushInputs() {
		if (!raw.registered) {
			raw.registered = true;
			iron.App.notifyOnUpdate(update);
		}

		raw.paintVec = raw.coords;
	}

	static function update() {
		var mouse = iron.system.Input.getMouse();
		var paintX = mouse.viewX / iron.App.w();
		var paintY = mouse.viewY / iron.App.h();
		if (mouse.started()) {
			raw.startX = mouse.viewX / iron.App.w();
			raw.startY = mouse.viewY / iron.App.h();
		}

		var pen = iron.system.Input.getPen();
		if (pen.down()) {
			paintX = pen.viewX / iron.App.w();
			paintY = pen.viewY / iron.App.h();
		}
		if (pen.started()) {
			raw.startX = pen.viewX / iron.App.w();
			raw.startY = pen.viewY / iron.App.h();
		}

		if (Operator.shortcut(Config.keymap.brush_ruler + "+" + Config.keymap.action_paint, ShortcutDown)) {
			if (raw.lockX) paintX = raw.startX;
			if (raw.lockY) paintY = raw.startY;
		}

		raw.coords.x = paintX;
		raw.coords.y = paintY;

		if (raw.lockBegin) {
			var dx = Math.abs(raw.lockStartX - mouse.viewX);
			var dy = Math.abs(raw.lockStartY - mouse.viewY);
			if (dx > 1 || dy > 1) {
				raw.lockBegin = false;
				dx > dy ? raw.lockY = true : raw.lockX = true;
			}
		}

		var kb = iron.system.Input.getKeyboard();
		if (kb.started(Config.keymap.brush_ruler)) {
			raw.lockStartX = mouse.viewX;
			raw.lockStartY = mouse.viewY;
			raw.lockBegin = true;
		}
		else if (kb.released(Config.keymap.brush_ruler)) {
			raw.lockX = raw.lockY = raw.lockBegin = false;
		}

		parseBrushInputs();
	}
	#end
}
