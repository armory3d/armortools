package arm;

import zui.Nodes;
import iron.data.SceneFormat;
import arm.ProjectBaseFormat;

@:structInit class TProjectFormat extends TProjectBaseFormat {
	@:optional public var material: TNodeCanvas;
	@:optional public var material_groups: Array<TNodeCanvas>;
	@:optional public var mesh_data: TMeshData;
	@:optional public var mesh_icon: haxe.io.Bytes;
	@:optional public var swatches: Array<TSwatchColor>;
}
