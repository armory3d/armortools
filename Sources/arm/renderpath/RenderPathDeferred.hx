package arm.renderpath;

import iron.RenderPath;
import armory.renderpath.Inc;

class RenderPathDeferred {

	#if (rp_renderer == "Deferred")

	static var path:RenderPath;

	#if (rp_gi != "Off")
	static var voxels = "voxels";
	static var voxelsLast = "voxels";
	#end

	public static function init(_path:RenderPath) {

		path = _path;

		#if (rp_shadowmap && kha_webgl)
		Inc.initEmpty();
		#end

		#if (rp_background == "World")
		{
			path.loadShader("shader_datas/world_pass/world_pass");
		}
		#end

		#if (rp_translucency)
		{
			Inc.initTranslucency();
		}
		#end

		#if (rp_gi != "Off")
		{
			Inc.initGI();
			#if arm_voxelgi_temporal
			{
				Inc.initGI("voxelsB");
			}
			#end
			#if cpp
			#if (rp_gi == "Voxel GI")
			{
				Inc.initGI("voxelsOpac");
				Inc.initGI("voxelsNor");
			}
			#end
			#end //cpp
		}
		#end

		{
			path.createDepthBuffer("main", "DEPTH24");

			var t = new RenderTargetRaw();
			t.name = "tex";
			t.width = 0;
			t.height = 0;
			t.displayp = Inc.getDisplayp();
			t.format = Inc.getHdrFormat();
			var ss = Inc.getSuperSampling();
			if (ss != 1) t.scale = ss;
			t.depth_buffer = "main";
			#if rp_autoexposure
			t.mipmaps = true;
			#end
			path.createRenderTarget(t);
			#if rp_autoexposure
			// Texture lod is fetched manually, prevent mipmap filtering
			t.mipmaps = false;
			#end
		}

		{
			var t = new RenderTargetRaw();
			t.name = "buf";
			t.width = 0;
			t.height = 0;
			t.displayp = Inc.getDisplayp();
			t.format = Inc.getHdrFormat();
			var ss = Inc.getSuperSampling();
			if (ss != 1) t.scale = ss;
			path.createRenderTarget(t);
		}

		{
			path.createDepthBuffer("main", "DEPTH24");

			var t = new RenderTargetRaw();
			t.name = "gbuffer0";
			t.width = 0;
			t.height = 0;
			t.displayp = Inc.getDisplayp();
			t.format = "RGBA64";
			var ss = Inc.getSuperSampling();
			if (ss != 1) t.scale = ss;
			t.depth_buffer = "main";
			path.createRenderTarget(t);
		}

		{
			var t = new RenderTargetRaw();
			t.name = "gbuffer1";
			t.width = 0;
			t.height = 0;
			t.displayp = Inc.getDisplayp();
			t.format = "RGBA64";
			var ss = Inc.getSuperSampling();
			if (ss != 1) t.scale = ss;
			path.createRenderTarget(t);
		}

		#if arm_veloc
		{
			var t = new RenderTargetRaw();
			t.name = "gbuffer2";
			t.width = 0;
			t.height = 0;
			t.displayp = Inc.getDisplayp();
			t.format = "RGBA64";
			var ss = Inc.getSuperSampling();
			if (ss != 1) t.scale = ss;
			path.createRenderTarget(t);
		}

		{
			var t = new RenderTargetRaw();
			t.name = "taa";
			t.width = 0;
			t.height = 0;
			t.displayp = Inc.getDisplayp();
			t.format = "RGBA32";
			var ss = Inc.getSuperSampling();
			if (ss != 1) t.scale = ss;
			path.createRenderTarget(t);
		}
		#end

		path.loadShader("shader_datas/deferred_indirect/deferred_indirect");
		path.loadShader("shader_datas/deferred_light/deferred_light");
		path.loadShader("shader_datas/deferred_light_quad/deferred_light_quad");

		#if ((rp_ssgi == "RTGI") || (rp_ssgi == "RTAO"))
		{
			path.loadShader("shader_datas/ssgi_pass/ssgi_pass");
			path.loadShader("shader_datas/ssgi_blur_pass/ssgi_blur_pass_x");
			path.loadShader("shader_datas/ssgi_blur_pass/ssgi_blur_pass_y");
		}
		#elseif (rp_ssgi == "SSAO")
		{
			path.loadShader("shader_datas/ssao_pass/ssao_pass");
			path.loadShader("shader_datas/blur_edge_pass/blur_edge_pass_x");
			path.loadShader("shader_datas/blur_edge_pass/blur_edge_pass_y");
		}
		#end

		#if ((rp_ssgi != "Off") || (rp_antialiasing == "SMAA") || (rp_antialiasing == "TAA"))
		{
			var t = new RenderTargetRaw();
			t.name = "bufa";
			t.width = 0;
			t.height = 0;
			t.displayp = Inc.getDisplayp();
			t.format = "RGBA32";
			var ss = Inc.getSuperSampling();
			if (ss != 1) t.scale = ss;
			path.createRenderTarget(t);
		}
		{
			var t = new RenderTargetRaw();
			t.name = "bufb";
			t.width = 0;
			t.height = 0;
			t.displayp = Inc.getDisplayp();
			t.format = "RGBA32";
			var ss = Inc.getSuperSampling();
			if (ss != 1) t.scale = ss;
			path.createRenderTarget(t);
		}
		#end

		#if rp_rendercapture
		{
			var t = new RenderTargetRaw();
			t.name = "capture";
			t.width = 0;
			t.height = 0;
			t.format = getRenderCaptureFormat();
			path.createRenderTarget(t);
		}
		#end

		#if rp_compositornodes
		{
			path.loadShader("shader_datas/compositor_pass/compositor_pass");
		}
		#end

		#if ((!rp_compositornodes) || (rp_antialiasing == "TAA") || (rp_rendercapture) || (rp_motionblur == "Camera") || (rp_motionblur == "Object"))
		{
			path.loadShader("shader_datas/copy_pass/copy_pass");
		}
		#end

		#if ((rp_antialiasing == "SMAA") || (rp_antialiasing == "TAA"))
		{
			path.loadShader("shader_datas/smaa_edge_detect/smaa_edge_detect");
			path.loadShader("shader_datas/smaa_blend_weight/smaa_blend_weight");
			path.loadShader("shader_datas/smaa_neighborhood_blend/smaa_neighborhood_blend");

			#if (rp_antialiasing == "TAA")
			{
				path.loadShader("shader_datas/taa_pass/taa_pass");
			}
			#end
		}
		#end

		#if (rp_supersampling == 4)
		{
			path.loadShader("shader_datas/supersample_resolve/supersample_resolve");
		}
		#end

		#if rp_volumetriclight
		{
			path.loadShader("shader_datas/volumetric_light_quad/volumetric_light_quad");
			path.loadShader("shader_datas/volumetric_light/volumetric_light");
			path.loadShader("shader_datas/blur_bilat_pass/blur_bilat_pass_x");
			path.loadShader("shader_datas/blur_bilat_blend_pass/blur_bilat_blend_pass_y");
			{
				var t = new RenderTargetRaw();
				t.name = "bufvola";
				t.width = 0;
				t.height = 0;
				t.displayp = Inc.getDisplayp();
				t.format = "R8";
				var ss = Inc.getSuperSampling();
				if (ss != 1) t.scale = ss;
				// t.scale = 0.5;
				path.createRenderTarget(t);
			}
			{
				var t = new RenderTargetRaw();
				t.name = "bufvolb";
				t.width = 0;
				t.height = 0;
				t.displayp = Inc.getDisplayp();
				t.format = "R8";
				var ss = Inc.getSuperSampling();
				if (ss != 1) t.scale = ss;
				// t.scale = 0.5;
				path.createRenderTarget(t);
			}
		}
		#end

		#if rp_ocean
		{
			path.loadShader("shader_datas/water_pass/water_pass");
		}
		#end

		#if rp_bloom
		{
			var t = new RenderTargetRaw();
			t.name = "bloomtex";
			t.width = 0;
			t.height = 0;
			t.scale = 0.25;
			t.format = Inc.getHdrFormat();
			path.createRenderTarget(t);
		}

		{
			var t = new RenderTargetRaw();
			t.name = "bloomtex2";
			t.width = 0;
			t.height = 0;
			t.scale = 0.25;
			t.format = Inc.getHdrFormat();
			path.createRenderTarget(t);
		}

		{
			path.loadShader("shader_datas/bloom_pass/bloom_pass");
			path.loadShader("shader_datas/blur_gaus_pass/blur_gaus_pass_x");
			path.loadShader("shader_datas/blur_gaus_pass/blur_gaus_pass_y");
			path.loadShader("shader_datas/blur_gaus_pass/blur_gaus_pass_y_blend");
		}
		#end

		#if rp_sss
		{
			path.loadShader("shader_datas/sss_pass/sss_pass_x");
			path.loadShader("shader_datas/sss_pass/sss_pass_y");
		}
		#end

		#if rp_ssr
		{
			path.loadShader("shader_datas/ssr_pass/ssr_pass");
			path.loadShader("shader_datas/blur_adaptive_pass/blur_adaptive_pass_x");
			path.loadShader("shader_datas/blur_adaptive_pass/blur_adaptive_pass_y3_blend");
			
			#if rp_ssr_half
			{
				var t = new RenderTargetRaw();
				t.name = "ssra";
				t.width = 0;
				t.height = 0;
				t.scale = 0.5;
				t.format = Inc.getHdrFormat();
				path.createRenderTarget(t);
			}
			{
				var t = new RenderTargetRaw();
				t.name = "ssrb";
				t.width = 0;
				t.height = 0;
				t.scale = 0.5;
				t.format = Inc.getHdrFormat();
				path.createRenderTarget(t);
			}
			#end
		}
		#end

		#if ((rp_motionblur == "Camera") || (rp_motionblur == "Object"))
		{
			#if (rp_motionblur == "Camera")
			{
				path.loadShader("shader_datas/motion_blur_pass/motion_blur_pass");
			}
			#else
			{
				path.loadShader("shader_datas/motion_blur_veloc_pass/motion_blur_veloc_pass");
			}
			#end
		}
		#end

		#if rp_soft_shadows
		{
			path.loadShader("shader_datas/dilate_pass/dilate_pass_x");
			path.loadShader("shader_datas/dilate_pass/dilate_pass_y");
			path.loadShader("shader_datas/visibility_pass/visibility_pass");
			path.loadShader("shader_datas/blur_shadow_pass/blur_shadow_pass_x");
			path.loadShader("shader_datas/blur_shadow_pass/blur_shadow_pass_y");
			{
				var t = new RenderTargetRaw();
				t.name = "visa";
				t.width = 0;
				t.height = 0;
				t.format = 'R16';
				path.createRenderTarget(t);
			}
			{
				var t = new RenderTargetRaw();
				t.name = "visb";
				t.width = 0;
				t.height = 0;
				t.format = 'R16';
				path.createRenderTarget(t);
			}
			{
				var t = new RenderTargetRaw();
				t.name = "dist";
				t.width = 0;
				t.height = 0;
				t.format = 'R16';
				path.createRenderTarget(t);
			}
		}
		#end

		// Paint
		{
			path.createDepthBuffer("paintdb", "DEPTH16");

			var t = new RenderTargetRaw();
			t.name = "texpaint";
			t.width = 4096;
			t.height = 4096;
			t.format = 'RGBA32';
			t.depth_buffer = "paintdb";
			path.createRenderTarget(t);
		}
		{
			var t = new RenderTargetRaw();
			t.name = "texpaint_nor";
			t.width = 4096;
			t.height = 4096;
			t.format = 'RGBA32';
			path.createRenderTarget(t);
		}
		{
			var t = new RenderTargetRaw();
			t.name = "texpaint_pack";
			t.width = 4096;
			t.height = 4096;
			t.format = 'RGBA32';
			path.createRenderTarget(t);
		}

		var w = 1;
		for (i in 0...8)
		{
			var t = new RenderTargetRaw();
			t.name = "texpaint_colorid" + i;
			t.width = w;
			t.height = w;
			t.format = 'RGBA32';
			path.createRenderTarget(t);
			w *= 2;
		}

		path.loadShader("shader_datas/max_luminance_pass/max_luminance_pass");
		//
	}

