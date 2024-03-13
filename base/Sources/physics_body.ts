
///if arm_physics

class PhysicsBodyRaw {
	_mass: f32 = 0.0;

	get mass(): f32 {
		return this._mass;
	}

	set mass(f: f32) {
		if (this.ready) {
			// remove();
			let t: PhysicsBodyRaw = new PhysicsBodyRaw();
			t._mass = f;
			physics_body_init(t, this.object);
			(this.object as any).physicsBody = t;
		}
		else this._mass = f;
	}

	object: object_t;
	friction: f32 = 0.5;
	restitution: f32 = 0.0;
	collision_margin: f32 = 0.0;
	linear_damping: f32 = 0.04;
	angular_damping: f32 = 0.1;
	linear_factors: f32[] = [1.0, 1.0, 1.0];
	angular_factors: f32[] = [1.0, 1.0, 1.0];
	linear_threshold: f32 = 0.0;
	angular_threshold: f32 = 0.0;
	ccd: bool = false; // Continuous collision detection
	trigger: bool = false;
	group: i32 = 1;
	mask: i32 = 1;
	shape: shape_type_t = shape_type_t.BOX;
	destroyed: bool = false;
	body_scale_x: f32; // Transform scale at creation time
	body_scale_y: f32;
	body_scale_z: f32;
	current_scale_x: f32;
	current_scale_y: f32;
	current_scale_z: f32;

	body: Ammo.btRigidBody = null;
	motion_state: Ammo.btMotionState;
	btshape: Ammo.btCollisionShape;
	ready: bool = false;
	id: i32 = 0;
	height_data: Uint8Array = null;
}

let physics_body_next_id: i32 = 0;
let physics_body_ammo_array: i32 = -1;
let physics_body_gimpact_registered: bool = false;
let physics_body_first: bool = true;
let physics_body_vec1: Ammo.btVector3;
let physics_body_vec2: Ammo.btVector3;
let physics_body_vec3: Ammo.btVector3;
let physics_body_quat1: Ammo.btQuaternion;
let physics_body_trans1: Ammo.btTransform;
let physics_body_trans2: Ammo.btTransform;
let physics_body_quat: quat_t = quat_create();

let physics_body_convex_hull_cache: map_t<mesh_data_t, Ammo.btConvexHullShape> = map_create();
let physics_body_triangle_mesh_cache: map_t<mesh_data_t, Ammo.btTriangleMesh> = map_create();
let physics_body_users_cache: map_t<mesh_data_t, i32> = map_create();

function physics_body_create(): PhysicsBodyRaw {
	if (physics_body_first) {
		physics_body_first = false;
		physics_body_vec1 = new Ammo.btVector3(0, 0, 0);
		physics_body_vec2 = new Ammo.btVector3(0, 0, 0);
		physics_body_vec3 = new Ammo.btVector3(0, 0, 0);
		physics_body_quat1 = new Ammo.btQuaternion(0, 0, 0, 0);
		physics_body_trans1 = new Ammo.btTransform();
		physics_body_trans2 = new Ammo.btTransform();
	}
	let pb: PhysicsBodyRaw = new PhysicsBodyRaw();
	return pb;
}

function physics_body_with_margin(pb: PhysicsBodyRaw, f: f32) {
	return f - f * pb.collision_margin;
}

