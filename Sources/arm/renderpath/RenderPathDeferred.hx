package arm.renderpath;

import iron.RenderPath;
import armory.renderpath.Inc;
import arm.ui.UITrait;
import arm.ui.*;

class RenderPathDeferred {

	#if (rp_renderer == "Deferred")

	public static var path:RenderPath;

	#if rp_voxelao
	static var voxels = "voxels";
	static var voxelsLast = "voxels";
	#end
	static var initVoxels = true; // Bake AO
	static var taaFrame = 0;

	public static function init(_path:RenderPath) {

		path = _path;

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

		{
			var t = new RenderTargetRaw();
			t.name = "texpaint_picker";
			t.width = 1;
			t.height = 1;
			t.format = 'RGBA32';
			path.createRenderTarget(t);
		}
		{
			var t = new RenderTargetRaw();
			t.name = "texpaint_nor_picker";
			t.width = 1;
			t.height = 1;
			t.format = 'RGBA32';
			path.createRenderTarget(t);
		}
		{
			var t = new RenderTargetRaw();
			t.name = "texpaint_pack_picker";
			t.width = 1;
			t.height = 1;
			t.format = 'RGBA32';
			path.createRenderTarget(t);
		}

		path.loadShader("shader_datas/copy_mrt3_pass/copy_mrt3_pass");

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
				t.width = arm.util.RenderUtil.matPreviewSize;
				t.height = arm.util.RenderUtil.matPreviewSize;
				t.format = Inc.getHdrFormat();
				t.scale = Inc.getSuperSampling();
				t.depth_buffer = "mmain";
				path.createRenderTarget(t);
			}

			{
				var t = new RenderTargetRaw();
				t.name = "mbuf";
				t.width = arm.util.RenderUtil.matPreviewSize;
				t.height = arm.util.RenderUtil.matPreviewSize;
				t.format = Inc.getHdrFormat();
				t.scale = Inc.getSuperSampling();
				path.createRenderTarget(t);
			}

			{
				var t = new RenderTargetRaw();
				t.name = "mgbuffer0";
				t.width = arm.util.RenderUtil.matPreviewSize;
				t.height = arm.util.RenderUtil.matPreviewSize;
				t.format = "RGBA64";
				t.scale = Inc.getSuperSampling();
				t.depth_buffer = "mmain";
				path.createRenderTarget(t);
			}

			{
				var t = new RenderTargetRaw();
				t.name = "mgbuffer1";
				t.width = arm.util.RenderUtil.matPreviewSize;
				t.height = arm.util.RenderUtil.matPreviewSize;
				t.format = "RGBA64";
				t.scale = Inc.getSuperSampling();
				path.createRenderTarget(t);
			}

			#if rp_gbuffer2
			{
				var t = new RenderTargetRaw();
				t.name = "mgbuffer2";
				t.width = arm.util.RenderUtil.matPreviewSize;
				t.height = arm.util.RenderUtil.matPreviewSize;
				t.format = "RGBA64";
				t.scale = Inc.getSuperSampling();
				path.createRenderTarget(t);
			}
			#end

