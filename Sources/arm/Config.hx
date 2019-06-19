package arm;

import arm.ui.UITrait;

class Config {

	public static function init():TConfig {
		var C:TConfig = cast armory.data.Config.raw;

		if (!armory.data.Config.configLoaded) {
			C.rp_bloom = true;
			C.rp_gi = false;
			C.rp_motionblur = false;
			C.rp_shadowmap_cube = 0;
			C.rp_shadowmap_cascade = 0;
			C.rp_ssgi = true;
			C.rp_ssr = false;
			C.rp_supersample = 1.0;
		}

		if (C.ui_layout == null) C.ui_layout = 0;
		if (C.undo_steps == null) C.undo_steps = 4; // Max steps to keep
		if (C.keymap == null) {
			C.keymap = {};
			C.keymap.action_paint = "left";
			C.keymap.action_rotate = "right";
			C.keymap.action_rotate_light = "middle+shift";
			C.keymap.action_pan = "middle";
			C.keymap.select_material = "shift+number";
			C.keymap.cycle_layers = "ctrl+tab";
			C.keymap.brush_radius = "f";
			C.keymap.brush_ruler = "shift";
			C.keymap.file_new = "ctrl+n";
			C.keymap.file_open = "ctrl+o";
			C.keymap.file_save = "ctrl+s";
			C.keymap.file_save_as = "ctrl+shift+s";
			C.keymap.edit_undo = "ctrl+z";
			C.keymap.edit_redo = "ctrl+shift+z";
			C.keymap.view_reset = "0";
			C.keymap.view_front = "1";
			C.keymap.view_back = "ctrl+1";
			C.keymap.view_right = "3";
			C.keymap.view_left = "ctrl+3";
			C.keymap.view_top = "7";
			C.keymap.view_bottom = "ctrl+7";
			C.keymap.view_camera_type = "5";
			C.keymap.view_orbit_left = "4";
			C.keymap.view_orbit_right = "6";
			C.keymap.view_orbit_top = "8";
			C.keymap.view_orbit_bottom = "2";
			C.keymap.view_orbit_opposite = "9";
			C.keymap.view_distract_free = "f11";
			C.keymap.tool_brush = "b";
			C.keymap.tool_eraser = "e";
			C.keymap.tool_fill = "g";
			C.keymap.tool_decal = "d";
			C.keymap.tool_text = "t";
			C.keymap.tool_clone = "l";
			C.keymap.tool_blur = "u";
			C.keymap.tool_particle = "p";
			C.keymap.tool_bake = "k";
			C.keymap.tool_colorid = "c";
			C.keymap.tool_picker = "v";
			C.keymap.toggle_2d_view = "shift+tab";
			C.keymap.toggle_node_editor = "tab";
			C.keymap.import_assets = "ctrl+shift+i";
			C.keymap.export_textures = "ctrl+shift+e";
			C.keymap.node_search = "space";
		}
		
		return C;
	}

	public static function applyConfig() {
		var C = App.C;
		C.rp_ssgi = UITrait.inst.hssgi.selected;
		C.rp_ssr = UITrait.inst.hssr.selected;
		C.rp_bloom = UITrait.inst.hbloom.selected;
		C.rp_gi = UITrait.inst.hvxao.selected;
		C.rp_supersample = getSuperSampleSize(UITrait.inst.hsupersample.position);
		
		var current = @:privateAccess kha.graphics4.Graphics2.current;
		if (current != null) current.end();
		
		armory.data.Config.save();
		armory.renderpath.RenderPathCreator.applyConfig();
		
		if (current != null) current.begin(false);
		UITrait.inst.ddirty = 2;
	}

	public static inline function getSuperSampleQuality(f:Float):Int {
		return f == 1.0 ? 0 : f == 1.5 ? 1 : f == 2.0 ? 2 : 3;
	}

	public static inline function getSuperSampleSize(i:Int):Float {
		return i == 0 ? 1.0 : i == 1 ? 1.5 : i == 2 ? 2.0 : 4.0;
	}

	public static function getTextureRes():Int {
		var resHandle = UITrait.inst.resHandle;
		if (resHandle.position == 0) return 1024;
		if (resHandle.position == 1) return 2048;
		if (resHandle.position == 2) return 4096;
		if (resHandle.position == 3) return 8192;
		if (resHandle.position == 4) return 16384;
		return 0;
	}

	public static function getTextureResBias():Float {
		var resHandle = UITrait.inst.resHandle;
		if (resHandle.position == 0) return 2.0;
		if (resHandle.position == 1) return 1.5;
		if (resHandle.position == 2) return 1.0;
		if (resHandle.position == 3) return 0.5;
		if (resHandle.position == 4) return 0.25;
		if (resHandle.position == 5) return 0.125;
		return 1.0;
	}

	public static function getTextureResPos(i:Int):Int {
		if (i == 1024) return 0;
		if (i == 2048) return 1;
		if (i == 4096) return 2;
		if (i == 8192) return 3;
		if (i == 16384) return 4;
		return 0;
	}
}

typedef TConfig = {
	@:optional var debug_console:Null<Bool>;
	@:optional var window_mode:Null<Int>; // window, fullscreen
	@:optional var window_w:Null<Int>;
	@:optional var window_h:Null<Int>;
	@:optional var window_resizable:Null<Bool>;
	@:optional var window_maximizable:Null<Bool>;
	@:optional var window_minimizable:Null<Bool>;
	@:optional var window_vsync:Null<Bool>;
	@:optional var window_msaa:Null<Int>;
	@:optional var window_scale:Null<Float>;
	@:optional var rp_supersample:Null<Float>;
	@:optional var rp_shadowmap_cube:Null<Int>; // size
	@:optional var rp_shadowmap_cascade:Null<Int>; // size for single cascade
	@:optional var rp_ssgi:Null<Bool>;
	@:optional var rp_ssr:Null<Bool>;
	@:optional var rp_bloom:Null<Bool>;
	@:optional var rp_motionblur:Null<Bool>;
	@:optional var rp_gi:Null<Bool>;
	// Ext
	@:optional var version:Null<Int>;
	@:optional var plugins:Array<String>;
	@:optional var ui_layout:Null<Int>;
	@:optional var undo_steps:Null<Int>;
	@:optional var keymap:Dynamic; // Map<String, String>
}
