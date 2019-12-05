package arm.render;

import kha.System;
import iron.RenderPath;
import iron.Scene;
import iron.math.Mat4;
import iron.math.Vec4;
import iron.math.Quat;
import iron.system.Input;
import iron.object.MeshObject;
#if arm_painter
import arm.util.ViewportUtil;
import arm.util.RenderUtil;
import arm.ui.UITrait;
import arm.ui.UIView2D;
import arm.node.MaterialParser;
import arm.Tool;
#end

class RenderPathDeferred {

	public static var path:RenderPath;

	#if rp_voxelao
	static var voxels = "voxels";
	static var voxelsLast = "voxels";
	public static var voxelFrame = 0;
	public static var voxelFreq = 6; // Revoxelizing frequency
	#end
	static var taaFrame = 0;
	static var lastX = -1.0;
	static var lastY = -1.0;

	public static function init(_path:RenderPath) {

		path = _path;

		path.loadShader("world_pass/world_pass/world_pass");

		#if rp_voxelao
		{
			Inc.initGI();
			path.loadShader("deferred_light/deferred_light/deferred_light_voxel");
		}
		#end

		{
			path.createDepthBuffer("main", "DEPTH24");

			var t = new RenderTargetRaw();
			t.name = "gbuffer0";
			t.width = 0;
			t.height = 0;
			t.format = "RGBA64";
			t.scale = Inc.getSuperSampling();
			t.depth_buffer = "main";
			path.createRenderTarget(t);
		}

		{
			var t = new RenderTargetRaw();
			t.name = "tex";
			t.width = 0;
			t.height = 0;
			t.format = Inc.getHdrFormat();
			t.scale = Inc.getSuperSampling();
			t.depth_buffer = "main";
			path.createRenderTarget(t);
		}

		{
			var t = new RenderTargetRaw();
			t.name = "buf";
			t.width = 0;
			t.height = 0;
			#if kha_direct3d12 // Match raytrace_target format
			t.format = "RGBA128";
			#else
			t.format = Inc.getHdrFormat();
			#end
			t.scale = Inc.getSuperSampling();
			path.createRenderTarget(t);
		}

		{
			var t = new RenderTargetRaw();
			t.name = "gbuffer1";
			t.width = 0;
			t.height = 0;
			t.format = "RGBA64";
			t.scale = Inc.getSuperSampling();
			path.createRenderTarget(t);
		}

		{
			var t = new RenderTargetRaw();
			t.name = "gbuffer2";
			t.width = 0;
			t.height = 0;
			t.format = "RGBA64";
			t.scale = Inc.getSuperSampling();
			path.createRenderTarget(t);
		}

		{
			var t = new RenderTargetRaw();
			t.name = "taa";
			t.width = 0;
			t.height = 0;
			t.format = "RGBA32";
			t.scale = Inc.getSuperSampling();
			path.createRenderTarget(t);
		}

		path.loadShader("deferred_light/deferred_light/deferred_light");

		{
			path.loadShader("shader_datas/ssgi_pass/ssgi_pass");
			path.loadShader("shader_datas/blur_edge_pass/blur_edge_pass_x");
			path.loadShader("shader_datas/blur_edge_pass/blur_edge_pass_y");
		}
		{
			var t = new RenderTargetRaw();
			t.name = "singlea";
			t.width = 0;
			t.height = 0;
			t.format = "R8";
			t.scale = Inc.getSuperSampling();
			path.createRenderTarget(t);
		}
		{
			var t = new RenderTargetRaw();
			t.name = "singleb";
			t.width = 0;
			t.height = 0;
			t.format = "R8";
			t.scale = Inc.getSuperSampling();
			path.createRenderTarget(t);
		}

		{
			var t = new RenderTargetRaw();
			t.name = "bufa";
			t.width = 0;
			t.height = 0;
			t.format = "RGBA32";
			t.scale = Inc.getSuperSampling();
			path.createRenderTarget(t);
		}
		{
			var t = new RenderTargetRaw();
			t.name = "bufb";
			t.width = 0;
			t.height = 0;
			t.format = "RGBA32";
			t.scale = Inc.getSuperSampling();
			path.createRenderTarget(t);
		}

		{
			path.loadShader("shader_datas/compositor_pass/compositor_pass");
		}

		{
			path.loadShader("shader_datas/copy_pass/copy_pass");
		}

		{
			path.loadShader("shader_datas/smaa_edge_detect/smaa_edge_detect");
			path.loadShader("shader_datas/smaa_blend_weight/smaa_blend_weight");
			path.loadShader("shader_datas/smaa_neighborhood_blend/smaa_neighborhood_blend");
			path.loadShader("shader_datas/taa_pass/taa_pass");
		}

		{
			path.loadShader("shader_datas/supersample_resolve/supersample_resolve");
		}

		#if arm_world
		{
			path.loadShader("water_pass/water_pass/water_pass");
			path.loadShader("shader_datas/copy_pass/copy_pass");
			Scene.active.embedData("water_base.png", function() {});
			Scene.active.embedData("water_detail.png", function() {});
			Scene.active.embedData("water_foam.png", function() {});
			Scene.active.embedData("water_foam.png", function() {});
			Scene.active.embedData("clouds_base.raw", function() {});
			Scene.active.embedData("clouds_detail.raw", function() {});
			Scene.active.embedData("clouds_map.png", function() {});
		}
		#end

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

		#if rp_autoexposure
		{
			var t = new RenderTargetRaw();
			t.name = "histogram";
			t.width = 1;
			t.height = 1;
			t.format = Inc.getHdrFormat();
			path.createRenderTarget(t);
		}

		{
			path.loadShader("shader_datas/histogram_pass/histogram_pass");
		}
		#end

		{
			path.loadShader("shader_datas/ssr_pass/ssr_pass");
			path.loadShader("shader_datas/blur_adaptive_pass/blur_adaptive_pass_x");
			path.loadShader("shader_datas/blur_adaptive_pass/blur_adaptive_pass_y3_blend");
		}

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

		{
			var t = new RenderTargetRaw();
			t.name = "taa2";
			t.width = 0;
			t.height = 0;
			t.format = "RGBA32";
			t.scale = Inc.getSuperSampling();
			path.createRenderTarget(t);
		}

		#if arm_painter
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
				t.width = RenderUtil.matPreviewSize;
				t.height = RenderUtil.matPreviewSize;
				t.format = Inc.getHdrFormat();
				t.scale = Inc.getSuperSampling();
				t.depth_buffer = "mmain";
				path.createRenderTarget(t);
			}

