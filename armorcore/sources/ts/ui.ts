
let ui_children: map_t<string, ui_handle_t> = map_create();
// let ui_tr: (id: string, vars: map_t<string, string>)=>string;

let _ui_row2: f32[] = [1.0 / 2.0, 1.0 / 2.0];
let _ui_row3: f32[] = [1.0 / 3.0, 1.0 / 3.0, 1.0 / 3.0];
let _ui_row4: f32[] = [1.0 / 4.0, 1.0 / 4.0, 1.0 / 4.0, 1.0 / 4.0];
let _ui_row5: f32[] = [1.0 / 5.0, 1.0 / 5.0, 1.0 / 5.0, 1.0 / 5.0, 1.0 / 5.0];
let _ui_row6: f32[] = [1.0 / 6.0, 1.0 / 6.0, 1.0 / 6.0, 1.0 / 6.0, 1.0 / 6.0, 1.0 / 6.0];
let _ui_row7: f32[] = [1.0 / 7.0, 1.0 / 7.0, 1.0 / 7.0, 1.0 / 7.0, 1.0 / 7.0, 1.0 / 7.0, 1.0 / 7.0];

function ui_row2() {
	ui_row(_ui_row2);
}

function ui_row3() {
	ui_row(_ui_row3);
}

function ui_row4() {
	ui_row(_ui_row4);
}

function ui_row5() {
	ui_row(_ui_row5);
}

function ui_row6() {
	ui_row(_ui_row6);
}

function ui_row7() {
	ui_row(_ui_row7);
}

function ui_SCALE(ui: ui_t): f32 {
	let current: ui_t = ui_get_current();
	ui_set_current(ui);
	let f: f32 = UI_SCALE();
	ui_set_current(current);
	return f;
}

function ui_ELEMENT_OFFSET(ui: ui_t): f32 {
	let current: ui_t = ui_get_current();
	ui_set_current(ui);
	let f: f32 = UI_ELEMENT_OFFSET();
	ui_set_current(current);
	return f;
}

function ui_ELEMENT_W(ui: ui_t): f32 {
	let current: ui_t = ui_get_current();
	ui_set_current(ui);
	let f: f32 = UI_ELEMENT_W();
	ui_set_current(current);
	return f;
}

function ui_ELEMENT_H(ui: ui_t): f32 {
	let current: ui_t = ui_get_current();
	ui_set_current(ui);
	let f: f32 = UI_ELEMENT_H();
	ui_set_current(current);
	return f;
}

function ui_MENUBAR_H(ui: ui_t): f32 {
	let button_offset_y: f32 = (ui.ops.theme.ELEMENT_H * ui_SCALE(ui) - ui.ops.theme.BUTTON_H * ui_SCALE(ui)) / 2;
	return ui.ops.theme.BUTTON_H * ui_SCALE(ui) * 1.1 + 2 + button_offset_y;
}

function ui_nodes_LINE_H(): f32 {
	return UI_LINE_H();
}

function ui_nodes_SCALE(): f32 {
	return UI_NODES_SCALE();
}

function ui_nodes_p(f: f32): f32 {
	return ui_p(f);
}

function ui_nodes_NODE_X(node: ui_node_t): f32 {
	return UI_NODE_X(node);
}

function ui_nodes_NODE_Y(node: ui_node_t): f32 {
	return UI_NODE_Y(node);
}

function ui_nodes_NODE_W(node: ui_node_t): f32 {
	return UI_NODE_W(node);
}

function ui_nodes_NODE_H(canvas: ui_node_canvas_t, node: ui_node_t): f32 {
	return UI_NODE_H(canvas, node);
}

function ui_nodes_OUTPUT_Y(sockets_count: i32, pos: i32): f32 {
	return UI_OUTPUT_Y(sockets_count, pos);
}

function ui_nodes_INPUT_Y(canvas: ui_node_canvas_t, sockets: ui_node_socket_t[], pos: i32): f32 {
	return UI_INPUT_Y(canvas, sockets.buffer, sockets.length, pos);
}

function ui_nodes_OUTPUTS_H(sockets_count: i32, length: i32 = -1): f32 {
	return UI_OUTPUTS_H(sockets_count, length);
}

function ui_nodes_BUTTONS_H(node: ui_node_t): f32 {
	return UI_BUTTONS_H(node);
}

function ui_nodes_PAN_X(): f32 {
	return UI_NODES_PAN_X();
}

function ui_nodes_PAN_Y(): f32 {
	return UI_NODES_PAN_Y();
}

