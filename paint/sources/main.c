

#include <iron.h>
#include "../libs/asim.h"
#include "../libs/asim.h"
#include "enums.c"
#include "types.c"
#include "globals.c"
#include "functions.c"

void gpu_vertex_struct_add(gpu_vertex_structure_t *raw, string_t *name, gpu_vertex_data_t data) {
	gpu_vertex_element_t *e = ADDRESS(ARRAY_ACCESS(raw->elements, raw->size));
	e->name                 = string_copy(name);
	e->data                 = data;
	raw->size++;
}

f32 ui_MENUBAR_H(ui_t *ui) {
	f32 button_offset_y = (ui->ops->theme->ELEMENT_H * UI_SCALE() - ui->ops->theme->BUTTON_H * UI_SCALE()) / (float)2;
	return ui->ops->theme->BUTTON_H * UI_SCALE() * 1.1 + 2 + button_offset_y;
}

f32 ui_nodes_INPUT_Y(ui_node_canvas_t *canvas, ui_node_t *node, i32 pos) {
	return UI_INPUT_Y(canvas, node, pos);
}

ui_handle_t *ui_handle(string_t *s) {
	ui_handle_t *h = any_map_get(ui_children, s);
	if (h == null) {
		h = ui_handle_create();
		any_map_set(ui_children, s, h);
		return h;
	}
	h->init = false;
	return h;
}

ui_t *ui_create(ui_options_t *ops) {
	ui_t *raw = GC_ALLOC_INIT(ui_t, {0});
	ui_init(raw, ops);
	return raw;
}

ui_theme_t *ui_theme_create() {
	ui_theme_t *raw = GC_ALLOC_INIT(ui_theme_t, {0});
	ui_theme_default(raw);
	return raw;
}

void nodes_on_custom_button(i32 node_id, string_t *button_name) {
	void (*f)(i32) = any_map_get(ui_nodes_custom_buttons, button_name);
	f(node_id);
}

ui_nodes_t *ui_nodes_create() {
	ui_nodes_t *raw = GC_ALLOC_INIT(ui_nodes_t, {0});
	ui_nodes_init(raw);
	gc_unroot(ui_nodes_exclude_remove);
	ui_nodes_exclude_remove = any_array_create_from_raw(
	    (any[]){
	        "OUTPUT_MATERIAL_PBR",
	        "GROUP_OUTPUT",
	        "GROUP_INPUT",
	        "BrushOutputNode",
	    },
	    4);
	gc_root(ui_nodes_exclude_remove);
	ui_nodes_on_custom_button = nodes_on_custom_button;
	return raw;
}

ui_node_socket_t *ui_get_socket(ui_node_t_array_t *nodes, i32 id) {
	for (i32 i = 0; i < nodes->length; ++i) {
		ui_node_t *n = nodes->buffer[i];
		for (i32 j = 0; j < n->inputs->length; ++j) {
			ui_node_socket_t *s = n->inputs->buffer[j];
			if (s->id == id) {
				return s;
			}
		}
		for (i32 j = 0; j < n->outputs->length; ++j) {
			ui_node_socket_t *s = n->outputs->buffer[j];
			if (s->id == id) {
				return s;
			}
		}
	}
	return null;
}

void ui_set_font(ui_t *ui, draw_font_t *font) {
	draw_font_init(font);
	ui->ops->font = font;
}


#include "brush_nodes/boolean_node.c"
#include "brush_nodes/brush_output_node.c"
#include "brush_nodes/color_node.c"
#include "brush_nodes/float_node.c"
#include "brush_nodes/input_node.c"
#include "brush_nodes/integer_node.c"
#include "brush_nodes/math_node.c"
#include "brush_nodes/null_node.c"
#include "brush_nodes/random_node.c"
#include "brush_nodes/separate_vector_node.c"
#include "brush_nodes/string_node.c"
#include "brush_nodes/tex_image_node.c"
#include "brush_nodes/time_node.c"
#include "brush_nodes/vector_math_node.c"
#include "brush_nodes/vector_node.c"