function physics_body_init(pb: PhysicsBodyRaw, o: object_t) {
	pb.object = o;
	if (pb.ready) {
		return;
	}
	pb.ready = true;

	if (pb.object.ext_type != "mesh_object_t") {
		return; // No mesh data
	}
	let transform: transform_t = o.transform;
	let physics: PhysicsWorldRaw = physics_world_active;

	if (pb.shape == shape_type_t.BOX) {
		physics_world_vec1.setX(physics_body_with_margin(pb, transform.dim.x / 2));
		physics_world_vec1.setY(physics_body_with_margin(pb, transform.dim.y / 2));
		physics_world_vec1.setZ(physics_body_with_margin(pb, transform.dim.z / 2));
		pb.btshape = new Ammo.btBoxShape(physics_world_vec1);
	}
	else if (pb.shape == shape_type_t.SPHERE) {
		pb.btshape = new Ammo.btSphereShape(physics_body_with_margin(pb, transform.dim.x / 2));
	}
	else if (pb.shape == shape_type_t.CONVEX_HULL) {
		let shape_convex: Ammo.btConvexHullShape = physics_body_fill_convex_hull(pb, transform.scale, pb.collision_margin);
		pb.btshape = shape_convex;
	}
	else if (pb.shape == shape_type_t.CONE) {
		let cone_z: Ammo.btConeShapeZ = new Ammo.btConeShapeZ(
			physics_body_with_margin(pb, transform.dim.x / 2), // Radius
			physics_body_with_margin(pb, transform.dim.z));	 // Height
		let cone: Ammo.btConeShape = cone_z;
		pb.btshape = cone;
	}
	else if (pb.shape == shape_type_t.CYLINDER) {
		physics_world_vec1.setX(physics_body_with_margin(pb, transform.dim.x / 2));
		physics_world_vec1.setY(physics_body_with_margin(pb, transform.dim.y / 2));
		physics_world_vec1.setZ(physics_body_with_margin(pb, transform.dim.z / 2));
		let cyl_z: Ammo.btCylinderShapeZ = new Ammo.btCylinderShapeZ(physics_world_vec1);
		let cyl: Ammo.btCylinderShape = cyl_z;
		pb.btshape = cyl;
	}
	else if (pb.shape == shape_type_t.CAPSULE) {
		let r: f32 = transform.dim.x / 2;
		let caps_z: Ammo.btCapsuleShapeZ = new Ammo.btCapsuleShapeZ(
			physics_body_with_margin(pb, r), // Radius
			physics_body_with_margin(pb, transform.dim.z - r * 2)); // Distance between 2 sphere centers
		let caps: Ammo.btCapsuleShape = caps_z;
		pb.btshape = caps;
	}
	else if (pb.shape == shape_type_t.MESH) {
		let mesh_interface: Ammo.btTriangleMesh = physics_body_fill_triangle_mesh(pb, transform.scale);
		if (pb.mass > 0) {
			let shape_gimpact: Ammo.btGImpactMeshShape = new Ammo.btGImpactMeshShape(mesh_interface);
			shape_gimpact.updateBound();
			let shape_concave: Ammo.btConcaveShape = shape_gimpact;
			pb.btshape = shape_concave;
			if (!physics_body_gimpact_registered) {
				physics_body_gimpact_registered = true;
				new Ammo.GImpactCollisionAlgorithm().registerAlgorithm(physics.dispatcher);
			}
		}
		else {
			let shape_bvh: Ammo.btBvhTriangleMeshShape = new Ammo.btBvhTriangleMeshShape(mesh_interface, true, true);
			let shape_tri: Ammo.btTriangleMeshShape = shape_bvh;
			let shape_concave: Ammo.btConcaveShape = shape_tri;
			pb.btshape = shape_concave;
		}
	}
	else if (pb.shape == shape_type_t.TERRAIN) {
		let length: i32 = pb.height_data.length;
		if (physics_body_ammo_array == -1) {
			physics_body_ammo_array = Ammo._malloc(length);
		}
		// From texture bytes
		for (let i: i32 = 0; i < length; ++i) {
			Ammo.HEAPU8[physics_body_ammo_array + i] = pb.height_data[i];
		}
		let slice: i32 = math_floor(math_sqrt(length)); // Assuming square terrain data
		let axis: i32 = 2; // z
		let data_type: i32 = 5; // u8
		pb.btshape = new Ammo.btHeightfieldTerrainShape(slice, slice, physics_body_ammo_array, 1 / 255, 0, 1, axis, data_type, false);
		physics_body_vec1.setX(transform.dim.x / slice);
		physics_body_vec1.setY(transform.dim.y / slice);
		physics_body_vec1.setZ(transform.dim.z);
		pb.btshape.setLocalScaling(physics_body_vec1);
	}

	physics_body_trans1.setIdentity();
	physics_body_vec1.setX(transform_world_x(transform));
	physics_body_vec1.setY(transform_world_y(transform));
	physics_body_vec1.setZ(transform_world_z(transform));
	physics_body_trans1.setOrigin(physics_body_vec1);
	quat_from_mat(physics_body_quat, transform.world);
	physics_body_quat1.setValue(physics_body_quat.x, physics_body_quat.y, physics_body_quat.z, physics_body_quat.w);
	physics_body_trans1.setRotation(physics_body_quat1);
	physics_body_trans2.setIdentity();
	pb.motion_state = new Ammo.btDefaultMotionState(physics_body_trans1, physics_body_trans2); // Transform, center of mass offset

	physics_body_vec1.setX(0);
	physics_body_vec1.setY(0);
	physics_body_vec1.setZ(0);
	let inertia: Ammo.btVector3 = physics_body_vec1;

	if (pb.mass > 0) {
		pb.btshape.calculateLocalInertia(pb.mass, inertia);
	}
	let body_ci: Ammo.btRigidBodyConstructionInfo = new Ammo.btRigidBodyConstructionInfo(pb.mass, pb.motion_state, pb.btshape, inertia);
	pb.body = new Ammo.btRigidBody(body_ci);

	pb.body.setFriction(pb.friction);
	if (pb.shape == shape_type_t.SPHERE || pb.shape == shape_type_t.CYLINDER || pb.shape == shape_type_t.CONE || pb.shape == shape_type_t.CAPSULE) {
		pb.angular_damping += pb.friction;
	}
	pb.body.setRestitution(pb.restitution);
	// pb.body.setSleepingThresholds(linearThreshold, angularThreshold);
	// pb.body.setDeactivationTime(deactivationTime);
	pb.body.setDamping(pb.linear_damping, pb.angular_damping);
	physics_body_set_linear_factor(pb, pb.linear_factors[0], pb.linear_factors[1], pb.linear_factors[2]);
	physics_body_set_angular_factor(pb, pb.angular_factors[0], pb.angular_factors[1], pb.angular_factors[2]);
	if (pb.trigger) {
		pb.body.setCollisionFlags(pb.body.getCollisionFlags() | collision_flags_t.CF_NO_CONTACT_RESPONSE);
	}
	if (pb.mass == 0.0) {
		pb.body.setCollisionFlags(pb.body.getCollisionFlags() | collision_flags_t.CF_STATIC_OBJECT);
	}
	if (pb.ccd) {
		physics_body_set_ccd(pb, transform.radius);
	}

	pb.body_scale_x = pb.current_scale_x = transform.scale.x;
	pb.body_scale_y = pb.current_scale_y = transform.scale.y;
	pb.body_scale_z = pb.current_scale_z = transform.scale.z;

	pb.id = physics_body_next_id++;
	pb.body.userIndex = pb.id;

	physics_world_add_body(physics, pb);

	// notifyOnRemove(removeFromWorld);

	Ammo.destroy(body_ci);
}

