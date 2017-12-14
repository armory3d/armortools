// Reference: https://github.com/armory3d/armory_docs/blob/master/dev/renderpath.md
package arm.renderpath;

import iron.RenderPath;

class RenderPathCreator {

	static var path:RenderPath;

	public static function get():RenderPath {
		path = new RenderPath();
		init();
		path.commands = commands;
		return path;
	}

	#if (rp_renderer == "Forward")
	static function init() {
		// #if rp_shadowmap // Auto-created
		// {
		//	var size = getShadowmapSize();
		// 	var t = new RenderTargetRaw();
		// 	t.name = "shadowMap";
		// 	t.width = size;
		// 	t.height = size;
		// 	t.format = "DEPTH16";
		// 	path.createRenderTarget(t);
		// }
		// #end

		#if (rp_background == "World")
		{
			path.loadShader("shader_datas/world/world");
		}
		#end

		#if rp_render_to_texture
		{
			path.createDepthBuffer("main", "DEPTH24");

			var t = new RenderTargetRaw();
			t.name = "lbuf";
			t.width = 0;
			t.height = 0;
			t.format = getHdrFormat();
			t.displayp = getDisplayp();
			var ss = getSuperSampling();
			if (ss != 1) t.scale = ss;
			t.depth_buffer = "main";
			path.createRenderTarget(t);

			#if rp_compositornodes
			{
				path.loadShader("shader_datas/compositor_pass/compositor_pass");
			}
			#else
			{
				path.loadShader("shader_datas/copy_pass/copy_pass");
			}
			#end
		}
		#end

		#if (rp_translucency)
		{
			initTranslucency();
		}
		#end

		#if (rp_gi != "Off")
		{
			initGI();
		}
		#end
	}

	static function commands() {

		#if rp_shadowmap
		{
			var faces = path.getLamp(path.currentLampIndex).data.raw.shadowmap_cube ? 6 : 1;
			for (i in 0...faces) {
				if (faces > 1) path.currentFace = i;
				path.setTarget("shadowMap");
				path.clearTarget(null, 1.0);
				path.drawMeshes("shadowmap");
			}
			path.currentFace = -1;
		}
		#end

		#if (rp_gi != "Off")
		{
			if (path.voxelize()) {
				path.clearImage("voxels", 0x00000000);
				path.setTarget("");
				var res = getVoxelRes();
				path.setViewport(res, res);
				path.bindTarget("voxels", "voxels");
				#if rp_shadowmap
				{
					path.bindTarget("shadowMap", "shadowMap");
				}
				#end
				path.drawMeshes("voxel");
				path.generateMipmaps("voxels");
			}
		}
		#end

		#if rp_render_to_texture
		{
			path.setTarget("lbuf");
		}
		#else
		{
			path.setTarget("");
		}
		#end

		#if (rp_background == "Clear")
		{
			path.clearTarget(-1, 1.0);
		}
		#else
		{
			path.clearTarget(null, 1.0);
		}
		#end

		#if rp_depthprepass
		{
			path.drawMeshes("depth");
		}
		#end

		#if rp_shadowmap
		{
			path.bindTarget("shadowMap", "shadowMap");
		}
		#end

		#if (rp_gi != "Off")
		{
			path.bindTarget("voxels", "voxels");
		}
		#end

		function drawMeshes() {
			path.drawMeshes("mesh");
			#if (rp_background == "World")
			{
				path.drawSkydome("shader_datas/world/world");
			}
			#end

			#if rp_translucency
			{
				drawTranslucency("lbuf");
			}
			#end
		}

		#if rp_stereo
		{
			path.drawStereo(drawMeshes);
		}
		#else
		{
			drawMeshes();
		}
		#end

		#if rp_render_to_texture
		{
			path.setTarget("");
			path.bindTarget("lbuf", "tex");

			#if rp_compositornodes
			{
				path.drawShader("shader_datas/compositor_pass/compositor_pass");
			}
			#else
			{
				path.drawShader("shader_datas/copy_pass/copy_pass");
			}
			#end
		}
		#end

		#if rp_overlays
		{
			path.clearTarget(null, 1.0);
			path.drawMeshes("overlay");
		}
		#end
	}
	#end





