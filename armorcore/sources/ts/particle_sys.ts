
///if arm_particles

type particle_sys_t = {
	data?: particle_data_t;
	speed?: f32;
	particles?: particle_t[];
	ready?: bool;
	frame_rate?: i32;
	lifetime?: f32;
	animtime?: f32;
	time?: f32;
	spawn_rate?: f32;
	seed?: i32;

	r?: particle_data_t;
	gx?: f32;
	gy?: f32;
	gz?: f32;
	alignx?: f32;
	aligny?: f32;
	alignz?: f32;
	dimx?: f32;
	dimy?: f32;

	count?: i32;
	lap?: i32;
	lap_time?: f32;
	m?: mat4_t;

	owner_loc?: vec4_t;
	owner_rot?: quat_t;
	owner_scale?: vec4_t;
};

type particle_t = {
	i?: i32;
	x?: f32;
	y?: f32;
	z?: f32;
	camera_dist?: f32;
};

function particle_create(): particle_t {
	let raw: particle_t = {};
	raw.x = 0;
	raw.y = 0;
	raw.z = 0;
	return raw;
}

function particle_sys_create(scene_name: string, ref: particle_ref_t): particle_sys_t {
	let raw: particle_sys_t = {};
	raw.speed = 1.0;
	raw.frame_rate = 24;
	raw.time = 0.0;
	raw.seed = ref.seed;
	raw.count = 0;
	raw.lap = 0;
	raw.lap_time = 0.0;
	raw.m = mat4_identity();
	raw.owner_loc = vec4_create();
	raw.owner_rot = quat_create();
	raw.owner_scale = vec4_create();
	raw.particles = [];
	raw.ready = false;
	let b: particle_data_t = data_get_particle(scene_name, ref.particle);
	raw.data = b;
	raw.r = raw.data;
	raw.gx = 0;
	raw.gy = 0;
	raw.gz = -9.81 * raw.r.weight_gravity;
	raw.alignx = raw.r.object_align_factor[0] / 2;
	raw.aligny = raw.r.object_align_factor[1] / 2;
	raw.alignz = raw.r.object_align_factor[2] / 2;
	raw.lifetime = raw.r.lifetime / raw.frame_rate;
	raw.animtime = (raw.r.frame_end - raw.r.frame_start) / raw.frame_rate;
	raw.spawn_rate = ((raw.r.frame_end - raw.r.frame_start) / raw.r.count) / raw.frame_rate;
	for (let i: i32 = 0; i < raw.r.count; ++i) {
		let p: particle_t = particle_create();
		p.i = i;
		array_push(raw.particles, p);
	}
	raw.ready = true;
	return raw;
}

function particle_sys_pause(raw: particle_sys_t) {
	raw.lifetime = 0;
}

function particle_sys_resume(raw: particle_sys_t) {
	raw.lifetime = raw.r.lifetime / raw.frame_rate;
}

function particle_sys_update(raw: particle_sys_t, object: mesh_object_t, owner: mesh_object_t) {
	if (!raw.ready || object == null || raw.speed == 0.0) {
		return;
	}

	// Copy owner world transform but discard scale
	let dec: mat4_decomposed_t = mat4_decompose(owner.base.transform.world);
	raw.owner_loc = dec.loc;
	raw.owner_rot = dec.rot;
	raw.owner_scale = dec.scl;
	object.base.transform.loc = raw.owner_loc;
	object.base.transform.rot = raw.owner_rot;

	// Set particle size per particle system
	object.base.transform.scale = vec4_create(raw.r.particle_size, raw.r.particle_size, raw.r.particle_size, 1);

	transform_build_matrix(object.base.transform);
	transform_build_matrix(owner.base.transform);
	object.base.transform.dim = vec4_clone(owner.base.transform.dim);

	raw.dimx = object.base.transform.dim.x;
	raw.dimy = object.base.transform.dim.y;

	// Animate
	raw.time += time_real_delta() * raw.speed;
	raw.lap = math_floor(raw.time / raw.animtime);
	raw.lap_time = raw.time - raw.lap * raw.animtime;
	raw.count = math_floor(raw.lap_time / raw.spawn_rate);

	particle_sys_update_gpu(raw, object, owner);
}

function particle_sys_get_data(raw: particle_sys_t): mat4_t {
	let hair: bool = raw.r.type == 1;
	raw.m.m00 = raw.r.loop ? raw.animtime : -raw.animtime;
	raw.m.m01 = hair ? 1 / raw.particles.length : raw.spawn_rate;
	raw.m.m02 = hair ? 1 : raw.lifetime;
	raw.m.m03 = raw.particles.length;
	raw.m.m10 = hair ? 0 : raw.alignx;
	raw.m.m11 = hair ? 0 : raw.aligny;
	raw.m.m12 = hair ? 0 : raw.alignz;
	raw.m.m13 = hair ? 0 : raw.r.factor_random;
	raw.m.m20 = hair ? 0 : raw.gx * raw.r.mass;
	raw.m.m21 = hair ? 0 : raw.gy * raw.r.mass;
	raw.m.m22 = hair ? 0 : raw.gz * raw.r.mass;
	raw.m.m23 = hair ? 0 : raw.r.lifetime_random;
	raw.m.m30 = 1; // tilesx
	raw.m.m31 = 1; // tilesy
	raw.m.m32 = 1; // tilesframerate
	raw.m.m33 = hair ? 1 : raw.lap_time;
	return raw.m;
}

