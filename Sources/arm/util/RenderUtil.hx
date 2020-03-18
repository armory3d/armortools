package arm.util;

import kha.Image;
import kha.Font;
import kha.graphics4.TextureFormat;
import iron.Scene;
import iron.RenderPath;
import iron.object.MeshObject;
import iron.math.Mat4;
import iron.math.Vec4;
import iron.data.MaterialData;
import iron.data.ShaderData;
import arm.ui.UISidebar;
import arm.render.RenderPathPreview;
import arm.render.RenderPathPaint;
import arm.render.RenderPathDeferred;
import arm.node.MaterialParser;
import arm.io.ImportFont;
import arm.Enums;

class RenderUtil {

	public static inline var matPreviewSize = 256;
	public static inline var decalPreviewSize = 512;

	public static function makeMaterialPreview() {
		UISidebar.inst.materialPreview = true;

		var sphere: MeshObject = cast Scene.active.getChild(".Sphere");
		sphere.visible = true;
		var meshes = Scene.active.meshes;
		Scene.active.meshes = [sphere];
		var painto = Context.paintObject;
		Context.paintObject = sphere;

		if (UISidebar.inst.worktab.position == SpaceScene) {
			sphere.materials[0] = Context.materialScene.data;
			Context.materialScene.previewReady = true;
		}
		else {
			sphere.materials[0] = Project.materials[0].data;
			Context.material.previewReady = true;
		}

		UISidebar.inst.savedCamera.setFrom(Scene.active.camera.transform.local);
		var m = new Mat4(0.9146286343879498, -0.0032648027153306235, 0.404281837254303, 0.4659988049397712, 0.404295023959927, 0.007367569133732468, -0.9145989516155143, -1.0687517188018691, 0.000007410128652369705, 0.9999675337275382, 0.008058532943908717, 0.015935682577325486, 0, 0, 0, 1);
		Scene.active.camera.transform.setMatrix(m);
		var savedFov = Scene.active.camera.data.raw.fov;
		Scene.active.camera.data.raw.fov = 0.92;
		ViewportUtil.updateCameraType(CameraPerspective);
		var light = Scene.active.lights[0];
		var savedLight = light.data.raw.strength;
		var probe = Scene.active.world.probe;
		var savedProbe = probe.raw.strength;
		#if arm_world
		light.data.raw.strength = 1;
		probe.raw.strength = 1;
		#else
		light.data.raw.strength = 1500;
		probe.raw.strength = 4;
		#end

		Scene.active.world.envmap = UISidebar.inst.previewEnvmap;
		// No resize
		@:privateAccess RenderPath.active.lastW = matPreviewSize;
		@:privateAccess RenderPath.active.lastH = matPreviewSize;
		Scene.active.camera.buildProjection();
		Scene.active.camera.buildMatrix();

		MaterialParser.parseMeshPreviewMaterial();
		var _commands = RenderPath.active.commands;
		RenderPath.active.commands = RenderPathPreview.commandsPreview;
		RenderPath.active.renderFrame(RenderPath.active.frameG);
		RenderPath.active.commands = _commands;

		UISidebar.inst.materialPreview = false;
		@:privateAccess RenderPath.active.lastW = iron.App.w();
		@:privateAccess RenderPath.active.lastH = iron.App.h();

		// Restore
		sphere.visible = false;
		Scene.active.meshes = meshes;
		Context.paintObject = painto;

		Scene.active.camera.transform.setMatrix(UISidebar.inst.savedCamera);
		ViewportUtil.updateCameraType(UISidebar.inst.cameraType);
		Scene.active.camera.data.raw.fov = savedFov;
		Scene.active.camera.buildProjection();
		Scene.active.camera.buildMatrix();
		light.data.raw.strength = savedLight;
		probe.raw.strength = savedProbe;
		Scene.active.world.envmap = UISidebar.inst.showEnvmap ? UISidebar.inst.savedEnvmap : UISidebar.inst.emptyEnvmap;
		MaterialParser.parseMeshMaterial();
		Context.ddirty = 0;
	}

