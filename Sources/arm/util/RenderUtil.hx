package arm.util;

import iron.object.MeshObject;
import iron.math.Mat4;
import arm.ui.*;

class RenderUtil {
	public static function makeDecalPreview() {
		if (UITrait.inst.decalImage == null) {
			UITrait.inst.decalImage = kha.Image.createRenderTarget(512, 512);
		}
		UITrait.inst.decalPreview = true;

		var painto = UITrait.inst.paintObject;
		for (p in UITrait.inst.paintObjects) p.visible = false;

		var plane:MeshObject = cast iron.Scene.active.getChild(".Plane");
		plane.transform.rot.fromEuler(-Math.PI / 2, 0, 0);
		plane.transform.buildMatrix();
		plane.visible = true;
		UITrait.inst.paintObject = plane;
		
		UITrait.inst.savedCamera.setFrom(iron.Scene.active.camera.transform.local);
		var m = Mat4.identity();
		m.translate(0, 0, 1);
		iron.Scene.active.camera.transform.setMatrix(m);
		var savedFov = iron.Scene.active.camera.data.raw.fov;
		iron.Scene.active.camera.data.raw.fov = 0.92;
		var light = iron.Scene.active.lights[0];
		light.visible = false;
		iron.Scene.active.world.envmap = UITrait.inst.previewEnvmap;

		// No jitter
		// @:privateAccess iron.Scene.active.camera.frame = 0;
		// No resize
		@:privateAccess iron.RenderPath.active.lastW = 512;
		@:privateAccess iron.RenderPath.active.lastH = 512;
		iron.Scene.active.camera.buildProjection();
		iron.Scene.active.camera.buildMatrix();

		UINodes.inst.parseMeshPreviewMaterial();
		iron.RenderPath.active.commands = arm.renderpath.RenderPathPreview.commandsDecal;
		iron.RenderPath.active.renderFrame(iron.RenderPath.active.frameG);
		iron.RenderPath.active.commands = arm.renderpath.RenderPathDeferred.commands;

		UITrait.inst.decalPreview = false;
		@:privateAccess iron.RenderPath.active.lastW = iron.App.w();
		@:privateAccess iron.RenderPath.active.lastH = iron.App.h();

		// Restore
		plane.visible = false;
		for (p in UITrait.inst.paintObjects) p.visible = true;
		UITrait.inst.paintObject = painto;

		iron.Scene.active.camera.transform.setMatrix(UITrait.inst.savedCamera);
		iron.Scene.active.camera.data.raw.fov = savedFov;
		iron.Scene.active.camera.buildProjection();
		iron.Scene.active.camera.buildMatrix();
		var light = iron.Scene.active.lights[0];
		light.visible = true;
		// light.data.raw.cast_shadow = true;
		iron.Scene.active.world.envmap = UITrait.inst.showEnvmap ? UITrait.inst.savedEnvmap : UITrait.inst.emptyEnvmap;
		
		UINodes.inst.parseMeshMaterial();
		UITrait.inst.ddirty = 2;
	}

	public static function makeMaterialPreview() {
		UITrait.inst.materialPreview = true;

		var painto = UITrait.inst.paintObject;
		for (p in UITrait.inst.paintObjects) p.visible = false;

		var sphere:MeshObject = cast iron.Scene.active.getChild(".Sphere");
		sphere.visible = true;
		UITrait.inst.paintObject = sphere;

		//
		sphere.materials[0] = UITrait.inst.worktab.position == 1 ? UITrait.inst.selectedMaterial2.data : UITrait.inst.materials[0].data;
		var gizmo_vis = UITrait.inst.gizmo.visible;
		// var grid_vis = UITrait.inst.grid.visible;
		UITrait.inst.gizmo.visible = false;
		// UITrait.inst.grid.visible = false;
		//
		
		UITrait.inst.savedCamera.setFrom(iron.Scene.active.camera.transform.local);
		var m = new Mat4(0.9146286343879498, -0.0032648027153306235, 0.404281837254303, 0.4659988049397712, 0.404295023959927, 0.007367569133732468, -0.9145989516155143, -1.0687517188018691, 0.000007410128652369705, 0.9999675337275382, 0.008058532943908717, 0.015935682577325486, 0, 0, 0, 1);
		iron.Scene.active.camera.transform.setMatrix(m);
		var savedFov = iron.Scene.active.camera.data.raw.fov;
		iron.Scene.active.camera.data.raw.fov = 0.92;
		var light = iron.Scene.active.lights[0];
		var savedLight = light.data.raw.strength;
		light.data.raw.strength = 1000;
		var probe = iron.Scene.active.world.probe;
		var savedProbe = probe.raw.strength;
		probe.raw.strength = 2;
		iron.Scene.active.world.envmap = UITrait.inst.previewEnvmap;
		// No jitter
		// @:privateAccess iron.Scene.active.camera.frame = 0;
		// No resize
		@:privateAccess iron.RenderPath.active.lastW = 200;
		@:privateAccess iron.RenderPath.active.lastH = 200;
		iron.Scene.active.camera.buildProjection();
		iron.Scene.active.camera.buildMatrix();

		UINodes.inst.parseMeshPreviewMaterial();
		iron.RenderPath.active.commands = arm.renderpath.RenderPathPreview.commandsPreview;
		iron.RenderPath.active.renderFrame(iron.RenderPath.active.frameG);
		iron.RenderPath.active.commands = arm.renderpath.RenderPathDeferred.commands;

		UITrait.inst.materialPreview = false;
		@:privateAccess iron.RenderPath.active.lastW = iron.App.w();
		@:privateAccess iron.RenderPath.active.lastH = iron.App.h();

		// Restore
		sphere.visible = false;
		for (p in UITrait.inst.paintObjects) p.visible = true;
		UITrait.inst.paintObject = painto;

		UITrait.inst.gizmo.visible = gizmo_vis;
		// UITrait.inst.grid.visible = grid_vis;

		iron.Scene.active.camera.transform.setMatrix(UITrait.inst.savedCamera);
		iron.Scene.active.camera.data.raw.fov = savedFov;
		iron.Scene.active.camera.buildProjection();
		iron.Scene.active.camera.buildMatrix();
		light.data.raw.strength = savedLight;
		probe.raw.strength = savedProbe;
		iron.Scene.active.world.envmap = UITrait.inst.showEnvmap ? UITrait.inst.savedEnvmap : UITrait.inst.emptyEnvmap;
		UINodes.inst.parseMeshMaterial();
	}

	public static function makeTextPreview() {
		var text = UITrait.inst.textToolText;
		var font = UITrait.inst.getTextToolFont();
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
			UITrait.inst.textToolImage = kha.Image.createRenderTarget(texW, texW, kha.graphics4.TextureFormat.L8);
		}
		var g2 = UITrait.inst.textToolImage.g2;
		g2.begin(true, 0xff000000);
		g2.font = font;
		g2.fontSize = fontSize;
		g2.color = 0xffffffff;
		g2.drawString(text, texW / 2 - textW / 2, texW / 2 - textH / 2);
		g2.end();
	}

	public static function makeDecalMaskPreview() {
		var texw = 1024;
		var off = 12;
		if (UITrait.inst.decalMaskImage == null) {
			UITrait.inst.decalMaskImage = kha.Image.createRenderTarget(texw, texw, kha.graphics4.TextureFormat.L8);
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