function physics_body_physics_update(pb: PhysicsBodyRaw) {
	if (!pb.ready) {
		return;
	}
	let trans: Ammo.btTransform = pb.body.getWorldTransform();

	let p: Ammo.btVector3 = trans.getOrigin();
	let q: Ammo.btQuaternion = trans.getRotation();
	let qw: Ammo.btQuadWord = q;

	let transform: transform_t = pb.object.transform;
	vec4_set(transform.loc, p.x(), p.y(), p.z());
	quat_set(transform.rot, qw.x(), qw.y(), qw.z(), qw.w());
	if (pb.object.parent != null) {
		let ptransform: transform_t = pb.object.parent.transform;
		transform.loc.x -= transform_world_x(ptransform);
		transform.loc.y -= transform_world_y(ptransform);
		transform.loc.z -= transform_world_z(ptransform);
	}
	transform_build_matrix(transform);
}

function physics_body_remove_from_world(pb: PhysicsBodyRaw) {
	physics_world_remove_body(physics_world_active, pb);
}

function physics_body_activate(pb: PhysicsBodyRaw) {
	pb.body.activate(false);
}

function physics_body_set_gravity(pb: PhysicsBodyRaw, v: vec4_t) {
	physics_body_vec1.setValue(v.x, v.y, v.z);
	pb.body.setGravity(physics_body_vec1);
}

function physics_body_apply_force(pb: PhysicsBodyRaw, force: vec4_t, loc: vec4_t = null) {
	physics_body_activate(pb);
	physics_body_vec1.setValue(force.x, force.y, force.z);
	if (loc == null) {
		pb.body.applyCentralForce(physics_body_vec1);
	}
	else {
		physics_body_vec2.setValue(loc.x, loc.y, loc.z);
		pb.body.applyForce(physics_body_vec1, physics_body_vec2);
	}
}

function physics_body_apply_impulse(pb: PhysicsBodyRaw, impulse: vec4_t, loc: vec4_t = null) {
	physics_body_activate(pb);
	physics_body_vec1.setValue(impulse.x, impulse.y, impulse.z);
	if (loc == null) {
		pb.body.applyCentralImpulse(physics_body_vec1);
	}
	else {
		physics_body_vec2.setValue(loc.x, loc.y, loc.z);
		pb.body.applyImpulse(physics_body_vec1, physics_body_vec2);
	}
}

