// File browser panel

typedef struct browser_node {
	char        *name;
	char        *path;
	bool         is_dir;
	bool         expanded;
	any_array_t *children;
} browser_node_t;

ui_window_t    *browser_hwnd = NULL;
browser_node_t *browser_root = NULL;

static browser_node_t *browser_find(any_array_t *nodes, char *name) {
	if (nodes == NULL) {
		return NULL;
	}
	for (i32 i = 0; i < (i32)nodes->length; ++i) {
		browser_node_t *n = nodes->buffer[i];
		if (string_equals(n->name, name)) {
			return n;
		}
	}
	return NULL;
}

// Reads the folder contents, expanded sub-folders stay expanded
static void browser_load(browser_node_t *node) {
	any_array_t *old   = node->children;
	any_array_t *files = file_read_directory(node->path);
	node->children     = any_array_create(0);
	for (i32 i = 0; i < (i32)files->length; ++i) {
		char *name = files->buffer[i];
		if (name[0] == '\0') {
			continue;
		}
		browser_node_t *n = ALLOC_INIT(browser_node_t, {0});
		n->name           = string_copy(name);
		n->path           = path_join(node->path, name);
		n->is_dir         = iron_is_directory(n->path);
		browser_node_t *o = browser_find(old, name);
		if (n->is_dir && o != NULL && o->is_dir && o->expanded) {
			n->expanded = true;
			n->children = o->children;
			browser_load(n);
		}
		any_array_push(node->children, n);
	}
}

void browser_refresh() {
	if (browser_root != NULL) {
		browser_load(browser_root);
		browser_hwnd->redraws = 2;
	}
}

void browser_open(char *dir) {
	browser_root                = ALLOC_INIT(browser_node_t, {0});
	browser_root->name          = string_copy(pad_file_name(dir));
	browser_root->path          = string_copy(dir);
	browser_root->is_dir        = true;
	browser_root->expanded      = true;
	browser_hwnd->scroll_offset = 0;
	browser_refresh();
}

void browser_init() {
	browser_hwnd = ui_window_create();
}

#define BROWSER_ICON_COL 0xff6e6e6e
#define BROWSER_ICON_W   18 // Room for the icon before the name, unscaled

enum { BROWSER_ICON_FILE, BROWSER_ICON_FOLDER, BROWSER_ICON_FOLDER_OPEN };

static void browser_draw_icon(i32 icon, f32 x, f32 y) {
	f32 s = UI_SCALE();
	draw_set_color(BROWSER_ICON_COL);
	if (icon == BROWSER_ICON_FOLDER) {
		y += math_floor((UI_ELEMENT_H() - 10 * s) / 2.0);
		draw_filled_rect(x, y, 5 * s, 2 * s); // Tab
		draw_filled_rect(x, y + 2 * s, 12 * s, 8 * s);
	}
	else if (icon == BROWSER_ICON_FOLDER_OPEN) {
		y += math_floor((UI_ELEMENT_H() - 10 * s) / 2.0);
		draw_filled_rect(x, y, 5 * s, 2 * s); // Tab
		draw_rect(x + 0.5 * s, y + 2.5 * s, 11 * s, 7 * s, s);
		// Front cover leaning to the right
		draw_filled_triangle(x + 3 * s, y + 5 * s, x + 15 * s, y + 5 * s, x + 12 * s, y + 10 * s);
		draw_filled_triangle(x + 3 * s, y + 5 * s, x + 12 * s, y + 10 * s, x, y + 10 * s);
	}
	else {
		y += math_floor((UI_ELEMENT_H() - 12 * s) / 2.0);
		draw_rect(x + 1.5 * s, y, 9 * s, 12 * s, s);
		draw_filled_rect(x + 3.5 * s, y + 3 * s, 5 * s, s); // Text lines
		draw_filled_rect(x + 3.5 * s, y + 6 * s, 5 * s, s);
	}
}

static void browser_draw(browser_node_t *node, i32 depth) {
	f32 _x           = g_ui->_x;
	i32 _w           = g_ui->_w;
	i32 _TEXT_OFFSET = g_theme->TEXT_OFFSET;
	i32 _TEXT_COL    = g_theme->TEXT_COL;
	f32 indent       = depth * 14 * UI_SCALE();
	for (i32 i = 0; i < (i32)node->children->length; ++i) {
		browser_node_t *n = node->children->buffer[i];
		g_ui->_x          = _x + indent;
		g_ui->_w          = _w - indent;
		f32 icon_x        = g_ui->_x + UI_TEXT_OFFSET();
		f32 icon_y        = g_ui->_y;
		bool selected     = !n->is_dir && editor_current != NULL && string_equals(editor_current->path, n->path);
		// Name is placed after the icon
		g_theme->TEXT_OFFSET = _TEXT_OFFSET + BROWSER_ICON_W;
		g_theme->TEXT_COL    = n->is_dir ? g_theme->LABEL_COL : _TEXT_COL;
		bool clicked         = pad_row(n->name, selected);
		g_theme->TEXT_OFFSET = _TEXT_OFFSET;
		g_theme->TEXT_COL    = _TEXT_COL;
		g_ui->_x             = _x;
		g_ui->_w             = _w;
		if (n->is_dir) {
			if (clicked) {
				n->expanded = !n->expanded;
				if (n->expanded) {
					browser_load(n);
				}
			}
			browser_draw_icon(n->expanded ? BROWSER_ICON_FOLDER_OPEN : BROWSER_ICON_FOLDER, icon_x, icon_y);
			if (n->expanded) {
				browser_draw(n, depth + 1);
			}
		}
		else {
			browser_draw_icon(BROWSER_ICON_FILE, icon_x, icon_y);
			if (clicked) {
				editor_open(n->path);
			}
		}
	}
}

void browser_render(i32 x, i32 y, i32 w, i32 h) {
	if (!ui_window(browser_hwnd, x, y, w, h, false)) {
		return;
	}
	if (browser_root == NULL) {
		return;
	}
	pad_fade_begin();
	i32 _FILL_BUTTON_BG     = g_theme->FILL_BUTTON_BG;
	g_theme->FILL_BUTTON_BG = false;
	browser_draw(browser_root, 0);
	g_theme->FILL_BUTTON_BG = _FILL_BUTTON_BG;
	pad_fade_end();
}
