package arm.renderpath;

import iron.RenderPath;
import armory.renderpath.Inc;

class RenderPathDeferred {

	#if (rp_renderer == "Deferred")

	public static var path:RenderPath;

	#if (rp_gi != "Off")
	static var voxels = "voxels";
	static var voxelsLast = "voxels";
	#end
	static var initVoxels = true; // Bake AO
	static var taaFrame = 0;

	public static function init(_path:RenderPath) {

		path = _path;

		#if arm_editor
		armory.data.Config.raw.rp_gi = true;
		#end

		armory.renderpath.RenderPathDeferred.init(path);

		{
			var t = new RenderTargetRaw();
			t.name = "taa2";
			t.width = 0;
			t.height = 0;
			t.displayp = Inc.getDisplayp();
			t.format = "RGBA32";
			t.scale = Inc.getSuperSampling();
			path.createRenderTarget(t);
		}

		{
			path.createDepthBuffer("paintdb", "DEPTH16");
		}

		{
			var t = new RenderTargetRaw();
			t.name = "texpaint_colorid";
			t.width = 1;
			t.height = 1;
			t.format = 'RGBA32';
			path.createRenderTarget(t);
		}

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

	@:access(iron.RenderPath)
	public static function commands() {

		if (kha.System.windowWidth() == 0 || kha.System.windowHeight() == 0) return;

		var ssaa4 = armory.data.Config.raw.rp_supersample == 4 ? true : false;

		if (!arm.UITrait.inst.dirty()) {
			path.setTarget("");
			path.bindTarget(taaFrame % 2 == 0 ? "taa" : "taa2", "tex");
			ssaa4 ?
				path.drawShader("shader_datas/supersample_resolve/supersample_resolve") :
				path.drawShader("shader_datas/copy_pass/copy_pass");
			return;
		}

		var tid = arm.UITrait.inst.selectedLayer.id;

		if (arm.UITrait.inst.pushUndo && arm.UITrait.inst.C.undo_steps > 0) {
			var i = arm.UITrait.inst.undoI;
			if (arm.UITrait.inst.paintHeight) {
				path.setTarget("texpaint_undo" + i, ["texpaint_nor_undo" + i, "texpaint_pack_undo" + i, "texpaint_opt_undo" + i]);
			}
			else {
				path.setTarget("texpaint_undo" + i, ["texpaint_nor_undo" + i, "texpaint_pack_undo" + i]);
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
			arm.UITrait.inst.undoLayers[arm.UITrait.inst.undoI].targetObject = arm.UITrait.inst.paintObject;
			arm.UITrait.inst.undoLayers[arm.UITrait.inst.undoI].targetLayer = arm.UITrait.inst.selectedLayer;
			arm.UITrait.inst.undoI = (arm.UITrait.inst.undoI + 1) % arm.UITrait.inst.C.undo_steps;
			if (arm.UITrait.inst.undos < arm.UITrait.inst.C.undo_steps) arm.UITrait.inst.undos++;
			arm.UITrait.inst.redos = 0;
			arm.UITrait.inst.pushUndo = false;
		}

		if (arm.UITrait.inst.depthDirty()) {
			path.setTarget("texpaint" + tid);
			path.clearTarget(null, 1.0);
			path.drawMeshes("depth");
		}

		if (arm.UITrait.inst.paintDirty()) {
			if (arm.UITrait.inst.brushType == 4) { // Pick Color Id
				path.setTarget("texpaint_colorid");
				path.clearTarget(0xff000000);
				path.bindTarget("gbuffer2", "gbuffer2");
				path.drawMeshes("paint");
				arm.UITrait.inst.headerHandle.redraws = 2;
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

				//
				#if (!kha_opengl)
				path.setDepthFrom("bufa", "texpaint" + tid); // Unbind depth so we can read it
				path.setDepthFrom("texpaint" + tid, "texpaint_nor" + tid);
				path.depthToRenderTarget.set("paintdb", path.renderTargets.get("bufa"));
				#end
				//

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
					path.bindTarget("texpaint_colorid", "texpaint_colorid");
				} 

				// Read texcoords from gbuffer
				if (UITrait.inst.brushType == 2 && UITrait.inst.fillTypeHandle.position == 1) { // Face fill
					path.bindTarget("gbuffer2", "gbuffer2");
				}

				path.drawMeshes("paint");

				//
				#if (!kha_opengl)
				path.setDepthFrom("texpaint" + tid, "bufa"); // Re-bind depth
				path.setDepthFrom("bufa", "texpaint_nor" + tid);
				var tid0 = UITrait.inst.layers[0].id;
				path.depthToRenderTarget.set("paintdb", path.renderTargets.get("texpaint" + tid0));
				#end
				//
			}
		}
		//

		path.setTarget("gbuffer0"); // Only clear gbuffer0
		path.clearTarget(null, 1.0);
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

		// Paint
		tid = arm.UITrait.inst.layers[0].id;
		path.bindTarget("texpaint" + tid, "texpaint");
		path.bindTarget("texpaint_nor" + tid, "texpaint_nor");
		path.bindTarget("texpaint_pack" + tid, "texpaint_pack");
		if (arm.UITrait.inst.paintHeight) {
			path.bindTarget("texpaint_opt" + tid, "texpaint_opt");
		}
		for (i in 1...arm.UITrait.inst.layers.length) {
			tid = arm.UITrait.inst.layers[i].id;
			path.bindTarget("texpaint" + tid, "texpaint" + tid);
			path.bindTarget("texpaint_nor" + tid, "texpaint_nor" + tid);
			path.bindTarget("texpaint_pack" + tid, "texpaint_pack" + tid);
			if (arm.UITrait.inst.paintHeight) path.bindTarget("texpaint_opt" + tid, "texpaint_opt" + tid);
		}
		//

		RenderPathCreator.drawMeshes();

		#if rp_decals
		{
			#if (!kha_opengl)
			path.setDepthFrom("gbuffer0", "gbuffer1"); // Unbind depth so we can read it
			path.depthToRenderTarget.set("main", path.renderTargets.get("tex"));
			#end

			path.setTarget("gbuffer0", ["gbuffer1"]);
			path.bindTarget("_main", "gbufferD");
			path.drawDecals("decal");
			
			#if (!kha_opengl)
			path.setDepthFrom("gbuffer0", "tex"); // Re-bind depth
			path.depthToRenderTarget.set("main", path.renderTargets.get("gbuffer0"));
			#end
		}
		#end

		#if ((rp_ssgi == "RTGI") || (rp_ssgi == "RTAO"))
		{
			if (armory.data.Config.raw.rp_ssgi != false) {
				path.setTarget("singlea");
				path.bindTarget("_main", "gbufferD");
				path.bindTarget("gbuffer0", "gbuffer0");
				// #if (rp_ssgi == "RTGI")
				// path.bindTarget("gbuffer1", "gbuffer1");
				// #end
				path.drawShader("shader_datas/ssgi_pass/ssgi_pass");

				path.setTarget("singleb");
				path.bindTarget("singlea", "tex");
				path.bindTarget("gbuffer0", "gbuffer0");
				path.drawShader("shader_datas/blur_edge_pass/blur_edge_pass_x");

				path.setTarget("singlea");
				path.bindTarget("singleb", "tex");
				path.bindTarget("gbuffer0", "gbuffer0");
				path.drawShader("shader_datas/blur_edge_pass/blur_edge_pass_y");
			}
		}	
		#end

		// Voxels
		#if (rp_gi != "Off")
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
				var voxtex = voxels;

				path.clearImage(voxtex, 0x00000000);
				path.setTarget("");
				path.setViewport(res, res);
				path.bindTarget(voxtex, "voxels");
				path.drawMeshes("voxel");
				path.generateMipmaps(voxels);
			}
		}
		#end

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
			if (armory.data.Config.raw.rp_ssgi != false) {
				path.bindTarget("singlea", "ssaotex");
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
			voxelao_pass = true;
			path.bindTarget(voxels, "voxels");
			#if arm_voxelgi_temporal
			{
				path.bindTarget(voxelsLast, "voxelsLast");
			}
			#end
		}
		#end
		
