#include <iron.h>

typedef struct storage {
	char                  *project;
	char                  *file;
	char                  *text;
	bool                   modified;
	struct string_t_array *expanded;
	i32                    window_w;
	i32                    window_h;
	i32                    window_x;
	i32                    window_y;
	i32                    sidebar_w;
} storage_t;

typedef struct config {
	char *server;
} config_t;

typedef struct ui_coloring_t_array {
	ui_coloring_t **buffer;
	int             length;
	int             capacity;
} ui_coloring_t_array_t;

typedef struct string_t_array {
	char **buffer;
	int    length;
	int    capacity;
} string_t_array_t;

extern any_map_t *ui_children;

ui_t        *ui_create(ui_options_t *ops);
ui_handle_t *ui_handle(char *s);
void         drop_files(char *path);
char        *encode_storage();
void         on_shutdown();
void         _main();
char        *armpack_to_string(buffer_t *bytes);
void         list_folder(char *path);
void         render(void *_);
void         save_file();
char        *build_file();
void         build_project();
void         draw_minimap();
bool         hit_test(f32 mx, f32 my, f32 x, f32 y, f32 w, f32 h);
void         on_border_hover(ui_handle_t *handle, i32 side);
void         toggle_fullscreen();

ui_t          *ui;
ui_theme_t    *theme;
ui_handle_t   *text_handle;
ui_handle_t   *sidebar_handle;
ui_handle_t   *editor_handle;
storage_t     *storage;
bool           resizing_sidebar;
i32            minimap_w;
i32            minimap_h;
i32            minimap_box_h;
bool           minimap_scrolling;
gpu_texture_t *minimap;
bool           redraw_minimap;
i32            window_header_h;
i32            _window_w;
i32            _window_h;
config_t      *config_raw;

void _kickstart() {
	gc_unroot(ui_children);
	ui_children = any_map_create();
	gc_root(ui_children);

	gc_unroot(text_handle);
	text_handle = ui_handle_create();
	gc_root(text_handle);

	gc_unroot(sidebar_handle);
	sidebar_handle = ui_handle_create();
	gc_root(sidebar_handle);

	gc_unroot(editor_handle);
	editor_handle = ui_handle_create();
	gc_root(editor_handle);

	storage           = NULL;
	resizing_sidebar  = false;
	minimap_w         = 150;
	minimap_h         = 0;
	minimap_box_h     = 0;
	minimap_scrolling = false;
	minimap           = NULL;
	redraw_minimap    = true;
	window_header_h   = 0;

	_main();
	iron_start();
}

void drop_files(char *path) {
	storage->project        = string_copy(path);
	sidebar_handle->redraws = 1;
}

char *encode_storage() {
	json_encode_begin();
	json_encode_string("project", storage->project);
	json_encode_string("file", storage->file);
	json_encode_string("text", "");
	json_encode_bool("modified", storage->modified);
	json_encode_string_array("expanded", storage->expanded);
	json_encode_i32("window_w", storage->window_w);
	json_encode_i32("window_h", storage->window_h);
	json_encode_i32("window_x", storage->window_x);
	json_encode_i32("window_y", storage->window_y);
	json_encode_i32("sidebar_w", storage->sidebar_w);
	char *config_json = json_encode_end();
	return config_json;
}

void on_shutdown() {
	char *storage_string = sys_string_to_buffer(encode_storage());
	iron_file_save_bytes(string("%s/config.json", iron_internal_save_path()), storage_string, 0);
}

