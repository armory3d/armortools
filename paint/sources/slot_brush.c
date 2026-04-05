
#include "global.h"

buffer_t      *slot_brush_default_canvas = NULL;

slot_brush_t *slot_brush_create(ui_node_canvas_t *c) {
	slot_brush_t *raw  = GC_ALLOC_INIT(slot_brush_t, {0});
	raw->nodes         = ui_nodes_create();
	raw->preview_ready = false;
	raw->id            = 0;

	for (i32 i = 0; i < project_brushes->length; ++i) {
		slot_brush_t *brush = project_brushes->buffer[i];
		if (brush->id >= raw->id) {
			raw->id = brush->id + 1;
		}
	}

	if (c == NULL) {
		if (slot_brush_default_canvas == NULL) { // Synchronous
			buffer_t *b = data_get_blob("default_brush.arm");
			gc_unroot(slot_brush_default_canvas);
			slot_brush_default_canvas = b;
			gc_root(slot_brush_default_canvas);
		}
		raw->canvas = armpack_decode(slot_brush_default_canvas);
		raw->canvas = util_clone_canvas(raw->canvas); // Clone to create GC references

		i32 id            = (raw->id + 1);
		raw->canvas->name = string("Brush %d", id);
	}
	else {
		raw->canvas = util_clone_canvas(c);
	}

	return raw;
}