function particle_sys_update_gpu(raw: particle_sys_t, object: mesh_object_t, owner: mesh_object_t) {
	if (!object.data._.instanced) {
		particle_sys_setup_geom(raw, object, owner);
	}
	// GPU particles transform is attached to owner object
}

function particle_sys_rand(max: i32): i32 {
	return math_floor(math_random() * max);
}

function particle_sys_setup_geom(raw: particle_sys_t, object: mesh_object_t, owner: mesh_object_t) {
	let instanced_data: f32_array_t = f32_array_create(raw.particles.length * 3);
	let i: i32 = 0;

	let norm_fac: f32 = 1 / 32767; // pa.values are not normalized
	let scale_pos_owner: f32 = owner.data.scale_pos;
	let scale_pos_particle: f32 = object.data.scale_pos;
	let particle_size: f32 = raw.r.particle_size;
	let scale_fac: vec4_t = vec4_clone(owner.base.transform.scale);
	scale_fac = vec4_mult(scale_fac, scale_pos_owner / (particle_size * scale_pos_particle));

	if (raw.r.emit_from == 0) { // Vert
		let pa: vertex_array_t = mesh_data_get_vertex_array(owner.data, "pos");
		let size: i32 = mesh_data_get_vertex_size(pa.data);

		for (let x: i32 = 0; x < raw.particles.length; ++x) {
			let j: i32 = math_floor(particle_sys_fhash(i) * (pa.values.length / size));
			instanced_data[i] = pa.values[j * size    ] * norm_fac * scale_fac.x;
			i++;
			instanced_data[i] = pa.values[j * size + 1] * norm_fac * scale_fac.y;
			i++;
			instanced_data[i] = pa.values[j * size + 2] * norm_fac * scale_fac.z;
			i++;
		}
	}
	else if (raw.r.emit_from == 1) { // Face
		let positions: i16_array_t = mesh_data_get_vertex_array(owner.data, "pos").values;

		for (let x: i32 = 0; x < raw.particles.length; ++x) {
			// Choose random index array (there is one per material) and random face
			let ia: u32_array_t = owner.data._.indices[particle_sys_rand(owner.data._.indices.length)];
			let face_index: i32 = particle_sys_rand(math_floor(ia.length / 3));

			let i0: i32 = ia[face_index * 3 + 0];
			let i1: i32 = ia[face_index * 3 + 1];
			let i2: i32 = ia[face_index * 3 + 2];

			let v0: vec4_t = vec4_create(positions[i0 * 4], positions[i0 * 4 + 1], positions[i0 * 4 + 2]);
			let v1: vec4_t = vec4_create(positions[i1 * 4], positions[i1 * 4 + 1], positions[i1 * 4 + 2]);
			let v2: vec4_t = vec4_create(positions[i2 * 4], positions[i2 * 4 + 1], positions[i2 * 4 + 2]);

			let pos: vec4_t = particle_sys_random_point_in_triangle(v0, v1, v2);

			instanced_data[i] = pos.x * norm_fac * scale_fac.x;
			i++;
			instanced_data[i] = pos.y * norm_fac * scale_fac.y;
			i++;
			instanced_data[i] = pos.z * norm_fac * scale_fac.z;
			i++;
		}
	}
	else if (raw.r.emit_from == 2) { // Volume
		let scale_factor_volume: vec4_t = vec4_clone(object.base.transform.dim);
		scale_factor_volume = vec4_mult(scale_factor_volume, 0.5 / (particle_size * scale_pos_particle));

		for (let x: i32 = 0; x < raw.particles.length; ++x) {
			instanced_data[i] = (math_random() * 2.0 - 1.0) * scale_factor_volume.x;
			i++;
			instanced_data[i] = (math_random() * 2.0 - 1.0) * scale_factor_volume.y;
			i++;
			instanced_data[i] = (math_random() * 2.0 - 1.0) * scale_factor_volume.z;
			i++;
		}
	}

	mesh_data_setup_inst(object.data, instanced_data, 1);
}

function particle_sys_fhash(n: i32): f32 {
	let s: f32 = n + 1.0;
	s *= math_fmod(9301.0, s);
	s = math_fmod(s * 9301.0 + 49297.0, 233280.0);
	return s / 233280.0;
}

function particle_sys_remove(raw: particle_sys_t) {}

function particle_sys_random_point_in_triangle(a: vec4_t, b: vec4_t, c: vec4_t): vec4_t {
	// Generate a random point in a square where (0, 0) <= (x, y) < (1, 1)
	let x: f32 = math_random();
	let y: f32 = math_random();

	if (x + y > 1) {
		// We are in the upper right triangle in the square, mirror to lower left
		x = 1 - x;
		y = 1 - y;
	}

	// Transform the point to the triangle abc
	let u: vec4_t = vec4_sub(b, a);
	let v: vec4_t = vec4_sub(c, a);
	return vec4_add(a, vec4_add(vec4_mult(u, x), vec4_mult(v, y)));
}

///end
