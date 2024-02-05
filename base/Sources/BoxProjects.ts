
class BoxProjects {

	static htab = Handle.create();
	static hsearch = Handle.create();
	static iconMap: Map<string, image_t> = null;

	static show = () => {
		if (BoxProjects.iconMap != null) {
			for (let handle of BoxProjects.iconMap.keys()) {
				Data.deleteImage(handle);
			}
			BoxProjects.iconMap = null;
		}

		let draggable;
		///if (krom_android || krom_ios)
		draggable = false;
		///else
		draggable = true;
		///end

		UIBox.showCustom((ui: ZuiRaw) => {
			///if (krom_android || krom_ios)
			BoxProjects.alignToFullScreen();
			///end

			///if (krom_android || krom_ios)
			BoxProjects.projectsTab(ui);
			BoxProjects.getStartedTab(ui);
			///else
			BoxProjects.recentProjectsTab(ui);
			///end

		}, 600, 400, null, draggable);
	}

	static projectsTab = (ui: ZuiRaw) => {
		if (Zui.tab(BoxProjects.htab, tr("Projects"), true)) {
			Zui.beginSticky();

			BoxProjects.drawBadge(ui);

			if (Zui.button(tr("New"))) {
				Project.projectNew();
				Viewport.scaleToBounds();
				UIBox.hide();
				// Pick unique name
				let i = 0;
				let j = 0;
				let title = tr("untitled") + i;
				while (j < Config.raw.recent_projects.length) {
					let base = Config.raw.recent_projects[j];
					base = base.substring(base.lastIndexOf(Path.sep) + 1, base.lastIndexOf("."));
					j++;
					if (title == base) {
						i++;
						title = tr("untitled") + i;
						j = 0;
					}
				}
				sys_title_set(title);
			}
			Zui.endSticky();
			Zui.separator(3, false);

			let slotw = Math.floor(150 * Zui.SCALE(ui));
			let num = Math.floor(sys_width() / slotw);
			let recent_projects = Config.raw.recent_projects;
			let show_asset_names = true;

			for (let row = 0; row < Math.ceil(recent_projects.length / num); ++row) {
				let mult = show_asset_names ? 2 : 1;
				let ar = [];
				for (let i = 0; i < num * mult; ++i) ar.push(1 / num);
				Zui.row(ar);

				ui._x += 2;
				let off = show_asset_names ? Zui.ELEMENT_OFFSET(ui) * 16.0 : 6;
				if (row > 0) ui._y += off;

				for (let j = 0; j < num; ++j) {
					let imgw = Math.floor(128 * Zui.SCALE(ui));
					let i = j + row * num;
					if (i >= recent_projects.length) {
						Zui.endElement(imgw);
						if (show_asset_names) Zui.endElement(0);
						continue;
					}

					let path = recent_projects[i];

					///if krom_ios
					let documentDirectory = Krom.saveDialog("", "");
					documentDirectory = documentDirectory.substr(0, documentDirectory.length - 8); // Strip /'untitled'
					path = documentDirectory + path;
					///end

					let iconPath = path.substr(0, path.length - 4) + "_icon.png";
					if (BoxProjects.iconMap == null) BoxProjects.iconMap = new Map();
					let icon = BoxProjects.iconMap.get(iconPath);
					if (icon == null) {
						Data.getImage(iconPath, (image: image_t) => {
							icon = image;
							BoxProjects.iconMap.set(iconPath, icon);
						});
					}

					let uix = ui._x;
					if (icon != null) {
						Zui.fill(0, 0, 128, 128, ui.t.SEPARATOR_COL);

						let state = Zui.image(icon, 0xffffffff, 128  * Zui.SCALE(ui));
						if (state == State.Released) {
							let _uix = ui._x;
							ui._x = uix;
							Zui.fill(0, 0, 128, 128, 0x66000000);
							ui._x = _uix;
							let doImport = () => {
								App.notifyOnInit(() => {
									UIBox.hide();
									ImportArm.runProject(path);
								});
							}

							///if (krom_android || krom_ios)
							Base.notifyOnNextFrame(() => {
								Console.toast(tr("Opening project"));
								Base.notifyOnNextFrame(doImport);
							});
							///else
							doImport();
							///end
						}

						let name = path.substring(path.lastIndexOf(Path.sep) + 1, path.lastIndexOf("."));
						if (ui.isHovered && ui.inputReleasedR) {
							UIMenu.draw((ui: ZuiRaw) => {
								// if (UIMenu.menuButton(ui, tr("Duplicate"))) {}
								if (UIMenu.menuButton(ui, tr("Delete"))) {
									App.notifyOnInit(() => {
										File.delete(path);
										File.delete(iconPath);
										let dataPath = path.substr(0, path.length - 4);
										File.delete(dataPath);
										recent_projects.splice(i, 1);
									});
								}
							}, 1);
						}

						if (show_asset_names) {
							ui._x = uix - (150 - 128) / 2;
							ui._y += slotw * 0.9;
							Zui.text(name, Align.Center);
							if (ui.isHovered) Zui.tooltip(name);
							ui._y -= slotw * 0.9;
							if (i == recent_projects.length - 1) {
								ui._y += j == num - 1 ? imgw : imgw + Zui.ELEMENT_H(ui) + Zui.ELEMENT_OFFSET(ui);
							}
						}
					}
					else {
						Zui.endElement(0);
						if (show_asset_names) Zui.endElement(0);
						ui._x = uix;
					}
				}

				ui._y += 150;
			}
		}
	}

