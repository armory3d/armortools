package arm.node;

import zui.Nodes;
import iron.data.SceneFormat;
import iron.data.ShaderData;
import iron.data.MaterialData;
import arm.ui.UIHeader;
import arm.ui.UINodes;
import arm.ui.UISidebar;
import arm.shader.NodeShader;
import arm.shader.NodeShaderContext;
import arm.shader.NodeShaderData;
import arm.shader.ShaderFunctions;
import arm.shader.MaterialParser;
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
		if (UIHeader.inst.worktab.position == SpaceRender) return;
		var m = Project.materials[0].data;

		for (c in m.shader.contexts) {
			if (c.raw.name == "mesh") {
				m.shader.raw.contexts.remove(c.raw);
				m.shader.contexts.remove(c);
				c.delete();
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
						c.delete();
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

		var con = MakeMesh.run(new NodeShaderData({name: "Material", canvas: null}));
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
			var con = MakeMesh.run(new NodeShaderData({name: "Material", canvas: null}), i);
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

		#if rp_voxelao
		makeVoxel(m);
		#end

		#if (kha_direct3d12 || kha_vulkan)
		arm.render.RenderPathRaytrace.dirty = 1;
		#end
	}

	public static function parseParticleMaterial() {
		var m = Context.particleMaterial;
		var sc: ShaderContext = null;
		for (c in m.shader.contexts) if (c.raw.name == "mesh") { sc = c; break; }
		if (sc != null) {
			m.shader.raw.contexts.remove(sc.raw);
			m.shader.contexts.remove(sc);
		}
		var con = MakeParticle.run(new NodeShaderData({name: "MaterialParticle", canvas: null}));
		if (sc != null) sc.delete();
		sc = new ShaderContext(con.data, function(sc: ShaderContext){});
		m.shader.raw.contexts.push(sc.raw);
		m.shader.contexts.push(sc);
	}

	public static function parseMeshPreviewMaterial() {
		if (!getMOut()) return;

		var m = UIHeader.inst.worktab.position == SpaceRender ? Context.materialScene.data : Project.materials[0].data;
		var scon: ShaderContext = null;
		for (c in m.shader.contexts) if (c.raw.name == "mesh") { scon = c; break; }
		m.shader.raw.contexts.remove(scon.raw);
		m.shader.contexts.remove(scon);

		var mcon: TMaterialContext = { name: "mesh", bind_textures: [] };

		var sd = new NodeShaderData({name: "Material", canvas: null});
		var con = MakeMeshPreview.run(sd, mcon);

		for (i in 0...m.contexts.length) {
			if (m.contexts[i].raw.name == "mesh") {
				m.contexts[i] = new MaterialContext(mcon, function(self: MaterialContext) {});
				break;
			}
		}

		if (scon != null) scon.delete();

		var compileError = false;
		scon = new ShaderContext(con.data, function(scon: ShaderContext) {
			if (scon == null) compileError = true;
		});
		if (compileError) return;

		m.shader.raw.contexts.push(scon.raw);
		m.shader.contexts.push(scon);

		#if rp_voxelao
		if (UIHeader.inst.worktab.position == SpaceRender) {
			makeVoxel(m);
		}
		#end
	}

	#if rp_voxelao
	static function makeVoxel(m: MaterialData) {
		var rebuild = heightUsed;
		if (Config.raw.rp_gi != false && rebuild) {
			var scon: ShaderContext = null;
			for (c in m.shader.contexts) if (c.raw.name == "voxel") { scon = c; break; }
			if (scon != null) MakeVoxel.run(scon);
		}
	}
	#end

	public static function parsePaintMaterial() {
		if (!getMOut()) return;

		if (UIHeader.inst.worktab.position == SpaceRender) {
			parseMeshPreviewMaterial();
			return;
		}

		{
			var current = @:privateAccess kha.graphics2.Graphics.current;
			if (current != null) current.end();
			bakeBlurNodes();
			if (current != null) current.begin(false);
		}

		var m = Project.materials[0].data;
		var scon: ShaderContext = null;
		var mcon: MaterialContext = null;
		for (c in m.shader.contexts) {
			if (c.raw.name == "paint") {
				m.shader.raw.contexts.remove(c.raw);
				m.shader.contexts.remove(c);
				if (c != defaultScon) c.delete();
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

	static function bakeBlurNodes() {
		if (Context.nodePreviewsBlur != null) {
			for (image in Context.nodePreviewsBlur) {
				image.unload();
			}
			Context.nodePreviewsBlur = null;
		}
		if (Context.nodePreviewsWarp != null) {
			for (img in Context.nodePreviewsWarp) {
				img.unload();
			}
			Context.nodePreviewsWarp = null;
		}
		for (node in UINodes.inst.getCanvasMaterial().nodes) {
			if (node.type == "BLUR") {
				if (Context.nodePreviewsBlur == null) {
					Context.nodePreviewsBlur = new Map();
				}
				var image = kha.Image.createRenderTarget(Std.int(Config.getTextureResX() / 4), Std.int(Config.getTextureResY() / 4));
				Context.nodePreviewsBlur.set(MaterialParser.node_name(node), image);
				MaterialParser.blur_passthrough = true;
				RenderUtil.makeNodePreview(UINodes.inst.getCanvasMaterial(), node, image);
				MaterialParser.blur_passthrough = false;
			}
			if (node.type ==  "DIRECT_WARP") {
				if (Context.nodePreviewsWarp == null) {
					Context.nodePreviewsWarp = new Map();
				}
				var image = kha.Image.createRenderTarget(Std.int(Config.getTextureResX()), Std.int(Config.getTextureResY()));
				Context.nodePreviewsWarp.set(MaterialParser.node_name(node), image);
				MaterialParser.warp_passthrough = true;
				RenderUtil.makeNodePreview(UINodes.inst.getCanvasMaterial(), node, image);
				MaterialParser.warp_passthrough = false;
			}
		}
	}

	public static function parseNodePreviewMaterial(node: TNode): { scon: ShaderContext, mcon: MaterialContext } {
		var sdata = new NodeShaderData({ name: "Material", canvas: UINodes.inst.getCanvasMaterial() });
		var mcon_raw: TMaterialContext = { name: "mesh", bind_textures: [] };
		var con = MakeNodePreview.run(sdata, mcon_raw, node);
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

	public static inline function getDisplaceStrength():Float {
		var sc = Context.mainObject().transform.scale.x;
		return Config.raw.displace_strength * 0.02 * sc;
	}

	public static inline function voxelgiHalfExtents():String {
		var ext = Context.vxaoExt;
		return 'const vec3 voxelgiHalfExtents = vec3($ext, $ext, $ext);';
	}
}
