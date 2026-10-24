#include "../global.h"

static gpu_texture_t *box_bake_preview;
static bool           box_bake_blender_panel = false, box_bake_risks = true, box_bake_analysis_dirty = true, box_bake_texture_large = false;
static bool           box_bake_maps[4] = {true, false, false, false};
static int            box_bake_rays    = 0;
static float          box_bake_pan_x, box_bake_pan_y;
static float          box_bake_ao_radius = .1f, box_bake_curvature_radius = .1f, box_bake_strength = 2, box_bake_texture_zoom = 1, box_bake_texture_u = .5f,
                      box_bake_texture_v = .5f;
static bn_face       *box_bake_checked_high;
static int            box_bake_result_version, box_bake_analysis_version;
static char          *box_bake_map_names[] = {"Normal", "Ambient Occlusion", "Curvature", "Height"};
static bool           box_bake_low_show = true, box_bake_high_show = true, box_bake_alignment = false, box_bake_add_layer = true;
static bool           box_bake_directx = false, box_bake_backfaces = true, box_bake_antialias = true;
static int            box_bake_resolution = 2, box_bake_padding = 16, box_bake_view = 0, box_bake_source = 0;
static float          box_bake_outer = 0.02f, box_bake_inner = 0.02f, box_bake_yaw = 25, box_bake_pitch = 15, box_bake_zoom = 1;
static bn_v3          box_bake_center;
static float          box_bake_radius     = 1;
static int            box_bake_target_uid = -1;

static void box_bake_fit() {
	bn_v3  min = {INFINITY, INFINITY, INFINITY}, max = {-INFINITY, -INFINITY, -INFINITY};
	mat4_t adjustment = normal_bake_adjustment();
	for (int i = 0; i < normal_bake.low_count; i++)
		for (int k = 0; k < 3; k++) {
			bn_v3 p = normal_bake.low[i].face.p[k];
			min     = bn_min(min, p);
			max     = bn_max(max, p);
		}
	if (!normal_bake.running)
		for (int i = 0; i < normal_bake.high_count; i++)
			for (int k = 0; k < 3; k++) {
				bn_v3 p = normal_bake_point(normal_bake.high[i].p[k], adjustment, 1);
				min     = bn_min(min, p);
				max     = bn_max(max, p);
			}
	if (isfinite(min.x)) {
		box_bake_center = bn_mul(bn_add(min, max), 0.5f);
		box_bake_radius = fmaxf(0.0001f, sqrtf(bn_dot(bn_sub(max, min), bn_sub(max, min))) * 0.5f);
	}
	box_bake_zoom  = 1;
	box_bake_pan_x = box_bake_pan_y = 0;
	normal_bake.preview_dirty       = true;
}

static bn_v3 box_bake_project(bn_v3 p) {
	p         = bn_sub(p, box_bake_center);
	float yaw = box_bake_yaw * math_pi() / 180, pitch = box_bake_pitch * math_pi() / 180;
	float x = cosf(yaw) * p.x - sinf(yaw) * p.y, y = sinf(yaw) * p.x + cosf(yaw) * p.y;
	float z     = cosf(pitch) * p.z + sinf(pitch) * y;
	float scale = fminf(box_bake_preview->width, box_bake_preview->height) * .43f * box_bake_zoom / box_bake_radius;
	return (bn_v3){box_bake_preview->width * (.5f + box_bake_pan_x) + x * scale, box_bake_preview->height * (.5f + box_bake_pan_y) - z * scale, 0};
}

static void box_bake_draw_face(bn_face *f, mat4_t m) {
	bn_v3 p[3];
	for (int k = 0; k < 3; k++)
		p[k] = box_bake_project(normal_bake_point(f->p[k], m, 1));
	for (int k = 0; k < 3; k++)
		draw_line(p[k].x, p[k].y, p[(k + 1) % 3].x, p[(k + 1) % 3].y, 1);
}

