
#include "global.h"

#define NODES_MESH_SOCKET_COL 0xff00d6a3
#define NODES_MESH_MAX_DEPTH  64

static any_array_t       *nodes_mesh_slots = NULL;
static ui_node_t_array_t *nodes_mesh_category_input;
static ui_node_t_array_t *nodes_mesh_category_modifier;
static ui_node_t_array_t *nodes_mesh_category_combine;
static ui_node_t_array_t *nodes_mesh_category_utilities;
static bool               nodes_mesh_refresh_pending = false;
static ui_node_t         *nodes_mesh_input_def;
static ui_node_t         *nodes_mesh_output_def;

static ui_node_canvas_t *nodes_mesh_eval_canvas;
static mesh_nodes_t     *nodes_mesh_eval_slot;
static mesh_object_t    *nodes_mesh_eval_object;
static bool              nodes_mesh_eval_unwrap;
static i32               nodes_mesh_eval_depth;

static mesh_geo_t *nodes_mesh_eval_node(ui_node_t *node);

// Script node state, valid while its script runs
static bool         nodes_mesh_script_running = false;
static mesh_geo_t  *nodes_mesh_script_in      = NULL;
static mesh_geo_t  *nodes_mesh_script_out     = NULL;
static mesh_geo_t **nodes_mesh_script_geos    = NULL;
static i32          nodes_mesh_script_count   = 0;
static i32          nodes_mesh_script_cap     = 0;

static ui_node_t        *nodes_mesh_script_node  = NULL;
static ui_node_button_t *nodes_mesh_script_dirty = NULL;
static ui_node_t        *nodes_mesh_socket_node  = NULL;

static void nodes_mesh_def_button(ui_node_t *n, char *name, char *type, f32 value, char *texts);

static char *nodes_mesh_script_text(ui_node_t *node) {
	ui_node_button_t *but = node->buttons->buffer[0];
	return but->data != NULL ? (char *)but->data->buffer : "";
}

static void nodes_mesh_script_button(i32 node_id) {
	ui_node_t        *node = ui_get_node(ui_nodes_get_canvas(true)->nodes, node_id);
	ui_node_button_t *but  = node->buttons->buffer[0];
	char             *text = nodes_mesh_script_text(node);
	if (node->buttons->length < 2) {
		// Node saved before inputs could be added
		nodes_mesh_def_button(node, "nodes_mesh_script_add_button", "CUSTOM", 0, NULL);
		node->buttons->buffer[1]->output = -1;
		node->buttons->buffer[1]->height = 1;
	}
	if (!ui_nodes_code_panel(node)) {
		// Apply the edit of a text field hidden while selected
		if (nodes_mesh_script_dirty == but) {
			nodes_mesh_script_dirty = NULL;
			g_ui->changed           = true;
		}
		but->height = 1;
		return;
	}
	ui_set_next_id((ui_id_t)&but->data);
	bool selected = g_ui->text_selected_id == ui_widget_id(NULL, UI_ID_TEXT);
	bool changed  = g_ui->changed;
	ui_nodes_code_begin();
	ui_text_area(&text, &ui_nodes_editor_state(node)->line, UI_ALIGN_LEFT, true, "", false);
	ui_nodes_code_end();
	if (ui_item_changed()) {
		but->data = u8_array_create_from_string(text);
		if (selected) {
			nodes_mesh_script_dirty = but;
		}
	}
	// The mesh is rebuilt once the text field is left
	if (selected) {
		g_ui->changed = changed;
	}
	else if (nodes_mesh_script_dirty == but) {
		nodes_mesh_script_dirty = NULL;
		g_ui->changed           = true;
	}
	but->height = ui_line_count(text) + 2;
}

// Inputs added to a script node are read by name with mesh_geo_input_value() and mesh_geo_input_vector()
ui_node_socket_t *nodes_mesh_script_add_socket(ui_node_canvas_t *canvas, ui_node_t *node, char *type, char *name) {
	if (!string_equals(type, "RGBA") && !string_equals(type, "VECTOR")) {
		type = "VALUE";
	}
	if (name == NULL || string_equals(name, "")) {
		// First free name, sockets are looked up by it
		char *base = nodes_material_get_socket_name(type);
		name       = base;
		for (i32 i = 2; i < 999; ++i) {
			bool found = false;
			for (i32 j = 0; j < node->inputs->length; ++j) {
				if (string_equals(node->inputs->buffer[j]->name, name)) {
					found = true;
					break;
				}
			}
			if (!found) {
				break;
			}
			name = string("%s %s", base, i32_to_string(i));
		}
	}
	ui_node_socket_t *soc = nodes_material_create_socket(NULL, node, string_copy(name), type, canvas, 0.0, 1.0, NULL);
	soc->display          = string_equals(type, "VECTOR") ? 1 : 0;
	any_array_push(node->inputs, soc);
	return soc;
}

static void nodes_mesh_script_add_menu_draw() {
	ui_node_t *node    = nodes_mesh_socket_node;
	char      *names[] = {tr("RGBA"), tr("Vector"), tr("Value")};
	char      *types[] = {"RGBA", "VECTOR", "VALUE"};
	for (i32 i = 0; i < 3; ++i) {
		if (ui_menu_button(names[i], "", ICON_NONE)) {
			ui_nodes_push_undo(NULL);
			nodes_mesh_script_add_socket(ui_nodes_get_canvas(true), node, types[i], NULL);
			ui_nodes_canvas_changed();
		}
	}
}

static void nodes_mesh_script_add_button(i32 node_id) {
	if (ui_icon_button(tr("Add"), ICON_PLUS, UI_ALIGN_CENTER)) {
		nodes_mesh_socket_node = ui_get_node(ui_nodes_get_canvas(true)->nodes, node_id);
		ui_menu_draw(&nodes_mesh_script_add_menu_draw, -1, -1);
	}
}

// Node definitions

static ui_node_socket_t *nodes_mesh_socket(char *name, char *type, f32_array_t *value, f32 min, f32 max, f32 precision) {
	bool is_mesh = string_equals(type, "MESH");
	return ALLOC_INIT(ui_node_socket_t, {.name          = name,
	                                     .type          = type,
	                                     .color         = is_mesh ? NODES_MESH_SOCKET_COL : nodes_material_get_socket_color(type),
	                                     .default_value = value,
	                                     .min           = min,
	                                     .max           = max,
	                                     .precision     = precision,
	                                     .display       = string_equals(type, "VECTOR") ? 1 : 0});
}

static ui_node_t *nodes_mesh_def(ui_node_t_array_t *category, char *name, char *type, u32 color, char *output_type) {
	ui_node_t *n =
	    ALLOC_INIT(ui_node_t,
	               {.name = name, .type = type, .color = color, .inputs = any_array_create(0), .outputs = any_array_create(0), .buttons = any_array_create(0)});
	if (output_type != NULL) {
		char        *output_name = string_equals(output_type, "MESH") ? _tr("Mesh") : string_equals(output_type, "VECTOR") ? _tr("Vector") : _tr("Value");
		f32_array_t *value       = string_equals(output_type, "VECTOR") ? f32_array_create_xyz(0.0, 0.0, 0.0) : f32_array_create_x(0.0);
		any_array_push(n->outputs, nodes_mesh_socket(output_name, output_type, value, 0.0, 1.0, 100));
		n->outputs->buffer[0]->display = 0;
	}
	if (category != NULL) {
		any_array_push(category, n);
	}
	return n;
}

static void nodes_mesh_def_mesh(ui_node_t *n) {
	any_array_push(n->inputs, nodes_mesh_socket(_tr("Mesh"), "MESH", f32_array_create_x(0.0), 0.0, 1.0, 100));
}

static void nodes_mesh_def_value(ui_node_t *n, char *name, f32 value, f32 min, f32 max, f32 precision) {
	any_array_push(n->inputs, nodes_mesh_socket(name, "VALUE", f32_array_create_x(value), min, max, precision));
}

static void nodes_mesh_def_vector(ui_node_t *n, char *name, f32 x, f32 y, f32 z, f32 min, f32 max) {
	any_array_push(n->inputs, nodes_mesh_socket(name, "VECTOR", f32_array_create_xyz(x, y, z), min, max, 100));
}

static void nodes_mesh_def_button(ui_node_t *n, char *name, char *type, f32 value, char *texts) {
	ui_node_button_t *b = ALLOC_INIT(ui_node_button_t, {.name          = name,
	                                                    .type          = type,
	                                                    .output        = 0,
	                                                    .default_value = f32_array_create_x(value),
	                                                    .data          = texts != NULL ? u8_array_create_from_string(texts) : NULL,
	                                                    .min           = 0.0,
	                                                    .max           = 1.0,
	                                                    .precision     = 100,
	                                                    .height        = 0});
	any_array_push(n->buttons, b);
}

