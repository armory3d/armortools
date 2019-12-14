package arm.render;

import iron.math.Mat4;
import iron.math.Vec4;
import iron.system.Input;
import iron.object.MeshObject;
import iron.RenderPath;
import iron.Scene;
import arm.util.ViewportUtil;
import arm.ui.UITrait;
import arm.ui.UIView2D;
import arm.node.MaterialParser;
import arm.Tool;

#if arm_painter

class RenderPathPaint {

	static var path: RenderPath;
	static var initVoxels = true; // Bake AO
	static var pushUndoLast: Bool;
	static var painto: MeshObject = null;
	static var planeo: MeshObject = null;
	static var visibles: Array<Bool> = null;
	static var mergedObjectVisible = false;
	static var savedFov = 0.0;
	static var dilated = true;
	static var baking = false;

	public static function init(_path: RenderPath) {
		path = _path;

		{
			var t = new RenderTargetRaw();
			t.name = "texpaint_blend0";
			t.width = Config.getTextureRes();
			t.height = Config.getTextureRes();
			t.format = "R8";
			path.createRenderTarget(t);
		}
		{
			var t = new RenderTargetRaw();
			t.name = "texpaint_blend1";
			t.width = Config.getTextureRes();
			t.height = Config.getTextureRes();
			t.format = "R8";
			path.createRenderTarget(t);
		}
		{
			var t = new RenderTargetRaw();
			t.name = "texpaint_colorid";
			t.width = 1;
			t.height = 1;
			t.format = "RGBA32";
			path.createRenderTarget(t);
		}
		{
			var t = new RenderTargetRaw();
			t.name = "texpaint_picker";
			t.width = 1;
			t.height = 1;
			t.format = "RGBA32";
			path.createRenderTarget(t);
		}
		{
			var t = new RenderTargetRaw();
			t.name = "texpaint_nor_picker";
			t.width = 1;
			t.height = 1;
			t.format = "RGBA32";
			path.createRenderTarget(t);
		}
		{
			var t = new RenderTargetRaw();
			t.name = "texpaint_pack_picker";
			t.width = 1;
			t.height = 1;
			t.format = "RGBA32";
			path.createRenderTarget(t);
		}

		path.loadShader("shader_datas/copy_mrt3_pass/copy_mrt3_pass");
		path.loadShader("shader_datas/dilate_pass/dilate_pass");
	}

