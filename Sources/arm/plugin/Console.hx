package arm.plugin;

import kha.Image;
import kha.Scheduler;
import zui.Zui;
import zui.Id;

class Console {

	static var first = true;
	static var lastTime = 0.0;
	static var frameTime = 0.0;
	static var totalTime = 0.0;
	static var frames = 0;
	static var frameTimeAvg = 0.0;
	static var graph:Image = null;
	static var graphA:Image = null;
	static var graphB:Image = null;
	static var lrow = [1/2, 1/2];
	static var haxeTrace:Dynamic->haxe.PosInfos->Void = null;
	static var lastTraces:Array<String> = [''];

	static function consoleTrace(v:Dynamic, ?inf:haxe.PosInfos) {
		lastTraces.unshift(Std.string(v));
		if (lastTraces.length > 10) lastTraces.pop();
		haxeTrace(v, inf);
	}

	static function updateGraph() {
		if (graph == null) {
			graphA = Image.createRenderTarget(280, 33);
			graphB = Image.createRenderTarget(280, 33);
			graph = graphA;
		}
		else graph = graph == graphA ? graphB : graphA;
		var graphPrev = graph == graphA ? graphB : graphA;

		graph.g2.begin(true, 0x00000000);
		graph.g2.color = 0xffffffff;
		graph.g2.drawImage(graphPrev, -3, 0);

		var avg = Math.round(frameTimeAvg * 1000);
		var miss = avg > 16.7 ? (avg - 16.7) / 16.7 : 0.0;
		graph.g2.color = kha.Color.fromFloats(miss, 1 - miss, 0, 1.0);
		graph.g2.fillRect(280 - 3, 33 - avg, 3, avg);

		graph.g2.color = 0xff000000;
		graph.g2.fillRect(280 - 3, 33 - 17, 3, 1);

		graph.g2.end();
	}

	public static function render(ui:Zui) {

		if (first) {
			iron.App.notifyOnRender2D(tick);
			first = false;
			if (haxeTrace == null) {
				haxeTrace = haxe.Log.trace;
				haxe.Log.trace = consoleTrace;
			}
		}

		var avg = Math.round(frameTimeAvg * 10000) / 10;
		var fpsAvg = avg > 0 ? Math.round(1000 / avg) : 0;

		if (ui.panel(Id.handle({selected: false}), 'Profiler')) {
			if (graph != null) ui.image(graph);
			ui.indent();

			ui.row(lrow);
			ui.text('Frame');
			ui.text('$avg ms / $fpsAvg fps', Align.Right);

			ui.unindent();
		}
		ui.separator();

		// if (ui.panel(Id.handle({selected: false}), 'Render Targets')) {
		// 	ui.indent();
		// 	#if (kha_opengl || kha_webgl)
		// 	ui.imageInvertY = true;
		// 	#end
		// 	for (rt in iron.RenderPath.active.renderTargets) {
		// 		ui.text(rt.raw.name);
		// 		if (rt.image != null && !rt.is3D) {
		// 			ui.image(rt.image);
		// 		}
		// 	}
		// 	#if (kha_opengl || kha_webgl)
		// 	ui.imageInvertY = false;
		// 	#end
		// 	ui.unindent();
		// }

		// if (ui.panel(Id.handle({selected: false}), 'Cached Materials')) {
		// 	ui.indent();
		// 	for (c in iron.data.Data.cachedMaterials) {
		// 		ui.text(c.name);
		// 	}
		// 	ui.unindent();
		// }

		// if (ui.panel(Id.handle({selected: false}), 'Cached Shaders')) {
		// 	ui.indent();
		// 	for (c in iron.data.Data.cachedShaders) {
		// 		ui.text(c.name);
		// 	}
		// 	ui.unindent();
		// }

		#if js
		if (ui.panel(Id.handle({selected: false}), 'Console')) {
			ui.indent();
			ui.row([8/10, 2/10]);
			var t = ui.textInput(Id.handle());
			if (ui.button("Run")) {
				try { trace("> " + t); js.Lib.eval(t); }
				catch(e:Dynamic) { trace(e); }
			}
			for (t in lastTraces) ui.text(t);
			ui.unindent();
		}
		#end
	}

	static function tick(g:kha.graphics2.Graphics) {
		totalTime += frameTime;
		frames++;
		if (totalTime > 1.0) {
			arm.ui.UITrait.inst.hwnd.redraws = 1;
			frameTimeAvg = totalTime / frames;
			totalTime = 0;
			frames = 0;
			g.end();
			updateGraph();
			g.begin(false);
		}
		frameTime = Scheduler.realTime() - lastTime;
		lastTime = Scheduler.realTime();
	}
}
