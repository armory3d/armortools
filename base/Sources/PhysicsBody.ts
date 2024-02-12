
///if arm_physics

class PhysicsBodyRaw {
	_mass = 0.0;

	get mass(): f32 {
		return this._mass;
	}

	set mass(f: f32) {
		if (this.ready) {
			// remove();
			let t = new PhysicsBodyRaw();
			t._mass = f;
			PhysicsBody.init(t, this.object);
			(this.object as any).physicsBody = t;
		}
		else this._mass = f;
	}

	object: object_t;
	friction = 0.5;
	restitution = 0.0;
	collisionMargin = 0.0;
	linearDamping = 0.04;
	angularDamping = 0.1;
	linearFactors = [1.0, 1.0, 1.0];
	angularFactors = [1.0, 1.0, 1.0];
	linearThreshold = 0.0;
	angularThreshold = 0.0;
	ccd = false; // Continuous collision detection
	trigger = false;
	group = 1;
	mask = 1;
	shape = ShapeType.ShapeBox;
	destroyed = false;
	bodyScaleX: f32; // Transform scale at creation time
	bodyScaleY: f32;
	bodyScaleZ: f32;
	currentScaleX: f32;
	currentScaleY: f32;
	currentScaleZ: f32;

	body: Ammo.btRigidBody = null;
	motionState: Ammo.btMotionState;
	btshape: Ammo.btCollisionShape;
	ready = false;
	id = 0;
	heightData: Uint8Array = null;
}

class PhysicsBody {

	static nextId = 0;
	static ammoArray: i32 = -1;
	static gimpactRegistered = false;
	static first = true;
	static vec1: Ammo.btVector3;
	static vec2: Ammo.btVector3;
	static vec3: Ammo.btVector3;
	static quat1: Ammo.btQuaternion;
	static trans1: Ammo.btTransform;
	static trans2: Ammo.btTransform;
	static quat = quat_create();