	// Paint
	static var initVoxels = true;
	//

	@:access(iron.RenderPath)
	public static function commands() {

		if (arm.UITrait.inst.redraw()) { //

		// Paint
		if (arm.UITrait.inst.depthDirty()) {
			path.setTarget("texpaint");
			path.clearTarget(null, 1.0);
			path.drawMeshes("depth"); // TODO: CHECK DEPTH EXPORT
		}

		if (arm.UITrait.inst.paintDirty()) {
			if (arm.UITrait.inst.brushType == 4) { // Pick Color Id
				path.setTarget("texpaint_colorid7");
				path.clearTarget(0xff000000);
				path.bindTarget("_paintdb", "paintdb");
				path.drawMeshes("paint");
				// Extract picked color to 1x1 texture
				for (i in 0...7) {
					var j = 7 - i;
					path.setTarget("texpaint_colorid" + (j - 1));
					path.bindTarget("texpaint_colorid" + j, "tex");
					path.drawShader("shader_datas/max_luminance_pass/max_luminance_pass");
				}
			}
			else {
				if (arm.UITrait.inst.brushType == 3) { // Bake AO
					if (initVoxels) {
						initVoxels = false;
						var t = new RenderTargetRaw();
						t.name = "voxels";
						t.format = "R8";
						t.width = 256;
						t.height = 256;
						t.depth = 256;
						t.is_image = true;
						t.mipmaps = true;
						path.createRenderTarget(t);
					}
					path.clearImage("voxels", 0x00000000);
					path.setTarget("");
					path.setViewport(256, 256);
					path.bindTarget("voxels", "voxels");
					path.drawMeshes("voxel");
					path.generateMipmaps("voxels");
				}
				path.setTarget("texpaint", ["texpaint_nor", "texpaint_pack"]);
				path.bindTarget("_paintdb", "paintdb");
				if (arm.UITrait.inst.brushType == 3) { // Bake AO
					path.bindTarget("voxels", "voxels");
				}
				if (arm.UITrait.inst.colorIdPicked) {
					path.bindTarget("texpaint_colorid0", "texpaint_colorid0");
				} 
				path.drawMeshes("paint");
			}
		}
		//

		#if rp_dynres
		{
			DynamicResolutionScale.run(path);
		}
		#end

		#if arm_veloc
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

		#if (rp_background == "Clear")
		{
			path.clearTarget(-1, 1.0);
		}
		#else
		{
			if (!UITrait.drawWorld) { //
				path.clearTarget(UITrait.worldColor, 1.0);//
			} //
			else { //
				path.clearTarget(null, 1.0);
			} //
		}
		#end

		// Paint
		path.bindTarget("texpaint", "texpaint");
		path.bindTarget("texpaint_nor", "texpaint_nor");
		path.bindTarget("texpaint_pack", "texpaint_pack");
		//

		path.drawMeshes("mesh");

		#if rp_decals
		{
			// path.setTarget("gbuffer0", ["gbuffer1"]);
			path.bindTarget("_main", "gbufferD");
			path.drawDecals("decal");
		}
		#end

		#if ((rp_ssgi == "RTGI") || (rp_ssgi == "RTAO"))
		{
			path.setTarget("bufa");
			path.bindTarget("_main", "gbufferD");
			path.bindTarget("gbuffer0", "gbuffer0");
			#if (rp_ssgi == "RTGI")
			path.bindTarget("gbuffer1", "gbuffer1");
			#end
			path.drawShader("shader_datas/ssgi_pass/ssgi_pass");

			path.setTarget("bufb");
			path.bindTarget("gbuffer0", "gbuffer0");
			path.bindTarget("bufa", "tex");
			path.drawShader("shader_datas/ssgi_blur_pass/ssgi_blur_pass_x");

			path.setTarget("bufa");
			path.bindTarget("gbuffer0", "gbuffer0");
			path.bindTarget("bufb", "tex");
			path.drawShader("shader_datas/ssgi_blur_pass/ssgi_blur_pass_y");
		}	
		#elseif (rp_ssgi == "SSAO")
		{	
			path.setTarget("bufa");
			path.bindTarget("_main", "gbufferD");
			path.bindTarget("gbuffer0", "gbuffer0");
			path.drawShader("shader_datas/ssao_pass/ssao_pass");

			path.setTarget("bufb");
			path.bindTarget("bufa", "tex");
			path.bindTarget("gbuffer0", "gbuffer0");
			path.drawShader("shader_datas/blur_edge_pass/blur_edge_pass_x");

			path.setTarget("bufa");
			path.bindTarget("bufb", "tex");
			path.bindTarget("gbuffer0", "gbuffer0");
			path.drawShader("shader_datas/blur_edge_pass/blur_edge_pass_y");
		}
		#end

		// Voxels
		#if (rp_gi != "Off")
		{
			var voxelize = path.voxelize();

			#if arm_voxelgi_temporal
			voxelize = ++RenderPathCreator.voxelFrame % RenderPathCreator.voxelFreq == 0;

			if (voxelize) {
				voxels = voxels == "voxels" ? "voxelsB" : "voxels";
				voxelsLast = voxels == "voxels" ? "voxelsB" : "voxels";
			}
			#end

			#if cpp
			if (voxelize) {

				var res = Inc.getVoxelRes();

				#if (rp_gi == "Voxel GI")
				var voxtex = "voxelsOpac";
				#else
				var voxtex = voxels;
				#end

				path.clearImage(voxtex, 0x00000000);
				path.setTarget("");
				path.setViewport(res, res);
				path.bindTarget(voxtex, "voxels");
				path.drawMeshes("voxel");

				#if (rp_gi == "Voxel GI")
				Inc.computeVoxels();
					#if (rp_gi_bounces)
					voxels = "voxelsOpac";
					#end
				#else
				path.generateMipmaps(voxels);
				#end
			}
			#else
			if (voxelize) {
				path.clearImage(voxels, 0x00000000);
				path.setTarget("");
				var res = Inc.getVoxelRes();
				path.setViewport(res, res);
				path.bindTarget(voxels, "voxels");
				#if ((rp_shadowmap) && (rp_gi == "Voxel GI"))
				{
					Inc.bindShadowMap();
				}
				#end
				path.drawMeshes("voxel");
				path.generateMipmaps(voxels);
			}
			#end //cpp
		}
		#end

		// Indirect
		path.setTarget("tex");
		// path.bindTarget("_main", "gbufferD");
		path.bindTarget("gbuffer0", "gbuffer0");
		path.bindTarget("gbuffer1", "gbuffer1");
		#if (rp_ssgi != "Off")
		{
			path.bindTarget("bufa", "ssaotex");
		}
		#end
		#if (rp_gi != "Off")
		{
			path.bindTarget(voxels, "voxels");
			#if arm_voxelgi_temporal
			{
				path.bindTarget(voxelsLast, "voxelsLast");
			}
			#end
		}
		#end
		path.drawShader("shader_datas/deferred_indirect/deferred_indirect");

		// Direct
		var lamps = iron.Scene.active.lamps;
		for (i in 0...lamps.length) {
			var l = lamps[i];
			if (!l.visible) continue;
			path.currentLampIndex = i;

			#if (rp_shadowmap)
			{
				if (path.lampCastShadow()) {
					Inc.drawShadowMap(l);
				}
			}
			#end

			path.setTarget("tex");
			// path.bindTarget("_main", "gbufferD");
			path.bindTarget("gbuffer0", "gbuffer0");
			path.bindTarget("gbuffer1", "gbuffer1");

			#if rp_shadowmap
			{
				if (path.lampCastShadow()) {
					#if rp_soft_shadows
					path.bindTarget("visa", "svisibility");
					#else
					Inc.bindShadowMap();
					#end
				}
			}
			#end

			#if ((rp_voxelgi_shadows) || (rp_voxelgi_refraction))
			{
				path.bindTarget(voxels, "voxels");
			}
			#end

			if (path.lampIsSun()) {
				path.drawShader("shader_datas/deferred_light_quad/deferred_light_quad");
			}
			else {
				path.drawLampVolume("shader_datas/deferred_light/deferred_light");
			}

			#if rp_volumetriclight
			{
				path.setTarget("bufvola");
				path.bindTarget("_main", "gbufferD");
				Inc.bindShadowMap();
				if (path.lampIsSun()) {
					path.drawShader("shader_datas/volumetric_light_quad/volumetric_light_quad");
				}
				else {
					path.drawLampVolume("shader_datas/volumetric_light/volumetric_light");
				}

				path.setTarget("bufvolb");
				path.bindTarget("bufvola", "tex");
				path.drawShader("shader_datas/blur_bilat_pass/blur_bilat_pass_x");

				path.setTarget("tex");
				path.bindTarget("bufvolb", "tex");
				path.drawShader("shader_datas/blur_bilat_blend_pass/blur_bilat_blend_pass_y");
			}
			#end
		}
		path.currentLampIndex = 0;
		
		#if (rp_background == "World")
		{
			if (UITrait.drawWorld) { //
				path.drawSkydome("shader_datas/world_pass/world_pass");
			} //
		}
		#end

		#if rp_ocean
		{
			path.setTarget("tex");
			path.bindTarget("_main", "gbufferD");
			path.drawShader("shader_datas/water_pass/water_pass");
		}
		#end

		#if rp_blending
		{
			path.drawMeshes("blend");
		}
		#end

		#if rp_translucency
		{
			var hasLamp = iron.Scene.active.lamps.length > 0;
			if (hasLamp) Inc.drawTranslucency("tex");
		}
		#end

		#if rp_bloom
		{
			path.setTarget("bloomtex");
			path.bindTarget("tex", "tex");
			path.drawShader("shader_datas/bloom_pass/bloom_pass");

			path.setTarget("bloomtex2");
			path.bindTarget("bloomtex", "tex");
			path.drawShader("shader_datas/blur_gaus_pass/blur_gaus_pass_x");

			path.setTarget("bloomtex");
			path.bindTarget("bloomtex2", "tex");
			path.drawShader("shader_datas/blur_gaus_pass/blur_gaus_pass_y");

			path.setTarget("bloomtex2");
			path.bindTarget("bloomtex", "tex");
			path.drawShader("shader_datas/blur_gaus_pass/blur_gaus_pass_x");

			path.setTarget("bloomtex");
			path.bindTarget("bloomtex2", "tex");
			path.drawShader("shader_datas/blur_gaus_pass/blur_gaus_pass_y");

			path.setTarget("bloomtex2");
			path.bindTarget("bloomtex", "tex");
			path.drawShader("shader_datas/blur_gaus_pass/blur_gaus_pass_x");

			path.setTarget("bloomtex");
			path.bindTarget("bloomtex2", "tex");
			path.drawShader("shader_datas/blur_gaus_pass/blur_gaus_pass_y");

			path.setTarget("bloomtex2");
			path.bindTarget("bloomtex", "tex");
			path.drawShader("shader_datas/blur_gaus_pass/blur_gaus_pass_x");

			path.setTarget("tex");
			path.bindTarget("bloomtex2", "tex");
			path.drawShader("shader_datas/blur_gaus_pass/blur_gaus_pass_y_blend");
		}
		#end

		#if rp_sss
		{
			path.setTarget("buf");
			path.bindTarget("tex", "tex");
			path.bindTarget("_main", "gbufferD");
			path.bindTarget("gbuffer1", "gbuffer1");
			path.drawShader("shader_datas/sss_pass/sss_pass_x");

			path.setTarget("tex");
			// TODO: can not bind tex
			path.bindTarget("tex", "tex");
			path.bindTarget("_main", "gbufferD");
			path.bindTarget("gbuffer1", "gbuffer1");
			path.drawShader("shader_datas/sss_pass/sss_pass_y");
		}
		#end

		#if rp_ssr
		{
			#if rp_ssr_half
			var targeta = "ssra";
			var targetb = "ssrb";
			#else
			var targeta = "buf";
			var targetb = "gbuffer1";
			#end
			path.setTarget(targeta);
			path.bindTarget("tex", "tex");
			path.bindTarget("_main", "gbufferD");
			path.bindTarget("gbuffer0", "gbuffer0");
			path.drawShader("shader_datas/ssr_pass/ssr_pass");

			path.setTarget(targetb);
			path.bindTarget(targeta, "tex");
			path.bindTarget("gbuffer0", "gbuffer0");
			path.drawShader("shader_datas/blur_adaptive_pass/blur_adaptive_pass_x");

			path.setTarget("tex");
			path.bindTarget(targetb, "tex");
			path.bindTarget("gbuffer0", "gbuffer0");
			path.drawShader("shader_datas/blur_adaptive_pass/blur_adaptive_pass_y3_blend");
		}
		#end

		#if ((rp_motionblur == "Camera") || (rp_motionblur == "Object"))
		{
			path.setTarget("buf");
			path.bindTarget("tex", "tex");
			path.bindTarget("gbuffer0", "gbuffer0");
			#if (rp_motionblur == "Camera")
			{
				path.bindTarget("_main", "gbufferD");
				path.drawShader("shader_datas/motion_blur_pass/motion_blur_pass");
			}
			#else
			{
				path.bindTarget("gbuffer2", "sveloc");
				path.drawShader("shader_datas/motion_blur_veloc_pass/motion_blur_veloc_pass");
			}
			#end
			path.setTarget("tex");
			path.bindTarget("buf", "tex");
			path.drawShader("shader_datas/copy_pass/copy_pass");
		}
		#end

		// We are just about to enter compositing, add more custom passes here
		// #if rp_custom_pass
		// {
		// }
		// #end

		// Begin compositor
		#if rp_autoexposure
		{
			path.generateMipmaps("tex");
		}
		#end

		} // redraw

		#if ((rp_supersampling == 4) || (rp_rendercapture))
		var framebuffer = "buf";
		#else
		var framebuffer = "";
		#end

		if (arm.UITrait.inst.redraw()) { //

		#if ((rp_antialiasing == "Off") || (rp_antialiasing == "FXAA"))
		{
			path.setTarget(framebuffer);
		}
		#else
		{
			path.setTarget("buf");
		}
		#end
		
		path.bindTarget("tex", "tex");
		#if rp_compositordepth
		{
			path.bindTarget("_main", "gbufferD");
		}
		#end

		#if rp_compositornodes
		{
			path.drawShader("shader_datas/compositor_pass/compositor_pass");
		}
		#else
		{
			path.drawShader("shader_datas/copy_pass/copy_pass");
		}
		#end
		// End compositor

		#if rp_overlays
		{
			path.clearTarget(null, 1.0);
			path.drawMeshes("overlay");
		}
		#end

		} // redraw

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

			#if (rp_antialiasing == "TAA")
			path.setTarget("bufa");
			#else
			path.setTarget(framebuffer);
			#end
			path.bindTarget("buf", "colorTex");
			path.bindTarget("bufb", "blendTex");
			#if (rp_antialiasing == "TAA")
			{
				path.bindTarget("gbuffer2", "sveloc");
			}
			#end
			path.drawShader("shader_datas/smaa_neighborhood_blend/smaa_neighborhood_blend");

			#if (rp_antialiasing == "TAA")
			{
				path.setTarget(framebuffer);
				path.bindTarget("bufa", "tex");
				path.bindTarget("taa", "tex2");
				path.bindTarget("gbuffer2", "sveloc");
				path.drawShader("shader_datas/taa_pass/taa_pass");

				path.setTarget("taa");
				path.bindTarget("bufa", "tex");
				path.drawShader("shader_datas/copy_pass/copy_pass");
			}
			#end
		}
		#end

		#if (rp_supersampling == 4)
		{
			#if rp_rendercapture
			// TODO: ss4 + capture broken
			var final = "capture";
			#else
			var final = "";
			#end
			path.setTarget(final);
			path.bindTarget(framebuffer, "tex");
			path.drawShader("shader_datas/supersample_resolve/supersample_resolve");
		}
		#elseif (rp_rendercapture)
		{
			path.setTarget("capture");
			path.bindTarget(framebuffer, "tex");
			path.drawShader("shader_datas/copy_pass/copy_pass");
		}
		#end

		arm.UITrait.inst.dirty = false; //
	}
	#end
}
