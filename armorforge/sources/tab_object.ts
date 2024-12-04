
function tab_object_draw(htab: ui_handle_t) {
    let ui: ui_t = ui_base_ui;
	if (ui_tab(htab, tr("Object"))) {

        if (context_raw.selected_object != null) {
            let h: ui_handle_t = ui_handle(__ID__);
            h.selected = context_raw.selected_object.visible;
            context_raw.selected_object.visible = ui_check(h, "Visible");

            let t: transform_t = context_raw.selected_object.transform;
            let rot: vec4_t = quat_get_euler(t.rot);
            rot = vec4_mult(rot, 180 / 3.141592);
            let f: f32 = 0.0;
            let changed: bool = false;

            ui_text("Transform", ui_align_t.LEFT, ui.ops.theme.SEPARATOR_COL);

            ui_row4();
            ui_text("Loc");

            h = ui_handle(__ID__);
            h.text = f32_to_string(t.loc.x);
            f = parse_float(ui_text_input(h, "X"));
            if (h.changed) {
                changed = true;
                t.loc.x = f;
            }

            h = ui_handle(__ID__);
            h.text = f32_to_string(t.loc.y);
            f = parse_float(ui_text_input(h, "Y"));
            if (h.changed) {
                changed = true;
                t.loc.y = f;
            }

            h = ui_handle(__ID__);
            h.text = f32_to_string(t.loc.z);
            f = parse_float(ui_text_input(h, "Z"));
            if (h.changed) {
                changed = true;
                t.loc.z = f;
            }

            ui_row4();
            ui_text("Rotation");

            h = ui_handle(__ID__);
            h.text = f32_to_string(rot.x);
            f = parse_float(ui_text_input(h, "X"));
            if (h.changed) {
                changed = true;
                rot.x = f;
            }

            h = ui_handle(__ID__);
            h.text = f32_to_string(rot.y);
            f = parse_float(ui_text_input(h, "Y"));
            if (h.changed) {
                changed = true;
                rot.y = f;
            }

            h = ui_handle(__ID__);
            h.text = f32_to_string(rot.z);
            f = parse_float(ui_text_input(h, "Z"));
            if (h.changed) {
                changed = true;
                rot.z = f;
            }

            ui_row4();
            ui_text("Scale");

            h = ui_handle(__ID__);
            h.text = f32_to_string(t.scale.x);
            f = parse_float(ui_text_input(h, "X"));
            if (h.changed) {
                changed = true;
                t.scale.x = f;
            }

            h = ui_handle(__ID__);
            h.text = f32_to_string(t.scale.y);
            f = parse_float(ui_text_input(h, "Y"));
            if (h.changed) {
                changed = true;
                t.scale.y = f;
            }

            h = ui_handle(__ID__);
            h.text = f32_to_string(t.scale.z);
            f = parse_float(ui_text_input(h, "Z"));
            if (h.changed) {
                changed = true;
                t.scale.z = f;
            }

            ui_row4();
            ui_text("Dimensions");

            h = ui_handle(__ID__);
            h.text = f32_to_string(t.dim.x);
            f = parse_float(ui_text_input(h, "X"));
            if (h.changed) {
                changed = true;
                t.dim.x = f;
            }

            h = ui_handle(__ID__);
            h.text = f32_to_string(t.dim.y);
            f = parse_float(ui_text_input(h, "Y"));
            if (h.changed) {
                changed = true;
                t.dim.y = f;
            }

            h = ui_handle(__ID__);
            h.text = f32_to_string(t.dim.z);
            f = parse_float(ui_text_input(h, "Z"));
            if (h.changed) {
                changed = true;
                t.dim.z = f;
            }

            if (changed) {
                rot = vec4_mult(rot, 3.141592 / 180);
                context_raw.selected_object.transform.rot = quat_from_euler(rot.x, rot.y, rot.z);
                transform_build_matrix(context_raw.selected_object.transform);
                transform_compute_dim(context_raw.selected_object.transform);
                ///if arm_physics
                let pb: physics_body_t = map_get(physics_body_object_map, context_raw.selected_object.uid);
                if (pb != null) {
                    physics_body_sync_transform(pb);
                }
                ///end
            }

            ui_text("Physics", ui_align_t.LEFT, ui.ops.theme.SEPARATOR_COL);

            let pb: physics_body_t = map_get(physics_body_object_map, context_raw.selected_object.uid);

            let hshape: ui_handle_t = ui_handle(__ID__);
            let shape_combo: string[] = [
				tr("None"),
				tr("Box"),
				tr("Sphere"),
				tr("Convex Hull"),
				tr("Mesh"),
			];
            hshape.position = pb != null ? pb.shape + 1 : 0;
			ui_combo(hshape, shape_combo, tr("Shape"), true);

            let hdynamic: ui_handle_t = ui_handle(__ID__);
            hdynamic.selected = pb != null ? pb.mass > 0 : false;
            ui_check(hdynamic, "Dynamic");

            if (hshape.changed || hdynamic.changed) {
                sim_remove(context_raw.selected_object.uid);
                if (hshape.position > 0) {
                    sim_add(context_raw.selected_object, hshape.position - 1, hdynamic.selected ? 1.0 : 0.0);
                }
			}
        }
    }
}