void nodes_mesh_init() {
	if (nodes_mesh_list != NULL) {
		return;
	}

	u32 col_input    = 0xffb34f5a;
	u32 col_modifier = 0xff4982a0;
	u32 col_combine  = 0xff448c6d;
	u32 col_utility  = 0xff62676d;

	nodes_mesh_category_input = any_array_create(0);
	nodes_mesh_input_def      = nodes_mesh_def(nodes_mesh_category_input, _tr("Mesh Input"), "MESH_INPUT", col_input, "MESH");
	ui_node_t *n              = nodes_mesh_def(nodes_mesh_category_input, _tr("Shape"), "MESH_SHAPE", col_input, "MESH");
	nodes_mesh_def_button(n, _tr("Shape"), "ENUM", 0, "Box\nSphere\nCylinder\nCone\nTorus\nPlane");
	nodes_mesh_def_vector(n, _tr("Size"), 1.0, 1.0, 1.0, 0.0, 10.0);
	nodes_mesh_def_value(n, _tr("Segments"), 32, 3, 128, 1);

	// Another mesh of the scene, default value holds the object index, uid and the last synced index
	n = nodes_mesh_def(nodes_mesh_category_input, _tr("Mesh"), "MESH_OBJECT", col_input, "MESH");
	nodes_mesh_def_button(n, _tr("Mesh"), "ENUM", 0, NULL);
	n->buttons->buffer[0]->default_value = f32_array_create_xyz(0, -1, 0);

	nodes_mesh_category_modifier = any_array_create(0);
	n                            = nodes_mesh_def(nodes_mesh_category_modifier, _tr("Transform"), "MESH_TRANSFORM", col_modifier, "MESH");
	nodes_mesh_def_mesh(n);
	nodes_mesh_def_vector(n, _tr("Location"), 0.0, 0.0, 0.0, -10.0, 10.0);
	nodes_mesh_def_vector(n, _tr("Rotation"), 0.0, 0.0, 0.0, -180.0, 180.0);
	nodes_mesh_def_vector(n, _tr("Scale"), 1.0, 1.0, 1.0, -10.0, 10.0);

	n = nodes_mesh_def(nodes_mesh_category_modifier, _tr("Array"), "MESH_ARRAY", col_modifier, "MESH");
	nodes_mesh_def_button(n, _tr("Relative Offset"), "BOOL", 1, NULL);
	nodes_mesh_def_mesh(n);
	nodes_mesh_def_value(n, _tr("Count"), 2, 1, 32, 1);
	nodes_mesh_def_vector(n, _tr("Offset"), 1.0, 0.0, 0.0, -10.0, 10.0);

	n = nodes_mesh_def(nodes_mesh_category_modifier, _tr("Mirror"), "MESH_MIRROR", col_modifier, "MESH");
	nodes_mesh_def_button(n, _tr("Axis"), "ENUM", 0, "X\nY\nZ");
	nodes_mesh_def_mesh(n);

	n = nodes_mesh_def(nodes_mesh_category_modifier, _tr("Subdivide"), "MESH_SUBDIVIDE", col_modifier, "MESH");
	nodes_mesh_def_mesh(n);
	nodes_mesh_def_value(n, _tr("Levels"), 1, 1, 4, 1);

	n = nodes_mesh_def(nodes_mesh_category_modifier, _tr("Smooth"), "MESH_SMOOTH", col_modifier, "MESH");
	nodes_mesh_def_mesh(n);
	nodes_mesh_def_value(n, _tr("Iterations"), 1, 1, 8, 1);

	n = nodes_mesh_def(nodes_mesh_category_modifier, _tr("Decimate"), "MESH_DECIMATE", col_modifier, "MESH");
	nodes_mesh_def_mesh(n);
	nodes_mesh_def_value(n, _tr("Ratio"), 0.5, 0.01, 1.0, 100);

	n = nodes_mesh_def(nodes_mesh_category_modifier, _tr("Bevel"), "MESH_BEVEL", col_modifier, "MESH");
	nodes_mesh_def_mesh(n);
	nodes_mesh_def_value(n, _tr("Amount"), 0.1, 0.0, 1.0, 100);

	// Runs a minic script on the mesh
	n = nodes_mesh_def(nodes_mesh_category_modifier, _tr("Script"), "MESH_SCRIPT", col_modifier, "MESH");
	nodes_mesh_def_button(n, "nodes_mesh_script_button", "CUSTOM", 0, "void main() {\n    mesh_geo_t *g = mesh_geo_input();\n    mesh_geo_output(g);\n}");
	n->buttons->buffer[0]->output = -1;
	n->buttons->buffer[0]->height = 6;
	n->width                      = 300;
	nodes_mesh_def_button(n, "nodes_mesh_script_add_button", "CUSTOM", 0, NULL);
	n->buttons->buffer[1]->output = -1;
	n->buttons->buffer[1]->height = 1;
	nodes_mesh_def_mesh(n);
	any_map_set(ui_nodes_custom_buttons, "nodes_mesh_script_button", nodes_mesh_script_button);
	any_map_set(ui_nodes_custom_buttons, "nodes_mesh_script_add_button", nodes_mesh_script_add_button);

	nodes_mesh_category_combine = any_array_create(0);
	n                           = nodes_mesh_def(nodes_mesh_category_combine, _tr("Join"), "MESH_JOIN", col_combine, "MESH");
	nodes_mesh_def_mesh(n);
	nodes_mesh_def_mesh(n);

	n = nodes_mesh_def(nodes_mesh_category_combine, _tr("Boolean"), "MESH_BOOLEAN", col_combine, "MESH");
	nodes_mesh_def_button(n, _tr("Operation"), "ENUM", MESH_BOOLEAN_DIFFERENCE, "Union\nDifference\nIntersect");
	nodes_mesh_def_mesh(n);
	nodes_mesh_def_mesh(n);

	nodes_mesh_category_utilities = any_array_create(0);
	n                             = nodes_mesh_def(nodes_mesh_category_utilities, _tr("Value"), "MESH_VALUE", col_utility, "VALUE");
	nodes_mesh_def_value(n, _tr("Value"), 1.0, -10.0, 10.0, 100);

	n = nodes_mesh_def(nodes_mesh_category_utilities, _tr("Math"), "MESH_MATH", col_utility, "VALUE");
	nodes_mesh_def_button(n, _tr("Operation"), "ENUM", 0,
	                      "Add\nSubtract\nMultiply\nDivide\nPower\nMinimum\nMaximum\nModulo\nSine\nCosine\nAbsolute\nRound\nFloor");
	nodes_mesh_def_value(n, _tr("Value"), 0.5, -10.0, 10.0, 100);
	nodes_mesh_def_value(n, _tr("Value"), 0.5, -10.0, 10.0, 100);

	n = nodes_mesh_def(nodes_mesh_category_utilities, _tr("Vector"), "MESH_VECTOR", col_utility, "VECTOR");
	nodes_mesh_def_value(n, "X", 0.0, -10.0, 10.0, 100);
	nodes_mesh_def_value(n, "Y", 0.0, -10.0, 10.0, 100);
	nodes_mesh_def_value(n, "Z", 0.0, -10.0, 10.0, 100);

	// Placed once per canvas
	nodes_mesh_output_def = nodes_mesh_def(NULL, _tr("Mesh Output"), "MESH_OUTPUT", col_input, NULL);
	nodes_mesh_def_mesh(nodes_mesh_output_def);

	nodes_mesh_categories = any_array_create_from_raw(
	    (void *[]){
	        _tr("Input"),
	        _tr("Modifier"),
	        _tr("Combine"),
	        _tr("Utilities"),
	    },
	    4);

	nodes_mesh_list = any_array_create_from_raw(
	    (void *[]){
	        nodes_mesh_category_input,
	        nodes_mesh_category_modifier,
	        nodes_mesh_category_combine,
	        nodes_mesh_category_utilities,
	    },
	    4);
}

// Geometry

mesh_geo_t *util_geo_create(i32 vertex_count, i32 index_count) {
	mesh_geo_t *g      = calloc(1, sizeof(mesh_geo_t));
	g->vertex_count    = vertex_count;
	g->index_count     = index_count;
	g->vertex_capacity = vertex_count;
	g->index_capacity  = index_count;
	g->pos             = calloc(vertex_count * 3 + 1, sizeof(f32));
	g->nor             = calloc(vertex_count * 3 + 1, sizeof(f32));
	g->tex             = calloc(vertex_count * 2 + 1, sizeof(f32));
	g->ind             = calloc(index_count + 1, sizeof(u32));
	return g;
}

void util_geo_free(mesh_geo_t *g) {
	free(g->pos);
	free(g->nor);
	free(g->tex);
	free(g->ind);
	free(g);
}

static mesh_geo_t *nodes_mesh_geo_from_data(mesh_data_t *md) {
	i16_array_t *va0 = md->vertex_arrays->buffer[0]->values;
	i16_array_t *va1 = md->vertex_arrays->buffer[1]->values;
	i16_array_t *va2 = md->vertex_arrays->length > 2 ? md->vertex_arrays->buffer[2]->values : NULL;
	mesh_geo_t  *g   = util_geo_create(va0->length / 4, md->index_array->length);
	f32          sp  = md->scale_pos / 32767.0;
	f32          st  = md->scale_tex / 32767.0;
	for (i32 i = 0; i < g->vertex_count; ++i) {
		g->pos[i * 3]     = va0->buffer[i * 4] * sp;
		g->pos[i * 3 + 1] = va0->buffer[i * 4 + 1] * sp;
		g->pos[i * 3 + 2] = va0->buffer[i * 4 + 2] * sp;
		g->nor[i * 3]     = va1->buffer[i * 2] / 32767.0;
		g->nor[i * 3 + 1] = va1->buffer[i * 2 + 1] / 32767.0;
		g->nor[i * 3 + 2] = va0->buffer[i * 4 + 3] / 32767.0;
		if (va2 != NULL) {
			g->tex[i * 2]     = va2->buffer[i * 2] * st;
			g->tex[i * 2 + 1] = va2->buffer[i * 2 + 1] * st;
		}
	}
	memcpy(g->ind, md->index_array->buffer, g->index_count * sizeof(u32));
	return g;
}

static i16 nodes_mesh_pack(f32 f) {
	f = f < -1.0 ? -1.0 : (f > 1.0 ? 1.0 : f);
	return (i16)math_round(f * 32767.0);
}

