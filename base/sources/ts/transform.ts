
type transform_t = {
	world?: mat4_t;
	local?: mat4_t;
	loc?: vec4_t;
	rot?: quat_t;
	scale?: vec4_t;
	scale_world?: f32;
	world_unpack?: mat4_t;
	dirty?: bool;
	object?: object_t;
	dim?: vec4_t;
	radius?: f32;
};

function transform_create(object: object_t): transform_t {
	let raw: transform_t = {};
	raw.scale_world      = 1.0;
	raw.object           = object;
	transform_reset(raw);
	return raw;
}

function transform_reset(raw: transform_t) {
	raw.world        = mat4_identity();
	raw.world_unpack = mat4_identity();
	raw.local        = mat4_identity();
	raw.loc          = vec4_create();
	raw.rot          = quat_create();
	raw.scale        = vec4_create(1.0, 1.0, 1.0);
	raw.dim          = vec4_create(2.0, 2.0, 2.0);
	raw.radius       = 1.0;
	raw.dirty        = true;
}

function transform_update(raw: transform_t) {
	if (raw.dirty) {
		transform_build_matrix(raw);
	}
}

function transform_build_matrix(raw: transform_t) {
	raw.local = mat4_compose(raw.loc, raw.rot, raw.scale);

	if (raw.object.parent != null) {
		raw.world = mat4_mult_mat3x4(raw.local, raw.object.parent.transform.world);
	}
	else {
		raw.world = mat4_clone(raw.local);
	}

	raw.world_unpack = mat4_clone(raw.world);
	if (raw.scale_world != 1.0) {
		raw.world_unpack.m00 *= raw.scale_world;
		raw.world_unpack.m01 *= raw.scale_world;
		raw.world_unpack.m02 *= raw.scale_world;
		raw.world_unpack.m03 *= raw.scale_world;
		raw.world_unpack.m10 *= raw.scale_world;
		raw.world_unpack.m11 *= raw.scale_world;
		raw.world_unpack.m12 *= raw.scale_world;
		raw.world_unpack.m13 *= raw.scale_world;
		raw.world_unpack.m20 *= raw.scale_world;
		raw.world_unpack.m21 *= raw.scale_world;
		raw.world_unpack.m22 *= raw.scale_world;
		raw.world_unpack.m23 *= raw.scale_world;
	}

	transform_compute_dim(raw);

	// Update children
	for (let i: i32 = 0; i < raw.object.children.length; ++i) {
		let n: object_t = raw.object.children[i];
		transform_build_matrix(n.transform);
	}

	raw.dirty = false;
}

function transform_set_matrix(raw: transform_t, mat: mat4_t) {
	raw.local = mat4_clone(mat);
	transform_decompose(raw);
	transform_build_matrix(raw);
}

function transform_decompose(raw: transform_t) {
	let dec: mat4_decomposed_t = mat4_decompose(raw.local);
	raw.loc                    = dec.loc;
	raw.rot                    = dec.rot;
	raw.scale                  = dec.scl;
}

function transform_rotate(raw: transform_t, axis: vec4_t, f: f32) {
	let q: quat_t = quat_from_axis_angle(axis, f);
	raw.rot       = quat_mult(q, raw.rot);
	transform_build_matrix(raw);
}

function transform_move(raw: transform_t, axis: vec4_t, f: f32 = 1.0) {
	raw.loc = vec4_fadd(raw.loc, axis.x * f, axis.y * f, axis.z * f);
	transform_build_matrix(raw);
}

function transform_compute_radius(raw: transform_t) {
	raw.radius = math_sqrt(raw.dim.x * raw.dim.x + raw.dim.y * raw.dim.y + raw.dim.z * raw.dim.z);
}

function transform_compute_dim(raw: transform_t) {
	if (raw.object.raw == null && raw.object.ext_type == "mesh_object_t") {
		let mo: mesh_object_t = raw.object.ext;
		let aabb: vec4_t      = mesh_data_calculate_aabb(mo.data);
		let o: object_t       = raw.object;
		o.raw                 = {};
		o.raw.dimensions      = f32_array_create_xyz(aabb.x, aabb.y, aabb.z);
	}

	if (raw.object.raw == null || raw.object.raw.dimensions == null) {
		raw.dim = vec4_create(2 * raw.scale.x, 2 * raw.scale.y, 2 * raw.scale.z);
	}
	else {
		let d: f32_array_t = raw.object.raw.dimensions;
		raw.dim            = vec4_create(d[0] * raw.scale.x, d[1] * raw.scale.y, d[2] * raw.scale.z);
	}
	transform_compute_radius(raw);
}

function transform_look(raw: transform_t): vec4_t {
	return mat4_look(raw.world);
}

function transform_right(raw: transform_t): vec4_t {
	return mat4_right(raw.world);
}

function transform_up(raw: transform_t): vec4_t {
	return mat4_up(raw.world);
}

function transform_world_x(raw: transform_t): f32 {
	return raw.world.m30;
}

function transform_world_y(raw: transform_t): f32 {
	return raw.world.m31;
}

function transform_world_z(raw: transform_t): f32 {
	return raw.world.m32;
}
