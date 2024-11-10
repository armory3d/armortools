#pragma once

#include "iron_ui.h"
#include "iron_armpack.h"

typedef struct ui_canvas_control {
	float pan_x;
	float pan_y;
	float zoom;
	bool controls_down;
} ui_canvas_control_t;

typedef struct ui_node_socket {
	int id;
	int node_id;
	char *name;
	char *type;
	uint32_t color;
	f32_array_t *default_value;
	float min;
	float max;
	float precision;
	int display;
} ui_node_socket_t;

typedef struct ui_node_button {
	char *name;
	char *type;
	int output;
	f32_array_t *default_value;
	u8_array_t *data;
	float min;
	float max;
	float precision;
	float height;
} ui_node_button_t;

typedef struct ui_node_socket_array {
	ui_node_socket_t **buffer;
	int length;
	int capacity;
} ui_node_socket_array_t;

typedef struct ui_node_button_array {
	ui_node_button_t **buffer;
	int length;
	int capacity;
} ui_node_button_array_t;

typedef struct ui_node {
	int id;
	char *name;
	char *type;
	float x;
	float y;
	uint32_t color;
	ui_node_socket_array_t *inputs;
	ui_node_socket_array_t *outputs;
	ui_node_button_array_t *buttons;
	float width;
} ui_node_t;

typedef struct ui_node_link {
	int id;
	int from_id;
	int from_socket;
	int to_id;
	int to_socket;
} ui_node_link_t;

typedef struct ui_node_array {
	ui_node_t **buffer;
	int length;
	int capacity;
} ui_node_array_t;

typedef struct ui_node_link_array {
	ui_node_link_t **buffer;
	int length;
	int capacity;
} ui_node_link_array_t;

typedef struct ui_node_canvas {
	char *name;
	ui_node_array_t *nodes;
	ui_node_link_array_t *links;
} ui_node_canvas_t;

typedef struct ui_nodes {
	bool nodes_drag;
	i32_array_t *nodes_selected_id;
	float pan_x;
	float pan_y;
	float zoom;
	int uiw;
	int uih;
	bool _input_started;
	void (*color_picker_callback)(uint32_t);
	void *color_picker_callback_data;
	float scale_factor;
	float ELEMENT_H;
	bool dragged;
	ui_node_t *move_on_top;
	int link_drag_id;
	bool is_new_link;
	int snap_from_id;
	int snap_to_id;
	int snap_socket;
	float snap_x;
	float snap_y;
	ui_handle_t *handle;
} ui_nodes_t;

void ui_nodes_init(ui_nodes_t *nodes);
void ui_node_canvas(ui_nodes_t *nodes, ui_node_canvas_t *canvas);
void ui_nodes_rgba_popup(ui_handle_t *nhandle, float *val, int x, int y);

void ui_remove_node(ui_node_t *n, ui_node_canvas_t *canvas);

float UI_NODES_SCALE();
float UI_NODES_PAN_X();
float UI_NODES_PAN_Y();
extern char *ui_clipboard;
extern char_ptr_array_t *ui_nodes_exclude_remove;
extern bool ui_nodes_socket_released;
extern char_ptr_array_t *(*ui_nodes_enum_texts)(char *);
extern void (*ui_nodes_on_custom_button)(int, char *);
extern ui_canvas_control_t *(*ui_nodes_on_canvas_control)(void);
extern void (*ui_nodes_on_canvas_released)(void);
extern void (*ui_nodes_on_socket_released)(int);
extern void (*ui_nodes_on_link_drag)(int, bool);

void ui_node_canvas_encode(ui_node_canvas_t *canvas);
uint32_t ui_node_canvas_encoded_size(ui_node_canvas_t *canvas);
char *ui_node_canvas_to_json(ui_node_canvas_t *canvas);

float UI_NODE_X(ui_node_t *node);
float UI_NODE_Y(ui_node_t *node);
float UI_NODE_W(ui_node_t *node);
float UI_NODE_H(ui_node_canvas_t *canvas, ui_node_t *node);
float UI_OUTPUT_Y(int sockets_count, int pos);
float UI_INPUT_Y(ui_node_canvas_t *canvas, ui_node_socket_t **sockets, int sockets_count, int pos);
float UI_OUTPUTS_H(int sockets_count, int length);
float UI_BUTTONS_H(ui_node_t *node);
float UI_LINE_H();
float UI_NODES_PAN_X();
float UI_NODES_PAN_Y();

float ui_p(float f);
int ui_get_socket_id(ui_node_array_t *nodes);
ui_node_link_t *ui_get_link(ui_node_link_array_t *links, int id);
int ui_next_link_id(ui_node_link_array_t *links);
ui_node_t *ui_get_node(ui_node_array_t *nodes, int id);
int ui_next_node_id(ui_node_array_t *nodes);
