package arm.shader;

import zui.Nodes;
import iron.data.SceneFormat;
import iron.data.ShaderData;
import iron.data.MaterialData;
import arm.ui.UINodes;
import arm.shader.NodeShaderContext;
import arm.shader.NodeShaderData;
import arm.shader.MaterialParser;
import arm.util.RenderUtil;

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
		if (!Context.raw.textureFilter) {
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
			if (!Context.raw.textureFilter) {
				scon.overrideContext.filter = "point";
			}
			m.shader.raw.contexts.push(scon.raw);
			m.shader.contexts.push(scon);

			var mcon = new MaterialContext({ name: "mesh" + i, bind_textures: [] }, function(self: MaterialContext) {});
			m.raw.contexts.push(mcon.raw);
			m.contexts.push(mcon);
		}

		Context.raw.ddirty = 2;

		#if rp_voxels
		makeVoxel(m);
		#end
	}

	public static function parseParticleMaterial() {
		var m = Context.raw.particleMaterial;
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
		var con = MakeSculpt.run(sdata, mcon);

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
		Context.raw.nodePreviewsUsed = [];
		if (Context.raw.nodePreviews == null) Context.raw.nodePreviews = [];
		traverseNodes(UINodes.inst.getCanvasMaterial().nodes, null, []);
		for (key in Context.raw.nodePreviews.keys()) {
			if (Context.raw.nodePreviewsUsed.indexOf(key) == -1) {
				var image = Context.raw.nodePreviews.get(key);
				App.notifyOnNextFrame(image.unload);
				Context.raw.nodePreviews.remove(key);
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
			var image = Context.raw.nodePreviews.get(id);
			Context.raw.nodePreviewsUsed.push(id);
			var resX = Std.int(Config.getTextureResX() / 4);
			var resY = Std.int(Config.getTextureResY() / 4);
			if (image == null || image.width != resX || image.height != resY) {
				if (image != null) image.unload();
				image = kha.Image.createRenderTarget(resX, resY);
				Context.raw.nodePreviews.set(id, image);
			}

			MaterialParser.blur_passthrough = true;
			RenderUtil.makeNodePreview(UINodes.inst.getCanvasMaterial(), node, image, group, parents);
			MaterialParser.blur_passthrough = false;
		}
		else if (node.type == "DIRECT_WARP") {
			var id = MaterialParser.node_name(node, parents);
			var image = Context.raw.nodePreviews.get(id);
			Context.raw.nodePreviewsUsed.push(id);
			var resX = Std.int(Config.getTextureResX());
			var resY = Std.int(Config.getTextureResY());
			if (image == null || image.width != resX || image.height != resY) {
				if (image != null) image.unload();
				image = kha.Image.createRenderTarget(resX, resY);
				Context.raw.nodePreviews.set(id, image);
			}

			MaterialParser.warp_passthrough = true;
			RenderUtil.makeNodePreview(UINodes.inst.getCanvasMaterial(), node, image, group, parents);
			MaterialParser.warp_passthrough = false;
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
		arm.logic.LogicParser.parse(Context.raw.brush.canvas, false);
	}

	public static inline function getDisplaceStrength():Float {
		var sc = Context.mainObject().transform.scale.x;
		return Config.raw.displace_strength * 0.02 * sc;
	}

	public static inline function voxelgiHalfExtents():String {
		var ext = Context.raw.vxaoExt;
		return 'const vec3 voxelgiHalfExtents = vec3($ext, $ext, $ext);';
	}

	static function deleteContext(c: ShaderContext) {
		arm.App.notifyOnNextFrame(function() { // Ensure pipeline is no longer in use
			c.delete();
		});
	}
}
