package arm.render;

#if arm_vr

import kha.System;
import iron.RenderPath;
import iron.Scene;
import arm.ui.UISidebar;

class RenderPathForwardVR {

	public static var path: RenderPath;
	static var m = iron.math.Mat4.identity();

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

		{
			var current = RenderPathDeferred.taaFrame % 2 == 0 ? "bufa" : "taa2";
			var output = RenderPathDeferred.taaFrame % 2 == 0 ? current : "taa";

			m.setFrom(Scene.active.camera.transform.world);
			var t = Context.object.transform;
			t.translate(0, 1, -1);
			t.buildMatrix();

			for (eye in 0...2) {
				var view = Krom.vrGetSensorStateView(eye);
				var proj = Krom.vrGetSensorStateProjection(eye);
				Scene.active.camera.V._00 = view._00;
				Scene.active.camera.V._01 = view._01;
				Scene.active.camera.V._02 = view._02;
				Scene.active.camera.V._03 = view._03;
				Scene.active.camera.V._10 = view._10;
				Scene.active.camera.V._11 = view._11;
				Scene.active.camera.V._12 = view._12;
				Scene.active.camera.V._13 = view._13;
				Scene.active.camera.V._20 = view._20;
				Scene.active.camera.V._21 = view._21;
				Scene.active.camera.V._22 = view._22;
				Scene.active.camera.V._23 = view._23;
				Scene.active.camera.V._30 = view._30;
				Scene.active.camera.V._31 = view._31;
				Scene.active.camera.V._32 = view._32;
				Scene.active.camera.V._33 = view._33;
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
			t.translate(0, -1, 1);
			t.buildMatrix();

			Krom.vrBegin();

			path.drawStereo(function(eye: Int) {
				path.currentG = path.frameG;
				path.bindTarget("eye" + eye, "tex");
				path.drawShader("shader_datas/copy_pass/copy_pass");
			});

			Krom.vrWarpSwap();
		}

		iron.Scene.active.camera.buildMatrix();

		#if arm_painter
		RenderPathPaint.begin();
		#end // arm_painter

		RenderPathForward.drawGbuffer();
		RenderPathForward.drawForward();

		#if arm_painter
		RenderPathPaint.draw();
		#end

		#if arm_painter
		RenderPathPaint.end();
		#end

		RenderPathDeferred.taaFrame++;
	}
}

#end
