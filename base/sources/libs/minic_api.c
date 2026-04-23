#include "iron_array.h"
#include "iron_string.h"
#include "minic.h"
#include <math.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void console_log(char *s);

static const char *minic_read_str(minic_val_t v) {
	if (v.type == MINIC_T_PTR && v.p != NULL) {
		return (const char *)v.p;
	}
	return "";
}

static int minic_vformat(const char *fmt, minic_val_t *args, int argc, char *buf, int bufsize) {
	int pos = 0;
	int arg = 0;
	while (*fmt != '\0') {
		if (*fmt != '%') {
			if (buf && pos < bufsize - 1)
				buf[pos] = *fmt;
			pos++;
			fmt++;
			continue;
		}
		fmt++;
		char spec = *fmt++;
		char tmp[64];
		int  n = 0;
		if (spec == 'd' || spec == 'i') {
			int iv = arg < argc ? (int)minic_val_to_d(args[arg++]) : 0;
			n      = snprintf(tmp, sizeof(tmp), "%d", iv);
		}
		else if (spec == 'u') {
			unsigned uv = arg < argc ? (unsigned)(int)minic_val_to_d(args[arg++]) : 0u;
			n           = snprintf(tmp, sizeof(tmp), "%u", uv);
		}
		else if (spec == 'f') {
			double dv = arg < argc ? minic_val_to_d(args[arg++]) : 0.0;
			n         = snprintf(tmp, sizeof(tmp), "%f", dv);
		}
		else if (spec == 'g') {
			double dv = arg < argc ? minic_val_to_d(args[arg++]) : 0.0;
			n         = snprintf(tmp, sizeof(tmp), "%g", dv);
		}
		else if (spec == 'e') {
			double dv = arg < argc ? minic_val_to_d(args[arg++]) : 0.0;
			n         = snprintf(tmp, sizeof(tmp), "%e", dv);
		}
		else if (spec == 's') {
			const char *sv   = arg < argc ? minic_read_str(args[arg++]) : "";
			int         slen = (int)strlen(sv);
			if (buf) {
				int copy = slen < bufsize - 1 - pos ? slen : bufsize - 1 - pos;
				if (copy > 0)
					memcpy(buf + pos, sv, copy);
			}
			pos += slen;
			continue;
		}
		else if (spec == 'p') {
			void *pv = (arg < argc && args[arg].type == MINIC_T_PTR) ? args[arg++].p : (void *)(uintptr_t)(uint64_t)minic_val_to_d(args[arg++]);
			n        = snprintf(tmp, sizeof(tmp), "%p", pv);
		}
		else if (spec == 'c') {
			if (buf && pos < bufsize - 1)
				buf[pos] = (char)(arg < argc ? (int)minic_val_to_d(args[arg++]) : 0);
			pos++;
			continue;
		}
		else if (spec == '%') {
			if (buf && pos < bufsize - 1)
				buf[pos] = '%';
			pos++;
			continue;
		}
		else {
			if (buf && pos < bufsize - 1)
				buf[pos] = '%';
			pos++;
			if (buf && pos < bufsize - 1)
				buf[pos] = spec;
			pos++;
			continue;
		}
		if (n > 0) {
			if (buf) {
				int copy = n < bufsize - 1 - pos ? n : bufsize - 1 - pos;
				if (copy > 0)
					memcpy(buf + pos, tmp, copy);
			}
			pos += n;
		}
	}
	if (buf && pos < bufsize)
		buf[pos] = '\0';
	return pos;
}

static minic_val_t minic_printf_native(minic_val_t *args, int argc) {
	if (argc < 1 || args[0].type != MINIC_T_PTR)
		return minic_val_int(0);
	const char *fmt = (const char *)args[0].p;
	int         len = minic_vformat(fmt, args + 1, argc - 1, NULL, 0);
	char       *buf = (char *)malloc(len + 1);
	minic_vformat(fmt, args + 1, argc - 1, buf, len + 1);
	// int written = fputs(buf, stdout);
	console_log(buf);
	free(buf);
	return minic_val_int(len);
}

static minic_val_t minic_string_native(minic_val_t *args, int argc) {
	if (argc < 1 || args[0].type != MINIC_T_PTR)
		return minic_val_ptr(NULL);
	const char *fmt = (const char *)args[0].p;
	int         len = minic_vformat(fmt, args + 1, argc - 1, NULL, 0);
	char       *buf = string_alloc(len + 1);
	minic_vformat(fmt, args + 1, argc - 1, buf, len + 1);
	return minic_val_ptr(buf);
}

#include "engine.h"
#include "iron_armpack.h"
#include "iron_array.h"
#include "iron_draw.h"
#include "iron_file.h"
#include "iron_gc.h"
#include "iron_input.h"
#include "iron_json.h"
#include "iron_map.h"
#include "iron_obj.h"
#include "iron_shape.h"
#include "iron_sys.h"
#include "iron_tilesheet.h"
#include "iron_ui.h"

#include "../../../paint/sources/types.h"

#define R(name, sig) minic_register(#name, sig, (minic_ext_fn_raw_t)name)

// iron_math.h struct layout helpers

static vec2_t minic_get_vec2(void *p) {
	minic_val_t *f = (minic_val_t *)p;
	vec2_t       v;
	v.x = f[0].f;
	v.y = f[1].f;
	return v;
}
static void minic_set_vec2(void *p, vec2_t v) {
	minic_val_t *f = (minic_val_t *)p;
	f[0]           = minic_val_float(v.x);
	f[1]           = minic_val_float(v.y);
}
static vec4_t minic_get_vec4(void *p) {
	minic_val_t *f = (minic_val_t *)p;
	vec4_t       v;
	v.x = f[0].f;
	v.y = f[1].f;
	v.z = f[2].f;
	v.w = f[3].f;
	return v;
}
static void minic_set_vec4(void *p, vec4_t v) {
	minic_val_t *f = (minic_val_t *)p;
	f[0]           = minic_val_float(v.x);
	f[1]           = minic_val_float(v.y);
	f[2]           = minic_val_float(v.z);
	f[3]           = minic_val_float(v.w);
}
static quat_t minic_get_quat(void *p) {
	minic_val_t *f = (minic_val_t *)p;
	quat_t       q;
	q.x = f[0].f;
	q.y = f[1].f;
	q.z = f[2].f;
	q.w = f[3].f;
	return q;
}
static void minic_set_quat(void *p, quat_t q) {
	minic_val_t *f = (minic_val_t *)p;
	f[0]           = minic_val_float(q.x);
	f[1]           = minic_val_float(q.y);
	f[2]           = minic_val_float(q.z);
	f[3]           = minic_val_float(q.w);
}
static mat3_t minic_get_mat3(void *p) {
	minic_val_t *f = (minic_val_t *)p;
	mat3_t       m;
	m.m00 = f[0].f;
	m.m01 = f[1].f;
	m.m02 = f[2].f;
	m.m10 = f[3].f;
	m.m11 = f[4].f;
	m.m12 = f[5].f;
	m.m20 = f[6].f;
	m.m21 = f[7].f;
	m.m22 = f[8].f;
	return m;
}
static void minic_set_mat3(void *p, mat3_t m) {
	minic_val_t *f = (minic_val_t *)p;
	f[0]           = minic_val_float(m.m00);
	f[1]           = minic_val_float(m.m01);
	f[2]           = minic_val_float(m.m02);
	f[3]           = minic_val_float(m.m10);
	f[4]           = minic_val_float(m.m11);
	f[5]           = minic_val_float(m.m12);
	f[6]           = minic_val_float(m.m20);
	f[7]           = minic_val_float(m.m21);
	f[8]           = minic_val_float(m.m22);
}
static mat4_t minic_get_mat4(void *p) {
	minic_val_t *f = (minic_val_t *)p;
	mat4_t       m;
	m.m00 = f[0].f;
	m.m01 = f[1].f;
	m.m02 = f[2].f;
	m.m03 = f[3].f;
	m.m10 = f[4].f;
	m.m11 = f[5].f;
	m.m12 = f[6].f;
	m.m13 = f[7].f;
	m.m20 = f[8].f;
	m.m21 = f[9].f;
	m.m22 = f[10].f;
	m.m23 = f[11].f;
	m.m30 = f[12].f;
	m.m31 = f[13].f;
	m.m32 = f[14].f;
	m.m33 = f[15].f;
	return m;
}
static void minic_set_mat4(void *p, mat4_t m) {
	minic_val_t *f = (minic_val_t *)p;
	f[0]           = minic_val_float(m.m00);
	f[1]           = minic_val_float(m.m01);
	f[2]           = minic_val_float(m.m02);
	f[3]           = minic_val_float(m.m03);
	f[4]           = minic_val_float(m.m10);
	f[5]           = minic_val_float(m.m11);
	f[6]           = minic_val_float(m.m12);
	f[7]           = minic_val_float(m.m13);
	f[8]           = minic_val_float(m.m20);
	f[9]           = minic_val_float(m.m21);
	f[10]          = minic_val_float(m.m22);
	f[11]          = minic_val_float(m.m23);
	f[12]          = minic_val_float(m.m30);
	f[13]          = minic_val_float(m.m31);
	f[14]          = minic_val_float(m.m32);
	f[15]          = minic_val_float(m.m33);
}

#define MN(sym) static minic_val_t mn_##sym(minic_val_t *_a, int _c)
#define RV2()   ((minic_val_t *)minic_alloc(2 * (int)sizeof(minic_val_t)))
#define RV4()   ((minic_val_t *)minic_alloc(4 * (int)sizeof(minic_val_t)))
#define RM3()   ((minic_val_t *)minic_alloc(9 * (int)sizeof(minic_val_t)))
#define RM4()   ((minic_val_t *)minic_alloc(16 * (int)sizeof(minic_val_t)))
#define RET2(v)                   \
	do {                          \
		minic_val_t *_o = RV2();  \
		minic_set_vec2(_o, v);    \
		return minic_val_ptr(_o); \
	} while (0)
#define RET4(v)                   \
	do {                          \
		minic_val_t *_o = RV4();  \
		minic_set_vec4(_o, v);    \
		return minic_val_ptr(_o); \
	} while (0)
#define RETQ(q)                   \
	do {                          \
		minic_val_t *_o = RV4();  \
		minic_set_quat(_o, q);    \
		return minic_val_ptr(_o); \
	} while (0)
#define RET3(m)                   \
	do {                          \
		minic_val_t *_o = RM3();  \
		minic_set_mat3(_o, m);    \
		return minic_val_ptr(_o); \
	} while (0)
#define RET4M(m)                  \
	do {                          \
		minic_val_t *_o = RM4();  \
		minic_set_mat4(_o, m);    \
		return minic_val_ptr(_o); \
	} while (0)
#define V2(i) minic_get_vec2(_a[i].p)
#define V4(i) minic_get_vec4(_a[i].p)
#define QT(i) minic_get_quat(_a[i].p)
#define M3(i) minic_get_mat3(_a[i].p)
#define M4(i) minic_get_mat4(_a[i].p)
#define AF(i) (_a[i].f)
#define AP(i) (_a[i].p)

// vec2
MN(vec2_len) {
	return minic_val_float(vec2_len(V2(0)));
}
MN(vec2_set_len) {
	RET2(vec2_set_len(V2(0), AF(1)));
}
MN(vec2_mult) {
	RET2(vec2_mult(V2(0), AF(1)));
}
MN(vec2_add) {
	RET2(vec2_add(V2(0), V2(1)));
}
MN(vec2_sub) {
	RET2(vec2_sub(V2(0), V2(1)));
}
MN(vec2_cross) {
	return minic_val_float(vec2_cross(V2(0), V2(1)));
}
MN(vec2_norm) {
	RET2(vec2_norm(V2(0)));
}
MN(vec2_dot) {
	return minic_val_float(vec2_dot(V2(0), V2(1)));
}
MN(vec2_nan) {
	RET2(vec2_nan());
}
MN(vec2_isnan) {
	return minic_val_int(vec2_isnan(V2(0)));
}

// vec4
MN(vec4_cross) {
	RET4(vec4_cross(V4(0), V4(1)));
}
MN(vec4_add) {
	RET4(vec4_add(V4(0), V4(1)));
}
MN(vec4_fadd) {
	RET4(vec4_fadd(V4(0), AF(1), AF(2), AF(3), AF(4)));
}
MN(vec4_norm) {
	RET4(vec4_norm(V4(0)));
}
MN(vec4_mult) {
	RET4(vec4_mult(V4(0), AF(1)));
}
MN(vec4_dot) {
	return minic_val_float(vec4_dot(V4(0), V4(1)));
}
MN(vec4_apply_proj) {
	RET4(vec4_apply_proj(V4(0), M4(1)));
}
MN(vec4_apply_mat4) {
	RET4(vec4_apply_mat4(V4(0), M4(1)));
}
MN(vec4_apply_axis_angle) {
	RET4(vec4_apply_axis_angle(V4(0), V4(1), AF(2)));
}
MN(vec4_apply_quat) {
	RET4(vec4_apply_quat(V4(0), QT(1)));
}
MN(vec4_equals) {
	return minic_val_int(vec4_equals(V4(0), V4(1)));
}
MN(vec4_almost_equals) {
	return minic_val_int(vec4_almost_equals(V4(0), V4(1), AF(2)));
}
MN(vec4_len) {
	return minic_val_float(vec4_len(V4(0)));
}
MN(vec4_sub) {
	RET4(vec4_sub(V4(0), V4(1)));
}
MN(vec4_dist) {
	return minic_val_float(vec4_dist(V4(0), V4(1)));
}
MN(vec4_reflect) {
	RET4(vec4_reflect(V4(0), V4(1)));
}
MN(vec4_clamp) {
	RET4(vec4_clamp(V4(0), AF(1), AF(2)));
}
MN(vec4_x_axis) {
	RET4(vec4_x_axis());
}
MN(vec4_y_axis) {
	RET4(vec4_y_axis());
}
MN(vec4_z_axis) {
	RET4(vec4_z_axis());
}
MN(vec4_nan) {
	RET4(vec4_nan());
}
MN(vec4_isnan) {
	return minic_val_int(vec4_isnan(V4(0)));
}

