package arm;

import zui.Nodes;
import iron.data.SceneFormat;

typedef TProjectFormat = {
	public var version:String;
	public var brush_nodes:Array<TNodeCanvas>;
	public var material_nodes:Array<TNodeCanvas>;
	public var assets:Array<String>;
	public var layer_datas:Array<TLayerData>;
	public var mesh_datas:Array<TMeshData>;
}

typedef TLayerData = {
	public var res:Int;
	public var texpaint:haxe.io.Bytes;
	public var texpaint_nor:haxe.io.Bytes;
	public var texpaint_pack:haxe.io.Bytes;
	public var texpaint_mask:haxe.io.Bytes;
	public var opacity_mask:Float;
	public var material_mask:Int;
	public var object_mask:Int;
}

typedef TAPConfig = {
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
	// @:optional var version:Null<Float>;
	@:optional var plugins:Array<String>;
	@:optional var ui_layout:Null<Int>;
	@:optional var undo_steps:Null<Int>;
}
