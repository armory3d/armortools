package arm.util;

import kha.graphics4.TextureFormat;
import kha.graphics4.VertexBuffer;
import kha.graphics4.IndexBuffer;
import kha.graphics4.Usage;
import kha.graphics4.VertexStructure;
import kha.graphics4.VertexData;
import kha.Image;
import zui.Nodes;
import iron.object.MeshObject;
import iron.math.Mat4;
import iron.math.Vec4;
import iron.math.Quat;
import iron.Scene;
import iron.RenderPath;
import arm.render.RenderPathPreview;
import arm.render.RenderPathPaint;
import arm.render.RenderPathBase;
import arm.shader.MakeMaterial;

class RenderUtil {

	public static inline var materialPreviewSize = 256;
	public static inline var decalPreviewSize = 512;
	public static inline var layerPreviewSize = 200;
	static var screenAlignedFullVB: VertexBuffer = null;
	static var screenAlignedFullIB: IndexBuffer = null;

	public static function makeMaterialPreview() {
		Context.raw.materialPreview = true;

		var sphere: MeshObject = cast Scene.active.getChild(".Sphere");
		sphere.visible = true;
		var meshes = Scene.active.meshes;
		Scene.active.meshes = [sphere];
		var painto = Context.raw.paintObject;
		Context.raw.paintObject = sphere;

		sphere.materials[0] = Project.materials[0].data;
		Context.raw.material.previewReady = true;

		Context.raw.savedCamera.setFrom(Scene.active.camera.transform.local);
		var m = new Mat4(0.9146286343879498, -0.0032648027153306235, 0.404281837254303, 0.4659988049397712, 0.404295023959927, 0.007367569133732468, -0.9145989516155143, -1.0687517188018691, 0.000007410128652369705, 0.9999675337275382, 0.008058532943908717, 0.015935682577325486, 0, 0, 0, 1);
		Scene.active.camera.transform.setMatrix(m);
		var savedFov = Scene.active.camera.data.raw.fov;
		Scene.active.camera.data.raw.fov = 0.92;
		Viewport.updateCameraType(CameraPerspective);
		var light = Scene.active.lights[0];
		var _lightStrength = light.data.raw.strength;
		var probe = Scene.active.world.probe;
		var _probeStrength = probe.raw.strength;
		light.data.raw.strength = 0;
		probe.raw.strength = 7;
		var _envmapAngle = Context.raw.envmapAngle;
		Context.raw.envmapAngle = 6.0;
		var _brushScale = Context.raw.brushScale;
		Context.raw.brushScale = 1.5;
		var _brushNodesScale = Context.raw.brushNodesScale;
		Context.raw.brushNodesScale = 1.0;

		Scene.active.world.envmap = Context.raw.previewEnvmap;
		// No resize
		@:privateAccess RenderPath.active.lastW = materialPreviewSize;
		@:privateAccess RenderPath.active.lastH = materialPreviewSize;
		Scene.active.camera.buildProjection();
		Scene.active.camera.buildMatrix();

		MakeMaterial.parseMeshPreviewMaterial();
		var _commands = RenderPath.active.commands;
		RenderPath.active.commands = RenderPathPreview.commandsPreview;
		RenderPath.active.renderFrame(RenderPath.active.frameG);
		RenderPath.active.commands = _commands;

		Context.raw.materialPreview = false;
		@:privateAccess RenderPath.active.lastW = iron.App.w();
		@:privateAccess RenderPath.active.lastH = iron.App.h();

		// Restore
		sphere.visible = false;
		Scene.active.meshes = meshes;
		Context.raw.paintObject = painto;

		Scene.active.camera.transform.setMatrix(Context.raw.savedCamera);
		Viewport.updateCameraType(Context.raw.cameraType);
		Scene.active.camera.data.raw.fov = savedFov;
		Scene.active.camera.buildProjection();
		Scene.active.camera.buildMatrix();
		light.data.raw.strength = _lightStrength;
		probe.raw.strength = _probeStrength;
		Context.raw.envmapAngle = _envmapAngle;
		Context.raw.brushScale = _brushScale;
		Context.raw.brushNodesScale = _brushNodesScale;
		Scene.active.world.envmap = Context.raw.showEnvmap ? Context.raw.savedEnvmap : Context.raw.emptyEnvmap;
		MakeMaterial.parseMeshMaterial();
		Context.raw.ddirty = 0;
	}

