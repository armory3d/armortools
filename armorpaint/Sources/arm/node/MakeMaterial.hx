package arm.node;

import zui.Nodes;
import iron.data.SceneFormat;
import iron.data.ShaderData;
import iron.data.MaterialData;
import iron.RenderPath;
import arm.ui.UIHeader;
import arm.ui.UINodes;
import arm.ui.UISidebar;
import arm.shader.NodeShader;
import arm.shader.NodeShaderContext;
import arm.shader.NodeShaderData;
import arm.shader.ShaderFunctions;
import arm.shader.MaterialParser;
import arm.render.RenderPathPaint;
import arm.util.RenderUtil;
import arm.Enums;

class MakeMaterial {

	public static var defaultScon: ShaderContext = null;
	public static var defaultMcon: MaterialContext = null;

	public static var heightUsed = false;
	public static var emisUsed = false;
	public static var subsUsed = false;

	static function getMOut(): Bool {
		for (n in UINodes.inst.getCanvasMaterial().nodes) if (n.type == "OUTPUT_MATERIAL_PBR") return true;
		return false;
	}

	public static function parseMeshMaterial() {
		var m = Project.materials[0].data;

		for (c in m.shader.contexts) {
			if (c.raw.name == "mesh") {
				m.shader.raw.contexts.remove(c.raw);
				m.shader.contexts.remove(c);
				deleteContext(c);
				break;
			}
		}

		if (MakeMesh.layerPassCount > 1) {
			var i = 0;
			while (i < m.shader.contexts.length) {
				var c = m.shader.contexts[i];
				for (j in 1...MakeMesh.layerPassCount) {
					if (c.raw.name == "mesh" + j) {
						m.shader.raw.contexts.remove(c.raw);
						m.shader.contexts.remove(c);
						deleteContext(c);
						i--;
						break;
					}
				}
				i++;
			}

			i = 0;
			while (i < m.contexts.length) {
				var c = m.contexts[i];
				for (j in 1...MakeMesh.layerPassCount) {
					if (c.raw.name == "mesh" + j) {
						m.raw.contexts.remove(c.raw);
						m.contexts.remove(c);
						i--;
						break;
					}
				}
				i++;
			}
		}

		var con = MakeMesh.run(new NodeShaderData({ name: "Material", canvas: null }));
		var scon = new ShaderContext(con.data, function(scon: ShaderContext){});
		scon.overrideContext = {};
		if (con.frag.sharedSamplers.length > 0) {
			var sampler = con.frag.sharedSamplers[0];
			scon.overrideContext.shared_sampler = sampler.substr(sampler.lastIndexOf(" ") + 1);
		}
		if (!Context.textureFilter) {
			scon.overrideContext.filter = "point";
		}
		m.shader.raw.contexts.push(scon.raw);
		m.shader.contexts.push(scon);

		for (i in 1...MakeMesh.layerPassCount) {
			var con = MakeMesh.run(new NodeShaderData({ name: "Material", canvas: null }), i);
			var scon = new ShaderContext(con.data, function(scon: ShaderContext){});
			scon.overrideContext = {};
			if (con.frag.sharedSamplers.length > 0) {
				var sampler = con.frag.sharedSamplers[0];
				scon.overrideContext.shared_sampler = sampler.substr(sampler.lastIndexOf(" ") + 1);
			}
			if (!Context.textureFilter) {
				scon.overrideContext.filter = "point";
			}
			m.shader.raw.contexts.push(scon.raw);
			m.shader.contexts.push(scon);

			var mcon = new MaterialContext({ name: "mesh" + i, bind_textures: [] }, function(self: MaterialContext) {});
			m.raw.contexts.push(mcon.raw);
			m.contexts.push(mcon);
		}

		Context.ddirty = 2;

		#if rp_voxels
		makeVoxel(m);
		#end

		#if (kha_direct3d12 || kha_vulkan)
		arm.render.RenderPathRaytrace.dirty = 1;
		#end
	}