function _ui_image(image: image_t, tint: i32 = 0xffffffff, h: f32 = -1.0, sx: i32 = 0, sy: i32 = 0, sw: i32 = 0, sh: i32 = 0): ui_state_t {
	if (image.texture_ != null) {
		return ui_sub_image(image.texture_, false, tint, h, sx, sy, sw, sh);
	}
	else {
		return ui_sub_image(image.render_target_, true, tint, h, sx, sy, sw, sh);
	}
}

function _ui_tooltip_image(image: image_t, max_width: i32 = 0) {
	if (image.texture_ != null) {
		return ui_tooltip_image(image.texture_, max_width);
	}
	else {
		return ui_tooltip_render_target(image.render_target_, max_width);
	}
}

function _ui_set_scale(ui: ui_t, factor: f32) {
	let current: ui_t = ui_get_current();
	ui_set_current(ui);
	ui_set_scale(factor);
	ui_set_current(current);
}

function _ui_end_element(element_size: f32 = -1.0) {
	if (element_size < 0) {
		ui_end_element();
	}
	else {
		ui_end_element_of_size(element_size);
	}
}

function _ui_end_input(ui: ui_t) {
	let current: ui_t = ui_get_current();
	ui_set_current(ui);
	ui_end_input();
	ui_set_current(current);
}

function ui_handle(s: string): ui_handle_t {
	let h: ui_handle_t = map_get(ui_children, s);
	if (h == null) {
		h = ui_handle_create();
		map_set(ui_children, s, h);
		return h;
	}
	h.init = false;
	return h;
}

function ui_create(ops: ui_options_t): ui_t {
    let raw: ui_t = {};
    ui_init(raw, ops);
    return raw;
}

function ui_theme_create(): ui_theme_t {
	let raw: ui_theme_t = {};
	ui_theme_default(raw);
	return raw;
}

let ui_nodes_custom_buttons: map_t<string, (i: i32)=>void> = map_create();

function nodes_on_custom_button(node_id: i32, button_name: string) {
	let f: (i: i32) => void = map_get(ui_nodes_custom_buttons, button_name);
	f(node_id);
}

function ui_nodes_create(): ui_nodes_t {
	let raw: ui_nodes_t = {};
	ui_nodes_init(raw);
	ui_nodes_exclude_remove = [
		"OUTPUT_MATERIAL_PBR",
		"GROUP_OUTPUT",
		"GROUP_INPUT",
		"BrushOutputNode"
	];
	ui_nodes_on_custom_button = nodes_on_custom_button;
	return raw;
}

function ui_get_socket(nodes: ui_node_t[], id: i32): ui_node_socket_t {
	for (let i: i32 = 0; i < nodes.length; ++i) {
		let n: ui_node_t = nodes[i];
		for (let j: i32 = 0; j < n.inputs.length; ++j) {
			let s: ui_node_socket_t = n.inputs[j];
			if (s.id == id) {
				return s;
			}
		}
		for (let j: i32 = 0; j < n.outputs.length; ++j) {
			let s: ui_node_socket_t = n.outputs[j];
			if (s.id == id) {
				return s;
			}
		}
	}
	return null;
}

function ui_set_font(raw: ui_t, font: g2_font_t) {
	g2_font_init(font); // Make sure font_ is ready
	raw.ops.font = font;
}

declare let ui_nodes_enum_texts: (s: string)=>string[];
declare let ui_touch_scroll: bool;
declare let ui_touch_hold : bool;
declare let ui_touch_tooltip: bool;
declare let ui_always_redraw_window: bool;
declare let ui_on_border_hover: any;
declare let ui_on_text_hover: any;
declare let ui_on_deselect_text: any;
declare let ui_on_tab_drop: any;
declare let ui_nodes_on_link_drag: any;
declare let ui_nodes_on_socket_released: any;
declare let ui_nodes_on_canvas_released: any;
declare let ui_nodes_on_canvas_control: any;
declare let ui_text_area_line_numbers: bool;
declare let ui_text_area_scroll_past_end: bool;
declare let ui_text_area_coloring: any;
declare let ui_nodes_socket_released: bool;
declare let ui_is_cut: bool;
declare let ui_is_copy: bool;
declare let ui_is_paste: bool;
declare let ui_nodes_exclude_remove: string[];
declare let ui_clipboard: string;

