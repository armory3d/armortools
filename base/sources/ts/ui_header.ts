
let ui_header_default_h: i32 = 28;
let ui_header_h: i32 = ui_header_default_h;
let ui_header_handle: ui_handle_t = ui_handle_create();
let ui_header_worktab: ui_handle_t = ui_handle_create();

function ui_header_init() {
	ui_header_handle.layout = ui_layout_t.HORIZONTAL;
}

function ui_header_render_ui() {
	let ui: ui_t = ui_base_ui;
	if (config_raw.touch_ui) {
		ui_header_h = ui_header_default_h + 6;
	}
	else {
		ui_header_h = ui_header_default_h;
	}
	ui_header_h = math_floor(ui_header_h * ui_SCALE(ui));

	if (config_raw.layout[layout_size_t.HEADER] == 0) {
		return;
	}

	let nodesw: i32 = (ui_nodes_show || ui_view2d_show) ? config_raw.layout[layout_size_t.NODES_W] : 0;
	///if is_lab
	let ww: i32 = iron_window_width() - nodesw;
	///else
	let ww: i32 = iron_window_width() - ui_toolbar_w(true) - config_raw.layout[layout_size_t.SIDEBAR_W] - nodesw;
	///end

	if (ui_window(ui_header_handle, sys_x(), ui_header_h, ww, ui_header_h)) {
		ui._y += 2;
		ui_header_draw_tool_properties(ui);
	}
}
