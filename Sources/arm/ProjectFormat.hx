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