// quat
MN(quat_from_axis_angle) {
	RETQ(quat_from_axis_angle(V4(0), AF(1)));
}
MN(quat_from_mat) {
	RETQ(quat_from_mat(M4(0)));
}
MN(quat_from_rot_mat) {
	RETQ(quat_from_rot_mat(M4(0)));
}
MN(quat_mult) {
	RETQ(quat_mult(QT(0), QT(1)));
}
MN(quat_norm) {
	RETQ(quat_norm(QT(0)));
}
MN(quat_get_euler) {
	RET4(quat_get_euler(QT(0)));
}
MN(quat_from_euler) {
	RETQ(quat_from_euler(AF(0), AF(1), AF(2)));
}
MN(quat_dot) {
	return minic_val_float(quat_dot(QT(0), QT(1)));
}
MN(quat_from_to) {
	RETQ(quat_from_to(V4(0), V4(1)));
}
MN(quat_inv) {
	RETQ(quat_inv(QT(0)));
}

// mat3
MN(mat3_identity) {
	RET3(mat3_identity());
}
MN(mat3_translation) {
	RET3(mat3_translation(AF(0), AF(1)));
}
MN(mat3_rotation) {
	RET3(mat3_rotation(AF(0)));
}
MN(mat3_scale) {
	RET3(mat3_scale(M3(0), V4(1)));
}
MN(mat3_set_from4) {
	RET3(mat3_set_from4(M4(0)));
}
MN(mat3_multmat) {
	RET3(mat3_multmat(M3(0), M3(1)));
}
MN(mat3_transpose) {
	RET3(mat3_transpose(M3(0)));
}
MN(mat3_nan) {
	RET3(mat3_nan());
}
MN(mat3_isnan) {
	return minic_val_int(mat3_isnan(M3(0)));
}

// mat4
MN(mat4_identity) {
	RET4M(mat4_identity());
}
MN(mat4_persp) {
	RET4M(mat4_persp(AF(0), AF(1), AF(2), AF(3)));
}
MN(mat4_ortho) {
	RET4M(mat4_ortho(AF(0), AF(1), AF(2), AF(3), AF(4), AF(5)));
}
MN(mat4_rot_z) {
	RET4M(mat4_rot_z(AF(0)));
}
MN(mat4_compose) {
	RET4M(mat4_compose(V4(0), QT(1), V4(2)));
}
MN(mat4_set_loc) {
	RET4M(mat4_set_loc(M4(0), V4(1)));
}
MN(mat4_from_quat) {
	RET4M(mat4_from_quat(QT(0)));
}
MN(mat4_translate) {
	RET4M(mat4_translate(M4(0), AF(1), AF(2), AF(3)));
}
MN(mat4_scale) {
	RET4M(mat4_scale(M4(0), V4(1)));
}
MN(mat4_mult_mat3x4) {
	RET4M(mat4_mult_mat3x4(M4(0), M4(1)));
}
MN(mat4_mult_mat) {
	RET4M(mat4_mult_mat(M4(0), M4(1)));
}
MN(mat4_inv) {
	RET4M(mat4_inv(M4(0)));
}
MN(mat4_transpose) {
	RET4M(mat4_transpose(M4(0)));
}
MN(mat4_transpose3) {
	RET4M(mat4_transpose3(M4(0)));
}
MN(mat4_get_loc) {
	RET4(mat4_get_loc(M4(0)));
}
MN(mat4_get_scale) {
	RET4(mat4_get_scale(M4(0)));
}
MN(mat4_mult) {
	RET4M(mat4_mult(M4(0), AF(1)));
}
MN(mat4_to_rot) {
	RET4M(mat4_to_rot(M4(0)));
}
MN(mat4_right) {
	RET4(mat4_right(M4(0)));
}
MN(mat4_look) {
	RET4(mat4_look(M4(0)));
}
MN(mat4_up) {
	RET4(mat4_up(M4(0)));
}
MN(mat4_to_f32_array) {
	return minic_val_ptr(mat4_to_f32_array(M4(0)));
}
MN(mat4_determinant) {
	return minic_val_float(mat4_determinant(M4(0)));
}
MN(mat4_nan) {
	RET4M(mat4_nan());
}
MN(mat4_isnan) {
	return minic_val_int(mat4_isnan(M4(0)));
}

// transform
MN(transform_set_matrix) {
	transform_set_matrix(AP(0), M4(1));
	return minic_val_void();
}
MN(transform_rotate) {
	transform_rotate(AP(0), V4(1), AF(2));
	return minic_val_void();
}
MN(transform_move) {
	transform_move(AP(0), V4(1), AF(2));
	return minic_val_void();
}
MN(transform_look) {
	RET4(transform_look(AP(0)));
}
MN(transform_right) {
	RET4(transform_right(AP(0)));
}
MN(transform_up) {
	RET4(transform_up(AP(0)));
}

// iron_shape / iron_draw
MN(line_draw_render) {
	line_draw_render(M4(0));
	return minic_val_void();
}
MN(line_draw_bounds) {
	line_draw_bounds(M4(0), V4(1));
	return minic_val_void();
}
MN(shape_draw_sphere) {
	shape_draw_sphere(M4(0));
	return minic_val_void();
}
MN(draw_set_transform) {
	draw_set_transform(M3(0));
	return minic_val_void();
}

#undef MN
#undef RV2
#undef RV4
#undef RM3
#undef RM4
#undef RET2
#undef RET4
#undef RETQ
#undef RET3
#undef RET4M
#undef V2
#undef V4
#undef QT
#undef M3
#undef M4
#undef AF
#undef AI
#undef AP

// paint
void *plugin_create();
void  plugin_notify_on_ui(void *plugin, void *f);
void  plugin_notify_on_update(void *plugin, void *f);
void  plugin_notify_on_delete(void *plugin, void *f);

void *script_update_fn = NULL;
void  iron_delay_idle_sleep();
void  script_on_update(void *_) {
    iron_delay_idle_sleep();
    minic_call_fn(script_update_fn, NULL, 0);
}
void script_notify_on_update(void *fn) {
	if (script_update_fn == NULL) {
		sys_notify_on_update(script_on_update, NULL);
	}
	script_update_fn = fn;
}

void *script_next_frame_fn = NULL;
void  script_on_next_frame(void *_) {
    minic_call_fn(script_next_frame_fn, NULL, 0);
}
void script_notify_on_next_frame(void *fn) {
	sys_notify_on_next_frame(script_on_next_frame, NULL);
	script_next_frame_fn = fn;
}

void  console_info(char *s);
void  console_error(char *s);
void  ui_box_show_message(char *title, char *text, bool copyable);
void  ui_files_show(char *filters, bool is_save, bool open_multiple, void (*files_done)(char *));
void *_ui_files_done;
void  _ui_files_show_done(char *path) {
    minic_val_t args[1] = {minic_val_ptr(path)};
    minic_call_fn(_ui_files_done, args, 1);
}
void ui_files_show2(char *filters, bool is_save, bool open_multiple, void *files_done) {
	_ui_files_done = files_done;
	ui_files_show(filters, is_save, open_multiple, _ui_files_show_done);
}
void              project_save(bool save_and_quit);
extern char      *project_filepath;
extern context_t *g_context;
extern config_t  *g_config;
extern project_t *g_project;
char             *project_filepath_get() {
    return project_filepath;
}
void project_filepath_set(char *s) {
	project_filepath = string_copy(s);
}
context_t *script_get_context() {
	return g_context;
}
config_t *script_get_config() {
	return g_config;
}
project_t *script_get_project() {
	return g_project;
}
void           context_set_viewport_shader(void *viewport_shader);
void           context_set_viewport_mode(int mode);
void           context_set_camera_controls(int i);
void           node_shader_write_frag(void *raw, char *s);
mesh_object_t *context_main_object();
void           export_texture_run(char *path, bool bake_material);
void           context_select_tool(i32 i);

extern any_map_t      *import_texture_importers;
extern string_array_t *_path_texture_formats;
extern any_map_t      *import_mesh_importers;
extern string_array_t *_path_mesh_formats;

static any_map_t *custom_texture_importers = NULL;
static any_map_t *custom_mesh_importers    = NULL;

gpu_texture_t *plugin_import_custom_texture(char *path) {
	char       *format  = substring(path, string_last_index_of(path, ".") + 1, string_length(path));
	void       *fn      = any_map_get(custom_texture_importers, format);
	minic_val_t args[1] = {minic_val_ptr(path)};
	minic_val_t r       = minic_call_fn(fn, args, 1);
	return r.p;
}
typedef struct raw_mesh raw_mesh_t;
raw_mesh_t             *plugin_import_custom_mesh(char *path) {
    char       *format  = substring(path, string_last_index_of(path, ".") + 1, string_length(path));
    void       *fn      = any_map_get(custom_mesh_importers, format);
    minic_val_t args[1] = {minic_val_ptr(path)};
    minic_val_t r       = minic_call_fn(fn, args, 1);
    return r.p;
}

void plugin_register_texture(char *format, void *fn) {
	any_map_set(import_texture_importers, format, plugin_import_custom_texture);
	any_array_push(_path_texture_formats, format);

	if (custom_texture_importers == NULL) {
		custom_texture_importers = any_map_create();
		gc_root(custom_texture_importers);
	}
	any_map_set(custom_texture_importers, format, fn);
}
void plugin_unregister_texture(char *format) {
	map_delete(import_texture_importers, format);
	array_splice(_path_texture_formats, string_array_index_of(_path_texture_formats, format), 1);
}
void plugin_register_mesh(char *format, void *fn) {
	any_map_set(import_mesh_importers, format, plugin_import_custom_mesh);
	any_array_push(_path_mesh_formats, format);

	if (custom_mesh_importers == NULL) {
		custom_mesh_importers = any_map_create();
		gc_root(custom_mesh_importers);
	}
	any_map_set(custom_mesh_importers, format, fn);
}
void plugin_unregister_mesh(char *format) {
	map_delete(import_mesh_importers, format);
	array_splice(_path_mesh_formats, string_array_index_of(_path_mesh_formats, format), 1);
}
raw_mesh_t *plugin_make_raw_mesh(char *name, i16_array_t *posa, i16_array_t *nora, u32_array_t *inda, float scale_pos) {
	raw_mesh_t *mesh = gc_alloc(sizeof(raw_mesh_t));
	memset(mesh, 0, sizeof(raw_mesh_t));
	mesh->name         = name;
	mesh->posa         = posa;
	mesh->nora         = nora;
	mesh->inda         = inda;
	mesh->scale_pos    = scale_pos;
	mesh->scale_tex    = 1.0f;
	mesh->vertex_count = posa->length / 4;
	mesh->index_count  = inda->length;
	return mesh;
}

extern void *nodes_material_categories;
extern void *nodes_brush_categories;
extern void *nodes_material_list;
extern void *nodes_brush_list;
void         nodes_material_init();
void         nodes_brush_list_init();
void         plugin_material_category_add(char *category_name, any_array_t *node_list) {
    any_array_push(nodes_material_categories, category_name);
    nodes_material_init();
    any_array_push(nodes_material_list, node_list);
}

void plugin_brush_category_add(char *category_name, any_array_t *node_list) {
	any_array_push(nodes_brush_categories, category_name);
	nodes_brush_list_init();
	any_array_push(nodes_brush_list, node_list);
}

void plugin_material_category_remove(char *category_name) {
	int i = array_index_of(nodes_material_categories, category_name);
	array_splice(nodes_material_list, i, 1);
	array_splice(nodes_material_categories, i, 1);
}

void plugin_brush_category_remove(char *category_name) {
	int i = array_index_of(nodes_brush_categories, category_name);
	array_splice(nodes_brush_list, i, 1);
	array_splice(nodes_brush_categories, i, 1);
}

extern any_map_t *parser_material_custom_nodes;
void              plugin_material_custom_nodes_set(char *node_type, void *fn) {
    any_map_set(parser_material_custom_nodes, node_type, fn);
}

extern any_map_t *parser_logic_custom_nodes;
void              plugin_brush_custom_nodes_set(char *node_type, void *fn) {
    any_map_set(parser_logic_custom_nodes, node_type, fn);
}

void plugin_material_custom_nodes_remove(char *node_type) {
	map_delete(parser_material_custom_nodes, node_type);
}

void plugin_brush_custom_nodes_remove(char *node_type) {
	map_delete(parser_logic_custom_nodes, node_type);
}

extern void *parser_material_kong;
void        *plugin_material_kong_get() {
    return parser_material_kong;
}
char *parser_material_parse_value_input(ui_node_socket_t *inp, bool vector_as_grayscale);
void  node_shader_write_frag(void *raw, char *s);

