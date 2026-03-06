void nodes_brush_init() {
	gc_unroot(nodes_brush_creates);
	nodes_brush_creates = any_map_create();
	gc_root(nodes_brush_creates);
	any_map_set(nodes_brush_creates, "brush_output_node", brush_output_node_create);
	// any_map_set(nodes_brush_creates, "tex_image_node", tex_image_node_create);
	any_map_set(nodes_brush_creates, "TEX_IMAGE", tex_image_node_create);
	any_map_set(nodes_brush_creates, "input_node", input_node_create);
	any_map_set(nodes_brush_creates, "math_node", math_node_create);
	any_map_set(nodes_brush_creates, "random_node", random_node_create);
	any_map_set(nodes_brush_creates, "separate_vector_node", separate_vector_node_create);
	any_map_set(nodes_brush_creates, "time_node", time_node_create);
	any_map_set(nodes_brush_creates, "float_node", float_node_create);
	any_map_set(nodes_brush_creates, "vector_node", vector_node_create);
	any_map_set(nodes_brush_creates, "vector_math_node", vector_math_node_create);

	nodes_brush_list_init();
}

void nodes_brush_list_init() {
	if (nodes_brush_list != null) {
		return;
	}

	gc_unroot(nodes_brush_category0);
	nodes_brush_category0 = any_array_create_from_raw(
	    (any[]){
	        tex_image_node_def,
	        input_node_def,
	        math_node_def,
	        random_node_def,
	        separate_vector_node_def,
	        time_node_def,
	        float_node_def,
	        vector_node_def,
	        vector_math_node_def,
	    },
	    9);
	gc_root(nodes_brush_category0);

	gc_unroot(nodes_brush_list);
	nodes_brush_list = any_array_create_from_raw(
	    (any[]){
	        nodes_brush_category0,
	    },
	    1);
	gc_root(nodes_brush_list);
}

ui_node_t *nodes_brush_create_node(string_t *node_type) {
	for (i32 i = 0; i < nodes_brush_list->length; ++i) {
		ui_node_t_array_t *c = nodes_brush_list->buffer[i];
		for (i32 i = 0; i < c->length; ++i) {
			ui_node_t *n = c->buffer[i];
			if (string_equals(n->type, node_type)) {
				ui_node_canvas_t *canvas = context_raw->brush->canvas;
				ui_nodes_t       *nodes  = context_raw->brush->nodes;
				ui_node_t        *node   = ui_nodes_make_node(n, nodes, canvas);
				any_array_push(canvas->nodes, node);
				return node;
			}
		}
	}
	return null;
}
