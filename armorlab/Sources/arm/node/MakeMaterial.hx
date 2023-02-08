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
import arm.Enums;

class MakeMaterial {

	public static var defaultScon: ShaderContext = null;
	public static var defaultMcon: MaterialContext = null;
	public static var heightUsed = false;

	public static function parseMeshMaterial() {
		var m = Project.materialData;

		for (c in m.shader.contexts) {
			if (c.raw.name == "mesh") {
				m.shader.raw.contexts.remove(c.raw);
				m.shader.contexts.remove(c);
				deleteContext(c);
				break;
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
		scon.overrideContext.addressing = "repeat";
		m.shader.raw.contexts.push(scon.raw);
		m.shader.contexts.push(scon);

		Context.ddirty = 2;

		#if rp_voxels
		makeVoxel(m);
		#end

		#if (kha_direct3d12 || kha_vulkan)
		arm.render.RenderPathRaytrace.dirty = 1;
		#end
	}

	#if rp_voxels
	static function makeVoxel(m: MaterialData) {
		// var rebuild = heightUsed;
		// if (Config.raw.rp_gi != false && rebuild) {
		// 	var scon: ShaderContext = null;
		// 	for (c in m.shader.contexts) {
		// 		if (c.raw.name == "voxel") {
		// 			scon = c;
		// 			break;
		// 		}
		// 	}
		// 	if (scon != null) MakeVoxel.run(scon);
		// }
	}
	#end

	public static function parsePaintMaterial() {
		var m = Project.materialData;
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

		var sdata = new NodeShaderData({ name: "Material", canvas: null });
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