#include "material_nodes/attribute_node.c"
#include "material_nodes/blur_node.c"
#include "material_nodes/brick_texture_node.c"
#include "material_nodes/brightness_contrast_node.c"
#include "material_nodes/bump_node.c"
#include "material_nodes/camera_data_node.c"
#include "material_nodes/camera_texture_node.c"
#include "material_nodes/checker_texture_node.c"
#include "material_nodes/clamp_node.c"
#include "material_nodes/color_mask_node.c"
#include "material_nodes/color_ramp_node.c"
#include "material_nodes/combine_xyz_node.c"
#include "material_nodes/curvature_bake_node.c"
#include "material_nodes/fresnel_node.c"
#include "material_nodes/gabor_texture_node.c"
#include "material_nodes/gamma_node.c"
#include "material_nodes/geometry_node.c"
#include "material_nodes/gradient_texture_node.c"
#include "material_nodes/group_node.c"
#include "material_nodes/hue_saturation_value_node.c"
#include "material_nodes/image_texture_node.c"
#include "material_nodes/invert_color_node.c"
#include "material_nodes/layer_mask_node.c"
#include "material_nodes/layer_node.c"
#include "material_nodes/layer_weight_node.c"
#include "material_nodes/magic_texture_node.c"
#include "material_nodes/map_range_node.c"
#include "material_nodes/mapping_node.c"
#include "material_nodes/material_node.c"
#include "material_nodes/material_output_node.c"
#include "material_nodes/math2_node.c"
#include "material_nodes/mix_color_node.c"
#include "material_nodes/mix_normal_map_node.c"
#include "material_nodes/noise_texture_node.c"
#include "material_nodes/normal_map_node.c"
#include "material_nodes/normal_node.c"
#include "material_nodes/object_info_node.c"
#include "material_nodes/picker_node.c"
#include "material_nodes/quantize_node.c"
#include "material_nodes/replace_color_node.c"
#include "material_nodes/rgb_curves_node.c"
#include "material_nodes/rgb_node.c"
#include "material_nodes/rgb_to_bw_node.c"
#include "material_nodes/script_node.c"
#include "material_nodes/separate_xyz_node.c"
#include "material_nodes/shader_node.c"
#include "material_nodes/tangent_node.c"
#include "material_nodes/text_texture_node.c"
#include "material_nodes/texture_coordinate_node.c"
#include "material_nodes/uv_map_node.c"
#include "material_nodes/value_node.c"
#include "material_nodes/vector_curves_node.c"
#include "material_nodes/vector_math2_node.c"
#include "material_nodes/vector_transform_node.c"
#include "material_nodes/voronoi_texture_node.c"
#include "material_nodes/warp_node.c"
#include "material_nodes/wave_texture_node.c"
#include "material_nodes/wavelength_node.c"
#include "material_nodes/wireframe_node.c"

#include "neural_nodes/edit_image_node.c"
#include "neural_nodes/image_to_3d_mesh_node.c"
#include "neural_nodes/image_to_depth_node.c"
#include "neural_nodes/image_to_normal_map_node.c"
#include "neural_nodes/image_to_pbr_node.c"
#include "neural_nodes/inpaint_image_node.c"
#include "neural_nodes/neural_node.c"
#include "neural_nodes/neural_node_models.c"
#include "neural_nodes/outpaint_image_node.c"
#include "neural_nodes/text_to_image_node.c"
#include "neural_nodes/tile_image_node.c"
#include "neural_nodes/upscale_image_node.c"
#include "neural_nodes/vary_image_node.c"