	public static function makeDecalPreview() {
		var current = @:privateAccess kha.graphics2.Graphics.current;
		if (current != null) current.end();

		if (Context.raw.decalImage == null) {
			Context.raw.decalImage = Image.createRenderTarget(decalPreviewSize, decalPreviewSize);
		}
		Context.raw.decalPreview = true;

		var plane: MeshObject = cast Scene.active.getChild(".Plane");
		plane.transform.scale.set(1, 1, 1);
		plane.transform.rot.fromEuler(-Math.PI / 2, 0, 0);
		plane.transform.buildMatrix();
		plane.visible = true;
		var meshes = Scene.active.meshes;
		Scene.active.meshes = [plane];
		var painto = Context.raw.paintObject;
		Context.raw.paintObject = plane;

		Context.raw.savedCamera.setFrom(Scene.active.camera.transform.local);
		var m = Mat4.identity();
		m.translate(0, 0, 1);
		Scene.active.camera.transform.setMatrix(m);
		var savedFov = Scene.active.camera.data.raw.fov;
		Scene.active.camera.data.raw.fov = 0.92;
		Viewport.updateCameraType(CameraPerspective);
		var light = Scene.active.lights[0];
		light.visible = false;
		Scene.active.world.envmap = Context.raw.previewEnvmap;

		// No resize
		@:privateAccess RenderPath.active.lastW = decalPreviewSize;
		@:privateAccess RenderPath.active.lastH = decalPreviewSize;
		Scene.active.camera.buildProjection();
		Scene.active.camera.buildMatrix();

		MakeMaterial.parseMeshPreviewMaterial();
		var _commands = RenderPath.active.commands;
		RenderPath.active.commands = RenderPathPreview.commandsDecal;
		RenderPath.active.renderFrame(RenderPath.active.frameG);
		RenderPath.active.commands = _commands;

		Context.raw.decalPreview = false;
		@:privateAccess RenderPath.active.lastW = iron.App.w();
		@:privateAccess RenderPath.active.lastH = iron.App.h();

		// Restore
		plane.visible = false;
		Scene.active.meshes = meshes;
		Context.raw.paintObject = painto;

		Scene.active.camera.transform.setMatrix(Context.raw.savedCamera);
		Scene.active.camera.data.raw.fov = savedFov;
		Viewport.updateCameraType(Context.raw.cameraType);
		Scene.active.camera.buildProjection();
		Scene.active.camera.buildMatrix();
		var light = Scene.active.lights[0];
		light.visible = true;
		Scene.active.world.envmap = Context.raw.showEnvmap ? Context.raw.savedEnvmap : Context.raw.emptyEnvmap;

		MakeMaterial.parseMeshMaterial();
		Context.raw.ddirty = 1; // Refresh depth for decal paint

		if (current != null) current.begin(false);
	}

	public static function makeTextPreview() {
		var current = @:privateAccess kha.graphics2.Graphics.current;
		if (current != null) current.end();

		var text = Context.raw.textToolText;
		var font = Context.raw.font.font;
		var fontSize = 200;
		var textW = Std.int(font.width(fontSize, text));
		var textH = Std.int(font.height(fontSize));
		var texW = textW + 32;
		if (texW < 512) texW = 512;
		if (Context.raw.textToolImage != null && Context.raw.textToolImage.width < texW) {
			Context.raw.textToolImage.unload();
			Context.raw.textToolImage = null;
		}
		if (Context.raw.textToolImage == null) {
			#if kha_metal
			Context.raw.textToolImage = Image.createRenderTarget(texW, texW, TextureFormat.RGBA32);
			#else
			Context.raw.textToolImage = Image.createRenderTarget(texW, texW, TextureFormat.L8);
			#end
		}
		var g2 = Context.raw.textToolImage.g2;
		g2.begin(true, 0xff000000);
		g2.font = font;
		g2.fontSize = fontSize;
		g2.color = 0xffffffff;
		g2.drawString(text, texW / 2 - textW / 2, texW / 2 - textH / 2);
		g2.end();

		if (current != null) current.begin(false);
	}