static mesh_data_t *nodes_mesh_geo_to_data(mesh_geo_t *g, char *name) {
	f32 scale_pos = 0.0;
	for (i32 i = 0; i < g->vertex_count * 3; ++i) {
		scale_pos = math_max(scale_pos, math_abs(g->pos[i]));
	}
	if (scale_pos <= 0.0) {
		scale_pos = 1.0;
	}
	f32 scale_tex = 1.0;
	for (i32 i = 0; i < g->vertex_count * 2; ++i) {
		scale_tex = math_max(scale_tex, math_abs(g->tex[i]));
	}

	i16_array_t *va0  = i16_array_create(g->vertex_count * 4);
	i16_array_t *va1  = i16_array_create(g->vertex_count * 2);
	i16_array_t *va2  = i16_array_create(g->vertex_count * 2);
	u32_array_t *inda = u32_array_create(g->index_count);
	for (i32 i = 0; i < g->vertex_count; ++i) {
		va0->buffer[i * 4]     = nodes_mesh_pack(g->pos[i * 3] / scale_pos);
		va0->buffer[i * 4 + 1] = nodes_mesh_pack(g->pos[i * 3 + 1] / scale_pos);
		va0->buffer[i * 4 + 2] = nodes_mesh_pack(g->pos[i * 3 + 2] / scale_pos);
		va0->buffer[i * 4 + 3] = nodes_mesh_pack(g->nor[i * 3 + 2]);
		va1->buffer[i * 2]     = nodes_mesh_pack(g->nor[i * 3]);
		va1->buffer[i * 2 + 1] = nodes_mesh_pack(g->nor[i * 3 + 1]);
		va2->buffer[i * 2]     = nodes_mesh_pack(g->tex[i * 2] / scale_tex);
		va2->buffer[i * 2 + 1] = nodes_mesh_pack(g->tex[i * 2 + 1] / scale_tex);
	}
	memcpy(inda->buffer, g->ind, g->index_count * sizeof(u32));

	mesh_data_t *raw   = ALLOC_INIT(mesh_data_t, {.name          = string_copy(name),
	                                              .vertex_arrays = any_array_create_from_raw(
                                                    (void *[]){
                                                        ALLOC_INIT(vertex_array_t, {.values = va0, .attrib = "pos", .data = "short4norm"}),
                                                        ALLOC_INIT(vertex_array_t, {.values = va1, .attrib = "nor", .data = "short2norm"}),
                                                        ALLOC_INIT(vertex_array_t, {.values = va2, .attrib = "tex", .data = "short2norm"}),
                                                    },
                                                    3),
	                                              .index_array = inda,
	                                              .scale_pos   = scale_pos,
	                                              .scale_tex   = scale_tex});
	mesh_data_t *md    = mesh_data_create(raw);
	md->_->owns_arrays = true;
	return md;
}

static void nodes_mesh_geo_flip(mesh_geo_t *g) {
	for (i32 i = 0; i + 2 < g->index_count; i += 3) {
		u32 t         = g->ind[i + 1];
		g->ind[i + 1] = g->ind[i + 2];
		g->ind[i + 2] = t;
	}
}

static void nodes_mesh_geo_copy_into(mesh_geo_t *dst, mesh_geo_t *src, i32 vertex_offset, i32 index_offset) {
	memcpy(dst->pos + vertex_offset * 3, src->pos, src->vertex_count * 3 * sizeof(f32));
	memcpy(dst->nor + vertex_offset * 3, src->nor, src->vertex_count * 3 * sizeof(f32));
	memcpy(dst->tex + vertex_offset * 2, src->tex, src->vertex_count * 2 * sizeof(f32));
	for (i32 i = 0; i < src->index_count; ++i) {
		dst->ind[index_offset + i] = src->ind[i] + vertex_offset;
	}
}

// Takes ownership of a and b
static mesh_geo_t *nodes_mesh_geo_join(mesh_geo_t *a, mesh_geo_t *b) {
	mesh_geo_t *g = util_geo_create(a->vertex_count + b->vertex_count, a->index_count + b->index_count);
	nodes_mesh_geo_copy_into(g, a, 0, 0);
	nodes_mesh_geo_copy_into(g, b, a->vertex_count, a->index_count);
	util_geo_free(a);
	util_geo_free(b);
	return g;
}

static void nodes_mesh_geo_transform(mesh_geo_t *g, mat4_t m) {
	mat4_t nm = mat4_transpose3(mat4_inv(m));
	for (i32 i = 0; i < g->vertex_count; ++i) {
		vec4_t p          = vec4_apply_mat4((vec4_t){g->pos[i * 3], g->pos[i * 3 + 1], g->pos[i * 3 + 2], 1.0}, m);
		vec4_t n          = vec4_norm(vec4_apply_mat4((vec4_t){g->nor[i * 3], g->nor[i * 3 + 1], g->nor[i * 3 + 2], 0.0}, nm));
		g->pos[i * 3]     = p.x;
		g->pos[i * 3 + 1] = p.y;
		g->pos[i * 3 + 2] = p.z;
		g->nor[i * 3]     = n.x;
		g->nor[i * 3 + 1] = n.y;
		g->nor[i * 3 + 2] = n.z;
	}
	if (mat4_determinant(m) < 0.0) {
		nodes_mesh_geo_flip(g);
	}
}

// Shapes, each one fits into a unit box

static void nodes_mesh_shape_sphere(f32 u, f32 v, f32 *pos, f32 *nor) {
	f32 theta = u * 2.0 * math_pi();
	f32 phi   = (1.0 - v) * math_pi();
	nor[0]    = math_sin(phi) * math_cos(theta);
	nor[1]    = math_sin(phi) * math_sin(theta);
	nor[2]    = math_cos(phi);
	pos[0]    = nor[0] * 0.5;
	pos[1]    = nor[1] * 0.5;
	pos[2]    = nor[2] * 0.5;
}

static void nodes_mesh_shape_cylinder(f32 u, f32 v, f32 *pos, f32 *nor) {
	f32 theta = u * 2.0 * math_pi();
	nor[0]    = math_cos(theta);
	nor[1]    = math_sin(theta);
	nor[2]    = 0.0;
	pos[0]    = nor[0] * 0.5;
	pos[1]    = nor[1] * 0.5;
	pos[2]    = v - 0.5;
}

static void nodes_mesh_shape_cone(f32 u, f32 v, f32 *pos, f32 *nor) {
	f32 theta = u * 2.0 * math_pi();
	f32 r     = 0.5 * (1.0 - v);
	f32 len   = math_sqrt(1.25);
	nor[0]    = math_cos(theta) / len;
	nor[1]    = math_sin(theta) / len;
	nor[2]    = 0.5 / len;
	pos[0]    = math_cos(theta) * r;
	pos[1]    = math_sin(theta) * r;
	pos[2]    = v - 0.5;
}

static void nodes_mesh_shape_torus(f32 u, f32 v, f32 *pos, f32 *nor) {
	f32 theta = u * 2.0 * math_pi();
	f32 phi   = v * 2.0 * math_pi();
	f32 major = 0.375;
	f32 minor = 0.125;
	nor[0]    = math_cos(phi) * math_cos(theta);
	nor[1]    = math_cos(phi) * math_sin(theta);
	nor[2]    = math_sin(phi);
	pos[0]    = (major + minor * math_cos(phi)) * math_cos(theta);
	pos[1]    = (major + minor * math_cos(phi)) * math_sin(theta);
	pos[2]    = minor * math_sin(phi);
}

static void nodes_mesh_shape_plane(f32 u, f32 v, f32 *pos, f32 *nor) {
	pos[0] = u - 0.5;
	pos[1] = v - 0.5;
	pos[2] = 0.0;
	nor[0] = 0.0;
	nor[1] = 0.0;
	nor[2] = 1.0;
}

static mesh_geo_t *nodes_mesh_shape_grid(i32 nu, i32 nv, void (*f)(f32, f32, f32 *, f32 *)) {
	mesh_geo_t *g = util_geo_create((nu + 1) * (nv + 1), nu * nv * 6);
	for (i32 j = 0; j <= nv; ++j) {
		for (i32 i = 0; i <= nu; ++i) {
			i32 vi = j * (nu + 1) + i;
			f32 u  = i / (f32)nu;
			f32 v  = j / (f32)nv;
			f(u, v, g->pos + vi * 3, g->nor + vi * 3);
			g->tex[vi * 2]     = u;
			g->tex[vi * 2 + 1] = 1.0 - v;
		}
	}
	i32 ii = 0;
	for (i32 j = 0; j < nv; ++j) {
		for (i32 i = 0; i < nu; ++i) {
			u32 a        = j * (nu + 1) + i;
			u32 b        = a + 1;
			u32 c        = b + nu + 1;
			u32 d        = a + nu + 1;
			g->ind[ii++] = a;
			g->ind[ii++] = b;
			g->ind[ii++] = c;
			g->ind[ii++] = a;
			g->ind[ii++] = c;
			g->ind[ii++] = d;
		}
	}
	return g;
}

static mesh_geo_t *nodes_mesh_shape_cap(i32 segments, f32 z) {
	mesh_geo_t *g = util_geo_create(segments + 2, segments * 3);
	for (i32 i = 0; i <= segments + 1; ++i) {
		// Last vertex is the center
		f32 theta         = i / (f32)segments * 2.0 * math_pi();
		f32 r             = i > segments ? 0.0 : 0.5;
		g->pos[i * 3]     = math_cos(theta) * r;
		g->pos[i * 3 + 1] = math_sin(theta) * r;
		g->pos[i * 3 + 2] = z;
		g->nor[i * 3 + 2] = z > 0.0 ? 1.0 : -1.0;
		g->tex[i * 2]     = g->pos[i * 3] + 0.5;
		g->tex[i * 2 + 1] = 0.5 - g->pos[i * 3 + 1];
	}
	for (i32 i = 0; i < segments; ++i) {
		g->ind[i * 3]     = segments + 1;
		g->ind[i * 3 + 1] = z > 0.0 ? i : i + 1;
		g->ind[i * 3 + 2] = z > 0.0 ? i + 1 : i;
	}
	return g;
}