static void box_bake_preview_update(int width, int height) {
	if (box_bake_preview && (box_bake_preview->width != width || box_bake_preview->height != height)) {
		gpu_delete_texture(box_bake_preview);
		box_bake_preview = NULL;
	}
	if (!normal_bake.preview_dirty && box_bake_preview)
		return;
	if (!box_bake_preview)
		box_bake_preview = gpu_create_render_target(width, height, GPU_TEXTURE_FORMAT_RGBA32);
	gpu_texture_t *saved  = _draw_current;
	bool           in_use = gpu_in_use;
	if (in_use)
		draw_end();
	draw_begin(box_bake_preview, true, 0xff202329);
	if (box_bake_high_show) {
		draw_set_color(0x88ffad60);
		int    step = normal_bake.high_count / 16000 + 1;
		mat4_t m    = normal_bake_adjustment();
		for (int i = 0; i < normal_bake.high_count; i += step)
			box_bake_draw_face(&normal_bake.high[i], m);
	}
	if (box_bake_low_show) {
		draw_set_color(0xcc62c8ff);
		int      step = normal_bake.low_count / 16000 + 1;
		uint8_t *risk = !normal_bake.running && !box_bake_analysis_dirty
		                    ? (normal_bake.analysis_risk ? normal_bake.analysis_risk : normal_bake_current()->result.face_risk)
		                    : NULL;
		for (int i = 0; i < normal_bake.low_count; i += step) {
			int r = box_bake_risks && risk ? risk[i] : 0;
			draw_set_color(r == 3 ? 0xffff4848 : r == 2 ? 0xffff922e : r == 1 ? 0xffffdf42 : 0xcc62c8ff);
			box_bake_draw_face(&normal_bake.low[i].face, mat4_identity());
		}
	}
	draw_set_color(0xffffffff);
	draw_end();
	if (in_use)
		draw_begin(saved, false, 0);
	normal_bake.preview_dirty = false;
}

static void box_bake_close() {
	bake_normal_close();
	if (box_bake_preview)
		gpu_delete_texture(box_bake_preview);
	box_bake_preview = NULL;
}

static void box_bake_picked(char *path) {
	bake_normal_configure_blender(g_config->blender, "", false);
	box_bake_checked_high   = NULL;
	box_bake_analysis_dirty = true;
	bake_normal_load(path);
	ui_box_hwnd->redraws = 2;
}

static void box_bake_apply_next(void *unused) {
	if (!bake_normal_apply_all(box_bake_add_layer)) {
		snprintf(normal_bake.error, sizeof(normal_bake.error), "Could not apply bake; verify the target still exists");
		ui_box_hwnd->redraws = 2;
	}
	ui_box_hwnd->redraws = 2;
}