	public static function parseParticleMaterial() {
		var m = Context.particleMaterial;
		var sc: ShaderContext = null;
		for (c in m.shader.contexts) {
			if (c.raw.name == "mesh") {
				sc = c;
				break;
			}
		}
		if (sc != null) {
			m.shader.raw.contexts.remove(sc.raw);
			m.shader.contexts.remove(sc);
		}
		var con = MakeParticle.run(new NodeShaderData({ name: "MaterialParticle", canvas: null }));
		if (sc != null) deleteContext(sc);
		sc = new ShaderContext(con.data, function(sc: ShaderContext){});
		m.shader.raw.contexts.push(sc.raw);
		m.shader.contexts.push(sc);
	}

	public static function parseMeshPreviewMaterial() {
		if (!getMOut()) return;

		var m = Project.materials[0].data;
		var scon: ShaderContext = null;
		for (c in m.shader.contexts) {
			if (c.raw.name == "mesh") {
				scon = c;
				break;
			}
		}
		m.shader.raw.contexts.remove(scon.raw);
		m.shader.contexts.remove(scon);

		var mcon: TMaterialContext = { name: "mesh", bind_textures: [] };

		var sd = new NodeShaderData({ name: "Material", canvas: null });
		var con = MakeMeshPreview.run(sd, mcon);

		for (i in 0...m.contexts.length) {
			if (m.contexts[i].raw.name == "mesh") {
				m.contexts[i] = new MaterialContext(mcon, function(self: MaterialContext) {});
				break;
			}
		}

		if (scon != null) deleteContext(scon);

		var compileError = false;
		scon = new ShaderContext(con.data, function(scon: ShaderContext) {
			if (scon == null) compileError = true;
		});
		if (compileError) return;

		m.shader.raw.contexts.push(scon.raw);
		m.shader.contexts.push(scon);
	}

	#if rp_voxels
	static function makeVoxel(m: MaterialData) {
		var rebuild = heightUsed;
		if (Config.raw.rp_gi != false && rebuild) {
			var scon: ShaderContext = null;
			for (c in m.shader.contexts) {
				if (c.raw.name == "voxel") {
					scon = c;
					break;
				}
			}
			if (scon != null) MakeVoxel.run(scon);
		}
	}
	#end

	public static function parsePaintMaterial(bakePreviews = true) {
		if (!getMOut()) return;

		if (bakePreviews) {
			var current = @:privateAccess kha.graphics2.Graphics.current;
			if (current != null) current.end();
			bakeNodePreviews();
			if (current != null) current.begin(false);
		}

		var m = Project.materials[0].data;
		var scon: ShaderContext = null;
		var mcon: MaterialContext = null;
		for (c in m.shader.contexts) {
			if (c.raw.name == "paint") {
				m.shader.raw.contexts.remove(c.raw);
				m.shader.contexts.remove(c);
				if (c != defaultScon) deleteContext(c);
				break;
			}
		}
		for (c in m.contexts) {
			if (c.raw.name == "paint") {
				m.raw.contexts.remove(c.raw);
				m.contexts.remove(c);
				break;
			}
		}

		var sdata = new NodeShaderData({ name: "Material", canvas: UINodes.inst.getCanvasMaterial() });
		var mcon: TMaterialContext = { name: "paint", bind_textures: [] };
		var con = MakePaint.run(sdata, mcon);

		var compileError = false;
		var scon = new ShaderContext(con.data, function(scon: ShaderContext) {
			if (scon == null) compileError = true;
		});
		if (compileError) return;
		scon.overrideContext = {};
		scon.overrideContext.addressing = "repeat";
		var mcon = new MaterialContext(mcon, function(mcon: MaterialContext) {});

		m.shader.raw.contexts.push(scon.raw);
		m.shader.contexts.push(scon);
		m.raw.contexts.push(mcon.raw);
		m.contexts.push(mcon);

		if (defaultScon == null) defaultScon = scon;
		if (defaultMcon == null) defaultMcon = mcon;
	}

