
let plugin = new arm.Plugin();

let h1 = new zui.Handle();
let first = true;
let lastTime = 0.0;
let frameTime = 0.0;
let totalTime = 0.0;
let frames = 0;
let frameTimeAvg = 0.0;
let graph = null;
let graphA = null;
let graphB = null;
let lrow = [1/2, 1/2];

plugin.drawUI = function(ui) {
	if (ui.panel(h1, "Profiler")) {
		let avg = Math.round(frameTimeAvg * 10000) / 10;
		let fpsAvg = avg > 0 ? Math.round(1000 / avg) : 0;

		if (first) {
			first = false;
			iron.App.notifyOnRender2D(tick);
		}

		if (graph != null) ui.image(graph);
		ui.indent();
		ui.row(lrow);
		ui.text('Frame');
		ui.text(`${avg} ms / ${fpsAvg} fps`, 2); // Align.Right
		ui.unindent();
	}
}

let updateGraph = function() {
	if (graph === null) {
		graphA = core.Image.createRenderTarget(280, 33);
		graphB = core.Image.createRenderTarget(280, 33);
		graph = graphA;
	}
	else graph = graph === graphA ? graphB : graphA;
	let graphPrev = graph === graphA ? graphB : graphA;

	let g2 = graph.get_g2();
	g2.begin(true, 0x00000000);
	g2.set_color(0xffffffff);
	g2.drawImage(graphPrev, -3, 0);

	let avg = Math.round(frameTimeAvg * 1000);
	let miss = avg > 16.7 ? (avg - 16.7) / 16.7 : 0.0;
	g2.set_color(core.colorFromFloats(miss, 1 - miss, 0, 1.0));
	g2.fillRect(280 - 3, 33 - avg, 3, avg);

	g2.set_color(0xff000000);
	g2.fillRect(280 - 3, 33 - 17, 3, 1);

	g2.end();
}

let tick = function(g2) {
	totalTime += frameTime;
	frames++;
	if (totalTime > 1.0) {
		arm.UISidebar.inst.hwnd0.redraws = 1;
		frameTimeAvg = totalTime / frames;
		totalTime = 0;
		frames = 0;
		g2.end();
		updateGraph();
		g2.begin(false);
	}
	frameTime = core.Scheduler.realTime() - lastTime;
	lastTime = core.Scheduler.realTime();
}
