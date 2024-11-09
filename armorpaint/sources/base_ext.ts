
function base_ext_init() {
}

function base_ext_render() {

    if (context_raw.frame == 2) {
		util_render_make_material_preview();
		ui_base_hwnds[tab_area_t.SIDEBAR1].redraws = 2;

		if (history_undo_layers == null) {
			history_undo_layers = [];
			for (let i: i32 = 0; i < config_raw.undo_steps; ++i) {
				let len: i32 = history_undo_layers.length;
				let ext: string = "_undo" + len;
				let l: slot_layer_t = slot_layer_create(ext);
				array_push(history_undo_layers, l);
			}
		}

        ///if is_sculpt
		app_notify_on_next_frame(function () {
			app_notify_on_next_frame(function () {
				context_raw.project_type = project_model_t.SPHERE;
				project_new();
			});
		});
		///end
    }
}

function base_ext_flatten(height_to_normal: bool = false, layers: slot_layer_t[] = null): slot_layer_t {
	if (layers == null) {
		layers = project_layers;
	}
	base_make_temp_img();
	base_make_export_img();

	let empty_rt: render_target_t = map_get(render_path_render_targets, "empty_white");
	let empty: image_t = empty_rt._image;

	// Clear export layer
	g4_begin(base_expa);
	g4_clear(color_from_floats(0.0, 0.0, 0.0, 0.0));
	g4_end();
	g4_begin(base_expb);
	g4_clear(color_from_floats(0.5, 0.5, 1.0, 0.0));
	g4_end();
	g4_begin(base_expc);
	g4_clear(color_from_floats(1.0, 0.0, 0.0, 0.0));
	g4_end();

	// Flatten layers
	for (let i: i32 = 0; i < layers.length; ++i) {
		let l1: slot_layer_t = layers[i];
		if (!slot_layer_is_visible(l1)) {
			continue;
		}
		if (!slot_layer_is_layer(l1)) {
			continue;
		}

		let mask: image_t = empty;
		let l1masks: slot_layer_t[] = slot_layer_get_masks(l1);
		if (l1masks != null) {
			if (l1masks.length > 1) {
				base_make_temp_mask_img();
				g2_begin(pipes_temp_mask_image);
				g2_clear(0x00000000);
				g2_end();
				let l1: slot_layer_t = { texpaint: pipes_temp_mask_image };
				for (let i: i32 = 0; i < l1masks.length; ++i) {
					base_merge_layer(l1, l1masks[i]);
				}
				mask = pipes_temp_mask_image;
			}
			else {
				mask = l1masks[0].texpaint;
			}
		}

		if (l1.paint_base) {
			g2_begin(base_temp_image); // Copy to temp
			g2_set_pipeline(pipes_copy);
			g2_draw_image(base_expa, 0, 0);
			g2_set_pipeline(null);
			g2_end();

			g4_begin(base_expa);
			g4_set_pipeline(pipes_merge);
			g4_set_tex(pipes_tex0, l1.texpaint);
			g4_set_tex(pipes_tex1, empty);
			g4_set_tex(pipes_texmask, mask);
			g4_set_tex(pipes_texa, base_temp_image);
			g4_set_float(pipes_opac, slot_layer_get_opacity(l1));
			g4_set_int(pipes_blending, layers.length > 1 ? l1.blending : 0);
			g4_set_vertex_buffer(const_data_screen_aligned_vb);
			g4_set_index_buffer(const_data_screen_aligned_ib);
			g4_draw();
			g4_end();
		}

		if (l1.paint_nor) {
			g2_begin(base_temp_image);
			g2_set_pipeline(pipes_copy);
			g2_draw_image(base_expb, 0, 0);
			g2_set_pipeline(null);
			g2_end();

			g4_begin(base_expb);
			g4_set_pipeline(pipes_merge);
			g4_set_tex(pipes_tex0, l1.texpaint);
			g4_set_tex(pipes_tex1, l1.texpaint_nor);
			g4_set_tex(pipes_texmask, mask);
			g4_set_tex(pipes_texa, base_temp_image);
			g4_set_float(pipes_opac, slot_layer_get_opacity(l1));
			g4_set_int(pipes_blending, l1.paint_nor_blend ? -2 : -1);
			g4_set_vertex_buffer(const_data_screen_aligned_vb);
			g4_set_index_buffer(const_data_screen_aligned_ib);
			g4_draw();
			g4_end();
		}

		if (l1.paint_occ || l1.paint_rough || l1.paint_met || l1.paint_height) {
			g2_begin(base_temp_image);
			g2_set_pipeline(pipes_copy);
			g2_draw_image(base_expc, 0, 0);
			g2_set_pipeline(null);
			g2_end();

			if (l1.paint_occ && l1.paint_rough && l1.paint_met && l1.paint_height) {
				base_commands_merge_pack(pipes_merge, base_expc, l1.texpaint, l1.texpaint_pack, slot_layer_get_opacity(l1), mask, l1.paint_height_blend ? -3 : -1);
			}
			else {
				if (l1.paint_occ) {
					base_commands_merge_pack(pipes_merge_r, base_expc, l1.texpaint, l1.texpaint_pack, slot_layer_get_opacity(l1), mask);
				}
				if (l1.paint_rough) {
					base_commands_merge_pack(pipes_merge_g, base_expc, l1.texpaint, l1.texpaint_pack, slot_layer_get_opacity(l1), mask);
				}
				if (l1.paint_met) {
					base_commands_merge_pack(pipes_merge_b, base_expc, l1.texpaint, l1.texpaint_pack, slot_layer_get_opacity(l1), mask);
				}
			}
		}
	}

	///if arm_metal
	// Flush command list
	g2_begin(base_expa);
	g2_end();
	g2_begin(base_expb);
	g2_end();
	g2_begin(base_expc);
	g2_end();
	///end

	let l0: slot_layer_t = {
		texpaint: base_expa,
		texpaint_nor: base_expb,
		texpaint_pack: base_expc
	};

	// Merge height map into normal map
	if (height_to_normal && make_material_height_used) {

		g2_begin(base_temp_image);
		g2_set_pipeline(pipes_copy);
		g2_draw_image(l0.texpaint_nor, 0, 0);
		g2_set_pipeline(null);
		g2_end();

		g4_begin(l0.texpaint_nor);
		g4_set_pipeline(pipes_merge);
		g4_set_tex(pipes_tex0, base_temp_image);
		g4_set_tex(pipes_tex1, l0.texpaint_pack);
		g4_set_tex(pipes_texmask, empty);
		g4_set_tex(pipes_texa, empty);
		g4_set_float(pipes_opac, 1.0);
		g4_set_int(pipes_blending, -4);
		g4_set_vertex_buffer(const_data_screen_aligned_vb);
		g4_set_index_buffer(const_data_screen_aligned_ib);
		g4_draw();
		g4_end();
	}

	return l0;
}

function base_ext_on_layers_resized() {
	app_notify_on_init(function () {
		base_resize_layers();
		let _layer: slot_layer_t = context_raw.layer;
		let _material: slot_material_t = context_raw.material;
		for (let i: i32 = 0; i < project_layers.length; ++i) {
			let l: slot_layer_t = project_layers[i];
			if (l.fill_layer != null) {
				context_raw.layer = l;
				context_raw.material = l.fill_layer;
				base_update_fill_layer();
			}
		}
		context_raw.layer = _layer;
		context_raw.material = _material;
		make_material_parse_paint_material();
	});
	util_uv_uvmap = null;
	util_uv_uvmap_cached = false;
	util_uv_trianglemap = null;
	util_uv_trianglemap_cached = false;
	util_uv_dilatemap_cached = false;
	///if (arm_direct3d12 || arm_vulkan || arm_metal)
	render_path_raytrace_ready = false;
	///end
}
