
var plugin = new arm.Plugin();

var h1 = plugin.handle();
var h2 = plugin.handle();
var h3 = plugin.handle();
var h4 = plugin.handle();
var h5 = plugin.handle();
var h6 = plugin.handle();
var h7 = plugin.handle();

var x = 0.0;
var y = 0.0;
var z = 0.0;

plugin.drawUI = function(ui) {
	if (ui.panel(h1, "Presentation Mode Plugin")) {
		ui.slider(h2, "Rotate X", 0, 1, true);
		ui.slider(h3, "Rotate Y", 0, 1, true);
		ui.slider(h4, "Rotate Z", 0, 1, true);
	}
}

plugin.update = function(ui) {

	if (h2.value == 0 && h3.value == 0 && h4.value == 0) return;

	x += h2.value / 50;
	y += h3.value / 50;
	z += h4.value / 50;

	var scene = plugin.scene();
	var o = scene.meshes[0];
	o.transform.setRotation(x, y, z);
}
