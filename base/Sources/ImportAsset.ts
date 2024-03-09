
class ImportAsset {

	static run = (path: string, dropX: f32 = -1.0, dropY: f32 = -1.0, showBox: bool = true, hdrAsEnvmap: bool = true, done: ()=>void = null) => {

		if (path.startsWith("cloud")) {
			let do_cache_cloud = () => {
				file_cache_cloud(path, (abs: string) => {
					if (abs == null) return;
					ImportAsset.run(abs, dropX, dropY, showBox, hdrAsEnvmap, done);
				});
			}

			///if (krom_android || krom_ios)
			base_notify_on_next_frame(() => {
				console_toast(tr("Downloading"));
				base_notify_on_next_frame(do_cache_cloud);
			});
			///else
			do_cache_cloud();
			///end

			return;
		}

		if (path_is_mesh(path)) {
			showBox ? project_import_mesh_box(path) : ImportMesh.run(path);
			if (dropX > 0) UIBox.click_to_hide = false; // Prevent closing when going back to window after drag and drop
		}
		else if (path_is_texture(path)) {
			ImportTexture.run(path, hdrAsEnvmap);
			// Place image node
			let x0: i32 = UINodes.wx;
			let x1: i32 = UINodes.wx + UINodes.ww;
			if (UINodes.show && dropX > x0 && dropX < x1) {
				let asset_index: i32 = 0;
				for (let i: i32 = 0; i < project_assets.length; ++i) {
					if (project_assets[i].file == path) {
						asset_index = i;
						break;
					}
				}
				UINodes.accept_asset_drag(asset_index);
				UINodes.get_nodes().nodesDrag = false;
				UINodes.hwnd.redraws = 2;
			}

			///if is_paint
			if (context_raw.tool == workspace_tool_t.COLORID && project_asset_names.length == 1) {
				UIHeader.header_handle.redraws = 2;
				context_raw.ddirty = 2;
			}
			///end
		}
		else if (path_is_project(path)) {
			ImportArm.run_project(path);
		}
		else if (path_is_plugin(path)) {
			ImportPlugin.run(path);
		}
		else if (path_is_gimp_color_palette(path)) {
			ImportGpl.run(path, false);
		}
		///if is_paint
		else if (path_is_font(path)) {
			ImportFont.run(path);
		}
		else if (path_is_folder(path)) {
			ImportFolder.run(path);
		}
		///end
		else {
			if (context_enable_import_plugin(path)) {
				ImportAsset.run(path, dropX, dropY, showBox);
			}
			else {
				console_error(strings_error1());
			}
		}

		if (done != null) done();
	}
}