	public static function makeFontPreview() {
		var current = @:privateAccess kha.graphics2.Graphics.current;
		if (current != null) current.end();

		var text = "Abg";
		var font = Context.raw.font.font;
		var fontSize = 318;
		var textW = Std.int(font.width(fontSize, text)) + 8;
		var textH = Std.int(font.height(fontSize)) + 8;
		if (Context.raw.font.image == null) {
			Context.raw.font.image = Image.createRenderTarget(512, 512, TextureFormat.RGBA32);
		}
		var g2 = Context.raw.font.image.g2;
		g2.begin(true, 0x00000000);
		g2.font = font;
		g2.fontSize = fontSize;
		g2.color = 0xffffffff;
		g2.drawString(text, 512 / 2 - textW / 2, 512 / 2 - textH / 2);
		g2.end();
		Context.raw.font.previewReady = true;

		if (current != null) current.begin(false);
	}

	public static function makeBrushPreview() {
		if (RenderPathPaint.liveLayerLocked) return;
		Context.raw.materialPreview = true;

		var current = @:privateAccess kha.graphics2.Graphics.current;
		if (current != null) current.end();

		// Prepare layers
		if (RenderPathPaint.liveLayer == null) {
			RenderPathPaint.liveLayer = new arm.data.LayerSlot("_live");
		}

		var l = RenderPathPaint.liveLayer;
		l.clear();

		if (Context.raw.brush.image == null) {
			Context.raw.brush.image = Image.createRenderTarget(materialPreviewSize, materialPreviewSize);
			Context.raw.brush.imageIcon = Image.createRenderTarget(50, 50);
		}

		var _material = Context.raw.material;
		Context.raw.material = new arm.data.MaterialSlot();
		var _tool = Context.raw.tool;
		Context.raw.tool = ToolBrush;

		var _layer = Context.raw.layer;
		if (Context.raw.layer.isMask()) {
			Context.raw.layer = Context.raw.layer.parent;
		}

		var _fill_layer = Context.raw.layer.fill_layer;
		Context.raw.layer.fill_layer = null;

		RenderPathPaint.useLiveLayer(true);
		MakeMaterial.parsePaintMaterial(false);

		var path = RenderPath.active;
		var hid = History.undoI - 1 < 0 ? Config.raw.undo_steps - 1 : History.undoI - 1;
		path.renderTargets.set("texpaint_undo" + hid, path.renderTargets.get("empty_black"));

		// Set plane mesh
		var painto = Context.raw.paintObject;
		var visibles: Array<Bool> = [];
		for (p in Project.paintObjects) {
			visibles.push(p.visible);
			p.visible = false;
		}
		var mergedObjectVisible = false;
		if (Context.raw.mergedObject != null) {
			mergedObjectVisible = Context.raw.mergedObject.visible;
			Context.raw.mergedObject.visible = false;
		}

		var cam = Scene.active.camera;
		Context.raw.savedCamera.setFrom(cam.transform.local);
		var savedFov = cam.data.raw.fov;
		Viewport.updateCameraType(CameraPerspective);
		var m = Mat4.identity();
		m.translate(0, 0, 0.5);
		cam.transform.setMatrix(m);
		cam.data.raw.fov = 0.92;
		cam.buildProjection();
		cam.buildMatrix();
		m.getInverse(Scene.active.camera.VP);

		var planeo: MeshObject = cast Scene.active.getChild(".Plane");
		planeo.visible = true;
		Context.raw.paintObject = planeo;

		var v = new Vec4();
		var sx = v.set(m._00, m._01, m._02).length();
		planeo.transform.rot.fromEuler(-Math.PI / 2, 0, 0);
		planeo.transform.scale.set(sx, 1.0, sx);
		planeo.transform.loc.set(m._30, -m._31, 0.0);
		planeo.transform.buildMatrix();

		RenderPathPaint.liveLayerDrawn = 0;
		RenderPathBase.drawGbuffer();

		// Paint brush preview
		var _brushRadius = Context.raw.brushRadius;
		var _brushOpacity = Context.raw.brushOpacity;
		var _brushHardness = Context.raw.brushHardness;
		Context.raw.brushRadius = 0.33;
		Context.raw.brushOpacity = 1.0;
		Context.raw.brushHardness = 0.8;
		var _x = Context.raw.paintVec.x;
		var _y = Context.raw.paintVec.y;
		var _lastX = Context.raw.lastPaintVecX;
		var _lastY = Context.raw.lastPaintVecY;
		var _pdirty = Context.raw.pdirty;
		Context.raw.pdirty = 2;

		var pointsX = [0.2, 0.2,  0.35, 0.5,  0.5, 0.5,  0.65, 0.8,  0.8, 0.8];
		var pointsY = [0.5, 0.5,  0.35 - 0.04, 0.2 - 0.08,  0.4 + 0.015, 0.6 + 0.03,  0.45 - 0.025, 0.3 - 0.05,  0.5 + 0.025, 0.7 + 0.05];
		for (i in 1...pointsX.length) {
			Context.raw.lastPaintVecX = pointsX[i - 1];
			Context.raw.lastPaintVecY = pointsY[i - 1];
			Context.raw.paintVec.x = pointsX[i];
			Context.raw.paintVec.y = pointsY[i];
			RenderPathPaint.commandsPaint(false);
		}

		Context.raw.brushRadius = _brushRadius;
		Context.raw.brushOpacity = _brushOpacity;
		Context.raw.brushHardness = _brushHardness;
		Context.raw.paintVec.x = _x;
		Context.raw.paintVec.y = _y;
		Context.raw.lastPaintVecX = _lastX;
		Context.raw.lastPaintVecY = _lastY;
		Context.raw.prevPaintVecX = -1;
		Context.raw.prevPaintVecY = -1;
		Context.raw.pdirty = _pdirty;
		RenderPathPaint.useLiveLayer(false);
		Context.raw.layer.fill_layer = _fill_layer;
		Context.raw.layer = _layer;
		Context.raw.material = _material;
		Context.raw.tool = _tool;
		function _init() {
			MakeMaterial.parsePaintMaterial(false);
		}
		iron.App.notifyOnInit(_init);

		// Restore paint mesh
		Context.raw.materialPreview = false;
		planeo.visible = false;
		for (i in 0...Project.paintObjects.length) {
			Project.paintObjects[i].visible = visibles[i];
		}
		if (Context.raw.mergedObject != null) {
			Context.raw.mergedObject.visible = mergedObjectVisible;
		}
		Context.raw.paintObject = painto;
		Scene.active.camera.transform.setMatrix(Context.raw.savedCamera);
		Scene.active.camera.data.raw.fov = savedFov;
		Viewport.updateCameraType(Context.raw.cameraType);
		Scene.active.camera.buildProjection();
		Scene.active.camera.buildMatrix();

		// Scale layer down to to image preview
		if (App.pipeMerge == null) App.makePipe();
		var l = RenderPathPaint.liveLayer;
		var target = Context.raw.brush.image;
		target.g2.begin(true, 0x00000000);
		target.g2.pipeline = App.pipeCopy;
		target.g2.drawScaledImage(l.texpaint, 0, 0, target.width, target.height);
		target.g2.pipeline = null;
		target.g2.end();

		// Scale image preview down to to icon
		path.renderTargets.get("texpreview").image = Context.raw.brush.image;
		path.renderTargets.get("texpreview_icon").image = Context.raw.brush.imageIcon;
		path.setTarget("texpreview_icon");
		path.bindTarget("texpreview", "tex");
		path.drawShader("shader_datas/supersample_resolve/supersample_resolve");

		Context.raw.brush.previewReady = true;
		Context.raw.brushBlendDirty = true;

		if (current != null) current.begin(false);
	}

