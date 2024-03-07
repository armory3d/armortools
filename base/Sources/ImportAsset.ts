
class ImportAsset {

	static run = (path: string, dropX: f32 = -1.0, dropY: f32 = -1.0, showBox: bool = true, hdrAsEnvmap: bool = true, done: ()=>void = null) => {

		if (path.startsWith("cloud")) {
			let doCacheCloud = () => {
				File.cache_cloud(path, (abs: string) => {
					if (abs == null) return;
					ImportAsset.run(abs, dropX, dropY, showBox, hdrAsEnvmap, done);
				});
			}

			///if (krom_android || krom_ios)
			Base.notify_on_next_frame(() => {
				Console.toast(tr("Downloading"));
				Base.notify_on_next_frame(doCacheCloud);
			});
			///else
			doCacheCloud();
			///end

			return;
		}

		if (Path.is_mesh(path)) {
			showBox ? Project.import_mesh_box(path) : ImportMesh.run(path);
			if (dropX > 0) UIBox.click_to_hide = false; // Prevent closing when going back to window after drag and drop
		}
		else if (Path.is_texture(path)) {
			ImportTexture.run(path, hdrAsEnvmap);
			// Place image node
			let x0: i32 = UINodes.wx;
			let x1: i32 = UINodes.wx + UINodes.ww;
			if (UINodes.show && dropX > x0 && dropX < x1) {
				let assetIndex: i32 = 0;
				for (let i: i32 = 0; i < Project.assets.length; ++i) {
					if (Project.assets[i].file == path) {
						assetIndex = i;
						break;
					}
				}
				UINodes.accept_asset_drag(assetIndex);
				UINodes.get_nodes().nodesDrag = false;
				UINodes.hwnd.redraws = 2;
			}

			///if is_paint
			if (Context.raw.tool == workspace_tool_t.COLORID && Project.asset_names.length == 1) {
				UIHeader.header_handle.redraws = 2;
				Context.raw.ddirty = 2;
			}
			///end
		}
		else if (Path.is_project(path)) {
			ImportArm.run_project(path);
		}
		else if (Path.is_plugin(path)) {
			ImportPlugin.run(path);
		}
		else if (Path.is_gimp_color_palette(path)) {
			ImportGpl.run(path, false);
		}
		///if is_paint
		else if (Path.is_font(path)) {
			ImportFont.run(path);
		}
		else if (Path.is_folder(path)) {
			ImportFolder.run(path);
		}
		///end
		else {
			if (Context.enable_import_plugin(path)) {
				ImportAsset.run(path, dropX, dropY, showBox);
			}
			else {
				Console.error(Strings.error1());
			}
		}

		if (done != null) done();
	}
}