void minic_register_builtins() {
	minic_register_native("printf", minic_printf_native);
	minic_register_native("string", minic_string_native);

	// iron_array
	static const char        *array_fields[]  = {"buffer", "length", "capacity"};
	static const int          array_offsets[] = {(int)offsetof(f32_array_t, buffer), (int)offsetof(f32_array_t, length), (int)offsetof(f32_array_t, capacity)};
	static const minic_type_t array_types[]   = {MINIC_T_PTR, MINIC_T_INT, MINIC_T_INT};
	static const minic_type_t array_int_derefs[]   = {MINIC_T_INT, MINIC_T_INT, MINIC_T_INT};
	static const minic_type_t array_float_derefs[] = {MINIC_T_FLOAT, MINIC_T_INT, MINIC_T_INT};
	static const minic_type_t array_ptr_derefs[]   = {MINIC_T_PTR, MINIC_T_INT, MINIC_T_INT};
	minic_register_struct_native("i8_array_t", array_fields, array_offsets, array_types, array_int_derefs, 3);
	minic_register_struct_native("u8_array_t", array_fields, array_offsets, array_types, array_int_derefs, 3);
	minic_register_struct_native("i16_array_t", array_fields, array_offsets, array_types, array_int_derefs, 3);
	minic_register_struct_native("u16_array_t", array_fields, array_offsets, array_types, array_int_derefs, 3);
	minic_register_struct_native("i32_array_t", array_fields, array_offsets, array_types, array_int_derefs, 3);
	minic_register_struct_native("u32_array_t", array_fields, array_offsets, array_types, array_int_derefs, 3);
	minic_register_struct_native("f32_array_t", array_fields, array_offsets, array_types, array_float_derefs, 3);
	minic_register_struct_native("any_array_t", array_fields, array_offsets, array_types, array_ptr_derefs, 3);
	minic_register_struct_native("string_array_t", array_fields, array_offsets, array_types, array_ptr_derefs, 3);
	minic_register_struct_native("buffer_t", array_fields, array_offsets, array_types, array_int_derefs, 3);
	minic_struct_set_size("i8_array_t", (int)sizeof(i8_array_t));
	minic_struct_set_size("u8_array_t", (int)sizeof(u8_array_t));
	minic_struct_set_size("i16_array_t", (int)sizeof(i16_array_t));
	minic_struct_set_size("u16_array_t", (int)sizeof(u16_array_t));
	minic_struct_set_size("i32_array_t", (int)sizeof(i32_array_t));
	minic_struct_set_size("u32_array_t", (int)sizeof(u32_array_t));
	minic_struct_set_size("f32_array_t", (int)sizeof(f32_array_t));
	minic_struct_set_size("any_array_t", (int)sizeof(any_array_t));
	minic_struct_set_size("string_array_t", (int)sizeof(string_array_t));
	minic_struct_set_size("buffer_t", (int)sizeof(buffer_t));

	// iron_math
	static const char *vec2_fields[] = {"x", "y"};
	static const char *vec3_fields[] = {"x", "y", "z"};
	static const char *vec4_fields[] = {"x", "y", "z", "w"};
	static const char *quat_fields[] = {"x", "y", "z", "w"};
	static const char *mat3_fields[] = {"m00", "m01", "m02", "m10", "m11", "m12", "m20", "m21", "m22"};
	static const char *mat4_fields[] = {"m00", "m01", "m02", "m03", "m10", "m11", "m12", "m13", "m20", "m21", "m22", "m23", "m30", "m31", "m32", "m33"};
	minic_register_struct("vec2_t", vec2_fields, 2);
	minic_register_struct("vec3_t", vec3_fields, 3);
	minic_register_struct("vec4_t", vec4_fields, 4);
	minic_register_struct("quat_t", quat_fields, 4);
	minic_register_struct("mat3_t", mat3_fields, 9);
	minic_register_struct("mat4_t", mat4_fields, 16);

	// iron_ui
	static const char *ui_layout_names[]  = {"UI_LAYOUT_VERTICAL", "UI_LAYOUT_HORIZONTAL"};
	static const int   ui_layout_values[] = {0, 1};
	minic_register_enum("ui_layout_t", ui_layout_names, ui_layout_values, 2);

	static const char *ui_align_names[]  = {"UI_ALIGN_LEFT", "UI_ALIGN_CENTER", "UI_ALIGN_RIGHT"};
	static const int   ui_align_values[] = {0, 1, 2};
	minic_register_enum("ui_align_t", ui_align_names, ui_align_values, 3);

	static const char *ui_state_names[]  = {"UI_STATE_IDLE", "UI_STATE_STARTED", "UI_STATE_DOWN", "UI_STATE_RELEASED", "UI_STATE_HOVERED"};
	static const int   ui_state_values[] = {0, 1, 2, 3, 4};
	minic_register_enum("ui_state_t", ui_state_names, ui_state_values, 5);

	static const char *ui_link_style_names[]  = {"UI_LINK_STYLE_LINE", "UI_LINK_STYLE_CUBIC_BEZIER"};
	static const int   ui_link_style_values[] = {0, 1};
	minic_register_enum("ui_link_style_t", ui_link_style_names, ui_link_style_values, 2);

	static const char *ui_node_flag_names[]  = {"UI_NODE_FLAG_NONE", "UI_NODE_FLAG_COLLAPSED", "UI_NODE_FLAG_PREVIEW"};
	static const int   ui_node_flag_values[] = {0, 1, 2};
	minic_register_enum("ui_node_flag_t", ui_node_flag_names, ui_node_flag_values, 3);

	static const char *ui_handle_fields[]  = {"i",       "f",       "b",       "layout",         "scroll_offset",
	                                          "color",   "redraws", "text",    "scroll_enabled", "drag_enabled",
	                                          "changed", "init",    "children"};
	static const int   ui_handle_offsets[] = {
        (int)offsetof(ui_handle_t, i),
        (int)offsetof(ui_handle_t, f),
        (int)offsetof(ui_handle_t, b),
        (int)offsetof(ui_handle_t, layout),
        (int)offsetof(ui_handle_t, scroll_offset),
        (int)offsetof(ui_handle_t, color),
        (int)offsetof(ui_handle_t, redraws),
        (int)offsetof(ui_handle_t, text),
        (int)offsetof(ui_handle_t, scroll_enabled),
        (int)offsetof(ui_handle_t, drag_enabled),
        (int)offsetof(ui_handle_t, changed),
        (int)offsetof(ui_handle_t, init),
        (int)offsetof(ui_handle_t, children),
    };
	static const minic_type_t ui_handle_types[]       = {MINIC_T_INT, MINIC_T_FLOAT, MINIC_T_INT, MINIC_T_INT, MINIC_T_FLOAT, MINIC_T_INT, MINIC_T_INT,
	                                                     MINIC_T_PTR, MINIC_T_INT,   MINIC_T_INT, MINIC_T_INT, MINIC_T_INT,   MINIC_T_PTR};
	static const minic_type_t ui_handle_deref_types[] = {MINIC_T_INT,  MINIC_T_FLOAT, MINIC_T_INT, MINIC_T_INT, MINIC_T_FLOAT, MINIC_T_INT, MINIC_T_INT,
	                                                     MINIC_T_CHAR, MINIC_T_INT,   MINIC_T_INT, MINIC_T_INT, MINIC_T_INT,   MINIC_T_PTR};
	minic_register_struct_native("ui_handle_t", ui_handle_fields, ui_handle_offsets, ui_handle_types, ui_handle_deref_types, 13);
	minic_struct_set_size("ui_handle_t", (int)sizeof(ui_handle_t));
	minic_struct_field_set_type("ui_handle_t", "children", "any_array_t");

	static const char *ui_node_socket_fields[]  = {"id", "node_id", "name", "type", "color", "default_value", "min", "max", "precision", "display"};
	static const int   ui_node_socket_offsets[] = {
        (int)offsetof(ui_node_socket_t, id),      (int)offsetof(ui_node_socket_t, node_id), (int)offsetof(ui_node_socket_t, name),
        (int)offsetof(ui_node_socket_t, type),    (int)offsetof(ui_node_socket_t, color),   (int)offsetof(ui_node_socket_t, default_value),
        (int)offsetof(ui_node_socket_t, min),     (int)offsetof(ui_node_socket_t, max),     (int)offsetof(ui_node_socket_t, precision),
        (int)offsetof(ui_node_socket_t, display),
    };
	static const minic_type_t ui_node_socket_types[]       = {MINIC_T_INT, MINIC_T_INT,   MINIC_T_PTR,   MINIC_T_PTR,   MINIC_T_INT,
	                                                          MINIC_T_PTR, MINIC_T_FLOAT, MINIC_T_FLOAT, MINIC_T_FLOAT, MINIC_T_INT};
	static const minic_type_t ui_node_socket_deref_types[] = {MINIC_T_INT, MINIC_T_INT,   MINIC_T_CHAR,  MINIC_T_CHAR,  MINIC_T_INT,
	                                                          MINIC_T_PTR, MINIC_T_FLOAT, MINIC_T_FLOAT, MINIC_T_FLOAT, MINIC_T_INT};
	minic_register_struct_native("ui_node_socket_t", ui_node_socket_fields, ui_node_socket_offsets, ui_node_socket_types, ui_node_socket_deref_types, 10);
	minic_struct_set_size("ui_node_socket_t", (int)sizeof(ui_node_socket_t));
	minic_struct_field_set_type("ui_node_socket_t", "default_value", "f32_array_t");

	static const char *ui_node_button_fields[]  = {"name", "type", "output", "default_value", "data", "min", "max", "precision", "height"};
	static const int   ui_node_button_offsets[] = {
        (int)offsetof(ui_node_button_t, name),          (int)offsetof(ui_node_button_t, type),      (int)offsetof(ui_node_button_t, output),
        (int)offsetof(ui_node_button_t, default_value), (int)offsetof(ui_node_button_t, data),      (int)offsetof(ui_node_button_t, min),
        (int)offsetof(ui_node_button_t, max),           (int)offsetof(ui_node_button_t, precision), (int)offsetof(ui_node_button_t, height),
    };
	static const minic_type_t ui_node_button_types[]       = {MINIC_T_PTR,   MINIC_T_PTR,   MINIC_T_INT,   MINIC_T_PTR,  MINIC_T_PTR,
	                                                          MINIC_T_FLOAT, MINIC_T_FLOAT, MINIC_T_FLOAT, MINIC_T_FLOAT};
	static const minic_type_t ui_node_button_deref_types[] = {MINIC_T_CHAR,  MINIC_T_CHAR,  MINIC_T_INT,   MINIC_T_PTR,  MINIC_T_PTR,
	                                                          MINIC_T_FLOAT, MINIC_T_FLOAT, MINIC_T_FLOAT, MINIC_T_FLOAT};
	minic_register_struct_native("ui_node_button_t", ui_node_button_fields, ui_node_button_offsets, ui_node_button_types, ui_node_button_deref_types, 9);
	minic_struct_set_size("ui_node_button_t", (int)sizeof(ui_node_button_t));
	minic_struct_field_set_type("ui_node_button_t", "default_value", "f32_array_t");
	minic_struct_field_set_type("ui_node_button_t", "data", "u8_array_t");

	static const char *ui_node_link_fields[]  = {"id", "from_id", "from_socket", "to_id", "to_socket"};
	static const int   ui_node_link_offsets[] = {
        (int)offsetof(ui_node_link_t, id),    (int)offsetof(ui_node_link_t, from_id),   (int)offsetof(ui_node_link_t, from_socket),
        (int)offsetof(ui_node_link_t, to_id), (int)offsetof(ui_node_link_t, to_socket),
    };
	static const minic_type_t ui_node_link_types[]       = {MINIC_T_INT, MINIC_T_INT, MINIC_T_INT, MINIC_T_INT, MINIC_T_INT};
	static const minic_type_t ui_node_link_deref_types[] = {MINIC_T_INT, MINIC_T_INT, MINIC_T_INT, MINIC_T_INT, MINIC_T_INT};
	minic_register_struct_native("ui_node_link_t", ui_node_link_fields, ui_node_link_offsets, ui_node_link_types, ui_node_link_deref_types, 5);
	minic_struct_set_size("ui_node_link_t", (int)sizeof(ui_node_link_t));

	static const char *ui_node_fields[]  = {"id", "name", "type", "x", "y", "color", "inputs", "outputs", "buttons", "width", "flags"};
	static const int   ui_node_offsets[] = {
        (int)offsetof(ui_node_t, id),      (int)offsetof(ui_node_t, name),  (int)offsetof(ui_node_t, type),   (int)offsetof(ui_node_t, x),
        (int)offsetof(ui_node_t, y),       (int)offsetof(ui_node_t, color), (int)offsetof(ui_node_t, inputs), (int)offsetof(ui_node_t, outputs),
        (int)offsetof(ui_node_t, buttons), (int)offsetof(ui_node_t, width), (int)offsetof(ui_node_t, flags),
    };
	static const minic_type_t ui_node_types[]       = {MINIC_T_INT, MINIC_T_PTR, MINIC_T_PTR, MINIC_T_FLOAT, MINIC_T_FLOAT, MINIC_T_INT,
	                                                   MINIC_T_PTR, MINIC_T_PTR, MINIC_T_PTR, MINIC_T_FLOAT, MINIC_T_INT};
	static const minic_type_t ui_node_deref_types[] = {MINIC_T_INT, MINIC_T_CHAR, MINIC_T_CHAR, MINIC_T_FLOAT, MINIC_T_FLOAT, MINIC_T_INT,
	                                                   MINIC_T_PTR, MINIC_T_PTR,  MINIC_T_PTR,  MINIC_T_FLOAT, MINIC_T_INT};
	minic_register_struct_native("ui_node_t", ui_node_fields, ui_node_offsets, ui_node_types, ui_node_deref_types, 11);
	minic_struct_set_size("ui_node_t", (int)sizeof(ui_node_t));
	minic_struct_field_set_type("ui_node_t", "inputs", "any_array_t");
	minic_struct_field_set_type("ui_node_t", "outputs", "any_array_t");
	minic_struct_field_set_type("ui_node_t", "buttons", "any_array_t");

	// engine.h array types
	minic_register_struct_native("obj_t_array_t", array_fields, array_offsets, array_types, array_ptr_derefs, 3);
	minic_register_struct_native("vertex_array_t_array_t", array_fields, array_offsets, array_types, array_ptr_derefs, 3);
	minic_register_struct_native("shader_data_t_array_t", array_fields, array_offsets, array_types, array_ptr_derefs, 3);
	minic_register_struct_native("tex_unit_t_array_t", array_fields, array_offsets, array_types, array_ptr_derefs, 3);
	minic_register_struct_native("shader_const_t_array_t", array_fields, array_offsets, array_types, array_ptr_derefs, 3);
	minic_register_struct_native("vertex_element_t_array_t", array_fields, array_offsets, array_types, array_ptr_derefs, 3);
	minic_register_struct_native("bind_const_t_array_t", array_fields, array_offsets, array_types, array_ptr_derefs, 3);
	minic_register_struct_native("bind_tex_t_array_t", array_fields, array_offsets, array_types, array_ptr_derefs, 3);
	minic_register_struct_native("material_context_t_array_t", array_fields, array_offsets, array_types, array_ptr_derefs, 3);
	minic_register_struct_native("frustum_plane_array_t", array_fields, array_offsets, array_types, array_ptr_derefs, 3);
	minic_struct_set_size("obj_t_array_t", (int)sizeof(obj_t_array_t));
	minic_struct_set_size("vertex_array_t_array_t", (int)sizeof(vertex_array_t_array_t));
	minic_struct_set_size("shader_data_t_array_t", (int)sizeof(shader_data_t_array_t));
	minic_struct_set_size("tex_unit_t_array_t", (int)sizeof(tex_unit_t_array_t));
	minic_struct_set_size("shader_const_t_array_t", (int)sizeof(shader_const_t_array_t));
	minic_struct_set_size("vertex_element_t_array_t", (int)sizeof(vertex_element_t_array_t));
	minic_struct_set_size("bind_const_t_array_t", (int)sizeof(bind_const_t_array_t));
	minic_struct_set_size("bind_tex_t_array_t", (int)sizeof(bind_tex_t_array_t));
	minic_struct_set_size("material_context_t_array_t", (int)sizeof(material_context_t_array_t));
	minic_struct_set_size("frustum_plane_array_t", (int)sizeof(frustum_plane_array_t));

	// engine.h structs
	// obj_t
	static const char *obj_fields[]  = {"name", "type", "data_ref", "transform", "dimensions", "visible", "spawn", "anim", "material_ref", "children", "_"};
	static const int   obj_offsets[] = {
        (int)offsetof(obj_t, name),         (int)offsetof(obj_t, type),     (int)offsetof(obj_t, data_ref), (int)offsetof(obj_t, transform),
        (int)offsetof(obj_t, dimensions),   (int)offsetof(obj_t, visible),  (int)offsetof(obj_t, spawn),    (int)offsetof(obj_t, anim),
        (int)offsetof(obj_t, material_ref), (int)offsetof(obj_t, children), (int)offsetof(obj_t, _),
    };
	static const minic_type_t obj_types[]       = {MINIC_T_PTR, MINIC_T_PTR, MINIC_T_PTR, MINIC_T_PTR, MINIC_T_PTR, MINIC_T_INT,
	                                               MINIC_T_INT, MINIC_T_PTR, MINIC_T_PTR, MINIC_T_PTR, MINIC_T_PTR};
	static const minic_type_t obj_deref_types[] = {MINIC_T_CHAR, MINIC_T_CHAR, MINIC_T_CHAR, MINIC_T_PTR, MINIC_T_PTR, MINIC_T_INT,
	                                               MINIC_T_INT,  MINIC_T_PTR,  MINIC_T_CHAR, MINIC_T_PTR, MINIC_T_PTR};
	minic_register_struct_native("obj_t", obj_fields, obj_offsets, obj_types, obj_deref_types, 11);
	minic_struct_set_size("obj_t", (int)sizeof(obj_t));
	minic_struct_field_set_type("obj_t", "transform", "f32_array_t");
	minic_struct_field_set_type("obj_t", "dimensions", "f32_array_t");
	minic_struct_field_set_type("obj_t", "children", "obj_t_array_t");

	// vertex_array_t
	static const char *va_fields[]  = {"attrib", "data", "values"};
	static const int   va_offsets[] = {
        (int)offsetof(vertex_array_t, attrib),
        (int)offsetof(vertex_array_t, data),
        (int)offsetof(vertex_array_t, values),
    };
	static const minic_type_t va_types[]       = {MINIC_T_PTR, MINIC_T_PTR, MINIC_T_PTR};
	static const minic_type_t va_deref_types[] = {MINIC_T_CHAR, MINIC_T_CHAR, MINIC_T_PTR};
	minic_register_struct_native("vertex_array_t", va_fields, va_offsets, va_types, va_deref_types, 3);
	minic_struct_set_size("vertex_array_t", (int)sizeof(vertex_array_t));
	minic_struct_field_set_type("vertex_array_t", "values", "i16_array_t");

	// mesh_data_t
	static const char *mesh_data_fields[]  = {"name", "scale_pos", "scale_tex", "vertex_arrays", "index_array", "_"};
	static const int   mesh_data_offsets[] = {
        (int)offsetof(mesh_data_t, name),          (int)offsetof(mesh_data_t, scale_pos),   (int)offsetof(mesh_data_t, scale_tex),
        (int)offsetof(mesh_data_t, vertex_arrays), (int)offsetof(mesh_data_t, index_array), (int)offsetof(mesh_data_t, _),
    };
	static const minic_type_t mesh_data_types[]       = {MINIC_T_PTR, MINIC_T_FLOAT, MINIC_T_FLOAT, MINIC_T_PTR, MINIC_T_PTR, MINIC_T_PTR};
	static const minic_type_t mesh_data_deref_types[] = {MINIC_T_CHAR, MINIC_T_FLOAT, MINIC_T_FLOAT, MINIC_T_PTR, MINIC_T_PTR, MINIC_T_PTR};
	minic_register_struct_native("mesh_data_t", mesh_data_fields, mesh_data_offsets, mesh_data_types, mesh_data_deref_types, 6);
	minic_struct_set_size("mesh_data_t", (int)sizeof(mesh_data_t));
	minic_struct_field_set_type("mesh_data_t", "vertex_arrays", "vertex_array_t_array_t");
	minic_struct_field_set_type("mesh_data_t", "index_array", "u32_array_t");

	// camera_data_t
	static const char *cam_data_fields[]  = {"name", "near_plane", "far_plane", "fov", "aspect", "frustum_culling", "ortho"};
	static const int   cam_data_offsets[] = {
        (int)offsetof(camera_data_t, name),  (int)offsetof(camera_data_t, near_plane), (int)offsetof(camera_data_t, far_plane),
        (int)offsetof(camera_data_t, fov),   (int)offsetof(camera_data_t, aspect),     (int)offsetof(camera_data_t, frustum_culling),
        (int)offsetof(camera_data_t, ortho),
    };
	static const minic_type_t cam_data_types[]       = {MINIC_T_PTR, MINIC_T_FLOAT, MINIC_T_FLOAT, MINIC_T_FLOAT, MINIC_T_FLOAT, MINIC_T_INT, MINIC_T_PTR};
	static const minic_type_t cam_data_deref_types[] = {MINIC_T_CHAR, MINIC_T_FLOAT, MINIC_T_FLOAT, MINIC_T_FLOAT, MINIC_T_FLOAT, MINIC_T_INT, MINIC_T_PTR};
	minic_register_struct_native("camera_data_t", cam_data_fields, cam_data_offsets, cam_data_types, cam_data_deref_types, 7);
	minic_struct_set_size("camera_data_t", (int)sizeof(camera_data_t));
	minic_struct_field_set_type("camera_data_t", "ortho", "f32_array_t");

	// world_data_t
	static const char *world_data_fields[]  = {"name", "color", "strength", "irradiance", "radiance", "radiance_mipmaps", "envmap", "_"};
	static const int   world_data_offsets[] = {
        (int)offsetof(world_data_t, name),       (int)offsetof(world_data_t, color),    (int)offsetof(world_data_t, strength),
        (int)offsetof(world_data_t, irradiance), (int)offsetof(world_data_t, radiance), (int)offsetof(world_data_t, radiance_mipmaps),
        (int)offsetof(world_data_t, envmap),     (int)offsetof(world_data_t, _),
    };
	static const minic_type_t world_data_types[] = {MINIC_T_PTR, MINIC_T_INT, MINIC_T_FLOAT, MINIC_T_PTR, MINIC_T_PTR, MINIC_T_INT, MINIC_T_PTR, MINIC_T_PTR};
	static const minic_type_t world_data_deref_types[] = {MINIC_T_CHAR, MINIC_T_INT, MINIC_T_FLOAT, MINIC_T_CHAR,
	                                                      MINIC_T_CHAR, MINIC_T_INT, MINIC_T_CHAR,  MINIC_T_PTR};
	minic_register_struct_native("world_data_t", world_data_fields, world_data_offsets, world_data_types, world_data_deref_types, 8);
	minic_struct_set_size("world_data_t", (int)sizeof(world_data_t));

	// vertex_element_t
	static const char        *ve_fields[]      = {"name", "data"};
	static const int          ve_offsets[]     = {(int)offsetof(vertex_element_t, name), (int)offsetof(vertex_element_t, data)};
	static const minic_type_t ve_types[]       = {MINIC_T_PTR, MINIC_T_PTR};
	static const minic_type_t ve_deref_types[] = {MINIC_T_CHAR, MINIC_T_CHAR};
	minic_register_struct_native("vertex_element_t", ve_fields, ve_offsets, ve_types, ve_deref_types, 2);
	minic_struct_set_size("vertex_element_t", (int)sizeof(vertex_element_t));

	// shader_const_t
	static const char        *sc_fields[]  = {"name", "type", "link"};
	static const int          sc_offsets[] = {(int)offsetof(shader_const_t, name), (int)offsetof(shader_const_t, type), (int)offsetof(shader_const_t, link)};
	static const minic_type_t sc_types[]   = {MINIC_T_PTR, MINIC_T_PTR, MINIC_T_PTR};
	static const minic_type_t sc_deref_types[] = {MINIC_T_CHAR, MINIC_T_CHAR, MINIC_T_CHAR};
	minic_register_struct_native("shader_const_t", sc_fields, sc_offsets, sc_types, sc_deref_types, 3);
	minic_struct_set_size("shader_const_t", (int)sizeof(shader_const_t));

	// tex_unit_t
	static const char        *tu_fields[]      = {"name", "link"};
	static const int          tu_offsets[]     = {(int)offsetof(tex_unit_t, name), (int)offsetof(tex_unit_t, link)};
	static const minic_type_t tu_types[]       = {MINIC_T_PTR, MINIC_T_PTR};
	static const minic_type_t tu_deref_types[] = {MINIC_T_CHAR, MINIC_T_CHAR};
	minic_register_struct_native("tex_unit_t", tu_fields, tu_offsets, tu_types, tu_deref_types, 2);
	minic_struct_set_size("tex_unit_t", (int)sizeof(tex_unit_t));

	// shader_context_t
	static const char *sctx_fields[]  = {"name",
	                                     "depth_write",
	                                     "compare_mode",
	                                     "cull_mode",
	                                     "vertex_shader",
	                                     "fragment_shader",
	                                     "shader_from_source",
	                                     "blend_source",
	                                     "blend_destination",
	                                     "alpha_blend_source",
	                                     "alpha_blend_destination",
	                                     "color_attachments",
	                                     "depth_attachment",
	                                     "vertex_elements",
	                                     "constants",
	                                     "texture_units"};
	static const int   sctx_offsets[] = {
        (int)offsetof(shader_context_t, name),
        (int)offsetof(shader_context_t, depth_write),
        (int)offsetof(shader_context_t, compare_mode),
        (int)offsetof(shader_context_t, cull_mode),
        (int)offsetof(shader_context_t, vertex_shader),
        (int)offsetof(shader_context_t, fragment_shader),
        (int)offsetof(shader_context_t, shader_from_source),
        (int)offsetof(shader_context_t, blend_source),
        (int)offsetof(shader_context_t, blend_destination),
        (int)offsetof(shader_context_t, alpha_blend_source),
        (int)offsetof(shader_context_t, alpha_blend_destination),
        (int)offsetof(shader_context_t, color_attachments),
        (int)offsetof(shader_context_t, depth_attachment),
        (int)offsetof(shader_context_t, vertex_elements),
        (int)offsetof(shader_context_t, constants),
        (int)offsetof(shader_context_t, texture_units),
    };
	static const minic_type_t sctx_types[]       = {MINIC_T_PTR, MINIC_T_INT, MINIC_T_PTR, MINIC_T_PTR, MINIC_T_PTR, MINIC_T_PTR, MINIC_T_INT, MINIC_T_PTR,
	                                                MINIC_T_PTR, MINIC_T_PTR, MINIC_T_PTR, MINIC_T_PTR, MINIC_T_PTR, MINIC_T_PTR, MINIC_T_PTR, MINIC_T_PTR};
	static const minic_type_t sctx_deref_types[] = {MINIC_T_CHAR, MINIC_T_INT,  MINIC_T_CHAR, MINIC_T_CHAR, MINIC_T_CHAR, MINIC_T_CHAR,
	                                                MINIC_T_INT,  MINIC_T_CHAR, MINIC_T_CHAR, MINIC_T_CHAR, MINIC_T_CHAR, MINIC_T_PTR,
	                                                MINIC_T_CHAR, MINIC_T_PTR,  MINIC_T_PTR,  MINIC_T_PTR};
	minic_register_struct_native("shader_context_t", sctx_fields, sctx_offsets, sctx_types, sctx_deref_types, 16);
	minic_struct_set_size("shader_context_t", (int)sizeof(shader_context_t));
	minic_struct_field_set_type("shader_context_t", "color_attachments", "string_array_t");
	minic_struct_field_set_type("shader_context_t", "vertex_elements", "vertex_element_t_array_t");
	minic_struct_field_set_type("shader_context_t", "constants", "shader_const_t_array_t");
	minic_struct_field_set_type("shader_context_t", "texture_units", "tex_unit_t_array_t");

	// shader_data_t
	static const char        *sd_fields[]      = {"name", "contexts"};
	static const int          sd_offsets[]     = {(int)offsetof(shader_data_t, name), (int)offsetof(shader_data_t, contexts)};
	static const minic_type_t sd_types[]       = {MINIC_T_PTR, MINIC_T_PTR};
	static const minic_type_t sd_deref_types[] = {MINIC_T_CHAR, MINIC_T_PTR};
	minic_register_struct_native("shader_data_t", sd_fields, sd_offsets, sd_types, sd_deref_types, 2);
	minic_struct_set_size("shader_data_t", (int)sizeof(shader_data_t));
	minic_struct_field_set_type("shader_data_t", "contexts", "any_array_t");

	// bind_const_t
	static const char        *bc_fields[]      = {"name", "vec"};
	static const int          bc_offsets[]     = {(int)offsetof(bind_const_t, name), (int)offsetof(bind_const_t, vec)};
	static const minic_type_t bc_types[]       = {MINIC_T_PTR, MINIC_T_PTR};
	static const minic_type_t bc_deref_types[] = {MINIC_T_CHAR, MINIC_T_PTR};
	minic_register_struct_native("bind_const_t", bc_fields, bc_offsets, bc_types, bc_deref_types, 2);
	minic_struct_set_size("bind_const_t", (int)sizeof(bind_const_t));
	minic_struct_field_set_type("bind_const_t", "vec", "f32_array_t");

	// bind_tex_t
	static const char        *bt_fields[]      = {"name", "file"};
	static const int          bt_offsets[]     = {(int)offsetof(bind_tex_t, name), (int)offsetof(bind_tex_t, file)};
	static const minic_type_t bt_types[]       = {MINIC_T_PTR, MINIC_T_PTR};
	static const minic_type_t bt_deref_types[] = {MINIC_T_CHAR, MINIC_T_CHAR};
	minic_register_struct_native("bind_tex_t", bt_fields, bt_offsets, bt_types, bt_deref_types, 2);
	minic_struct_set_size("bind_tex_t", (int)sizeof(bind_tex_t));

	// material_context_t
	static const char *mctx_fields[]  = {"name", "bind_constants", "bind_textures", "_"};
	static const int   mctx_offsets[] = {
        (int)offsetof(material_context_t, name),
        (int)offsetof(material_context_t, bind_constants),
        (int)offsetof(material_context_t, bind_textures),
        (int)offsetof(material_context_t, _),
    };
	static const minic_type_t mctx_types[]       = {MINIC_T_PTR, MINIC_T_PTR, MINIC_T_PTR, MINIC_T_PTR};
	static const minic_type_t mctx_deref_types[] = {MINIC_T_CHAR, MINIC_T_PTR, MINIC_T_PTR, MINIC_T_PTR};
	minic_register_struct_native("material_context_t", mctx_fields, mctx_offsets, mctx_types, mctx_deref_types, 4);
	minic_struct_set_size("material_context_t", (int)sizeof(material_context_t));
	minic_struct_field_set_type("material_context_t", "bind_constants", "bind_const_t_array_t");
	minic_struct_field_set_type("material_context_t", "bind_textures", "bind_tex_t_array_t");

	// material_data_t
	static const char *md_fields[]  = {"name", "shader", "contexts", "_"};
	static const int   md_offsets[] = {
        (int)offsetof(material_data_t, name),
        (int)offsetof(material_data_t, shader),
        (int)offsetof(material_data_t, contexts),
        (int)offsetof(material_data_t, _),
    };
	static const minic_type_t md_types[]       = {MINIC_T_PTR, MINIC_T_PTR, MINIC_T_PTR, MINIC_T_PTR};
	static const minic_type_t md_deref_types[] = {MINIC_T_CHAR, MINIC_T_CHAR, MINIC_T_PTR, MINIC_T_PTR};
	minic_register_struct_native("material_data_t", md_fields, md_offsets, md_types, md_deref_types, 4);
	minic_struct_set_size("material_data_t", (int)sizeof(material_data_t));
	minic_struct_field_set_type("material_data_t", "contexts", "material_context_t_array_t");

	// render_target_t
	static const char *rt_fields[]  = {"name", "width", "height", "format", "scale", "_image"};
	static const int   rt_offsets[] = {
        (int)offsetof(render_target_t, name),   (int)offsetof(render_target_t, width), (int)offsetof(render_target_t, height),
        (int)offsetof(render_target_t, format), (int)offsetof(render_target_t, scale), (int)offsetof(render_target_t, _image),
    };
	static const minic_type_t rt_types[]       = {MINIC_T_PTR, MINIC_T_INT, MINIC_T_INT, MINIC_T_PTR, MINIC_T_FLOAT, MINIC_T_PTR};
	static const minic_type_t rt_deref_types[] = {MINIC_T_CHAR, MINIC_T_INT, MINIC_T_INT, MINIC_T_CHAR, MINIC_T_FLOAT, MINIC_T_PTR};
	minic_register_struct_native("render_target_t", rt_fields, rt_offsets, rt_types, rt_deref_types, 6);
	minic_struct_set_size("render_target_t", (int)sizeof(render_target_t));

	// object_t
	static const char *object_fields[]  = {"uid",      "urandom", "raw",    "name",     "transform", "parent",
	                                       "children", "visible", "culled", "is_empty", "ext",       "ext_type"};
	static const int   object_offsets[] = {
        (int)offsetof(object_t, uid),       (int)offsetof(object_t, urandom),  (int)offsetof(object_t, raw),      (int)offsetof(object_t, name),
        (int)offsetof(object_t, transform), (int)offsetof(object_t, parent),   (int)offsetof(object_t, children), (int)offsetof(object_t, visible),
        (int)offsetof(object_t, culled),    (int)offsetof(object_t, is_empty), (int)offsetof(object_t, ext),      (int)offsetof(object_t, ext_type),
    };
	static const minic_type_t object_types[]       = {MINIC_T_INT, MINIC_T_FLOAT, MINIC_T_PTR, MINIC_T_PTR, MINIC_T_PTR, MINIC_T_PTR,
	                                                  MINIC_T_PTR, MINIC_T_INT,   MINIC_T_INT, MINIC_T_INT, MINIC_T_PTR, MINIC_T_PTR};
	static const minic_type_t object_deref_types[] = {MINIC_T_INT, MINIC_T_FLOAT, MINIC_T_PTR, MINIC_T_CHAR, MINIC_T_PTR, MINIC_T_PTR,
	                                                  MINIC_T_PTR, MINIC_T_INT,   MINIC_T_INT, MINIC_T_INT,  MINIC_T_PTR, MINIC_T_CHAR};
	minic_register_struct_native("object_t", object_fields, object_offsets, object_types, object_deref_types, 12);
	minic_struct_set_size("object_t", (int)sizeof(object_t));
	minic_struct_field_set_type("object_t", "raw", "obj_t");
	minic_struct_field_set_type("object_t", "transform", "transform_t");
	minic_struct_field_set_type("object_t", "children", "any_array_t");

	// mesh_object_t
	static const char *mo_fields[]  = {"base", "data", "material", "camera_dist", "frustum_culling", "skip_context", "force_context"};
	static const int   mo_offsets[] = {
        (int)offsetof(mesh_object_t, base),
        (int)offsetof(mesh_object_t, data),
        (int)offsetof(mesh_object_t, material),
        (int)offsetof(mesh_object_t, camera_dist),
        (int)offsetof(mesh_object_t, frustum_culling),
        (int)offsetof(mesh_object_t, skip_context),
        (int)offsetof(mesh_object_t, force_context),
    };
	static const minic_type_t mo_types[]       = {MINIC_T_PTR, MINIC_T_PTR, MINIC_T_PTR, MINIC_T_FLOAT, MINIC_T_INT, MINIC_T_PTR, MINIC_T_PTR};
	static const minic_type_t mo_deref_types[] = {MINIC_T_PTR, MINIC_T_PTR, MINIC_T_PTR, MINIC_T_FLOAT, MINIC_T_INT, MINIC_T_CHAR, MINIC_T_CHAR};
	minic_register_struct_native("mesh_object_t", mo_fields, mo_offsets, mo_types, mo_deref_types, 7);
	minic_struct_set_size("mesh_object_t", (int)sizeof(mesh_object_t));
	minic_struct_field_set_type("mesh_object_t", "base", "object_t");
	minic_struct_field_set_type("mesh_object_t", "data", "mesh_data_t");
	minic_struct_field_set_type("mesh_object_t", "material", "material_data_t");

	// transform_t
	static const char *transform_fields[]  = {"scale_world", "dirty", "object", "radius"};
	static const int   transform_offsets[] = {
        (int)offsetof(transform_t, scale_world),
        (int)offsetof(transform_t, dirty),
        (int)offsetof(transform_t, object),
        (int)offsetof(transform_t, radius),
    };
	static const minic_type_t transform_types[]       = {MINIC_T_FLOAT, MINIC_T_INT, MINIC_T_PTR, MINIC_T_FLOAT};
	static const minic_type_t transform_deref_types[] = {MINIC_T_FLOAT, MINIC_T_INT, MINIC_T_PTR, MINIC_T_FLOAT};
	minic_register_struct_native("transform_t", transform_fields, transform_offsets, transform_types, transform_deref_types, 4);
	minic_struct_set_size("transform_t", (int)sizeof(transform_t));
	minic_struct_field_set_type("transform_t", "object", "object_t");

	// camera_object_t
	static const char *camobj_fields[]  = {"base", "data", "frame", "frustum_planes"};
	static const int   camobj_offsets[] = {
        (int)offsetof(camera_object_t, base),
        (int)offsetof(camera_object_t, data),
        (int)offsetof(camera_object_t, frame),
        (int)offsetof(camera_object_t, frustum_planes),
    };
	static const minic_type_t camobj_types[]       = {MINIC_T_PTR, MINIC_T_PTR, MINIC_T_INT, MINIC_T_PTR};
	static const minic_type_t camobj_deref_types[] = {MINIC_T_PTR, MINIC_T_PTR, MINIC_T_INT, MINIC_T_PTR};
	minic_register_struct_native("camera_object_t", camobj_fields, camobj_offsets, camobj_types, camobj_deref_types, 4);
	minic_struct_set_size("camera_object_t", (int)sizeof(camera_object_t));
	minic_struct_field_set_type("camera_object_t", "base", "object_t");
	minic_struct_field_set_type("camera_object_t", "data", "camera_data_t");
	minic_struct_field_set_type("camera_object_t", "frustum_planes", "frustum_plane_array_t");

	// frustum_plane_t
	static const char        *fp_fields[]      = {"constant"};
	static const int          fp_offsets[]     = {(int)offsetof(frustum_plane_t, constant)};
	static const minic_type_t fp_types[]       = {MINIC_T_FLOAT};
	static const minic_type_t fp_deref_types[] = {MINIC_T_FLOAT};
	minic_register_struct_native("frustum_plane_t", fp_fields, fp_offsets, fp_types, fp_deref_types, 1);
	minic_struct_set_size("frustum_plane_t", (int)sizeof(frustum_plane_t));

	// cached_shader_context_t
	static const char        *csc_fields[]      = {"context"};
	static const int          csc_offsets[]     = {(int)offsetof(cached_shader_context_t, context)};
	static const minic_type_t csc_types[]       = {MINIC_T_PTR};
	static const minic_type_t csc_deref_types[] = {MINIC_T_PTR};
	minic_register_struct_native("cached_shader_context_t", csc_fields, csc_offsets, csc_types, csc_deref_types, 1);
	minic_struct_set_size("cached_shader_context_t", (int)sizeof(cached_shader_context_t));
	minic_struct_field_set_type("cached_shader_context_t", "context", "shader_context_t");

	// config_t (16 of 62 fields; most script-relevant)
	static const char *config_fields[]  = {"window_w",      "window_h",      "window_scale", "rp_supersample", "recent_projects", "plugins",
	                                       "keymap",        "theme",         "undo_steps",   "camera_fov",     "layer_res",       "brush_live",
	                                       "node_previews", "material_live", "workspace",    "workflow"};
	static const int   config_offsets[] = {
        (int)offsetof(config_t, window_w),       (int)offsetof(config_t, window_h),        (int)offsetof(config_t, window_scale),
        (int)offsetof(config_t, rp_supersample), (int)offsetof(config_t, recent_projects), (int)offsetof(config_t, plugins),
        (int)offsetof(config_t, keymap),         (int)offsetof(config_t, theme),           (int)offsetof(config_t, undo_steps),
        (int)offsetof(config_t, camera_fov),     (int)offsetof(config_t, layer_res),       (int)offsetof(config_t, brush_live),
        (int)offsetof(config_t, node_previews),  (int)offsetof(config_t, material_live),   (int)offsetof(config_t, workspace),
        (int)offsetof(config_t, workflow),
    };
	static const minic_type_t config_types[] = {MINIC_T_INT, MINIC_T_INT,   MINIC_T_FLOAT, MINIC_T_FLOAT, MINIC_T_PTR, MINIC_T_PTR, MINIC_T_PTR, MINIC_T_PTR,
	                                            MINIC_T_INT, MINIC_T_FLOAT, MINIC_T_INT,   MINIC_T_INT,   MINIC_T_INT, MINIC_T_INT, MINIC_T_INT, MINIC_T_INT};
	static const minic_type_t config_deref_types[] = {MINIC_T_INT,  MINIC_T_INT,  MINIC_T_FLOAT, MINIC_T_FLOAT, MINIC_T_PTR, MINIC_T_PTR,
	                                                  MINIC_T_CHAR, MINIC_T_CHAR, MINIC_T_INT,   MINIC_T_FLOAT, MINIC_T_INT, MINIC_T_INT,
	                                                  MINIC_T_INT,  MINIC_T_INT,  MINIC_T_INT,   MINIC_T_INT};
	minic_register_struct_native("config_t", config_fields, config_offsets, config_types, config_deref_types, 16);
	minic_struct_set_size("config_t", (int)sizeof(config_t));
	minic_struct_field_set_type("config_t", "recent_projects", "string_array_t");
	minic_struct_field_set_type("config_t", "plugins", "string_array_t");

	// context_t (16 of many fields; most script-relevant)
	static const char *ctx_fields[]  = {"paint_object", "ddirty",         "pdirty",        "rdirty",        "material",       "layer",
	                                    "brush",        "tool",           "brush_radius",  "brush_opacity", "brush_hardness", "brush_scale",
	                                    "brush_angle",  "brush_blending", "viewport_mode", "xray"};
	static const int   ctx_offsets[] = {
        (int)offsetof(context_t, paint_object),  (int)offsetof(context_t, ddirty),         (int)offsetof(context_t, pdirty),
        (int)offsetof(context_t, rdirty),        (int)offsetof(context_t, material),       (int)offsetof(context_t, layer),
        (int)offsetof(context_t, brush),         (int)offsetof(context_t, tool),           (int)offsetof(context_t, brush_radius),
        (int)offsetof(context_t, brush_opacity), (int)offsetof(context_t, brush_hardness), (int)offsetof(context_t, brush_scale),
        (int)offsetof(context_t, brush_angle),   (int)offsetof(context_t, brush_blending), (int)offsetof(context_t, viewport_mode),
        (int)offsetof(context_t, xray),
    };
	static const minic_type_t ctx_types[] = {MINIC_T_PTR,   MINIC_T_INT,   MINIC_T_INT,   MINIC_T_INT,   MINIC_T_PTR,   MINIC_T_PTR, MINIC_T_PTR, MINIC_T_INT,
	                                         MINIC_T_FLOAT, MINIC_T_FLOAT, MINIC_T_FLOAT, MINIC_T_FLOAT, MINIC_T_FLOAT, MINIC_T_INT, MINIC_T_INT, MINIC_T_INT};
	static const minic_type_t ctx_deref_types[] = {MINIC_T_PTR,   MINIC_T_INT, MINIC_T_INT,   MINIC_T_INT,   MINIC_T_PTR,   MINIC_T_PTR,
	                                               MINIC_T_PTR,   MINIC_T_INT, MINIC_T_FLOAT, MINIC_T_FLOAT, MINIC_T_FLOAT, MINIC_T_FLOAT,
	                                               MINIC_T_FLOAT, MINIC_T_INT, MINIC_T_INT,   MINIC_T_INT};
	minic_register_struct_native("context_t", ctx_fields, ctx_offsets, ctx_types, ctx_deref_types, 16);
	minic_struct_set_size("context_t", (int)sizeof(context_t));
	minic_struct_field_set_type("context_t", "paint_object", "mesh_object_t");

	// project_t (16 of 25 fields; packed_assets, icons, mesh_assets, atlas, envmap_blur omitted)
	static const char *pf_fields[]  = {"version",     "assets",       "is_bgra",       "envmap",      "envmap_strength", "envmap_angle",
	                                   "camera_fov",  "camera_world", "camera_origin", "swatches",    "brush_nodes",     "material_nodes",
	                                   "font_assets", "layer_datas",  "mesh_datas",    "script_datas"};
	static const int   pf_offsets[] = {
        (int)offsetof(project_t, version),      (int)offsetof(project_t, assets),          (int)offsetof(project_t, is_bgra),
        (int)offsetof(project_t, envmap),       (int)offsetof(project_t, envmap_strength), (int)offsetof(project_t, envmap_angle),
        (int)offsetof(project_t, camera_fov),   (int)offsetof(project_t, camera_world),    (int)offsetof(project_t, camera_origin),
        (int)offsetof(project_t, swatches),     (int)offsetof(project_t, brush_nodes),     (int)offsetof(project_t, material_nodes),
        (int)offsetof(project_t, font_assets),  (int)offsetof(project_t, layer_datas),     (int)offsetof(project_t, mesh_datas),
        (int)offsetof(project_t, script_datas),
    };
	static const minic_type_t pf_types[]       = {MINIC_T_PTR, MINIC_T_PTR, MINIC_T_INT, MINIC_T_PTR, MINIC_T_FLOAT, MINIC_T_FLOAT, MINIC_T_FLOAT, MINIC_T_PTR,
	                                              MINIC_T_PTR, MINIC_T_PTR, MINIC_T_PTR, MINIC_T_PTR, MINIC_T_PTR,   MINIC_T_PTR,   MINIC_T_PTR,   MINIC_T_PTR};
	static const minic_type_t pf_deref_types[] = {MINIC_T_CHAR,  MINIC_T_PTR, MINIC_T_INT, MINIC_T_CHAR, MINIC_T_FLOAT, MINIC_T_FLOAT,
	                                              MINIC_T_FLOAT, MINIC_T_PTR, MINIC_T_PTR, MINIC_T_PTR,  MINIC_T_PTR,   MINIC_T_PTR,
	                                              MINIC_T_PTR,   MINIC_T_PTR, MINIC_T_PTR, MINIC_T_PTR};
	minic_register_struct_native("project_t", pf_fields, pf_offsets, pf_types, pf_deref_types, 16);
	minic_struct_set_size("project_t", (int)sizeof(project_t));
	minic_struct_field_set_type("project_t", "assets", "string_array_t");
	minic_struct_field_set_type("project_t", "camera_world", "f32_array_t");
	minic_struct_field_set_type("project_t", "camera_origin", "f32_array_t");
	minic_struct_field_set_type("project_t", "font_assets", "string_array_t");
	minic_struct_field_set_type("project_t", "script_datas", "string_array_t");

	// iron_math
	R(iron_random_init, "v(i)");
	R(iron_random_get, "i()");
	R(iron_random_get_max, "i(i)");
	R(iron_random_get_in, "i(i,i)");
	R(vec4_fdist, "f(f,f,f,f,f,f)");
	R(mat4_cofactor, "f(f,f,f,f,f,f,f,f,f)");
	R(cosf, "f(f)");
	R(sinf, "f(f)");

	// iron_math
#define MR(sym) minic_register_native(#sym, mn_##sym)
	MR(vec2_len);
	MR(vec2_set_len);
	MR(vec2_mult);
	MR(vec2_add);
	MR(vec2_sub);
	MR(vec2_cross);
	MR(vec2_norm);
	MR(vec2_dot);
	MR(vec2_nan);
	MR(vec2_isnan);
	MR(vec4_cross);
	MR(vec4_add);
	MR(vec4_fadd);
	MR(vec4_norm);
	MR(vec4_mult);
	MR(vec4_dot);
	MR(vec4_apply_proj);
	MR(vec4_apply_mat4);
	MR(vec4_apply_axis_angle);
	MR(vec4_apply_quat);
	MR(vec4_equals);
	MR(vec4_almost_equals);
	MR(vec4_len);
	MR(vec4_sub);
	MR(vec4_dist);
	MR(vec4_reflect);
	MR(vec4_clamp);
	MR(vec4_x_axis);
	MR(vec4_y_axis);
	MR(vec4_z_axis);
	MR(vec4_nan);
	MR(vec4_isnan);
	MR(quat_from_axis_angle);
	MR(quat_from_mat);
	MR(quat_from_rot_mat);
	MR(quat_mult);
	MR(quat_norm);
	MR(quat_get_euler);
	MR(quat_from_euler);
	MR(quat_dot);
	MR(quat_from_to);
	MR(quat_inv);
	MR(mat3_identity);
	MR(mat3_translation);
	MR(mat3_rotation);
	MR(mat3_scale);
	MR(mat3_set_from4);
	MR(mat3_multmat);
	MR(mat3_transpose);
	MR(mat3_nan);
	MR(mat3_isnan);
	MR(mat4_identity);
	MR(mat4_persp);
	MR(mat4_ortho);
	MR(mat4_rot_z);
	MR(mat4_compose);
	MR(mat4_set_loc);
	MR(mat4_from_quat);
	MR(mat4_translate);
	MR(mat4_scale);
	MR(mat4_mult_mat3x4);
	MR(mat4_mult_mat);
	MR(mat4_inv);
	MR(mat4_transpose);
	MR(mat4_transpose3);
	MR(mat4_get_loc);
	MR(mat4_get_scale);
	MR(mat4_mult);
	MR(mat4_to_rot);
	MR(mat4_right);
	MR(mat4_look);
	MR(mat4_up);
	MR(mat4_to_f32_array);
	MR(mat4_determinant);
	MR(mat4_nan);
	MR(mat4_isnan);
	MR(transform_set_matrix);
	MR(transform_rotate);
	MR(transform_move);
	MR(transform_look);
	MR(transform_right);
	MR(transform_up);
#undef MR

	// object
	R(object_create, "p(i)");
	R(object_set_parent, "v(p,p)");
	R(object_remove_super, "v(p)");
	R(object_remove, "v(p)");
	R(object_get_child, "p(p,p)");

	// transform
	R(transform_create, "p(p)");
	R(transform_reset, "v(p)");
	R(transform_update, "v(p)");
	R(transform_build_matrix, "v(p)");
	R(transform_decompose, "v(p)");
	R(transform_compute_radius, "v(p)");
	R(transform_compute_dim, "v(p)");
	R(transform_world_x, "f(p)");
	R(transform_world_y, "f(p)");
	R(transform_world_z, "f(p)");

	// camera_data
	R(camera_data_parse, "p(p,p)");
	R(camera_data_get_raw_by_name, "p(p,p)");

	// camera_object
	R(camera_object_create, "p(p)");
	R(camera_object_build_proj, "v(p,f)");
	R(camera_object_remove, "v(p)");
	R(camera_object_render_frame, "v(p)");
	R(camera_object_proj_jitter, "v(p)");
	R(camera_object_build_mat, "v(p)");
	R(camera_object_sphere_in_frustum, "i(p,p,f,f,f,f)");

	// frustum_plane
	R(frustum_plane_create, "p()");
	R(frustum_plane_normalize, "v(p)");
	R(frustum_plane_set_components, "v(p,f,f,f,f)");

	// world_data
	R(world_data_parse, "p(p,p)");
	R(world_data_get_raw_by_name, "p(p,p)");
	R(world_data_get_empty_irradiance, "p()");
	R(world_data_set_irradiance, "p(p)");
	R(world_data_load_envmap, "v(p)");

	// material_data
	R(material_data_create, "p(p,p)");
	R(material_data_parse, "p(p,p)");
	R(material_data_get_raw_by_name, "p(p,p)");
	R(material_data_get_context, "p(p,p)");
	R(material_context_load, "v(p)");

	// shader_data / shader_context
	R(shader_data_create, "p(p)");
	R(shader_data_ext, "p()");
	R(shader_data_parse, "p(p,p)");
	R(shader_data_get_raw_by_name, "p(p,p)");
	R(shader_data_delete, "v(p)");
	R(shader_data_get_context, "p(p,p)");
	R(shader_context_load, "v(p)");
	R(shader_context_compile, "v(p)");
	R(shader_context_type_size, "i(p)");
	R(shader_context_type_pad, "i(i,i)");
	R(shader_context_finish_compile, "v(p)");
	R(shader_context_parse_vertex_struct, "v(p)");
	R(shader_context_delete, "v(p)");
	R(shader_context_get_compare_mode, "i(p)");
	R(shader_context_get_cull_mode, "i(p)");
	R(shader_context_get_blend_fac, "i(p)");
	R(shader_context_get_tex_format, "i(p)");
	R(shader_context_add_const, "v(p,i)");
	R(shader_context_add_tex, "v(p,i)");

	// mesh_data
	R(mesh_data_parse, "p(p,p)");
	R(mesh_data_get_raw_by_name, "p(p,p)");
	R(mesh_data_create, "p(p)");
	R(mesh_data_get_vertex_size, "i(p)");
	R(mesh_data_build_vertices, "v(p,p)");
	R(mesh_data_build_indices, "v(p,p)");
	R(mesh_data_get_vertex_array, "p(p,p)");
	R(mesh_data_build, "v(p)");
	R(mesh_data_delete, "v(p)");

	// mesh_object
	R(mesh_object_create, "p(p,p)");
	R(mesh_object_set_data, "v(p,p)");
	R(mesh_object_remove, "v(p)");
	R(mesh_object_cull_material, "i(p,p)");
	R(mesh_object_cull_mesh, "i(p,p,p)");
	R(mesh_object_render, "v(p,p,p)");
	R(mesh_object_valid_context, "i(p,p,p)");

	// uniforms
	R(uniforms_set_context_consts, "v(p,p)");
	R(uniforms_set_obj_consts, "v(p,p)");
	R(uniforms_bind_render_target, "v(p,p,p)");
	R(uniforms_set_context_const, "i(i,p)");
	R(uniforms_set_obj_const, "v(p,i,p)");
	R(uniforms_set_material_consts, "v(p,p)");
	R(current_material, "p(p)");
	R(uniforms_set_material_const, "v(i,p,p)");

	// data
	R(data_get_mesh, "p(p,p)");
	R(data_get_camera, "p(p,p)");
	R(data_get_material, "p(p,p)");
	R(data_get_world, "p(p,p)");
	R(data_get_shader, "p(p,p)");
	R(data_get_scene_raw, "p(p)");
	R(data_get_image, "p(p)");
	R(data_get_blob, "p(p)");
	R(data_get_video, "p(p)");
	R(data_get_font, "p(p)");
	R(data_delete_mesh, "v(p)");
	R(data_delete_blob, "v(p)");
	R(data_delete_image, "v(p)");
	R(data_delete_video, "v(p)");
	R(data_delete_font, "v(p)");
	R(data_is_abs, "i(p)");
	R(data_is_up, "i(p)");
	R(data_resolve_path, "p(p)");
	R(data_path, "p()");

	// scene
	R(scene_create, "p(p)");
	R(scene_remove, "v()");
	R(scene_set_active, "p(p)");
	R(scene_render_frame, "v()");
	R(scene_add_object, "p(p)");
	R(scene_get_child, "p(p)");
	R(scene_add_mesh_object, "p(p,p,p)");
	R(scene_add_camera_object, "p(p,p)");
	R(scene_traverse_objects, "v(p,p,p)");
	R(scene_add_scene, "p(p,p)");
	R(scene_get_objects_count, "i(p)");
	R(scene_spawn_object, "p(p,p,i)");
	R(scene_get_raw_object_by_name, "p(p,p)");
	R(scene_traverse_objs, "p(p,p)");
	R(scene_create_object, "p(p,p,p)");
	R(scene_create_mesh_object, "p(p,p,p,p)");
	R(scene_return_mesh_object, "p(p,p,p,p,p)");
	R(scene_return_object, "p(p,p)");
	R(scene_gen_transform, "v(p,p)");
	R(scene_load_embedded_data, "v(p)");
	R(scene_embed_data, "v(p)");

	// render_path
	R(render_path_ready, "i()");
	R(render_path_render_frame, "v()");
	R(render_path_set_target, "v(p,p,p,i,i,f)");
	R(render_path_end, "v()");
	R(render_path_draw_meshes, "v(p)");
	R(render_path_submit_draw, "v(p)");
	R(render_path_draw_skydome, "v(p)");
	R(render_path_bind_target, "v(p,p)");
	R(render_path_draw_shader, "v(p)");
	R(render_path_load_shader, "v(p)");
	R(render_path_resize, "v()");
	R(render_path_create_render_target, "p(p)");
	R(render_path_create_image, "p(p)");
	R(render_path_get_tex_format, "i(p)");
	R(render_target_create, "p()");

	// ui
	R(ui_init, "v(p,p)");
	R(ui_begin, "v(p)");
	R(ui_begin_sticky, "v()");
	R(ui_end_sticky, "v()");
	R(ui_begin_region, "v(p,i,i,i)");
	R(ui_end_region, "v()");
	R(ui_window, "b(p,i,i,i,i,i)");
	R(ui_button, "b(p,i,p)");
	R(ui_text, "i(p,i,i)");
	R(ui_tab, "b(p,p,i,i,i)");
	R(ui_panel, "b(p,p,i,i,i)");
	R(ui_sub_image, "i(p,i,i,i,i,i,i)");
	R(ui_image, "i(p,i,i)");
	R(ui_text_input, "p(p,p,i,i,i)");
	R(ui_check, "b(p,p,p)");
	R(ui_radio, "b(p,i,p,p)");
	R(ui_combo, "i(p,p,p,i,i,i)");
	R(ui_slider, "f(p,p,f,f,i,f,i,i,i)");
	R(ui_row, "v(p)");
	R(ui_row2, "v()");
	R(ui_row3, "v()");
	R(ui_row4, "v()");
	R(ui_row5, "v()");
	R(ui_row6, "v()");
	R(ui_row7, "v()");
	R(ui_separator, "v(i,i)");
	R(ui_tooltip, "v(p)");
	R(ui_tooltip_image, "v(p,i)");
	R(ui_end, "v()");
	R(ui_end_window, "v()");
	R(ui_hovered_tab_name, "p()");
	R(ui_set_hovered_tab_name, "v(p)");
	R(ui_mouse_down, "v(p,i,i,i)");
	R(ui_mouse_move, "v(p,i,i,i,i)");
	R(ui_mouse_up, "v(p,i,i,i)");
	R(ui_mouse_wheel, "v(p,f)");
	R(ui_pen_down, "v(p,i,i,f)");
	R(ui_pen_up, "v(p,i,i,f)");
	R(ui_pen_move, "v(p,i,i,f)");
	R(ui_key_down, "v(p,i)");
	R(ui_key_up, "v(p,i)");
	R(ui_key_press, "v(p,i)");
	R(ui_copy, "p()");
	R(ui_cut, "p()");
	R(ui_paste, "v(p)");
	R(ui_theme_default, "v(p)");
	R(ui_get_current, "p()");
	R(ui_set_current, "v(p)");
	R(ui_handle_create, "p()");
	R(ui_nest, "p(p,i)");
	R(ui_set_scale, "v(f)");
	R(ui_get_hover, "i(f)");
	R(ui_get_released, "i(f)");
	R(ui_input_in_rect, "i(f,f,f,f)");
	R(ui_fill, "v(f,f,f,f,i)");
	R(ui_rect, "v(f,f,f,f,i,f)");
	R(ui_line_count, "i(p)");
	R(ui_extract_line, "p(p,i)");
	R(ui_extract_line_off, "p(p,i,p)");
	R(ui_is_visible, "i(f)");
	R(ui_end_element, "v()");
	R(ui_end_element_of_size, "v(f)");
	R(ui_end_input, "v()");
	R(ui_end_frame, "v()");
	R(ui_fade_color, "v(f)");
	R(ui_draw_string, "v(p,f,f,i,i)");
	R(ui_draw_shadow, "v(f,f,f,f)");
	R(ui_draw_rect, "v(i,f,f,f,f)");
	R(ui_draw_round_bottom, "v(f,f,f)");
	R(ui_start_text_edit, "v(p,i)");
	R(ui_remove_char_at, "v(p,i)");
	R(ui_remove_chars_at, "v(p,i,i)");
	R(ui_insert_char_at, "v(p,i,i)");
	R(ui_insert_chars_at, "v(p,i,p)");
	R(UI_SCALE, "f()");
	R(UI_ELEMENT_W, "f()");
	R(UI_ELEMENT_H, "f()");
	R(UI_ELEMENT_OFFSET, "f()");
	R(UI_ARROW_SIZE, "f()");
	R(UI_BUTTON_H, "f()");
	R(UI_CHECK_SIZE, "f()");
	R(UI_CHECK_SELECT_SIZE, "f()");
	R(UI_FONT_SIZE, "f()");
	R(UI_SCROLL_W, "f()");
	R(UI_TEXT_OFFSET, "f()");
	R(UI_TAB_W, "f()");
	R(UI_HEADER_DRAG_H, "f()");
	R(UI_TOOLTIP_DELAY, "f()");
	R(ui_float_input, "f(p,p,i,f)");
	R(ui_inline_radio, "i(p,p,i)");
	R(ui_color_wheel, "i(p,i,f,f,i,p,p)");
	R(ui_text_area, "p(p,i,i,p,i)");
	R(ui_begin_menu, "v()");
	R(ui_end_menu, "v()");
	R(ui_menubar_button, "i(p)");
	R(ui_hsv_to_rgb, "v(f,f,f,p)");
	R(ui_rgb_to_hsv, "v(f,f,f,p)");
	R(ui_color_r, "i(i)");
	R(ui_color_g, "i(i)");
	R(ui_color_b, "i(i)");
	R(ui_color_a, "i(i)");
	R(ui_color, "i(i,i,i,i)");

	// ui_nodes
	R(ui_nodes_init, "v(p)");
	R(ui_node_canvas, "v(p,p)");
	R(ui_nodes_rgba_popup, "v(p,p,i,i)");
	R(ui_remove_node, "v(p,p)");
	R(UI_NODES_SCALE, "f()");
	R(UI_NODES_PAN_X, "f()");
	R(UI_NODES_PAN_Y, "f()");
	R(ui_node_canvas_encode, "v(p)");
	R(ui_node_canvas_encoded_size, "i(p)");
	R(ui_node_canvas_to_json, "p(p)");
	R(UI_NODE_X, "f(p)");
	R(UI_NODE_Y, "f(p)");
	R(UI_NODE_W, "f(p)");
	R(UI_NODE_H, "f(p,p)");
	R(UI_OUTPUT_Y, "f(p,i)");
	R(UI_INPUT_Y, "f(p,p,i)");
	R(UI_OUTPUTS_H, "f(p,i)");
	R(UI_BUTTONS_H, "f(p)");
	R(UI_LINE_H, "f()");
	R(ui_p, "f(f)");
	R(ui_get_socket_id, "i(p)");
	R(ui_get_link, "p(p,i)");
	R(ui_next_link_id, "i(p)");
	R(ui_get_node, "p(p,i)");
	R(ui_next_node_id, "i(p)");

	// sys
	R(sys_start, "v(p)");
	R(sys_time, "f()");
	R(sys_delta, "f()");
	R(sys_real_delta, "f()");
	R(sys_w, "i()");
	R(sys_h, "i()");
	R(sys_x, "i()");
	R(sys_y, "i()");
	R(sys_title, "p()");
	R(sys_title_set, "v(p)");
	R(sys_display_primary_id, "i()");
	R(sys_display_width, "i()");
	R(sys_display_height, "i()");
	R(sys_display_frequency, "i()");
	R(sys_display_ppi, "i()");
	R(sys_shader_ext, "p()");
	R(sys_get_shader, "p(p)");
	R(sys_notify_on_app_state, "v(p,p,p,p,p)");
	R(sys_notify_on_drop_files, "v(p)");
	R(sys_notify_on_update, "v(p,p)");
	R(sys_notify_on_render, "v(p,p)");
	R(sys_notify_on_next_frame, "v(p,p)");
	R(sys_notify_on_end_frame, "v(p,p)");
	R(sys_remove_update, "v(p)");
	R(sys_remove_render, "v(p)");
	R(sys_remove_end_frame, "v(p)");
	R(sys_render, "v()");
	R(sys_foreground, "v()");
	R(sys_resume, "v()");
	R(sys_pause, "v()");
	R(sys_background, "v()");
	R(sys_shutdown, "v()");
	R(sys_drop_files, "v(p)");
	R(sys_foreground_callback, "v()");
	R(sys_resume_callback, "v()");
	R(sys_pause_callback, "v()");
	R(sys_background_callback, "v()");
	R(sys_shutdown_callback, "v()");
	R(sys_drop_files_callback, "v(p)");
	R(sys_keyboard_down_callback, "v(i)");
	R(sys_keyboard_up_callback, "v(i)");
	R(sys_mouse_down_callback, "v(i,i,i)");
	R(sys_mouse_up_callback, "v(i,i,i)");
	R(sys_mouse_move_callback, "v(i,i,i,i)");
	R(sys_mouse_wheel_callback, "v(f)");
	R(sys_touch_down_callback, "v(i,i,i)");
	R(sys_touch_up_callback, "v(i,i,i)");
	R(sys_touch_move_callback, "v(i,i,i)");
	R(sys_pen_down_callback, "v(i,i,f)");
	R(sys_pen_up_callback, "v(i,i,f)");
	R(sys_pen_move_callback, "v(i,i,f)");
	R(data_path, "p()");
	R(video_unload, "v(p)");
	R(sys_buffer_to_string, "p(p)");
	R(sys_string_to_buffer, "p(p)");

	// iron_shape
	R(line_draw_init, "v()");
	minic_register_native("line_draw_render", mn_line_draw_render);
	minic_register_native("line_draw_bounds", mn_line_draw_bounds);
	R(line_draw_lineb, "v(i,i,i,i,i,i)");
	R(line_draw_line, "v(f,f,f,f,f,f)");
	R(line_draw_begin, "v()");
	R(line_draw_end, "v()");
	minic_register_native("shape_draw_sphere", mn_shape_draw_sphere);

	// iron_draw
	R(draw_init, "v(p,p,p,p,p,p,p,p,p,p)");
	R(draw_begin, "v(p,i,i)");
	R(draw_scaled_sub_image, "v(p,f,f,f,f,f,f,f,f)");
	R(draw_scaled_image, "v(p,f,f,f,f)");
	R(draw_sub_image, "v(p,f,f,f,f,f,f)");
	R(draw_image, "v(p,f,f)");
	R(draw_filled_triangle, "v(f,f,f,f,f,f)");
	R(draw_filled_rect, "v(f,f,f,f)");
	R(draw_rect, "v(f,f,f,f,f)");
	R(draw_line, "v(f,f,f,f,f)");
	R(draw_line_aa, "v(f,f,f,f,f)");
	R(draw_string, "v(p,f,f)");
	R(draw_end, "v()");
	R(draw_set_color, "v(i)");
	R(draw_get_color, "i()");
	R(draw_set_pipeline, "v(p)");
	minic_register_native("draw_set_transform", mn_draw_set_transform);
	R(draw_set_font, "i(p,i)");
	R(draw_font_init, "v(p)");
	R(draw_font_destroy, "v(p)");
	R(draw_font_13, "v(p)");
	R(draw_font_has_glyph, "i(i)");
	R(draw_font_add_glyph, "v(i)");
	R(draw_font_init_glyphs, "v(i,i)");
	R(draw_font_count, "i(p)");
	R(draw_font_height, "i(p,i)");
	R(draw_sub_string_width, "f(p,i,p,i,i)");
	R(draw_string_width, "i(p,i,p)");
	R(draw_filled_circle, "v(f,f,f,i)");
	R(draw_circle, "v(f,f,f,i,f)");
	R(draw_cubic_bezier, "v(p,p,i,f)");

	// iron_string
	R(string_alloc, "p(i)");
	R(string_copy, "p(p)");
	R(string_length, "i(p)");
	R(string_equals, "i(p,p)");
	R(i32_to_string, "p(i)");
	R(i32_to_string_hex, "p(i)");
	R(i64_to_string, "p(i)");
	R(u64_to_string, "p(i)");
	R(f32_to_string, "p(f)");
	R(f32_to_string_with_zeros, "p(f)");
	R(string_strip_trailing_zeros, "v(p)");
	R(string_index_of, "i(p,p)");
	R(string_index_of_pos, "i(p,p,i)");
	R(string_last_index_of, "i(p,p)");
	R(string_split, "p(p,p)");
	R(string_array_join, "p(p,p)");
	R(string_replace_all, "p(p,p,p)");
	R(substring, "p(p,i,i)");
	R(string_from_char_code, "p(i)");
	R(char_code_at, "i(p,i)");
	R(char_at, "p(p,i)");
	R(starts_with, "i(p,p)");
	R(ends_with, "i(p,p)");
	R(to_lower_case, "p(p)");
	R(to_upper_case, "p(p)");
	R(trim_end, "p(p)");
	R(string_utf8_decode, "i(p,p)");

	// iron_file
	R(iron_file_reader_open, "i(p,p,i)");
	R(iron_file_reader_close, "i(p)");
	R(iron_file_reader_read, "i(p,p,i)");
	R(iron_file_reader_size, "i(p)");
	R(iron_file_reader_pos, "i(p)");
	R(iron_file_reader_seek, "i(p,i)");
	R(iron_internal_set_files_location, "v(p)");
	R(iron_internal_files_location, "p()");
	R(iron_internal_file_reader_open, "i(p,p,i)");
	R(iron_file_writer_open, "i(p,p)");
	R(iron_file_writer_write, "v(p,p,i)");
	R(iron_file_writer_close, "v(p)");
	R(iron_read_directory, "p(p)");
	R(iron_create_directory, "v(p)");
	R(iron_is_directory, "i(p)");
	R(iron_file_exists, "i(p)");
	R(iron_delete_file, "v(p)");
	R(iron_file_save_bytes, "v(p,p,i)");
	R(iron_file_download, "v(p,p,i,p)");
	R(file_read_directory, "p(p)");
	R(file_copy, "v(p,p)");
	R(file_start, "v(p)");
	R(file_download_to, "v(p,p,p,i)");
	R(file_cache_cloud, "v(p,p,p)");
	R(file_init_cloud, "v(p,p)");

	// iron_gc
	R(gc_alloc, "p(i)");
	R(gc_leaf, "v(p)");
	R(gc_root, "v(p)");
	R(gc_unroot, "v(p)");
	R(gc_realloc, "p(p,i)");
	R(gc_free, "v(p)");
	R(gc_pause, "v()");
	R(gc_resume, "v()");
	R(gc_run, "v()");
	R(gc_start, "v(p)");
	R(gc_stop, "v()");

	// iron_map
	R(i32_map_set, "v(p,p,i)");
	R(f32_map_set, "v(p,p,f)");
	R(any_map_set, "v(p,p,p)");
	R(i32_map_get, "i(p,p)");
	R(f32_map_get, "f(p,p)");
	R(any_map_get, "p(p,p)");
	R(map_delete, "v(p,p)");
	R(map_keys, "p(p)");
	R(i32_map_create, "p()");
	R(any_map_create, "p()");
	R(i32_imap_set, "v(p,i,i)");
	R(any_imap_set, "v(p,i,p)");
	R(i32_imap_get, "i(p,i)");
	R(any_imap_get, "p(p,i)");
	R(imap_delete, "v(p,i)");
	R(imap_keys, "p(p)");
	R(any_imap_create, "p()");

	// iron_array
	R(array_free, "v(p)");
	R(i8_array_push, "v(p,i)");
	R(u8_array_push, "v(p,i)");
	R(i16_array_push, "v(p,i)");
	R(u16_array_push, "v(p,i)");
	R(i32_array_push, "v(p,i)");
	R(u32_array_push, "v(p,i)");
	R(f32_array_push, "v(p,f)");
	R(any_array_push, "v(p,p)");
	R(string_array_push, "v(p,p)");
	R(i8_array_resize, "v(p,i)");
	R(u8_array_resize, "v(p,i)");
	R(i16_array_resize, "v(p,i)");
	R(u16_array_resize, "v(p,i)");
	R(i32_array_resize, "v(p,i)");
	R(u32_array_resize, "v(p,i)");
	R(f32_array_resize, "v(p,i)");
	R(any_array_resize, "v(p,i)");
	R(string_array_resize, "v(p,i)");
	R(buffer_resize, "v(p,i)");
	R(array_sort, "v(p,p)");
	R(i32_array_sort, "v(p,p)");
	R(array_pop, "p(p)");
	R(i32_array_pop, "i(p)");
	R(array_shift, "p(p)");
	R(array_splice, "v(p,i,i)");
	R(i32_array_splice, "v(p,i,i)");
	R(array_concat, "p(p,p)");
	R(array_slice, "p(p,i,i)");
	R(array_insert, "v(p,i,p)");
	R(array_remove, "v(p,p)");
	R(string_array_remove, "v(p,p)");
	R(i32_array_remove, "v(p,i)");
	R(array_index_of, "i(p,p)");
	R(string_array_index_of, "i(p,p)");
	R(i32_array_index_of, "i(p,i)");
	R(array_reverse, "v(p)");
	R(buffer_slice, "p(p,i,i)");
	R(buffer_get_u8, "i(p,i)");
	R(buffer_get_i8, "i(p,i)");
	R(buffer_get_u16, "i(p,i)");
	R(buffer_get_i16, "i(p,i)");
	R(buffer_get_f16, "f(p,i)");
	R(buffer_get_u32, "i(p,i)");
	R(buffer_get_i32, "i(p,i)");
	R(buffer_get_f32, "f(p,i)");
	R(buffer_get_f64, "f(p,i)");
	R(buffer_get_i64, "i(p,i)");
	R(buffer_set_u8, "v(p,i,i)");
	R(buffer_set_i8, "v(p,i,i)");
	R(buffer_set_u16, "v(p,i,i)");
	R(buffer_set_i16, "v(p,i,i)");
	R(buffer_set_u32, "v(p,i,i)");
	R(buffer_set_i32, "v(p,i,i)");
	R(buffer_set_f32, "v(p,i,f)");
	R(buffer_create, "p(i)");
	R(buffer_create_from_raw, "p(p,i)");
	R(f32_array_create, "p(i)");
	R(f32_array_create_from_buffer, "p(p)");
	R(f32_array_create_from_array, "p(p)");
	R(f32_array_create_from_raw, "p(p,i)");
	R(f32_array_create_x, "p(f)");
	R(f32_array_create_xy, "p(f,f)");
	R(f32_array_create_xyz, "p(f,f,f)");
	R(f32_array_create_xyzw, "p(f,f,f,f)");
	R(f32_array_create_xyzwv, "p(f,f,f,f,f)");
	R(u32_array_create, "p(i)");
	R(u32_array_create_from_array, "p(p)");
	R(u32_array_create_from_raw, "p(p,i)");
	R(i32_array_create, "p(i)");
	R(i32_array_create_from_array, "p(p)");
	R(i32_array_create_from_raw, "p(p,i)");
	R(u16_array_create, "p(i)");
	R(u16_array_create_from_raw, "p(p,i)");
	R(i16_array_create, "p(i)");
	R(i16_array_create_from_array, "p(p)");
	R(i16_array_create_from_raw, "p(p,i)");
	R(u8_array_create, "p(i)");
	R(u8_array_create_from_array, "p(p)");
	R(u8_array_create_from_raw, "p(p,i)");
	R(u8_array_create_from_string, "p(p)");
	R(u8_array_to_string, "p(p)");
	R(i8_array_create, "p(i)");
	R(i8_array_create_from_raw, "p(p,i)");
	R(any_array_create, "p(i)");
	R(any_array_create_from_raw, "p(p,i)");
	R(string_array_create, "p(i)");
	R(float_to_half_fast, "i(f)");
	R(half_to_u8_fast, "i(i)");

	// iron_input
	R(input_reset, "v()");
	R(input_end_frame, "v()");
	R(input_on_foreground, "v()");
	R(input_register, "v()");
	R(mouse_end_frame, "v()");
	R(mouse_reset, "v()");
	R(mouse_button_index, "i(p)");
	R(mouse_down, "i(p)");
	R(mouse_down_any, "i()");
	R(mouse_started, "i(p)");
	R(mouse_started_any, "i()");
	R(mouse_released, "i(p)");
	R(mouse_down_listener, "v(i,i,i)");
	R(mouse_up_listener, "v(i,i,i)");
	R(mouse_move_listener, "v(i,i,i,i)");
	R(mouse_wheel_listener, "v(f)");
	R(mouse_view_x, "f()");
	R(mouse_view_y, "f()");
	R(keyboard_end_frame, "v()");
	R(keyboard_reset, "v()");
	R(keyboard_down, "i(p)");
	R(keyboard_started, "i(p)");
	R(keyboard_started_any, "i()");
	R(keyboard_released, "i(p)");
	R(keyboard_repeat, "i(p)");
	R(keyboard_key_code, "p(i)");
	R(keyboard_down_listener, "v(i)");
	R(keyboard_up_listener, "v(i)");

	// paint
	R(plugin_create, "p()");
	R(plugin_notify_on_ui, "v(p,p)");
	R(plugin_notify_on_update, "v(p,p)");
	R(plugin_notify_on_delete, "v(p,p)");
	R(script_notify_on_update, "v(p)");
	R(script_notify_on_next_frame, "v(p)");
	R(console_info, "v(p)");
	R(console_error, "v(p)");
	R(console_log, "v(p)");
	R(ui_box_show_message, "v(p,p,i)");
	R(ui_files_show2, "v(p,i,i,p)");
	R(project_save, "v(i)");
	R(project_filepath_get, "p()");
	R(project_filepath_set, "v(p)");
	R(script_get_context, "p()");
	R(script_get_config, "p()");
	R(script_get_project, "p()");
	R(context_set_viewport_shader, "v(p)");
	R(context_set_viewport_mode, "v(i)");
	R(context_set_camera_controls, "v(i)");
	R(node_shader_write_frag, "v(p,p)");
	R(plugin_register_texture, "v(p,p)");
	R(plugin_unregister_texture, "v(p)");
	R(plugin_register_mesh, "v(p,p)");
	R(plugin_unregister_mesh, "v(p)");
	R(plugin_make_raw_mesh, "p(p,p,p,p,f)");
	R(plugin_material_category_add, "v(p,p)");
	R(plugin_brush_category_add, "v(p,p)");
	R(plugin_material_category_remove, "v(p)");
	R(plugin_brush_category_remove, "v(p)");
	R(plugin_material_custom_nodes_set, "v(p,p)");
	R(plugin_brush_custom_nodes_set, "v(p,p)");
	R(plugin_material_custom_nodes_remove, "v(p)");
	R(plugin_brush_custom_nodes_remove, "v(p)");
	R(plugin_material_kong_get, "p()");
	R(parser_material_parse_value_input, "p(p,i)");
	R(node_shader_write_frag, "v(p,p)");
	R(context_main_object, "p()");
	R(export_texture_run, "v(p,i)");
	R(context_select_tool, "v(i)");

	// json
	R(json_parse, "p(p)");
	R(json_parse_to_map, "p(p)");
	R(json_encode_begin, "v()");
	R(json_encode_end, "p()");
	R(json_encode_string, "v(p,p)");
	R(json_encode_string_array, "v(p,p)");
	R(json_encode_f32, "v(p,f)");
	R(json_encode_i32, "v(p,i)");
	R(json_encode_null, "v(p)");
	R(json_encode_f32_array, "v(p,p)");
	R(json_encode_i32_array, "v(p,p)");
	R(json_encode_bool, "v(p,i)");
	R(json_encode_begin_array, "v(p)");
	R(json_encode_end_array, "v()");
	R(json_encode_begin_object, "v()");
	R(json_encode_end_object, "v()");
	R(json_encode_map, "v(p)");
	R(json_encode_to_armpack, "p(p)");

	// armpack
	R(armpack_decode, "p(p)");
	R(armpack_decode_to_map, "p(p)");
	R(armpack_decode_to_json, "p(p)");
	R(armpack_encode_start, "v(p)");
	R(armpack_encode_end, "i()");
	R(armpack_encode_map, "v(i)");
	R(armpack_encode_array, "v(i)");
	R(armpack_encode_array_f32, "v(p)");
	R(armpack_encode_array_i32, "v(p)");
	R(armpack_encode_array_i16, "v(p)");
	R(armpack_encode_array_u8, "v(p)");
	R(armpack_encode_array_string, "v(p)");
	R(armpack_encode_string, "v(p)");
	R(armpack_encode_i32, "v(i)");
	R(armpack_encode_f32, "v(f)");
	R(armpack_encode_bool, "v(i)");
	R(armpack_encode_null, "v()");
	R(armpack_size_map, "i()");
	R(armpack_size_array, "i()");
	R(armpack_size_array_f32, "i(p)");
	R(armpack_size_array_u8, "i(p)");
	R(armpack_size_string, "i(p)");
	R(armpack_size_i32, "i()");
	R(armpack_size_f32, "i()");
	R(armpack_size_bool, "i()");
	R(armpack_map_get_f32, "f(p,p)");
	R(armpack_map_get_i32, "i(p,p)");
}

#undef R
