
function layers_ext_flatten(height_to_normal: bool = false, layers: slot_layer_t[] = null): slot_layer_t {
	let texpaint: image_t = context_raw.brush_output_node_inst.texpaint;
	let texpaint_nor: image_t = context_raw.brush_output_node_inst.texpaint_nor;
	let texpaint_pack: image_t = context_raw.brush_output_node_inst.texpaint_pack;

	let nodes: ui_nodes_t = ui_nodes_get_nodes();
	let canvas: ui_node_canvas_t = ui_nodes_get_canvas(true);
	if (nodes.nodes_selected_id.length > 0) {
		let node: ui_node_t = ui_get_node(canvas.nodes, nodes.nodes_selected_id[0]);
		let brush_node: logic_node_ext_t = parser_logic_get_logic_node(node);
		if (brush_node != null && logic_node_get_cached_image(brush_node.base) != null) {
			texpaint = logic_node_get_cached_image(brush_node.base);
			let texpaint_nor_rt: render_target_t = map_get(render_path_render_targets, "texpaint_nor_empty");
			let texpaint_pack_rt: render_target_t = map_get(render_path_render_targets, "texpaint_pack_empty");
			texpaint_nor = texpaint_nor_rt._image;
			texpaint_pack = texpaint_pack_rt._image;
		}
	}

	let l: slot_layer_t = { texpaint: texpaint, texpaint_nor: texpaint_nor, texpaint_pack: texpaint_pack };
	return l;
}

function layers_ext_on_resized() {
	image_unload(context_raw.brush_output_node_inst.texpaint);
	let texpaint_rt: render_target_t = map_get(render_path_render_targets, "texpaint");
	context_raw.brush_output_node_inst.texpaint = texpaint_rt._image = image_create_render_target(config_get_texture_res_x(), config_get_texture_res_y());

	image_unload(context_raw.brush_output_node_inst.texpaint_nor);
	let texpaint_nor_rt: render_target_t = map_get(render_path_render_targets, "texpaint_nor");
	context_raw.brush_output_node_inst.texpaint_nor = texpaint_nor_rt._image = image_create_render_target(config_get_texture_res_x(), config_get_texture_res_y());

	image_unload(context_raw.brush_output_node_inst.texpaint_pack);
	let texpaint_pack_rt: render_target_t = map_get(render_path_render_targets, "texpaint_pack");
	context_raw.brush_output_node_inst.texpaint_pack = texpaint_pack_rt._image = image_create_render_target(config_get_texture_res_x(), config_get_texture_res_y());

	if (inpaint_node_image != null) {
		image_unload(inpaint_node_image);
		inpaint_node_image = null;
		image_unload(inpaint_node_mask);
		inpaint_node_mask = null;
		inpaint_node_init();
	}

	if (photo_to_pbr_node_images != null) {
		for (let i: i32 = 0; i < photo_to_pbr_node_images.length; ++i) {
			let image: image_t = photo_to_pbr_node_images[i];
			image_unload(image);
		}
		photo_to_pbr_node_images = null;
		photo_to_pbr_node_init();
	}

	if (tiling_node_image != null) {
		image_unload(tiling_node_image);
		tiling_node_image = null;
		tiling_node_init();
	}

	let texpaint_blend0_rt: render_target_t = map_get(render_path_render_targets, "texpaint_blend0");
	image_unload(texpaint_blend0_rt._image);
	texpaint_blend0_rt._image = image_create_render_target(config_get_texture_res_x(), config_get_texture_res_y(), tex_format_t.R8);

	let texpaint_blend1_rt: render_target_t = map_get(render_path_render_targets, "texpaint_blend1");
	image_unload(texpaint_blend1_rt._image);
	texpaint_blend1_rt._image = image_create_render_target(config_get_texture_res_x(), config_get_texture_res_y(), tex_format_t.R8);

	if (map_get(render_path_render_targets, "texpaint_node") != null) {
		map_delete(render_path_render_targets, "texpaint_node");
	}
	if (map_get(render_path_render_targets, "texpaint_node_target") != null) {
		map_delete(render_path_render_targets, "texpaint_node_target");
	}

	app_notify_on_next_frame(function () {
		layers_init();
	});

	render_path_raytrace_ready = false;
}
