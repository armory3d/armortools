
class TabTextures {

	static draw = (htab: Handle) => {
		let ui = UIBase.ui;
		let statush = Config.raw.layout[LayoutSize.LayoutStatusH];
		if (ui.tab(htab, tr("Textures")) && statush > UIStatus.defaultStatusH * ui.SCALE()) {

			ui.beginSticky();

			if (Config.raw.touch_ui) {
				ui.row([1 / 4, 1 / 4]);
			}
			else {
				ui.row([1 / 14, 1 / 14]);
			}

			if (ui.button(tr("Import"))) {
				UIFiles.show(Path.textureFormats.join(","), false, true, (path: string) => {
					ImportAsset.run(path, -1.0, -1.0, true, false);
					UIBase.hwnds[TabArea.TabStatus].redraws = 2;
				});
			}
			if (ui.isHovered) ui.tooltip(tr("Import texture file") + ` (${Config.keymap.file_import_assets})`);

			if (ui.button(tr("2D View"))) UIBase.show2DView(View2DType.View2DAsset);

			ui.endSticky();

			if (Project.assets.length > 0) {

				///if (is_paint || is_sculpt)
				let statusw = System.width - UIToolbar.toolbarw - Config.raw.layout[LayoutSize.LayoutSidebarW];
				///end
				///if is_lab
				let statusw = System.width;
				///end

				let slotw = Math.floor(52 * ui.SCALE());
				let num = Math.floor(statusw / slotw);

				for (let row = 0; row < Math.floor(Math.ceil(Project.assets.length / num)); ++row) {
					let mult = Config.raw.show_asset_names ? 2 : 1;
					let ar = [];
					for (let i = 0; i < num * mult; ++i) ar.push(1 / num);
					ui.row(ar);

					ui._x += 2;
					let off = Config.raw.show_asset_names ? ui.ELEMENT_OFFSET() * 10.0 : 6;
					if (row > 0) ui._y += off;

					for (let j = 0; j < num; ++j) {
						let imgw = Math.floor(50 * ui.SCALE());
						let i = j + row * num;
						if (i >= Project.assets.length) {
							ui.endElement(imgw);
							if (Config.raw.show_asset_names) ui.endElement(0);
							continue;
						}

						let asset = Project.assets[i];
						let img = Project.getImage(asset);
						let uix = ui._x;
						let uiy = ui._y;
						let sw = img.height < img.width ? img.height : 0;
						if (ui.image(img, 0xffffffff, slotw, 0, 0, sw, sw) == State.Started && ui.inputY > ui._windowY) {
							let mouse = Input.getMouse();
							Base.dragOffX = -(mouse.x - uix - ui._windowX - 3);
							Base.dragOffY = -(mouse.y - uiy - ui._windowY + 1);
							Base.dragAsset = asset;
							Context.raw.texture = asset;

							if (Time.time() - Context.raw.selectTime < 0.25) UIBase.show2DView(View2DType.View2DAsset);
							Context.raw.selectTime = Time.time();
							UIView2D.hwnd.redraws = 2;
						}

						if (asset == Context.raw.texture) {
							let _uix = ui._x;
							let _uiy = ui._y;
							ui._x = uix;
							ui._y = uiy;
							let off = i % 2 == 1 ? 1 : 0;
							let w = 50;
							ui.fill(0,               0, w + 3,       2, ui.t.HIGHLIGHT_COL);
							ui.fill(0,     w - off + 2, w + 3, 2 + off, ui.t.HIGHLIGHT_COL);
							ui.fill(0,               0,     2,   w + 3, ui.t.HIGHLIGHT_COL);
							ui.fill(w + 2,           0,     2,   w + 4, ui.t.HIGHLIGHT_COL);
							ui._x = _uix;
							ui._y = _uiy;
						}

						let isPacked = Project.raw.packed_assets != null && Project.packedAssetExists(Project.raw.packed_assets, asset.file);

						if (ui.isHovered) {
							ui.tooltipImage(img, 256);
							ui.tooltip(asset.name + (isPacked ? " " + tr("(packed)") : ""));
						}

						if (ui.isHovered && ui.inputReleasedR) {
							Context.raw.texture = asset;

							let count = 0;

							///if (is_paint || is_sculpt)
							count = isPacked ? 6 : 8;
							///end
							///if is_lab
							count = isPacked ? 6 : 6;
							///end

							UIMenu.draw((ui: Zui) => {
								if (UIMenu.menuButton(ui, tr("Export"))) {
									UIFiles.show("png", true, false, (path: string) => {
										Base.notifyOnNextFrame(() => {
											///if (is_paint || is_sculpt)
											if (Base.pipeMerge == null) Base.makePipe();
											///end
											///if is_lab
											if (Base.pipeCopy == null) Base.makePipe();
											///end

											let target = Image.createRenderTarget(TabTextures.to_pow2(img.width), TabTextures.to_pow2(img.height));
											target.g2.begin(false);
											target.g2.pipeline = Base.pipeCopy;
											target.g2.drawScaledImage(img, 0, 0, target.width, target.height);
											target.g2.pipeline = null;
											target.g2.end();
											Base.notifyOnNextFrame(() => {
												let f = UIFiles.filename;
												if (f == "") f = tr("untitled");
												if (!f.endsWith(".png")) f += ".png";
												Krom.writePng(path + Path.sep + f, target.getPixels(), target.width, target.height, 0);
												target.unload();
											});
										});
									});
								}
								if (UIMenu.menuButton(ui, tr("Reimport"))) {
									Project.reimportTexture(asset);
								}

								///if (is_paint || is_sculpt)
								if (UIMenu.menuButton(ui, tr("To Mask"))) {
									Base.notifyOnNextFrame(() => {
										Base.createImageMask(asset);
									});
								}
								///end

								if (UIMenu.menuButton(ui, tr("Set as Envmap"))) {
									Base.notifyOnNextFrame(() => {
										ImportEnvmap.run(asset.file, img);
									});
								}

								///if is_paint
								if (UIMenu.menuButton(ui, tr("Set as Color ID Map"))) {
									Context.raw.colorIdHandle.position = i;
									Context.raw.colorIdPicked = false;
									UIToolbar.toolbarHandle.redraws = 1;
									if (Context.raw.tool == WorkspaceTool.ToolColorId) {
										UIHeader.headerHandle.redraws = 2;
										Context.raw.ddirty = 2;
									}
								}
								///end

								if (UIMenu.menuButton(ui, tr("Delete"), "delete")) {
									TabTextures.deleteTexture(asset);
								}
								if (!isPacked && UIMenu.menuButton(ui, tr("Open Containing Directory..."))) {
									File.start(asset.file.substr(0, asset.file.lastIndexOf(Path.sep)));
								}
								if (!isPacked && UIMenu.menuButton(ui, tr("Open in Browser"))) {
									TabBrowser.showDirectory(asset.file.substr(0, asset.file.lastIndexOf(Path.sep)));
								}
							}, count);
						}

						if (Config.raw.show_asset_names) {
							ui._x = uix;
							ui._y += slotw * 0.9;
							ui.text(Project.assets[i].name, Align.Center);
							if (ui.isHovered) ui.tooltip(Project.assets[i].name);
							ui._y -= slotw * 0.9;
							if (i == Project.assets.length - 1) {
								ui._y += j == num - 1 ? imgw : imgw + ui.ELEMENT_H() + ui.ELEMENT_OFFSET();
							}
						}
					}
				}
			}
			else {
				let img = Res.get("icons.k");
				let r = Res.tile50(img, 0, 1);
				ui.image(img, ui.t.BUTTON_COL, r.h, r.x, r.y, r.w, r.h);
				if (ui.isHovered) ui.tooltip(tr("Drag and drop files here"));
			}

			let inFocus = ui.inputX > ui._windowX && ui.inputX < ui._windowX + ui._windowW &&
						  ui.inputY > ui._windowY && ui.inputY < ui._windowY + ui._windowH;
			if (inFocus && ui.isDeleteDown && Project.assets.length > 0 && Project.assets.indexOf(Context.raw.texture) >= 0) {
				ui.isDeleteDown = false;
				TabTextures.deleteTexture(Context.raw.texture);
			}
		}
	}

