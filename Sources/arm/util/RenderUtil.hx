package arm.util;

import kha.Image;
import kha.Font;
import kha.graphics4.TextureFormat;
import iron.Scene;
import iron.RenderPath;
import iron.object.MeshObject;
import iron.math.Mat4;
import arm.ui.UITrait;
import arm.render.RenderPathPreview;
import arm.node.MaterialParser;
import arm.io.ImportFont;
import arm.Tool;

class RenderUtil {

	public static inline var matPreviewSize = 200;
	public static inline var decalPreviewSize = 512;

	public static function makeMaterialPreview() {
		UITrait.inst.materialPreview = true;

		var sphere: MeshObject = cast Scene.active.getChild(".Sphere");
		sphere.visible = true;
		var meshes = Scene.active.meshes;
		Scene.active.meshes = [sphere];
		var painto = Context.paintObject;
		Context.paintObject = sphere;

		if (UITrait.inst.worktab.position == SpaceScene) {
			sphere.materials[0] = Context.materialScene.data;
			Context.materialScene.previewReady = true;
		}
		else {
			sphere.materials[0] = Project.materials[0].data;
			Context.material.previewReady = true;
		}

		UITrait.inst.savedCamera.setFrom(Scene.active.camera.transform.local);
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

		Scene.active.world.envmap = UITrait.inst.previewEnvmap;
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

		UITrait.inst.materialPreview = false;
		@:privateAccess RenderPath.active.lastW = iron.App.w();
		@:privateAccess RenderPath.active.lastH = iron.App.h();

		// Restore
		sphere.visible = false;
		Scene.active.meshes = meshes;
		Context.paintObject = painto;

		Scene.active.camera.transform.setMatrix(UITrait.inst.savedCamera);
		ViewportUtil.updateCameraType(UITrait.inst.cameraType);
		Scene.active.camera.data.raw.fov = savedFov;
		Scene.active.camera.buildProjection();
		Scene.active.camera.buildMatrix();
		light.data.raw.strength = savedLight;
		probe.raw.strength = savedProbe;
		Scene.active.world.envmap = UITrait.inst.showEnvmap ? UITrait.inst.savedEnvmap : UITrait.inst.emptyEnvmap;
		MaterialParser.parseMeshMaterial();
		Context.ddirty = 0;
	}

	public static function makeDecalPreview() {
		if (UITrait.inst.decalImage == null) {
			UITrait.inst.decalImage = Image.createRenderTarget(RenderUtil.decalPreviewSize, RenderUtil.decalPreviewSize);
		}
		UITrait.inst.decalPreview = true;

		var plane: MeshObject = cast Scene.active.getChild(".Plane");
		plane.transform.scale.set(1, 1, 1);
		plane.transform.rot.fromEuler(-Math.PI / 2, 0, 0);
		plane.transform.buildMatrix();
		plane.visible = true;
		var meshes = Scene.active.meshes;
		Scene.active.meshes = [plane];
		var painto = Context.paintObject;
		Context.paintObject = plane;

		UITrait.inst.savedCamera.setFrom(Scene.active.camera.transform.local);
		var m = Mat4.identity();
		m.translate(0, 0, 1);
		Scene.active.camera.transform.setMatrix(m);
		var savedFov = Scene.active.camera.data.raw.fov;
		Scene.active.camera.data.raw.fov = 0.92;
		ViewportUtil.updateCameraType(CameraPerspective);
		var light = Scene.active.lights[0];
		light.visible = false;
		Scene.active.world.envmap = UITrait.inst.previewEnvmap;

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

		UITrait.inst.decalPreview = false;
		@:privateAccess RenderPath.active.lastW = iron.App.w();
		@:privateAccess RenderPath.active.lastH = iron.App.h();

		// Restore
		plane.visible = false;
		Scene.active.meshes = meshes;
		Context.paintObject = painto;

		Scene.active.camera.transform.setMatrix(UITrait.inst.savedCamera);
		Scene.active.camera.data.raw.fov = savedFov;
		ViewportUtil.updateCameraType(UITrait.inst.cameraType);
		Scene.active.camera.buildProjection();
		Scene.active.camera.buildMatrix();
		var light = Scene.active.lights[0];
		light.visible = true;
		Scene.active.world.envmap = UITrait.inst.showEnvmap ? UITrait.inst.savedEnvmap : UITrait.inst.emptyEnvmap;

		MaterialParser.parseMeshMaterial();
		Context.ddirty = 0;
	}

	public static function makeTextPreview() {
		var text = UITrait.inst.textToolText;
		var font = getTextToolFont();
		var fontSize = 200;
		var textW = Std.int(font.width(fontSize, text));
		var textH = Std.int(font.height(fontSize));
		var texW = textW + 32;
		if (texW < 512) texW = 512;
		if (UITrait.inst.textToolImage != null && UITrait.inst.textToolImage.width < texW) {
			UITrait.inst.textToolImage.unload();
			UITrait.inst.textToolImage = null;
		}
		if (UITrait.inst.textToolImage == null) {
			UITrait.inst.textToolImage = Image.createRenderTarget(texW, texW, TextureFormat.L8);
		}
		var g2 = UITrait.inst.textToolImage.g2;
		g2.begin(true, 0xff000000);
		g2.font = font;
		g2.fontSize = fontSize;
		g2.color = 0xffffffff;
		g2.drawString(text, texW / 2 - textW / 2, texW / 2 - textH / 2);
		g2.end();
	}

	static function getTextToolFont(): Font {
		var fontName = ImportFont.fontList[UITrait.inst.textToolHandle.position];
		if (fontName == "default.ttf") return UITrait.inst.ui.ops.font;
		return ImportFont.fontMap.get(fontName);
	}

	public static function makeDecalMaskPreview() {
		var texw = 1024;
		var off = 12;
		if (UITrait.inst.decalMaskImage == null) {
			UITrait.inst.decalMaskImage = Image.createRenderTarget(texw, texw, TextureFormat.L8);
		}
		var g2 = UITrait.inst.decalMaskImage.g2;
		g2.begin(true, 0xff000000);
		g2.color = 0xffffffff;
		var mask = UITrait.inst.decalMaskHandle.position;
		if (mask == 0) {
			g2.fillRect(off, off, texw - off, texw - off);
		}
		else if (mask == 1) {
			kha.graphics2.GraphicsExtension.fillCircle(g2, texw / 2, texw / 2, texw / 2 - off);
		}
		else if (mask == 2) {
			g2.fillTriangle(texw / 2, off, off, texw - off, texw - off, texw - off);
		}
		g2.end();
	}
}
