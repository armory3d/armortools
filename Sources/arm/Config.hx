package arm;

import haxe.Json;
import haxe.io.Bytes;
import kha.Display;
import iron.data.Data;
#if arm_painter
import arm.ui.UITrait;
import arm.render.Inc;
import arm.sys.Path;
import arm.Tool;
#end

class Config {

	public static var raw: TConfig = null;
	public static var keymap: Dynamic;
	public static var configLoaded = false;

	public static function load(done: Void->Void) {
		try {
			Data.getBlob((Path.isProtected() ? Krom.savePath() : "") + "config.arm", function(blob: kha.Blob) {
				configLoaded = true;
				raw = Json.parse(blob.toString());
				done();
			});
		}
		catch (e: Dynamic) { done(); }
	}

	public static function save() {
		var path = (Path.isProtected() ? Krom.savePath() : Path.data() + Path.sep) + "config.arm";
		var bytes = Bytes.ofString(Json.stringify(raw));
		Krom.fileSaveBytes(path, bytes.getData());
	}

	public static function init() {
		if (raw == null) raw = {};
		if (raw.window_mode == null) raw.window_mode = 0;
		if (raw.window_resizable == null) raw.window_resizable = true;
		if (raw.window_minimizable == null) raw.window_minimizable = true;
		if (raw.window_maximizable == null) raw.window_maximizable = true;
		if (raw.window_w == null) raw.window_w = 1600;
		if (raw.window_h == null) raw.window_h = 900;
		if (raw.window_x == null) raw.window_x = -1;
		if (raw.window_y == null) raw.window_y = -1;
		if (raw.window_scale == null) raw.window_scale = 1.0;
		if (raw.window_vsync == null) raw.window_vsync = true;

		if (!configLoaded) {
			raw.rp_bloom = false;
			raw.rp_gi = false;
			raw.rp_motionblur = false;
			raw.rp_ssgi = true;
			raw.rp_ssr = false;
			raw.rp_supersample = 1.0;
			var disp = Display.primary;
			if (disp != null && disp.width >= 3000 && disp.height >= 2000) {
				raw.window_scale = 2.0;
			}
		}

		#if arm_painter
		if (raw.undo_steps == null) raw.undo_steps = 4; // Max steps to keep
		if (raw.keymap == null) raw.keymap = "default.json";
		loadKeymap();
		#end
	}

	public static function restore() {
		zui.Zui.Handle.global = new zui.Zui.Handle();
		configLoaded = false;
		raw = null;
		init();
		#if arm_painter
		applyConfig();
		#end
	}

	public static inline function getSuperSampleQuality(f: Float): Int {
		return f == 0.25 ? 0 :
			   f == 0.5 ? 1 :
			   f == 1.0 ? 2 :
			   f == 1.5 ? 3 :
			   f == 2.0 ? 4 : 5;
	}

	public static inline function getSuperSampleSize(i: Int): Float {
		return i == 0 ? 0.25 :
			   i == 1 ? 0.5 :
			   i == 2 ? 1.0 :
			   i == 3 ? 1.5 :
			   i == 4 ? 2.0 : 4.0;
	}

	#if arm_painter
	public static function applyConfig() {
		var C = Config.raw;
		C.rp_ssgi = UITrait.inst.hssgi.selected;
		C.rp_ssr = UITrait.inst.hssr.selected;
		C.rp_bloom = UITrait.inst.hbloom.selected;
		C.rp_gi = UITrait.inst.hvxao.selected;
		C.rp_supersample = getSuperSampleSize(UITrait.inst.hsupersample.position);
		iron.object.Uniforms.defaultFilter = C.rp_supersample < 1.0 ? kha.graphics4.TextureFilter.PointFilter : kha.graphics4.TextureFilter.LinearFilter;

		var current = @:privateAccess kha.graphics4.Graphics2.current;
		if (current != null) current.end();

		save();
		Inc.applyConfig();

		if (current != null) current.begin(false);
		Context.ddirty = 2;
	}

	public static function loadKeymap() {
		Data.getBlob("keymap_presets/" + raw.keymap, function(blob: kha.Blob) { // Sync
			keymap = Json.parse(blob.toString());
		});
	}

	public static function saveKeymap() {
		var path = Data.dataPath + "keymap_presets/" + raw.keymap;
		var bytes = Bytes.ofString(Json.stringify(keymap));
		Krom.fileSaveBytes(path, bytes.getData());
	}

	public static function getTextureRes(): Int {
		if (App.resHandle.position == Res128) return 128;
		if (App.resHandle.position == Res256) return 256;
		if (App.resHandle.position == Res512) return 512;
		if (App.resHandle.position == Res1024) return 1024;
		if (App.resHandle.position == Res2048) return 2048;
		if (App.resHandle.position == Res4096) return 4096;
		if (App.resHandle.position == Res8192) return 8192;
		if (App.resHandle.position == Res16384) return 16384;
		return 0;
	}

	public static function getTextureResBias(): Float {
		if (App.resHandle.position == Res128) return 16.0;
		if (App.resHandle.position == Res256) return 8.0;
		if (App.resHandle.position == Res512) return 4.0;
		if (App.resHandle.position == Res1024) return 2.0;
		if (App.resHandle.position == Res2048) return 1.5;
		if (App.resHandle.position == Res4096) return 1.0;
		if (App.resHandle.position == Res8192) return 0.5;
		if (App.resHandle.position == Res16384) return 0.25;
		return 1.0;
	}

	public static function getTextureResPos(i: Int): Int {
		if (i == 128) return Res128;
		if (i == 256) return Res256;
		if (i == 512) return Res512;
		if (i == 1024) return Res1024;
		if (i == 2048) return Res2048;
		if (i == 4096) return Res4096;
		if (i == 8192) return Res8192;
		if (i == 16384) return Res16384;
		return 0;
	}
	#end
}

typedef TConfig = {
	@:optional var window_mode: Null<Int>; // window, fullscreen
	@:optional var window_w: Null<Int>;
	@:optional var window_h: Null<Int>;
	@:optional var window_x: Null<Int>;
	@:optional var window_y: Null<Int>;
	@:optional var window_resizable: Null<Bool>;
	@:optional var window_maximizable: Null<Bool>;
	@:optional var window_minimizable: Null<Bool>;
	@:optional var window_vsync: Null<Bool>;
	@:optional var window_scale: Null<Float>;
	@:optional var rp_supersample: Null<Float>;
	@:optional var rp_ssgi: Null<Bool>;
	@:optional var rp_ssr: Null<Bool>;
	@:optional var rp_bloom: Null<Bool>;
	@:optional var rp_motionblur: Null<Bool>;
	@:optional var rp_gi: Null<Bool>;
	// Ext
	@:optional var version: Null<Int>;
	@:optional var plugins: Array<String>; // List of enabled plugins
	@:optional var bookmarks: Array<String>; // Bookmarked folders in browser
	@:optional var undo_steps: Null<Int>;	// Number of undo steps to preserve
	@:optional var keymap: String; // Link to keymap file
}
