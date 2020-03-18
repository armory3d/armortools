package arm;

import iron.object.Object;
import iron.object.MeshObject;
import arm.data.MaterialSlot;
import arm.data.LayerSlot;
import arm.data.BrushSlot;
import arm.util.UVUtil;
import arm.util.RenderUtil;
import arm.util.ParticleUtil;
import arm.ui.UISidebar;
import arm.ui.UIToolbar;
import arm.ui.UINodes;
import arm.ui.UIView2D;
import arm.ui.UIHeader;
import arm.node.MaterialParser;
import arm.Enums;
import arm.Project;

class Context {

	public static var material: MaterialSlot;
	public static var materialScene: MaterialSlot;
	public static var layer: LayerSlot;
	public static var layerIsMask = false; // Mask selected for active layer
	public static var brush: BrushSlot;
	public static var texture: TAsset = null;
	public static var object: Object;
	public static var paintObject: MeshObject;
	public static var mergedObject: MeshObject = null; // For object mask
	public static var tool = 0;

	public static var ddirty = 0; // depth
	public static var pdirty = 0; // paint
	public static var rdirty = 0; // render
	public static var brushBlendDirty = true;
	public static var layerPreviewDirty = true;
	public static var layersPreviewDirty = false;

	public static function selectMaterialScene(i: Int) {
		if (Project.materialsScene.length <= i || object == paintObject) return;
		materialScene = Project.materialsScene[i];
		if (Std.is(object, MeshObject)) {
			var mats = cast(object, MeshObject).materials;
			for (i in 0...mats.length) mats[i] = materialScene.data;
		}
		MaterialParser.parsePaintMaterial();
		UISidebar.inst.hwnd.redraws = 2;
	}

	public static function selectMaterial(i: Int) {
		if (Project.materials.length <= i) return;
		setMaterial(Project.materials[i]);
	}

	public static function setMaterial(m: MaterialSlot) {
		if (Project.materials.indexOf(m) == -1) return;
		material = m;
		MaterialParser.parsePaintMaterial();
		UISidebar.inst.hwnd1.redraws = 2;
		UIHeader.inst.headerHandle.redraws = 2;
		UINodes.inst.hwnd.redraws = 2;

		var decal = tool == ToolDecal || tool == ToolText;
		if (decal) {
			var current = @:privateAccess kha.graphics4.Graphics2.current;
			if (current != null) current.end();
			RenderUtil.makeDecalPreview();
			if (current != null) current.begin(false);
		}
	}

	public static function selectBrush(i: Int) {
		if (Project.brushes.length <= i) return;
		setBrush(Project.brushes[i]);
	}

	public static function setBrush(b: BrushSlot) {
		if (Project.brushes.indexOf(b) == -1) return;
		brush = b;
		MaterialParser.parseBrush();
		UISidebar.inst.parseBrushInputs();
		UISidebar.inst.hwnd1.redraws = 2;
		UINodes.inst.hwnd.redraws = 2;
	}

	public static function setLayer(l: LayerSlot, isMask = false) {
		if (l == layer && layerIsMask == isMask) return;
		layer = l;
		layerIsMask = isMask;
		UIHeader.inst.headerHandle.redraws = 2;

		var current = @:privateAccess kha.graphics4.Graphics2.current;
		if (current != null) current.end();

		Layers.setObjectMask();
		MaterialParser.parseMeshMaterial();
		MaterialParser.parsePaintMaterial();

		if (current != null) current.begin(false);

		UISidebar.inst.hwnd.redraws = 2;
		UIView2D.inst.hwnd.redraws = 2;
	}

	public static function selectTool(i: Int) {
		tool = i;
		MaterialParser.parsePaintMaterial();
		MaterialParser.parseMeshMaterial();
		UIHeader.inst.headerHandle.redraws = 2;
		UIToolbar.inst.toolbarHandle.redraws = 2;
		ddirty = 3;

		var decal = tool == ToolDecal || tool == ToolText;
		if (decal) {
			var current = @:privateAccess kha.graphics4.Graphics2.current;
			if (current != null) current.end();

			if (tool == ToolText) {
				RenderUtil.makeTextPreview();
			}

			RenderUtil.makeDecalPreview();

			if (current != null) current.begin(false);
		}

		if (tool == ToolParticle) {
			ParticleUtil.initParticle();
			MaterialParser.parseParticleMaterial();
		}
	}

	public static function selectObject(o: Object) {
		object = o;

		if (UISidebar.inst.worktab.position == SpaceScene) {
			if (Std.is(o, MeshObject)) {
				for (i in 0...Project.materialsScene.length) {
					if (Project.materialsScene[i].data == cast(o, MeshObject).materials[0]) {
						// selectMaterial(i); // loop
						materialScene = Project.materialsScene[i];
						UISidebar.inst.hwnd.redraws = 2;
						break;
					}
				}
			}
		}
	}

	public static function selectPaintObject(o: MeshObject) {
		UIHeader.inst.headerHandle.redraws = 2;
		for (p in Project.paintObjects) p.skip_context = "paint";
		paintObject = o;

		var mask = layer.objectMask;
		if (UISidebar.inst.layerFilter > 0) mask = UISidebar.inst.layerFilter;

		if (mergedObject == null || mask > 0) {
			paintObject.skip_context = "";
		}
		UVUtil.uvmapCached = false;
		UVUtil.trianglemapCached = false;
	}

	public static function mainObject(): MeshObject {
		for (po in Project.paintObjects) if (po.children.length > 0) return po;
		return Project.paintObjects[0];
	}
}
