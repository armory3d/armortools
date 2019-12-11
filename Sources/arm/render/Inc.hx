package arm.render;

import iron.math.Vec4;
import iron.math.Mat4;
import iron.math.Quat;
import iron.object.MeshObject;
import iron.system.Input;
import iron.RenderPath;
import iron.Scene;
import arm.ui.UITrait;
import arm.Tool;

class Inc {

	static var path: RenderPath;
	public static var superSample = 1.0;

	static var pointIndex = 0;
	static var spotIndex = 0;
	static var lastFrame = -1;
	static var lastX = -1.0;
	static var lastY = -1.0;

	#if rp_voxelao
	static var voxelsCreated = false;
	#end

	public static function init(_path: RenderPath) {
		path = _path;
		var config = Config.raw;
		superSample = config.rp_supersample;
	}

	public static function applyConfig() {
		var config = Config.raw;
		if (superSample != config.rp_supersample) {
			superSample = config.rp_supersample;
			for (rt in path.renderTargets) {
				if (rt.raw.width == 0 && rt.raw.scale != null) {
					rt.raw.scale = getSuperSampling();
				}
			}
			path.resize();
		}
		#if rp_voxelao
		if (!voxelsCreated) initGI();
		#end
	}

	#if rp_voxelao
	public static function initGI(tname = "voxels") {
		var config = Config.raw;
		if (config.rp_gi != true || voxelsCreated) return;
		voxelsCreated = true;

		var t = new RenderTargetRaw();
		t.name = tname;
		t.format = "R8";
		var res = getVoxelRes();
		var resZ =  getVoxelResZ();
		t.width = res;
		t.height = res;
		t.depth = Std.int(res * resZ);
		t.is_image = true;
		t.mipmaps = true;
		path.createRenderTarget(t);

		#if arm_voxelgi_temporal
		{
			var tB = new RenderTargetRaw();
			tB.name = t.name + "B";
			tB.format = t.format;
			tB.width = t.width;
			tB.height = t.height;
			tB.depth = t.depth;
			tB.is_image = t.is_image;
			tB.mipmaps = t.mipmaps;
			path.createRenderTarget(tB);
		}
		#end
	}

	public static inline function getVoxelRes(): Int {
		return 256;
	}

	public static inline function getVoxelResZ(): Float {
		return 1.0;
	}
	#end

	public static inline function getSuperSampling(): Float {
		return superSample;
	}

	#if arm_painter
	public static function drawCompass(currentG: kha.graphics4.Graphics) {
		if (UITrait.inst.showCompass) {
			var scene = Scene.active;
			var cam = Scene.active.camera;
			var gizmo: MeshObject = cast scene.getChild(".GizmoTranslate");

			var visible = gizmo.visible;
			var parent = gizmo.parent;
			var loc = gizmo.transform.loc;
			var rot = gizmo.transform.rot;
			var crot = cam.transform.rot;
			var ratio = iron.App.w() / iron.App.h();
			var P = cam.P;
			cam.P = Mat4.ortho(-8 * ratio, 8 * ratio, -8, 8, -2, 2);
			gizmo.visible = true;
			gizmo.parent = cam;
			gizmo.transform.loc = new Vec4(7.2 * ratio, -7.6, -1);
			gizmo.transform.rot = new Quat(-crot.x, -crot.y, -crot.z, crot.w);
			gizmo.transform.scale.set(0.5, 0.5, 0.5);
			gizmo.transform.buildMatrix();

			gizmo.render(currentG, "overlay", []);

			cam.P = P;
			gizmo.visible = visible;
			gizmo.parent = parent;
			gizmo.transform.loc = loc;
			gizmo.transform.rot = rot;
			gizmo.transform.buildMatrix();
		}
	}
	#end

	public static function beginSplit() {
		if (UITrait.inst.splitView) {

			if (UITrait.inst.viewIndexLast == -1 && UITrait.inst.viewIndex == -1) {
				// Begin split, draw right viewport first
				UITrait.inst.viewIndex = 1;
			}
			else {
				// Set current viewport
				UITrait.inst.viewIndex = Input.getMouse().viewX > arm.App.w() / 2 ? 1 : 0;
			}

			var cam = Scene.active.camera;
			if (UITrait.inst.viewIndexLast > -1) {
				// Save current viewport camera
				arm.plugin.Camera.inst.views[UITrait.inst.viewIndexLast].setFrom(cam.transform.local);
			}

			if (UITrait.inst.viewIndexLast != UITrait.inst.viewIndex) {
				// Redraw on current viewport change
				Context.ddirty = 1;
			}

			cam.transform.setMatrix(arm.plugin.Camera.inst.views[UITrait.inst.viewIndex]);
			cam.buildMatrix();
			cam.buildProjection();
		}
	}

	public static function endSplit() {
		UITrait.inst.viewIndexLast = UITrait.inst.viewIndex;
		UITrait.inst.viewIndex = -1;
	}

	public static inline function ssaa4(): Bool {
		return Config.raw.rp_supersample == 4;
	}

	public static function isCached(): Bool {
		#if (!arm_creator)
		if (Context.ddirty <= 0 && Context.rdirty <= 0 && (Context.pdirty <= 0 || UITrait.inst.worktab.position == SpaceScene)) {
			var mouse = Input.getMouse();
			var mx = lastX;
			var my = lastY;
			lastX = mouse.viewX;
			lastY = mouse.viewY;
			if (mx != lastX || my != lastY || mouse.locked) Context.ddirty = 0;
			if (Context.ddirty > -2) {
				path.setTarget("");
				path.bindTarget("taa", "tex");
				ssaa4() ?
					path.drawShader("shader_datas/supersample_resolve/supersample_resolve") :
					path.drawShader("shader_datas/copy_pass/copy_pass");
				if (UITrait.inst.brush3d) RenderPathPaint.commandsCursor();
				if (Context.ddirty <= 0) Context.ddirty--;
			}
			endSplit();
			RenderPathPaint.finishPaint();
			return true;
		}
		#end
		return false;
	}
}
