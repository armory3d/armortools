
class UtilParticle {

	static initParticle = () => {
		if (Context.raw.particleMaterial != null) return;

		let raw: TParticleData = {
			name: "Particles",
			type: 0,
			loop: false,
			count: 1000,
			frame_start: 0,
			frame_end: 1000,
			lifetime: 400,
			lifetime_random: 0.5,
			emit_from: 1,
			object_align_factor: new Float32Array([0, 0, -40]),
			factor_random: 2.0,
			physics_type: 0,
			particle_size: 1.0,
			size_random: 0,
			mass: 1,
			instance_object: ".Particle",
			weight_gravity: 1
		};
		Scene.active.raw.particle_datas = [raw];
		let particle_refs: TParticleReference[] = [
			{
				name: "Particles",
				particle: "Particles",
				seed: 0
			}
		];

		{
			let t = new RenderTargetRaw();
			t.name = "texparticle";
			t.width = 0;
			t.height = 0;
			t.format = "R8";
			t.scale = RenderPathBase.getSuperSampling();
			RenderPath.active.createRenderTarget(t);
		}

		for (let mat of Scene.active.raw.material_datas) {
			if (mat.name == "Material2") {
				let m: TMaterialData = JSON.parse(JSON.stringify(mat));
				m.name = "MaterialParticle";
				Scene.active.raw.material_datas.push(m);
				break;
			}
		}

		Data.getMaterial("Scene", "MaterialParticle", (md: MaterialData) => {
			Context.raw.particleMaterial = md;

			for (let obj of Scene.active.raw.objects) {
				if (obj.name == ".Sphere") {
					let particle: TObj = JSON.parse(JSON.stringify(obj));
					particle.name = ".Particle";
					particle.is_particle = true;
					particle.material_refs = ["MaterialParticle"];
					Scene.active.raw.objects.push(particle);
					for (let i = 0; i < 16; ++i) particle.transform.values[i] *= 0.01;
					break;
				}
			}

			Scene.active.spawnObject(".Sphere", null, (o: BaseObject) => {
				let mo: MeshObject = o as MeshObject;
				mo.name = ".ParticleEmitter";
				mo.raw = JSON.parse(JSON.stringify(mo.raw));
				mo.raw.particle_refs = particle_refs;
				///if arm_particles
				mo.setupParticleSystem("Scene", particle_refs[0]);
				///end
			});
		});
	}

	///if arm_physics

	static initParticlePhysics = () => {
		if (PhysicsWorld.active != null) {
			UtilParticle.initParticleMesh();
			return;
		}

		PhysicsWorld.load(() => {
			PhysicsWorld.create();
			UtilParticle.initParticleMesh();
		});
	}

	static initParticleMesh = () => {
		if (Context.raw.paintBody != null) return;

		let po = Context.raw.mergedObject != null ? Context.raw.mergedObject : Context.raw.paintObject;

		po.transform.scale.x = po.parent.transform.scale.x;
		po.transform.scale.y = po.parent.transform.scale.y;
		po.transform.scale.z = po.parent.transform.scale.z;

		Context.raw.paintBody = PhysicsBody.create();
		Context.raw.paintBody.shape = ShapeType.ShapeMesh;
		PhysicsBody.init(Context.raw.paintBody, po);
		po.addTrait(Context.raw.paintBody);
	}

	///end
}
