package arm.renderpath;

import iron.RenderPath;
import armory.renderpath.Inc;
import arm.util.ViewportUtil;
import arm.ui.UITrait;
import arm.ui.UIView2D;

class RenderPathDeferred {

	#if (rp_renderer == "Deferred")

	public static var path:RenderPath;

	#if rp_voxelao
	static var voxels = "voxels";
	static var voxelsLast = "voxels";
	#end
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
				var t = new RenderTargetRaw();
				t.name = "texpreview_icon";
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

	static var lastX = -1.0;
	static var lastY = -1.0;

	@:access(iron.RenderPath)
	public static function commands() {

		if (kha.System.windowWidth() == 0 || kha.System.windowHeight() == 0) return;

		var ssaa4 = armory.data.Config.raw.rp_supersample == 4 ? true : false;
		
		var mouse = iron.system.Input.getMouse();
		var mx = lastX;
		var my = lastY;
		lastX = mouse.x;
		lastY = mouse.y;

		if (!UITrait.inst.dirty()) {
			if (mx != lastX || my != lastY || UITrait.inst.ddirty == 0) {
				UITrait.inst.ddirty--;
				path.setTarget("");
				path.bindTarget(taaFrame % 2 == 0 ? "taa" : "taa2", "tex");
				ssaa4 ?
					path.drawShader("shader_datas/supersample_resolve/supersample_resolve") :
					path.drawShader("shader_datas/copy_pass/copy_pass");
				if (UITrait.inst.brush3d) RenderPathPaint.commandsCursor();
			}
			return;
		}

		// Match projection matrix jitter
		@:privateAccess iron.Scene.active.camera.frame = taaFrame;
		@:privateAccess iron.Scene.active.camera.projectionJitter();

		var tid = UITrait.inst.selectedLayer.id;

		if (UITrait.inst.pushUndo && UITrait.inst.undoLayers != null) {
			var isMask = UITrait.inst.selectedLayerIsMask;
			var i = UITrait.inst.undoI;
			if (isMask) {
				path.setTarget("texpaint_mask_undo" + i);
				path.bindTarget("texpaint_mask" + tid, "tex");
				path.drawShader("shader_datas/copy_pass/copy_pass");
			}
			else {
				path.setTarget("texpaint_undo" + i, ["texpaint_nor_undo" + i, "texpaint_pack_undo" + i]);
				path.bindTarget((isMask ? "texpaint_mask" : "texpaint") + tid, "tex0");
				path.bindTarget("texpaint_nor" + tid, "tex1");
				path.bindTarget("texpaint_pack" + tid, "tex2");
				path.drawShader("shader_datas/copy_mrt3_pass/copy_mrt3_pass");
			}
			var undoLayer = UITrait.inst.undoLayers[UITrait.inst.undoI];
			undoLayer.targetObject = UITrait.inst.paintObject;
			undoLayer.targetLayer = UITrait.inst.selectedLayer;
			undoLayer.targetIsMask = isMask;
			UITrait.inst.undoI = (UITrait.inst.undoI + 1) % App.C.undo_steps;
			if (UITrait.inst.undos < App.C.undo_steps) UITrait.inst.undos++;
			if (UITrait.inst.redos > 0) {
				for (i in 0...UITrait.inst.redos) History.stack.pop();
				UITrait.inst.redos = 0;
			}
			UITrait.inst.pushUndo = false;

			History.stack.push(UITrait.inst.toolNames[UITrait.inst.selectedTool]);
			while (History.stack.length > App.C.undo_steps + 1) History.stack.shift();
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
			ViewportUtil.updateCameraType(0);
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

		if (UITrait.inst.undoLayers != null) {

			// Symmetry
			if (UITrait.inst.symX || UITrait.inst.symY || UITrait.inst.symZ) {
				UITrait.inst.ddirty = 2;
				var t = UITrait.inst.paintObject.transform;
				var sx = t.scale.x;
				var sy = t.scale.y;
				var sz = t.scale.z;
				if (UITrait.inst.symX) {
					t.scale.set(-sx, sy, sz);
					t.buildMatrix();
					RenderPathPaint.commandsPaint();
				}
				if (UITrait.inst.symY) {
					t.scale.set(sx, -sy, sz);
					t.buildMatrix();
					RenderPathPaint.commandsPaint();
				}
				if (UITrait.inst.symZ) {
					t.scale.set(sx, sy, -sz);
					t.buildMatrix();
					RenderPathPaint.commandsPaint();
				}
				if (UITrait.inst.symX && UITrait.inst.symY) {
					t.scale.set(-sx, -sy, sz);
					t.buildMatrix();
					RenderPathPaint.commandsPaint();
				}
				if (UITrait.inst.symX && UITrait.inst.symZ) {
					t.scale.set(-sx, sy, -sz);
					t.buildMatrix();
					RenderPathPaint.commandsPaint();
				}
				if (UITrait.inst.symY && UITrait.inst.symZ) {
					t.scale.set(sx, -sy, -sz);
					t.buildMatrix();
					RenderPathPaint.commandsPaint();
				}
				if (UITrait.inst.symX && UITrait.inst.symY && UITrait.inst.symZ) {
					t.scale.set(-sx, -sy, -sz);
					t.buildMatrix();
					RenderPathPaint.commandsPaint();
				}
				t.scale.set(sx, sy, sz);
				t.buildMatrix();
			}

			RenderPathPaint.commandsPaint();
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
			ViewportUtil.updateCameraType(UITrait.inst.cameraType);
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
			var ssgi = armory.data.Config.raw.rp_ssgi != false && UITrait.inst.cameraType == 0;
			if (ssgi) {
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
			var ssgi = armory.data.Config.raw.rp_ssgi != false && UITrait.inst.cameraType == 0;
			if (ssgi) {
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
			path.setTarget("buf");
			path.clearTarget(null, 1.0);
			var currentG = path.currentG;
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
				
				gizmo.render(currentG, "overlay", []);
				
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
					if (taaFrame == 0) path.bindTarget("bufa", "tex");
					else path.bindTarget(taaFrame % 2 == 0 ? "taa2" : "taa", "tex");
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

		if (UITrait.inst.brush3d) RenderPathPaint.commandsCursor();

		taaFrame++;
		UITrait.inst.ddirty--;
		UITrait.inst.pdirty--;
		UITrait.inst.rdirty--;
	}

	#end
}