static mesh_geo_t *nodes_mesh_shape_box() {
	mesh_geo_t *g = util_geo_create(24, 36);
	for (i32 face = 0; face < 6; ++face) {
		i32 axis  = face / 2;
		f32 sign  = face % 2 == 0 ? 1.0 : -1.0;
		i32 u     = (axis + 1) % 3;
		i32 v     = (axis + 2) % 3;
		f32 cu[4] = {-0.5, 0.5, 0.5, -0.5};
		f32 cv[4] = {-0.5, -0.5, 0.5, 0.5};
		for (i32 i = 0; i < 4; ++i) {
			i32 vi                = face * 4 + i;
			i32 corner            = sign > 0.0 ? i : 3 - i;
			g->pos[vi * 3 + axis] = 0.5 * sign;
			g->pos[vi * 3 + u]    = cu[corner];
			g->pos[vi * 3 + v]    = cv[corner];
			g->nor[vi * 3 + axis] = sign;
			g->tex[vi * 2]        = cu[corner] + 0.5;
			g->tex[vi * 2 + 1]    = 0.5 - cv[corner];
		}
		u32 quad[6] = {0, 1, 2, 0, 2, 3};
		for (i32 i = 0; i < 6; ++i) {
			g->ind[face * 6 + i] = face * 4 + quad[i];
		}
	}
	return g;
}

static mesh_geo_t *nodes_mesh_shape(i32 shape, f32 *size, i32 segments) {
	segments      = segments < 3 ? 3 : (segments > 512 ? 512 : segments);
	mesh_geo_t *g = NULL;
	if (shape == 1) {
		g = nodes_mesh_shape_grid(segments, math_max(segments / 2, 2), nodes_mesh_shape_sphere);
	}
	else if (shape == 2) {
		g = nodes_mesh_shape_grid(segments, 1, nodes_mesh_shape_cylinder);
		g = nodes_mesh_geo_join(g, nodes_mesh_shape_cap(segments, 0.5));
		g = nodes_mesh_geo_join(g, nodes_mesh_shape_cap(segments, -0.5));
	}
	else if (shape == 3) {
		g = nodes_mesh_shape_grid(segments, 1, nodes_mesh_shape_cone);
		g = nodes_mesh_geo_join(g, nodes_mesh_shape_cap(segments, -0.5));
	}
	else if (shape == 4) {
		g = nodes_mesh_shape_grid(segments, math_max(segments / 2, 3), nodes_mesh_shape_torus);
	}
	else if (shape == 5) {
		g = nodes_mesh_shape_grid(1, 1, nodes_mesh_shape_plane);
	}
	else {
		g = nodes_mesh_shape_box();
	}

	// Drop the collapsed triangles at poles and tips
	i32 index_count = 0;
	for (i32 i = 0; i + 2 < g->index_count; i += 3) {
		f32 *a  = g->pos + g->ind[i] * 3;
		f32 *b  = g->pos + g->ind[i + 1] * 3;
		f32 *c  = g->pos + g->ind[i + 2] * 3;
		f32  cx = (b[1] - a[1]) * (c[2] - a[2]) - (b[2] - a[2]) * (c[1] - a[1]);
		f32  cy = (b[2] - a[2]) * (c[0] - a[0]) - (b[0] - a[0]) * (c[2] - a[2]);
		f32  cz = (b[0] - a[0]) * (c[1] - a[1]) - (b[1] - a[1]) * (c[0] - a[0]);
		if (cx * cx + cy * cy + cz * cz < 1e-16) {
			continue;
		}
		g->ind[index_count++] = g->ind[i];
		g->ind[index_count++] = g->ind[i + 1];
		g->ind[index_count++] = g->ind[i + 2];
	}
	g->index_count = index_count;

	for (i32 i = 0; i < g->vertex_count; ++i) {
		f32 *p   = g->pos + i * 3;
		f32 *n   = g->nor + i * 3;
		f32  len = 0.0;
		for (i32 k = 0; k < 3; ++k) {
			p[k] *= size[k];
			n[k] /= math_max(math_abs(size[k]), 0.0001);
			len += n[k] * n[k];
		}
		len = math_sqrt(len);
		for (i32 k = 0; k < 3; ++k) {
			n[k] /= len;
		}
	}
	return g;
}

// Evaluation

static ui_node_t *nodes_mesh_linked_node(ui_node_canvas_t *c, ui_node_t *node, i32 socket) {
	for (i32 i = 0; i < c->links->length; ++i) {
		ui_node_link_t *l = c->links->buffer[i];
		if (l->to_id == node->id && l->to_socket == socket) {
			return ui_get_node(c->nodes, l->from_id);
		}
	}
	return NULL;
}

static i32 nodes_mesh_button(ui_node_t *node, i32 button);

static f32 nodes_mesh_math(i32 op, f32 a, f32 b) {
	switch (op) {
	case 1:
		return a - b;
	case 2:
		return a * b;
	case 3:
		return a / (b == 0.0 ? 0.000001 : b);
	case 4:
		return math_pow(a, b);
	case 5:
		return math_min(a, b);
	case 6:
		return math_max(a, b);
	case 7:
		return b == 0.0 ? 0.0 : math_fmod(a, b);
	case 8:
		return math_sin(a);
	case 9:
		return math_cos(a);
	case 10:
		return math_abs(a);
	case 11:
		return math_round(a);
	case 12:
		return math_floor(a);
	default:
		return a + b;
	}
}

// Socket value, taken from the linked value, math or vector node when there is one
static vec4_t nodes_mesh_vector(ui_node_t *node, i32 socket) {
	ui_node_socket_t *s      = node->inputs->buffer[socket];
	f32              *v      = s->default_value->buffer;
	vec4_t            result = s->default_value->length > 2 ? (vec4_t){v[0], v[1], v[2], 1.0} : (vec4_t){v[0], v[0], v[0], 1.0};
	ui_node_t        *from   = nodes_mesh_linked_node(nodes_mesh_eval_canvas, node, socket);
	if (from == NULL || nodes_mesh_eval_depth >= NODES_MESH_MAX_DEPTH) {
		return result;
	}
	nodes_mesh_eval_depth++;
	if (string_equals(from->type, "MESH_VALUE")) {
		result = nodes_mesh_vector(from, 0);
	}
	else if (string_equals(from->type, "MESH_MATH")) {
		f32 f  = nodes_mesh_math(nodes_mesh_button(from, 0), nodes_mesh_vector(from, 0).x, nodes_mesh_vector(from, 1).x);
		result = (vec4_t){f, f, f, 1.0};
	}
	else if (string_equals(from->type, "MESH_VECTOR")) {
		result = (vec4_t){nodes_mesh_vector(from, 0).x, nodes_mesh_vector(from, 1).x, nodes_mesh_vector(from, 2).x, 1.0};
	}
	nodes_mesh_eval_depth--;
	return result;
}

static f32 nodes_mesh_value(ui_node_t *node, i32 socket) {
	return nodes_mesh_vector(node, socket).x;
}

static i32 nodes_mesh_button(ui_node_t *node, i32 button) {
	return (i32)node->buttons->buffer[button]->default_value->buffer[0];
}

static mesh_object_t *nodes_mesh_object(i32 uid);

static bool nodes_mesh_has_object_nodes(ui_node_canvas_t *c) {
	for (i32 i = 0; i < c->nodes->length; ++i) {
		if (string_equals(c->nodes->buffer[i]->type, "MESH_OBJECT")) {
			return true;
		}
	}
	return false;
}

static mesh_nodes_t *nodes_mesh_find(i32 uid) {
	for (i32 i = 0; nodes_mesh_slots != NULL && i < nodes_mesh_slots->length; ++i) {
		mesh_nodes_t *s = nodes_mesh_slots->buffer[i];
		if (s->uid == uid) {
			return s;
		}
	}
	return NULL;
}

static mesh_object_t *nodes_mesh_node_object(ui_node_t *n) {
	f32_array_t *v = n->buttons->buffer[0]->default_value;
	return v->length > 2 && v->buffer[1] >= 0 ? nodes_mesh_object((i32)v->buffer[1]) : NULL;
}

// Scene mesh nodes follow their object when meshes are reordered
static void nodes_mesh_sync_objects(ui_node_canvas_t *c) {
	mesh_object_t_array_t *objects = g_project->_->paint_objects;
	for (i32 i = 0; i < c->nodes->length; ++i) {
		ui_node_t *n = c->nodes->buffer[i];
		if (!string_equals(n->type, "MESH_OBJECT")) {
			continue;
		}
		ui_node_button_t *b = n->buttons->buffer[0];
		if (b->default_value->length < 3) {
			b->default_value = f32_array_create_xyz(b->default_value->buffer[0], -1, b->default_value->buffer[0]);
		}
		f32 *v     = b->default_value->buffer;
		i32  index = (i32)v[0];
		if (v[1] < 0 || index != (i32)v[2]) {
			// Loaded from a file or picked by the user
			v[1] = index >= 0 && index < objects->length ? objects->buffer[index]->base->uid : -1;
		}
		else {
			mesh_object_t *o = nodes_mesh_object((i32)v[1]);
			if (o != NULL) {
				index = array_index_of(objects, o);
			}
		}
		v[0] = index;
		v[2] = index;
	}
}

// Whether the nodes of an object use the mesh of the target object
static bool nodes_mesh_reaches(mesh_object_t *o, i32 target_uid, i32 depth) {
	if (o->base->uid == target_uid) {
		return true;
	}
	mesh_nodes_t *slot = nodes_mesh_find(o->base->uid);
	if (slot == NULL || depth > NODES_MESH_MAX_DEPTH) {
		return false;
	}
	for (i32 i = 0; i < slot->canvas->nodes->length; ++i) {
		ui_node_t *n = slot->canvas->nodes->buffer[i];
		if (!string_equals(n->type, "MESH_OBJECT")) {
			continue;
		}
		mesh_object_t *ref = nodes_mesh_node_object(n);
		if (ref != NULL && nodes_mesh_reaches(ref, target_uid, depth + 1)) {
			return true;
		}
	}
	return false;
}