	@:access(iron.RenderPath)
	public static function commandsPaint() {
		var tid = Context.layer.id;

		if (Context.pdirty > 0 && UITrait.inst.worktab.position != SpaceScene) {
			if (Context.tool == ToolParticle) {
				path.setTarget("texparticle");
				path.clearTarget(0x00000000);
				path.bindTarget("_main", "gbufferD");
				if ((UITrait.inst.xray || UITrait.inst.brushAngleReject) && UITrait.inst.brush3d) {
					path.bindTarget("gbuffer0", "gbuffer0");
				}

				var mo: MeshObject = cast Scene.active.getChild(".ParticleEmitter");
				mo.visible = true;
				mo.render(path.currentG, "mesh", @:privateAccess path.bindParams);
				mo.visible = false;

				mo = cast Scene.active.getChild(".Particle");
				mo.visible = true;
				mo.render(path.currentG, "mesh", @:privateAccess path.bindParams);
				mo.visible = false;
				@:privateAccess path.end();
			}

			if (Context.tool == ToolColorId) {
				path.setTarget("texpaint_colorid");
				path.clearTarget(0xff000000);
				path.bindTarget("gbuffer2", "gbuffer2");
				path.drawMeshes("paint");
				UITrait.inst.headerHandle.redraws = 2;
			}
			else if (Context.tool == ToolPicker) {
				path.setTarget("texpaint_picker", ["texpaint_nor_picker", "texpaint_pack_picker"]);
				path.clearTarget(0xff000000);
				path.bindTarget("gbuffer2", "gbuffer2");
				tid = Context.layer.id;
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
				UITrait.inst.uvxPicked = a.get(3) / 255;
				UITrait.inst.normalRPicked = b.get(0) / 255;
				UITrait.inst.normalGPicked = b.get(1) / 255;
				UITrait.inst.normalBPicked = b.get(2) / 255;
				UITrait.inst.uvyPicked = c.get(3) / 255;
				UITrait.inst.occlusionPicked = c.get(0) / 255;
				UITrait.inst.roughnessPicked = c.get(1) / 255;
				UITrait.inst.metallicPicked = c.get(2) / 255;
				// Pick material
				if (UITrait.inst.pickerSelectMaterial) {
					var matid = b.get(3);
					for (m in Project.materials) {
						if (m.id == matid) {
							Context.setMaterial(m);
							UITrait.inst.materialIdPicked = matid;
							break;
						}
					}
				}
			}
			else {
				#if (!kha_direct3d12)
				if (Context.tool == ToolBake && UITrait.inst.bakeType == BakeAO) {
					if (initVoxels) {
						initVoxels = false;
						// Init voxel texture
						var rp_gi = Config.raw.rp_gi;
						Config.raw.rp_gi = true;
						#if rp_voxelao
						Inc.initGI();
						#end
						Config.raw.rp_gi = rp_gi;
					}
					path.clearImage("voxels", 0x00000000);
					path.setTarget("");
					path.setViewport(256, 256);
					path.bindTarget("voxels", "voxels");
					path.drawMeshes("voxel");
					path.generateMipmaps("voxels");
				}
				#end

				if (Context.tool == ToolBake && UITrait.inst.brushTime == iron.system.Time.delta) {
					// Clear to black on bake start
					path.setTarget("texpaint" + tid);
					path.clearTarget(0xff000000);
				}

				var blendA = "texpaint_blend0";
				var blendB = "texpaint_blend1";
				path.setTarget(blendB);
				path.bindTarget(blendA, "tex");
				path.drawShader("shader_datas/copy_pass/copy_pass");

				var isMask = Context.layerIsMask;
				var texpaint = isMask ? "texpaint_mask" + tid : "texpaint" + tid;
				path.setTarget(texpaint, ["texpaint_nor" + tid, "texpaint_pack" + tid, blendA]);
				path.bindTarget("_main", "gbufferD");
				if ((UITrait.inst.xray || UITrait.inst.brushAngleReject) && UITrait.inst.brush3d) {
					path.bindTarget("gbuffer0", "gbuffer0");
				}
				path.bindTarget(blendB, "paintmask");
				#if (!kha_direct3d12)
				if (Context.tool == ToolBake && UITrait.inst.bakeType == BakeAO) {
					path.bindTarget("voxels", "voxels");
				}
				#end
				if (UITrait.inst.colorIdPicked) {
					path.bindTarget("texpaint_colorid", "texpaint_colorid");
				}

				// Read texcoords from gbuffer
				var readTC = (Context.tool == ToolFill && UITrait.inst.fillTypeHandle.position == FillFace) ||
							  Context.tool == ToolClone ||
							  Context.tool == ToolBlur;
				if (readTC) {
					path.bindTarget("gbuffer2", "gbuffer2");
				}

				path.drawMeshes("paint");

				if (Context.tool == ToolBake && UITrait.inst.bakeType == BakeCurvature && UITrait.inst.bakeCurvSmooth > 0) {
					if (path.renderTargets.get("texpaint_blur") == null) {
						var t = new RenderTargetRaw();
						t.name = "texpaint_blur";
						t.width = Std.int(Config.getTextureRes() * 0.95);
						t.height = Std.int(Config.getTextureRes() * 0.95);
						t.format = "RGBA32";
						path.createRenderTarget(t);
					}
					var blurs = Math.round(UITrait.inst.bakeCurvSmooth);
					for (i in 0...blurs) {
						path.setTarget("texpaint_blur");
						path.bindTarget(texpaint, "tex");
						path.drawShader("shader_datas/copy_pass/copy_pass");
						path.setTarget(texpaint);
						path.bindTarget("texpaint_blur", "tex");
						path.drawShader("shader_datas/copy_pass/copy_pass");
					}
				}
			}
		}
	}

