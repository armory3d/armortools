
function util_particle_init() {
	if (context_raw.particle_material != null) {
		return;
	}

	let raw: particle_data_t = {
		name: "Particles",
		type: 0,
		loop: false,
		count: 1000,
		frame_start: 0,
		frame_end: 1000,
		lifetime: 400,
		lifetime_random: 0.5,
		emit_from: 1,
		object_align_factor: f32_array_create_xyz(0, 0, -40),
		factor_random: 2.0,
		physics_type: 0,
		particle_size: 1.0,
		size_random: 0,
		mass: 1,
		instance_object: ".Particle",
		weight_gravity: 1
	};
	_scene_raw.particle_datas = [raw];
	let particle_refs: particle_ref_t[] = [
		{
			name: "Particles",
			particle: "Particles",
			seed: 0
		}
	];

	{
		let t: render_target_t = render_target_create();
		t.name = "texparticle";
		t.width = 0;
		t.height = 0;
		t.format = "R8";
		t.scale = render_path_base_get_super_sampling();
		render_path_create_render_target(t);
	}

	for (let i: i32 = 0; i < _scene_raw.material_datas.length; ++i) {
		let mat: material_data_t = _scene_raw.material_datas[i];
		if (mat.name == "Material2") {
			let m: material_data_t = util_clone_material_data(mat);
			m.name = "MaterialParticle";
			array_push(_scene_raw.material_datas, m);
			break;
		}
	}

	let md: material_data_t = data_get_material("Scene", "MaterialParticle");
	context_raw.particle_material = md;

	for (let i: i32 = 0; i < _scene_raw.objects.length; ++i) {
		let obj: obj_t = _scene_raw.objects[i];
		if (obj.name == ".Sphere") {
			let particle: obj_t = util_clone_obj(obj);
			particle.name = ".Particle";
			if (particle.particles == null) {
				particle.particles = {};
			}
			particle.particles.is_particle = true;
			particle.material_refs = ["MaterialParticle"];
			array_push(_scene_raw.objects, particle);
			for (let i: i32 = 0; i < 16; ++i) {
				particle.transform[i] *= 0.01;
			}
			break;
		}
	}

	let o: object_t = scene_spawn_object(".Sphere");
	let mo: mesh_object_t = o.ext;
	mo.base.name = ".ParticleEmitter";
	mo.base.raw = util_clone_obj(mo.base.raw);
	let oraw: obj_t = mo.base.raw;
	oraw.particles = {};
	mo.base.raw.particles.refs = particle_refs;
	///if arm_particles
	mesh_object_setup_particle_system(mo, "Scene", particle_refs[0]);
	///end
}

///if arm_physics

function util_particle_init_physics() {
	if (physics_world_active != null) {
		util_particle_init_mesh();
		return;
	}

	physics_world_create();
	util_particle_init_mesh();
}

function util_particle_init_mesh() {
	if (context_raw.paint_body != null) {
		return;
	}

	let po: mesh_object_t = context_raw.merged_object != null ? context_raw.merged_object : context_raw.paint_object;

	po.base.transform.scale.x = po.base.parent.transform.scale.x;
	po.base.transform.scale.y = po.base.parent.transform.scale.y;
	po.base.transform.scale.z = po.base.parent.transform.scale.z;

	context_raw.paint_body = physics_body_create();
	context_raw.paint_body.shape = physics_shape_t.MESH;
	physics_body_init(context_raw.paint_body, po.base);
}

///end
