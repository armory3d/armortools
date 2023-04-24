package arm;

import zui.Nodes;
import iron.data.SceneFormat;

@:structInit class TProjectFormat {
	@:optional public var version: String;
	@:optional public var assets: Array<String>; // texture_assets
	@:optional public var is_bgra: Null<Bool>; // Swapped red and blue channels for layer textures
	@:optional public var packed_assets: Array<TPackedAsset>;
	@:optional public var envmap: String; // Asset name
	@:optional public var envmap_strength: Null<Float>;
	@:optional public var camera_world: kha.arrays.Float32Array;
	@:optional public var camera_origin: kha.arrays.Float32Array;
	@:optional public var camera_fov: Null<Float>;
	@:optional public var swatches: Array<TSwatchColor>;

	#if (is_paint || is_sculpt)
	@:optional public var brush_nodes: Array<TNodeCanvas>;
	@:optional public var brush_icons: Array<haxe.io.Bytes>;
	@:optional public var material_nodes: Array<TNodeCanvas>;
	@:optional public var material_groups: Array<TNodeCanvas>;
	@:optional public var material_icons: Array<haxe.io.Bytes>;
	@:optional public var font_assets: Array<String>;
	@:optional public var layer_datas: Array<TLayerData>;
	@:optional public var mesh_datas: Array<TMeshData>;
	@:optional public var mesh_assets: Array<String>;
	@:optional public var mesh_icons: Array<haxe.io.Bytes>;
	#end

	#if is_paint
	@:optional public var atlas_objects: Array<Int>;
	@:optional public var atlas_names: Array<String>;
	#end

	#if is_lab
	@:optional public var material: TNodeCanvas;
	@:optional public var material_groups: Array<TNodeCanvas>;
	@:optional public var mesh_data: TMeshData;
	@:optional public var mesh_icon: haxe.io.Bytes;
	#end
}

@:structInit class TAsset {
	public var id: Int;
	public var name: String;
	public var file: String;
}

@:structInit class TPackedAsset {
	public var name: String;
	public var bytes: haxe.io.Bytes;
}

@:structInit class TSwatchColor {
	public var base: kha.Color;
	public var opacity: Float;
	public var occlusion: Float;
	public var roughness: Float;
	public var metallic: Float;
	public var normal: kha.Color;
	public var emission: Float;
	public var height: Float;
	public var subsurface: Float;
}

#if (is_paint || is_sculpt)
@:structInit class TLayerData {
	public var name: String;
	public var res: Int; // Width pixels
	public var bpp: Int; // Bits per pixel
	public var texpaint: haxe.io.Bytes;
	public var uv_scale: Float;
	public var uv_rot: Float;
	public var uv_type: Int;
	public var decal_mat: kha.arrays.Float32Array;
	public var opacity_mask: Float;
	public var fill_layer: Int;
	public var object_mask: Int;
	public var blending: Int;
	public var parent: Int;
	public var visible: Bool;
	#if is_paint
	public var texpaint_nor: haxe.io.Bytes;
	public var texpaint_pack: haxe.io.Bytes;
	public var paint_base: Bool;
	public var paint_opac: Bool;
	public var paint_occ: Bool;
	public var paint_rough: Bool;
	public var paint_met: Bool;
	public var paint_nor: Bool;
	public var paint_nor_blend: Bool;
	public var paint_height: Bool;
	public var paint_height_blend: Bool;
	public var paint_emis: Bool;
	public var paint_subs: Bool;
	#end
}
#end