	static function bakeNodePreviews() {
		Context.nodePreviewsUsed = [];
		if (Context.nodePreviews == null) Context.nodePreviews = [];
		traverseNodes(UINodes.inst.getCanvasMaterial().nodes, null, []);
		for (key in Context.nodePreviews.keys()) {
			if (Context.nodePreviewsUsed.indexOf(key) == -1) {
				var image = Context.nodePreviews.get(key);
				App.notifyOnNextFrame(image.unload);
				Context.nodePreviews.remove(key);
			}
		}
	}

	static function traverseNodes(nodes: Array<TNode>, group: TNodeCanvas, parents: Array<TNode>) {
		for (node in nodes) {
			bakeNodePreview(node, group, parents);
			if (node.type == "GROUP") {
				for (g in Project.materialGroups) {
					if (g.canvas.name == node.name) {
						parents.push(node);
						traverseNodes(g.canvas.nodes, g.canvas, parents);
						parents.pop();
						break;
					}
				}
			}
		}
	}

	static function bakeNodePreview(node: TNode, group: TNodeCanvas, parents: Array<TNode>) {
		if (node.type == "BLUR") {
			var id = MaterialParser.node_name(node, parents);
			var image = Context.nodePreviews.get(id);
			Context.nodePreviewsUsed.push(id);
			var resX = Std.int(Config.getTextureResX() / 4);
			var resY = Std.int(Config.getTextureResY() / 4);
			if (image == null || image.width != resX || image.height != resY) {
				if (image != null) image.unload();
				image = kha.Image.createRenderTarget(resX, resY);
				Context.nodePreviews.set(id, image);
			}

			MaterialParser.blur_passthrough = true;
			RenderUtil.makeNodePreview(UINodes.inst.getCanvasMaterial(), node, image, group, parents);
			MaterialParser.blur_passthrough = false;
		}
		else if (node.type == "DIRECT_WARP") {
			var id = MaterialParser.node_name(node, parents);
			var image = Context.nodePreviews.get(id);
			Context.nodePreviewsUsed.push(id);
			var resX = Std.int(Config.getTextureResX());
			var resY = Std.int(Config.getTextureResY());
			if (image == null || image.width != resX || image.height != resY) {
				if (image != null) image.unload();
				image = kha.Image.createRenderTarget(resX, resY);
				Context.nodePreviews.set(id, image);
			}

			MaterialParser.warp_passthrough = true;
			RenderUtil.makeNodePreview(UINodes.inst.getCanvasMaterial(), node, image, group, parents);
			MaterialParser.warp_passthrough = false;
		}
		else if (node.type == "BAKE_CURVATURE") {
			var id = MaterialParser.node_name(node, parents);
			var image = Context.nodePreviews.get(id);
			Context.nodePreviewsUsed.push(id);
			var resX = Std.int(Config.getTextureResX());
			var resY = Std.int(Config.getTextureResY());
			if (image == null || image.width != resX || image.height != resY) {
				if (image != null) image.unload();
				image = kha.Image.createRenderTarget(resX, resY, kha.graphics4.TextureFormat.L8);
				Context.nodePreviews.set(id, image);
			}

			if (RenderPathPaint.liveLayer == null) {
				RenderPathPaint.liveLayer = new arm.data.LayerSlot("_live");
			}

			var _space = UIHeader.inst.worktab.position;
			var _tool = Context.tool;
			var _bakeType = Context.bakeType;
			UIHeader.inst.worktab.position = SpacePaint;
			Context.tool = ToolBake;
			Context.bakeType = BakeCurvature;

			MaterialParser.bake_passthrough = true;
			MaterialParser.start_node = node;
			MaterialParser.start_group = group;
			MaterialParser.start_parents = parents;
			parsePaintMaterial(false);
			MaterialParser.bake_passthrough = false;
			MaterialParser.start_node = null;
			MaterialParser.start_group = null;
			MaterialParser.start_parents = null;
			Context.pdirty = 1;
			RenderPathPaint.useLiveLayer(true);
			RenderPathPaint.commandsPaint(false);
			RenderPathPaint.dilate(true, false);
			RenderPathPaint.useLiveLayer(false);
			Context.pdirty = 0;

			UIHeader.inst.worktab.position = _space;
			Context.tool = _tool;
			Context.bakeType = _bakeType;
			parsePaintMaterial(false);

			var rts = RenderPath.active.renderTargets;
			var texpaint_live = rts.get("texpaint_live");

			image.g2.begin(false);
			image.g2.drawImage(texpaint_live.image, 0, 0);
			image.g2.end();
		}
	}