void _main() {
	iron_set_app_name("ArmorPad");
	buffer_t *blob_storage = iron_load_blob(string("%s/config.json", iron_internal_save_path()));
	if (blob_storage == NULL) {
		gc_unroot(storage);
		storage = GC_ALLOC_INIT(storage_t, {0});
		gc_root(storage);
		storage->project   = "";
		storage->file      = "untitled";
		storage->text      = "";
		storage->modified  = false;
		storage->expanded  = any_array_create_from_raw((void *[]){}, 0);
		storage->window_w  = 1600;
		storage->window_h  = 900;
		storage->window_x  = -1;
		storage->window_y  = -1;
		storage->sidebar_w = 230;
	}
	else {
		gc_unroot(storage);
		storage = json_parse(sys_buffer_to_string(blob_storage));
		gc_root(storage);
	}
	text_handle->text = string_copy(storage->text);
	iron_window_options_t *ops =
	    GC_ALLOC_INIT(iron_window_options_t, {.title     = "ArmorPad",
	                                          .x         = storage->window_x,
	                                          .y         = storage->window_y,
	                                          .width     = storage->window_w,
	                                          .height    = storage->window_h,
	                                          .features  = IRON_WINDOW_FEATURES_RESIZABLE | IRON_WINDOW_FEATURES_MAXIMIZABLE | IRON_WINDOW_FEATURES_MINIMIZABLE,
	                                          .mode      = IRON_WINDOW_MODE_WINDOW,
	                                          .frequency = 60,
	                                          .vsync     = true,
	                                          .display_index = 0,
	                                          .visible       = true,
	                                          .color_bits    = 32,
	                                          .depth_bits    = 0});
	sys_start(ops);
	draw_font_t *font = data_get_font("font_mono.ttf");
	draw_font_init(font);
	gc_unroot(theme);
	theme = GC_ALLOC_INIT(ui_theme_t, {0});
	gc_root(theme);
	ui_theme_default(theme);
	theme->WINDOW_BG_COL = 0xff000000;
	theme->SEPARATOR_COL = 0xff000000;
	theme->BUTTON_COL    = 0xff000000;
	theme->FONT_SIZE     = 18;
	ui_options_t *ui_ops = GC_ALLOC_INIT(ui_options_t, {.scale_factor = 1.0, .theme = theme, .font = font});
	gc_unroot(ui);
	ui = ui_create(ui_ops);
	gc_root(ui);
	buffer_t           *blob_coloring = data_get_blob("text_coloring.json");
	ui_text_coloring_t *text_coloring = json_parse(sys_buffer_to_string(blob_coloring));
	gc_unroot(ui_text_area_coloring);
	ui_text_area_coloring = text_coloring;
	gc_root(ui_text_area_coloring);
	gc_unroot(ui_on_border_hover);
	ui_on_border_hover = on_border_hover;
	gc_root(ui_on_border_hover);
	ui_text_area_line_numbers    = true;
	ui_text_area_scroll_past_end = true;
	sys_notify_on_render(render, NULL);
	_iron_set_drop_files_callback(drop_files);
	iron_set_application_state_callback(NULL, NULL, NULL, NULL, on_shutdown);
}

char *armpack_to_string(buffer_t *bytes) {
	return "";
}

void list_folder(char *path) {
	string_t_array_t *files = file_read_directory(path);
	for (i32 i = 0; i < files->length; ++i) {
		char *f           = files->buffer[i];
		char *abs         = string("%s/%s", path, f);
		bool  is_file     = string_index_of(f, ".") >= 0;
		bool  is_expanded = string_array_index_of(storage->expanded, abs) >= 0;

		// Active file
		if (string_equals(abs, storage->file)) {
			ui_fill(0, 1, ui->_w - 1, UI_ELEMENT_H() - 1, theme->PRESSED_COL);
		}
		char *prefix = "";
		if (!is_file) {
			prefix = is_expanded ? "- " : "+ ";
		}
		if (ui_button(string("%s%s", prefix, f), UI_ALIGN_LEFT, "")) {
			// Open file
			if (is_file) {
				storage->file   = string_copy(abs);
				buffer_t *bytes = iron_load_blob(storage->file);
				if (ends_with(f, ".arm")) {
					storage->text = string_copy(armpack_to_string(bytes));
				}
				else {
					storage->text = string_copy(sys_buffer_to_string(bytes));
				}
				storage->text          = string_copy(string_replace_all(storage->text, "\r", ""));
				text_handle->text      = string_copy(storage->text);
				editor_handle->redraws = 1;
				redraw_minimap         = true;
				iron_window_set_title(abs);
			}
			// Expand folder
			else {
				if (string_array_index_of(storage->expanded, abs) == -1) {
					any_array_push(storage->expanded, abs);
				}
				else {
					string_array_remove(storage->expanded, abs);
				}
			}
		}
		if (is_expanded) {
			ui->_x += 16;
			list_folder(abs);
			ui->_x -= 16;
		}
	}
}

