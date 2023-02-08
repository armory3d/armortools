package arm;

import zui.Nodes;
import iron.data.SceneFormat;

typedef TProjectFormat = {
	@:optional public var version: String;
	@:optional public var material: TNodeCanvas;
	@:optional public var material_groups: Array<TNodeCanvas>;
	@:optional public var assets: Array<String>; // texture_assets
	@:optional public var mesh_data: TMeshData;
	@:optional public var mesh_icon: haxe.io.Bytes;
	@:optional public var swatches: Array<TSwatchColor>;
	@:optional public var is_bgra: Null<Bool>; // Swapped red and blue channels for layer textures
	@:optional public var packed_assets: Array<TPackedAsset>;
	@:optional public var envmap: String; // Asset name
	@:optional public var envmap_strength: Null<Float>;
	@:optional public var camera_world: kha.arrays.Float32Array;
	@:optional public var camera_origin: kha.arrays.Float32Array;
	@:optional public var camera_fov: Null<Float>;
}

typedef TAsset = {
	public var id: Int;
	public var name: String;
	public var file: String;
}

typedef TPackedAsset = {
	public var name: String;
	public var bytes: haxe.io.Bytes;
}

typedef TSwatchColor = {
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