			{
				var t = new RenderTargetRaw();
				t.name = "mbuf";
				t.width = RenderUtil.matPreviewSize;
				t.height = RenderUtil.matPreviewSize;
				t.format = Inc.getHdrFormat();
				t.scale = Inc.getSuperSampling();
				path.createRenderTarget(t);
			}

			{
				var t = new RenderTargetRaw();
				t.name = "mgbuffer0";
				t.width = RenderUtil.matPreviewSize;
				t.height = RenderUtil.matPreviewSize;
				t.format = "RGBA64";
				t.scale = Inc.getSuperSampling();
				t.depth_buffer = "mmain";
				path.createRenderTarget(t);
			}

			{
				var t = new RenderTargetRaw();
				t.name = "mgbuffer1";
				t.width = RenderUtil.matPreviewSize;
				t.height = RenderUtil.matPreviewSize;
				t.format = "RGBA64";
				t.scale = Inc.getSuperSampling();
				path.createRenderTarget(t);
			}

			{
				var t = new RenderTargetRaw();
				t.name = "mgbuffer2";
				t.width = RenderUtil.matPreviewSize;
				t.height = RenderUtil.matPreviewSize;
				t.format = "RGBA64";
				t.scale = Inc.getSuperSampling();
				path.createRenderTarget(t);
			}

