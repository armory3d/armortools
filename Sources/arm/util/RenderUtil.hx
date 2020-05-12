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
import arm.ui.UIHeader;
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
		Context.materialPreview = true;

		var sphere: MeshObject = cast Scene.active.getChild(".Sphere");
		sphere.visible = true;
		var meshes = Scene.active.meshes;
		Scene.active.meshes = [sphere];
		var painto = Context.paintObject;
		Context.paintObject = sphere;

		if (UIHeader.inst.worktab.position == SpaceRender) {
			sphere.materials[0] = Context.materialScene.data;
			Context.materialScene.previewReady = true;
		}
		else {
			sphere.materials[0] = Project.materials[0].data;
			Context.material.previewReady = true;
		}

		Context.savedCamera.setFrom(Scene.active.camera.transform.local);
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

		Scene.active.world.envmap = Context.previewEnvmap;
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

		Context.materialPreview = false;
		@:privateAccess RenderPath.active.lastW = iron.App.w();
		@:privateAccess RenderPath.active.lastH = iron.App.h();

		// Restore
		sphere.visible = false;
		Scene.active.meshes = meshes;
		Context.paintObject = painto;

		Scene.active.camera.transform.setMatrix(Context.savedCamera);
		ViewportUtil.updateCameraType(Context.cameraType);
		Scene.active.camera.data.raw.fov = savedFov;
		Scene.active.camera.buildProjection();
		Scene.active.camera.buildMatrix();
		light.data.raw.strength = savedLight;
		probe.raw.strength = savedProbe;
		Scene.active.world.envmap = Context.showEnvmap ? Context.savedEnvmap : Context.emptyEnvmap;
		MaterialParser.parseMeshMaterial();
		Context.ddirty = 0;
	}

	public static function makeDecalPreview() {
		var current = @:privateAccess kha.graphics4.Graphics2.current;
		if (current != null) current.end();

		if (Context.decalImage == null) {
			Context.decalImage = Image.createRenderTarget(RenderUtil.decalPreviewSize, RenderUtil.decalPreviewSize);
		}
		Context.decalPreview = true;

		var plane: MeshObject = cast Scene.active.getChild(".Plane");
		plane.transform.scale.set(1, 1, 1);
		plane.transform.rot.fromEuler(-Math.PI / 2, 0, 0);
		plane.transform.buildMatrix();
		plane.visible = true;
		var meshes = Scene.active.meshes;
		Scene.active.meshes = [plane];
		var painto = Context.paintObject;
		Context.paintObject = plane;

		Context.savedCamera.setFrom(Scene.active.camera.transform.local);
		var m = Mat4.identity();
		m.translate(0, 0, 1);
		Scene.active.camera.transform.setMatrix(m);
		var savedFov = Scene.active.camera.data.raw.fov;
		Scene.active.camera.data.raw.fov = 0.92;
		ViewportUtil.updateCameraType(CameraPerspective);
		var light = Scene.active.lights[0];
		light.visible = false;
		Scene.active.world.envmap = Context.previewEnvmap;

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

		Context.decalPreview = false;
		@:privateAccess RenderPath.active.lastW = iron.App.w();
		@:privateAccess RenderPath.active.lastH = iron.App.h();

		// Restore
		plane.visible = false;
		Scene.active.meshes = meshes;
		Context.paintObject = painto;

		Scene.active.camera.transform.setMatrix(Context.savedCamera);
		Scene.active.camera.data.raw.fov = savedFov;
		ViewportUtil.updateCameraType(Context.cameraType);
		Scene.active.camera.buildProjection();
		Scene.active.camera.buildMatrix();
		var light = Scene.active.lights[0];
		light.visible = true;
		Scene.active.world.envmap = Context.showEnvmap ? Context.savedEnvmap : Context.emptyEnvmap;

		MaterialParser.parseMeshMaterial();
		Context.ddirty = 0;

		if (current != null) current.begin(false);
	}

	public static function makeTextPreview() {
		var current = @:privateAccess kha.graphics4.Graphics2.current;
		if (current != null) current.end();

		var text = Context.textToolText;
		var font = Context.font.font;
		var fontSize = 200;
		var textW = Std.int(font.width(fontSize, text));
		var textH = Std.int(font.height(fontSize));
		var texW = textW + 32;
		if (texW < 512) texW = 512;
		if (Context.textToolImage != null && Context.textToolImage.width < texW) {
			Context.textToolImage.unload();
			Context.textToolImage = null;
		}
		if (Context.textToolImage == null) {
			#if kha_metal
			Context.textToolImage = Image.createRenderTarget(texW, texW, TextureFormat.RGBA32);
			#else
			Context.textToolImage = Image.createRenderTarget(texW, texW, TextureFormat.L8);
			#end
		}
		var g2 = Context.textToolImage.g2;
		g2.begin(true, 0xff000000);
		g2.font = font;
		g2.fontSize = fontSize;
		g2.color = 0xffffffff;
		g2.drawString(text, texW / 2 - textW / 2, texW / 2 - textH / 2);
		g2.end();

		if (current != null) current.begin(false);
	}

	public static function makeFontPreview() {
		var current = @:privateAccess kha.graphics4.Graphics2.current;
		if (current != null) current.end();

		var text = "Abg";
		var font = Context.font.font;
		var fontSize = 120;
		var textW = Std.int(font.width(fontSize, text)) + 8;
		var textH = Std.int(font.height(fontSize)) + 8;
		if (Context.font.image == null) {
			// Context.font.image = Image.createRenderTarget(200, 200, TextureFormat.L8);
			Context.font.image = Image.createRenderTarget(200, 200, TextureFormat.RGBA32);
		}
		var g2 = Context.font.image.g2;
		g2.begin(true, 0x00000000);
		g2.font = font;
		g2.fontSize = fontSize;
		g2.color = 0xffffffff;
		g2.drawString(text, 200 / 2 - textW / 2, 200 / 2 - textH / 2);
		g2.end();
		Context.font.previewReady = true;

		if (current != null) current.begin(false);
	}

	public static function makeBrushPreview() {

		if (RenderPathPaint.liveLayerLocked) return;

		var current = @:privateAccess kha.graphics4.Graphics2.current;
		if (current != null) current.end();

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
		Context.savedCamera.setFrom(cam.transform.local);
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
		var _brushRadius = Context.brushRadius;
		var _brushOpacity = Context.brushOpacity;
		var _brushHardness = Context.brushHardness;
		Context.brushRadius = 0.25;
		Context.brushOpacity = 1.0;
		Context.brushHardness = 0.8;
		var _x = Context.paintVec.x;
		var _y = Context.paintVec.y;
		var _lastX = Context.lastPaintVecX;
		var _lastY = Context.lastPaintVecY;
		var _pdirty = Context.pdirty;
		Context.pdirty = 2;

		// var pointsX = [0.2, 0.5, 0.5, 0.8, 0.8];
		// var pointsY = [0.5, 0.2, 0.6, 0.3, 0.7];
		var pointsX = [0.2, 0.2,  0.35, 0.5,  0.5, 0.5,  0.65, 0.8,  0.8, 0.8];
		var pointsY = [0.5, 0.5,  0.35, 0.2,  0.4, 0.6,  0.45, 0.3,  0.5, 0.7];
		for (i in 1...pointsX.length) {
			Context.lastPaintVecX = pointsX[i - 1];
			Context.lastPaintVecY = pointsY[i - 1];
			Context.paintVec.x = pointsX[i];
			Context.paintVec.y = pointsY[i];
			Context.parseBrushInputs();
			RenderPathPaint.commandsPaint();
		}

		Context.brushRadius = _brushRadius;
		Context.brushOpacity = _brushOpacity;
		Context.brushHardness = _brushHardness;
		Context.paintVec.x = _x;
		Context.paintVec.y = _y;
		Context.lastPaintVecX = _lastX;
		Context.lastPaintVecY = _lastY;
		Context.prevPaintVecX = -1;
		Context.prevPaintVecY = -1;
		Context.pdirty = _pdirty;
		// scons[_si] = _scon;
		// mcons[_mi] = _mcon;
		Context.material = _material;
		function _parse(_) {
			MaterialParser.parsePaintMaterial();
			iron.App.removeRender(_parse);
		}
		iron.App.notifyOnRender(_parse);

		// Restore paint mesh
		planeo.visible = false;
		for (i in 0...Project.paintObjects.length) {
			Project.paintObjects[i].visible = visibles[i];
		}
		if (Context.mergedObject != null) {
			Context.mergedObject.visible = mergedObjectVisible;
		}
		Context.paintObject = painto;
		Scene.active.camera.transform.setMatrix(Context.savedCamera);
		Scene.active.camera.data.raw.fov = savedFov;
		ViewportUtil.updateCameraType(Context.cameraType);
		Scene.active.camera.buildProjection();
		Scene.active.camera.buildMatrix();

		RenderPathPaint.useLiveLayer(false);

		// Scale layer down to to image preview
		if (Layers.pipeMerge == null) Layers.makePipe();
		var l = RenderPathPaint.liveLayer;
		var target = Context.brush.image;
		target.g2.begin(true, 0x00000000);
		target.g2.pipeline = Layers.pipeCopy;
		target.g2.drawScaledImage(l.texpaint, 0, 0, target.width, target.height);
		target.g2.pipeline = null;
		target.g2.end();

		// Scale image preview down to to icon
		path.renderTargets.get("texpreview").image = Context.brush.image;
		path.renderTargets.get("texpreview_icon").image = Context.brush.imageIcon;
		path.setTarget("texpreview_icon");
		path.bindTarget("texpreview", "tex");
		path.drawShader("shader_datas/supersample_resolve/supersample_resolve");

		Context.brush.previewReady = true;
		Context.brushBlendDirty = true;

		if (current != null) current.begin(false);
	}
}
