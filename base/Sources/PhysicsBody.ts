
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
			PhysicsBody.init(t, this.object);
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

class PhysicsBody {

	static next_id: i32 = 0;
	static ammo_array: i32 = -1;
	static gimpact_registered: bool = false;
	static first: bool = true;
	static vec1: Ammo.btVector3;
	static vec2: Ammo.btVector3;
	static vec3: Ammo.btVector3;
	static quat1: Ammo.btQuaternion;
	static trans1: Ammo.btTransform;
	static trans2: Ammo.btTransform;
	static quat: quat_t = quat_create();

	static convex_hull_cache: Map<mesh_data_t, Ammo.btConvexHullShape> = new Map();
	static triangle_mesh_cache: Map<mesh_data_t, Ammo.btTriangleMesh> = new Map();
	static users_cache: Map<mesh_data_t, i32> = new Map();

	static create(): PhysicsBodyRaw {
		if (PhysicsBody.first) {
			PhysicsBody.first = false;
			PhysicsBody.vec1 = new Ammo.btVector3(0, 0, 0);
			PhysicsBody.vec2 = new Ammo.btVector3(0, 0, 0);
			PhysicsBody.vec3 = new Ammo.btVector3(0, 0, 0);
			PhysicsBody.quat1 = new Ammo.btQuaternion(0, 0, 0, 0);
			PhysicsBody.trans1 = new Ammo.btTransform();
			PhysicsBody.trans2 = new Ammo.btTransform();
		}
		let pb: PhysicsBodyRaw = new PhysicsBodyRaw();
		return pb;
	}

	static with_margin = (pb: PhysicsBodyRaw, f: f32) => {
		return f - f * pb.collision_margin;
	}