function physics_body_apply_torque(pb: PhysicsBodyRaw, torque: vec4_t) {
	physics_body_activate(pb);
	physics_body_vec1.setValue(torque.x, torque.y, torque.z);
	pb.body.applyTorque(physics_body_vec1);
}

function physics_body_apply_torque_impulse(pb: PhysicsBodyRaw, torque: vec4_t) {
	physics_body_activate(pb);
	physics_body_vec1.setValue(torque.x, torque.y, torque.z);
	pb.body.applyTorqueImpulse(physics_body_vec1);
}

function physics_body_set_linear_factor(pb: PhysicsBodyRaw, x: f32, y: f32, z: f32) {
	physics_body_vec1.setValue(x, y, z);
	pb.body.setLinearFactor(physics_body_vec1);
}

function physics_body_set_angular_factor(pb: PhysicsBodyRaw, x: f32, y: f32, z: f32) {
	physics_body_vec1.setValue(x, y, z);
	pb.body.setAngularFactor(physics_body_vec1);
}

function physics_body_get_linear_velocity(pb: PhysicsBodyRaw): vec4_t {
	let v: Ammo.btVector3 = pb.body.getLinearVelocity();
	return vec4_create(v.x(), v.y(), v.z());
}

function physics_body_set_linear_velocity(pb: PhysicsBodyRaw, x: f32, y: f32, z: f32) {
	physics_body_vec1.setValue(x, y, z);
	pb.body.setLinearVelocity(physics_body_vec1);
}

function physics_body_get_angular_velocity(pb: PhysicsBodyRaw): vec4_t {
	let v: Ammo.btVector3 = pb.body.getAngularVelocity();
	return vec4_create(v.x(), v.y(), v.z());
}

function physics_body_set_angular_velocity(pb: PhysicsBodyRaw, x: f32, y: f32, z: f32) {
	physics_body_vec1.setValue(x, y, z);
	pb.body.setAngularVelocity(physics_body_vec1);
}

function physics_body_set_friction(pb: PhysicsBodyRaw, f: f32) {
	pb.body.setFriction(f);
	pb.friction = f;
}

function physics_body_set_scale(pb: PhysicsBodyRaw, v: vec4_t) {
	pb.current_scale_x = v.x;
	pb.current_scale_y = v.y;
	pb.current_scale_z = v.z;
	physics_body_vec1.setX(v.x / pb.body_scale_x);
	physics_body_vec1.setY(v.y / pb.body_scale_y);
	physics_body_vec1.setZ(v.z / pb.body_scale_z);
	pb.btshape.setLocalScaling(physics_body_vec1);
	let world_dyn: Ammo.btDynamicsWorld = physics_world_active.world;
	let world_col: Ammo.btCollisionWorld = world_dyn;
	world_col.updateSingleAabb(pb.body);
}

function physics_body_sync_transform(pb: PhysicsBodyRaw) {
	let t: transform_t = pb.object.transform;
	transform_build_matrix(t);
	physics_body_vec1.setValue(transform_world_x(t), transform_world_y(t), transform_world_z(t));
	physics_body_trans1.setOrigin(physics_body_vec1);
	quat_from_mat(physics_body_quat, t.world);
	physics_body_quat1.setValue(physics_body_quat.x, physics_body_quat.y, physics_body_quat.z, physics_body_quat.w);
	physics_body_trans1.setRotation(physics_body_quat1);
	pb.body.setWorldTransform(physics_body_trans1);
	if (pb.current_scale_x != t.scale.x || pb.current_scale_y != t.scale.y || pb.current_scale_z != t.scale.z) {
		physics_body_set_scale(pb, t.scale);
	}
	physics_body_activate(pb);
}

function physics_body_set_ccd(pb: PhysicsBodyRaw, sphereRadius: f32, motionThreshold = 1e-7) {
	pb.body.setCcdSweptSphereRadius(sphereRadius);
	pb.body.setCcdMotionThreshold(motionThreshold);
}