static void box_bake_controls() {
	bool busy = normal_bake.running;
	if (!busy && box_bake_target_uid != normal_bake.target_uid) {
		box_bake_target_uid = normal_bake.target_uid;
		box_bake_fit();
		box_bake_outer = box_bake_inner = box_bake_radius * 0.02f;
		box_bake_ao_radius = box_bake_curvature_radius = box_bake_radius * .1f;
	}
	g_ui->enabled            = !busy;
	string_array_t *names    = string_array_create(0);
	i32_array_t    *ids      = i32_array_create(0);
	int             selected = 0, source = -1;
	for (int i = 0; i < g_project->_->paint_objects->length; i++) {
		mesh_object_t *o = g_project->_->paint_objects->buffer[i];
		if (!o->data->index_array->length)
			continue;
		if (o->base->uid == normal_bake.target_uid)
			selected = names->length;
		if (o->base->uid == normal_bake.high_uid)
			source = names->length;
		string_array_push(names, o->base->name);
		i32_array_push(ids, o->base->uid);
	}
	ui_row2();
	ui_combo(&selected, names, tr("Low Poly"), true, UI_ALIGN_LEFT, true);
	if (ui_item_changed() && selected >= 0 && selected < ids->length) {
		bake_normal_select_low(normal_bake_object(ids->buffer[selected]), 0);
		box_bake_checked_high   = NULL;
		box_bake_analysis_dirty = true;
		box_bake_fit();
		box_bake_outer = box_bake_inner = box_bake_radius * 0.02f;
		box_bake_ao_radius = box_bake_curvature_radius = box_bake_radius * .1f;
	}
	int uv = normal_bake.uv;
	ui_combo(&uv, any_array_create_from_raw_tmp((void *[]){"uv0", "uv1"}, 2), tr("UV Map"), true, UI_ALIGN_LEFT, true);
	if (ui_item_changed()) {
		bake_normal_select_low(normal_bake_object(normal_bake.target_uid), uv);
		box_bake_analysis_dirty = true;
	}
	ui_row2();
	ui_combo(&box_bake_source, any_array_create_from_raw_tmp((void *[]){tr("External File"), tr("Project Mesh")}, 2), tr("High Poly"), true, UI_ALIGN_LEFT,
	         true);
	if (box_bake_source == 0) {
		if (ui_button(tr("Load High Poly..."), UI_ALIGN_CENTER, ""))
			ui_files_show("fbx,obj,blend", false, false, box_bake_picked);
		if (!normal_bake.running && normal_bake.source_count) {
			string_array_t *objects = string_array_create(0);
			string_array_push(objects, tr("Choose an object..."));
			for (int i = 0; i < normal_bake.source_count; i++)
				string_array_push(objects, normal_bake.source_names[i]);
			string_array_push(objects, tr("All Objects"));
			int index = normal_bake.high ? (normal_bake.source_index < 0 ? normal_bake.source_count + 1 : normal_bake.source_index + 1) : 0;
			ui_combo(&index, objects, tr("Source Object"), true, UI_ALIGN_LEFT, true);
			if (ui_item_changed() && index > 0) {
				box_bake_checked_high   = NULL;
				box_bake_analysis_dirty = true;
				bake_normal_select_source(index > normal_bake.source_count ? -1 : index - 1);
			}
			array_free(objects);
			free(objects);
		}
	}
	else {
		ui_combo(&source, names, tr("Source"), true, UI_ALIGN_LEFT, true);
		if (ui_item_changed() && source >= 0 && source < ids->length) {
			bake_normal_select_high(normal_bake_object(ids->buffer[source]));
			box_bake_checked_high   = NULL;
			box_bake_analysis_dirty = true;
			box_bake_fit();
		}
	}
	array_free(names);
	free(names);
	array_free(ids);
	free(ids);
	ui_text(normal_bake.path ? normal_bake.path : tr("High-poly source stays outside the paint atlas"), UI_ALIGN_LEFT, 0);
	if (box_bake_source == 0 && ui_panel(&box_bake_blender_panel, tr("Blender Import"), false, false, false)) {
		import_blend_mesh_ui();
	}
	busy          = normal_bake.running;
	g_ui->enabled = !busy;
	if (ui_panel(&box_bake_alignment, tr("High-Poly Alignment"), false, false, false)) {
		bool changed = false;
		ui_row3();
		ui_slider(&normal_bake.offset.x, tr("X"), -box_bake_radius * 2, box_bake_radius * 2, true, 1000, true, UI_ALIGN_LEFT, true);
		changed |= ui_item_changed();
		ui_slider(&normal_bake.offset.y, tr("Y"), -box_bake_radius * 2, box_bake_radius * 2, true, 1000, true, UI_ALIGN_LEFT, true);
		changed |= ui_item_changed();
		ui_slider(&normal_bake.offset.z, tr("Z"), -box_bake_radius * 2, box_bake_radius * 2, true, 1000, true, UI_ALIGN_LEFT, true);
		changed |= ui_item_changed();
		ui_row3();
		ui_slider(&normal_bake.rotation.x, tr("Rotate X"), -180, 180, true, 10, true, UI_ALIGN_LEFT, true);
		changed |= ui_item_changed();
		ui_slider(&normal_bake.rotation.y, tr("Rotate Y"), -180, 180, true, 10, true, UI_ALIGN_LEFT, true);
		changed |= ui_item_changed();
		ui_slider(&normal_bake.rotation.z, tr("Rotate Z"), -180, 180, true, 10, true, UI_ALIGN_LEFT, true);
		changed |= ui_item_changed();
		ui_row2();
		ui_slider(&normal_bake.scale, tr("Scale"), 0.001, 10, true, 1000, true, UI_ALIGN_LEFT, true);
		changed |= ui_item_changed();
		if (ui_button(tr("Reset Alignment"), UI_ALIGN_CENTER, "")) {
			normal_bake.scale  = 1;
			normal_bake.offset = normal_bake.rotation = (vec4_t){0};
			changed                                   = true;
		}
		if (changed) {
			normal_bake.result_stale  = normal_bake_current()->result.rgba != NULL;
			box_bake_analysis_dirty   = true;
			normal_bake.preview_dirty = true;
		}
	}
	ui_text(tr("Maps to Bake"), UI_ALIGN_LEFT, 0);
	int map_mask = 0;
	for (int type = 0; type < 4; type++) {
		if (type % 2 == 0)
			ui_row2();
		ui_check(&box_bake_maps[type], tr(box_bake_map_names[type]), "");
		if (ui_item_changed())
			normal_bake.result_stale = normal_bake_current()->result.rgba != NULL;
		if (box_bake_maps[type])
			map_mask |= 1 << type;
	}
	if (box_bake_maps[BN_AO]) {
		ui_slider(&box_bake_ao_radius, tr("Occlusion Distance"), .0001, box_bake_radius * 2, true, 10000, true, UI_ALIGN_LEFT, true);
		if (ui_item_changed())
			normal_bake.result_stale = normal_bake_current()->result.rgba != NULL;
	}
	if (box_bake_maps[BN_CURVATURE]) {
		ui_slider(&box_bake_curvature_radius, tr("Curvature Radius"), .0001, box_bake_radius * 2, true, 10000, true, UI_ALIGN_LEFT, true);
		if (ui_item_changed())
			normal_bake.result_stale = normal_bake_current()->result.rgba != NULL;
		ui_slider(&box_bake_strength, tr("Curvature Strength"), .1, 10, true, 100, true, UI_ALIGN_LEFT, true);
		if (ui_item_changed())
			normal_bake.result_stale = normal_bake_current()->result.rgba != NULL;
	}
	if (box_bake_maps[BN_AO] || box_bake_maps[BN_CURVATURE]) {
		ui_combo(&box_bake_rays, any_array_create_from_raw_tmp((void *[]){"16", "32", "64", "128"}, 4), tr("Rays per Sample"), true, UI_ALIGN_LEFT, true);
		if (ui_item_changed())
			normal_bake.result_stale = normal_bake_current()->result.rgba != NULL;
	}
	ui_combo(&box_bake_resolution, any_array_create_from_raw_tmp((void *[]){"256", "512", "1024", "2048", "4096"}, 5), tr("Resolution"), true, UI_ALIGN_LEFT,
	         true);
	if (ui_item_changed())
		normal_bake.result_stale = normal_bake_current()->result.rgba != NULL;
	ui_slider(&box_bake_outer, tr("Outer Distance"), 0, box_bake_radius, true, 10000, true, UI_ALIGN_LEFT, true);
	if (ui_item_changed()) {
		normal_bake.result_stale  = normal_bake_current()->result.rgba != NULL;
		box_bake_analysis_dirty   = true;
		normal_bake.preview_dirty = true;
	}
	ui_slider(&box_bake_inner, tr("Inner Distance"), 0, box_bake_radius, true, 10000, true, UI_ALIGN_LEFT, true);
	if (ui_item_changed()) {
		normal_bake.result_stale  = normal_bake_current()->result.rgba != NULL;
		box_bake_analysis_dirty   = true;
		normal_bake.preview_dirty = true;
	}
	ui_row3();
	ui_slider_int(&box_bake_padding, tr("Padding"), 0, 64, true, true, UI_ALIGN_LEFT, true);
	if (ui_item_changed()) {
		normal_bake.result_stale = normal_bake_current()->result.rgba != NULL;
	}
	ui_check(&box_bake_antialias, tr("4 Samples"), "");
	if (ui_item_changed()) {
		normal_bake.result_stale = normal_bake_current()->result.rgba != NULL;
	}
	ui_check(&box_bake_backfaces, tr("Hit Backfaces"), "");
	if (ui_item_changed()) {
		normal_bake.result_stale  = normal_bake_current()->result.rgba != NULL;
		box_bake_analysis_dirty   = true;
		normal_bake.preview_dirty = true;
	}
	if (box_bake_maps[BN_NORMAL]) {
		ui_check(&box_bake_directx, tr("DirectX Normal Map"), "");
		if (ui_item_changed())
			normal_bake.result_stale = normal_bake_current()->result.rgba != NULL;
	}
	if (box_bake_maps[BN_HEIGHT])
		ui_text(tr("Mid-gray = low surface; white = outward."), UI_ALIGN_LEFT, 0);
	if (box_bake_maps[BN_CURVATURE])
		ui_text(tr("White = convex; black = concave. Usable as mask."), UI_ALIGN_LEFT, 0);
	ui_check(&box_bake_add_layer, tr("Create Fill Layer"), "");
	g_ui->enabled = true;
	if (!busy && normal_bake.error[0])
		ui_text(normal_bake.error, UI_ALIGN_LEFT, 0xff6060ff);
	if (!busy)
		ui_text(string_tmp("%s: %d   %s: %d", tr("Low-Poly Triangles"), normal_bake.low_count, tr("High-Poly Triangles"), normal_bake.high_count),
		        UI_ALIGN_LEFT, 0);
	if (normal_bake.status == 3)
		ui_text(string_tmp("%s: %d / %d (%.1f%%)", tr("Missed Texels"), normal_bake_current()->result.missed, normal_bake_current()->result.covered,
		                   normal_bake_current()->result.covered ? 100.0f * normal_bake_current()->result.missed / normal_bake_current()->result.covered : 0),
		        UI_ALIGN_LEFT, 0);
	if (normal_bake.result_stale)
		ui_text(tr("Settings changed - bake again to apply."), UI_ALIGN_LEFT, 0xffffd15a);
	int pending = 0, applied = 0;
	if (!normal_bake.running)
		for (int type = 0; type < 4; type++) {
			normal_bake_output *output = &normal_bake.outputs[type];
			if (output->applied)
				applied++;
			if (output->result.rgba && !output->applied && output->result.covered > output->result.missed)
				pending++;
		}
	if (!pending && applied > 0 && normal_bake.status == 3)
		ui_text(tr("Maps added to the project."), UI_ALIGN_LEFT, 0);
	g_ui->enabled = !normal_bake.running && normal_bake.high && normal_bake.low;
	if (ui_button(tr("Check Projection"), UI_ALIGN_CENTER, "")) {
		if (bake_normal_analyze(box_bake_outer, box_bake_inner, box_bake_backfaces))
			box_bake_analysis_dirty = false;
	}
	busy = normal_bake.running;
	ui_row3();
	g_ui->enabled = !busy && map_mask != 0 && normal_bake.high && normal_bake.low;
	if (ui_button(tr("Bake Selected"), UI_ALIGN_CENTER, "")) {
		bake_normal_set_type(BN_AO, 16 << box_bake_rays, box_bake_ao_radius, 2);
		bake_normal_set_type(BN_CURVATURE, 16 << box_bake_rays, box_bake_curvature_radius, box_bake_strength);
		bake_normal_set_maps(map_mask);
		box_bake_view           = 0;
		box_bake_analysis_dirty = false;
		if (!bake_normal_start(256 << box_bake_resolution, box_bake_outer, box_bake_inner, box_bake_padding, box_bake_directx, box_bake_backfaces,
		                       box_bake_antialias ? 4 : 1))
			snprintf(normal_bake.error, sizeof(normal_bake.error), "Check the target, resolution and projection distances");
	}
	g_ui->enabled = !normal_bake.running && normal_bake.status == 3 && pending > 0 && !normal_bake.result_stale;
	if (ui_button(tr("Apply Maps"), UI_ALIGN_CENTER, ""))
		sys_notify_on_next_frame(box_bake_apply_next, NULL);
	g_ui->enabled = !normal_bake.running;
	if (ui_button(tr("Close"), UI_ALIGN_CENTER, ""))
		ui_box_hide();
}

