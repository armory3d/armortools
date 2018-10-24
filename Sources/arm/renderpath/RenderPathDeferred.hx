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

	public static function drawMeshes() {
		path.drawMeshes("mesh");
	}

	public static function applyConfig() {
		Inc.applyConfig();
	}

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
			#if (rp_gi == "Voxel GI")
			{
				Inc.initGI("voxelsOpac");
				Inc.initGI("voxelsNor");
				#if (rp_gi_bounces)
				Inc.initGI("voxelsBounce");
				#end
			}
			#end
			#if (rp_gi == "Voxel AO")
			path.loadShader("shader_datas/deferred_indirect/deferred_indirect_VoxelAOvar");
			#end
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
			t.scale = Inc.getSuperSampling();
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
			t.scale = Inc.getSuperSampling();
			path.createRenderTarget(t);
		}

		{
			var t = new RenderTargetRaw();
			t.name = "gbuffer0";
			t.width = 0;
			t.height = 0;
			t.displayp = Inc.getDisplayp();
			t.format = "RGBA64";
			t.scale = Inc.getSuperSampling();
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
			t.scale = Inc.getSuperSampling();
			path.createRenderTarget(t);
		}

		#if rp_gbuffer2
		{
			var t = new RenderTargetRaw();
			t.name = "gbuffer2";
			t.width = 0;
			t.height = 0;
			t.displayp = Inc.getDisplayp();
			t.format = "RGBA64";
			t.scale = Inc.getSuperSampling();
			path.createRenderTarget(t);
		}

		{
			var t = new RenderTargetRaw();
			t.name = "taa";
			t.width = 0;
			t.height = 0;
			t.displayp = Inc.getDisplayp();
			t.format = "RGBA32";
			t.scale = Inc.getSuperSampling();
			path.createRenderTarget(t);
		}
		#end

		path.loadShader("shader_datas/deferred_indirect/deferred_indirect");
		path.loadShader("shader_datas/deferred_light/deferred_light");
		path.loadShader("shader_datas/deferred_light_quad/deferred_light_quad");

		#if rp_probes
		path.loadShader("shader_datas/probe_planar/probe_planar");
		path.loadShader("shader_datas/probe_cubemap/probe_cubemap");
		path.loadShader("shader_datas/copy_pass/copy_pass");
		#end

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
			t.scale = Inc.getSuperSampling();
			path.createRenderTarget(t);
		}
		{
			var t = new RenderTargetRaw();
			t.name = "bufb";
			t.width = 0;
			t.height = 0;
			t.displayp = Inc.getDisplayp();
			t.format = "RGBA32";
			t.scale = Inc.getSuperSampling();
			path.createRenderTarget(t);
		}
		#end

		// #if rp_rendercapture
		// {
		// 	var t = new RenderTargetRaw();
		// 	t.name = "capture";
		// 	t.width = 0;
		// 	t.height = 0;
		// 	t.format = Inc.getRenderCaptureFormat();
		// 	path.createRenderTarget(t);
		// }
		// #end

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
				t.scale = Inc.getSuperSampling();
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
				t.scale = Inc.getSuperSampling();
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

		#if arm_config
		{
			var t = new RenderTargetRaw();
			t.name = "empty_white";
			t.width = 1;
			t.height = 1;
			t.format = 'R8';
			var rt = new RenderTarget(t);
			var b = haxe.io.Bytes.alloc(1);
			b.set(0, 255);
			rt.image = kha.Image.fromBytes(b, t.width, t.height, kha.graphics4.TextureFormat.L8);
			path.renderTargets.set(t.name, rt);
		}
		#end

		// Paint
		{
			path.createDepthBuffer("paintdb", "DEPTH16");
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
		path.loadShader("shader_datas/copy_mrt3_pass/copy_mrt3_pass");
		path.loadShader("shader_datas/copy_mrt4_pass/copy_mrt4_pass");

		{ // Material preview
			{
				var t = new RenderTargetRaw();
				t.name = "texpreview";
				t.width = 1;
				t.height = 1;
				t.format = 'RGBA32';
				path.createRenderTarget(t);
			}

			{
				path.createDepthBuffer("mmain", "DEPTH24");

				var t = new RenderTargetRaw();
				t.name = "mtex";
				t.width = 100;
				t.height = 100;
				t.format = Inc.getHdrFormat();
				t.scale = Inc.getSuperSampling();
				t.depth_buffer = "mmain";
				path.createRenderTarget(t);
			}

			{
				var t = new RenderTargetRaw();
				t.name = "mbuf";
				t.width = 100;
				t.height = 100;
				t.format = Inc.getHdrFormat();
				t.scale = Inc.getSuperSampling();
				path.createRenderTarget(t);
			}

			{
				var t = new RenderTargetRaw();
				t.name = "mgbuffer0";
				t.width = 100;
				t.height = 100;
				t.format = "RGBA64";
				t.scale = Inc.getSuperSampling();
				t.depth_buffer = "mmain";
				path.createRenderTarget(t);
			}

			{
				var t = new RenderTargetRaw();
				t.name = "mgbuffer1";
				t.width = 100;
				t.height = 100;
				t.format = "RGBA64";
				t.scale = Inc.getSuperSampling();
				path.createRenderTarget(t);
			}

			#if rp_gbuffer2
			{
				var t = new RenderTargetRaw();
				t.name = "mgbuffer2";
				t.width = 100;
				t.height = 100;
				t.format = "RGBA64";
				t.scale = Inc.getSuperSampling();
				path.createRenderTarget(t);
			}
			#end

			#if ((rp_antialiasing == "SMAA") || (rp_antialiasing == "TAA"))
			{
				var t = new RenderTargetRaw();
				t.name = "mbufa";
				t.width = 100;
				t.height = 100;
				t.format = "RGBA32";
				t.scale = Inc.getSuperSampling();
				path.createRenderTarget(t);
			}
			{
				var t = new RenderTargetRaw();
				t.name = "mbufb";
				t.width = 100;
				t.height = 100;
				t.format = "RGBA32";
				t.scale = Inc.getSuperSampling();
				path.createRenderTarget(t);
			}
			#end
		}
		//
	}

	// Paint
	static var initVoxels = true;
	public static function drawShadowMap(l:iron.object.LightObject) {
		#if (rp_shadowmap)
		var faces = l.data.raw.shadowmap_cube ? 6 : 1;
		for (i in 0...faces) {
			if (faces > 1) path.currentFace = i;
			path.setTarget(Inc.getShadowMap());
			// Paint
			if (arm.UITrait.inst.paintHeight) {
				var tid = arm.UITrait.inst.layers[0].id;
				path.bindTarget("texpaint_opt" + tid, "texpaint_opt");
			}
			//
			path.clearTarget(null, 1.0);
			path.drawMeshes("shadowmap");
		}
		path.currentFace = -1;

		// One light at a time for now, precompute all lights for tiled
		#if rp_soft_shadows

		if (l.raw.type != "point") {
			path.setTarget("visa"); // Merge using min blend
			Inc.bindShadowMap();
			path.drawShader("shader_datas/dilate_pass/dilate_pass_x");

			path.setTarget("visb");
			path.bindTarget("visa", "shadowMap");
			path.drawShader("shader_datas/dilate_pass/dilate_pass_y");
		}

		path.setTarget("visa", ["dist"]);
		//if (i == 0) path.clearTarget(0x00000000);
		if (l.raw.type != "point") path.bindTarget("visb", "dilate");
		Inc.bindShadowMap();
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

		#end
	}

	@:access(iron.RenderPath)
	public static function commands() {

		if (arm.App.realw() == 0 || arm.App.realh() == 0) return;

		// Paint
		if (!arm.UITrait.inst.dirty()) {
			path.setTarget("");
			path.bindTarget("bufb", "tex");
			path.drawShader("shader_datas/copy_pass/copy_pass");
			return;
		}

		var tid = arm.UITrait.inst.selectedLayer.id;

		if (arm.UITrait.inst.pushUndo) {
			if (arm.UITrait.inst.paintHeight) {
				path.setTarget("texpaint_undo", ["texpaint_nor_undo", "texpaint_pack_undo", "texpaint_opt_undo"]);
			}
			else {
				path.setTarget("texpaint_undo", ["texpaint_nor_undo", "texpaint_pack_undo"]);
			}
			
			path.bindTarget("texpaint" + tid, "tex0");
			path.bindTarget("texpaint_nor" + tid, "tex1");
			path.bindTarget("texpaint_pack" + tid, "tex2");
			if (arm.UITrait.inst.paintHeight) {
				path.bindTarget("texpaint_opt" + tid, "tex3");
				path.drawShader("shader_datas/copy_mrt4_pass/copy_mrt4_pass");
			}
			else {
				path.drawShader("shader_datas/copy_mrt3_pass/copy_mrt3_pass");
			}
			arm.UITrait.inst.pushUndo = false;
		}

		if (arm.UITrait.inst.depthDirty()) {
			path.setTarget("texpaint" + tid);
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
				if (arm.UITrait.inst.paintHeight) {
					path.setTarget("texpaint" + tid, ["texpaint_nor" + tid, "texpaint_pack" + tid, "texpaint_opt" + tid]);
				}
				else {
					path.setTarget("texpaint" + tid, ["texpaint_nor" + tid, "texpaint_pack" + tid]);
				}
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
		tid = arm.UITrait.inst.layers[0].id;
		path.bindTarget("texpaint" + tid, "texpaint");
		path.bindTarget("texpaint_nor" + tid, "texpaint_nor");
		path.bindTarget("texpaint_pack" + tid, "texpaint_pack");
		if (arm.UITrait.inst.paintHeight) path.bindTarget("texpaint_opt" + tid, "texpaint_opt");
		if (arm.UITrait.inst.layers.length > 1) {
			tid = arm.UITrait.inst.layers[1].id;
			path.bindTarget("texpaint" + tid, "texpaint1");
			path.bindTarget("texpaint_nor" + tid, "texpaint_nor1");
			path.bindTarget("texpaint_pack" + tid, "texpaint_pack1");
			if (arm.UITrait.inst.paintHeight) path.bindTarget("texpaint_opt" + tid, "texpaint_opt1");
		}
		//

		#if rp_stereo
		{
			path.drawStereo(drawMeshes);
		}
		#else
		{
			RenderPathCreator.drawMeshes();
		}
		#end

		#if rp_decals
		{
			// path.setTarget("gbuffer0", ["gbuffer1"]);
			path.bindTarget("_main", "gbufferD");
			path.drawDecals("decal");
		}
		#end

		#if ((rp_ssgi == "RTGI") || (rp_ssgi == "RTAO"))
		{
			if (armory.data.Config.raw.rp_ssgi != false) {
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
		}	
		#elseif (rp_ssgi == "SSAO")
		{
			if (armory.data.Config.raw.rp_ssgi != false) {
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
		}
		#end

		// Voxels
		#if (rp_gi != "Off")
		var relight = false;
		if (armory.data.Config.raw.rp_gi != false)
		{
			var voxelize = path.voxelize();

			#if arm_voxelgi_temporal
			voxelize = ++RenderPathCreator.voxelFrame % RenderPathCreator.voxelFreq == 0;

			if (voxelize) {
				voxels = voxels == "voxels" ? "voxelsB" : "voxels";
				voxelsLast = voxels == "voxels" ? "voxelsB" : "voxels";
			}
			#end

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

				relight = true;
			}

			#if ((rp_gi == "Voxel GI") && (rp_voxelgi_relight))
			// Relight if light was moved
			for (light in iron.Scene.active.lights) {
				if (light.transform.diff()) { relight = true; break; }
			}
			#end

			if (relight) {
				#if (rp_gi == "Voxel GI")
					// Inc.computeVoxelsBegin();
					// for (i in 0...lights.length) Inc.computeVoxels(i); // Redraws SM
					// Inc.computeVoxelsEnd();
					#if (rp_gi_bounces)
					voxels = "voxelsBounce";
					#end
				#else
				path.generateMipmaps(voxels); // AO
				#end
			}
		}
		#end

		// Indirect
		path.setTarget("tex");
		// path.bindTarget("_main", "gbufferD");
		path.bindTarget("gbuffer0", "gbuffer0");
		path.bindTarget("gbuffer1", "gbuffer1");
		#if (rp_ssgi != "Off")
		{
			if (armory.data.Config.raw.rp_ssgi != false) {
				path.bindTarget("bufa", "ssaotex");
			}
			else {
				path.bindTarget("empty_white", "ssaotex");
			}
		}
		#end
		var voxelao_pass = false;
		#if (rp_gi != "Off")
		if (armory.data.Config.raw.rp_gi != false)
		{
			#if (rp_gi == "Voxel AO")
			voxelao_pass = true;
			#end
			path.bindTarget(voxels, "voxels");
			#if arm_voxelgi_temporal
			{
				path.bindTarget(voxelsLast, "voxelsLast");
			}
			#end
		}
		#end
		
		if (voxelao_pass) {
			path.drawShader("shader_datas/deferred_indirect/deferred_indirect_VoxelAOvar");
		}
		else {
			path.drawShader("shader_datas/deferred_indirect/deferred_indirect");
		}
		
		#if rp_probes
		if (!path.isProbe) {
			var probes = iron.Scene.active.probes;
			for (i in 0...probes.length) {
				var p = probes[i];
				if (!p.visible || p.culled) continue;
				path.currentProbeIndex = i;
				path.setTarget("tex");
				path.bindTarget("gbuffer0", "gbuffer0");
				path.bindTarget("gbuffer1", "gbuffer1");
				path.bindTarget(p.raw.name, "probeTex");
				if (p.data.raw.type == "planar") {
					path.drawVolume(p, "shader_datas/probe_planar/probe_planar");
				}
				else if (p.data.raw.type == "cubemap") {
					path.drawVolume(p, "shader_datas/probe_cubemap/probe_cubemap");
				}
			}
		}
		#end

		// Direct
		var lights = iron.Scene.active.lights;
		#if (rp_gi == "Voxel GI")
		if (relight) Inc.computeVoxelsBegin();
		#end
		for (i in 0...lights.length) {
			var l = lights[i];
			if (!l.visible) continue;
			path.currentLightIndex = i;

			#if (rp_shadowmap)
			{
				if (path.lightCastShadow()) {
					// Paint
					// Inc.drawShadowMap(l);
					drawShadowMap(l);
					//
				}
			}
			#end

			#if (rp_gi == "Voxel GI")
			if (relight) Inc.computeVoxels(i);
			#end

			path.setTarget("tex");
			// path.bindTarget("_main", "gbufferD");
			path.bindTarget("gbuffer0", "gbuffer0");
			path.bindTarget("gbuffer1", "gbuffer1");
			#if rp_gbuffer2_direct
			path.bindTarget("gbuffer2", "gbuffer2");
			#end

			#if rp_shadowmap
			{
				if (path.lightCastShadow()) {
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

			if (path.lightIsSun()) {
				path.drawShader("shader_datas/deferred_light_quad/deferred_light_quad");
			}
			else {
				path.drawLightVolume("shader_datas/deferred_light/deferred_light");
			}

			#if rp_volumetriclight
			{
				path.setTarget("bufvola");
				path.bindTarget("_main", "gbufferD");
				Inc.bindShadowMap();
				if (path.lightIsSun()) {
					path.drawShader("shader_datas/volumetric_light_quad/volumetric_light_quad");
				}
				else {
					path.drawLightVolume("shader_datas/volumetric_light/volumetric_light");
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
		path.currentLightIndex = 0;
		#if (rp_gi == "Voxel GI")
		if (relight) Inc.computeVoxelsEnd();
		#end

		#if (rp_background == "World")
		{
			path.drawSkydome("shader_datas/world_pass/world_pass");
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
			var hasLight = iron.Scene.active.lights.length > 0;
			if (hasLight) Inc.drawTranslucency("tex");
		}
		#end

		#if rp_bloom
		{
			if (armory.data.Config.raw.rp_bloom != false) {
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
		}
		#end

		#if rp_sss
		{
			path.setTarget("buf");
			path.bindTarget("tex", "tex");
			path.bindTarget("_main", "gbufferD");
			path.bindTarget("gbuffer2", "gbuffer2");
			path.drawShader("shader_datas/sss_pass/sss_pass_x");

			path.setTarget("tex");
			// TODO: can not bind tex
			path.bindTarget("tex", "tex");
			path.bindTarget("_main", "gbufferD");
			path.bindTarget("gbuffer2", "gbuffer2");
			path.drawShader("shader_datas/sss_pass/sss_pass_y");
		}
		#end

		#if rp_ssr
		{
			if (armory.data.Config.raw.rp_ssr != false) {
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
				path.bindTarget("gbuffer1", "gbuffer1");
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
		}
		#end

		#if ((rp_motionblur == "Camera") || (rp_motionblur == "Object"))
		{
			if (armory.data.Config.raw.rp_motionblur != false) {
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

		#if ((rp_antialiasing == "Off") || (rp_antialiasing == "FXAA") || (!rp_render_to_texture))
		{
			RenderPathCreator.finalTarget = path.currentTarget;
			path.setTarget(framebuffer);
		}
		#else
		{
			RenderPathCreator.finalTarget = path.currentTarget;
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
			if (!path.isProbe) path.drawShader("shader_datas/compositor_pass/compositor_pass");
			else path.drawShader("shader_datas/copy_pass/copy_pass");
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
			path.isProbe ? path.setTarget(framebuffer) : path.setTarget("bufa");
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
				if (!path.isProbe) { // No last frame for probe

					// Paint
					var isLast = arm.UITrait.inst.ddirty == 1 || arm.UITrait.inst.rdirty == 1;
					path.setTarget(isLast ? "bufb" : framebuffer);
					path.bindTarget("bufa", "tex");
					path.bindTarget("taa", "tex2");
					path.bindTarget("gbuffer2", "sveloc");
					path.drawShader("shader_datas/taa_pass/taa_pass");
					if (isLast) {
						path.setTarget(framebuffer);
						path.bindTarget("bufb", "tex");
						path.drawShader("shader_datas/copy_pass/copy_pass");
					}
					else {
						path.setTarget("taa");
						path.bindTarget("bufa", "tex");
						path.drawShader("shader_datas/copy_pass/copy_pass");
					}
					//
					// path.setTarget(framebuffer);
					// path.bindTarget("bufa", "tex");
					// path.bindTarget("taa", "tex2");
					// path.bindTarget("gbuffer2", "sveloc");
					// path.drawShader("shader_datas/taa_pass/taa_pass");
					// path.setTarget("taa");
					// path.bindTarget("bufa", "tex");
					// path.drawShader("shader_datas/copy_pass/copy_pass");
					//
				}
			}
			#end
		}
		#end

		#if (rp_supersampling == 4)
		{
			// #if rp_rendercapture
			// TODO: ss4 + capture broken
			// var finalTarget = "capture";
			// #else
			var finalTarget = "";
			// #end
			path.setTarget(finalTarget);
			path.bindTarget(framebuffer, "tex");
			path.drawShader("shader_datas/supersample_resolve/supersample_resolve");
		}
		// #elseif (rp_rendercapture)
		// {
			// path.setTarget("capture");
			// path.bindTarget(framebuffer, "tex");
			// path.drawShader("shader_datas/copy_pass/copy_pass");
		// }
		#end

		// paint
		arm.UITrait.inst.ddirty--;
		arm.UITrait.inst.pdirty--;
		arm.UITrait.inst.rdirty--;
		//
	}

	@:access(iron.RenderPath)
	public static function commandsPreview() {

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

		#if (rp_background == "Clear")
		{
			path.clearTarget(-1, 1.0);
		}
		#else
		{
			path.clearTarget(null, 1.0);
		}
		#end

		RenderPathCreator.drawMeshes();

		// Indirect
		path.setTarget("mtex");
		// path.bindTarget("_mmain", "gbufferD");
		path.bindTarget("mgbuffer0", "gbuffer0");
		path.bindTarget("mgbuffer1", "gbuffer1");
		#if (rp_ssgi != "Off")
		{
			path.bindTarget("empty_white", "ssaotex");
		}
		#end
		path.drawShader("shader_datas/deferred_indirect/deferred_indirect");

		// Direct
		var lights = iron.Scene.active.lights;
		for (i in 0...lights.length) {
			var l = lights[i];
			if (!l.visible) continue;
			path.currentLightIndex = i;

			path.setTarget("mtex");
			// path.bindTarget("_mmain", "gbufferD");
			path.bindTarget("mgbuffer0", "gbuffer0");
			path.bindTarget("mgbuffer1", "gbuffer1");
			#if rp_gbuffer2_direct
			path.bindTarget("mgbuffer2", "gbuffer2");
			#end

			if (path.lightIsSun()) {
				path.drawShader("shader_datas/deferred_light_quad/deferred_light_quad");
			}
			else {
				path.drawLightVolume("shader_datas/deferred_light/deferred_light");
			}
		}
		path.currentLightIndex = 0;

		#if (rp_background == "World")
		{
			path.drawSkydome("shader_datas/world_pass/world_pass");
		}
		#end
		
		var framebuffer = "texpreview";

		#if arm_editor
		var selectedMat = arm.UITrait.inst.htab.position == 0 ? arm.UITrait.inst.selectedMaterial2 : arm.UITrait.inst.selectedMaterial;
		#else
		var selectedMat = arm.UITrait.inst.selectedMaterial;
		#end
		iron.RenderPath.active.renderTargets.get("texpreview").image = selectedMat.image;

		#if ((rp_antialiasing == "Off") || (rp_antialiasing == "FXAA") || (!rp_render_to_texture))
		{
			path.setTarget(framebuffer);
		}
		#else
		{
			path.setTarget("mbuf");
		}
		#end
		
		path.bindTarget("mtex", "tex");
		#if rp_compositordepth
		{
			path.bindTarget("_mmain", "gbufferD");
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
			path.clearTarget(0x00000000, 0.0);

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
	public static function commandsSticker() {

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

		#if (rp_background == "Clear")
		{
			path.clearTarget(-1, 1.0);
		}
		#else
		{
			path.clearTarget(null, 1.0);
		}
		#end

		RenderPathCreator.drawMeshes();

		// Indirect
		path.setTarget("tex");
		// path.bindTarget("_main", "gbufferD");
		path.bindTarget("gbuffer0", "gbuffer0");
		path.bindTarget("gbuffer1", "gbuffer1");
		#if (rp_ssgi != "Off")
		{
			path.bindTarget("empty_white", "ssaotex");
		}
		#end
		path.drawShader("shader_datas/deferred_indirect/deferred_indirect");

		// Direct
		var lights = iron.Scene.active.lights;
		for (i in 0...lights.length) {
			var l = lights[i];
			if (!l.visible) continue;
			path.currentLightIndex = i;

			path.setTarget("tex");
			// path.bindTarget("_main", "gbufferD");
			path.bindTarget("gbuffer0", "gbuffer0");
			path.bindTarget("gbuffer1", "gbuffer1");
			#if rp_gbuffer2_direct
			path.bindTarget("gbuffer2", "gbuffer2");
			#end

			if (path.lightIsSun()) {
				path.drawShader("shader_datas/deferred_light_quad/deferred_light_quad");
			}
			else {
				path.drawLightVolume("shader_datas/deferred_light/deferred_light");
			}
		}
		path.currentLightIndex = 0;

		#if (rp_background == "World")
		{
			path.drawSkydome("shader_datas/world_pass/world_pass");
		}
		#end
		
		var framebuffer = "texpreview";

		iron.RenderPath.active.renderTargets.get("texpreview").image = arm.UITrait.inst.stickerImage;

		#if ((rp_antialiasing == "Off") || (rp_antialiasing == "FXAA") || (!rp_render_to_texture))
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
			path.clearTarget(0x00000000, 0.0);

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

	#end
}