	#if (rp_renderer == "Deferred")
	static function init() {
		#if (rp_background == "World")
		{
			path.loadShader("shader_datas/world/world");
		}
		#end

		#if (rp_translucency)
		{
			initTranslucency();
		}
		#end

		#if (rp_gi != "Off")
		{
			initGI();
		}
		#end

		{
			path.createDepthBuffer("main", "DEPTH24");

			var t = new RenderTargetRaw();
			t.name = "tex";
			t.width = 0;
			t.height = 0;
			t.displayp = getDisplayp();
			t.format = getHdrFormat();
			var ss = getSuperSampling();
			if (ss != 1) t.scale = ss;
			t.depth_buffer = "main";
			#if rp_autoexposure
			t.mipmaps = true;
			#end
			path.createRenderTarget(t);
		}

		{
			var t = new RenderTargetRaw();
			t.name = "buf";
			t.width = 0;
			t.height = 0;
			t.displayp = getDisplayp();
			t.format = getHdrFormat();
			var ss = getSuperSampling();
			if (ss != 1) t.scale = ss;
			path.createRenderTarget(t);
		}

		{
			path.createDepthBuffer("main", "DEPTH24");

			var t = new RenderTargetRaw();
			t.name = "gbuffer0";
			t.width = 0;
			t.height = 0;
			t.displayp = getDisplayp();
			t.format = "RGBA64";
			var ss = getSuperSampling();
			if (ss != 1) t.scale = ss;
			t.depth_buffer = "main";
			path.createRenderTarget(t);
		}

		{
			var t = new RenderTargetRaw();
			t.name = "gbuffer1";
			t.width = 0;
			t.height = 0;
			t.displayp = getDisplayp();
			t.format = "RGBA64";
			var ss = getSuperSampling();
			if (ss != 1) t.scale = ss;
			path.createRenderTarget(t);
		}

		#if arm_veloc
		{
			var t = new RenderTargetRaw();
			t.name = "gbuffer2";
			t.width = 0;
			t.height = 0;
			t.displayp = getDisplayp();
			t.format = "RGBA64";
			var ss = getSuperSampling();
			if (ss != 1) t.scale = ss;
			path.createRenderTarget(t);
		}

		{
			var t = new RenderTargetRaw();
			t.name = "taa";
			t.width = 0;
			t.height = 0;
			t.displayp = getDisplayp();
			t.format = "RGBA32";
			var ss = getSuperSampling();
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
			t.displayp = getDisplayp();
			t.format = "RGBA32";
			var ss = getSuperSampling();
			if (ss != 1) t.scale = ss;
			path.createRenderTarget(t);
		}
		{
			var t = new RenderTargetRaw();
			t.name = "bufb";
			t.width = 0;
			t.height = 0;
			t.displayp = getDisplayp();
			t.format = "RGBA32";
			var ss = getSuperSampling();
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
			path.loadShader("shader_datas/blur_edge_pass/blur_edge_pass_x");
			path.loadShader("shader_datas/blur_edge_pass/blur_edge_pass_y_blend_add");
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
			t.format = getHdrFormat();
			path.createRenderTarget(t);
		}

		{
			var t = new RenderTargetRaw();
			t.name = "bloomtex2";
			t.width = 0;
			t.height = 0;
			t.scale = 0.25;
			t.format = getHdrFormat();
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
				t.format = getHdrFormat();
				path.createRenderTarget(t);
			}
			{
				var t = new RenderTargetRaw();
				t.name = "ssrb";
				t.width = 0;
				t.height = 0;
				t.scale = 0.5;
				t.format = getHdrFormat();
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
		//
	}

	static function drawShadowMap(l:iron.object.LampObject) {
		var faces = l.data.raw.shadowmap_cube ? 6 : 1;
		for (j in 0...faces) {
			if (faces > 1) path.currentFace = j;
			path.setTarget("shadowMap");
			path.clearTarget(null, 1.0);
			path.drawMeshes("shadowmap");
		}
		path.currentFace = -1;

		// One lamp at a time for now, precompute all lamps for tiled
		#if rp_soft_shadows

		path.setTarget("visa"); // Merge using min blend
		path.bindTarget("shadowMap", "shadowMap");
		path.drawShader("shader_datas/dilate_pass/dilate_pass_x");

		path.setTarget("visb");
		path.bindTarget("visa", "shadowMap");
		path.drawShader("shader_datas/dilate_pass/dilate_pass_y");

		path.setTarget("visa", ["dist"]);
		//if (i == 0) path.clearTarget(0x00000000);
		path.bindTarget("visb", "dilate");
		path.bindTarget("shadowMap", "shadowMap");
		//path.bindTarget("_main", "gbufferD");
		path.bindTarget("gbuffer0", "gbuffer0");
		path.drawShader("shader_datas/visibility_pass/visibility_pass");
		
		path.setTarget("visb");
		path.bindTarget("visa", "tex");
		path.bindTarget("gbuffer0", "gbuffer0");
		path.bindTarget("dist", "dist");
		path.drawShader("shader_datas/blur_shadow_pass/blur_shadow_pass_x");

		path.setTarget("visa");
		path.bindTarget("visb", "tex");
		path.bindTarget("gbuffer0", "gbuffer0");
		path.bindTarget("dist", "dist");
		path.drawShader("shader_datas/blur_shadow_pass/blur_shadow_pass_y");
		#end
	}

	static function commands() {

		// Paint
		if (arm.UITrait.depthDirty()) {
			path.setTarget("texpaint");
			path.clearTarget(null, 1.0);
			path.drawMeshes("depth"); // TODO: CHECK DEPTH EXPORT
		}

		if (arm.UITrait.paintDirty()) {
			path.setTarget("texpaint", ["texpaint_nor", "texpaint_pack"]);
			path.bindTarget("_paintdb", "paintdb");
			path.drawMeshes("paint");
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
			path.clearTarget(null, 1.0);
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
			#if ((rp_ssgi == "RTGI"))
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
			#if ((rp_shadowmap) && (rp_gi == "Voxel GI"))
			{
				if (path.lampCastShadow() && iron.Scene.active.lamps.length > 0) {
					drawShadowMap(iron.Scene.active.lamps[0]);
				}
			}
			#end
			
			if (path.voxelize()) {
				path.clearImage("voxels", 0x00000000);
				path.setTarget("");
				var res = getVoxelRes();
				path.setViewport(res, res);
				path.bindTarget("voxels", "voxels");
				#if ((rp_shadowmap) && (rp_gi == "Voxel GI"))
				{
					path.bindTarget("shadowMap", "shadowMap");
				}
				#end
				path.drawMeshes("voxel");
				path.generateMipmaps("voxels");
			}
		}
		#end

		// Indirect
		path.setTarget("tex");
		path.bindTarget("_main", "gbufferD");
		path.bindTarget("gbuffer0", "gbuffer0");
		path.bindTarget("gbuffer1", "gbuffer1");
		#if (rp_ssgi != "Off")
		{
			path.bindTarget("bufa", "ssaotex");
		}
		#end
		#if (rp_gi != "Off")
		{
			path.bindTarget("voxels", "voxels");
		}
		#end
		path.drawShader("shader_datas/deferred_indirect/deferred_indirect");

		// Direct
		var lamps = iron.Scene.active.lamps;
		for (i in 0...lamps.length) {
			var l = lamps[i];
			if (!l.visible) continue;
			path.currentLampIndex = i;

			#if ((rp_shadowmap) && (rp_gi != "Voxel GI"))
			{
				if (path.lampCastShadow()) {
					drawShadowMap(l);
				}
			}
			#end

			path.setTarget("tex");
			path.bindTarget("_main", "gbufferD");
			path.bindTarget("gbuffer0", "gbuffer0");
			path.bindTarget("gbuffer1", "gbuffer1");

			#if rp_shadowmap
			{
				if (path.lampCastShadow()) {
					#if rp_soft_shadows
					path.bindTarget("visa", "svisibility");
					#else
					path.bindTarget("shadowMap", "shadowMap");
					#end
				}
			}
			#end

			#if ((rp_voxelgi_shadows) || (rp_voxelgi_refraction))
			{
				path.bindTarget("voxels", "voxels");
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
				path.setTarget("buf");
				path.bindTarget("_main", "gbufferD");
				path.bindTarget("shadowMap", "shadowMap");
				if (path.lampIsSun()) {
					path.drawShader("shader_datas/volumetric_light_quad/volumetric_light_quad");
				}
				else {
					path.drawLampVolume("shader_datas/volumetric_light/volumetric_light");
				}
				path.setTarget("gbuffer1");
				path.bindTarget("buf", "tex");
				path.bindTarget("gbuffer0", "gbuffer0");
				path.drawShader("shader_datas/blur_edge_pass/blur_edge_pass_x");

				path.setTarget("tex");
				path.bindTarget("gbuffer1", "tex");
				path.bindTarget("gbuffer0", "gbuffer0");
				path.drawShader("shader_datas/blur_edge_pass/blur_edge_pass_y_blend_add");
			}
			#end
		}
		path.currentLampIndex = 0;

		#if (rp_background == "World")
		{
			path.drawSkydome("shader_datas/world/world");
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
			drawTranslucency("tex");
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

		#if ((rp_supersampling == 4) || (rp_rendercapture))
		var framebuffer = "buf";
		#else
		var framebuffer = "";
		#end

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
	}
	#end





	// #if (rp_renderer == "Deferred Plus")
	// static function init() {

	// }

	// static function commands() {

	// }
	// #end





	#if (rp_translucency)
	static function initTranslucency() {
		path.createDepthBuffer("main", "DEPTH24");

		var t = new RenderTargetRaw();
		t.name = "accum";
		t.width = 0;
		t.height = 0;
		t.displayp = getDisplayp();
		t.format = "RGBA64";
		var ss = getSuperSampling();
		if (ss != 1) t.scale = ss;
		t.depth_buffer = "main";
		path.createRenderTarget(t);

		var t = new RenderTargetRaw();
		t.name = "revealage";
		t.width = 0;
		t.height = 0;
		t.displayp = getDisplayp();
		t.format = "RGBA64";
		var ss = getSuperSampling();
		if (ss != 1) t.scale = ss;
		t.depth_buffer = "main";
		path.createRenderTarget(t);

		path.loadShader("shader_datas/translucent_resolve/translucent_resolve");
	}

	static function drawTranslucency(target:String) {
		path.setTarget("accum");
		path.clearTarget(0xff000000);
		path.setTarget("revealage");
		path.clearTarget(0xffffffff);
		path.setTarget("accum", ["revealage"]);
		#if rp_shadowmap
		{
			path.bindTarget("shadowMap", "shadowMap");
		}
		#end
		path.drawMeshes("translucent");
		#if rp_render_to_texture
		{
			path.setTarget(target);
		}
		#else
		{
			path.setTarget("");
		}
		#end
		path.bindTarget("accum", "gbuffer0");
		path.bindTarget("revealage", "gbuffer1");
		path.drawShader("shader_datas/translucent_resolve/translucent_resolve");
	}
	#end

	#if (rp_gi != "Off")
	static function initGI() {
		var t = new RenderTargetRaw();
		t.name = "voxels";
		#if (rp_gi == "Voxel AO")
		{
			t.format = "R8";
		}
		#elseif (rp_voxelgi_hdr)
		{
			t.format = "RGBA64";
		}
		#else
		{
			t.format = "RGBA32";
		}
		#end
		var res = getVoxelRes();
		var resZ =  getVoxelResZ();
		t.width = res;
		t.height = res;
		t.depth = Std.int(res * resZ);
		t.is_image = true;
		path.createRenderTarget(t);
	}
	#end

	static inline function getShadowmapSize():Int {
		#if (rp_shadowmap_size == 512)
		return 512;
		#elseif (rp_shadowmap_size == 1024)
		return 1024;
		#elseif (rp_shadowmap_size == 2048)
		return 2048;
		#elseif (rp_shadowmap_size == 4096)
		return 4096;
		#elseif (rp_shadowmap_size == 8192)
		return 8192;
		#elseif (rp_shadowmap_size == 16384)
		return 16384;
		#else
		return 0;
		#end
	}

	static inline function getVoxelRes():Int {
		#if (rp_voxelgi_resolution == 512)
		return 512;
		#elseif (rp_voxelgi_resolution == 256)
		return 256;
		#elseif (rp_voxelgi_resolution == 128)
		return 128;
		#elseif (rp_voxelgi_resolution == 64)
		return 64;
		#elseif (rp_voxelgi_resolution == 32)
		return 32;
		#else
		return 0;
		#end
	}

	static inline function getVoxelResZ():Float {
		#if (rp_voxelgi_resolution_z == 1.0)
		return 1.0;
		#elseif (rp_voxelgi_resolution_z == 0.5)
		return 0.5;
		#elseif (rp_voxelgi_resolution_z == 0.25)
		return 0.25;
		#else
		return 0.0;
		#end
	}

	static inline function getSuperSampling():Int {
		#if (rp_supersampling == 2)
		return 2;
		#elseif (rp_supersampling == 4)
		return 4;
		#else
		return 1;
		#end
	}

	static inline function getHdrFormat():String {
		#if rp_hdr
		return "RGBA64";
		#else
		return "RGBA32";
		#end
	}

	static inline function getDisplayp():Null<Int> {
		#if (rp_resolution == 480)
		return 480;
		#elseif (rp_resolution == 720)
		return 720;
		#elseif (rp_resolution == 1080)
		return 1080;
		#elseif (rp_resolution == 1440)
		return 1440;
		#elseif (rp_resolution == 2160)
		return 2160;
		#else
		return null;
		#end
	}

	static inline function getRenderCaptureFormat():String {
		#if (rp_rendercapture_format == "8bit")
		return "RGBA32";
		#elseif (rp_rendercapture_format == "16bit")
		return "RGBA64";
		#elseif (rp_rendercapture_format == "32bit")
		return "RGBA128";
		#else
		return "RGBA32";
		#end
	}
}