			{
				var t = new RenderTargetRaw();
				t.name = "mbufa";
				t.width = RenderUtil.matPreviewSize;
				t.height = RenderUtil.matPreviewSize;
				t.format = "RGBA32";
				t.scale = Inc.getSuperSampling();
				path.createRenderTarget(t);
			}
			{
				var t = new RenderTargetRaw();
				t.name = "mbufb";
				t.width = RenderUtil.matPreviewSize;
				t.height = RenderUtil.matPreviewSize;
				t.format = "RGBA32";
				t.scale = Inc.getSuperSampling();
				path.createRenderTarget(t);
			}
		}

		#end // arm_painter
	}

	static inline function ssaa4():Bool { return Config.raw.rp_supersample == 4; }

	@:access(iron.RenderPath)
	public static function commands() {

		if (System.windowWidth() == 0 || System.windowHeight() == 0) return;

		#if arm_painter

		if (UITrait.inst.splitView) {

			if (UITrait.inst.viewIndexLast == -1 && UITrait.inst.viewIndex == -1) {
				// Begin split, draw right viewport first
				UITrait.inst.viewIndex = 1;
			}
			else {
				// Set current viewport
				UITrait.inst.viewIndex = Input.getMouse().viewX > arm.App.w() / 2 ? 1 : 0;
			}

			var cam = Scene.active.camera;
			if (UITrait.inst.viewIndexLast > -1) {
				// Save current viewport camera
				arm.plugin.Camera.inst.views[UITrait.inst.viewIndexLast].setFrom(cam.transform.local);
			}

			if (UITrait.inst.viewIndexLast != UITrait.inst.viewIndex) {
				// Redraw on current viewport change
				Context.ddirty = 1;
			}

			cam.transform.setMatrix(arm.plugin.Camera.inst.views[UITrait.inst.viewIndex]);
			cam.buildMatrix();
			cam.buildProjection();
		}

		var mouse = Input.getMouse();
		var mx = lastX;
		var my = lastY;
		lastX = mouse.viewX;
		lastY = mouse.viewY;

		#if (!arm_creator)
		if (Context.ddirty <= 0 && Context.rdirty <= 0 && (Context.pdirty <= 0 || UITrait.inst.worktab.position == SpaceScene)) {
			if (mx != lastX || my != lastY || mouse.locked) Context.ddirty = 0;
			if (Context.ddirty > -2) {
				path.setTarget("");
				path.bindTarget("taa", "tex");
				ssaa4() ?
					path.drawShader("shader_datas/supersample_resolve/supersample_resolve") :
					path.drawShader("shader_datas/copy_pass/copy_pass");
				if (UITrait.inst.brush3d) RenderPathPaint.commandsCursor();
				if (Context.ddirty <= 0) Context.ddirty--;
			}
			endSplit();
			return;
		}
		#end

		// Match projection matrix jitter
		var skipTaa = UITrait.inst.splitView;
		if (!skipTaa) {
			@:privateAccess Scene.active.camera.frame = taaFrame;
			@:privateAccess Scene.active.camera.projectionJitter();
		}
		Scene.active.camera.buildMatrix();

		var pushUndoLast = History.pushUndo;
		if (History.pushUndo && History.undoLayers != null) {
			History.paint();
		}

		// 2D paint
		var painto:MeshObject = null;
		var planeo:MeshObject = null;
		var visibles:Array<Bool> = null;
		var mergedObjectVisible = false;
		var savedFov = 0.0;
		if (UITrait.inst.paint2d) {
			// Set plane mesh
			painto = Context.paintObject;
			visibles = [];
			for (p in Project.paintObjects) {
				visibles.push(p.visible);
				p.visible = false;
			}
			if (Context.mergedObject != null) {
				mergedObjectVisible = Context.mergedObject.visible;
				Context.mergedObject.visible = false;
			}

			var cam = Scene.active.camera;
			UITrait.inst.savedCamera.setFrom(cam.transform.local);
			savedFov = cam.data.raw.fov;
			ViewportUtil.updateCameraType(0);
			var m = Mat4.identity();
			m.translate(0, 0, 0.5);
			cam.transform.setMatrix(m);
			cam.data.raw.fov = 0.92;
			cam.buildProjection();
			cam.buildMatrix();

			var tw = 0.95 * UIView2D.inst.panScale;
			var tx = UIView2D.inst.panX / iron.App.w();
			var ty = UIView2D.inst.panY / iron.App.h();

			m.setIdentity();
			m.scale(new Vec4(tw, tw, 1));
			m.setLoc(new Vec4(tx, ty, 0));
			var m2 = Mat4.identity();
			m2.getInverse(Scene.active.camera.VP);
			m.multmat(m2);

			planeo = cast Scene.active.getChild(".Plane");
			planeo.visible = true;
			Context.paintObject = planeo;

			var v = new Vec4();
			var sx = v.set(m._00, m._01, m._02).length();
			planeo.transform.rot.fromEuler(-Math.PI / 2, 0, 0);
			planeo.transform.scale.set(sx, 1.0, sx);
			planeo.transform.loc.set(m._30, -m._31, 0.0);
			planeo.transform.buildMatrix();
		}

		if (UITrait.inst.splitView) {
			if (Context.pdirty > 0) {
				var cam = Scene.active.camera;

				UITrait.inst.viewIndex = UITrait.inst.viewIndex == 0 ? 1 : 0;
				cam.transform.setMatrix(arm.plugin.Camera.inst.views[UITrait.inst.viewIndex]);
				cam.buildMatrix();
				cam.buildProjection();

				drawGbuffer();

				#if kha_direct3d12
				UITrait.inst.viewportMode == 10 ? drawRaytraced() : drawDeferred;
				#else
				drawDeferred();
				#end

				UITrait.inst.viewIndex = UITrait.inst.viewIndex == 0 ? 1 : 0;
				cam.transform.setMatrix(arm.plugin.Camera.inst.views[UITrait.inst.viewIndex]);
				cam.buildMatrix();
				cam.buildProjection();
			}
		}

		#end // arm_painter

		// Geometry
		drawGbuffer();

		#if arm_painter

		if (History.undoLayers != null) {

			// Symmetry
			if (UITrait.inst.symX || UITrait.inst.symY || UITrait.inst.symZ) {
				Context.ddirty = 2;
				var t = Context.paintObject.transform;
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

			if (Context.tool == ToolBake) {
				if (UITrait.inst.bakeType == 2) { // Normal (Tangent)
					UITrait.inst.bakeType = 3; // Bake high poly world normals
					MaterialParser.parsePaintMaterial();
					var _paintObject = Context.paintObject;
					var highPoly = Project.paintObjects[UITrait.inst.bakeHighPoly];
					var _visible = highPoly.visible;
					highPoly.visible = true;
					Context.selectPaintObject(highPoly);
					RenderPathPaint.commandsPaint();
					highPoly.visible = _visible;
					UITrait.inst.sub--;
					if (pushUndoLast) History.paint();

					UITrait.inst.bakeType = 2;
					MaterialParser.parsePaintMaterial();
					Context.selectPaintObject(_paintObject);
					RenderPathPaint.commandsPaint();
				}
				else if (UITrait.inst.bakeType == 7) { // Object ID
					var _layerFilter = UITrait.inst.layerFilter;
					var _paintObject = Context.paintObject;
					var isMerged = Context.mergedObject != null;
					var _visible = isMerged && Context.mergedObject.visible;
					UITrait.inst.layerFilter = 1;
					if (isMerged) Context.mergedObject.visible = false;

					for (p in Project.paintObjects) {
						Context.selectPaintObject(p);
						RenderPathPaint.commandsPaint();
					}

					UITrait.inst.layerFilter = _layerFilter;
					Context.selectPaintObject(_paintObject);
					if (isMerged) Context.mergedObject.visible = _visible;
				}
				#if kha_direct3d12
				else if (UITrait.inst.bakeType == 0 || // AO (DXR)
						 UITrait.inst.bakeType == 8 || // Lightmap (DXR)
						 UITrait.inst.bakeType == 9) { // Bent Normal (DXR)
					RenderPathRaytrace.commandsBake();
				}
				#end
				else {
					RenderPathPaint.commandsPaint();
				}
			}
			else { // Paint
				RenderPathPaint.commandsPaint();
			}
		}

		//

		if (Context.brushBlendDirty) {
			Context.brushBlendDirty = false;
			path.setTarget("texpaint_blend0", ["texpaint_blend1"]);
			path.clearTarget(0x00000000);
		}

		if (UITrait.inst.paint2d) {
			// Restore paint mesh
			planeo.visible = false;
			for (i in 0...Project.paintObjects.length) {
				Project.paintObjects[i].visible = visibles[i];
			}
			if (Context.mergedObject != null) {
				Context.mergedObject.visible = mergedObjectVisible;
			}
			Context.paintObject = painto;
			Scene.active.camera.transform.setMatrix(UITrait.inst.savedCamera);
			Scene.active.camera.data.raw.fov = savedFov;
			ViewportUtil.updateCameraType(UITrait.inst.cameraType);
			Scene.active.camera.buildProjection();
			Scene.active.camera.buildMatrix();

			drawGbuffer();
		}

		#if kha_direct3d12
		if (UITrait.inst.viewportMode == 10) { // Ray-traced
			drawRaytraced();
			return;
		}
		#end

		#end // arm_painter

		drawDeferred();

		#if arm_painter
		if (UITrait.inst.brush3d) RenderPathPaint.commandsCursor();
		Context.ddirty--;
		Context.pdirty--;
		Context.rdirty--;
		endSplit();
		#end

		taaFrame++;
	}

	static function drawDeferred() {
		#if arm_painter
		var cameraType = UITrait.inst.cameraType;
		var ddirty = Context.ddirty;
		#else
		var cameraType = 0;
		var ddirty = 2;
		#end

		var ssgi = Config.raw.rp_ssgi != false && cameraType == 0;
		if (ssgi && ddirty > 0 && taaFrame > 0) {
			path.setTarget("singlea");
			path.bindTarget("_main", "gbufferD");
			path.bindTarget("gbuffer0", "gbuffer0");
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

		// Voxels
		#if rp_voxelao
		if (Config.raw.rp_gi != false)
		{
			var voxelize = path.voxelize() && ddirty > 0 && taaFrame > 0;

			#if arm_voxelgi_temporal
			voxelize = ++voxelFrame % voxelFreq == 0;

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
				#if arm_painter
				if (arm.node.MaterialBuilder.heightUsed) {
					var tid = Project.layers[0].id;
					path.bindTarget("texpaint_pack" + tid, "texpaint_pack");
				}
				#end
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
		var ssgi = Config.raw.rp_ssgi != false && cameraType == 0;
		if (ssgi && taaFrame > 0) {
			path.bindTarget("singlea", "ssaotex");
		}
		else {
			path.bindTarget("empty_white", "ssaotex");
		}
		var voxelao_pass = false;
		#if rp_voxelao
		if (Config.raw.rp_gi != false)
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
			path.drawShader("deferred_light/deferred_light/deferred_light_voxel") :
			path.drawShader("deferred_light/deferred_light/deferred_light");

		#if arm_world
		{
			#if arm_creator
			var waterPass = Project.waterPass;
			#else
			var waterPass = true;
			#end

			if (waterPass) {
				path.setTarget("buf");
				path.bindTarget("tex", "tex");
				path.drawShader("shader_datas/copy_pass/copy_pass");
				path.setTarget("tex");
				path.bindTarget("_main", "gbufferD");
				path.bindTarget("buf", "tex");
				path.drawShader("water_pass/water_pass/water_pass");
			}
		}
		#end

		#if (!kha_opengl)
		path.setDepthFrom("tex", "gbuffer0"); // Re-bind depth
		#end

		path.setTarget("tex"); // Re-binds depth
		path.drawSkydome("world_pass/world_pass/world_pass");

		#if rp_blending
		{
			path.drawMeshes("blend");
		}
		#end

		#if rp_translucency
		{
			var hasLight = Scene.active.lights.length > 0;
			if (hasLight) Inc.drawTranslucency("tex");
		}
		#end

		if (Config.raw.rp_bloom != false) {
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

		if (Config.raw.rp_ssr != false) {
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

		#if ((rp_motionblur == "Camera") || (rp_motionblur == "Object"))
		{
			if (Config.raw.rp_motionblur != false) {
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

		path.setTarget("buf");

		path.bindTarget("tex", "tex");
		#if rp_compositordepth
		{
			path.bindTarget("_main", "gbufferD");
		}
		#end
		path.drawShader("shader_datas/compositor_pass/compositor_pass");
		// End compositor

		{
			path.setTarget("buf");
			var currentG = path.currentG;
			path.drawMeshes("overlay");
			#if arm_painter
			drawCompass(currentG);
			#end
		}

		{
			var current = taaFrame % 2 == 0 ? "bufa" : "taa2";
			var last = taaFrame % 2 == 0 ? "taa2" : "bufa";

			path.setTarget(current);
			path.clearTarget(0x00000000);
			path.bindTarget("buf", "colorTex");
			path.drawShader("shader_datas/smaa_edge_detect/smaa_edge_detect");

			path.setTarget("bufb");
			path.clearTarget(0x00000000);
			path.bindTarget(current, "edgesTex");
			path.drawShader("shader_datas/smaa_blend_weight/smaa_blend_weight");

			path.setTarget(current);
			path.bindTarget("buf", "colorTex");
			path.bindTarget("bufb", "blendTex");
			path.bindTarget("gbuffer2", "sveloc");
			path.drawShader("shader_datas/smaa_neighborhood_blend/smaa_neighborhood_blend");

			#if arm_painter
			var skipTaa = UITrait.inst.splitView;
			#else
			var skipTaa = false;
			#end

			if (skipTaa) {
				path.setTarget("taa");
				path.bindTarget(current, "tex");
				path.drawShader("shader_datas/copy_pass/copy_pass");
			}
			else {
				path.setTarget("taa");
				path.bindTarget(current, "tex");
				path.bindTarget(last, "tex2");
				path.bindTarget("gbuffer2", "sveloc");
				path.drawShader("shader_datas/taa_pass/taa_pass");
			}

			if (!ssaa4()) {
				path.setTarget("");
				path.bindTarget(taaFrame == 0 ? current : "taa", "tex");
				path.drawShader("shader_datas/copy_pass/copy_pass");
			}
		}

		if (ssaa4()) {
			path.setTarget("");
			path.bindTarget(taaFrame % 2 == 0 ? "taa2" : "taa", "tex");
			path.drawShader("shader_datas/supersample_resolve/supersample_resolve");
		}
	}

	#if kha_direct3d12
	static function drawRaytraced() {
		#if arm_painter
		if (Context.ddirty > 1) {
			RenderPathRaytrace.frame = 0;
		}
		#else
		RenderPathRaytrace.frame = 0;
		#end
		RenderPathRaytrace.commands();
		path.setTarget("buf");
		drawCompass(path.currentG);
		path.setTarget("taa");
		path.bindTarget("buf", "tex");
		path.drawShader("shader_datas/compositor_pass/compositor_pass");
		path.setTarget("");
		path.bindTarget("taa", "tex");
		path.drawShader("shader_datas/copy_pass/copy_pass");
		#if arm_painter
		if (UITrait.inst.brush3d) {
			RenderPathPaint.commandsCursor();
		}
		#end
	}
	#end

	static function drawGbuffer() {
		path.setTarget("gbuffer0"); // Only clear gbuffer0
		path.clearTarget(null, 1.0);
		path.setTarget("gbuffer2");
		path.clearTarget(0xff000000);
		path.setTarget("gbuffer0", ["gbuffer1", "gbuffer2"]);

		#if arm_painter
		var tid = Project.layers[0].id;
		path.bindTarget("texpaint" + tid, "texpaint");
		path.bindTarget("texpaint_nor" + tid, "texpaint_nor");
		path.bindTarget("texpaint_pack" + tid, "texpaint_pack");
		for (i in 1...Project.layers.length) {
			var l = Project.layers[i];
			tid = l.id;
			path.bindTarget("texpaint" + tid, "texpaint" + tid);
			path.bindTarget("texpaint_nor" + tid, "texpaint_nor" + tid);
			path.bindTarget("texpaint_pack" + tid, "texpaint_pack" + tid);
			if (l.texpaint_mask != null) {
				path.bindTarget("texpaint_mask" + tid, "texpaint_mask" + tid);
			}
		}
		#end

		path.drawMeshes("mesh");

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
	}

	#if arm_painter
	static function drawCompass(currentG:kha.graphics4.Graphics) {
		if (UITrait.inst.showCompass) {
			var scene = Scene.active;
			var cam = Scene.active.camera;
			var gizmo:MeshObject = cast scene.getChild(".GizmoTranslate");

			var visible = gizmo.visible;
			var parent = gizmo.parent;
			var loc = gizmo.transform.loc;
			var rot = gizmo.transform.rot;
			var crot = cam.transform.rot;
			var ratio = iron.App.w() / iron.App.h();
			var P = cam.P;
			cam.P = Mat4.ortho(-8 * ratio, 8 * ratio, -8, 8, -2, 2);
			gizmo.visible = true;
			gizmo.parent = cam;
			gizmo.transform.loc = new Vec4(7.2 * ratio, -7.6, -1);
			gizmo.transform.rot = new Quat(-crot.x, -crot.y, -crot.z, crot.w);
			gizmo.transform.scale.set(0.5, 0.5, 0.5);
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

	static function endSplit() {
		UITrait.inst.viewIndexLast = UITrait.inst.viewIndex;
		UITrait.inst.viewIndex = -1;
	}
	#end
}