declare function ui_nest(handle: ui_handle_t, pos: i32): ui_handle_t;
declare function ui_theme_default(theme: ui_theme_t): void;
declare function ui_tab(handle: ui_handle_t, text: string, vertical: bool = false, color: i32 = -1): bool;
declare function ui_combo(handle: ui_handle_t, texts: string[], label: string = "", show_label: bool = false, align: ui_align_t = ui_align_t.LEFT, search_bar: bool = true): i32;
declare function ui_slider(handle: ui_handle_t, text: string, from: f32 = 0.0, to: f32 = 1.0, filled: bool = false, precision: f32 = 100.0, display_value: bool = true, align: ui_align_t = ui_align_t.RIGHT, text_edit: bool = true): f32;
declare function ui_button(text: string, align: ui_align_t = ui_align_t.CENTER, label: string = ""): bool;
declare function ui_text(text: string, align: ui_align_t = ui_align_t.LEFT, bg: i32 = 0x00000000): ui_state_t;
declare function ui_text_input(handle: ui_handle_t, label: string = "", align: ui_align_t = ui_align_t.LEFT, editable: bool = true, live_update: bool = false): string;
declare function ui_check(handle: ui_handle_t, text: string, label: string = ""): bool;
declare function ui_color_wheel(handle: ui_handle_t, alpha: bool = false, w: f32 = -1.0, h: f32 = -1.0, color_preview: bool = true, picker: ()=>void = null, data: any = null): color_t;
declare function ui_hovered_tab_name(): string;
declare function ui_radio(handle: ui_handle_t, position: i32, text: string, label: string = ""): bool;
declare function ui_start_text_edit(handle: ui_handle_t, align: ui_align_t = ui_align_t.LEFT): void;
declare function ui_tooltip(s: string): void;
declare function ui_tooltip_image(tex: any, max_width: i32 = 0): void;
declare function ui_tooltip_render_target(rt: any, max_width: i32 = 0): void;
declare function ui_separator(h: i32 = 4, fill: bool = true): void;
declare function ui_text_area(handle: ui_handle_t, align: ui_align_t = ui_align_t.LEFT, editable: bool = true, label: string = "", word_wrap: bool = false): string;
declare function ui_window(handle: ui_handle_t, x: i32, y: i32, w: i32, h: i32, drag: bool = false): bool;
declare function ui_begin(ui: ui_t): void;
declare function ui_end(last: bool = true): void;
declare function ui_end_window(bind_global_g: bool = true): void;
declare function ui_end_region(last: bool = true): void;
declare function ui_float_input(handle: ui_handle_t, label: string = "", align: ui_align_t = ui_align_t.LEFT, precision: f32 = 1000.0): f32;
declare function ui_get_current(): ui_t;
declare function ui_remove_node(n: ui_node_t, canvas: ui_node_canvas_t): void;
declare function ui_next_link_id(links: ui_node_link_t[]): i32;
declare function ui_set_hovered_tab_name(s: string): void;
declare function ui_begin_sticky(): void;
declare function ui_end_sticky(): void;
declare function ui_row(r: f32[]): void;
declare function ui_handle_create(): ui_handle_t;
declare function ui_fill(x: i32, y: i32, w: i32, h: i32, color: i32): void;
declare function ui_rect(x: i32, y: i32, w: i32, h: i32, color: i32, strength: f32): void;
declare function ui_draw_shadow(x: i32, y: i32, w: i32, h: i32): void;
declare function ui_draw_rect(filled: bool, x: i32, y: i32, w: i32, h: i32): void;
declare function ui_draw_round_bottom(x: i32, y: i32, w: i32): void;
declare function ui_begin_menu(): void;
declare function ui_end_menu(): void;
declare function ui_menu_button(s: string): bool;
declare function ui_begin_region(ui: ui_t, x: i32, y: i32, w: i32): void;
declare function ui_end_region(last: bool): void;
declare function ui_inline_radio(handle: ui_handle_t, texts: string[], align: i32): int;
declare function ui_end_input(): void;
declare function ui_panel(handle: ui_handle_t, text: string, is_tree: bool, filled: bool): bool;
declare function ui_nodes_rgba_popup(nhandle: ui_handle_t, val: f32_ptr, x: i32, y: i32): void;
declare function ui_get_link(links: ui_node_link_t[], id: i32): ui_node_link_t;
declare function ui_get_node(nodes: ui_node_t[], id: i32): ui_node_t;
declare function ui_input_in_rect(x: f32, y: f32, w: f32, h: f32): bool;
declare function ui_get_socket_id(nodes: ui_node_t[]): i32;
declare function ui_draw_string(text: string, x_offset: f32, y_offset: f32, align: i32, truncation: bool): void;
declare function ui_next_node_id(nodes: ui_node_t[]): i32;
declare function ui_node_canvas(nodes: ui_nodes_t, canvas: ui_node_canvas_t): void;

declare type kinc_g4_texture_t = any;
declare type kinc_g4_render_target_t = any;
declare type ui_theme_t = any;