static void box_bake_viewer(float available) {
	bool busy     = normal_bake.running;
	g_ui->enabled = !busy;
	ui_row3();
	ui_check(&box_bake_low_show, tr("Low Poly"), "");
	if (ui_item_changed())
		normal_bake.preview_dirty = true;
	ui_check(&box_bake_high_show, tr("High Poly"), "");
	if (ui_item_changed())
		normal_bake.preview_dirty = true;
	if (ui_button(tr("Fit Both"), UI_ALIGN_CENTER, ""))
		box_bake_fit();
	bool texture = normal_bake.image != NULL;
	if (!texture || !box_bake_texture_large) {
		int h = (int)fmaxf(150 * UI_SCALE(), available * (texture ? .40f : .74f));
		box_bake_preview_update(g_ui->_w, h);
		ui_image(box_bake_preview, 0xffffffff, h);
		if (!busy && g_ui->is_hovered && g_context->camera_controls != CAMERA_CONTROLS_NONE && g_ui->combo_selected_id == 0) {
			bool rotate = camera_rotate_down(), pan = camera_pan_down();
			bool modif = keyboard_down("alt") || keyboard_down("shift") || keyboard_down("control");
			bool ruler = keyboard_down(any_map_get(g_keymap, "brush_ruler"));
			if (rotate) {
				float speed = g_config->camera_rotation_speed * 180 / math_pi() / (g_context->camera_controls == CAMERA_CONTROLS_ROTATE ? 120 : 100);
				box_bake_yaw += mouse_movement_x * speed;
				box_bake_pitch += ruler ? 0 : mouse_movement_y * speed;
				if (!g_config->camera_upside_down)
					box_bake_pitch = fmaxf(-89.9f, fminf(89.9f, box_bake_pitch));
			}
			if (pan) {
				box_bake_pan_x += mouse_movement_x * g_config->camera_pan_speed / box_bake_preview->width;
				box_bake_pan_y += mouse_movement_y * g_config->camera_pan_speed / box_bake_preview->height;
			}
			float zoom = 0;
			if (keymap_shortcut(any_map_get(g_keymap, "action_zoom"), SHORTCUT_TYPE_DOWN))
				zoom += camera_get_zoom_delta() / 300 * camera_get_zoom_speed();
			if (g_ui->input_wheel_delta && !modif)
				zoom -= g_ui->input_wheel_delta * .05f * camera_get_zoom_speed();
			if (zoom)
				box_bake_zoom = fmaxf(.1f, fminf(8, box_bake_zoom * expf(zoom)));
			if (rotate || pan || zoom) {
				normal_bake.preview_dirty = true;
				ui_box_hwnd->redraws      = 2;
			}
			g_ui->input_wheel_delta = 0;
		}
		ui_row2();
		ui_check(&box_bake_risks, tr("Highlight Projection Risks"), "");
		if (ui_item_changed())
			normal_bake.preview_dirty = true;
		ui_slider(&box_bake_zoom, tr("Zoom"), .1, 8, true, 100, true, UI_ALIGN_LEFT, true);
		if (ui_item_changed())
			normal_bake.preview_dirty = true;
		ui_text(tr("Red: no hit   Yellow: near limit   Orange: steep normal"), UI_ALIGN_LEFT, 0);
		if (box_bake_analysis_dirty)
			ui_text(tr("Use Check Projection to refresh the sampled comparison."), UI_ALIGN_LEFT, 0);
	}
	if (texture) {
		ui_row2();
		string_array_t *results = string_array_create(0);
		int             types[4], count = 0, selected = 0;
		for (int type = 0; type < 4; type++) {
			if (!(bake_normal_result_mask() & (1 << type)))
				continue;
			if (type == normal_bake.active_type)
				selected = count;
			types[count++] = type;
			string_array_push(results, tr(box_bake_map_names[type]));
		}
		string_array_push(results, tr("Projection Hits"));
		if (box_bake_view == 1)
			selected = count;
		ui_combo(&selected, results, tr("Result"), true, UI_ALIGN_LEFT, true);
		if (ui_item_changed()) {
			box_bake_view = selected == count ? 1 : 0;
			if (selected < count)
				bake_normal_select_result(types[selected]);
		}
		array_free(results);
		free(results);
		ui_check(&box_bake_texture_large, tr("Enlarge Texture"), "");
		gpu_texture_t *image = box_bake_view == 1 && normal_bake.diagnostic ? normal_bake.diagnostic : normal_bake.image;
		int            h     = (int)fminf(g_ui->_w, fmaxf(140 * UI_SCALE(), available * (box_bake_texture_large ? .78f : .32f)));
		float          x     = g_ui->_x;
		int            width = g_ui->_w;
		g_ui->_x += (width - h) * .5f;
		g_ui->_w           = h;
		float span         = 1 / box_bake_texture_zoom;
		box_bake_texture_u = fmaxf(span * .5f, fminf(1 - span * .5f, box_bake_texture_u));
		box_bake_texture_v = fmaxf(span * .5f, fminf(1 - span * .5f, box_bake_texture_v));
		ui_sub_image(image, 0xffffffff, h, (int)((box_bake_texture_u - span * .5f) * image->width), (int)((box_bake_texture_v - span * .5f) * image->height),
		             fmaxf(1, span * image->width), fmaxf(1, span * image->height));
		if (!busy && g_ui->is_hovered) {
			if (g_ui->input_down) {
				box_bake_texture_u -= g_ui->input_dx / h * span;
				box_bake_texture_v -= g_ui->input_dy / h * span;
				ui_box_hwnd->redraws = 2;
			}
			if (g_ui->input_wheel_delta) {
				box_bake_texture_zoom   = fmaxf(1, fminf(16, box_bake_texture_zoom * (1 - g_ui->input_wheel_delta * .1f)));
				g_ui->input_wheel_delta = 0;
			}
		}
		g_ui->_x = x;
		g_ui->_w = width;
		ui_row2();
		ui_slider(&box_bake_texture_zoom, tr("Texture Zoom"), 1, 16, true, 100, true, UI_ALIGN_LEFT, true);
		if (ui_button(tr("Reset Texture View"), UI_ALIGN_CENTER, "")) {
			box_bake_texture_zoom = 1;
			box_bake_texture_u = box_bake_texture_v = .5f;
		}
	}
}

