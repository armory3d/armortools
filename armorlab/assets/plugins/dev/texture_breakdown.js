
let plugin = plugin_create();
let h1 = zui_handle_create();
let h2 = zui_handle_create();

let slots = ["base", "occ", "rough", "nor"];
let breakdown = null;

plugin.draw_ui = function(ui) {
	if (zui_panel(h1, "Texture Breakdown")) {

		g2_end();
		draw_breakdown();
		g2_begin(ui);

		// zui_row([1 / 4]);
		// zui_combo(h2, ["Material", "Viewport"], "Type");

		zui_image(breakdown);
		if (ui.is_hovered && ui.input_released_r) {
			let x = ui.input_x - ui._window_x;
			let w = ui._window_w / slots.length;
			let i = (x / w) | 0;
			ui_menu_draw(function(ui) {
				zui_text(slots[i], 2, ui.t.HIGHLIGHT_COL);
				if (zui_button("Delete", 0)) {
					slots.splice(i, 1);
				}
			}, 2);
		}

		zui_row([1 / 4, 1 / 4]);

		if (zui_button("Add")) {
			ui_menu_draw(function(ui) {
				zui_text("Channel", 2, ui.t.HIGHLIGHT_COL);
				if (zui_button("Base Color", 0)) {
					slots.push("base");
				}
				if (zui_button("Occlusion", 0)) {
					slots.push("occ");
				}
				if (zui_button("Roughness", 0)) {
					slots.push("rough");
				}
				if (zui_button("Metallic", 0)) {
					slots.push("metal");
				}
				if (zui_button("Normal Map", 0)) {
					slots.push("nor");
				}
			}, 6);
		}

		if (zui_button("Export")) {
			ui_files_show("png", true, false, function(path) {
				base_notify_on_next_frame(function() {
					var f = ui_files_filename;
					if (f === "") {
						f = "untitled";
					}
					if (!f.endsWith(".png")) {
						f += ".png";
					}
					krom_write_png(path + path_sep + f, image_get_pixels(breakdown), breakdown.width, breakdown.height, 2);
				});
			});
		}
	}
}

function draw_breakdown(type) {
	if (breakdown === null) {
		breakdown = image_create_render_target(4096, 4096);
	}
	g2_begin(breakdown);
	g2_clear(0xff000000);
	g2_disable_scissor();

	if (h2.position === 0) { // Material
		var lay = brush_output_node_inst;
		for (let i = 0; i < slots.length; ++i) {
			g2_set_pipeline(ui_view2d_pipe);
			let image = lay.texpaint;
			let channel = 0;
			if (slots[i] === "occ") {
				image = lay.texpaint_pack;
				channel = 1;
			}
			else if (slots[i] === "rough") {
				image = lay.texpaint_pack;
				channel = 2;
			}
			else if (slots[i] === "metal") {
				image = lay.texpaint_pack;
				channel = 3;
			}
			else if (slots[i] === "nor") {
				image = lay.texpaint_nor;
				channel = 5;
			}
			g4_set_int(ui_view2d_channel_location, channel);
			var step_source = image.width / slots.length;
			var step_dest = breakdown.width / slots.length;
			g2_draw_scaled_sub_image(image, step_source * i, 0, step_source, image.height, step_dest * i, 0, step_dest, breakdown.height);
			g2_end(); // Flush
			g2_begin();
		}
	}
	else { // Viewport
	}

	g2_end();
}