	public static function makeDecalPreview() {
		if (UISidebar.inst.decalImage == null) {
			UISidebar.inst.decalImage = Image.createRenderTarget(RenderUtil.decalPreviewSize, RenderUtil.decalPreviewSize);
		}
		UISidebar.inst.decalPreview = true;

		var plane: MeshObject = cast Scene.active.getChild(".Plane");
		plane.transform.scale.set(1, 1, 1);
		plane.transform.rot.fromEuler(-Math.PI / 2, 0, 0);
		plane.transform.buildMatrix();
		plane.visible = true;
		var meshes = Scene.active.meshes;
		Scene.active.meshes = [plane];
		var painto = Context.paintObject;
		Context.paintObject = plane;

		UISidebar.inst.savedCamera.setFrom(Scene.active.camera.transform.local);
		var m = Mat4.identity();
		m.translate(0, 0, 1);
		Scene.active.camera.transform.setMatrix(m);
		var savedFov = Scene.active.camera.data.raw.fov;
		Scene.active.camera.data.raw.fov = 0.92;
		ViewportUtil.updateCameraType(CameraPerspective);
		var light = Scene.active.lights[0];
		light.visible = false;
		Scene.active.world.envmap = UISidebar.inst.previewEnvmap;

		// No resize
		@:privateAccess RenderPath.active.lastW = RenderUtil.decalPreviewSize;
		@:privateAccess RenderPath.active.lastH = RenderUtil.decalPreviewSize;
		Scene.active.camera.buildProjection();
		Scene.active.camera.buildMatrix();

		MaterialParser.parseMeshPreviewMaterial();
		var _commands = RenderPath.active.commands;
		RenderPath.active.commands = RenderPathPreview.commandsDecal;
		RenderPath.active.renderFrame(RenderPath.active.frameG);
		RenderPath.active.commands = _commands;

		UISidebar.inst.decalPreview = false;
		@:privateAccess RenderPath.active.lastW = iron.App.w();
		@:privateAccess RenderPath.active.lastH = iron.App.h();

		// Restore
		plane.visible = false;
		Scene.active.meshes = meshes;
		Context.paintObject = painto;

		Scene.active.camera.transform.setMatrix(UISidebar.inst.savedCamera);
		Scene.active.camera.data.raw.fov = savedFov;
		ViewportUtil.updateCameraType(UISidebar.inst.cameraType);
		Scene.active.camera.buildProjection();
		Scene.active.camera.buildMatrix();
		var light = Scene.active.lights[0];
		light.visible = true;
		Scene.active.world.envmap = UISidebar.inst.showEnvmap ? UISidebar.inst.savedEnvmap : UISidebar.inst.emptyEnvmap;

		MaterialParser.parseMeshMaterial();
		Context.ddirty = 0;
	}

	public static function makeTextPreview() {
		var text = UISidebar.inst.textToolText;
		var font = getTextToolFont();
		var fontSize = 200;
		var textW = Std.int(font.width(fontSize, text));
		var textH = Std.int(font.height(fontSize));
		var texW = textW + 32;
		if (texW < 512) texW = 512;
		if (UISidebar.inst.textToolImage != null && UISidebar.inst.textToolImage.width < texW) {
			UISidebar.inst.textToolImage.unload();
			UISidebar.inst.textToolImage = null;
		}
		if (UISidebar.inst.textToolImage == null) {
			UISidebar.inst.textToolImage = Image.createRenderTarget(texW, texW, TextureFormat.L8);
		}
		var g2 = UISidebar.inst.textToolImage.g2;
		g2.begin(true, 0xff000000);
		g2.font = font;
		g2.fontSize = fontSize;
		g2.color = 0xffffffff;
		g2.drawString(text, texW / 2 - textW / 2, texW / 2 - textH / 2);
		g2.end();
	}

