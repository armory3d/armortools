package arm.renderpath;

class RenderPathPreview {

	@:access(iron.RenderPath)
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

		#if arm_editor
		var selectedMat = arm.UITrait.inst.htab.position == 0 ? arm.UITrait.inst.selectedMaterial2 : arm.UITrait.inst.selectedMaterial;
		#else
		var selectedMat = arm.UITrait.inst.selectedMaterial;
		#end
		iron.RenderPath.active.renderTargets.get("texpreview").image = selectedMat.image;

		path.setTarget("mbuf");
		path.bindTarget("mtex", "tex");
		#if rp_compositordepth
		{
			path.bindTarget("_mmain", "gbufferD");
		}
		#end
		path.drawShader("shader_datas/compositor_pass/compositor_pass");

		#if ((rp_antialiasing == "SMAA") || (rp_antialiasing == "TAA"))
		{
			path.setTarget("mbufa");
			path.clearTarget(0x00000000);
			path.bindTarget("mbuf", "colorTex");
			path.drawShader("shader_datas/smaa_edge_detect/smaa_edge_detect");

			path.setTarget("mbufb");
			path.clearTarget(0x00000000);
			path.bindTarget("mbufa", "edgesTex");
			path.drawShader("shader_datas/smaa_blend_weight/smaa_blend_weight");

			path.setTarget(framebuffer);

			path.bindTarget("mbuf", "colorTex");
			path.bindTarget("mbufb", "blendTex");
			#if (rp_antialiasing == "TAA")
			{
				path.bindTarget("mgbuffer2", "sveloc");
			}
			#end
			path.drawShader("shader_datas/smaa_neighborhood_blend/smaa_neighborhood_blend");
		}
		#end
	}

	@:access(iron.RenderPath)
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

		iron.RenderPath.active.renderTargets.get("texpreview").image = arm.UITrait.inst.decalImage;

		path.setTarget("buf");
		path.bindTarget("tex", "tex");
		#if rp_compositordepth
		{
			path.bindTarget("_main", "gbufferD");
		}
		#end
		path.drawShader("shader_datas/compositor_pass/compositor_pass");

		#if ((rp_antialiasing == "SMAA") || (rp_antialiasing == "TAA"))
		{
			path.setTarget("bufa");
			path.clearTarget(0x00000000);
			path.bindTarget("buf", "colorTex");
			path.drawShader("shader_datas/smaa_edge_detect/smaa_edge_detect");

			path.setTarget("bufb");
			path.clearTarget(0x00000000);
			path.bindTarget("bufa", "edgesTex");
			path.drawShader("shader_datas/smaa_blend_weight/smaa_blend_weight");

			path.setTarget(framebuffer);

			path.bindTarget("buf", "colorTex");
			path.bindTarget("bufb", "blendTex");
			#if (rp_antialiasing == "TAA")
			{
				path.bindTarget("gbuffer2", "sveloc");
			}
			#end
			path.drawShader("shader_datas/smaa_neighborhood_blend/smaa_neighborhood_blend");
		}
		#end
	}
}