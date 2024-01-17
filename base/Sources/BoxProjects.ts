
class BoxProjects {

	static htab = new Handle();
	static hsearch = new Handle();
	static iconMap: Map<string, Image> = null;

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

		UIBox.showCustom((ui: Zui) => {
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

	static projectsTab = (ui: Zui) => {
		if (ui.tab(BoxProjects.htab, tr("Projects"), true)) {
			ui.beginSticky();

			BoxProjects.drawBadge(ui);

			if (ui.button(tr("New"))) {
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
				System.title = title;
			}
			ui.endSticky();
			ui.separator(3, false);

			let slotw = Math.floor(150 * ui.SCALE());
			let num = Math.floor(System.width / slotw);
			let recent_projects = Config.raw.recent_projects;
			let show_asset_names = true;

			for (let row = 0; row < Math.ceil(recent_projects.length / num); ++row) {
				let mult = show_asset_names ? 2 : 1;
				let ar = [];
				for (let i = 0; i < num * mult; ++i) ar.push(1 / num);
				ui.row(ar);

				ui._x += 2;
				let off = show_asset_names ? ui.ELEMENT_OFFSET() * 16.0 : 6;
				if (row > 0) ui._y += off;

				for (let j = 0; j < num; ++j) {
					let imgw = Math.floor(128 * ui.SCALE());
					let i = j + row * num;
					if (i >= recent_projects.length) {
						ui.endElement(imgw);
						if (show_asset_names) ui.endElement(0);
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
						Data.getImage(iconPath, (image: Image) => {
							icon = image;
							BoxProjects.iconMap.set(iconPath, icon);
						});
					}

					let uix = ui._x;
					if (icon != null) {
						ui.fill(0, 0, 128, 128, ui.t.SEPARATOR_COL);

						let state = ui.image(icon, 0xffffffff, 128  * ui.SCALE());
						if (state == State.Released) {
							let _uix = ui._x;
							ui._x = uix;
							ui.fill(0, 0, 128, 128, 0x66000000);
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
							UIMenu.draw((ui: Zui) => {
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
							ui.text(name, Align.Center);
							if (ui.isHovered) ui.tooltip(name);
							ui._y -= slotw * 0.9;
							if (i == recent_projects.length - 1) {
								ui._y += j == num - 1 ? imgw : imgw + ui.ELEMENT_H() + ui.ELEMENT_OFFSET();
							}
						}
					}
					else {
						ui.endElement(0);
						if (show_asset_names) ui.endElement(0);
						ui._x = uix;
					}
				}

				ui._y += 150;
			}
		}
	}

	static recentProjectsTab = (ui: Zui) => {
		if (ui.tab(BoxProjects.htab, tr("Recent"), true)) {

			BoxProjects.drawBadge(ui);

			ui.enabled = Config.raw.recent_projects.length > 0;
			BoxProjects.hsearch.text = ui.textInput(BoxProjects.hsearch, tr("Search"), Align.Left, true, true);
			ui.enabled = true;

			for (let path of Config.raw.recent_projects) {
				let file = path;
				///if krom_windows
				file = path.replace("/", "\\");
				///else
				file = path.replace("\\", "/");
				///end
				file = file.substr(file.lastIndexOf(Path.sep) + 1);

				if (file.toLowerCase().indexOf(BoxProjects.hsearch.text.toLowerCase()) < 0) continue; // Search filter

				if (ui.button(file, Align.Left) && File.exists(path)) {
					let current = Graphics2.current;
					if (current != null) current.end();

					ImportArm.runProject(path);

					if (current != null) current.begin(false);
					UIBox.hide();
				}
				if (ui.isHovered) ui.tooltip(path);
			}

			ui.enabled = Config.raw.recent_projects.length > 0;
			if (ui.button(tr("Clear"), Align.Left)) {
				Config.raw.recent_projects = [];
				Config.save();
			}
			ui.enabled = true;

			ui.endElement();
			if (ui.button(tr("New Project..."), Align.Left)) Project.projectNewBox();
			if (ui.button(tr("Open..."), Align.Left)) Project.projectOpen();
		}
	}

	static drawBadge = (ui: Zui) => {
		Data.getImage("badge.k", (img: Image) => {
			ui.image(img);
			ui.endElement();
		});
	}

	static getStartedTab = (ui: Zui) => {
		if (ui.tab(BoxProjects.htab, tr("Get Started"), true)) {
			if (ui.button(tr("Manual"))) {
				File.loadUrl(Manifest.url + "/manual");
			}
			if (ui.button(tr("How To"))) {
				File.loadUrl(Manifest.url + "/howto");
			}
			if (ui.button(tr("What's New"))) {
				File.loadUrl(Manifest.url + "/notes");
			}
		}
	}

	static alignToFullScreen = () => {
		UIBox.modalW = Math.floor(System.width / Base.uiBox.SCALE());
		UIBox.modalH = Math.floor(System.height / Base.uiBox.SCALE());
		let appw = System.width;
		let apph = System.height;
		let mw = appw;
		let mh = apph;
		UIBox.hwnd.dragX = Math.floor(-appw / 2 + mw / 2);
		UIBox.hwnd.dragY = Math.floor(-apph / 2 + mh / 2);
	}
}
