
class slot_brush_t {
	nodes: zui_nodes_t = zui_nodes_create();
	canvas: zui_node_canvas_t;
	image: image_t = null; // 200px
	image_icon: image_t = null; // 50px
	preview_ready: bool = false;
	id: i32 = 0;
}

let slot_brush_default_canvas: buffer_t = null;

function slot_brush_create(c: zui_node_canvas_t = null): slot_brush_t {
	let raw: slot_brush_t = new slot_brush_t();
	for (let brush of project_brushes) {
		if (brush.id >= raw.id) {
			raw.id = brush.id + 1;
		}
	}

	if (c == null) {
		if (slot_brush_default_canvas == null) { // Synchronous
			let b: buffer_t = data_get_blob("default_brush.arm")
			slot_brush_default_canvas = b;
		}
		raw.canvas = armpack_decode(slot_brush_default_canvas);
		raw.canvas.name = "Brush " + (raw.id + 1);
	}
	else {
		raw.canvas = c;
	}

	return raw;
}