	@:access(iron.RenderPath)
	public static function commandsCursor() {
		var tool = Context.tool;
		if (tool != ToolBrush &&
			tool != ToolEraser &&
			tool != ToolClone &&
			tool != ToolBlur &&
			tool != ToolParticle) {
			// tool != ToolDecal &&
			// tool != ToolText) {
				return;
		}
		if (!App.uienabled ||
			UITrait.inst.worktab.position == SpaceScene) {
			return;
		}

		var plane = cast(Scene.active.getChild(".Plane"), MeshObject);
		var geom = plane.data.geom;

		var g = path.frameG;
		if (Layers.pipeCursor == null) Layers.makeCursorPipe();

		path.setTarget("");
		g.setPipeline(Layers.pipeCursor);
		var decal = Context.tool == ToolDecal || Context.tool == ToolText;
		var img = decal ? UITrait.inst.decalImage : Res.get("cursor.k");
		g.setTexture(Layers.cursorTex, img);
		var gbuffer0 = path.renderTargets.get("gbuffer0").image;
		g.setTextureDepth(Layers.cursorGbufferD, gbuffer0);
		g.setTexture(Layers.cursorGbuffer0, gbuffer0);
		var mx = Input.getMouse().viewX / iron.App.w();
		var my = 1.0 - (Input.getMouse().viewY / iron.App.h());
		if (UITrait.inst.brushLocked) {
			mx = (UITrait.inst.lockStartedX - iron.App.x()) / iron.App.w();
			my = 1.0 - (UITrait.inst.lockStartedY - iron.App.y()) / iron.App.h();
		}
		g.setFloat2(Layers.cursorMouse, mx, my);
		g.setFloat2(Layers.cursorStep, 2 / gbuffer0.width, 2 / gbuffer0.height);
		g.setFloat(Layers.cursorRadius, UITrait.inst.brushRadius / 3.4);
		g.setMatrix(Layers.cursorVP, Scene.active.camera.VP.self);
		var helpMat = iron.math.Mat4.identity();
		helpMat.getInverse(Scene.active.camera.VP);
		g.setMatrix(Layers.cursorInvVP, helpMat.self);
		g.setVertexBuffer(geom.vertexBuffer);
		g.setIndexBuffer(geom.indexBuffers[0]);
		g.drawIndexedVertices();

		g.disableScissor();
		path.end();
	}

	public static function begin() {
		pushUndoLast = History.pushUndo;
		if (History.pushUndo && History.undoLayers != null) {
			History.paint();
		}

		// 2D paint
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
			ViewportUtil.updateCameraType(CameraPerspective);
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
	}

	public static function end() {
		if (UITrait.inst.brush3d) RenderPathPaint.commandsCursor();
		Context.ddirty--;
		Context.pdirty--;
		Context.rdirty--;
	}

	public static function draw() {
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
				if (Context.pdirty > 0) dilated = false;
				if (UITrait.inst.bakeType == BakeNormal || UITrait.inst.bakeType == BakeHeight || UITrait.inst.bakeType == BakeDerivative) {
					if (!baking && Context.pdirty > 0) {
						baking = true;
						var _bakeType = UITrait.inst.bakeType;
						UITrait.inst.bakeType = UITrait.inst.bakeType == BakeNormal ? BakeNormalObject : BakePosition; // Bake high poly data
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
						Context.selectPaintObject(_paintObject);

						function _renderFinal(_) {
							UITrait.inst.bakeType = _bakeType;
							MaterialParser.parsePaintMaterial();
							Context.pdirty = 1;
							RenderPathPaint.commandsPaint();
							Context.pdirty = 0;
							iron.App.removeRender(_renderFinal);
							baking = false;
						}
						function _renderDeriv(_) {
							UITrait.inst.bakeType = BakeHeight;
							MaterialParser.parsePaintMaterial();
							Context.pdirty = 1;
							RenderPathPaint.commandsPaint();
							Context.pdirty = 0;
							UITrait.inst.sub--;
							if (pushUndoLast) History.paint();
							iron.App.removeRender(_renderDeriv);
							iron.App.notifyOnRender(_renderFinal);
						}
						iron.App.notifyOnRender(UITrait.inst.bakeType == BakeDerivative ? _renderDeriv : _renderFinal);
					}
				}
				else if (UITrait.inst.bakeType == BakeObjectID) {
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
				else if (UITrait.inst.bakeType == BakeAO  ||
						 UITrait.inst.bakeType == BakeLightmap ||
						 UITrait.inst.bakeType == BakeBentNormal ||
						 UITrait.inst.bakeType == BakeThickness) {
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

			RenderPathDeferred.drawGbuffer();
		}
	}

	public static function bindLayers() {
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
	}

	public static function finishPaint() {
		if (Context.tool == ToolBake && !dilated && UITrait.inst.dilateRadius > 0) {
			Layers.makeTempImg();
			dilated = true;
			path.setTarget("temptex0");
			path.bindTarget("texpaint0", "tex");
			path.drawShader("shader_datas/copy_pass/copy_pass");
			path.setTarget("texpaint0");
			path.bindTarget("temptex0", "tex");
			path.drawShader("shader_datas/dilate_pass/dilate_pass");
		}
		// Brush stroke dilate
		// arm.util.UVUtil.cacheTriangleMap();
		// Layers.makeTempImg();
		// path.setTarget("temptex0");
		// path.bindTarget("texpaint0", "tex");
		// path.drawShader("shader_datas/copy_pass/copy_pass");
		// path.setTarget("texpaint0");
		// path.bindTarget("temptex0", "tex");
		// path.drawShader("shader_datas/dilate_pass/dilate_pass");
	}
}

#end