static void box_bake_progress_popup(float x, float y, float width, float height) {
	g_ui->enabled = true;
	int phase = normal_bake_read(&normal_bake.phase), progress = normal_bake_read(&normal_bake.progress);
	draw_set_color(0xb0000000);
	draw_filled_rect(x, y, width, height);
	float w = fminf(width - 32 * UI_SCALE(), 420 * UI_SCALE()), h = 170 * UI_SCALE();
	float left = x + (width - w) * .5f, top = y + fmaxf(0, (height - h) * .5f);
	draw_set_color(0xff30343b);
	draw_filled_rect(left, top, w, h);
	draw_set_color(0xffffffff);
	g_ui->_x    = left + 18 * UI_SCALE();
	g_ui->_y    = top + 14 * UI_SCALE();
	g_ui->_w    = w - 36 * UI_SCALE();
	char *title = normal_bake.loading ? tr("Loading High Poly") : normal_bake.analyzing ? tr("Checking Projection") : tr("Preparing Bake");
	if (!normal_bake.loading && !normal_bake.analyzing && phase >= 5)
		title = string_tmp("%s (%d / %d)", tr(box_bake_map_names[normal_bake_read(&normal_bake.current_map)]), normal_bake_read(&normal_bake.map_index) + 1,
		                   normal_bake.map_count);
	ui_text(title, UI_ALIGN_CENTER, 0);
	char *phases[] = {"Blender is evaluating geometry...", "Reading geometry",  "Preparing mesh",  "Transforming geometry",
	                  "Building acceleration structure",   "Projecting texels", "Padding texture", "Checking surface samples"};
	ui_text(phase == 0 ? tr(phases[phase]) : string_tmp("%s - %d%%", tr(phases[phase]), progress), UI_ALIGN_CENTER, 0);
	float bar = g_ui->_w, bar_y = g_ui->_y + 5 * UI_SCALE();
	draw_set_color(0xff181b20);
	draw_filled_rect(g_ui->_x, bar_y, bar, 8 * UI_SCALE());
	draw_set_color(0xff62b7ef);
	if (phase == 0) {
		float pos = (sinf((float)iron_time() * 2) + 1) * .5f;
		draw_filled_rect(g_ui->_x + pos * bar * .7f, bar_y, bar * .3f, 8 * UI_SCALE());
	}
	else
		draw_filled_rect(g_ui->_x, bar_y, bar * fmaxf(0, fminf(1, progress / 100.0f)), 8 * UI_SCALE());
	draw_set_color(0xffffffff);
	g_ui->_y += 28 * UI_SCALE();
	bool cancelled = normal_bake_read(&normal_bake.cancel) != 0;
	g_ui->enabled  = !cancelled;
	if (ui_button(cancelled ? tr("Cancelling...") : tr("Cancel"), UI_ALIGN_CENTER, ""))
		bake_normal_cancel();
	g_ui->enabled        = true;
	ui_box_hwnd->redraws = 2;
}