// Meshes which depend on each other use the original mesh instead of the output
static mesh_data_t *nodes_mesh_object_data(mesh_object_t *self, mesh_object_t *ref) {
	mesh_nodes_t *slot = nodes_mesh_find(ref->base->uid);
	if (slot != NULL && slot->output != slot->input && nodes_mesh_reaches(ref, self->base->uid, 0)) {
		return slot->input;
	}
	return ref->data;
}

static mat4_t nodes_mesh_object_matrix(mesh_object_t *self, mesh_object_t *ref) {
	return mat4_mult_mat(ref->base->transform->world, mat4_inv(self->base->transform->world));
}

static mesh_geo_t *nodes_mesh_eval_input(ui_node_t *node, i32 socket) {
	ui_node_t *from = nodes_mesh_linked_node(nodes_mesh_eval_canvas, node, socket);
	if (from == NULL || from->outputs->length == 0 || !string_equals(from->outputs->buffer[0]->type, "MESH")) {
		return util_geo_create(0, 0);
	}
	return nodes_mesh_eval_node(from);
}

static mesh_geo_t *nodes_mesh_geo_modify(mesh_geo_t *g, char *type, f32 value) {
	mesh_object_t *o = nodes_mesh_eval_object != NULL ? nodes_mesh_eval_object : g_context->paint_object;
	if (g->index_count == 0 || o == NULL) {
		return g;
	}

	mesh_object_t temp = *o;
	temp.data          = nodes_mesh_geo_to_data(g, o->base->name);
	util_geo_free(g);

	// Modifiers swap the data of the object and free the previous one on the next frame
	bool smooth = true;
	if (string_equals(type, "MESH_SUBDIVIDE")) {
		i32 levels = math_floor(value);
		for (i32 i = 0; i < levels && i < 6; ++i) {
			_util_mesh_subdivide(&temp);
		}
	}
	else if (string_equals(type, "MESH_SMOOTH")) {
		i32 iterations = math_floor(value);
		for (i32 i = 0; i < iterations && i < 32; ++i) {
			_util_mesh_smooth(&temp);
		}
	}
	else if (string_equals(type, "MESH_DECIMATE")) {
		if (value < 1.0) {
			_util_mesh_decimate(&temp, math_max(value, 0.01));
		}
	}
	else if (string_equals(type, "MESH_BEVEL")) {
		if (value > 0.0) {
			_util_mesh_bevel(&temp, value);
		}
	}
	else {
		smooth = value > 0.0; // Normals only
	}
	util_mesh_calc_normals(any_array_create_from_raw_tmp((void *[]){&temp}, 1), smooth);

	g = nodes_mesh_geo_from_data(temp.data);
	util_mesh_delete_data_uncache(temp.data);
	return g;
}

// Takes ownership of g
static mesh_geo_t *nodes_mesh_geo_array(mesh_geo_t *g, i32 count, vec4_t offset, bool relative) {
	count = count < 1 ? 1 : (count > 256 ? 256 : count);
	if (count == 1 || g->vertex_count == 0) {
		return g;
	}

	if (relative) {
		f32 mn[3] = {FLT_MAX, FLT_MAX, FLT_MAX};
		f32 mx[3] = {-FLT_MAX, -FLT_MAX, -FLT_MAX};
		for (i32 i = 0; i < g->vertex_count * 3; ++i) {
			mn[i % 3] = math_min(mn[i % 3], g->pos[i]);
			mx[i % 3] = math_max(mx[i % 3], g->pos[i]);
		}
		offset.x *= mx[0] - mn[0];
		offset.y *= mx[1] - mn[1];
		offset.z *= mx[2] - mn[2];
	}

	mesh_geo_t *out = util_geo_create(g->vertex_count * count, g->index_count * count);
	for (i32 c = 0; c < count; ++c) {
		i32 vo = g->vertex_count * c;
		nodes_mesh_geo_copy_into(out, g, vo, g->index_count * c);
		for (i32 i = 0; i < g->vertex_count; ++i) {
			out->pos[(vo + i) * 3] += offset.x * c;
			out->pos[(vo + i) * 3 + 1] += offset.y * c;
			out->pos[(vo + i) * 3 + 2] += offset.z * c;
		}
	}
	util_geo_free(g);
	return out;
}

// Takes ownership of g
static mesh_geo_t *nodes_mesh_geo_mirror(mesh_geo_t *g, i32 axis) {
	axis          = axis < 0 ? 0 : (axis > 2 ? 2 : axis);
	mesh_geo_t *m = util_geo_create(g->vertex_count, g->index_count);
	nodes_mesh_geo_copy_into(m, g, 0, 0);
	for (i32 i = 0; i < m->vertex_count; ++i) {
		m->pos[i * 3 + axis] = -m->pos[i * 3 + axis];
		m->nor[i * 3 + axis] = -m->nor[i * 3 + axis];
	}
	nodes_mesh_geo_flip(m);
	return nodes_mesh_geo_join(g, m);
}

// Takes ownership of a and b
static mesh_geo_t *nodes_mesh_geo_boolean(mesh_geo_t *a, mesh_geo_t *b, i32 op) {
	if (a->index_count == 0 || b->index_count == 0) {
		// Nothing to cut
		bool keep_a = op == MESH_BOOLEAN_DIFFERENCE || (op == MESH_BOOLEAN_UNION && b->index_count == 0);
		bool keep_b = op == MESH_BOOLEAN_UNION && !keep_a;
		if (!keep_a && !keep_b) {
			util_geo_free(b);
			b = util_geo_create(0, 0);
		}
		util_geo_free(keep_a ? b : a);
		return keep_a ? a : b;
	}
	mesh_geo_t *g = util_csg_boolean(a, b, op);
	util_geo_free(a);
	util_geo_free(b);
	nodes_mesh_eval_unwrap = true;
	return g;
}

static mesh_geo_t *nodes_mesh_geo_transform_euler(mesh_geo_t *g, vec4_t loc, vec4_t rot, vec4_t scale) {
	rot     = vec4_mult(rot, math_pi() / 180.0);
	scale.x = math_abs(scale.x) < 0.0001 ? 0.0001 : scale.x;
	scale.y = math_abs(scale.y) < 0.0001 ? 0.0001 : scale.y;
	scale.z = math_abs(scale.z) < 0.0001 ? 0.0001 : scale.z;
	nodes_mesh_geo_transform(g, mat4_compose(loc, quat_from_euler(rot.x, rot.y, rot.z), scale));
	return g;
}

// Script

static mesh_geo_t *nodes_mesh_script_track(mesh_geo_t *g) {
	if (!nodes_mesh_script_running) {
		return g; // Called from another script, left to the caller
	}
	if (nodes_mesh_script_count == nodes_mesh_script_cap) {
		nodes_mesh_script_cap  = nodes_mesh_script_cap == 0 ? 16 : nodes_mesh_script_cap * 2;
		nodes_mesh_script_geos = realloc(nodes_mesh_script_geos, nodes_mesh_script_cap * sizeof(mesh_geo_t *));
	}
	nodes_mesh_script_geos[nodes_mesh_script_count++] = g;
	return g;
}

// Copy of a mesh handed over by a script, with out of range counts, indices and values dropped
static mesh_geo_t *nodes_mesh_script_arg(mesh_geo_t *g) {
	if (g == NULL || g->pos == NULL || g->nor == NULL || g->tex == NULL || g->ind == NULL) {
		return util_geo_create(0, 0);
	}
	i32         vertex_count = g->vertex_count < 0 ? 0 : (g->vertex_count > g->vertex_capacity ? g->vertex_capacity : g->vertex_count);
	i32         index_count  = g->index_count < 0 ? 0 : (g->index_count > g->index_capacity ? g->index_capacity : g->index_count);
	mesh_geo_t *c            = util_geo_create(vertex_count, index_count - index_count % 3);
	for (i32 i = 0; i < vertex_count * 3; ++i) {
		c->pos[i] = isfinite(g->pos[i]) ? g->pos[i] : 0.0;
		c->nor[i] = isfinite(g->nor[i]) ? g->nor[i] : 0.0;
	}
	for (i32 i = 0; i < vertex_count * 2; ++i) {
		c->tex[i] = isfinite(g->tex[i]) ? g->tex[i] : 0.0;
	}
	i32 count = 0;
	for (i32 i = 0; i + 2 < index_count; i += 3) {
		if (g->ind[i] < (u32)vertex_count && g->ind[i + 1] < (u32)vertex_count && g->ind[i + 2] < (u32)vertex_count) {
			c->ind[count++] = g->ind[i];
			c->ind[count++] = g->ind[i + 1];
			c->ind[count++] = g->ind[i + 2];
		}
	}
	c->index_count = count;
	return c;
}

// Mesh the script node received, edited in place or replaced with mesh_geo_output()
mesh_geo_t *mesh_geo_input() {
	return nodes_mesh_script_in != NULL ? nodes_mesh_script_in : util_geo_create(0, 0);
}

void mesh_geo_output(mesh_geo_t *g) {
	if (nodes_mesh_script_running && g != NULL) {
		nodes_mesh_script_out = g;
	}
}

// The functions below leave their arguments as they are and return a new mesh
mesh_geo_t *mesh_geo_create(i32 vertex_count, i32 index_count) {
	return nodes_mesh_script_track(util_geo_create(vertex_count < 0 ? 0 : vertex_count, index_count < 0 ? 0 : index_count));
}

mesh_geo_t *mesh_geo_copy(mesh_geo_t *g) {
	return nodes_mesh_script_track(nodes_mesh_script_arg(g));
}

mesh_geo_t *mesh_geo_shape(i32 shape, f32 size_x, f32 size_y, f32 size_z, i32 segments) {
	f32 size[] = {size_x, size_y, size_z};
	return nodes_mesh_script_track(nodes_mesh_shape(shape, size, segments));
}

