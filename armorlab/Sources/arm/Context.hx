package arm;

import zui.Zui;
import iron.RenderPath;
import iron.object.MeshObject;
import arm.shader.NodeShader;
import arm.shader.MakeMaterial;
import arm.render.RenderPathDeferred;
import arm.render.RenderPathForward;
import arm.ui.UIBase;
import arm.ui.UINodes;
import arm.ui.UIHeader;
import arm.ui.BoxPreferences;
import arm.ProjectBaseFormat;
import arm.ContextFormat;

class Context {

	public static var raw: TContext = {};

	public static function setViewportMode(mode: ViewportMode) {
		if (mode == raw.viewportMode) return;

		raw.viewportMode = mode;
		var deferred = raw.renderMode != RenderForward && (raw.viewportMode == ViewLit || raw.viewportMode == ViewPathTrace);
		if (deferred) {
			RenderPath.active.commands = RenderPathDeferred.commands;
		}
		// else if (raw.viewportMode == ViewPathTrace) {
		// }
		else {
			if (RenderPathForward.path == null) {
				RenderPathForward.init(RenderPath.active);
			}
			RenderPath.active.commands = RenderPathForward.commands;
		}
		var _workspace = UIHeader.inst.worktab.position;
		UIHeader.inst.worktab.position = Space3D;
		MakeMaterial.parseMeshMaterial();
		UIHeader.inst.worktab.position = _workspace;
	}

	public static function layerFilterUsed(): Bool { return true; } ////

	public static function setSwatch(s: TSwatchColor) {
		raw.swatch = s;
		App.notifyOnNextFrame(function() {
			// MakeMaterial.parsePaintMaterial();
			// RenderUtil.makeMaterialPreview();
			// UIBase.inst.hwnd1.redraws = 2;
		});
	}

	public static function selectTool(i: Int) {
		raw.tool = i;
		MakeMaterial.parsePaintMaterial();
		MakeMaterial.parseMeshMaterial();
		raw.ddirty = 3;
	}

	public static function selectPaintObject(o: MeshObject) {
		raw.paintObject = o;
	}

	public static function mainObject(): MeshObject {
		return Project.paintObjects[0];
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
}