#include "args.c"
#include "base.c"
#include "box_export.c"
#include "box_preferences.c"
#include "box_projects.c"
#include "camera.c"
#include "compass.c"
#include "config.c"
#include "console.c"
#include "context.c"
#include "export_arm.c"
#include "export_exr.c"
#include "export_mesh.c"
#include "export_obj.c"
#include "export_texture.c"
#include "geom.c"
#include "gizmo.c"
#include "history.c"
#include "import_arm.c"
#include "import_asset.c"
#include "import_blend_material.c"
#include "import_blend_mesh.c"
#include "import_envmap.c"
#include "import_folder.c"
#include "import_font.c"
#include "import_keymap.c"
#include "import_legacy.c"
#include "import_mesh.c"
#include "import_obj.c"
#include "import_plugin.c"
#include "import_texture.c"
#include "import_theme.c"
#include "keymap.c"
#include "kickstart.c"
#include "logic_node.c"
#include "make_bake.c"
#include "make_blur.c"
#include "make_brush.c"
#include "make_clone.c"
#include "make_colorid_picker.c"
#include "make_discard.c"
#include "make_material.c"
#include "make_mesh.c"
#include "make_mesh_preview.c"
#include "make_node_preview.c"
#include "make_paint.c"
#include "make_particle.c"
#include "make_sculpt.c"
#include "make_texcoord.c"
#include "manifest.c"
#include "node_shader.c"
#include "nodes_brush.c"
#include "nodes_material.c"
#include "operator.c"
#include "parser_logic.c"
#include "parser_material.c"
#include "physics_body.c"
#include "physics_sim.c"
#include "physics_world.c"
#include "pipes.c"
#include "plugin.c"
#include "project.c"
#include "render_path_base.c"
#include "render_path_deferred.c"
#include "render_path_forward.c"
#include "render_path_paint.c"
#include "render_path_preview.c"
#include "render_path_raytrace.c"
#include "render_path_raytrace_bake.c"
#include "resource.c"
#include "slot_brush.c"
#include "slot_font.c"
#include "slot_layer.c"
#include "slot_material.c"
#include "strings.c"
#include "tab_browser.c"
#include "tab_brushes.c"
#include "tab_console.c"
#include "tab_fonts.c"
#include "tab_history.c"
#include "tab_layers.c"
#include "tab_materials.c"
#include "tab_meshes.c"
#include "tab_plugins.c"
#include "tab_scripts.c"
#include "tab_swatches.c"
#include "tab_textures.c"
#include "translator.c"
#include "ui_box.c"
#include "ui_files.c"
#include "ui_header.c"
#include "ui_menu.c"
#include "ui_menubar.c"
#include "ui_nodes.c"
#include "ui_sidebar.c"
#include "ui_statusbar.c"
#include "ui_toolbar.c"
#include "ui_view2d.c"
#include "uniforms_ext.c"
#include "util_clone.c"
#include "util_encode.c"
#include "util_mesh.c"
#include "util_particle.c"
#include "util_render.c"
#include "util_uv.c"
#include "viewport.c"

void _main() {
	sys_on_resize = base_on_resize;
	sys_on_w      = base_w;
	sys_on_h      = base_h;
	sys_on_x      = base_x;
	sys_on_y      = base_y;

	iron_set_app_name(manifest_title); // Used to locate external application data folder
	config_load();
	config_init();
	context_init();
	sys_start(config_get_options());
	if (config_raw->layout == null) {
		config_init_layout();
	}
	iron_set_app_name(manifest_title);
	scene_set_active("Scene");
	uniforms_ext_init();
	render_path_base_init();
	render_path_deferred_init(); // Allocate gbuffer
	if (config_raw->render_mode == RENDER_MODE_FORWARD) {
		render_path_forward_init();
		gc_unroot(render_path_commands);
		render_path_commands = render_path_forward_commands;
		gc_root(render_path_commands);
	}
	else {
		gc_unroot(render_path_commands);
		render_path_commands = render_path_deferred_commands;
		gc_root(render_path_commands);
	}

	base_init();
}
