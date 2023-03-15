package arm.render;

import iron.math.Mat4;
import iron.math.Vec4;
import iron.object.MeshObject;
import iron.data.SceneFormat;
import iron.data.MeshData;
import iron.system.Input;
import iron.RenderPath;
import iron.Scene;
import arm.ui.UIView2D;
import arm.ui.UIHeader;
import arm.ui.UIStatus;
import arm.shader.MakeMaterial;
import arm.Viewport;

class RenderPathPaint {

	public static var liveLayer: arm.data.LayerSlot = null;
	public static var liveLayerDrawn = 0;
	public static var liveLayerLocked = false;
	static var path: RenderPath;
	static var dilated = true;
	static var initVoxels = true; // Bake AO
	static var pushUndoLast: Bool;
	static var painto: MeshObject = null;
	static var planeo: MeshObject = null;
	static var visibles: Array<Bool> = null;
	static var mergedObjectVisible = false;
	static var savedFov = 0.0;
	static var baking = false;
	static var _texpaint: RenderTarget;
	static var _texpaint_nor: RenderTarget;
	static var _texpaint_pack: RenderTarget;
	static var _texpaint_undo: RenderTarget;
	static var _texpaint_nor_undo: RenderTarget;
	static var _texpaint_pack_undo: RenderTarget;
	static var lastX = -1.0;
	static var lastY = -1.0;

