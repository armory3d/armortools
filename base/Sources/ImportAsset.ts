
class ImportAsset {

	static run = (path: string, dropX = -1.0, dropY = -1.0, showBox = true, hdrAsEnvmap = true, done: ()=>void = null) => {

		if (path.startsWith("cloud")) {
			let doCacheCloud = () => {
				File.cacheCloud(path, (abs: string) => {
					if (abs == null) return;
					ImportAsset.run(abs, dropX, dropY, showBox, hdrAsEnvmap, done);
				});
			}

			///if (krom_android || krom_ios)
			Base.notifyOnNextFrame(() => {
				Console.toast(tr("Downloading"));
				Base.notifyOnNextFrame(doCacheCloud);
			});
			///else
			doCacheCloud();
			///end

			return;
		}

		if (Path.isMesh(path)) {
			showBox ? Project.importMeshBox(path) : ImportMesh.run(path);
			if (dropX > 0) UIBox.clickToHide = false; // Prevent closing when going back to window after drag and drop
		}
		else if (Path.isTexture(path)) {
			ImportTexture.run(path, hdrAsEnvmap);
			// Place image node
			let x0 = UINodes.wx;
			let x1 = UINodes.wx + UINodes.ww;
			if (UINodes.show && dropX > x0 && dropX < x1) {
				let assetIndex = 0;
				for (let i = 0; i < Project.assets.length; ++i) {
					if (Project.assets[i].file == path) {
						assetIndex = i;
						break;
					}
				}
				UINodes.acceptAssetDrag(assetIndex);
				UINodes.getNodes().nodesDrag = false;
				UINodes.hwnd.redraws = 2;
			}

			///if is_paint
			if (Context.raw.tool == WorkspaceTool.ToolColorId && Project.assetNames.length == 1) {
				UIHeader.headerHandle.redraws = 2;
				Context.raw.ddirty = 2;
			}
			///end
		}
		else if (Path.isProject(path)) {
			ImportArm.runProject(path);
		}
		else if (Path.isPlugin(path)) {
			ImportPlugin.run(path);
		}
		else if (Path.isGimpColorPalette(path)) {
			ImportGpl.run(path, false);
		}
		///if is_paint
		else if (Path.isFont(path)) {
			ImportFont.run(path);
		}
		else if (Path.isFolder(path)) {
			ImportFolder.run(path);
		}
		///end
		else {
			if (Context.enableImportPlugin(path)) {
				ImportAsset.run(path, dropX, dropY, showBox);
			}
			else {
				Console.error(Strings.error1());
			}
		}

		if (done != null) done();
	}
}