			#if ((rp_antialiasing == "SMAA") || (rp_antialiasing == "TAA"))
			{
				var t = new RenderTargetRaw();
				t.name = "mbufa";
				t.width = arm.util.RenderUtil.matPreviewSize;
				t.height = arm.util.RenderUtil.matPreviewSize;
				t.format = "RGBA32";
				t.scale = Inc.getSuperSampling();
				path.createRenderTarget(t);
			}
			{
				var t = new RenderTargetRaw();
				t.name = "mbufb";
				t.width = arm.util.RenderUtil.matPreviewSize;
				t.height = arm.util.RenderUtil.matPreviewSize;
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

		// Only enable vxao in scene mode
		// var restoreGI = false;
		// if (armory.data.Config.raw.rp_gi && UITrait.inst.worktab.position == 0) { // paint
		// 	armory.data.Config.raw.rp_gi = false;
		// 	restoreGI = true;
		// }

		var ssaa4 = armory.data.Config.raw.rp_supersample == 4 ? true : false;

		if (!UITrait.inst.dirty()) {
			path.setTarget("");
			path.bindTarget(taaFrame % 2 == 0 ? "taa" : "taa2", "tex");
			ssaa4 ?
				path.drawShader("shader_datas/supersample_resolve/supersample_resolve") :
				path.drawShader("shader_datas/copy_pass/copy_pass");
			return;
		}

		// Match projection matrix jitter
		@:privateAccess iron.Scene.active.camera.frame = taaFrame;
		@:privateAccess iron.Scene.active.camera.projectionJitter();

		var tid = UITrait.inst.selectedLayer.id;

		if (UITrait.inst.pushUndo) {
			var i = UITrait.inst.undoI;
			path.setTarget("texpaint_undo" + i, ["texpaint_nor_undo" + i, "texpaint_pack_undo" + i]);			
			path.bindTarget("texpaint" + tid, "tex0");
			path.bindTarget("texpaint_nor" + tid, "tex1");
			path.bindTarget("texpaint_pack" + tid, "tex2");
			path.drawShader("shader_datas/copy_mrt3_pass/copy_mrt3_pass");
			UITrait.inst.undoLayers[UITrait.inst.undoI].targetObject = UITrait.inst.paintObject;
			UITrait.inst.undoLayers[UITrait.inst.undoI].targetLayer = UITrait.inst.selectedLayer;
			UITrait.inst.undoI = (UITrait.inst.undoI + 1) % UITrait.inst.C.undo_steps;
			if (UITrait.inst.undos < UITrait.inst.C.undo_steps) UITrait.inst.undos++;
			UITrait.inst.redos = 0;
			UITrait.inst.pushUndo = false;
		}

		// 2D paint
		var painto:iron.object.MeshObject = null;
		var planeo:iron.object.MeshObject = null;
		var visibles:Array<Bool> = null;
		var mergedObjectVisible = false;
		var savedFov = 0.0;
		if (UITrait.inst.paint2d) {
			// Set plane mesh
			painto = UITrait.inst.paintObject;
			visibles = [];
			for (p in UITrait.inst.paintObjects) {
				visibles.push(p.visible);
				p.visible = false;
			}
			if (UITrait.inst.mergedObject != null) {
				mergedObjectVisible = UITrait.inst.mergedObject.visible;
				UITrait.inst.mergedObject.visible = false;
			}

			var cam = iron.Scene.active.camera;
			UITrait.inst.savedCamera.setFrom(cam.transform.local);
			savedFov = cam.data.raw.fov;
			var m = iron.math.Mat4.identity();
			m.translate(0, 0, 0.5);
			cam.transform.setMatrix(m);
			cam.data.raw.fov = 0.92;
			cam.buildProjection();
			cam.buildMatrix();

			var tw = 0.95 * UIView2D.inst.panScale;
			var tx = UIView2D.inst.panX / iron.App.w();
			var ty = UIView2D.inst.panY / iron.App.h();
			
			m.setIdentity();
			m.scale(new iron.math.Vec4(tw, tw, 1));
			m.setLoc(new iron.math.Vec4(tx, ty, 0));
			var m2 = iron.math.Mat4.identity();
			m2.getInverse(iron.Scene.active.camera.VP);
			m.multmat(m2);

			planeo = cast iron.Scene.active.getChild(".Plane");
			planeo.visible = true;
			UITrait.inst.paintObject = planeo;

			var v = new iron.math.Vec4();
			var sx = v.set(m._00, m._01, m._02).length();
			planeo.transform.rot.fromEuler(-Math.PI / 2, 0, 0);
			planeo.transform.scale.set(sx, 1.0, sx);
			planeo.transform.loc.set(m._30, -m._31, 0.0);
			planeo.transform.buildMatrix();
		}

		if (UITrait.inst.depthDirty()) {
			path.setTarget("texpaint" + tid);
			path.clearTarget(null, 1.0);
			path.drawMeshes("depth");
		}

		if (UITrait.inst.paintDirty()) {
			if (UITrait.inst.selectedTool == ToolParticle) {
				path.setTarget("texparticle");
				path.clearTarget(0x00000000);
				path.bindTarget("_paintdb", "paintdb");
				
				var mo:iron.object.MeshObject = cast iron.Scene.active.getChild(".ParticleEmitter");
				mo.visible = true;
				mo.render(path.currentG, "mesh", @:privateAccess path.bindParams);
				mo.visible = false;

				mo = cast iron.Scene.active.getChild(".Particle");
				mo.visible = true;
				mo.render(path.currentG, "mesh", @:privateAccess path.bindParams);
				mo.visible = false;
				@:privateAccess path.end(path.currentG);
			}
			
			if (UITrait.inst.selectedTool == ToolColorId) {
				path.setTarget("texpaint_colorid");
				path.clearTarget(0xff000000);
				path.bindTarget("gbuffer2", "gbuffer2");
				path.drawMeshes("paint");
				UITrait.inst.headerHandle.redraws = 2;
			}
			else if (UITrait.inst.selectedTool == ToolPicker) {
				path.setTarget("texpaint_picker", ["texpaint_nor_picker", "texpaint_pack_picker"]);
				path.clearTarget(0xff000000);
				path.bindTarget("gbuffer2", "gbuffer2");
				tid = UITrait.inst.layers[0].id;
				path.bindTarget("texpaint" + tid, "texpaint");
				path.bindTarget("texpaint_nor" + tid, "texpaint_nor");
				path.bindTarget("texpaint_pack" + tid, "texpaint_pack");
				path.drawMeshes("paint");
				UITrait.inst.headerHandle.redraws = 2;

				var texpaint_picker = path.renderTargets.get("texpaint_picker").image;
				var texpaint_nor_picker = path.renderTargets.get("texpaint_nor_picker").image;
				var texpaint_pack_picker = path.renderTargets.get("texpaint_pack_picker").image;
				var a = texpaint_picker.getPixels();
				var b = texpaint_nor_picker.getPixels();
				var c = texpaint_pack_picker.getPixels();
				// Picked surface values
				UITrait.inst.baseRPicked = a.get(0) / 255;
				UITrait.inst.baseGPicked = a.get(1) / 255;
				UITrait.inst.baseBPicked = a.get(2) / 255;
				UITrait.inst.normalRPicked = b.get(0) / 255;
				UITrait.inst.normalGPicked = b.get(1) / 255;
				UITrait.inst.normalBPicked = b.get(2) / 255;
				UITrait.inst.occlusionPicked = c.get(0) / 255;
				UITrait.inst.roughnessPicked = c.get(1) / 255;
				UITrait.inst.metallicPicked = c.get(2) / 255;
				// Pick material
				if (UITrait.inst.pickerSelectMaterial) {
					var matid = b.get(3);
					for (m in UITrait.inst.materials) {
						if (m.id == matid) {
							UITrait.inst.setMaterial(m);
							UITrait.inst.materialIdPicked = matid;
							break;
						}
					}
				}
			}
			else {
				if (UITrait.inst.selectedTool == ToolBake) {
					if (initVoxels) {
						initVoxels = false;
						// Init voxel texture
						var rp_gi = UITrait.inst.C.rp_gi;
						UITrait.inst.C.rp_gi = true;
						#if rp_voxelao
						Inc.initGI();
						#end
						UITrait.inst.C.rp_gi = rp_gi;
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

				var blendA = "texpaint_blend0";
				var blendB = "texpaint_blend1";
				path.setTarget(blendB);
				path.bindTarget(blendA, "tex");
				path.drawShader("shader_datas/copy_pass/copy_pass");
				var isMask = UITrait.inst.selectedLayerIsMask;
				var texpaint = isMask ? "texpaint_mask" + tid : "texpaint" + tid;
				path.setTarget(texpaint, ["texpaint_nor" + tid, "texpaint_pack" + tid, blendA]);
				path.bindTarget("_paintdb", "paintdb");
				path.bindTarget(blendB, "paintmask");
				if (UITrait.inst.selectedTool == ToolBake) {
					path.bindTarget("voxels", "voxels");
				}
				if (UITrait.inst.colorIdPicked) {
					path.bindTarget("texpaint_colorid", "texpaint_colorid");
				} 

				// Read texcoords from gbuffer
				var readTC = (UITrait.inst.selectedTool == ToolFill && UITrait.inst.fillTypeHandle.position == 1) || // Face fill
							  UITrait.inst.selectedTool == ToolClone ||
							  UITrait.inst.selectedTool == ToolBlur;
				if (readTC) {
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

		if (UITrait.inst.brushBlendDirty) {
			UITrait.inst.brushBlendDirty = false;
			path.setTarget("texpaint_blend0", ["texpaint_blend1"]);
			path.clearTarget(0x00000000);
		}

		if (UITrait.inst.paint2d) {
			// Restore paint mesh
			planeo.visible = false;
			for (i in 0...UITrait.inst.paintObjects.length) {
				UITrait.inst.paintObjects[i].visible = visibles[i];
			}
			if (UITrait.inst.mergedObject != null) {
				UITrait.inst.mergedObject.visible = mergedObjectVisible;
			}
			UITrait.inst.paintObject = painto;
			iron.Scene.active.camera.transform.setMatrix(UITrait.inst.savedCamera);
			iron.Scene.active.camera.data.raw.fov = savedFov;
			iron.Scene.active.camera.buildProjection();
			iron.Scene.active.camera.buildMatrix();
		}

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
		tid = UITrait.inst.layers[0].id;
		path.bindTarget("texpaint" + tid, "texpaint");
		path.bindTarget("texpaint_nor" + tid, "texpaint_nor");
		path.bindTarget("texpaint_pack" + tid, "texpaint_pack");
		for (i in 1...UITrait.inst.layers.length) {
			var l = UITrait.inst.layers[i];
			tid = l.id;
			path.bindTarget("texpaint" + tid, "texpaint" + tid);
			path.bindTarget("texpaint_nor" + tid, "texpaint_nor" + tid);
			path.bindTarget("texpaint_pack" + tid, "texpaint_pack" + tid);
			if (l.texpaint_mask != null) {
				path.bindTarget("texpaint_mask" + tid, "texpaint_mask" + tid);
			}
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
		#if rp_voxelao
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
		#if rp_voxelao
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

			if (UITrait.inst.showCompass) {
				var scene = iron.Scene.active;
				var cam = iron.Scene.active.camera;
				var gizmo:iron.object.MeshObject = cast scene.getChild(".GizmoTranslate");
				
				var visible = gizmo.visible;
				var parent = gizmo.parent;
				var loc = gizmo.transform.loc;
				var rot = gizmo.transform.rot;
				var crot = cam.transform.rot;
				var ratio = iron.App.w() / iron.App.h();
				var P = cam.P;
				cam.P = iron.math.Mat4.ortho(-8 * ratio, 8 * ratio, -8, 8, -2, 2);
				gizmo.visible = true;
				gizmo.parent = cam;
				gizmo.transform.loc = new iron.math.Vec4(7.2 * ratio, -7.6, -1);
				gizmo.transform.rot = new iron.math.Quat(-crot.x, -crot.y, -crot.z, crot.w);
				gizmo.transform.buildMatrix();
				
				gizmo.render(path.currentG, "overlay", []);
				
				cam.P = P;
				gizmo.visible = visible;
				gizmo.parent = parent;
				gizmo.transform.loc = loc;
				gizmo.transform.rot = rot;
				gizmo.transform.buildMatrix();
			}
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
		UITrait.inst.ddirty--;
		UITrait.inst.pdirty--;
		UITrait.inst.rdirty--;

		// if (restoreGI) armory.data.Config.raw.rp_gi = true;
	}

	#end
}