	public static function init(_path: RenderPath) {
		path = _path;

		{
			var t = new RenderTargetRaw();
			t.name = "texpaint_blend0";
			t.width = Config.getTextureResX();
			t.height = Config.getTextureResY();
			t.format = "R8";
			path.createRenderTarget(t);
		}
		{
			var t = new RenderTargetRaw();
			t.name = "texpaint_blend1";
			t.width = Config.getTextureResX();
			t.height = Config.getTextureResY();
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
		{
			var t = new RenderTargetRaw();
			t.name = "texpaint_uv_picker";
			t.width = 1;
			t.height = 1;
			t.format = "RGBA32";
			path.createRenderTarget(t);
		}
		{
			var t = new RenderTargetRaw();
			t.name = "texpaint_posnortex_picker0";
			t.width = 1;
			t.height = 1;
			t.format = "RGBA128";
			path.createRenderTarget(t);
		}
		{
			var t = new RenderTargetRaw();
			t.name = "texpaint_posnortex_picker1";
			t.width = 1;
			t.height = 1;
			t.format = "RGBA128";
			path.createRenderTarget(t);
		}

		path.loadShader("shader_datas/copy_mrt3_pass/copy_mrt3_pass");
		path.loadShader("shader_datas/copy_mrt3_pass/copy_mrt3RGBA64_pass");
		path.loadShader("shader_datas/dilate_pass/dilate_pass");
	}

	public static function commandsPaint(dilation = true) {
		var tid = Context.raw.layer.id;

		if (Context.raw.pdirty > 0) {
			#if arm_physics
			var particlePhysics = Context.raw.particlePhysics;
			#else
			var particlePhysics = false;
			#end
			if (Context.raw.tool == ToolParticle && !particlePhysics) {
				path.setTarget("texparticle");
				path.clearTarget(0x00000000);
				path.bindTarget("_main", "gbufferD");
				if ((Context.raw.xray || Config.raw.brush_angle_reject) && Config.raw.brush_3d) {
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

			if (Context.raw.tool == ToolColorId) {
				path.setTarget("texpaint_colorid");
				path.clearTarget(0xff000000);
				path.bindTarget("gbuffer2", "gbuffer2");
				path.drawMeshes("paint");
				UIHeader.inst.headerHandle.redraws = 2;
			}
			else if (Context.raw.tool == ToolPicker) {
				if (Context.raw.pickPosNorTex) {
					if (Context.raw.paint2d) {
						path.setTarget("gbuffer0", ["gbuffer1", "gbuffer2"]);
						path.drawMeshes("mesh");
					}
					path.setTarget("texpaint_posnortex_picker0", ["texpaint_posnortex_picker1"]);
					path.bindTarget("gbuffer2", "gbuffer2");
					path.bindTarget("_main", "gbufferD");
					path.drawMeshes("paint");
					var texpaint_posnortex_picker0 = path.renderTargets.get("texpaint_posnortex_picker0").image;
					var texpaint_posnortex_picker1 = path.renderTargets.get("texpaint_posnortex_picker1").image;
					var a = texpaint_posnortex_picker0.getPixels();
					var b = texpaint_posnortex_picker1.getPixels();
					Context.raw.posXPicked = a.getFloat(0);
					Context.raw.posYPicked = a.getFloat(4);
					Context.raw.posZPicked = a.getFloat(8);
					Context.raw.uvxPicked = a.getFloat(12);
					Context.raw.norXPicked = b.getFloat(0);
					Context.raw.norYPicked = b.getFloat(4);
					Context.raw.norZPicked = b.getFloat(8);
					Context.raw.uvyPicked = b.getFloat(12);
				}
				else {
					path.setTarget("texpaint_picker", ["texpaint_nor_picker", "texpaint_pack_picker", "texpaint_uv_picker"]);
					path.bindTarget("gbuffer2", "gbuffer2");
					tid = Context.raw.layer.id;
					var useLiveLayer = arm.ui.UIHeader.inst.worktab.position == SpaceMaterial;
					if (useLiveLayer) RenderPathPaint.useLiveLayer(true);
					path.bindTarget("texpaint" + tid, "texpaint");
					path.bindTarget("texpaint_nor" + tid, "texpaint_nor");
					path.bindTarget("texpaint_pack" + tid, "texpaint_pack");
					path.drawMeshes("paint");
					if (useLiveLayer) RenderPathPaint.useLiveLayer(false);
					UIHeader.inst.headerHandle.redraws = 2;
					UIStatus.inst.statusHandle.redraws = 2;

					var texpaint_picker = path.renderTargets.get("texpaint_picker").image;
					var texpaint_nor_picker = path.renderTargets.get("texpaint_nor_picker").image;
					var texpaint_pack_picker = path.renderTargets.get("texpaint_pack_picker").image;
					var texpaint_uv_picker = path.renderTargets.get("texpaint_uv_picker").image;
					var a = texpaint_picker.getPixels();
					var b = texpaint_nor_picker.getPixels();
					var c = texpaint_pack_picker.getPixels();
					var d = texpaint_uv_picker.getPixels();

					if (Context.raw.colorPickerCallback != null) {
						Context.raw.colorPickerCallback(Context.raw.pickedColor);
					}

					// Picked surface values
					#if (kha_metal || kha_vulkan)
					var i0 = 2;
					var i1 = 1;
					var i2 = 0;
					#else
					var i0 = 0;
					var i1 = 1;
					var i2 = 2;
					#end
					var i3 = 3;
					Context.raw.pickedColor.base.Rb = a.get(i0);
					Context.raw.pickedColor.base.Gb = a.get(i1);
					Context.raw.pickedColor.base.Bb = a.get(i2);
					Context.raw.pickedColor.normal.Rb = b.get(i0);
					Context.raw.pickedColor.normal.Gb = b.get(i1);
					Context.raw.pickedColor.normal.Bb = b.get(i2);
					Context.raw.pickedColor.occlusion = c.get(i0) / 255;
					Context.raw.pickedColor.roughness = c.get(i1) / 255;
					Context.raw.pickedColor.metallic = c.get(i2) / 255;
					Context.raw.pickedColor.height = c.get(i3) / 255;
					Context.raw.pickedColor.opacity = a.get(i3) / 255;
					Context.raw.uvxPicked = d.get(i0) / 255;
					Context.raw.uvyPicked = d.get(i1) / 255;
					// Pick material
					if (Context.raw.pickerSelectMaterial && Context.raw.colorPickerCallback == null) {
						// matid % 3 == 0 - normal, 1 - emission, 2 - subsurface
						var matid = Std.int((b.get(3) - (b.get(3) % 3)) / 3);
						for (m in Project.materials) {
							if (m.id == matid) {
								Context.setMaterial(m);
								Context.raw.materialIdPicked = matid;
								break;
							}
						}
					}
				}
			}
			else {
				#if rp_voxels
				if (Context.raw.tool == ToolBake && Context.raw.bakeType == BakeAO) {
					if (initVoxels) {
						initVoxels = false;
						var _rp_gi = Config.raw.rp_gi;
						Config.raw.rp_gi = true;
						RenderPathBase.initVoxels();
						Config.raw.rp_gi = _rp_gi;
					}
					path.clearImage("voxels", 0x00000000);
					path.setTarget("");
					path.setViewport(256, 256);
					path.bindTarget("voxels", "voxels");
					path.drawMeshes("voxel");
					path.generateMipmaps("voxels");
				}
				#end

				var texpaint = "texpaint" + tid;
				if (Context.raw.tool == ToolBake && Context.raw.brushTime == iron.system.Time.delta) {
					// Clear to black on bake start
					path.setTarget(texpaint);
					path.clearTarget(0xff000000);
				}

				path.setTarget("texpaint_blend1");
				path.bindTarget("texpaint_blend0", "tex");
				path.drawShader("shader_datas/copy_pass/copyR8_pass");
				var isMask = Context.raw.layer.isMask();
				if (isMask) {
					var ptid = Context.raw.layer.parent.id;
					if (Context.raw.layer.parent.isGroup()) { // Group mask
						for (c in Context.raw.layer.parent.getChildren()) {
							ptid = c.id;
							break;
						}
					}
					path.setTarget(texpaint, ["texpaint_nor" + ptid, "texpaint_pack" + ptid, "texpaint_blend0"]);
				}
				else {
					path.setTarget(texpaint, ["texpaint_nor" + tid, "texpaint_pack" + tid, "texpaint_blend0"]);
				}
				path.bindTarget("_main", "gbufferD");
				if ((Context.raw.xray || Config.raw.brush_angle_reject) && Config.raw.brush_3d) {
					path.bindTarget("gbuffer0", "gbuffer0");
				}
				path.bindTarget("texpaint_blend1", "paintmask");
				#if rp_voxels
				if (Context.raw.tool == ToolBake && Context.raw.bakeType == BakeAO) {
					path.bindTarget("voxels", "voxels");
				}
				#end
				if (Context.raw.colorIdPicked) {
					path.bindTarget("texpaint_colorid", "texpaint_colorid");
				}

				// Read texcoords from gbuffer
				var readTC = (Context.raw.tool == ToolFill && Context.raw.fillTypeHandle.position == FillFace) ||
							  Context.raw.tool == ToolClone ||
							  Context.raw.tool == ToolBlur ||
							  Context.raw.tool == ToolSmudge;
				if (readTC) {
					path.bindTarget("gbuffer2", "gbuffer2");
				}

				path.drawMeshes("paint");

				if (Context.raw.tool == ToolBake && Context.raw.bakeType == BakeCurvature && Context.raw.bakeCurvSmooth > 0) {
					if (path.renderTargets.get("texpaint_blur") == null) {
						var t = new RenderTargetRaw();
						t.name = "texpaint_blur";
						t.width = Std.int(Config.getTextureResX() * 0.95);
						t.height = Std.int(Config.getTextureResY() * 0.95);
						t.format = "RGBA32";
						path.createRenderTarget(t);
					}
					var blurs = Math.round(Context.raw.bakeCurvSmooth);
					for (i in 0...blurs) {
						path.setTarget("texpaint_blur");
						path.bindTarget(texpaint, "tex");
						path.drawShader("shader_datas/copy_pass/copy_pass");
						path.setTarget(texpaint);
						path.bindTarget("texpaint_blur", "tex");
						path.drawShader("shader_datas/copy_pass/copy_pass");
					}
				}

				if (dilation && Config.raw.dilate == DilateInstant) {
					dilate(true, false);
				}
			}
		}
	}

	public static function useLiveLayer(use: Bool) {
		var tid = Context.raw.layer.id;
		var hid = History.undoI - 1 < 0 ? Config.raw.undo_steps - 1 : History.undoI - 1;
		if (use) {
			_texpaint = path.renderTargets.get("texpaint" + tid);
			_texpaint_undo = path.renderTargets.get("texpaint_undo" + hid);
			_texpaint_nor_undo = path.renderTargets.get("texpaint_nor_undo" + hid);
			_texpaint_pack_undo = path.renderTargets.get("texpaint_pack_undo" + hid);
			_texpaint_nor = path.renderTargets.get("texpaint_nor" + tid);
			_texpaint_pack = path.renderTargets.get("texpaint_pack" + tid);
			path.renderTargets.set("texpaint_undo" + hid, path.renderTargets.get("texpaint" + tid));
			path.renderTargets.set("texpaint" + tid, path.renderTargets.get("texpaint_live"));
			if (Context.raw.layer.isLayer()) {
				path.renderTargets.set("texpaint_nor_undo" + hid, path.renderTargets.get("texpaint_nor" + tid));
				path.renderTargets.set("texpaint_pack_undo" + hid, path.renderTargets.get("texpaint_pack" + tid));
				path.renderTargets.set("texpaint_nor" + tid, path.renderTargets.get("texpaint_nor_live"));
				path.renderTargets.set("texpaint_pack" + tid, path.renderTargets.get("texpaint_pack_live"));
			}
		}
		else {
			path.renderTargets.set("texpaint" + tid, _texpaint);
			path.renderTargets.set("texpaint_undo" + hid, _texpaint_undo);
			if (Context.raw.layer.isLayer()) {
				path.renderTargets.set("texpaint_nor_undo" + hid, _texpaint_nor_undo);
				path.renderTargets.set("texpaint_pack_undo" + hid, _texpaint_pack_undo);
				path.renderTargets.set("texpaint_nor" + tid, _texpaint_nor);
				path.renderTargets.set("texpaint_pack" + tid, _texpaint_pack);
			}
		}
		liveLayerLocked = use;
	}

	static function commandsLiveBrush() {
		var tool = Context.raw.tool;
		if (tool != ToolBrush &&
			tool != ToolEraser &&
			tool != ToolClone &&
			tool != ToolDecal &&
			tool != ToolText &&
			tool != ToolBlur &&
			tool != ToolSmudge) {
				return;
		}

		if (liveLayerLocked) return;

		if (liveLayer == null) {
			liveLayer = new arm.data.LayerSlot("_live");
		}

		var tid = Context.raw.layer.id;
		if (Context.raw.layer.isMask()) {
			path.setTarget("texpaint_live");
			path.bindTarget("texpaint" + tid, "tex");
			path.drawShader("shader_datas/copy_pass/copy_pass");
		}
		else {
			path.setTarget("texpaint_live", ["texpaint_nor_live", "texpaint_pack_live"]);
			path.bindTarget("texpaint" + tid, "tex0");
			path.bindTarget("texpaint_nor" + tid, "tex1");
			path.bindTarget("texpaint_pack" + tid, "tex2");
			path.drawShader("shader_datas/copy_mrt3_pass/copy_mrt3_pass");
		}

		useLiveLayer(true);

		liveLayerDrawn = 2;

		UIView2D.inst.hwnd.redraws = 2;
		var _x = Context.raw.paintVec.x;
		var _y = Context.raw.paintVec.y;
		if (Context.raw.brushLocked) {
			Context.raw.paintVec.x = (Context.raw.lockStartedX - iron.App.x()) / iron.App.w();
			Context.raw.paintVec.y = (Context.raw.lockStartedY - iron.App.y()) / iron.App.h();
		}
		var _lastX = Context.raw.lastPaintVecX;
		var _lastY = Context.raw.lastPaintVecY;
		var _pdirty = Context.raw.pdirty;
		Context.raw.lastPaintVecX = Context.raw.paintVec.x;
		Context.raw.lastPaintVecY = Context.raw.paintVec.y;
		if (Operator.shortcut(Config.keymap.brush_ruler)) {
			Context.raw.lastPaintVecX = Context.raw.lastPaintX;
			Context.raw.lastPaintVecY = Context.raw.lastPaintY;
		}
		Context.raw.pdirty = 2;

		commandsSymmetry();
		commandsPaint();

		useLiveLayer(false);

		Context.raw.paintVec.x = _x;
		Context.raw.paintVec.y = _y;
		Context.raw.lastPaintVecX = _lastX;
		Context.raw.lastPaintVecY = _lastY;
		Context.raw.pdirty = _pdirty;
		Context.raw.brushBlendDirty = true;
	}

	public static function commandsCursor() {
		if (!Config.raw.brush_3d) return;
		var decal = Context.raw.tool == ToolDecal || Context.raw.tool == ToolText;
		var decalMask = decal && Operator.shortcut(Config.keymap.decal_mask, ShortcutDown);
		var tool = Context.raw.tool;
		if (tool != ToolBrush &&
			tool != ToolEraser &&
			tool != ToolClone &&
			tool != ToolBlur &&
			tool != ToolSmudge &&
			tool != ToolParticle &&
			!decalMask) {
				return;
		}

		var fillLayer = Context.raw.layer.fill_layer != null;
		var groupLayer = Context.raw.layer.isGroup();
		if (!App.uiEnabled || App.isDragging || fillLayer || groupLayer) {
			return;
		}

		var mx = Context.raw.paintVec.x;
		var my = 1.0 - Context.raw.paintVec.y;
		if (Context.raw.brushLocked) {
			mx = (Context.raw.lockStartedX - iron.App.x()) / iron.App.w();
			my = 1.0 - (Context.raw.lockStartedY - iron.App.y()) / iron.App.h();
		}
		var radius = decalMask ? Context.raw.brushDecalMaskRadius : Context.raw.brushRadius;
		drawCursor(mx, my, Context.raw.brushNodesRadius * radius / 3.4);
	}

	@:access(iron.RenderPath)
	static function drawCursor(mx: Float, my: Float, radius: Float, tintR = 1.0, tintG = 1.0, tintB = 1.0) {
		var plane = cast(Scene.active.getChild(".Plane"), MeshObject);
		var geom = plane.data.geom;

		var g = path.frameG;
		if (App.pipeCursor == null) App.makeCursorPipe();

		path.setTarget("");
		g.setPipeline(App.pipeCursor);
		var decal = Context.raw.tool == ToolDecal || Context.raw.tool == ToolText;
		var decalMask = decal && Operator.shortcut(Config.keymap.decal_mask, ShortcutDown);
		var img = (decal && !decalMask) ? Context.raw.decalImage : Res.get("cursor.k");
		g.setTexture(App.cursorTex, img);
		var gbuffer0 = path.renderTargets.get("gbuffer0").image;
		g.setTextureDepth(App.cursorGbufferD, gbuffer0);
		g.setFloat2(App.cursorMouse, mx, my);
		g.setFloat2(App.cursorTexStep, 1 / gbuffer0.width, 1 / gbuffer0.height);
		g.setFloat(App.cursorRadius, radius);
		var right = Scene.active.camera.rightWorld().normalize();
		g.setFloat3(App.cursorCameraRight, right.x, right.y, right.z);
		g.setFloat3(App.cursorTint, tintR, tintG, tintB);
		g.setMatrix(App.cursorVP, Scene.active.camera.VP.self);
		var helpMat = Mat4.identity();
		helpMat.getInverse(Scene.active.camera.VP);
		g.setMatrix(App.cursorInvVP, helpMat.self);
		#if (kha_metal || kha_vulkan)
		g.setVertexBuffer(geom.get([{name: "tex", data: "short2norm"}]));
		#else
		g.setVertexBuffer(geom.vertexBuffer);
		#end
		g.setIndexBuffer(geom.indexBuffers[0]);
		g.drawIndexedVertices();

		g.disableScissor();
		path.end();
	}

	static function commandsSymmetry() {
		if (Context.raw.symX || Context.raw.symY || Context.raw.symZ) {
			Context.raw.ddirty = 2;
			var t = Context.raw.paintObject.transform;
			var sx = t.scale.x;
			var sy = t.scale.y;
			var sz = t.scale.z;
			if (Context.raw.symX) {
				t.scale.set(-sx, sy, sz);
				t.buildMatrix();
				commandsPaint(false);
			}
			if (Context.raw.symY) {
				t.scale.set(sx, -sy, sz);
				t.buildMatrix();
				commandsPaint(false);
			}
			if (Context.raw.symZ) {
				t.scale.set(sx, sy, -sz);
				t.buildMatrix();
				commandsPaint(false);
			}
			if (Context.raw.symX && Context.raw.symY) {
				t.scale.set(-sx, -sy, sz);
				t.buildMatrix();
				commandsPaint(false);
			}
			if (Context.raw.symX && Context.raw.symZ) {
				t.scale.set(-sx, sy, -sz);
				t.buildMatrix();
				commandsPaint(false);
			}
			if (Context.raw.symY && Context.raw.symZ) {
				t.scale.set(sx, -sy, -sz);
				t.buildMatrix();
				commandsPaint(false);
			}
			if (Context.raw.symX && Context.raw.symY && Context.raw.symZ) {
				t.scale.set(-sx, -sy, -sz);
				t.buildMatrix();
				commandsPaint(false);
			}
			t.scale.set(sx, sy, sz);
			t.buildMatrix();
		}
	}

	static function paintEnabled(): Bool {
		var fillLayer = Context.raw.layer.fill_layer != null && Context.raw.tool != ToolPicker && Context.raw.tool != ToolColorId;
		var groupLayer = Context.raw.layer.isGroup();
		return !fillLayer && !groupLayer && !Context.raw.foregroundEvent;
	}

	public static function liveBrushDirty() {
		var mouse = Input.getMouse();
		var mx = lastX;
		var my = lastY;
		lastX = mouse.viewX;
		lastY = mouse.viewY;
		if (Config.raw.brush_live && Context.raw.pdirty <= 0) {
			var moved = (mx != lastX || my != lastY) && (Context.inViewport() || Context.in2dView());
			if (moved || Context.raw.brushLocked) {
				Context.raw.rdirty = 2;
			}
		}
	}

	public static function begin() {
		if (!dilated) {
			dilate(Config.raw.dilate == DilateDelayed, true);
			dilated = true;
		}

		if (!paintEnabled()) return;

		pushUndoLast = History.pushUndo;
		if (History.pushUndo && History.undoLayers != null) {
			History.paint();
		}

		if (Context.raw.paint2d) {
			setPlaneMesh();
		}

		if (liveLayerDrawn > 0) liveLayerDrawn--;

		if (Config.raw.brush_live && Context.raw.pdirty <= 0 && Context.raw.ddirty <= 0 && Context.raw.brushTime == 0) {
			// Depth is unchanged, draw before gbuffer gets updated
			commandsLiveBrush();
		}
	}

	public static function end() {
		commandsCursor();
		Context.raw.ddirty--;
		Context.raw.rdirty--;

		if (!paintEnabled()) return;
		Context.raw.pdirty--;
	}

	public static function draw() {
		if (!paintEnabled()) return;

		#if (!krom_ios) // No hover on iPad, decals are painted by pen release
		if (Config.raw.brush_live && Context.raw.pdirty <= 0 && Context.raw.ddirty > 0 && Context.raw.brushTime == 0) {
			// gbuffer has been updated now but brush will lag 1 frame
			commandsLiveBrush();
		}
		#end

		if (History.undoLayers != null) {
			commandsSymmetry();

			if (Context.raw.pdirty > 0) dilated = false;
			if (Context.raw.tool == ToolBake) {
				if (Context.raw.bakeType == BakeNormal || Context.raw.bakeType == BakeHeight || Context.raw.bakeType == BakeDerivative) {
					if (!baking && Context.raw.pdirty > 0) {
						baking = true;
						var _bakeType = Context.raw.bakeType;
						Context.raw.bakeType = Context.raw.bakeType == BakeNormal ? BakeNormalObject : BakePosition; // Bake high poly data
						MakeMaterial.parsePaintMaterial();
						var _paintObject = Context.raw.paintObject;
						var highPoly = Project.paintObjects[Context.raw.bakeHighPoly];
						var _visible = highPoly.visible;
						highPoly.visible = true;
						Context.selectPaintObject(highPoly);
						commandsPaint();
						highPoly.visible = _visible;
						if (pushUndoLast) History.paint();
						Context.selectPaintObject(_paintObject);

						function _renderFinal() {
							Context.raw.bakeType = _bakeType;
							MakeMaterial.parsePaintMaterial();
							Context.raw.pdirty = 1;
							commandsPaint();
							Context.raw.pdirty = 0;
							baking = false;
						}
						function _renderDeriv() {
							Context.raw.bakeType = BakeHeight;
							MakeMaterial.parsePaintMaterial();
							Context.raw.pdirty = 1;
							commandsPaint();
							Context.raw.pdirty = 0;
							if (pushUndoLast) History.paint();
							iron.App.notifyOnInit(_renderFinal);
						}
						iron.App.notifyOnInit(Context.raw.bakeType == BakeDerivative ? _renderDeriv : _renderFinal);
					}
				}
				else if (Context.raw.bakeType == BakeObjectID) {
					var _layerFilter = Context.raw.layerFilter;
					var _paintObject = Context.raw.paintObject;
					var isMerged = Context.raw.mergedObject != null;
					var _visible = isMerged && Context.raw.mergedObject.visible;
					Context.raw.layerFilter = 1;
					if (isMerged) Context.raw.mergedObject.visible = false;

					for (p in Project.paintObjects) {
						Context.selectPaintObject(p);
						commandsPaint();
					}

					Context.raw.layerFilter = _layerFilter;
					Context.selectPaintObject(_paintObject);
					if (isMerged) Context.raw.mergedObject.visible = _visible;
				}
				#if (kha_direct3d12 || kha_vulkan)
				else if (Context.raw.bakeType == BakeAO  ||
						 Context.raw.bakeType == BakeLightmap ||
						 Context.raw.bakeType == BakeBentNormal ||
						 Context.raw.bakeType == BakeThickness) {
					var dirty = RenderPathRaytraceBake.commands(MakeMaterial.parsePaintMaterial);
					if (dirty) UIHeader.inst.headerHandle.redraws = 2;
					if (Config.raw.dilate == DilateInstant) { // && Context.raw.pdirty == 1
						dilate(true, false);
					}
				}
				#end
				else {
					commandsPaint();
				}
			}
			else { // Paint
				commandsPaint();
			}
		}

		if (Context.raw.brushBlendDirty) {
			Context.raw.brushBlendDirty = false;
			#if kha_metal
			path.setTarget("texpaint_blend0");
			path.clearTarget(0x00000000);
			path.setTarget("texpaint_blend1");
			path.clearTarget(0x00000000);
			#else
			path.setTarget("texpaint_blend0", ["texpaint_blend1"]);
			path.clearTarget(0x00000000);
			#end
		}

		if (Context.raw.paint2d) {
			restorePlaneMesh();
		}
	}

	public static function setPlaneMesh() {
		Context.raw.paint2dView = true;
		painto = Context.raw.paintObject;
		visibles = [];
		for (p in Project.paintObjects) {
			visibles.push(p.visible);
			p.visible = false;
		}
		if (Context.raw.mergedObject != null) {
			mergedObjectVisible = Context.raw.mergedObject.visible;
			Context.raw.mergedObject.visible = false;
		}

		var cam = Scene.active.camera;
		Context.raw.savedCamera.setFrom(cam.transform.local);
		savedFov = cam.data.raw.fov;
		Viewport.updateCameraType(CameraPerspective);
		var m = Mat4.identity();
		m.translate(0, 0, 0.5);
		cam.transform.setMatrix(m);
		cam.data.raw.fov = 0.92;
		cam.buildProjection();
		cam.buildMatrix();

		var tw = 0.95 * UIView2D.inst.panScale;
		var tx = UIView2D.inst.panX / UIView2D.inst.ww;
		var ty = UIView2D.inst.panY / iron.App.h();
		m.setIdentity();
		m.scale(new Vec4(tw, tw, 1));
		m.setLoc(new Vec4(tx, ty, 0));
		var m2 = Mat4.identity();
		m2.getInverse(Scene.active.camera.VP);
		m.multmat(m2);

		var tiled = UIView2D.inst.tiledShow;
		if (tiled && Scene.active.getChild(".PlaneTiled") == null) {
			// 3x3 planes
			var posa = [32767,0,-32767,0,10922,0,-10922,0,10922,0,-32767,0,10922,0,-10922,0,-10922,0,10922,0,-10922,0,-10922,0,-10922,0,10922,0,-32767,0,32767,0,-32767,0,10922,0,10922,0,10922,0,-10922,0,32767,0,-10922,0,10922,0,32767,0,10922,0,10922,0,32767,0,10922,0,10922,0,-10922,0,-10922,0,-32767,0,10922,0,-32767,0,-10922,0,32767,0,-10922,0,10922,0,10922,0,10922,0,-10922,0,-10922,0,-32767,0,-32767,0,-10922,0,-32767,0,-32767,0,10922,0,-32767,0,-10922,0,-10922,0,-10922,0,-32767,0,32767,0,-32767,0,32767,0,-10922,0,10922,0,-10922,0,10922,0,-10922,0,10922,0,10922,0,-10922,0,10922,0,-10922,0,10922,0,-10922,0,32767,0,-32767,0,32767,0,10922,0,10922,0,10922,0,32767,0,-10922,0,32767,0,32767,0,10922,0,32767,0,32767,0,10922,0,32767,0,-10922,0,-10922,0,-10922,0,10922,0,-32767,0,10922,0,32767,0,-10922,0,32767,0,10922,0,10922,0,10922,0,-10922,0,-32767,0,-10922,0,-10922,0,-32767,0,-10922,0,10922,0,-32767,0,10922,0,-10922,0,-10922,0,-10922,0];
			var nora = [0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767];
			var texa = [32767,32767,0,0,0,32767,32767,32767,0,0,0,32767,32767,32767,0,0,0,32767,32767,32767,0,0,0,32767,32767,32767,0,0,0,32767,32767,32767,0,0,0,32767,32767,32767,0,0,0,32767,32767,32767,0,0,0,32767,32767,32767,0,0,0,32767,32767,32767,32767,0,0,0,32767,32767,32767,0,0,0,32767,32767,32767,0,0,0,32767,32767,32767,0,0,0,32767,32767,32767,0,0,0,32767,32767,32767,0,0,0,32767,32767,32767,0,0,0,32767,32767,32767,0,0,0,32767,32767,32767,0,0,0];
			var inda = [0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53];
			var raw: TMeshData = {
				name: ".PlaneTiled",
				vertex_arrays: [
					{ attrib: "pos", values: i16(posa), data: "short4norm" },
					{ attrib: "nor", values: i16(nora), data: "short2norm" },
					{ attrib: "tex", values: i16(texa), data: "short2norm" }
				],
				index_arrays: [
					{ values: u32(inda), material: 0 }
				],
				scale_pos: 1.5,
				scale_tex: 1.0
			};
			new MeshData(raw, function(md: MeshData) {
				var materials = cast(Scene.active.getChild(".Plane"), MeshObject).materials;
				var o = Scene.active.addMeshObject(md, materials);
				o.name = ".PlaneTiled";
			});
		}

		planeo = cast Scene.active.getChild(tiled ? ".PlaneTiled" : ".Plane");
		planeo.visible = true;
		Context.raw.paintObject = planeo;

		var v = new Vec4();
		var sx = v.set(m._00, m._01, m._02).length();
		planeo.transform.rot.fromEuler(-Math.PI / 2, 0, 0);
		planeo.transform.scale.set(sx, 1.0, sx);
		planeo.transform.scale.z *= Config.getTextureResY() / Config.getTextureResX();
		planeo.transform.loc.set(m._30, -m._31, 0.0);
		planeo.transform.buildMatrix();
	}

	public static function restorePlaneMesh() {
		Context.raw.paint2dView = false;
		planeo.visible = false;
		planeo.transform.loc.set(0.0, 0.0, 0.0);
		for (i in 0...Project.paintObjects.length) {
			Project.paintObjects[i].visible = visibles[i];
		}
		if (Context.raw.mergedObject != null) {
			Context.raw.mergedObject.visible = mergedObjectVisible;
		}
		Context.raw.paintObject = painto;
		Scene.active.camera.transform.setMatrix(Context.raw.savedCamera);
		Scene.active.camera.data.raw.fov = savedFov;
		Viewport.updateCameraType(Context.raw.cameraType);
		Scene.active.camera.buildProjection();
		Scene.active.camera.buildMatrix();

		RenderPathBase.drawGbuffer();
	}

	public static function bindLayers() {
		var isLive = Config.raw.brush_live && liveLayerDrawn > 0;
		var isMaterialSpace = UIHeader.inst.worktab.position == SpaceMaterial;
		if (isLive || isMaterialSpace) useLiveLayer(true);

		for (i in 0...Project.layers.length) {
			var l = Project.layers[i];
			path.bindTarget("texpaint" + l.id, "texpaint" + l.id);
			if (l.isLayer()) {
				path.bindTarget("texpaint_nor" + l.id, "texpaint_nor" + l.id);
				path.bindTarget("texpaint_pack" + l.id, "texpaint_pack" + l.id);
			}
		}
	}

	public static function unbindLayers() {
		var isLive = Config.raw.brush_live && liveLayerDrawn > 0;
		var isMaterialSpace = UIHeader.inst.worktab.position == SpaceMaterial;
		if (isLive || isMaterialSpace) useLiveLayer(false);
	}

	public static function dilate(base: Bool, nor_pack: Bool) {
		if (Config.raw.dilate_radius > 0 && !Context.raw.paint2d) {
			arm.util.UVUtil.cacheDilateMap();
			App.makeTempImg();
			var tid = Context.raw.layer.id;
			if (base) {
				var texpaint = "texpaint";
				path.setTarget("temptex0");
				path.bindTarget(texpaint + tid, "tex");
				path.drawShader("shader_datas/copy_pass/copy_pass");
				path.setTarget(texpaint + tid);
				path.bindTarget("temptex0", "tex");
				path.drawShader("shader_datas/dilate_pass/dilate_pass");
			}
			if (nor_pack && !Context.raw.layer.isMask()) {
				path.setTarget("temptex0");
				path.bindTarget("texpaint_nor" + tid, "tex");
				path.drawShader("shader_datas/copy_pass/copy_pass");
				path.setTarget("texpaint_nor" + tid);
				path.bindTarget("temptex0", "tex");
				path.drawShader("shader_datas/dilate_pass/dilate_pass");

				path.setTarget("temptex0");
				path.bindTarget("texpaint_pack" + tid, "tex");
				path.drawShader("shader_datas/copy_pass/copy_pass");
				path.setTarget("texpaint_pack" + tid);
				path.bindTarget("temptex0", "tex");
				path.drawShader("shader_datas/dilate_pass/dilate_pass");
			}
		}
	}

	static function u32(ar: Array<Int>): kha.arrays.Uint32Array {
		var res = new kha.arrays.Uint32Array(ar.length);
		for (i in 0...ar.length) res[i] = ar[i];
		return res;
	}

	static function i16(ar: Array<Int>): kha.arrays.Int16Array {
		var res = new kha.arrays.Int16Array(ar.length);
		for (i in 0...ar.length) res[i] = ar[i];
		return res;
	}
}