static void box_bake_draw() {
	if (!normal_bake.running && (box_bake_result_version != normal_bake.result_version || box_bake_analysis_version != normal_bake.analysis_version)) {
		box_bake_result_version   = normal_bake.result_version;
		box_bake_analysis_version = normal_bake.analysis_version;
		box_bake_analysis_dirty   = false;
		normal_bake.preview_dirty = true;
	}
	if (!normal_bake.running && normal_bake.high && box_bake_checked_high != normal_bake.high) {
		box_bake_checked_high = normal_bake.high;
		if (!normal_bake_current()->result.rgba && bake_normal_analyze(box_bake_outer, box_bake_inner, box_bake_backfaces))
			box_bake_analysis_dirty = false;
	}
	float x = g_ui->_x, y = g_ui->_y;
	int   width     = g_ui->_w;
	float gap       = 20 * UI_SCALE();
	int   left      = (int)(width * .36f);
	float available = fmaxf(400 * UI_SCALE(), g_ui->_window_h - 70 * UI_SCALE());
	g_ui->_w        = left;
	box_bake_controls();
	if (!ui_box_show) {
		g_ui->_x      = x;
		g_ui->_w      = width;
		g_ui->enabled = true;
		return;
	}
	float bottom = g_ui->_y;
	g_ui->_x     = x + left + gap;
	g_ui->_y     = y - ui_box_hwnd->scroll_offset;
	g_ui->_w     = width - left - gap;
	box_bake_viewer(available);
	bottom = fmaxf(bottom, g_ui->_y + ui_box_hwnd->scroll_offset);
	if (normal_bake.running)
		box_bake_progress_popup(x, y - ui_box_hwnd->scroll_offset, width, available);
	g_ui->_x      = x;
	g_ui->_w      = width;
	g_ui->_y      = bottom;
	g_ui->enabled = true;
}