mesh_geo_t *mesh_geo_transform(mesh_geo_t *g, f32 loc_x, f32 loc_y, f32 loc_z, f32 rot_x, f32 rot_y, f32 rot_z, f32 scale_x, f32 scale_y, f32 scale_z) {
	vec4_t loc   = {loc_x, loc_y, loc_z, 1.0};
	vec4_t rot   = {rot_x, rot_y, rot_z, 1.0};
	vec4_t scale = {scale_x, scale_y, scale_z, 1.0};
	return nodes_mesh_script_track(nodes_mesh_geo_transform_euler(nodes_mesh_script_arg(g), loc, rot, scale));
}

mesh_geo_t *mesh_geo_array(mesh_geo_t *g, i32 count, f32 offset_x, f32 offset_y, f32 offset_z, bool relative) {
	vec4_t offset = {offset_x, offset_y, offset_z, 1.0};
	return nodes_mesh_script_track(nodes_mesh_geo_array(nodes_mesh_script_arg(g), count, offset, relative));
}

mesh_geo_t *mesh_geo_mirror(mesh_geo_t *g, i32 axis) {
	return nodes_mesh_script_track(nodes_mesh_geo_mirror(nodes_mesh_script_arg(g), axis));
}

mesh_geo_t *mesh_geo_join(mesh_geo_t *a, mesh_geo_t *b) {
	mesh_geo_t *ca = nodes_mesh_script_arg(a);
	return nodes_mesh_script_track(nodes_mesh_geo_join(ca, nodes_mesh_script_arg(b)));
}

mesh_geo_t *mesh_geo_boolean(mesh_geo_t *a, mesh_geo_t *b, i32 op) {
	mesh_geo_t *ca = nodes_mesh_script_arg(a);
	return nodes_mesh_script_track(nodes_mesh_geo_boolean(ca, nodes_mesh_script_arg(b), op));
}

static mesh_geo_t *nodes_mesh_script_modify(mesh_geo_t *g, char *type, f32 value) {
	nodes_mesh_eval_unwrap = nodes_mesh_eval_unwrap || !string_equals(type, "MESH_NORMALS");
	return nodes_mesh_script_track(nodes_mesh_geo_modify(nodes_mesh_script_arg(g), type, value));
}

mesh_geo_t *mesh_geo_subdivide(mesh_geo_t *g, i32 levels) {
	return nodes_mesh_script_modify(g, "MESH_SUBDIVIDE", levels);
}

mesh_geo_t *mesh_geo_smooth(mesh_geo_t *g, i32 iterations) {
	return nodes_mesh_script_modify(g, "MESH_SMOOTH", iterations);
}

mesh_geo_t *mesh_geo_decimate(mesh_geo_t *g, f32 ratio) {
	return nodes_mesh_script_modify(g, "MESH_DECIMATE", ratio);
}

mesh_geo_t *mesh_geo_bevel(mesh_geo_t *g, f32 amount) {
	return nodes_mesh_script_modify(g, "MESH_BEVEL", amount);
}

mesh_geo_t *mesh_geo_normals(mesh_geo_t *g, bool smooth) {
	return nodes_mesh_script_modify(g, "MESH_NORMALS", smooth ? 1.0 : 0.0);
}

// Input added to the script node, linked value nodes are followed
static bool nodes_mesh_script_socket(char *name, i32 i, f32 *result) {
	ui_node_t *node = nodes_mesh_script_node;
	if (node == NULL || name == NULL || i < 0 || i > 3) {
		return false;
	}
	for (i32 j = 1; j < node->inputs->length; ++j) {
		ui_node_socket_t *s = node->inputs->buffer[j];
		if (!string_equals(s->name, name) || string_equals(s->type, "MESH")) {
			continue;
		}
		vec4_t v = nodes_mesh_vector(node, j);
		if (s->default_value->length > 3 && nodes_mesh_linked_node(nodes_mesh_eval_canvas, node, j) == NULL) {
			v.w = s->default_value->buffer[3]; // Alpha
		}
		*result = i == 0 ? v.x : i == 1 ? v.y : i == 2 ? v.z : v.w;
		return true;
	}
	return false;
}

f32 mesh_geo_input_value(char *name) {
	f32 f = 0.0;
	nodes_mesh_script_socket(name, 0, &f);
	return f;
}

// Component of a vector (0 - 2) or color (0 - 3) input
f32 mesh_geo_input_vector(char *name, i32 i) {
	f32 f = 0.0;
	nodes_mesh_script_socket(name, i, &f);
	return f;
}

static mesh_geo_t *nodes_mesh_eval_script(ui_node_t *node) {
	mesh_geo_t *g    = nodes_mesh_eval_input(node, 0);
	char       *text = nodes_mesh_script_text(node);
	if (!project_scripts_trusted || string_equals(text, "")) {
		return g;
	}

	// The script works on a copy, the input passes through when it fails
	nodes_mesh_script_running = true;
	nodes_mesh_script_count   = 0;
	nodes_mesh_script_in      = nodes_mesh_script_track(nodes_mesh_script_arg(g));
	nodes_mesh_script_out     = nodes_mesh_script_in;
	nodes_mesh_script_node    = node;

	i32 errors = minic_error_count;
	minic_ctx_free(minic_eval_named(text, "mesh script"));
	if (minic_error_count == errors) {
		util_geo_free(g);
		g = nodes_mesh_script_arg(nodes_mesh_script_out);
	}

	for (i32 i = 0; i < nodes_mesh_script_count; ++i) {
		util_geo_free(nodes_mesh_script_geos[i]);
	}
	nodes_mesh_script_count   = 0;
	nodes_mesh_script_in      = NULL;
	nodes_mesh_script_out     = NULL;
	nodes_mesh_script_node    = NULL;
	nodes_mesh_script_running = false;
	return g;
}

static mesh_geo_t *nodes_mesh_eval_node(ui_node_t *node) {
	if (++nodes_mesh_eval_depth > NODES_MESH_MAX_DEPTH) {
		return util_geo_create(0, 0); // Links form a loop
	}

	mesh_geo_t *g = NULL;
	if (string_equals(node->type, "MESH_INPUT")) {
		g = nodes_mesh_geo_from_data(nodes_mesh_eval_slot->input);
	}
	else if (string_equals(node->type, "MESH_OBJECT")) {
		mesh_object_t *self = nodes_mesh_eval_object;
		mesh_object_t *ref  = nodes_mesh_node_object(node);
		if (ref == NULL) {
			g = util_geo_create(0, 0);
		}
		else {
			g = nodes_mesh_geo_from_data(nodes_mesh_object_data(self, ref));
			if (ref != self) {
				nodes_mesh_geo_transform(g, nodes_mesh_object_matrix(self, ref));
			}
		}
	}
	else if (string_equals(node->type, "MESH_SHAPE")) {
		vec4_t size    = nodes_mesh_vector(node, 0);
		f32    sizes[] = {size.x, size.y, size.z};
		g              = nodes_mesh_shape(nodes_mesh_button(node, 0), sizes, math_floor(nodes_mesh_value(node, 1)));
	}
	else if (string_equals(node->type, "MESH_TRANSFORM")) {
		g = nodes_mesh_eval_input(node, 0);
		nodes_mesh_geo_transform_euler(g, nodes_mesh_vector(node, 1), nodes_mesh_vector(node, 2), nodes_mesh_vector(node, 3));
	}
	else if (string_equals(node->type, "MESH_ARRAY")) {
		g = nodes_mesh_eval_input(node, 0);
		g = nodes_mesh_geo_array(g, math_floor(nodes_mesh_value(node, 1)), nodes_mesh_vector(node, 2), nodes_mesh_button(node, 0) > 0);
	}
	else if (string_equals(node->type, "MESH_MIRROR")) {
		g = nodes_mesh_geo_mirror(nodes_mesh_eval_input(node, 0), nodes_mesh_button(node, 0));
	}
	else if (string_equals(node->type, "MESH_JOIN")) {
		mesh_geo_t *a = nodes_mesh_eval_input(node, 0);
		g             = nodes_mesh_geo_join(a, nodes_mesh_eval_input(node, 1));
	}
	else if (string_equals(node->type, "MESH_BOOLEAN")) {
		mesh_geo_t *a = nodes_mesh_eval_input(node, 0);
		g             = nodes_mesh_geo_boolean(a, nodes_mesh_eval_input(node, 1), nodes_mesh_button(node, 0));
	}
	else if (string_equals(node->type, "MESH_SUBDIVIDE") || string_equals(node->type, "MESH_SMOOTH") || string_equals(node->type, "MESH_DECIMATE") ||
	         string_equals(node->type, "MESH_BEVEL")) {
		g = nodes_mesh_eval_input(node, 0);
		if (g->index_count > 0) {
			g                      = nodes_mesh_geo_modify(g, node->type, nodes_mesh_value(node, 1));
			nodes_mesh_eval_unwrap = true;
		}
	}
	else if (string_equals(node->type, "MESH_SCRIPT")) {
		g = nodes_mesh_eval_script(node);
	}
	else {
		g = util_geo_create(0, 0);
	}

	nodes_mesh_eval_depth--;
	return g;
}

// Returns NULL when the original mesh is used as is
static mesh_data_t *nodes_mesh_eval(mesh_nodes_t *slot, mesh_object_t *o) {
	ui_node_t *output = NULL;
	for (i32 i = 0; i < slot->canvas->nodes->length; ++i) {
		ui_node_t *n = slot->canvas->nodes->buffer[i];
		if (string_equals(n->type, "MESH_OUTPUT")) {
			output = n;
			break;
		}
	}
	if (output == NULL) {
		return NULL;
	}
	ui_node_t *from = nodes_mesh_linked_node(slot->canvas, output, 0);
	if (from == NULL || from->outputs->length == 0 || string_equals(from->type, "MESH_INPUT")) {
		return NULL;
	}

	nodes_mesh_eval_canvas = slot->canvas;
	nodes_mesh_eval_slot   = slot;
	nodes_mesh_eval_object = o;
	nodes_mesh_eval_unwrap = false;
	nodes_mesh_eval_depth  = 0;
	mesh_geo_t *g          = nodes_mesh_eval_node(from);
	nodes_mesh_eval_object = NULL;

	if (g->index_count == 0) {
		// Keep the vertex buffers valid
		util_geo_free(g);
		g         = util_geo_create(3, 3);
		g->ind[1] = 1;
		g->ind[2] = 2;
		g->nor[2] = g->nor[5] = g->nor[8] = 1.0;
		nodes_mesh_eval_unwrap            = false;
	}

	mesh_data_t *md = nodes_mesh_geo_to_data(g, o->base->name);
	util_geo_free(g);
	return md;
}