void render(void *_) {
	storage->window_w = iron_window_width();
	storage->window_h = iron_window_height();
	storage->window_x = iron_window_x();
	storage->window_y = iron_window_y();
	if (ui->input_dx != 0 || ui->input_dy != 0) {
		iron_mouse_set_cursor(IRON_CURSOR_ARROW);
	}
	ui_begin(ui);
	if (ui_window(sidebar_handle, 0, 0, storage->sidebar_w, iron_window_height(), false)) {
		if (!string_equals(storage->project, "")) {
			list_folder(storage->project);
		}
		else {
			ui_button("Drop folder here", UI_ALIGN_LEFT, "");
		}
		ui_fill(iron_window_width() - minimap_w, 0, minimap_w, UI_ELEMENT_H() + UI_ELEMENT_OFFSET() + 1, theme->SEPARATOR_COL);
		ui_fill(storage->sidebar_w, 0, 1, iron_window_height(), theme->SEPARATOR_COL);
	}
	if (ui_window(editor_handle, storage->sidebar_w + 1, 0, iron_window_width() - storage->sidebar_w - minimap_w, iron_window_height(), false)) {
		ui_handle_t      *htab       = ui_handle("main.ts:204");
		char             *file_name  = substring(storage->file, string_last_index_of(storage->file, "/") + 1, string_length(storage->file));
		string_t_array_t *file_names = any_array_create_from_raw(
		    (void *[]){
		        file_name,
		    },
		    1);
		for (i32 i = 0; i < file_names->length; ++i) {
			char *tab_name = file_names->buffer[i];
			if (storage->modified) {
				tab_name = string("%s*", tab_name);
			}
			if (ui_tab(htab, tab_name, false, -1, false)) {
				// File modified
				if (ui->is_key_pressed) {
					storage->modified = true;
				}

				// Save
				if (ui->is_ctrl_down && ui->key_code == KEY_CODE_S) {
					save_file();
				}

				storage->text = string_copy(ui_text_area(text_handle, UI_ALIGN_LEFT, true, "", false));
			}
		}
		window_header_h = 32;
	}
	if (resizing_sidebar) {
		storage->sidebar_w += math_floor(ui->input_dx);
	}
	if (!ui->input_down) {
		resizing_sidebar = false;
	}

	// Minimap controls
	i32  minimap_x = iron_window_width() - minimap_w;
	i32  minimap_y = window_header_h + 1;
	bool redraw    = false;
	if (ui->input_started && hit_test(ui->input_x, ui->input_y, minimap_x + 5, minimap_y, minimap_w, minimap_h)) {
		minimap_scrolling = true;
	}
	if (!ui->input_down) {
		minimap_scrolling = false;
	}
	if (minimap_scrolling) {
		redraw = true;
	}

	// Build project
	if (ui->is_ctrl_down && ui->key_code == KEY_CODE_B) {
		save_file();
		build_project();
	}
	ui_end();
	if (ui->is_key_pressed && ui->key_code == KEY_CODE_F11) {
		toggle_fullscreen();
	}
	if (redraw) {
		editor_handle->redraws = 2;
	}
	if (minimap != NULL) {
		draw_begin(NULL, false, 0);
		draw_image(minimap, minimap_x, minimap_y);
		draw_end();
	}
	if (redraw_minimap) {
		redraw_minimap = false;
		draw_minimap();
	}
}

