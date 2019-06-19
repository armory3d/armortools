package arm;

import iron.RenderPath;
import iron.data.SceneFormat;
import iron.object.Object;
import iron.object.MeshObject;
import arm.ui.UITrait;

class Tool {

	static function f32(ar:Array<kha.FastFloat>):kha.arrays.Float32Array {
		var res = new kha.arrays.Float32Array(ar.length);
		for (i in 0...ar.length) res[i] = ar[i];
		return res;
	}

	public static function initParticle() {
		if (UITrait.inst.particleMaterial != null) return;

		var raw:TParticleData = {
			name: "Particles",
			type: 0,
			loop: false,
			render_emitter: false,
			count: 1000,
			frame_start: 0,
			frame_end: 1000,
			lifetime: 400,
			lifetime_random: 0.5,
			emit_from: 1,
			object_align_factor: f32([0, 0, -40]),
			factor_random: 2.0,
			physics_type: 0,
			particle_size: 1.0,
			size_random: 0,
			mass: 1,
			instance_object: ".Particle",
			weight_gravity: 1
		};
		iron.Scene.active.raw.particle_datas = [raw];
		var particle_refs:Array<TParticleReference> = [
			{
				name: "Particles",
				particle: "Particles",
				seed: 0
			}
		];

		{
			var t = new RenderTargetRaw();
			t.name = "texparticle";
			t.width = 0;
			t.height = 0;
			t.format = 'R8';
			t.scale = armory.renderpath.Inc.getSuperSampling();
			RenderPath.active.createRenderTarget(t);
		}

		for (mat in iron.Scene.active.raw.material_datas) {
			if (mat.name == 'Material2') {
				var m:TMaterialData = haxe.Json.parse(haxe.Json.stringify(mat));
				m.name = 'MaterialParticle';
				iron.Scene.active.raw.material_datas.push(m);
				break;
			}
		}

		iron.data.Data.getMaterial("Scene", "MaterialParticle", function(md:iron.data.MaterialData) {
			UITrait.inst.particleMaterial = md;

			for (obj in iron.Scene.active.raw.objects) {
				if (obj.name == '.Sphere') {
					var particle:TObj = haxe.Json.parse(haxe.Json.stringify(obj));
					particle.name = '.Particle';
					particle.is_particle = true;
					particle.material_refs = ['MaterialParticle'];
					iron.Scene.active.raw.objects.push(particle);
					for (i in 0...16) particle.transform.values[i] *= 0.01;
					break;
				}
			}

			iron.Scene.active.spawnObject(".Sphere", null, function(o:Object) {
				var mo:MeshObject = cast o;
				mo.name = ".ParticleEmitter";
				mo.raw.particle_refs = particle_refs;
				mo.setupParticleSystem("Scene", particle_refs[0]);
			});
		});
	}

	@:access(zui.Zui)
	public static function getTextToolFont():kha.Font {
		var fontName = arm.io.Importer.fontList[UITrait.inst.textToolHandle.position];
		if (fontName == 'default.ttf') return UITrait.inst.ui.ops.font;
		return arm.io.Importer.fontMap.get(fontName);
	}
}

@:enum abstract PaintTool(Int) from Int to Int {
	var ToolBrush = 0;
	var ToolEraser = 1;
	var ToolFill = 2;
	var ToolDecal = 3;
	var ToolText = 4;
	var ToolClone = 5;
	var ToolBlur = 6;
	var ToolParticle = 7;
	var ToolBake = 8;
	var ToolColorId = 9;
	var ToolPicker = 10;
}

@:enum abstract SceneTool(Int) from Int to Int {
	var ToolGizmo = 0;
}

@:enum abstract Workspace(Int) from Int to Int {
	var SpacePaint = 0;
	// var SpaceSculpt = 1;
	var SpaceScene = 1;
}
