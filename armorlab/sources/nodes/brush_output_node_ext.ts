
type brush_output_node_t = {
	base?: logic_node_t;
	raw?: ui_node_t;

	id?: i32;
	texpaint?: iron_g5_texture_t;
	texpaint_nor?: iron_g5_texture_t;
	texpaint_pack?: iron_g5_texture_t;
	texpaint_nor_empty?: iron_g5_texture_t;
	texpaint_pack_empty?: iron_g5_texture_t;
};

function brush_output_node_create_ext(n: brush_output_node_t) {

	n.base.get_as_image = brush_output_node_get_as_image;

	if (context_raw.brush_output_node_inst == null) {
		{
			let t: render_target_t = render_target_create();
			t.name = "texpaint";
			t.width = config_get_texture_res_x();
			t.height = config_get_texture_res_y();
			t.format = "RGBA32";
			n.texpaint = render_path_create_render_target(t)._image;
		}
		{
			let t: render_target_t = render_target_create();
			t.name = "texpaint_nor";
			t.width = config_get_texture_res_x();
			t.height = config_get_texture_res_y();
			t.format = "RGBA32";
			n.texpaint_nor = render_path_create_render_target(t)._image;
		}
		{
			let t: render_target_t = render_target_create();
			t.name = "texpaint_pack";
			t.width = config_get_texture_res_x();
			t.height = config_get_texture_res_y();
			t.format = "RGBA32";
			n.texpaint_pack = render_path_create_render_target(t)._image;
		}
		{
			let t: render_target_t = render_target_create();
			t.name = "texpaint_nor_empty";
			t.width = 1;
			t.height = 1;
			t.format = "RGBA32";
			n.texpaint_nor_empty = render_path_create_render_target(t)._image;
		}
		{
			let t: render_target_t = render_target_create();
			t.name = "texpaint_pack_empty";
			t.width = 1;
			t.height = 1;
			t.format = "RGBA32";
			n.texpaint_pack_empty = render_path_create_render_target(t)._image;
		}
	}
	else {
		n.texpaint = context_raw.brush_output_node_inst.texpaint;
		n.texpaint_nor = context_raw.brush_output_node_inst.texpaint_nor;
		n.texpaint_pack = context_raw.brush_output_node_inst.texpaint_pack;
	}

	context_raw.brush_output_node_inst = n;
}

function brush_output_node_get_as_image(self: brush_output_node_t, from: i32): iron_g5_texture_t {
	return logic_node_input_get_as_image(self.base.inputs[from]);
}

function brush_output_node_parse_inputs() {
	if (!context_raw.registered) {
		context_raw.registered = true;
		app_notify_on_update(brush_output_node_update);
	}

	context_raw.paint_vec = context_raw.coords;
}

function brush_output_node_run(from: i32) {
	let left: f32 = 0.0;
	let right: f32 = 1.0;
	let top: f32 = 0.0;
	let bottom: f32 = 1.0;

	// First time init
	if (context_raw.last_paint_x < 0 || context_raw.last_paint_y < 0) {
		context_raw.last_paint_vec_x = context_raw.paint_vec.x;
		context_raw.last_paint_vec_y = context_raw.paint_vec.y;
	}

	let nodes: ui_nodes_t = ui_nodes_get_nodes();
	let canvas: ui_node_canvas_t = ui_nodes_get_canvas(true);
	let inpaint: bool = nodes.nodes_selected_id.length > 0 && ui_get_node(canvas.nodes, nodes.nodes_selected_id[0]).type == "inpaint_node";

	// Paint bounds
	if (inpaint &&
		context_raw.paint_vec.x > left &&
		context_raw.paint_vec.x < right &&
		context_raw.paint_vec.y > 0 &&
		context_raw.paint_vec.y < 1 &&
		!base_is_dragging &&
		!base_is_resizing &&
		!base_is_scrolling() &&
		!base_is_combo_selected()) {

		let down: bool = mouse_down() || pen_down();

		// Prevent painting the same spot
		let same_spot: bool = context_raw.paint_vec.x == context_raw.last_paint_x && context_raw.paint_vec.y == context_raw.last_paint_y;
		if (down && same_spot) {
			context_raw.painted++;
		}
		else {
			context_raw.painted = 0;
		}
		context_raw.last_paint_x = context_raw.paint_vec.x;
		context_raw.last_paint_y = context_raw.paint_vec.y;

		if (context_raw.painted == 0) {
			brush_output_node_parse_inputs();
		}

		if (context_raw.painted <= 1) {
			context_raw.pdirty = 1;
			context_raw.rdirty = 2;
		}
	}
}

function brush_output_node_update() {
	let paint_x: f32 = mouse_view_x() / app_w();
	let paint_y: f32 = mouse_view_y() / app_h();
	if (mouse_started()) {
		context_raw.start_x = mouse_view_x() / app_w();
		context_raw.start_y = mouse_view_y() / app_h();
	}

	if (pen_down()) {
		paint_x = pen_view_x() / app_w();
		paint_y = pen_view_y() / app_h();
	}
	if (pen_started()) {
		context_raw.start_x = pen_view_x() / app_w();
		context_raw.start_y = pen_view_y() / app_h();
	}

	if (operator_shortcut(map_get(config_keymap, "brush_ruler") + "+" + map_get(config_keymap, "action_paint"), shortcut_type_t.DOWN)) {
		if (context_raw.lock_x) {
			paint_x = context_raw.start_x;
		}
		if (context_raw.lock_y) {
			paint_y = context_raw.start_y;
		}
	}

	context_raw.coords.x = paint_x;
	context_raw.coords.y = paint_y;

	if (context_raw.lock_begin) {
		let dx: i32 = math_abs(context_raw.lock_start_x - mouse_view_x());
		let dy: i32 = math_abs(context_raw.lock_start_y - mouse_view_y());
		if (dx > 1 || dy > 1) {
			context_raw.lock_begin = false;
			if (dx > dy) {
				context_raw.lock_y = true;
			}
			else {
				context_raw.lock_x = true;
			}
		}
	}

	if (keyboard_started(map_get(config_keymap, "brush_ruler"))) {
		context_raw.lock_start_x = mouse_view_x();
		context_raw.lock_start_y = mouse_view_y();
		context_raw.lock_begin = true;
	}
	else if (keyboard_released(map_get(config_keymap, "brush_ruler"))) {
		context_raw.lock_x = context_raw.lock_y = context_raw.lock_begin = false;
	}

	brush_output_node_parse_inputs();
}
