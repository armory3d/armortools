package arm.render;

#if arm_vr

import kha.System;
import iron.RenderPath;
import iron.Scene;
import arm.ui.UISidebar;

class RenderPathForwardVR {

	public static var path: RenderPath;
	static var m = iron.math.Mat4.identity();
	static var m0 = iron.math.Mat4.identity();
	static var q = new iron.math.Quat();

	public static function init(_path: RenderPath) {
		path = _path;

		{
			var t = new RenderTargetRaw();
			t.name = "eye0";
			t.width = 1440;
			t.height = 1600;
			t.format = "RGBA32";
			t.scale = Inc.getSuperSampling();
			path.createRenderTarget(t);
		}
		{
			var t = new RenderTargetRaw();
			t.name = "eye1";
			t.width = 1440;
			t.height = 1600;
			t.format = "RGBA32";
			t.scale = Inc.getSuperSampling();
			path.createRenderTarget(t);
		}

		path.createDepthBuffer("main_eye", "DEPTH24");

		{
			var t = new RenderTargetRaw();
			t.name = "gbuffer0_eye";
			t.width = 1440;
			t.height = 1600;
			t.format = "RGBA64";
			t.scale = Inc.getSuperSampling();
			t.depth_buffer = "main_eye";
			path.createRenderTarget(t);
		}
		{
			var t = new RenderTargetRaw();
			t.name = "gbuffer1_eye";
			t.width = 1440;
			t.height = 1600;
			t.format = "RGBA64";
			t.scale = Inc.getSuperSampling();
			path.createRenderTarget(t);
		}
		{
			var t = new RenderTargetRaw();
			t.name = "gbuffer2_eye";
			t.width = 1440;
			t.height = 1600;
			t.format = "RGBA64";
			t.scale = Inc.getSuperSampling();
			path.createRenderTarget(t);
		}
		{
			var t = new RenderTargetRaw();
			t.name = "buf_eye";
			t.width = 1440;
			t.height = 1600;
			#if (kha_direct3d12 || kha_vulkan)
			t.format = "RGBA64"; // Match raytrace_target format
			#else
			t.format = "RGBA32";
			#end
			t.scale = Inc.getSuperSampling();
			path.createRenderTarget(t);
		}
		{
			var t = new RenderTargetRaw();
			t.name = "bufa_eye";
			t.width = 1440;
			t.height = 1600;
			t.format = "RGBA32";
			t.scale = Inc.getSuperSampling();
			path.createRenderTarget(t);
		}
		{
			var t = new RenderTargetRaw();
			t.name = "taa_eye";
			t.width = 1440;
			t.height = 1600;
			t.format = "RGBA32";
			t.scale = Inc.getSuperSampling();
			path.createRenderTarget(t);
		}
		{
			var t = new RenderTargetRaw();
			t.name = "taa2_eye";
			t.width = 1440;
			t.height = 1600;
			t.format = "RGBA32";
			t.scale = Inc.getSuperSampling();
			path.createRenderTarget(t);
		}
	}

	@:access(iron.RenderPath)
	public static function commands() {

		if (System.windowWidth() == 0 || System.windowHeight() == 0) return;

		if (Krom.vrGetSensorStateHmdMounted()) {
			var current = RenderPathDeferred.taaFrame % 2 == 0 ? "bufa" : "taa2";
			var output = RenderPathDeferred.taaFrame % 2 == 0 ? current : "taa";

			m.setFrom(Scene.active.camera.transform.world);

			for (eye in 0...2) {
				var view = Krom.vrGetSensorStateView(eye);
				var proj = Krom.vrGetSensorStateProjection(eye);
				// Convert up axis
				q.fromAxisAngle(new iron.math.Vec4(1, 0, 0), -Math.PI / 2);
				Scene.active.camera.V.fromQuat(q);
				Scene.active.camera.V.translate(0, 1, -1);
				m0._00 = view._00;
				m0._01 = view._01;
				m0._02 = view._02;
				m0._03 = view._03;
				m0._10 = view._10;
				m0._11 = view._11;
				m0._12 = view._12;
				m0._13 = view._13;
				m0._20 = view._20;
				m0._21 = view._21;
				m0._22 = view._22;
				m0._23 = view._23;
				m0._30 = view._30;
				m0._31 = view._31;
				m0._32 = view._32;
				m0._33 = view._33;
				Scene.active.camera.V.multmat(m0);
				Scene.active.camera.P._00 = proj._00;
				Scene.active.camera.P._01 = proj._01;
				Scene.active.camera.P._02 = proj._02;
				Scene.active.camera.P._03 = proj._03;
				Scene.active.camera.P._10 = proj._10;
				Scene.active.camera.P._11 = proj._11;
				Scene.active.camera.P._12 = proj._12;
				Scene.active.camera.P._13 = proj._13;
				Scene.active.camera.P._20 = proj._20;
				Scene.active.camera.P._21 = proj._21;
				Scene.active.camera.P._22 = proj._22;
				Scene.active.camera.P._23 = proj._23;
				Scene.active.camera.P._30 = proj._30;
				Scene.active.camera.P._31 = proj._31;
				Scene.active.camera.P._32 = proj._32;
				Scene.active.camera.P._33 = proj._33;
				Scene.active.camera.VP.multmats(Scene.active.camera.P, Scene.active.camera.V);
				Scene.active.camera.transform.world.getInverse(Scene.active.camera.V);
				RenderPathForward.drawGbuffer("gbuffer0_eye", "gbuffer1_eye", "gbuffer2_eye");
				RenderPathForward.drawForward(true, "eye" + eye, "gbuffer0_eye", "gbuffer1_eye", "gbuffer2_eye", "buf_eye", "bufa_eye", "taa_eye", "taa2_eye");
			}

			Scene.active.camera.transform.world.setFrom(m);
			Scene.active.camera.buildProjection();

			Krom.vrBegin();

			path.drawStereo(function(eye: Int) {
				path.currentG = path.frameG;
				path.bindTarget("eye" + eye, "tex");
				path.drawShader("shader_datas/copy_pass/copy_pass");
			});

			Krom.vrWarpSwap();
		}

		iron.Scene.active.camera.buildMatrix();

		RenderPathPaint.begin();
		RenderPathForward.drawGbuffer();
		RenderPathForward.drawForward();
		RenderPathPaint.draw();
		RenderPathPaint.end();
		RenderPathDeferred.taaFrame++;
	}
}

#end
