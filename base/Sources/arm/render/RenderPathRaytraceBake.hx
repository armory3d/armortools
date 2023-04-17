package arm.render;

import iron.RenderPath;
import iron.Scene;

#if (kha_direct3d12 || kha_vulkan)

@:access(arm.render.RenderPathRaytrace)
class RenderPathRaytraceBake {

	public static var raysPix = 0;
	public static var raysSec = 0;
	static var raysTimer = 0.0;
	static var raysCounter = 0;
	static var lastLayer: kha.Image = null;
	static var lastBake = 0;

	public static function commands(parsePaintMaterial: ?Bool->Void): Bool {
		var path = RenderPathRaytrace.path;

		if (!RenderPathRaytrace.ready || !RenderPathRaytrace.isBake || lastBake != Context.raw.bakeType) {
			var rebuild = !(RenderPathRaytrace.ready && RenderPathRaytrace.isBake && lastBake != Context.raw.bakeType);
			lastBake = Context.raw.bakeType;
			RenderPathRaytrace.ready = true;
			RenderPathRaytrace.isBake = true;
			RenderPathRaytrace.lastEnvmap = null;
			lastLayer = null;

			if (path.renderTargets.get("baketex0") != null) {
				path.renderTargets.get("baketex0").image.unload();
				path.renderTargets.get("baketex1").image.unload();
				path.renderTargets.get("baketex2").image.unload();
			}

			{
				var t = new RenderTargetRaw();
				t.name = "baketex0";
				t.width = Config.getTextureResX();
				t.height = Config.getTextureResY();
				t.format = "RGBA64";
				path.createRenderTarget(t);
			}
			{
				var t = new RenderTargetRaw();
				t.name = "baketex1";
				t.width = Config.getTextureResX();
				t.height = Config.getTextureResY();
				t.format = "RGBA64";
				path.createRenderTarget(t);
			}
			{
				var t = new RenderTargetRaw();
				t.name = "baketex2";
				t.width = Config.getTextureResX();
				t.height = Config.getTextureResY();
				t.format = "RGBA64"; // Match raytrace_target format
				path.createRenderTarget(t);
			}

			var _bakeType = Context.raw.bakeType;
			Context.raw.bakeType = BakeInit;
			parsePaintMaterial();
			path.setTarget("baketex0");
			path.clearTarget(0x00000000); // Pixels with alpha of 0.0 are skipped during raytracing
			path.setTarget("baketex0", ["baketex1"]);
			path.drawMeshes("paint");
			Context.raw.bakeType = _bakeType;
			function _next() {
				parsePaintMaterial();
			}
			App.notifyOnNextFrame(_next);

			RenderPathRaytrace.raytraceInit(getBakeShaderName(), rebuild);

			return false;
		}

		if (!Context.raw.envmapLoaded) ContextBase.loadEnvmap();
		var probe = Scene.active.world.probe;
		var savedEnvmap = Context.raw.showEnvmapBlur ? probe.radianceMipmaps[0] : Context.raw.savedEnvmap;
		if (RenderPathRaytrace.lastEnvmap != savedEnvmap || lastLayer != Context.raw.layer.texpaint) {
			RenderPathRaytrace.lastEnvmap = savedEnvmap;
			lastLayer = Context.raw.layer.texpaint;

			var baketex0 = path.renderTargets.get("baketex0").image;
			var baketex1 = path.renderTargets.get("baketex1").image;
			var bnoise_sobol = Scene.active.embedded.get("bnoise_sobol.k");
			var bnoise_scramble = Scene.active.embedded.get("bnoise_scramble.k");
			var bnoise_rank = Scene.active.embedded.get("bnoise_rank.k");
			var texpaint_undo = RenderPath.active.renderTargets.get("texpaint_undo" + History.undoI).image;
			Krom.raytraceSetTextures(baketex0.renderTarget_, baketex1.renderTarget_, texpaint_undo.renderTarget_, savedEnvmap.texture_, bnoise_sobol.texture_, bnoise_scramble.texture_, bnoise_rank.texture_);
		}

		if (Context.raw.brushTime > 0) {
			Context.raw.pdirty = 2;
			Context.raw.rdirty = 2;
		}

		if (Context.raw.pdirty > 0) {
			var f32 = RenderPathRaytrace.f32;
			f32[0] = RenderPathRaytrace.frame++;
			f32[1] = Context.raw.bakeAoStrength;
			f32[2] = Context.raw.bakeAoRadius;
			f32[3] = Context.raw.bakeAoOffset;
			f32[4] = Scene.active.world.probe.raw.strength;
			f32[5] = Context.raw.bakeUpAxis;
			f32[6] = Context.raw.envmapAngle;

			var framebuffer = path.renderTargets.get("baketex2").image;
			Krom.raytraceDispatchRays(framebuffer.renderTarget_, f32.buffer);

			path.setTarget("texpaint" + Context.raw.layer.id);
			path.bindTarget("baketex2", "tex");
			path.drawShader("shader_datas/copy_pass/copy_pass");

			raysPix = RenderPathRaytrace.frame * 64;
			raysCounter += 64;
			raysTimer += iron.system.Time.realDelta;
			if (raysTimer >= 1) {
				raysSec = raysCounter;
				raysTimer = 0;
				raysCounter = 0;
			}
			return true;
		}
		else {
			RenderPathRaytrace.frame = 0;
			raysTimer = 0;
			raysCounter = 0;
			return false;
		}
	}

	static function getBakeShaderName(): String {
		return
			Context.raw.bakeType == BakeAO  		? "raytrace_bake_ao" + RenderPathRaytrace.ext :
			Context.raw.bakeType == BakeLightmap 	? "raytrace_bake_light" + RenderPathRaytrace.ext :
			Context.raw.bakeType == BakeBentNormal  ? "raytrace_bake_bent" + RenderPathRaytrace.ext :
												  "raytrace_bake_thick" + RenderPathRaytrace.ext;
	}
}

#end