	static init = (pb: PhysicsBodyRaw, o: object_t) => {
		pb.object = o;
		if (pb.ready) {
			return;
		}
		pb.ready = true;

		if (pb.object.ext_type != "mesh_object_t") {
			return; // No mesh data
		}
		let transform: transform_t = o.transform;
		let physics: PhysicsWorldRaw = PhysicsWorld.active;

		if (pb.shape == shape_type_t.BOX) {
			PhysicsWorld.vec1.setX(PhysicsBody.with_margin(pb, transform.dim.x / 2));
			PhysicsWorld.vec1.setY(PhysicsBody.with_margin(pb, transform.dim.y / 2));
			PhysicsWorld.vec1.setZ(PhysicsBody.with_margin(pb, transform.dim.z / 2));
			pb.btshape = new Ammo.btBoxShape(PhysicsWorld.vec1);
		}
		else if (pb.shape == shape_type_t.SPHERE) {
			pb.btshape = new Ammo.btSphereShape(PhysicsBody.with_margin(pb, transform.dim.x / 2));
		}
		else if (pb.shape == shape_type_t.CONVEX_HULL) {
			let shapeConvex: Ammo.btConvexHullShape = PhysicsBody.fill_convex_hull(pb, transform.scale, pb.collision_margin);
			pb.btshape = shapeConvex;
		}
		else if (pb.shape == shape_type_t.CONE) {
			let coneZ: Ammo.btConeShapeZ = new Ammo.btConeShapeZ(
				PhysicsBody.with_margin(pb, transform.dim.x / 2), // Radius
				PhysicsBody.with_margin(pb, transform.dim.z));	 // Height
			let cone: Ammo.btConeShape = coneZ;
			pb.btshape = cone;
		}
		else if (pb.shape == shape_type_t.CYLINDER) {
			PhysicsWorld.vec1.setX(PhysicsBody.with_margin(pb, transform.dim.x / 2));
			PhysicsWorld.vec1.setY(PhysicsBody.with_margin(pb, transform.dim.y / 2));
			PhysicsWorld.vec1.setZ(PhysicsBody.with_margin(pb, transform.dim.z / 2));
			let cylZ: Ammo.btCylinderShapeZ = new Ammo.btCylinderShapeZ(PhysicsWorld.vec1);
			let cyl: Ammo.btCylinderShape = cylZ;
			pb.btshape = cyl;
		}
		else if (pb.shape == shape_type_t.CAPSULE) {
			let r: f32 = transform.dim.x / 2;
			let capsZ: Ammo.btCapsuleShapeZ = new Ammo.btCapsuleShapeZ(
				PhysicsBody.with_margin(pb, r), // Radius
				PhysicsBody.with_margin(pb, transform.dim.z - r * 2)); // Distance between 2 sphere centers
			let caps: Ammo.btCapsuleShape = capsZ;
			pb.btshape = caps;
		}
		else if (pb.shape == shape_type_t.MESH) {
			let meshInterface: Ammo.btTriangleMesh = PhysicsBody.fill_triangle_mesh(pb, transform.scale);
			if (pb.mass > 0) {
				let shapeGImpact: Ammo.btGImpactMeshShape = new Ammo.btGImpactMeshShape(meshInterface);
				shapeGImpact.updateBound();
				let shapeConcave: Ammo.btConcaveShape = shapeGImpact;
				pb.btshape = shapeConcave;
				if (!PhysicsBody.gimpact_registered) {
					PhysicsBody.gimpact_registered = true;
					new Ammo.GImpactCollisionAlgorithm().registerAlgorithm(physics.dispatcher);
				}
			}
			else {
				let shapeBvh: Ammo.btBvhTriangleMeshShape = new Ammo.btBvhTriangleMeshShape(meshInterface, true, true);
				let shapeTri: Ammo.btTriangleMeshShape = shapeBvh;
				let shapeConcave: Ammo.btConcaveShape = shapeTri;
				pb.btshape = shapeConcave;
			}
		}
		else if (pb.shape == shape_type_t.TERRAIN) {
			let length: i32 = pb.height_data.length;
			if (PhysicsBody.ammo_array == -1) {
				PhysicsBody.ammo_array = Ammo._malloc(length);
			}
			// From texture bytes
			for (let i: i32 = 0; i < length; ++i) {
				Ammo.HEAPU8[PhysicsBody.ammo_array + i] = pb.height_data[i];
			}
			let slice: i32 = Math.floor(Math.sqrt(length)); // Assuming square terrain data
			let axis: i32 = 2; // z
			let dataType: i32 = 5; // u8
			pb.btshape = new Ammo.btHeightfieldTerrainShape(slice, slice, PhysicsBody.ammo_array, 1 / 255, 0, 1, axis, dataType, false);
			PhysicsBody.vec1.setX(transform.dim.x / slice);
			PhysicsBody.vec1.setY(transform.dim.y / slice);
			PhysicsBody.vec1.setZ(transform.dim.z);
			pb.btshape.setLocalScaling(PhysicsBody.vec1);
		}

		PhysicsBody.trans1.setIdentity();
		PhysicsBody.vec1.setX(transform_world_x(transform));
		PhysicsBody.vec1.setY(transform_world_y(transform));
		PhysicsBody.vec1.setZ(transform_world_z(transform));
		PhysicsBody.trans1.setOrigin(PhysicsBody.vec1);
		quat_from_mat(PhysicsBody.quat, transform.world);
		PhysicsBody.quat1.setValue(PhysicsBody.quat.x, PhysicsBody.quat.y, PhysicsBody.quat.z, PhysicsBody.quat.w);
		PhysicsBody.trans1.setRotation(PhysicsBody.quat1);
		PhysicsBody.trans2.setIdentity();
		pb.motion_state = new Ammo.btDefaultMotionState(PhysicsBody.trans1, PhysicsBody.trans2); // Transform, center of mass offset

		PhysicsBody.vec1.setX(0);
		PhysicsBody.vec1.setY(0);
		PhysicsBody.vec1.setZ(0);
		let inertia: Ammo.btVector3 = PhysicsBody.vec1;

		if (pb.mass > 0) {
			pb.btshape.calculateLocalInertia(pb.mass, inertia);
		}
		let bodyCI: Ammo.btRigidBodyConstructionInfo = new Ammo.btRigidBodyConstructionInfo(pb.mass, pb.motion_state, pb.btshape, inertia);
		pb.body = new Ammo.btRigidBody(bodyCI);

		pb.body.setFriction(pb.friction);
		if (pb.shape == shape_type_t.SPHERE || pb.shape == shape_type_t.CYLINDER || pb.shape == shape_type_t.CONE || pb.shape == shape_type_t.CAPSULE) {
			pb.angular_damping += pb.friction;
		}
		pb.body.setRestitution(pb.restitution);
		// pb.body.setSleepingThresholds(linearThreshold, angularThreshold);
		// pb.body.setDeactivationTime(deactivationTime);
		pb.body.setDamping(pb.linear_damping, pb.angular_damping);
		PhysicsBody.set_linear_factor(pb, pb.linear_factors[0], pb.linear_factors[1], pb.linear_factors[2]);
		PhysicsBody.set_angular_factor(pb, pb.angular_factors[0], pb.angular_factors[1], pb.angular_factors[2]);
		if (pb.trigger) {
			pb.body.setCollisionFlags(pb.body.getCollisionFlags() | collision_flags_t.CF_NO_CONTACT_RESPONSE);
		}
		if (pb.mass == 0.0) {
			pb.body.setCollisionFlags(pb.body.getCollisionFlags() | collision_flags_t.CF_STATIC_OBJECT);
		}
		if (pb.ccd) {
			PhysicsBody.set_ccd(pb, transform.radius);
		}

		pb.body_scale_x = pb.current_scale_x = transform.scale.x;
		pb.body_scale_y = pb.current_scale_y = transform.scale.y;
		pb.body_scale_z = pb.current_scale_z = transform.scale.z;

		pb.id = PhysicsBody.next_id++;
		pb.body.userIndex = pb.id;

		PhysicsWorld.add_body(physics, pb);

		// notifyOnRemove(removeFromWorld);

		Ammo.destroy(bodyCI);
	}