void save_file() {
	// Trim
	string_t_array_t *lines = string_split(storage->text, "\n");
	for (i32 i = 0; i < lines->length; ++i) {
		lines->buffer[i] = trim_end(lines->buffer[i]);
	}
	storage->text = string_copy(string_array_join(lines, "\n"));
	// Spaces to tabs
	storage->text     = string_copy(string_replace_all(storage->text, "    ", "\t"));
	text_handle->text = string_copy(storage->text);
	// Write bytes
	// let bytes: buffer_t;
	// if (ends_with(storage.file, ".arm")) {
	// 	armpack_encode(json_parse(storage.text));
	// }
	// else {
	// 	sys_string_to_buffer(storage.text);
	// }
	// iron_file_save_bytes(storage.file, bytes, bytes.length);
	storage->modified = false;
}

char *build_file() {
#ifdef IRON_WINDOWS
	return "\\build.bat";
#else
	return "/build.sh";
#endif
}

void build_project() {
	iron_sys_command(string("%s%s %s", storage->project, build_file(), storage->project));
}

void draw_minimap() {
	if (minimap_h != iron_window_height()) {
		minimap_h = iron_window_height();
		if (minimap != NULL) {
			gpu_delete_texture(minimap);
		}
		gc_unroot(minimap);
		minimap = gpu_create_render_target(minimap_w, minimap_h, GPU_TEXTURE_FORMAT_RGBA32);
		gc_root(minimap);
	}
	draw_begin(minimap, true, theme->SEPARATOR_COL);
	draw_set_color(0xff333333);
	string_t_array_t *lines           = string_split(storage->text, "\n");
	i32               minimap_full_h  = lines->length * 2;
	f32               scroll_progress = -editor_handle->scroll_offset / (float)(lines->length * UI_ELEMENT_H());
	i32               out_of_screen   = minimap_full_h - minimap_h;
	if (out_of_screen < 0) {
		out_of_screen = 0;
	}
	i32 offset = math_floor((out_of_screen * scroll_progress) / (float)2);
	for (i32 i = 0; i < lines->length; ++i) {
		if (i * 2 > minimap_h || i + offset >= lines->length) {
			// Out of screen
			break;
		}
		string_t_array_t *words = string_split(lines->buffer[i + offset], " ");
		i32               x     = 0;
		for (i32 j = 0; j < words->length; ++j) {
			char *word = words->buffer[j];
			draw_filled_rect(x, i * 2, string_length(word), 2);
			x += string_length(word) + 1;
		}
	}

	// Current position
	i32 visible_area = out_of_screen > 0 ? minimap_h : minimap_full_h;
	draw_set_color(0x11ffffff);
	minimap_box_h = math_floor((iron_window_height() - window_header_h) / (float)UI_ELEMENT_H() * 2);
	draw_filled_rect(0, scroll_progress * visible_area, minimap_w, minimap_box_h);
	draw_end();
}

bool hit_test(f32 mx, f32 my, f32 x, f32 y, f32 w, f32 h) {
	return mx > x && mx < x + w && my > y && my < y + h;
}

void on_border_hover(ui_handle_t *handle, i32 side) {
	if (handle != sidebar_handle) {
		return;
	}
	if (side != 1) {
		return; // Right
	}
	iron_mouse_set_cursor(IRON_CURSOR_SIZEWE);
	if (ui->input_started) {
		resizing_sidebar = true;
	}
}

void toggle_fullscreen() {
	if (iron_window_get_mode() == IRON_WINDOW_MODE_WINDOW) {
		_window_w = iron_window_width();
		_window_h = iron_window_height();
		iron_window_change_mode(IRON_WINDOW_MODE_FULLSCREEN);
	}
	else {
		iron_window_change_mode(IRON_WINDOW_MODE_WINDOW);
		iron_window_resize(_window_w, _window_h);
	}
}

////

any_map_t *ui_children;
any_map_t *ui_nodes_custom_buttons;
i32        pipes_offset;
char      *strings_check_internet_connection() {
    return "";
}
void  console_error(char *s) {}
void  console_info(char *s) {}
char *tr(char *id, any_map_t *vars) {
	return id;
}
i32 pipes_get_constant_location(char *s) {
	return 0;
}