	public static function parseNodePreviewMaterial(node: TNode, group: TNodeCanvas = null, parents: Array<TNode> = null): { scon: ShaderContext, mcon: MaterialContext } {
		if (node.outputs.length == 0) return null;
		var sdata = new NodeShaderData({ name: "Material", canvas: UINodes.inst.getCanvasMaterial() });
		var mcon_raw: TMaterialContext = { name: "mesh", bind_textures: [] };
		var con = MakeNodePreview.run(sdata, mcon_raw, node, group, parents);
		var compileError = false;
		var scon = new ShaderContext(con.data, function(scon: ShaderContext) {
			if (scon == null) compileError = true;
		});
		if (compileError) return null;
		var mcon = new MaterialContext(mcon_raw, function(mcon: MaterialContext) {});
		return { scon: scon, mcon: mcon };
	}

	public static function parseBrush() {
		Brush.parse(Context.brush.canvas, false);
	}

	public static function blendMode(frag: NodeShader, blending: Int, cola: String, colb: String, opac: String): String {
		if (blending == BlendMix) {
			return 'mix($cola, $colb, $opac)';
		}
		else if (blending == BlendDarken) {
			return 'mix($cola, min($cola, $colb), $opac)';
		}
		else if (blending == BlendMultiply) {
			return 'mix($cola, $cola * $colb, $opac)';
		}
		else if (blending == BlendBurn) {
			return 'mix($cola, vec3(1.0, 1.0, 1.0) - (vec3(1.0, 1.0, 1.0) - $cola) / $colb, $opac)';
		}
		else if (blending == BlendLighten) {
			return 'max($cola, $colb * $opac)';
		}
		else if (blending == BlendScreen) {
			return '(vec3(1.0, 1.0, 1.0) - (vec3(1.0 - $opac, 1.0 - $opac, 1.0 - $opac) + $opac * (vec3(1.0, 1.0, 1.0) - $colb)) * (vec3(1.0, 1.0, 1.0) - $cola))';
		}
		else if (blending == BlendDodge) {
			return 'mix($cola, $cola / (vec3(1.0, 1.0, 1.0) - $colb), $opac)';
		}
		else if (blending == BlendAdd) {
			return 'mix($cola, $cola + $colb, $opac)';
		}
		else if (blending == BlendOverlay) {
			return 'mix($cola, vec3(
				$cola.r < 0.5 ? 2.0 * $cola.r * $colb.r : 1.0 - 2.0 * (1.0 - $cola.r) * (1.0 - $colb.r),
				$cola.g < 0.5 ? 2.0 * $cola.g * $colb.g : 1.0 - 2.0 * (1.0 - $cola.g) * (1.0 - $colb.g),
				$cola.b < 0.5 ? 2.0 * $cola.b * $colb.b : 1.0 - 2.0 * (1.0 - $cola.b) * (1.0 - $colb.b)
			), $opac)';
		}
		else if (blending == BlendSoftLight) {
			return '((1.0 - $opac) * $cola + $opac * ((vec3(1.0, 1.0, 1.0) - $cola) * $colb * $cola + $cola * (vec3(1.0, 1.0, 1.0) - (vec3(1.0, 1.0, 1.0) - $colb) * (vec3(1.0, 1.0, 1.0) - $cola))))';
		}
		else if (blending == BlendLinearLight) {
			return '($cola + $opac * (vec3(2.0, 2.0, 2.0) * ($colb - vec3(0.5, 0.5, 0.5))))';
		}
		else if (blending == BlendDifference) {
			return 'mix($cola, abs($cola - $colb), $opac)';
		}
		else if (blending == BlendSubtract) {
			return 'mix($cola, $cola - $colb, $opac)';
		}
		else if (blending == BlendDivide) {
			return 'vec3(1.0 - $opac, 1.0 - $opac, 1.0 - $opac) * $cola + vec3($opac, $opac, $opac) * $cola / $colb';
		}
		else if (blending == BlendHue) {
			frag.add_function(ShaderFunctions.str_hue_sat);
			return 'mix($cola, hsv_to_rgb(vec3(rgb_to_hsv($colb).r, rgb_to_hsv($cola).g, rgb_to_hsv($cola).b)), $opac)';
		}
		else if (blending == BlendSaturation) {
			frag.add_function(ShaderFunctions.str_hue_sat);
			return 'mix($cola, hsv_to_rgb(vec3(rgb_to_hsv($cola).r, rgb_to_hsv($colb).g, rgb_to_hsv($cola).b)), $opac)';
		}
		else if (blending == BlendColor) {
			frag.add_function(ShaderFunctions.str_hue_sat);
			return 'mix($cola, hsv_to_rgb(vec3(rgb_to_hsv($colb).r, rgb_to_hsv($colb).g, rgb_to_hsv($cola).b)), $opac)';
		}
		else { // BlendValue
			frag.add_function(ShaderFunctions.str_hue_sat);
			return 'mix($cola, hsv_to_rgb(vec3(rgb_to_hsv($cola).r, rgb_to_hsv($cola).g, rgb_to_hsv($colb).b)), $opac)';
		}
	}

	public static function blendModeMask(frag: NodeShader, blending: Int, cola: String, colb: String, opac: String): String {
		if (blending == BlendMix) {
			return 'mix($cola, $colb, $opac)';
		}
		else if (blending == BlendDarken) {
			return 'mix($cola, min($cola, $colb), $opac)';
		}
		else if (blending == BlendMultiply) {
			return 'mix($cola, $cola * $colb, $opac)';
		}
		else if (blending == BlendBurn) {
			return 'mix($cola, 1.0 - (1.0 - $cola) / $colb, $opac)';
		}
		else if (blending == BlendLighten) {
			return 'max($cola, $colb * $opac)';
		}
		else if (blending == BlendScreen) {
			return '(1.0 - ((1.0 - $opac) + $opac * (1.0 - $colb)) * (1.0 - $cola))';
		}
		else if (blending == BlendDodge) {
			return 'mix($cola, $cola / (1.0 - $colb), $opac)';
		}
		else if (blending == BlendAdd) {
			return 'mix($cola, $cola + $colb, $opac)';
		}
		else if (blending == BlendOverlay) {
			return 'mix($cola, $cola < 0.5 ? 2.0 * $cola * $colb : 1.0 - 2.0 * (1.0 - $cola) * (1.0 - $colb), $opac)';
		}
		else if (blending == BlendSoftLight) {
			return '((1.0 - $opac) * $cola + $opac * ((1.0 - $cola) * $colb * $cola + $cola * (1.0 - (1.0 - $colb) * (1.0 - $cola))))';
		}
		else if (blending == BlendLinearLight) {
			return '($cola + $opac * (2.0 * ($colb - 0.5)))';
		}
		else if (blending == BlendDifference) {
			return 'mix($cola, abs($cola - $colb), $opac)';
		}
		else if (blending == BlendSubtract) {
			return 'mix($cola, $cola - $colb, $opac)';
		}
		else if (blending == BlendDivide) {
			return '(1.0 - $opac) * $cola + $opac * $cola / $colb';
		}
		else { // BlendHue, BlendSaturation, BlendColor, BlendValue
			return 'mix($cola, $colb, $opac)';
		}
	}

	public static inline function getDisplaceStrength():Float {
		var sc = Context.mainObject().transform.scale.x;
		return Config.raw.displace_strength * 0.02 * sc;
	}

	public static inline function voxelgiHalfExtents():String {
		var ext = Context.vxaoExt;
		return 'const vec3 voxelgiHalfExtents = vec3($ext, $ext, $ext);';
	}

	static function deleteContext(c: ShaderContext) {
		arm.App.notifyOnNextFrame(function() { // Ensure pipeline is no longer in use
			c.delete();
		});
	}
}
