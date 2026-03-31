
#include "global.h"

void tab_plugins_draw(ui_handle_t *htab) {
	if (ui_tab(htab, tr("Plugins"), false, -1, false)) {

		ui_begin_sticky();

		f32_array_t *row = f32_array_create_from_raw(
		    (f32[]){
		        -100,
		    },
		    1);
		ui_row(row);

		if (ui_icon_button(tr("Preferences"), ICON_COG, UI_ALIGN_CENTER)) {
			box_preferences_htab->i = PREFERENCES_TAB_PLUGINS;
			box_preferences_show();
		}
		ui_end_sticky();

		// Draw plugins
		string_array_t *keys = map_keys(plugin_map);
		for (i32 i = 0; i < keys->length; ++i) {
			plugin_t *p = any_map_get(plugin_map, keys->buffer[i]);
			if (p->on_ui != NULL) {
				minic_ctx_call_fn(p->ctx, p->on_ui, NULL, 0);
			}
		}

#ifdef is_debug
		string_array_t *rt_keys = map_keys(render_path_render_targets);
		array_sort(rt_keys, NULL);
		for (i32 i = 0; i < rt_keys->length; ++i) {
			render_target_t *rt = any_map_get(render_path_render_targets, rt_keys->buffer[i]);
			ui_text(rt_keys->buffer[i], UI_ALIGN_LEFT, 0x00000000);
			ui_image(rt->_image, 0xffffffff, -1.0);
		}
#endif
	}
}

#ifdef WITH_PLUGINS
void proc_uv_unwrap(void *mesh);
void plugin_uv_unwrap_button() {
	for (i32 i = 0; i < project_paint_objects->length; ++i) {
		mesh_data_t *md = project_paint_objects->buffer[i]->data;
		raw_mesh_t  *mesh =
		    GC_ALLOC_INIT(raw_mesh_t,
		                  {.posa = md->vertex_arrays->buffer[0]->values, .nora = md->vertex_arrays->buffer[1]->values, .texa = NULL, .inda = md->index_array});
		proc_uv_unwrap(mesh);
		md->vertex_arrays->buffer[0]->values = mesh->posa;
		md->vertex_arrays->buffer[1]->values = mesh->nora;
		md->vertex_arrays->buffer[2]->values = mesh->texa;
		md->index_array                      = mesh->inda;
		mesh_data_build(md);
	}
	util_mesh_merge(NULL);
}
#endif