	static recentProjectsTab = (ui: ZuiRaw) => {
		if (Zui.tab(BoxProjects.htab, tr("Recent"), true)) {

			BoxProjects.drawBadge(ui);

			ui.enabled = Config.raw.recent_projects.length > 0;
			BoxProjects.hsearch.text = Zui.textInput(BoxProjects.hsearch, tr("Search"), Align.Left, true, true);
			ui.enabled = true;

			for (let path of Config.raw.recent_projects) {
				let file = path;
				///if krom_windows
				file = path.replaceAll("/", "\\");
				///else
				file = path.replaceAll("\\", "/");
				///end
				file = file.substr(file.lastIndexOf(Path.sep) + 1);

				if (file.toLowerCase().indexOf(BoxProjects.hsearch.text.toLowerCase()) < 0) continue; // Search filter

				if (Zui.button(file, Align.Left) && File.exists(path)) {
					let current = _g2_current;
					if (current != null) g2_end(current);

					ImportArm.runProject(path);

					if (current != null) g2_begin(current, false);
					UIBox.hide();
				}
				if (ui.isHovered) Zui.tooltip(path);
			}

			ui.enabled = Config.raw.recent_projects.length > 0;
			if (Zui.button(tr("Clear"), Align.Left)) {
				Config.raw.recent_projects = [];
				Config.save();
			}
			ui.enabled = true;

			Zui.endElement();
			if (Zui.button(tr("New Project..."), Align.Left)) Project.projectNewBox();
			if (Zui.button(tr("Open..."), Align.Left)) Project.projectOpen();
		}
	}

	static drawBadge = (ui: ZuiRaw) => {
		Data.getImage("badge.k", (img: image_t) => {
			Zui.image(img);
			Zui.endElement();
		});
	}

	static getStartedTab = (ui: ZuiRaw) => {
		if (Zui.tab(BoxProjects.htab, tr("Get Started"), true)) {
			if (Zui.button(tr("Manual"))) {
				File.loadUrl(manifest_url + "/manual");
			}
			if (Zui.button(tr("How To"))) {
				File.loadUrl(manifest_url + "/howto");
			}
			if (Zui.button(tr("What's New"))) {
				File.loadUrl(manifest_url + "/notes");
			}
		}
	}

	static alignToFullScreen = () => {
		UIBox.modalW = Math.floor(sys_width() / Zui.SCALE(Base.uiBox));
		UIBox.modalH = Math.floor(sys_height() / Zui.SCALE(Base.uiBox));
		let appw = sys_width();
		let apph = sys_height();
		let mw = appw;
		let mh = apph;
		UIBox.hwnd.dragX = Math.floor(-appw / 2 + mw / 2);
		UIBox.hwnd.dragY = Math.floor(-apph / 2 + mh / 2);
	}
}
