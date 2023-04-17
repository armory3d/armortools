package arm;

import iron.object.MeshObject;
import iron.system.Input;
import arm.shader.MakeMaterial;
import arm.ui.UIBase;
import arm.ui.UINodes;
import arm.ProjectBaseFormat;
import arm.ContextFormat;

class Context {

	public static var raw: TContext = {};

	public static function useDeferred(): Bool {
		return raw.renderMode != RenderForward && (raw.viewportMode == ViewLit || raw.viewportMode == ViewPathTrace);
	}

	public static function layerFilterUsed(): Bool { return true; } ////

	public static function selectTool(i: Int) {
		@:privateAccess ContextBase.selectTool(i);
	}

	public static function selectPaintObject(o: MeshObject) {
		raw.paintObject = o;
	}

	public static function mainObject(): MeshObject {
		return Project.paintObjects[0];
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

	public static function inViewport(): Bool {
		return raw.paintVec.x < 1 && raw.paintVec.x > 0 &&
			   raw.paintVec.y < 1 && raw.paintVec.y > 0;
	}

	public static function inPaintArea(): Bool {
		return inViewport();
	}

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
		return -1;
	}
}