	public static function makeNodePreview(canvas: TNodeCanvas, node: TNode, image: kha.Image, group: TNodeCanvas = null, parents: Array<TNode> = null) {
		var res = MakeMaterial.parseNodePreviewMaterial(node, group, parents);
		if (res == null || res.scon == null) return;

		var g4 = image.g4;
		if (screenAlignedFullVB == null) {
			createScreenAlignedFullData();
		}

		var _scaleWorld = Context.raw.paintObject.transform.scaleWorld;
		Context.raw.paintObject.transform.scaleWorld = 3.0;
		Context.raw.paintObject.transform.buildMatrix();

		g4.begin();
		g4.setPipeline(res.scon.pipeState);
		iron.object.Uniforms.setContextConstants(g4, res.scon, [""]);
		iron.object.Uniforms.setObjectConstants(g4, res.scon, Context.raw.paintObject);
		iron.object.Uniforms.setMaterialConstants(g4, res.scon, res.mcon);
		g4.setVertexBuffer(screenAlignedFullVB);
		g4.setIndexBuffer(screenAlignedFullIB);
		g4.drawIndexedVertices();
		g4.end();

		Context.raw.paintObject.transform.scaleWorld = _scaleWorld;
		Context.raw.paintObject.transform.buildMatrix();
	}

