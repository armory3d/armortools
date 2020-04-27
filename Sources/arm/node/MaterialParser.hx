package arm.node;

import iron.data.SceneFormat;
import iron.data.ShaderData;
import iron.data.MaterialData;
import arm.ui.UIHeader;
import arm.ui.UINodes;
import arm.node.MaterialShader;
import arm.Enums;

class MaterialParser {

	public static var defaultScon: ShaderContext = null;
	public static var defaultMcon: MaterialContext = null;

	static function getMOut(): Bool {
		for (n in UINodes.inst.getCanvasMaterial().nodes) if (n.type == "OUTPUT_MATERIAL_PBR") return true;
		return false;
	}

	public static function parseMeshMaterial() {
		if (UIHeader.inst.worktab.position == SpaceRender) return;
		var m = Project.materials[0].data;
		var scon: ShaderContext = null;
		for (c in m.shader.contexts) if (c.raw.name == "mesh") { scon = c; break; }
		if (scon != null) {
			m.shader.raw.contexts.remove(scon.raw);
			m.shader.contexts.remove(scon);
		}
		var con = MaterialBuilder.make_mesh(new MaterialShaderData({name: "Material", canvas: null}));
		if (scon != null) scon.delete();
		scon = new ShaderContext(con.data, function(scon: ShaderContext){});
		scon.overrideContext = {}
		if (con.frag.sharedSamplers.length > 0) {
			var sampler = con.frag.sharedSamplers[0];
			scon.overrideContext.shared_sampler = sampler.substr(sampler.lastIndexOf(" ") + 1);
		}
		if (!Context.textureFilter) {
			scon.overrideContext.filter = "point";
		}
		m.shader.raw.contexts.push(scon.raw);
		m.shader.contexts.push(scon);
		Context.ddirty = 2;

		makeVoxel(m);
	}

	public static function parseParticleMaterial() {
		var m = Context.particleMaterial;
		var sc: ShaderContext = null;
		for (c in m.shader.contexts) if (c.raw.name == "mesh") { sc = c; break; }
		if (sc != null) {
			m.shader.raw.contexts.remove(sc.raw);
			m.shader.contexts.remove(sc);
		}
		var con = MaterialBuilder.make_particle(new MaterialShaderData({name: "MaterialParticle", canvas: null}));
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

		var sd = new MaterialShaderData({name: "Material", canvas: null});
		var con = MaterialBuilder.make_mesh_preview(sd, mcon);

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

		if (UIHeader.inst.worktab.position == SpaceRender) {
			makeVoxel(m);
		}
	}

	static function makeVoxel(m: MaterialData) {
		#if rp_voxelao
		var rebuild = MaterialBuilder.heightUsed;
		#if arm_world
		rebuild = true;
		#end
		if (Config.raw.rp_gi != false && rebuild) {
			var scon: ShaderContext = null;
			for (c in m.shader.contexts) if (c.raw.name == "voxel") { scon = c; break; }
			if (scon != null) MaterialBuilder.make_voxel(scon);
		}
		#end
	}

	public static function parsePaintMaterial() {
		if (!getMOut()) return;

		if (UIHeader.inst.worktab.position == SpaceRender) {
			parseMeshPreviewMaterial();
			return;
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

		var sdata = new MaterialShaderData({ name: "Material", canvas: UINodes.inst.getCanvasMaterial() });
		var mcon = { name: "paint", bind_textures: [] };
		var con = MaterialBuilder.make_paint(sdata, mcon);

		var compileError = false;
		var scon = new ShaderContext(con.data, function(scon: ShaderContext) {
			if (scon == null) compileError = true;
		});
		if (compileError) return;
		scon.overrideContext = {}
		scon.overrideContext.addressing = "repeat";
		var mcon = new MaterialContext(mcon, function(mcon: MaterialContext) {});

		m.shader.raw.contexts.push(scon.raw);
		m.shader.contexts.push(scon);
		m.raw.contexts.push(mcon.raw);
		m.contexts.push(mcon);

		if (defaultScon == null) defaultScon = scon;
		if (defaultMcon == null) defaultMcon = mcon;
	}

	public static function parseBrush() {
		Brush.parse(Context.brush.canvas, false);
	}
}