function physics_body_fill_convex_hull(pb: PhysicsBodyRaw, scale: vec4_t, margin: f32): Ammo.btConvexHullShape {
	// Check whether shape already exists
	let data: any = pb.object.ext.data;
	let shape: Ammo.btConvexHullShape = physics_body_convex_hull_cache.get(data);
	if (shape != null) {
		physics_body_users_cache.set(data, physics_body_users_cache.get(data) + 1);
		return shape;
	}

	shape = new Ammo.btConvexHullShape();
	physics_body_convex_hull_cache.set(data, shape);
	physics_body_users_cache.set(data, 1);

	let positions: i16_array_t = mesh_data_get_vertex_array(data, 'pos').values;

	let sx: f32 = scale.x * (1.0 - margin) * (1 / 32767);
	let sy: f32 = scale.y * (1.0 - margin) * (1 / 32767);
	let sz: f32 = scale.z * (1.0 - margin) * (1 / 32767);

	sx *= data.scale_pos;
	sy *= data.scale_pos;
	sz *= data.scale_pos;

	for (let i: i32 = 0; i < math_floor(positions.length / 4); ++i) {
		physics_body_vec1.setX(positions[i * 4    ] * sx);
		physics_body_vec1.setY(positions[i * 4 + 1] * sy);
		physics_body_vec1.setZ(positions[i * 4 + 2] * sz);
		shape.addPoint(physics_body_vec1, true);
	}
	return shape;
}

function physics_body_fill_triangle_mesh(pb: PhysicsBodyRaw, scale: vec4_t): Ammo.btTriangleMesh {
	// Check whether shape already exists
	let data: any = pb.object.ext.data;
	let triangle_mesh: Ammo.btTriangleMesh = physics_body_triangle_mesh_cache.get(data);
	if (triangle_mesh != null) {
		physics_body_users_cache.set(data, physics_body_users_cache.get(data) + 1);
		return triangle_mesh;
	}

	triangle_mesh = new Ammo.btTriangleMesh(true, true);
	physics_body_triangle_mesh_cache.set(data, triangle_mesh);
	physics_body_users_cache.set(data, 1);

	let positions: i16_array_t = mesh_data_get_vertex_array(data, 'pos').values;
	let indices: any = data._indices;

	let sx: f32 = scale.x * (1 / 32767);
	let sy: f32 = scale.y * (1 / 32767);
	let sz: f32 = scale.z * (1 / 32767);

	sx *= data.scale_pos;
	sy *= data.scale_pos;
	sz *= data.scale_pos;

	for (let ar of indices) {
		for (let i: i32 = 0; i < math_floor(ar.length / 3); ++i) {
			physics_body_vec1.setX(positions[ar[i * 3    ] * 4    ] * sx);
			physics_body_vec1.setY(positions[ar[i * 3    ] * 4 + 1] * sy);
			physics_body_vec1.setZ(positions[ar[i * 3    ] * 4 + 2] * sz);
			physics_body_vec2.setX(positions[ar[i * 3 + 1] * 4    ] * sx);
			physics_body_vec2.setY(positions[ar[i * 3 + 1] * 4 + 1] * sy);
			physics_body_vec2.setZ(positions[ar[i * 3 + 1] * 4 + 2] * sz);
			physics_body_vec3.setX(positions[ar[i * 3 + 2] * 4    ] * sx);
			physics_body_vec3.setY(positions[ar[i * 3 + 2] * 4 + 1] * sy);
			physics_body_vec3.setZ(positions[ar[i * 3 + 2] * 4 + 2] * sz);
			triangle_mesh.addTriangle(physics_body_vec1, physics_body_vec2, physics_body_vec3);
		}
	}
	return triangle_mesh;
}

function physics_body_delete(pb: PhysicsBodyRaw) {
	Ammo.destroy(pb.motion_state);
	Ammo.destroy(pb.body);

	// Delete shape if no other user is found
	if (pb.shape == shape_type_t.CONVEX_HULL || pb.shape == shape_type_t.MESH) {
		let data: any = pb.object.ext.data;
		let i: i32 = physics_body_users_cache.get(data) - 1;
		physics_body_users_cache.set(data, i);
		if (i <= 0) {
			Ammo.destroy(pb.btshape);
			pb.shape == shape_type_t.CONVEX_HULL ?
				physics_body_convex_hull_cache.delete(data) :
				physics_body_triangle_mesh_cache.delete(data);
		}
	}
	else Ammo.destroy(pb.btshape);
}

enum shape_type_t {
	BOX,
	SPHERE,
	CONVEX_HULL,
	MESH,
	CONE,
	CYLINDER,
	CAPSULE,
	TERRAIN,
}

enum collision_flags_t {
	CF_STATIC_OBJECT = 1,
	CF_KINEMATIC_OBJECT = 2,
	CF_NO_CONTACT_RESPONSE = 4,
	CF_CHARACTER_OBJECT = 16,
}

///end