declare type ui_t = {
	ops: ui_options_t;
	is_hovered: bool;
	is_typing: bool;
	is_escape_down: bool;
	is_delete_down: bool;
	is_return_down: bool;
	is_ctrl_down: bool;
	is_released: bool;
	is_key_pressed: bool;
	is_scrolling: bool;
	key_code: i32;
	input_started: bool;
	input_started_r: bool;
	input_released: bool;
	input_released_r: bool;
	input_x: i32;
	input_y: i32;
	input_started_x: i32;
	input_started_y: i32;
	input_enabled: bool;
	input_down_r: bool;
	input_dx: i32;
	input_dy: i32;
	input_wheel_delta: i32;
	_x: i32;
	_y: i32;
	_w: i32;
	_window_w: i32;
	_window_h: i32;
	_window_x: i32;
	_window_y: i32;
	scroll_enabled: bool;
	input_down: bool;
	font_size: i32;
	image_scroll_align: bool;
	changed: bool;
	font_offset_y: f32;
	enabled: bool;
	scissor: bool;
	text_selected_handle: ui_handle_t;
	submit_text_handle: ui_handle_t;
	combo_selected_handle: ui_handle_t;
	current_ratio: i32;
	image_invert_y: bool;
	elements_baked: bool;
	window_border_right: i32;
	window_border_top: i32;
	window_border_bottom: i32;
};

declare type ui_handle_t = {
	selected: bool;
	position: i32;
	color: u32;
	value: f32;
	text: string;
	// kinc_g4_render_target_t texture;
	redraws: i32;
	scroll_offset: f32;
	scroll_enabled: bool;
	layout: i32;
	last_max_x: f32;
	last_max_y: f32;
	drag_enabled: bool;
	drag_x: i32;
	drag_y: i32;
	changed: bool;
	init: bool;
	children: ui_handle_t[];
};

declare type ui_options_t = {
	font?: g2_font_t;
	theme?: ui_theme_t;
	scale_factor?: f32;
	color_wheel?: kinc_g4_texture_t;
	black_white_gradient?: kinc_g4_texture_t;
};

declare type ui_coloring_t = {
	color?: u32;
	start?: string[];
	end?: string;
	separated?: bool;
};

declare type ui_text_coloring_t = {
	colorings?: ui_coloring_t[];
	default_color?: u32;
};

declare type ui_canvas_control_t = {
	pan_x?: f32;
	pan_y?: f32;
	zoom?: f32;
	controls_down?: bool;
};

declare type ui_node_canvas_t = {
	name?: string;
	nodes?: ui_node_t[];
	links?: ui_node_link_t[];
};

declare type ui_node_t = {
	id?: i32;
	name?: string;
	type?: string;
	x?: f32;
	y?: f32;
	color?: u32;
	inputs?: ui_node_socket_t[];
	outputs?: ui_node_socket_t[];
	buttons?: ui_node_button_t[];
	width?: f32;
};

declare type ui_node_socket_t = {
	id?: i32;
	node_id?: i32;
	name?: string;
	type?: string;
	color?: u32;
	default_value?: f32_array_t;
	min?: f32;
	max?: f32;
	precision?: f32;
	display?: i32;
};

declare type ui_node_link_t = {
	id?: i32;
	from_id?: i32;
	from_socket?: i32;
	to_id?: i32;
	to_socket?: i32;
};

declare type ui_node_button_t = {
	name?: string;
	type?: string;
	output?: i32;
	default_value?: f32_array_t;
	data?: u8_array_t;
	min?: f32;
	max?: f32;
	precision?: f32;
	height?: f32;
};

declare type ui_nodes_t = {
	color_picker_callback?: (col: color_t)=>void;
	nodes_selected_id?: i32[];
	_input_started?: bool;
	nodes_drag?: bool;
	pan_x?: f32;
	pan_y?: f32;
	zoom?: f32;
	link_drag_id?: i32;
};

enum ui_layout_t {
	VERTICAL,
	HORIZONTAL,
}

enum ui_align_t {
	LEFT,
	CENTER,
	RIGHT,
}

enum ui_state_t {
	IDLE,
	STARTED,
	DOWN,
	RELEASED,
	HOVERED,
}

let ui_theme_keys: string[] = [
	"WINDOW_BG_COL",
	"HOVER_COL",
	"ACCENT_COL",
	"BUTTON_COL",
	"PRESSED_COL",
	"TEXT_COL",
	"LABEL_COL",
	"SEPARATOR_COL",
	"HIGHLIGHT_COL",
	"FONT_SIZE",
	"ELEMENT_W",
	"ELEMENT_H",
	"ELEMENT_OFFSET",
	"ARROW_SIZE",
	"BUTTON_H",
	"CHECK_SIZE",
	"CHECK_SELECT_SIZE",
	"SCROLL_W",
	"SCROLL_MINI_W",
	"TEXT_OFFSET",
	"TAB_W",
	"FILL_WINDOW_BG",
	"FILL_BUTTON_BG",
	"LINK_STYLE",
	"FULL_TABS",
	"ROUND_CORNERS",
	"SHADOWS",
];