	static convexHullCache = new Map<mesh_data_t, Ammo.btConvexHullShape>();
	static triangleMeshCache = new Map<mesh_data_t, Ammo.btTriangleMesh>();
	static usersCache = new Map<mesh_data_t, i32>();

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
		let pb = new PhysicsBodyRaw();
		return pb;
	}

	static withMargin = (pb: PhysicsBodyRaw, f: f32) => {
		return f - f * pb.collisionMargin;
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
		let transform = o.transform;
		let physics = PhysicsWorld.active;

		if (pb.shape == ShapeType.ShapeBox) {
			PhysicsWorld.vec1.setX(PhysicsBody.withMargin(pb, transform.dim.x / 2));
			PhysicsWorld.vec1.setY(PhysicsBody.withMargin(pb, transform.dim.y / 2));
			PhysicsWorld.vec1.setZ(PhysicsBody.withMargin(pb, transform.dim.z / 2));
			pb.btshape = new Ammo.btBoxShape(PhysicsWorld.vec1);
		}
		else if (pb.shape == ShapeType.ShapeSphere) {
			pb.btshape = new Ammo.btSphereShape(PhysicsBody.withMargin(pb, transform.dim.x / 2));
		}
		else if (pb.shape == ShapeType.ShapeConvexHull) {
			let shapeConvex = PhysicsBody.fillConvexHull(pb, transform.scale, pb.collisionMargin);
			pb.btshape = shapeConvex;
		}
		else if (pb.shape == ShapeType.ShapeCone) {
			let coneZ = new Ammo.btConeShapeZ(
				PhysicsBody.withMargin(pb, transform.dim.x / 2), // Radius
				PhysicsBody.withMargin(pb, transform.dim.z));	 // Height
			let cone: Ammo.btConeShape = coneZ;
			pb.btshape = cone;
		}
		else if (pb.shape == ShapeType.ShapeCylinder) {
			PhysicsWorld.vec1.setX(PhysicsBody.withMargin(pb, transform.dim.x / 2));
			PhysicsWorld.vec1.setY(PhysicsBody.withMargin(pb, transform.dim.y / 2));
			PhysicsWorld.vec1.setZ(PhysicsBody.withMargin(pb, transform.dim.z / 2));
			let cylZ = new Ammo.btCylinderShapeZ(PhysicsWorld.vec1);
			let cyl: Ammo.btCylinderShape = cylZ;
			pb.btshape = cyl;
		}
		else if (pb.shape == ShapeType.ShapeCapsule) {
			let r = transform.dim.x / 2;
			let capsZ = new Ammo.btCapsuleShapeZ(
				PhysicsBody.withMargin(pb, r), // Radius
				PhysicsBody.withMargin(pb, transform.dim.z - r * 2)); // Distance between 2 sphere centers
			let caps: Ammo.btCapsuleShape = capsZ;
			pb.btshape = caps;
		}
		else if (pb.shape == ShapeType.ShapeMesh) {
			let meshInterface = PhysicsBody.fillTriangleMesh(pb, transform.scale);
			if (pb.mass > 0) {
				let shapeGImpact = new Ammo.btGImpactMeshShape(meshInterface);
				shapeGImpact.updateBound();
				let shapeConcave: Ammo.btConcaveShape = shapeGImpact;
				pb.btshape = shapeConcave;
				if (!PhysicsBody.gimpactRegistered) {
					PhysicsBody.gimpactRegistered = true;
					new Ammo.GImpactCollisionAlgorithm().registerAlgorithm(physics.dispatcher);
				}
			}
			else {
				let shapeBvh = new Ammo.btBvhTriangleMeshShape(meshInterface, true, true);
				let shapeTri: Ammo.btTriangleMeshShape = shapeBvh;
				let shapeConcave: Ammo.btConcaveShape = shapeTri;
				pb.btshape = shapeConcave;
			}
		}
		else if (pb.shape == ShapeType.ShapeTerrain) {
			let length = pb.heightData.length;
			if (PhysicsBody.ammoArray == -1) {
				PhysicsBody.ammoArray = Ammo._malloc(length);
			}
			// From texture bytes
			for (let i = 0; i < length; ++i) {
				Ammo.HEAPU8[PhysicsBody.ammoArray + i] = pb.heightData[i];
			}
			let slice = Math.floor(Math.sqrt(length)); // Assuming square terrain data
			let axis = 2; // z
			let dataType = 5; // u8
			pb.btshape = new Ammo.btHeightfieldTerrainShape(slice, slice, PhysicsBody.ammoArray, 1 / 255, 0, 1, axis, dataType, false);
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
		pb.motionState = new Ammo.btDefaultMotionState(PhysicsBody.trans1, PhysicsBody.trans2); // Transform, center of mass offset

		PhysicsBody.vec1.setX(0);
		PhysicsBody.vec1.setY(0);
		PhysicsBody.vec1.setZ(0);
		let inertia = PhysicsBody.vec1;

		if (pb.mass > 0) {
			pb.btshape.calculateLocalInertia(pb.mass, inertia);
		}
		let bodyCI = new Ammo.btRigidBodyConstructionInfo(pb.mass, pb.motionState, pb.btshape, inertia);
		pb.body = new Ammo.btRigidBody(bodyCI);

		pb.body.setFriction(pb.friction);
		if (pb.shape == ShapeType.ShapeSphere || pb.shape == ShapeType.ShapeCylinder || pb.shape == ShapeType.ShapeCone || pb.shape == ShapeType.ShapeCapsule) {
			pb.angularDamping += pb.friction;
		}
		pb.body.setRestitution(pb.restitution);
		// pb.body.setSleepingThresholds(linearThreshold, angularThreshold);
		// pb.body.setDeactivationTime(deactivationTime);
		pb.body.setDamping(pb.linearDamping, pb.angularDamping);
		PhysicsBody.setLinearFactor(pb, pb.linearFactors[0], pb.linearFactors[1], pb.linearFactors[2]);
		PhysicsBody.setAngularFactor(pb, pb.angularFactors[0], pb.angularFactors[1], pb.angularFactors[2]);
		if (pb.trigger) {
			pb.body.setCollisionFlags(pb.body.getCollisionFlags() | CollisionFlags.CF_NO_CONTACT_RESPONSE);
		}
		if (pb.mass == 0.0) {
			pb.body.setCollisionFlags(pb.body.getCollisionFlags() | CollisionFlags.CF_STATIC_OBJECT);
		}
		if (pb.ccd) {
			PhysicsBody.setCcd(pb, transform.radius);
		}

		pb.bodyScaleX = pb.currentScaleX = transform.scale.x;
		pb.bodyScaleY = pb.currentScaleY = transform.scale.y;
		pb.bodyScaleZ = pb.currentScaleZ = transform.scale.z;

		pb.id = PhysicsBody.nextId++;
		pb.body.userIndex = pb.id;

		PhysicsWorld.addBody(physics, pb);

		// notifyOnRemove(removeFromWorld);

		Ammo.destroy(bodyCI);
	}

	static physicsUpdate = (pb: PhysicsBodyRaw) => {
		if (!pb.ready) {
			return;
		}
		let trans = pb.body.getWorldTransform();

		let p = trans.getOrigin();
		let q = trans.getRotation();
		let qw: Ammo.btQuadWord = q;

		let transform = pb.object.transform;
		vec4_set(transform.loc, p.x(), p.y(), p.z());
		quat_set(transform.rot, qw.x(), qw.y(), qw.z(), qw.w());
		if (pb.object.parent != null) {
			let ptransform = pb.object.parent.transform;
			transform.loc.x -= transform_world_x(ptransform);
			transform.loc.y -= transform_world_y(ptransform);
			transform.loc.z -= transform_world_z(ptransform);
		}
		transform_build_matrix(transform);
	}

	static removeFromWorld = (pb: PhysicsBodyRaw) => {
		PhysicsWorld.removeBody(PhysicsWorld.active, pb);
	}

	static activate = (pb: PhysicsBodyRaw) => {
		pb.body.activate(false);
	}

	static setGravity = (pb: PhysicsBodyRaw, v: vec4_t) => {
		PhysicsBody.vec1.setValue(v.x, v.y, v.z);
		pb.body.setGravity(PhysicsBody.vec1);
	}

	static applyForce = (pb: PhysicsBodyRaw, force: vec4_t, loc: vec4_t = null) => {
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

	static applyImpulse = (pb: PhysicsBodyRaw, impulse: vec4_t, loc: vec4_t = null) => {
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

	static applyTorque = (pb: PhysicsBodyRaw, torque: vec4_t) => {
		PhysicsBody.activate(pb);
		PhysicsBody.vec1.setValue(torque.x, torque.y, torque.z);
		pb.body.applyTorque(PhysicsBody.vec1);
	}

	static applyTorqueImpulse = (pb: PhysicsBodyRaw, torque: vec4_t) => {
		PhysicsBody.activate(pb);
		PhysicsBody.vec1.setValue(torque.x, torque.y, torque.z);
		pb.body.applyTorqueImpulse(PhysicsBody.vec1);
	}

	static setLinearFactor = (pb: PhysicsBodyRaw, x: f32, y: f32, z: f32) => {
		PhysicsBody.vec1.setValue(x, y, z);
		pb.body.setLinearFactor(PhysicsBody.vec1);
	}

	static setAngularFactor = (pb: PhysicsBodyRaw, x: f32, y: f32, z: f32) => {
		PhysicsBody.vec1.setValue(x, y, z);
		pb.body.setAngularFactor(PhysicsBody.vec1);
	}

	static getLinearVelocity = (pb: PhysicsBodyRaw): vec4_t => {
		let v = pb.body.getLinearVelocity();
		return vec4_create(v.x(), v.y(), v.z());
	}

	static setLinearVelocity = (pb: PhysicsBodyRaw, x: f32, y: f32, z: f32) => {
		PhysicsBody.vec1.setValue(x, y, z);
		pb.body.setLinearVelocity(PhysicsBody.vec1);
	}

	static getAngularVelocity = (pb: PhysicsBodyRaw): vec4_t => {
		let v = pb.body.getAngularVelocity();
		return vec4_create(v.x(), v.y(), v.z());
	}

	static setAngularVelocity = (pb: PhysicsBodyRaw, x: f32, y: f32, z: f32) => {
		PhysicsBody.vec1.setValue(x, y, z);
		pb.body.setAngularVelocity(PhysicsBody.vec1);
	}

	static setFriction = (pb: PhysicsBodyRaw, f: f32) => {
		pb.body.setFriction(f);
		pb.friction = f;
	}

	static setScale = (pb: PhysicsBodyRaw, v: vec4_t) => {
		pb.currentScaleX = v.x;
		pb.currentScaleY = v.y;
		pb.currentScaleZ = v.z;
		PhysicsBody.vec1.setX(v.x / pb.bodyScaleX);
		PhysicsBody.vec1.setY(v.y / pb.bodyScaleY);
		PhysicsBody.vec1.setZ(v.z / pb.bodyScaleZ);
		pb.btshape.setLocalScaling(PhysicsBody.vec1);
		let worldDyn: Ammo.btDynamicsWorld = PhysicsWorld.active.world;
		let worldCol: Ammo.btCollisionWorld = worldDyn;
		worldCol.updateSingleAabb(pb.body);
	}

	static syncTransform = (pb: PhysicsBodyRaw) => {
		let t = pb.object.transform;
		transform_build_matrix(t);
		PhysicsBody.vec1.setValue(transform_world_x(t), transform_world_y(t), transform_world_z(t));
		PhysicsBody.trans1.setOrigin(PhysicsBody.vec1);
		quat_from_mat(PhysicsBody.quat, t.world);
		PhysicsBody.quat1.setValue(PhysicsBody.quat.x, PhysicsBody.quat.y, PhysicsBody.quat.z, PhysicsBody.quat.w);
		PhysicsBody.trans1.setRotation(PhysicsBody.quat1);
		pb.body.setWorldTransform(PhysicsBody.trans1);
		if (pb.currentScaleX != t.scale.x || pb.currentScaleY != t.scale.y || pb.currentScaleZ != t.scale.z) {
			PhysicsBody.setScale(pb, t.scale);
		}
		PhysicsBody.activate(pb);
	}

	static setCcd = (pb: PhysicsBodyRaw, sphereRadius: f32, motionThreshold = 1e-7) => {
		pb.body.setCcdSweptSphereRadius(sphereRadius);
		pb.body.setCcdMotionThreshold(motionThreshold);
	}

	static fillConvexHull = (pb: PhysicsBodyRaw, scale: vec4_t, margin: f32): Ammo.btConvexHullShape => {
		// Check whether shape already exists
		let data = pb.object.ext.data;
		let shape = PhysicsBody.convexHullCache.get(data);
		if (shape != null) {
			PhysicsBody.usersCache.set(data, PhysicsBody.usersCache.get(data) + 1);
			return shape;
		}

		shape = new Ammo.btConvexHullShape();
		PhysicsBody.convexHullCache.set(data, shape);
		PhysicsBody.usersCache.set(data, 1);

		let positions = mesh_data_get_vertex_array(data, 'pos').values;

		let sx: f32 = scale.x * (1.0 - margin) * (1 / 32767);
		let sy: f32 = scale.y * (1.0 - margin) * (1 / 32767);
		let sz: f32 = scale.z * (1.0 - margin) * (1 / 32767);

		sx *= data.scale_pos;
		sy *= data.scale_pos;
		sz *= data.scale_pos;

		for (let i = 0; i < Math.floor(positions.length / 4); ++i) {
			PhysicsBody.vec1.setX(positions[i * 4    ] * sx);
			PhysicsBody.vec1.setY(positions[i * 4 + 1] * sy);
			PhysicsBody.vec1.setZ(positions[i * 4 + 2] * sz);
			shape.addPoint(PhysicsBody.vec1, true);
		}
		return shape;
	}

	static fillTriangleMesh = (pb: PhysicsBodyRaw, scale: vec4_t): Ammo.btTriangleMesh => {
		// Check whether shape already exists
		let data = pb.object.ext.data;
		let triangleMesh = PhysicsBody.triangleMeshCache.get(data);
		if (triangleMesh != null) {
			PhysicsBody.usersCache.set(data, PhysicsBody.usersCache.get(data) + 1);
			return triangleMesh;
		}

		triangleMesh = new Ammo.btTriangleMesh(true, true);
		PhysicsBody.triangleMeshCache.set(data, triangleMesh);
		PhysicsBody.usersCache.set(data, 1);

		let positions = mesh_data_get_vertex_array(data, 'pos').values;
		let indices = data._indices;

		let sx: f32 = scale.x * (1 / 32767);
		let sy: f32 = scale.y * (1 / 32767);
		let sz: f32 = scale.z * (1 / 32767);

		sx *= data.scale_pos;
		sy *= data.scale_pos;
		sz *= data.scale_pos;

		for (let ar of indices) {
			for (let i = 0; i < Math.floor(ar.length / 3); ++i) {
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
		Ammo.destroy(pb.motionState);
		Ammo.destroy(pb.body);

		// Delete shape if no other user is found
		if (pb.shape == ShapeType.ShapeConvexHull || pb.shape == ShapeType.ShapeMesh) {
			let data = pb.object.ext.data;
			let i = PhysicsBody.usersCache.get(data) - 1;
			PhysicsBody.usersCache.set(data, i);
			if (i <= 0) {
				Ammo.destroy(pb.btshape);
				pb.shape == ShapeType.ShapeConvexHull ?
					PhysicsBody.convexHullCache.delete(data) :
					PhysicsBody.triangleMeshCache.delete(data);
			}
		}
		else Ammo.destroy(pb.btshape);
	}
}

enum ShapeType {
	ShapeBox,
	ShapeSphere,
	ShapeConvexHull,
	ShapeMesh,
	ShapeCone,
	ShapeCylinder,
	ShapeCapsule,
	ShapeTerrain,
}

enum CollisionFlags {
	CF_STATIC_OBJECT = 1,
	CF_KINEMATIC_OBJECT = 2,
	CF_NO_CONTACT_RESPONSE = 4,
	CF_CHARACTER_OBJECT = 16,
}

// static ACTIVE_TAG = 1;
// static ISLAND_SLEEPING = 2;
// static WANTS_DEACTIVATION = 3;
// static DISABLE_DEACTIVATION = 4;
// static DISABLE_SIMULATION = 5;

///end