		voxelao_pass ?
			path.drawShader("shader_datas/deferred_light/deferred_light_VoxelAOvar") :
			path.drawShader("shader_datas/deferred_light/deferred_light");

		#if (!kha_opengl)
		path.setDepthFrom("tex", "gbuffer0"); // Re-bind depth
		#end

		path.setTarget("tex"); // Re-binds depth
		path.drawSkydome("shader_datas/world_pass/world_pass");

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
				var targeta = "buf";
				var targetb = "gbuffer1";

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

		// Begin compositor
		#if rp_autoexposure
		{
			path.generateMipmaps("tex");
		}
		#end

		RenderPathCreator.finalTarget = path.currentTarget;
		path.setTarget("buf");
		
		path.bindTarget("tex", "tex");
		#if rp_compositordepth
		{
			path.bindTarget("_main", "gbufferD");
		}
		#end
		path.drawShader("shader_datas/compositor_pass/compositor_pass");
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

			// #if (rp_antialiasing == "TAA")
			path.setTarget("bufa");
			// #else
			// path.setTarget("");
			// #end
			path.bindTarget("buf", "colorTex");
			path.bindTarget("bufb", "blendTex");
			// #if (rp_antialiasing == "TAA")
			// {
				path.bindTarget("gbuffer2", "sveloc");
			// }
			// #end
			path.drawShader("shader_datas/smaa_neighborhood_blend/smaa_neighborhood_blend");

			// #if (rp_antialiasing == "TAA")
			// {
				path.setTarget(taaFrame % 2 == 0 ? "taa2" : "taa");
				path.bindTarget(taaFrame % 2 == 0 ? "taa" : "taa2", "tex2");
				path.bindTarget("bufa", "tex");
				path.bindTarget("gbuffer2", "sveloc");
				path.drawShader("shader_datas/taa_pass/taa_pass");
				if (!ssaa4) {
					path.setTarget("");
					path.bindTarget(taaFrame % 2 == 0 ? "taa2" : "taa", "tex");
					path.drawShader("shader_datas/copy_pass/copy_pass");
				}
				
			// }
			// #end
		}
		#end

		#if (rp_supersampling == 4)
		{
			if (ssaa4) {
				path.setTarget("");
				path.bindTarget(taaFrame % 2 == 0 ? "taa2" : "taa", "tex");
				path.drawShader("shader_datas/supersample_resolve/supersample_resolve");
			}
		}
		#end

		taaFrame++;
		arm.UITrait.inst.ddirty--;
		arm.UITrait.inst.pdirty--;
		arm.UITrait.inst.rdirty--;
	}

	#end
}