	static function getTextToolFont(): Font {
		var fontName = ImportFont.fontList[UISidebar.inst.textToolHandle.position];
		if (fontName == "default.ttf") return UISidebar.inst.ui.ops.font;
		return ImportFont.fontMap.get(fontName);
	}

	public static function makeBrushPreview() {

		if (RenderPathPaint.liveLayerLocked) return;

		// Prepare layers
		if (RenderPathPaint.liveLayer == null) {
			RenderPathPaint.liveLayer = new arm.data.LayerSlot("_live");
			RenderPathPaint.liveLayer.createMask(0x00000000);
		}

		var l = RenderPathPaint.liveLayer;
		l.texpaint.g4.begin();
		l.texpaint.g4.clear(kha.Color.fromFloats(0.0, 0.0, 0.0, 0.0)); // Base
		l.texpaint.g4.end();

		l.texpaint_nor.g4.begin();
		l.texpaint_nor.g4.clear(kha.Color.fromFloats(0.5, 0.5, 1.0, 0.0)); // Nor
		l.texpaint_nor.g4.end();

		l.texpaint_pack.g4.begin();
		l.texpaint_pack.g4.clear(kha.Color.fromFloats(1.0, 0.0, 0.0, 0.0)); // Occ, rough, met
		l.texpaint_pack.g4.end();

		if (Context.brush.image == null) {
			Context.brush.image = Image.createRenderTarget(matPreviewSize, matPreviewSize);
			Context.brush.imageIcon = Image.createRenderTarget(50, 50);
		}

		// var scons = Project.materials[0].data.shader.contexts;
		// var mcons = Project.materials[0].data.contexts;
		// var _scon: ShaderContext = null;
		// var _mcon: MaterialContext = null;
		// var _si = 0;
		// var _mi = 0;
		// for (i in 0...scons.length) {
		// 	if (scons[i].raw.name == "paint") {
		// 		_si = i;
		// 		_scon = scons[i];
		// 		scons[i] = MaterialParser.defaultScon;
		// 		break;
		// 	}
		// }
		// for (i in 0...mcons.length) {
		// 	if (mcons[i].raw.name == "paint") {
		// 		_mi = i;
		// 		_mcon = mcons[i];
		// 		mcons[i] = MaterialParser.defaultMcon;
		// 		break;
		// 	}
		// }
		var _material = Context.material;
		Context.material = new arm.data.MaterialSlot();
		MaterialParser.parsePaintMaterial();

		RenderPathPaint.useLiveLayer(true);

		var path = RenderPath.active;
		var hid = History.undoI - 1 < 0 ? Config.raw.undo_steps - 1 : History.undoI - 1;
		path.renderTargets.set("texpaint_undo" + hid, path.renderTargets.get("empty_black"));

		// Set plane mesh
		var painto = Context.paintObject;
		var visibles: Array<Bool> = [];
		for (p in Project.paintObjects) {
			visibles.push(p.visible);
			p.visible = false;
		}
		var mergedObjectVisible = false;
		if (Context.mergedObject != null) {
			mergedObjectVisible = Context.mergedObject.visible;
			Context.mergedObject.visible = false;
		}

		var cam = Scene.active.camera;
		UISidebar.inst.savedCamera.setFrom(cam.transform.local);
		var savedFov = cam.data.raw.fov;
		ViewportUtil.updateCameraType(CameraPerspective);
		var m = Mat4.identity();
		m.translate(0, 0, 0.5);
		cam.transform.setMatrix(m);
		cam.data.raw.fov = 0.92;
		cam.buildProjection();
		cam.buildMatrix();
		m.getInverse(Scene.active.camera.VP);

		var planeo: MeshObject = cast Scene.active.getChild(".Plane");
		planeo.visible = true;
		Context.paintObject = planeo;

		var v = new Vec4();
		var sx = v.set(m._00, m._01, m._02).length();
		planeo.transform.rot.fromEuler(-Math.PI / 2, 0, 0);
		planeo.transform.scale.set(sx, 1.0, sx);
		planeo.transform.loc.set(m._30, -m._31, 0.0);
		planeo.transform.buildMatrix();

		RenderPathPaint.liveLayerDrawn = 0;
		RenderPathDeferred.drawGbuffer();

		// Paint brush preview
		var _brushRadius = UISidebar.inst.brushRadius;
		var _brushOpacity = UISidebar.inst.brushOpacity;
		var _brushHardness = UISidebar.inst.brushHardness;
		UISidebar.inst.brushRadius = 0.25;
		UISidebar.inst.brushOpacity = 1.0;
		UISidebar.inst.brushHardness = 0.8;
		var _x = UISidebar.inst.paintVec.x;
		var _y = UISidebar.inst.paintVec.y;
		var _lastX = UISidebar.inst.lastPaintVecX;
		var _lastY = UISidebar.inst.lastPaintVecY;
		var _pdirty = Context.pdirty;
		Context.pdirty = 2;

		// var pointsX = [0.2, 0.5, 0.5, 0.8, 0.8];
		// var pointsY = [0.5, 0.2, 0.6, 0.3, 0.7];
		var pointsX = [0.2, 0.2,  0.35, 0.5,  0.5, 0.5,  0.65, 0.8,  0.8, 0.8];
		var pointsY = [0.5, 0.5,  0.35, 0.2,  0.4, 0.6,  0.45, 0.3,  0.5, 0.7];
		for (i in 1...pointsX.length) {
			UISidebar.inst.lastPaintVecX = pointsX[i - 1];
			UISidebar.inst.lastPaintVecY = pointsY[i - 1];
			UISidebar.inst.paintVec.x = pointsX[i];
			UISidebar.inst.paintVec.y = pointsY[i];
			RenderPathPaint.commandsPaint();
		}

		UISidebar.inst.brushRadius = _brushRadius;
		UISidebar.inst.brushOpacity = _brushOpacity;
		UISidebar.inst.brushHardness = _brushHardness;
		UISidebar.inst.paintVec.x = _x;
		UISidebar.inst.paintVec.y = _y;
		UISidebar.inst.lastPaintVecX = _lastX;
		UISidebar.inst.lastPaintVecY = _lastY;
		Context.pdirty = _pdirty;
		// scons[_si] = _scon;
		// mcons[_mi] = _mcon;
		Context.material = _material;
		MaterialParser.parsePaintMaterial();

		// Restore paint mesh
		planeo.visible = false;
		for (i in 0...Project.paintObjects.length) {
			Project.paintObjects[i].visible = visibles[i];
		}
		if (Context.mergedObject != null) {
			Context.mergedObject.visible = mergedObjectVisible;
		}
		Context.paintObject = painto;
		Scene.active.camera.transform.setMatrix(UISidebar.inst.savedCamera);
		Scene.active.camera.data.raw.fov = savedFov;
		ViewportUtil.updateCameraType(UISidebar.inst.cameraType);
		Scene.active.camera.buildProjection();
		Scene.active.camera.buildMatrix();

		RenderPathPaint.useLiveLayer(false);

		// Scale layer down to to image preview
		var l = RenderPathPaint.liveLayer;
		var target = Context.brush.image;
		target.g2.begin(true, 0x00000000);
		target.g2.drawScaledImage(l.texpaint, 0, 0, target.width, target.height);
		target.g2.end();

		// Scale image preview down to to icon
		path.renderTargets.get("texpreview").image = Context.brush.image;
		path.renderTargets.get("texpreview_icon").image = Context.brush.imageIcon;
		path.setTarget("texpreview_icon");
		path.bindTarget("texpreview", "tex");
		path.drawShader("shader_datas/supersample_resolve/supersample_resolve");

		Context.brush.previewReady = true;
		Context.brushBlendDirty = true;
	}
}