void box_bake_normal_show() {
	if (normal_bake.running)
		return;
	bake_normal_close();
	if (!g_config->blender || !g_config->blender[0])
		g_config->blender = normal_bake_find_blender();
	box_bake_view   = 0;
	box_bake_source = 0;
	for (int type = 0; type < 4; type++)
		box_bake_maps[type] = type == BN_NORMAL;
	box_bake_checked_high   = NULL;
	box_bake_analysis_dirty = true;
	box_bake_texture_zoom   = 1;
	mesh_object_t *o        = g_context->paint_object;
	if (!o || !o->data->index_array->length)
		for (int i = 0; i < g_project->_->paint_objects->length; i++)
			if (g_project->_->paint_objects->buffer[i]->data->index_array->length) {
				o = g_project->_->paint_objects->buffer[i];
				break;
			}
	bake_normal_select_low(o, g_context->layer ? g_context->layer->uv_map : 0);
	box_bake_target_uid = normal_bake.target_uid;
	box_bake_fit();
	box_bake_outer = box_bake_inner = box_bake_radius * 0.02f;
	box_bake_ao_radius = box_bake_curvature_radius = box_bake_radius * .1f;
	ui_box_show_custom(box_bake_draw, 1240, 900, box_bake_close, true, tr("Bake Mesh Maps"));
	ui_box_click_to_hide = false;
}

void box_bake_normal_shutdown() {
	extern void (*ui_box_commands)(void);
	if (ui_box_show && ui_box_commands == box_bake_draw)
		ui_box_hide();
	if (box_bake_preview) {
		gpu_delete_texture(box_bake_preview);
		box_bake_preview = NULL;
	}
	bake_normal_shutdown();
}
