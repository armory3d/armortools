package arm.render;

import iron.RenderPath;
import arm.ui.UITrait;
import arm.Tool;

@:access(iron.RenderPath)
class RenderPathPreview {

	public static function commandsPreview() {

		var path = RenderPathDeferred.path;

		#if rp_gbuffer2
		{
			path.setTarget("mgbuffer2");
			path.clearTarget(0xff000000);
			path.setTarget("mgbuffer0", ["mgbuffer1", "mgbuffer2"]);
		}
		#else
		{
			path.setTarget("mgbuffer0", ["mgbuffer1"]);
		}
		#end

		path.clearTarget(null, 1.0);

		RenderPathCreator.drawMeshes();

		// ---
		// Deferred light
		// ---
		#if (!kha_opengl)
		path.setDepthFrom("mtex", "mgbuffer1"); // Unbind depth so we can read it
		#end
		path.setTarget("mtex");
		path.bindTarget("_mmain", "gbufferD");
		path.bindTarget("mgbuffer0", "gbuffer0");
		path.bindTarget("mgbuffer1", "gbuffer1");
		#if (rp_ssgi != "Off")
		{
			path.bindTarget("empty_white", "ssaotex");
		}
		#end
		path.drawShader("shader_datas/deferred_light/deferred_light");

		#if (!kha_opengl)
		path.setDepthFrom("mtex", "mgbuffer0"); // Re-bind depth
		#end

		path.setTarget("mtex"); // Re-binds depth
		path.drawSkydome("shader_datas/world_pass/world_pass");
		
		var framebuffer = "texpreview";
		var selectedMat = UITrait.inst.worktab.position == SpaceScene ? UITrait.inst.selectedMaterialScene : UITrait.inst.selectedMaterial;
		RenderPath.active.renderTargets.get("texpreview").image = selectedMat.image;
		RenderPath.active.renderTargets.get("texpreview_icon").image = selectedMat.imageIcon;

		path.setTarget(framebuffer);

		path.bindTarget("mtex", "tex");
		#if rp_compositordepth
		{
			path.bindTarget("_mmain", "gbufferD");
		}
		#end
		path.drawShader("shader_datas/compositor_pass/compositor_pass");

		path.setTarget("texpreview_icon");
		path.bindTarget("texpreview", "tex");
		path.drawShader("shader_datas/supersample_resolve/supersample_resolve");
	}

	public static function commandsDecal() {

		var path = RenderPathDeferred.path;
		
		#if rp_gbuffer2
		{
			path.setTarget("gbuffer2");
			path.clearTarget(0xff000000);
			path.setTarget("gbuffer0", ["gbuffer1", "gbuffer2"]);
		}
		#else
		{
			path.setTarget("gbuffer0", ["gbuffer1"]);
		}
		#end

		path.clearTarget(null, 1.0);

		RenderPathCreator.drawMeshes();

		// ---
		// Deferred light
		// ---
		#if (!kha_opengl)
		path.setDepthFrom("tex", "gbuffer1"); // Unbind depth so we can read it
		#end
		path.setTarget("tex");
		path.bindTarget("_main", "gbufferD");
		path.bindTarget("gbuffer0", "gbuffer0");
		path.bindTarget("gbuffer1", "gbuffer1");
		#if (rp_ssgi != "Off")
		{
			path.bindTarget("empty_white", "ssaotex");
		}
		#end
		path.drawShader("shader_datas/deferred_light/deferred_light");

		#if (!kha_opengl)
		path.setDepthFrom("tex", "gbuffer0"); // Re-bind depth
		#end

		path.setTarget("tex"); // Re-binds depth
		path.drawSkydome("shader_datas/world_pass/world_pass");
		
		var framebuffer = "texpreview";
		RenderPath.active.renderTargets.get("texpreview").image = UITrait.inst.decalImage;

		path.setTarget(framebuffer);

		path.bindTarget("tex", "tex");
		#if rp_compositordepth
		{
			path.bindTarget("_main", "gbufferD");
		}
		#end
		path.drawShader("shader_datas/compositor_pass/compositor_pass");
	}
}
