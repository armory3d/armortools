
class Config {

	static raw: config_t = null;
	static keymap: any;
	static config_loaded: bool = false;
	static button_align: zui_align_t = zui_align_t.LEFT;
	static default_button_spacing: string = "       ";
	static button_spacing: string = Config.default_button_spacing;

	static load = (done: ()=>void) => {
		try {
			let blob: ArrayBuffer = data_get_blob((Path.is_protected() ? krom_save_path() : "") + "config.json");
			Config.config_loaded = true;
			Config.raw = JSON.parse(sys_buffer_to_string(blob));
			done();
		}
		catch (e: any) {
			///if krom_linux
			try { // Protected directory
				let blob: ArrayBuffer = data_get_blob(krom_save_path() + "config.json");
				Config.config_loaded = true;
				Config.raw = JSON.parse(sys_buffer_to_string(blob));
				done();
			}
			catch (e: any) {
				krom_log(e);
				done();
			}
			///else
			done();
			///end
		}
	}

	static save = () => {
		// Use system application data folder
		// when running from protected path like "Program Files"
		let path: string = (Path.is_protected() ? krom_save_path() : Path.data() + Path.sep) + "config.json";
		let buffer: buffer_t = sys_string_to_buffer(JSON.stringify(Config.raw));
		krom_file_save_bytes(path, buffer);

		///if krom_linux // Protected directory
		if (!File.exists(path)) krom_file_save_bytes(krom_save_path() + "config.json", buffer);
		///end
	}

	static init = () => {
		if (!Config.config_loaded || Config.raw == null) {
			Config.raw = {};
			Config.raw.locale = "system";
			Config.raw.window_mode = 0;
			Config.raw.window_resizable = true;
			Config.raw.window_minimizable = true;
			Config.raw.window_maximizable = true;
			Config.raw.window_w = 1600;
			Config.raw.window_h = 900;
			///if krom_darwin
			Config.raw.window_w *= 2;
			Config.raw.window_h *= 2;
			///end
			Config.raw.window_x = -1;
			Config.raw.window_y = -1;
			Config.raw.window_scale = 1.0;
			if (sys_display_width() >= 2560 && sys_display_height() >= 1600) {
				Config.raw.window_scale = 2.0;
			}
			///if (krom_android || krom_ios || krom_darwin)
			Config.raw.window_scale = 2.0;
			///end
			Config.raw.window_vsync = true;
			Config.raw.window_frequency = sys_display_frequency();
			Config.raw.rp_bloom = false;
			Config.raw.rp_gi = false;
			Config.raw.rp_vignette = 0.2;
			Config.raw.rp_grain = 0.09;
			Config.raw.rp_motionblur = false;
			///if (krom_android || krom_ios)
			Config.raw.rp_ssao = false;
			///else
			Config.raw.rp_ssao = true;
			///end
			Config.raw.rp_ssr = false;
			Config.raw.rp_supersample = 1.0;
			Config.raw.version = manifest_version;
			Config.raw.sha = Config.get_sha();
			Base.init_config();
		}
		else {
			// Upgrade config format created by older ArmorPaint build
			// if (Config.raw.version != manifest_version) {
			// 	Config.raw.version = manifest_version;
			// 	save();
			// }
			if (Config.raw.sha != Config.get_sha()) {
				Config.config_loaded = false;
				Config.init();
				return;
			}
		}

		zui_set_touch_scroll(Config.raw.touch_ui);
		zui_set_touch_hold(Config.raw.touch_ui);
		zui_set_touch_tooltip(Config.raw.touch_ui);
		Base.res_handle.position = Config.raw.layer_res;
		Config.load_keymap();
	}

	static get_sha = (): string => {
		let sha: string = "";
		let blob: ArrayBuffer = data_get_blob("version.json");
		sha = JSON.parse(sys_buffer_to_string(blob)).sha;
		return sha;
	}

	static get_date = (): string => {
		let date: string = "";
		let blob: ArrayBuffer = data_get_blob("version.json");
		date = JSON.parse(sys_buffer_to_string(blob)).date;
		return date;
	}

	static get_options = (): kinc_sys_ops_t => {
		let windowMode: window_mode_t = Config.raw.window_mode == 0 ? window_mode_t.WINDOWED : window_mode_t.FULLSCREEN;
		let windowFeatures: window_features_t = window_features_t.NONE;
		if (Config.raw.window_resizable) windowFeatures |= window_features_t.RESIZABLE;
		if (Config.raw.window_maximizable) windowFeatures |= window_features_t.MAXIMIZABLE;
		if (Config.raw.window_minimizable) windowFeatures |= window_features_t.MINIMIZABLE;
		let title: string = "untitled - " + manifest_title;
		return {
			title: title,
			width: Config.raw.window_w,
			height: Config.raw.window_h,
			x: Config.raw.window_x,
			y: Config.raw.window_y,
			mode: windowMode,
			features: windowFeatures,
			vsync: Config.raw.window_vsync,
			frequency: Config.raw.window_frequency
		};
	}

	static restore = () => {
		zui_children = new Map(); // Reset ui handles
		Config.config_loaded = false;
		let _layout: i32[] = Config.raw.layout;
		Config.init();
		Config.raw.layout = _layout;
		Base.init_layout();
		Translator.load_translations(Config.raw.locale);
		Config.apply_config();
		Config.load_theme(Config.raw.theme);
	}

	static import_from = (from: config_t) => {
		let _sha: string = Config.raw.sha;
		let _version: string = Config.raw.version;
		Config.raw = from;
		Config.raw.sha = _sha;
		Config.raw.version = _version;
		zui_children = new Map(); // Reset ui handles
		Config.load_keymap();
		Base.init_layout();
		Translator.load_translations(Config.raw.locale);
		Config.apply_config();
		Config.load_theme(Config.raw.theme);
	}