static u32 nodes_mesh_hash_bytes(u32 hash, void *data, i32 length) {
	for (i32 i = 0; i < length; ++i) {
		hash = (hash ^ ((unsigned char *)data)[i]) * 16777619u;
	}
	return hash;
}

// Covers what changes the result, node positions are left out
static u32 nodes_mesh_hash(ui_node_canvas_t *c, mesh_object_t *o) {
	u32 hash = 2166136261u;
	for (i32 i = 0; i < c->nodes->length; ++i) {
		ui_node_t *n = c->nodes->buffer[i];
		hash         = nodes_mesh_hash_bytes(hash, &n->id, sizeof(n->id));
		hash         = nodes_mesh_hash_bytes(hash, n->type, string_length(n->type));
		if (string_equals(n->type, "MESH_OBJECT")) {
			// Rebuild when the other mesh changes or either of the two moves
			mesh_object_t *ref = nodes_mesh_node_object(n);
			if (ref != NULL) {
				mesh_data_t *data  = nodes_mesh_object_data(o, ref);
				mat4_t       m     = nodes_mesh_object_matrix(o, ref);
				i32          count = data->index_array->length;
				hash               = nodes_mesh_hash_bytes(hash, &data, sizeof(data));
				hash               = nodes_mesh_hash_bytes(hash, &count, sizeof(count));
				hash               = nodes_mesh_hash_bytes(hash, m.m, sizeof(f32) * 16);
			}
			hash = nodes_mesh_hash_bytes(hash, &ref, sizeof(ref));
			continue;
		}
		for (i32 j = 0; j < n->inputs->length; ++j) {
			f32_array_t *v = n->inputs->buffer[j]->default_value;
			if (j > 0 && string_equals(n->type, "MESH_SCRIPT")) {
				char *name = n->inputs->buffer[j]->name; // Scripts read their inputs by name
				hash       = nodes_mesh_hash_bytes(hash, name, string_length(name));
			}
			if (v != NULL) {
				hash = nodes_mesh_hash_bytes(hash, v->buffer, v->length * sizeof(f32));
			}
		}
		for (i32 j = 0; j < n->buttons->length; ++j) {
			f32_array_t *v = n->buttons->buffer[j]->default_value;
			if (v != NULL && v->length > 0) {
				hash = nodes_mesh_hash_bytes(hash, v->buffer, sizeof(f32));
			}
			u8_array_t *data = n->buttons->buffer[j]->data;
			if (data != NULL) {
				hash = nodes_mesh_hash_bytes(hash, data->buffer, data->length);
			}
		}
	}
	for (i32 i = 0; i < c->links->length; ++i) {
		ui_node_link_t *l = c->links->buffer[i];
		hash              = nodes_mesh_hash_bytes(hash, &l->from_id, sizeof(i32) * 4);
	}
	return hash;
}

// Slots

static mesh_object_t *nodes_mesh_object(i32 uid) {
	if (g_project->_->paint_objects == NULL) {
		return NULL;
	}
	for (i32 i = 0; i < g_project->_->paint_objects->length; ++i) {
		if (g_project->_->paint_objects->buffer[i]->base->uid == uid) {
			return g_project->_->paint_objects->buffer[i];
		}
	}
	return NULL;
}

static bool nodes_mesh_is_default(ui_node_canvas_t *c) {
	return c->nodes->length == 2 && c->links->length == 1;
}

static ui_node_canvas_t *nodes_mesh_default_canvas(ui_nodes_t *nodes) {
	nodes_mesh_init();
	ui_node_canvas_t *c  = ALLOC_INIT(ui_node_canvas_t, {.name = "", .nodes = any_array_create(0), .links = any_array_create(0)});
	ui_node_t        *in = ui_nodes_make_node(nodes_mesh_input_def, nodes, c);
	in->x                = 100;
	in->y                = 200;
	any_array_push(c->nodes, in);
	ui_node_t *out = ui_nodes_make_node(nodes_mesh_output_def, nodes, c);
	out->x         = 420;
	out->y         = 200;
	any_array_push(c->nodes, out);
	ui_node_link_t *l = ALLOC_INIT(ui_node_link_t, {.id = 0, .from_id = in->id, .from_socket = 0, .to_id = out->id, .to_socket = 0});
	any_array_push(c->links, l);
	ui_nodes_node_changed = NULL;
	return c;
}

mesh_nodes_t *nodes_mesh_get(mesh_object_t *o, bool create) {
	if (o == NULL) {
		return NULL;
	}
	mesh_nodes_t *slot = NULL;
	if (nodes_mesh_slots != NULL) {
		for (i32 i = 0; i < nodes_mesh_slots->length; ++i) {
			mesh_nodes_t *s = nodes_mesh_slots->buffer[i];
			if (s->uid == o->base->uid) {
				slot = s;
				break;
			}
		}
	}
	if (slot == NULL) {
		if (!create) {
			return NULL;
		}
		if (nodes_mesh_slots == NULL) {
			nodes_mesh_slots = any_array_create(0);
		}
		slot         = ALLOC_INIT(mesh_nodes_t, {.uid = o->base->uid, .input = o->data, .output = o->data});
		slot->nodes  = ui_nodes_create();
		slot->canvas = nodes_mesh_default_canvas(slot->nodes);
		slot->hash   = nodes_mesh_hash(slot->canvas, o);
		any_array_push(nodes_mesh_slots, slot);
	}
	if (slot->output == slot->input) {
		// The original mesh is in use, mesh tab operations may have replaced it
		slot->input  = o->data;
		slot->output = o->data;
	}
	slot->canvas->name = o->base->name;
	nodes_mesh_sync_objects(slot->canvas);
	return slot;
}

// The original mesh is kept in memory while the object shows the output mesh
bool nodes_mesh_holds(mesh_data_t *data) {
	if (nodes_mesh_slots == NULL) {
		return false;
	}
	for (i32 i = 0; i < nodes_mesh_slots->length; ++i) {
		mesh_nodes_t *s = nodes_mesh_slots->buffer[i];
		if (s->input == data && s->output != s->input) {
			return true;
		}
	}
	return false;
}

void nodes_mesh_update(mesh_object_t *o, bool force) {
	mesh_nodes_t *slot = nodes_mesh_get(o, false);
	if (slot == NULL || nodes_mesh_script_running) {
		return;
	}
	u32 hash = nodes_mesh_hash(slot->canvas, o);
	if (!force && hash == slot->hash) {
		return;
	}
	slot->hash = hash;

	mesh_data_t *out = nodes_mesh_eval(slot, o);
	if (out == NULL) {
		out = slot->input;
	}
	mesh_data_t *old = o->data;
	if (out == old) {
		return;
	}

	bool shared = util_mesh_data_is_shared(old);
	mesh_object_set_data(o, out);
	slot->output = out;
	transform_build_matrix(o->base->transform);
	nodes_mesh_refresh_pending = true; // Meshes using this one
	if (old != slot->input && !shared) {
		sys_notify_on_next_frame(&util_mesh_delete_data_uncache, old);
	}

	if (nodes_mesh_eval_unwrap && out != slot->input) {
		util_mesh_uv_unwrap(any_array_create_from_raw_tmp((void *[]){o}, 1));
	}
	else if (g_context->merged_object != NULL || g_project->_->paint_objects->length > 1) {
		util_mesh_merge(NULL);
	}

	char          *uid_key = i32_to_string(o->base->uid);
	gpu_texture_t *preview = tab_meshes_preview_map != NULL ? any_map_get(tab_meshes_preview_map, uid_key) : NULL;
	if (preview != NULL) {
		gpu_delete_texture(preview);
		map_delete(tab_meshes_preview_map, uid_key);
	}

	util_uv_uvmap_cached                              = false;
	util_uv_trianglemap_cached                        = false;
	util_uv_dilatemap_cached                          = false;
	render_path_raytrace_ready                        = false;
	g_context->ddirty                                 = 2;
	ui_base_hwnds->buffer[TAB_AREA_SIDEBAR0]->redraws = 2;
}

// Drops the nodes of an object, the output mesh stays with the object
void nodes_mesh_remove(mesh_object_t *o) {
	if (nodes_mesh_slots == NULL) {
		return;
	}
	nodes_mesh_refresh_pending = true;
	for (i32 i = 0; i < nodes_mesh_slots->length; ++i) {
		mesh_nodes_t *slot = nodes_mesh_slots->buffer[i];
		if (slot->uid != o->base->uid) {
			continue;
		}
		array_splice(nodes_mesh_slots, i, 1);
		mesh_data_t *input = slot->output != slot->input ? slot->input : NULL;
		if (input != NULL && util_mesh_data_owner(input) == -1 && !nodes_mesh_holds(input)) {
			sys_notify_on_next_frame(&util_mesh_delete_data_uncache, input);
		}
		return;
	}
}

bool nodes_mesh_has_nodes(mesh_object_t *o) {
	mesh_nodes_t *slot = nodes_mesh_get(o, false);
	return slot != NULL && !nodes_mesh_is_default(slot->canvas);
}

// The output mesh becomes the original mesh, the next edit starts from a default canvas
void nodes_mesh_apply(mesh_object_t *o) {
	nodes_mesh_remove(o);
	ui_nodes_hwnd->redraws                            = 2;
	ui_base_hwnds->buffer[TAB_AREA_SIDEBAR0]->redraws = 2;
}

