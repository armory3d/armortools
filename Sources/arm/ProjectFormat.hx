package arm;

import zui.Nodes;
import iron.data.SceneFormat;

typedef TProjectFormat = {
	public var version: String;
	@:optional public var brush_nodes: Array<TNodeCanvas>;
	@:optional public var brush_icons: Array<haxe.io.Bytes>;
	@:optional public var material_nodes: Array<TNodeCanvas>;
	@:optional public var material_icons: Array<haxe.io.Bytes>;
	@:optional public var assets: Array<String>; // texture_assets
	@:optional public var layer_datas: Array<TLayerData>;
	@:optional public var mesh_datas: Array<TMeshData>;
	@:optional public var mesh_assets: Array<String>;
	@:optional public var is_bgra: Null<Bool>; // Swapped red and blue channels for layer textures
}

typedef TLayerData = {
	public var name: String;
	public var res: Int; // Width pixels
	public var bpp: Int; // Bits per pixel
	public var texpaint: haxe.io.Bytes;
	public var texpaint_nor: haxe.io.Bytes;
	public var texpaint_pack: haxe.io.Bytes;
	public var texpaint_mask: haxe.io.Bytes;
	public var uv_scale: Float;
	public var uv_rot: Float;
	public var uv_type: Int;
	public var opacity_mask: Float;
	public var material_mask: Int;
	public var object_mask: Int;
	public var blending: Int;
	public var parent: Int;
	public var visible: Bool;
	public var paint_base: Bool;
	public var paint_opac: Bool;
	public var paint_occ: Bool;
	public var paint_rough: Bool;
	public var paint_met: Bool;
	public var paint_nor: Bool;
	public var paint_height: Bool;
	public var paint_emis: Bool;
	public var paint_subs: Bool;
}

typedef TAsset = {
	public var id: Int;
	public var name: String;
	public var file: String;
}
