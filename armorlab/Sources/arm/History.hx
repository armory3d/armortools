package arm;

import zui.Nodes;
import arm.sys.Path;
import arm.ui.UIFiles;
import arm.ui.UINodes;
import arm.Enums;

class History {

	public static var steps: Array<TStep>;
	public static var undoI = 0; // Undo layer
	public static var undos = 0; // Undos available
	public static var redos = 0; // Redos available

	public static function undo() {
		if (undos > 0) {
			var active = steps.length - 1 - redos;
			var step = steps[active];

			if (step.name == tr("Edit Nodes")) {
				swapCanvas(step);
			}
			undos--;
			redos++;
			Context.ddirty = 2;
		}
	}

	public static function redo() {
		if (redos > 0) {
			var active = steps.length - redos;
			var step = steps[active];

			if (step.name == tr("Edit Nodes")) {
				swapCanvas(step);
			}
			undos++;
			redos--;
			Context.ddirty = 2;
		}
	}

	public static function reset() {
		steps = [{name: tr("New")}];
		undos = 0;
		redos = 0;
		undoI = 0;
	}

	public static function editNodes(canvas: TNodeCanvas, canvas_group: Null<Int> = null) {
		var step = push(tr("Edit Nodes"));
		step.canvas_group = canvas_group;
		step.canvas = haxe.Json.parse(haxe.Json.stringify(canvas));
	}

	static function push(name: String): TStep {
		#if (krom_windows || krom_linux || krom_darwin)
		var filename = Project.filepath == "" ? UIFiles.filename : Project.filepath.substring(Project.filepath.lastIndexOf(Path.sep) + 1, Project.filepath.length - 4);
		kha.Window.get(0).title = filename + "* - " + Main.title;
		#end

		if (undos < Config.raw.undo_steps) undos++;
		if (redos > 0) {
			for (i in 0...redos) steps.pop();
			redos = 0;
		}

		steps.push({
			name: name
		});

		while (steps.length > Config.raw.undo_steps + 1) steps.shift();
		return steps[steps.length - 1];
	}

	static function getCanvasOwner(step: TStep): Dynamic {
		return null;
	}

	static function swapCanvas(step: TStep) {
		var _canvas = getCanvasOwner(step).canvas;
		getCanvasOwner(step).canvas = step.canvas;
		step.canvas = _canvas;

		UINodes.inst.canvasChanged();
		@:privateAccess UINodes.inst.getNodes().handle = new zui.Zui.Handle();
		UINodes.inst.hwnd.redraws = 2;
	}
}

typedef TStep = {
	public var name: String;
	@:optional public var canvas: TNodeCanvas; // Node history
	@:optional public var canvas_group: Int;
}