	static apply_config = () => {
		Config.raw.rp_ssao = Context.raw.hssao.selected;
		Config.raw.rp_ssr = Context.raw.hssr.selected;
		Config.raw.rp_bloom = Context.raw.hbloom.selected;
		Config.raw.rp_gi = Context.raw.hvxao.selected;
		Config.raw.rp_supersample = Config.get_super_sample_size(Context.raw.hsupersample.position);
		Config.save();
		Context.raw.ddirty = 2;

		let current: image_t = _g2_current;
		if (current != null) g2_end();
		RenderPathBase.apply_config();
		if (current != null) g2_begin(current);
	}

	static load_keymap = () => {
		if (Config.raw.keymap == "default.json") { // Built-in default
			Config.keymap = Base.default_keymap;
		}
		else {
			let blob: ArrayBuffer = data_get_blob("keymap_presets/" + Config.raw.keymap);
			Config.keymap = JSON.parse(sys_buffer_to_string(blob));
			// Fill in undefined keys with defaults
			for (let field in Base.default_keymap) {
				if (!(field in Config.keymap)) {
					let adefaultKeymap: any = Base.default_keymap;
					Config.keymap[field] = adefaultKeymap[field];
				}
			}
		}
	}

	static save_keymap = () => {
		if (Config.raw.keymap == "default.json") return;
		let path: string = data_path() + "keymap_presets/" + Config.raw.keymap;
		let buffer: buffer_t = sys_string_to_buffer(JSON.stringify(Config.keymap));
		krom_file_save_bytes(path, buffer);
	}

	static get_super_sample_quality = (f: f32): i32 => {
		return f == 0.25 ? 0 :
			   f == 0.5 ? 1 :
			   f == 1.0 ? 2 :
			   f == 1.5 ? 3 :
			   f == 2.0 ? 4 : 5;
	}

	static get_super_sample_size = (i: i32): f32 => {
		return i == 0 ? 0.25 :
			   i == 1 ? 0.5 :
			   i == 2 ? 1.0 :
			   i == 3 ? 1.5 :
			   i == 4 ? 2.0 : 4.0;
	}

	static get_texture_res = (): i32 => {
		let res: i32 = Base.res_handle.position;
		return res == texture_res_t.RES128 ? 128 :
			   res == texture_res_t.RES256 ? 256 :
			   res == texture_res_t.RES512 ? 512 :
			   res == texture_res_t.RES1024 ? 1024 :
			   res == texture_res_t.RES2048 ? 2048 :
			   res == texture_res_t.RES4096 ? 4096 :
			   res == texture_res_t.RES8192 ? 8192 :
			   res == texture_res_t.RES16384 ? 16384 : 0;
	}

	static get_texture_res_x = (): i32 => {
		return Context.raw.project_aspect_ratio == 2 ? Math.floor(Config.get_texture_res() / 2) : Config.get_texture_res();
	}

	static get_texture_res_y = (): i32 => {
		return Context.raw.project_aspect_ratio == 1 ? Math.floor(Config.get_texture_res() / 2) : Config.get_texture_res();
	}

	static get_texture_res_pos = (i: i32): i32 => {
		return i == 128 ? texture_res_t.RES128 :
			   i == 256 ? texture_res_t.RES256 :
			   i == 512 ? texture_res_t.RES512 :
			   i == 1024 ? texture_res_t.RES1024 :
			   i == 2048 ? texture_res_t.RES2048 :
			   i == 4096 ? texture_res_t.RES4096 :
			   i == 8192 ? texture_res_t.RES8192 :
			   i == 16384 ? texture_res_t.RES16384 : 0;
	}

	static load_theme = (theme: string, tagRedraw: bool = true) => {
		if (theme == "default.json") { // Built-in default
			Base.theme = zui_theme_create();
		}
		else {
			let b: ArrayBuffer = data_get_blob("themes/" + theme);
			let parsed: any = JSON.parse(sys_buffer_to_string(b));
			Base.theme = zui_theme_create();
			for (let key in Base.theme) {
				if (key == "theme_") continue;
				if (key.startsWith("set_")) continue;
				if (key.startsWith("get_")) key = key.substr(4);
				let atheme: any = Base.theme;
				atheme[key] = parsed[key];
			}
		}
		Base.theme.FILL_WINDOW_BG = true;
		if (tagRedraw) {
			for (let ui of Base.get_uis()) ui.t = Base.theme;
			UIBase.tag_ui_redraw();
		}
		if (Config.raw.touch_ui) {
			// Enlarge elements
			Base.theme.FULL_TABS = true;
			Base.theme.ELEMENT_H = 24 + 6;
			Base.theme.BUTTON_H = 22 + 6;
			Base.theme.FONT_SIZE = 13 + 2;
			Base.theme.ARROW_SIZE = 5 + 2;
			Base.theme.CHECK_SIZE = 15 + 4;
			Base.theme.CHECK_SELECT_SIZE = 8 + 2;
			Config.button_align = zui_align_t.LEFT;
			Config.button_spacing = "";
		}
		else {
			Base.theme.FULL_TABS = false;
			Config.button_align = zui_align_t.LEFT;
			Config.button_spacing = Config.default_button_spacing;
		}
	}

	static enable_plugin = (f: string) => {
		Config.raw.plugins.push(f);
		Plugin.start(f);
	}

	static disable_plugin = (f: string) => {
		array_remove(Config.raw.plugins, f);
		Plugin.stop(f);
	}
}
