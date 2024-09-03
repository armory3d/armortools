
type slot_brush_t = {
	nodes?: ui_nodes_t;
	canvas?: ui_node_canvas_t;
	image?: image_t; // 200px
	image_icon?: image_t; // 50px
	preview_ready?: bool;
	id?: i32;
};

let slot_brush_default_canvas: buffer_t = null;

function slot_brush_create(c: ui_node_canvas_t = null): slot_brush_t {
	let raw: slot_brush_t = {};
	raw.nodes = ui_nodes_create();
	raw.preview_ready = false;
	raw.id = 0;

	for (let i: i32 = 0; i < project_brushes.length; ++i) {
		let brush: slot_brush_t = project_brushes[i];
		if (brush.id >= raw.id) {
			raw.id = brush.id + 1;
		}
	}

	if (c == null) {
		if (slot_brush_default_canvas == null) { // Synchronous
			let b: buffer_t = data_get_blob("default_brush.arm");
			slot_brush_default_canvas = b;
		}
		raw.canvas = armpack_decode(slot_brush_default_canvas);
		let id: i32 = (raw.id + 1);
		raw.canvas.name = "Brush " + id;
	}
	else {
		raw.canvas = c;
	}

	return raw;
}