// Rebuilds meshes with scene mesh nodes after other meshes changed or moved
void nodes_mesh_request_refresh() {
	nodes_mesh_refresh_pending = true;
}

void nodes_mesh_refresh() {
	if (!nodes_mesh_refresh_pending || nodes_mesh_slots == NULL || mouse_down("left") || g_config->workspace == WORKSPACE_PLAYER) {
		return;
	}
	nodes_mesh_refresh_pending = false;
	for (i32 i = 0; i < nodes_mesh_slots->length; ++i) {
		mesh_nodes_t  *slot = nodes_mesh_slots->buffer[i];
		mesh_object_t *o    = nodes_mesh_object(slot->uid);
		if (o != NULL && nodes_mesh_has_object_nodes(slot->canvas)) {
			nodes_mesh_update(o, false);
		}
	}
}

void nodes_mesh_reset() {
	if (nodes_mesh_slots == NULL) {
		return;
	}
	while (nodes_mesh_slots->length > 0) {
		mesh_nodes_t *slot  = array_pop(nodes_mesh_slots);
		mesh_data_t  *input = slot->output != slot->input ? slot->input : NULL;
		if (input != NULL && g_project->_->paint_objects != NULL && util_mesh_data_owner(input) == -1 && !nodes_mesh_holds(input)) {
			util_mesh_delete_data_uncache(input);
		}
	}
}

// Project files store the original meshes and the nodes, outputs are rebuilt on load
void nodes_mesh_export_begin() {
	g_project->mesh_nodes         = NULL;
	g_project->mesh_nodes_objects = NULL;
	if (nodes_mesh_slots == NULL) {
		return;
	}
	for (i32 i = 0; i < g_project->_->paint_objects->length; ++i) {
		mesh_object_t *o    = g_project->_->paint_objects->buffer[i];
		mesh_nodes_t  *slot = nodes_mesh_get(o, false);
		if (slot == NULL || nodes_mesh_is_default(slot->canvas)) {
			continue;
		}
		if (g_project->mesh_nodes == NULL) {
			g_project->mesh_nodes         = any_array_create(0);
			g_project->mesh_nodes_objects = i32_array_create(0);
		}
		any_array_push(g_project->mesh_nodes, slot->canvas);
		i32_array_push(g_project->mesh_nodes_objects, i);
		if (slot->output != slot->input) {
			slot->output = o->data;
			o->data      = slot->input;
		}
	}
}

void nodes_mesh_export_end() {
	if (nodes_mesh_slots == NULL) {
		return;
	}
	for (i32 i = 0; i < nodes_mesh_slots->length; ++i) {
		mesh_nodes_t  *slot = nodes_mesh_slots->buffer[i];
		mesh_object_t *o    = nodes_mesh_object(slot->uid);
		if (o != NULL && slot->output != slot->input && o->data == slot->input) {
			o->data = slot->output;
		}
	}
}

void nodes_mesh_import() {
	if (g_project->mesh_nodes == NULL || g_project->mesh_nodes_objects == NULL) {
		return;
	}
	for (i32 i = 0; i < g_project->mesh_nodes->length && i < g_project->mesh_nodes_objects->length; ++i) {
		i32 index = g_project->mesh_nodes_objects->buffer[i];
		if (index < 0 || index >= g_project->_->paint_objects->length) {
			continue;
		}
		mesh_object_t *o    = g_project->_->paint_objects->buffer[index];
		mesh_nodes_t  *slot = nodes_mesh_get(o, true);
		slot->canvas        = util_clone_canvas(g_project->mesh_nodes->buffer[i]); // Project data is temporary
		for (i32 j = 0; j < slot->canvas->nodes->length; ++j) {
			ui_node_t *n = slot->canvas->nodes->buffer[j];
			if (string_equals(n->type, "MESH_OBJECT") && n->buttons->buffer[0]->default_value->length > 2) {
				n->buttons->buffer[0]->default_value->buffer[1] = -1; // Object uids are not kept in files
			}
		}
	}
	// Scene mesh nodes need all the canvases in place
	for (i32 i = 0; i < g_project->mesh_nodes->length && i < g_project->mesh_nodes_objects->length; ++i) {
		i32 index = g_project->mesh_nodes_objects->buffer[i];
		if (index >= 0 && index < g_project->_->paint_objects->length) {
			nodes_mesh_update(g_project->_->paint_objects->buffer[index], true);
		}
	}
}

// Script nodes are skipped until the scripts of a loaded project are trusted
bool nodes_mesh_canvases_have_script(ui_node_canvas_t_array_t *canvases) {
	for (i32 i = 0; canvases != NULL && i < canvases->length; ++i) {
		ui_node_canvas_t *c = canvases->buffer[i];
		for (i32 j = 0; c != NULL && j < c->nodes->length; ++j) {
			if (string_equals(c->nodes->buffer[j]->type, "MESH_SCRIPT")) {
				return true;
			}
		}
	}
	return false;
}

void nodes_mesh_update_scripts() {
	for (i32 i = 0; nodes_mesh_slots != NULL && i < nodes_mesh_slots->length; ++i) {
		mesh_nodes_t  *slot = nodes_mesh_slots->buffer[i];
		mesh_object_t *o    = nodes_mesh_object(slot->uid);
		if (o != NULL && nodes_mesh_canvases_have_script(any_array_create_from_raw_tmp((void *[]){slot->canvas}, 1))) {
			nodes_mesh_update(o, true);
		}
	}
}

ui_node_t *nodes_mesh_create_node(mesh_object_t *o, char *type) {
	nodes_mesh_init();
	if (string_equals(type, "MESH_OUTPUT")) {
		return NULL;
	}
	mesh_nodes_t *slot = nodes_mesh_get(o, true);
	for (i32 i = 0; i < nodes_mesh_list->length; ++i) {
		ui_node_t_array_t *category = nodes_mesh_list->buffer[i];
		for (i32 j = 0; j < category->length; ++j) {
			if (string_equals(category->buffer[j]->type, type)) {
				ui_node_t *node = ui_nodes_make_node(category->buffer[j], slot->nodes, slot->canvas);
				any_array_push(slot->canvas->nodes, node);
				return node;
			}
		}
	}
	return NULL;
}

char *nodes_mesh_reference() {
	return "//\n"
	       "// Mesh nodes build the mesh of an object from its original mesh, each object has its own canvas with a\n"
	       "// pre-created MESH_INPUT (original mesh) and MESH_OUTPUT node. Build it with script_mesh_create_node_at(),\n"
	       "// script_mesh_get_node(), script_mesh_connect() and apply with script_mesh_update(). Node values are set with\n"
	       "// script_material_set_float/set_vector/set_button/set_text, which work on any node.\n"
	       "//\n"
	       "// MESH_SCRIPT runs a minic script on the mesh linked to its input, the script text is button 0:\n"
	       "//     mesh_geo_t *g = mesh_geo_input(); // Editable copy of the input, float g->pos[vertex_count * 3],\n"
	       "//                                       // g->nor[vertex_count * 3], g->tex[vertex_count * 2], int g->ind[index_count]\n"
	       "//     mesh_geo_output(g);               // Mesh passed on by the node, the input mesh when not called\n"
	       "//     Arrays are not bounds checked. Counts can be lowered, to grow a mesh fill a new mesh_geo_create(vertex_count, index_count).\n"
	       "//     Triangles are counter clockwise, a mesh fits the default view at a size of about 1.\n"
	       "//     mesh_geo_* functions return a new mesh and leave their arguments unchanged, meshes are freed when the script ends.\n"
	       "//     mesh_geo_shape: 0 Box, 1 Sphere, 2 Cylinder, 3 Cone, 4 Torus, 5 Plane. mesh_geo_mirror axis: 0 X, 1 Y, 2 Z.\n"
	       "//     mesh_geo_boolean op: MESH_BOOLEAN_UNION, MESH_BOOLEAN_DIFFERENCE, MESH_BOOLEAN_INTERSECT. Rotations are in degrees.\n"
	       "//     Call mesh_geo_normals(g, smooth) after moving vertices. The input passes through when the script has an error.\n"
	       "//     Inputs: script_mesh_add_input(o, sc, \"VALUE\", \"Height\") adds a VALUE, VECTOR or RGBA socket after the mesh input\n"
	       "//     (socket 1, 2, ..), the Add button of the node does the same. The script reads them by name with\n"
	       "//     mesh_geo_input_value(\"Height\") and mesh_geo_input_vector(\"Offset\", i), i is 0 - 2 for x y z, 0 - 3 for r g b a.\n"
	       "//     Value, Math and Vector nodes can be linked into them.\n"
	       "//     Example:\n"
	       "//     mesh_object_t *o = script_get_context()->paint_object;\n"
	       "//     ui_node_t *sc = script_mesh_create_node_at(o, \"MESH_SCRIPT\", 300.0, 100.0);\n"
	       "//     script_material_set_text(sc, 0, \"void main() {\\n"
	       "mesh_geo_t *g = mesh_geo_subdivide(mesh_geo_input(), 2);\\n"
	       "for (int i = 0; i < g->vertex_count; i++) {\\n"
	       "g->pos[i * 3 + 2] += sin(g->pos[i * 3] * 8.0) * 0.05;\\n"
	       "}\\n"
	       "g = mesh_geo_boolean(mesh_geo_normals(g, true), mesh_geo_shape(1, 0.8, 0.8, 0.8, 32), MESH_BOOLEAN_DIFFERENCE);\\n"
	       "mesh_geo_output(g);\\n"
	       "}\");\n"
	       "//     script_mesh_connect(o, script_mesh_get_node(o, \"MESH_INPUT\"), 0, sc, 0);\n"
	       "//     script_mesh_connect(o, sc, 0, script_mesh_get_node(o, \"MESH_OUTPUT\"), 0);\n"
	       "//     script_mesh_update(o);\n";
}
