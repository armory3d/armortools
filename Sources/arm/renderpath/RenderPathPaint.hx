package arm.renderpath;

import iron.RenderPath;
import armory.renderpath.Inc;
import arm.ui.UITrait;
import arm.ui.*;

class RenderPathPaint {
	
	static var initVoxels = true; // Bake AO

	@:access(iron.RenderPath)
	public static function commandsPaint() {

		var path = RenderPathDeferred.path;
		
		if (UITrait.inst.paintDirty()) {

			var tid = UITrait.inst.selectedLayer.id;

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
				@:privateAccess path.end();
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
	}
}