	static physics_update = (pb: PhysicsBodyRaw) => {
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

	static remove_from_world = (pb: PhysicsBodyRaw) => {
		PhysicsWorld.remove_body(PhysicsWorld.active, pb);
	}

	static activate = (pb: PhysicsBodyRaw) => {
		pb.body.activate(false);
	}

	static set_gravity = (pb: PhysicsBodyRaw, v: vec4_t) => {
		PhysicsBody.vec1.setValue(v.x, v.y, v.z);
		pb.body.setGravity(PhysicsBody.vec1);
	}

	static apply_force = (pb: PhysicsBodyRaw, force: vec4_t, loc: vec4_t = null) => {
		PhysicsBody.activate(pb);
		PhysicsBody.vec1.setValue(force.x, force.y, force.z);
		if (loc == null) {
			pb.body.applyCentralForce(PhysicsBody.vec1);
		}
		else {
			PhysicsBody.vec2.setValue(loc.x, loc.y, loc.z);
			pb.body.applyForce(PhysicsBody.vec1, PhysicsBody.vec2);
		}
	}

	static apply_impulse = (pb: PhysicsBodyRaw, impulse: vec4_t, loc: vec4_t = null) => {
		PhysicsBody.activate(pb);
		PhysicsBody.vec1.setValue(impulse.x, impulse.y, impulse.z);
		if (loc == null) {
			pb.body.applyCentralImpulse(PhysicsBody.vec1);
		}
		else {
			PhysicsBody.vec2.setValue(loc.x, loc.y, loc.z);
			pb.body.applyImpulse(PhysicsBody.vec1, PhysicsBody.vec2);
		}
	}

	static apply_torque = (pb: PhysicsBodyRaw, torque: vec4_t) => {
		PhysicsBody.activate(pb);
		PhysicsBody.vec1.setValue(torque.x, torque.y, torque.z);
		pb.body.applyTorque(PhysicsBody.vec1);
	}

	static apply_torque_impulse = (pb: PhysicsBodyRaw, torque: vec4_t) => {
		PhysicsBody.activate(pb);
		PhysicsBody.vec1.setValue(torque.x, torque.y, torque.z);
		pb.body.applyTorqueImpulse(PhysicsBody.vec1);
	}

	static set_linear_factor = (pb: PhysicsBodyRaw, x: f32, y: f32, z: f32) => {
		PhysicsBody.vec1.setValue(x, y, z);
		pb.body.setLinearFactor(PhysicsBody.vec1);
	}

	static set_angular_factor = (pb: PhysicsBodyRaw, x: f32, y: f32, z: f32) => {
		PhysicsBody.vec1.setValue(x, y, z);
		pb.body.setAngularFactor(PhysicsBody.vec1);
	}

	static get_linear_velocity = (pb: PhysicsBodyRaw): vec4_t => {
		let v: Ammo.btVector3 = pb.body.getLinearVelocity();
		return vec4_create(v.x(), v.y(), v.z());
	}

	static set_linear_velocity = (pb: PhysicsBodyRaw, x: f32, y: f32, z: f32) => {
		PhysicsBody.vec1.setValue(x, y, z);
		pb.body.setLinearVelocity(PhysicsBody.vec1);
	}

	static get_angular_velocity = (pb: PhysicsBodyRaw): vec4_t => {
		let v: Ammo.btVector3 = pb.body.getAngularVelocity();
		return vec4_create(v.x(), v.y(), v.z());
	}

	static set_angular_velocity = (pb: PhysicsBodyRaw, x: f32, y: f32, z: f32) => {
		PhysicsBody.vec1.setValue(x, y, z);
		pb.body.setAngularVelocity(PhysicsBody.vec1);
	}

	static set_friction = (pb: PhysicsBodyRaw, f: f32) => {
		pb.body.setFriction(f);
		pb.friction = f;
	}

	static set_scale = (pb: PhysicsBodyRaw, v: vec4_t) => {
		pb.current_scale_x = v.x;
		pb.current_scale_y = v.y;
		pb.current_scale_z = v.z;
		PhysicsBody.vec1.setX(v.x / pb.body_scale_x);
		PhysicsBody.vec1.setY(v.y / pb.body_scale_y);
		PhysicsBody.vec1.setZ(v.z / pb.body_scale_z);
		pb.btshape.setLocalScaling(PhysicsBody.vec1);
		let worldDyn: Ammo.btDynamicsWorld = PhysicsWorld.active.world;
		let worldCol: Ammo.btCollisionWorld = worldDyn;
		worldCol.updateSingleAabb(pb.body);
	}

	static sync_transform = (pb: PhysicsBodyRaw) => {
		let t: transform_t = pb.object.transform;
		transform_build_matrix(t);
		PhysicsBody.vec1.setValue(transform_world_x(t), transform_world_y(t), transform_world_z(t));
		PhysicsBody.trans1.setOrigin(PhysicsBody.vec1);
		quat_from_mat(PhysicsBody.quat, t.world);
		PhysicsBody.quat1.setValue(PhysicsBody.quat.x, PhysicsBody.quat.y, PhysicsBody.quat.z, PhysicsBody.quat.w);
		PhysicsBody.trans1.setRotation(PhysicsBody.quat1);
		pb.body.setWorldTransform(PhysicsBody.trans1);
		if (pb.current_scale_x != t.scale.x || pb.current_scale_y != t.scale.y || pb.current_scale_z != t.scale.z) {
			PhysicsBody.set_scale(pb, t.scale);
		}
		PhysicsBody.activate(pb);
	}

	static set_ccd = (pb: PhysicsBodyRaw, sphereRadius: f32, motionThreshold = 1e-7) => {
		pb.body.setCcdSweptSphereRadius(sphereRadius);
		pb.body.setCcdMotionThreshold(motionThreshold);
	}

	static fill_convex_hull = (pb: PhysicsBodyRaw, scale: vec4_t, margin: f32): Ammo.btConvexHullShape => {
		// Check whether shape already exists
		let data: any = pb.object.ext.data;
		let shape: Ammo.btConvexHullShape = PhysicsBody.convex_hull_cache.get(data);
		if (shape != null) {
			PhysicsBody.users_cache.set(data, PhysicsBody.users_cache.get(data) + 1);
			return shape;
		}

		shape = new Ammo.btConvexHullShape();
		PhysicsBody.convex_hull_cache.set(data, shape);
		PhysicsBody.users_cache.set(data, 1);

		let positions: i16_array_t = mesh_data_get_vertex_array(data, 'pos').values;

		let sx: f32 = scale.x * (1.0 - margin) * (1 / 32767);
		let sy: f32 = scale.y * (1.0 - margin) * (1 / 32767);
		let sz: f32 = scale.z * (1.0 - margin) * (1 / 32767);

		sx *= data.scale_pos;
		sy *= data.scale_pos;
		sz *= data.scale_pos;

		for (let i: i32 = 0; i < Math.floor(positions.length / 4); ++i) {
			PhysicsBody.vec1.setX(positions[i * 4    ] * sx);
			PhysicsBody.vec1.setY(positions[i * 4 + 1] * sy);
			PhysicsBody.vec1.setZ(positions[i * 4 + 2] * sz);
			shape.addPoint(PhysicsBody.vec1, true);
		}
		return shape;
	}

	static fill_triangle_mesh = (pb: PhysicsBodyRaw, scale: vec4_t): Ammo.btTriangleMesh => {
		// Check whether shape already exists
		let data: any = pb.object.ext.data;
		let triangleMesh: Ammo.btTriangleMesh = PhysicsBody.triangle_mesh_cache.get(data);
		if (triangleMesh != null) {
			PhysicsBody.users_cache.set(data, PhysicsBody.users_cache.get(data) + 1);
			return triangleMesh;
		}

		triangleMesh = new Ammo.btTriangleMesh(true, true);
		PhysicsBody.triangle_mesh_cache.set(data, triangleMesh);
		PhysicsBody.users_cache.set(data, 1);

		let positions: i16_array_t = mesh_data_get_vertex_array(data, 'pos').values;
		let indices: any = data._indices;

		let sx: f32 = scale.x * (1 / 32767);
		let sy: f32 = scale.y * (1 / 32767);
		let sz: f32 = scale.z * (1 / 32767);

		sx *= data.scale_pos;
		sy *= data.scale_pos;
		sz *= data.scale_pos;

		for (let ar of indices) {
			for (let i: i32 = 0; i < Math.floor(ar.length / 3); ++i) {
				PhysicsBody.vec1.setX(positions[ar[i * 3    ] * 4    ] * sx);
				PhysicsBody.vec1.setY(positions[ar[i * 3    ] * 4 + 1] * sy);
				PhysicsBody.vec1.setZ(positions[ar[i * 3    ] * 4 + 2] * sz);
				PhysicsBody.vec2.setX(positions[ar[i * 3 + 1] * 4    ] * sx);
				PhysicsBody.vec2.setY(positions[ar[i * 3 + 1] * 4 + 1] * sy);
				PhysicsBody.vec2.setZ(positions[ar[i * 3 + 1] * 4 + 2] * sz);
				PhysicsBody.vec3.setX(positions[ar[i * 3 + 2] * 4    ] * sx);
				PhysicsBody.vec3.setY(positions[ar[i * 3 + 2] * 4 + 1] * sy);
				PhysicsBody.vec3.setZ(positions[ar[i * 3 + 2] * 4 + 2] * sz);
				triangleMesh.addTriangle(PhysicsBody.vec1, PhysicsBody.vec2, PhysicsBody.vec3);
			}
		}
		return triangleMesh;
	}

	static delete = (pb: PhysicsBodyRaw) => {
		Ammo.destroy(pb.motion_state);
		Ammo.destroy(pb.body);

		// Delete shape if no other user is found
		if (pb.shape == shape_type_t.CONVEX_HULL || pb.shape == shape_type_t.MESH) {
			let data: any = pb.object.ext.data;
			let i: i32 = PhysicsBody.users_cache.get(data) - 1;
			PhysicsBody.users_cache.set(data, i);
			if (i <= 0) {
				Ammo.destroy(pb.btshape);
				pb.shape == shape_type_t.CONVEX_HULL ?
					PhysicsBody.convex_hull_cache.delete(data) :
					PhysicsBody.triangle_mesh_cache.delete(data);
			}
		}
		else Ammo.destroy(pb.btshape);
	}
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