	public static function pickPosNorTex() {
		Context.raw.pickPosNorTex = true;
		Context.raw.pdirty = 1;
		var _tool = Context.raw.tool;
		Context.raw.tool = ToolPicker;
		MakeMaterial.parsePaintMaterial();
		if (Context.raw.paint2d) {
			arm.render.RenderPathPaint.setPlaneMesh();
		}
		arm.render.RenderPathPaint.commandsPaint(false);
		#if kha_metal
		// Flush command list
		arm.render.RenderPathPaint.commandsPaint(false);
		#end
		if (Context.raw.paint2d) {
			arm.render.RenderPathPaint.restorePlaneMesh();
		}
		Context.raw.tool = _tool;
		Context.raw.pickPosNorTex = false;
		MakeMaterial.parsePaintMaterial();
		Context.raw.pdirty = 0;
	}

	public static function getDecalMat(): Mat4 {
		pickPosNorTex();
		var decalMat = Mat4.identity();
		var loc = new Vec4(Context.raw.posXPicked, Context.raw.posYPicked, Context.raw.posZPicked);
		var rot = new Quat().fromTo(new Vec4(0.0, 0.0, -1.0), new Vec4(Context.raw.norXPicked, Context.raw.norYPicked, Context.raw.norZPicked));
		var scale = new Vec4(Context.raw.brushRadius * 0.5, Context.raw.brushRadius * 0.5, Context.raw.brushRadius * 0.5);
		decalMat.compose(loc, rot, scale);
		return decalMat;
	}

	static function createScreenAlignedFullData() {
		// Over-sized triangle
		var data = [-Std.int(32767 / 3), -Std.int(32767 / 3), 0, 32767, 0, 0, 0, 0, 0, 0, 0, 0,
					 32767,              -Std.int(32767 / 3), 0, 32767, 0, 0, 0, 0, 0, 0, 0, 0,
					-Std.int(32767 / 3),  32767,              0, 32767, 0, 0, 0, 0, 0, 0, 0, 0];
		var indices = [0, 1, 2];

		// Mandatory vertex data names and sizes
		var structure = new VertexStructure();
		structure.add("pos", VertexData.Short4Norm);
		structure.add("nor", VertexData.Short2Norm);
		structure.add("tex", VertexData.Short2Norm);
		structure.add("col", VertexData.Short4Norm);
		screenAlignedFullVB = new VertexBuffer(Std.int(data.length / Std.int(structure.byteSize() / 4)), structure, Usage.StaticUsage);
		var vertices = screenAlignedFullVB.lock();
		for (i in 0...Std.int(vertices.byteLength / 2)) vertices.setInt16(i * 2, data[i]);
		screenAlignedFullVB.unlock();

		screenAlignedFullIB = new IndexBuffer(indices.length, Usage.StaticUsage);
		var id = screenAlignedFullIB.lock();
		for (i in 0...id.length) id[i] = indices[i];
		screenAlignedFullIB.unlock();
	}
}