	static to_pow2 = (i: i32): i32 => {
		i--;
		i |= i >> 1;
		i |= i >> 2;
		i |= i >> 4;
		i |= i >> 8;
		i |= i >> 16;
		i++;
		return i;
	}

	static updateTexturePointers = (nodes: TNode[], i: i32) => {
		for (let n of nodes) {
			if (n.type == "TEX_IMAGE") {
				if (n.buttons[0].default_value == i) {
					n.buttons[0].default_value = 9999; // Texture deleted, use pink now
				}
				else if (n.buttons[0].default_value > i) {
					n.buttons[0].default_value--; // Offset by deleted texture
				}
			}
		}
	}

	static deleteTexture = (asset: TAsset) => {
		let i = Project.assets.indexOf(asset);
		if (Project.assets.length > 1) {
			Context.raw.texture = Project.assets[i == Project.assets.length - 1 ? i - 1 : i + 1];
		}
		UIBase.hwnds[TabArea.TabStatus].redraws = 2;

		///if is_paint
		if (Context.raw.tool == WorkspaceTool.ToolColorId && i == Context.raw.colorIdHandle.position) {
			UIHeader.headerHandle.redraws = 2;
			Context.raw.ddirty = 2;
			Context.raw.colorIdPicked = false;
			UIToolbar.toolbarHandle.redraws = 1;
		}
		///end

		Data.deleteImage(asset.file);
		Project.assetMap.delete(asset.id);
		Project.assets.splice(i, 1);
		Project.assetNames.splice(i, 1);
		let _next = () => {
			MakeMaterial.parsePaintMaterial();

			///if (is_paint || is_sculpt)
			UtilRender.makeMaterialPreview();
			UIBase.hwnds[TabArea.TabSidebar1].redraws = 2;
			///end
		}
		Base.notifyOnNextFrame(_next);

		for (let m of Project.materials) TabTextures.updateTexturePointers(m.canvas.nodes, i);
		///if (is_paint || is_sculpt)
		for (let b of Project.brushes) TabTextures.updateTexturePointers(b.canvas.nodes, i);
		///end
	}
}
