
function util_particle_init() {
	if (context_raw.particle_material != null) {
		return;
	}

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
