
///if arm_physics

class PhysicsBody {

	// @:keep
	props = ["mass"];

	mass = 0.0;

	// @:keep
	set mass(f: f32): f32 {
		if (ready) {
			// remove();
			let t = new PhysicsBody();
			t.mass = f;
			t.init(object);
			object.addTrait(t);
		}
		else mass = f;
		return f;
	}

	object: BaseObject;
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
	shape = ShapeBox;
	destroyed = false;
	bodyScaleX: f32; // Transform scale at creation time
	bodyScaleY: f32;
	bodyScaleZ: f32;
	currentScaleX: f32;
	currentScaleY: f32;
	currentScaleZ: f32;

	body: PhysicsBullet.RigidBody = null;
	motionState: PhysicsBullet.MotionState;
	btshape: PhysicsBullet.CollisionShape;
	ready = false;
	id = 0;
	heightData: Uint8Array = null;

	static nextId = 0;
	static ammoArray: i32 = -1;
	static gimpactRegistered = false;
	static first = true;
	static vec1: PhysicsBullet.Vector3;
	static vec2: PhysicsBullet.Vector3;
	static vec3: PhysicsBullet.Vector3;
	static quat1: PhysicsBullet.Quaternion;
	static trans1: PhysicsBullet.Transform;
	static trans2: PhysicsBullet.Transform;
	static quat = new Quat();

	static convexHullCache = new Map<MeshData, PhysicsBullet.ConvexHullShape>();
	static triangleMeshCache = new Map<MeshData, PhysicsBullet.TriangleMesh>();
	static usersCache = new Map<MeshData, i32>();

	constructor() {
		if (first) {
			first = false;
			vec1 = new PhysicsBullet.Vector3(0, 0, 0);
			vec2 = new PhysicsBullet.Vector3(0, 0, 0);
			vec3 = new PhysicsBullet.Vector3(0, 0, 0);
			quat1 = new PhysicsBullet.Quaternion(0, 0, 0, 0);
			trans1 = new PhysicsBullet.Transform();
			trans2 = new PhysicsBullet.Transform();
		}
	}

	withMargin(f: f32) {
		return f - f * collisionMargin;
	}

	init(o: BaseObject) {
		object = o;
		if (ready) return;
		ready = true;

		if (object.constructor != MeshObject) return; // No mesh data
		let transform = object.transform;
		let physics = PhysicsWorld.active;

		if (shape == ShapeBox) {
			vec1.setX(withMargin(transform.dim.x / 2));
			vec1.setY(withMargin(transform.dim.y / 2));
			vec1.setZ(withMargin(transform.dim.z / 2));
			btshape = new PhysicsBullet.BoxShape(vec1);
		}
		else if (shape == ShapeSphere) {
			btshape = new PhysicsBullet.SphereShape(withMargin(transform.dim.x / 2));
		}
		else if (shape == ShapeConvexHull) {
			let shapeConvex = fillConvexHull(transform.scale, collisionMargin);
			btshape = shapeConvex;
		}
		else if (shape == ShapeCone) {
			let coneZ = new PhysicsBullet.ConeShapeZ(
				withMargin(transform.dim.x / 2), // Radius
				withMargin(transform.dim.z));	 // Height
			let cone: PhysicsBullet.ConeShape = coneZ;
			btshape = cone;
		}
		else if (shape == ShapeCylinder) {
			vec1.setX(withMargin(transform.dim.x / 2));
			vec1.setY(withMargin(transform.dim.y / 2));
			vec1.setZ(withMargin(transform.dim.z / 2));
			let cylZ = new PhysicsBullet.CylinderShapeZ(vec1);
			let cyl: PhysicsBullet.CylinderShape = cylZ;
			btshape = cyl;
		}
		else if (shape == ShapeCapsule) {
			let r = transform.dim.x / 2;
			let capsZ = new PhysicsBullet.CapsuleShapeZ(
				withMargin(r), // Radius
				withMargin(transform.dim.z - r * 2)); // Distance between 2 sphere centers
			let caps: PhysicsBullet.CapsuleShape = capsZ;
			btshape = caps;
		}
		else if (shape == ShapeMesh) {
			let meshInterface = fillTriangleMesh(transform.scale);
			if (mass > 0) {
				let shapeGImpact = new PhysicsBullet.GImpactMeshShape(meshInterface);
				shapeGImpact.updateBound();
				let shapeConcave: PhysicsBullet.ConcaveShape = shapeGImpact;
				btshape = shapeConcave;
				if (!gimpactRegistered) {
					gimpactRegistered = true;
					new PhysicsBullet.GImpactCollisionAlgorithm().registerAlgorithm(physics.dispatcher);
				}
			}
			else {
				let shapeBvh = new PhysicsBullet.BvhTriangleMeshShape(meshInterface, true, true);
				let shapeTri: PhysicsBullet.TriangleMeshShape = shapeBvh;
				let shapeConcave: PhysicsBullet.ConcaveShape = shapeTri;
				btshape = shapeConcave;
			}
		}
		else if (shape == ShapeTerrain) {
			let length = heightData.length;
			if (ammoArray == -1) {
				ammoArray = PhysicsBullet.Ammo._malloc(length);
			}
			// From texture bytes
			for (let i = 0; i < length; ++i) {
				PhysicsBullet.Ammo.HEAPU8[ammoArray + i] = heightData[i];
			}
			let slice = Math.floor(Math.sqrt(length)); // Assuming square terrain data
			let axis = 2; // z
			let dataType = 5; // u8
			btshape = new PhysicsBullet.HeightfieldTerrainShape(slice, slice, ammoArray, 1 / 255, 0, 1, axis, dataType, false);
			vec1.setX(transform.dim.x / slice);
			vec1.setY(transform.dim.y / slice);
			vec1.setZ(transform.dim.z);
			btshape.setLocalScaling(vec1);
		}

		trans1.setIdentity();
		vec1.setX(transform.worldx());
		vec1.setY(transform.worldy());
		vec1.setZ(transform.worldz());
		trans1.setOrigin(vec1);
		quat.fromMat(transform.world);
		quat1.setValue(quat.x, quat.y, quat.z, quat.w);
		trans1.setRotation(quat1);
		trans2.setIdentity();
		motionState = new PhysicsBullet.DefaultMotionState(trans1, trans2); // Transform, center of mass offset

		vec1.setX(0);
		vec1.setY(0);
		vec1.setZ(0);
		let inertia = vec1;

		if (mass > 0) btshape.calculateLocalInertia(mass, inertia);
		let bodyCI = new PhysicsBullet.RigidBodyConstructionInfo(mass, motionState, btshape, inertia);
		body = new PhysicsBullet.RigidBody(bodyCI);

		body.setFriction(friction);
		if (shape == ShapeSphere || shape == ShapeCylinder || shape == ShapeCone || shape == ShapeCapsule) {
			angularDamping += friction;
		}
		body.setRestitution(restitution);
		// body.setSleepingThresholds(linearThreshold, angularThreshold);
		// body.setDeactivationTime(deactivationTime);
		body.setDamping(linearDamping, angularDamping);
		setLinearFactor(linearFactors[0], linearFactors[1], linearFactors[2]);
		setAngularFactor(angularFactors[0], angularFactors[1], angularFactors[2]);
		if (trigger) body.setCollisionFlags(body.getCollisionFlags() | PhysicsBullet.CollisionObject.CF_NO_CONTACT_RESPONSE);
		if (mass == 0.0) body.setCollisionFlags(body.getCollisionFlags() | PhysicsBullet.CollisionObject.CF_STATIC_OBJECT);
		if (ccd) setCcd(transform.radius);

		bodyScaleX = currentScaleX = transform.scale.x;
		bodyScaleY = currentScaleY = transform.scale.y;
		bodyScaleZ = currentScaleZ = transform.scale.z;

		id = nextId++;
		body.userIndex = id;

		physics.addBody(this);

		// notifyOnRemove(removeFromWorld);

		PhysicsBullet.Ammo.destroy(bodyCI);
	}

	physicsUpdate() {
		if (!ready) return;
		let trans = body.getWorldTransform();

		let p = trans.getOrigin();
		let q = trans.getRotation();
		let qw: PhysicsBullet.QuadWord = q;

		let transform = object.transform;
		transform.loc.set(p.x(), p.y(), p.z());
		transform.rot.set(qw.x(), qw.y(), qw.z(), qw.w());
		if (object.parent != null) {
			let ptransform = object.parent.transform;
			transform.loc.x -= ptransform.worldx();
			transform.loc.y -= ptransform.worldy();
			transform.loc.z -= ptransform.worldz();
		}
		transform.buildMatrix();
	}

	removeFromWorld() {
		PhysicsWorld.active.removeBody(this);
	}

	activate() {
		body.activate(false);
	}

	setGravity(v: Vec4) {
		vec1.setValue(v.x, v.y, v.z);
		body.setGravity(vec1);
	}

	applyForce(force: Vec4, loc: Vec4 = null) {
		activate();
		vec1.setValue(force.x, force.y, force.z);
		if (loc == null) {
			body.applyCentralForce(vec1);
		}
		else {
			vec2.setValue(loc.x, loc.y, loc.z);
			body.applyForce(vec1, vec2);
		}
	}

	applyImpulse(impulse: Vec4, loc: Vec4 = null) {
		activate();
		vec1.setValue(impulse.x, impulse.y, impulse.z);
		if (loc == null) {
			body.applyCentralImpulse(vec1);
		}
		else {
			vec2.setValue(loc.x, loc.y, loc.z);
			body.applyImpulse(vec1, vec2);
		}
	}

	applyTorque(torque: Vec4) {
		activate();
		vec1.setValue(torque.x, torque.y, torque.z);
		body.applyTorque(vec1);
	}

	applyTorqueImpulse(torque: Vec4) {
		activate();
		vec1.setValue(torque.x, torque.y, torque.z);
		body.applyTorqueImpulse(vec1);
	}

	setLinearFactor(x: f32, y: f32, z: f32) {
		vec1.setValue(x, y, z);
		body.setLinearFactor(vec1);
	}

	setAngularFactor(x: f32, y: f32, z: f32) {
		vec1.setValue(x, y, z);
		body.setAngularFactor(vec1);
	}

	getLinearVelocity(): Vec4 {
		let v = body.getLinearVelocity();
		return new Vec4(v.x(), v.y(), v.z());
	}

	setLinearVelocity(x: f32, y: f32, z: f32) {
		vec1.setValue(x, y, z);
		body.setLinearVelocity(vec1);
	}

	getAngularVelocity(): Vec4 {
		let v = body.getAngularVelocity();
		return new Vec4(v.x(), v.y(), v.z());
	}

	setAngularVelocity(x: f32, y: f32, z: f32) {
		vec1.setValue(x, y, z);
		body.setAngularVelocity(vec1);
	}

	setFriction(f: f32) {
		body.setFriction(f);
		this.friction = f;
	}

	setScale(v: Vec4) {
		currentScaleX = v.x;
		currentScaleY = v.y;
		currentScaleZ = v.z;
		vec1.setX(v.x / bodyScaleX);
		vec1.setY(v.y / bodyScaleY);
		vec1.setZ(v.z / bodyScaleZ);
		btshape.setLocalScaling(vec1);
		let worldDyn: PhysicsBullet.DynamicsWorld = PhysicsWorld.active.world;
		let worldCol: PhysicsBullet.CollisionWorld = worldDyn;
		worldCol.updateSingleAabb(body);
	}

	syncTransform() {
		let t = object.transform;
		t.buildMatrix();
		vec1.setValue(t.worldx(), t.worldy(), t.worldz());
		trans1.setOrigin(vec1);
		quat.fromMat(t.world);
		quat1.setValue(quat.x, quat.y, quat.z, quat.w);
		trans1.setRotation(quat1);
		body.setWorldTransform(trans1);
		if (currentScaleX != t.scale.x || currentScaleY != t.scale.y || currentScaleZ != t.scale.z) setScale(t.scale);
		activate();
	}

	setCcd(sphereRadius: f32, motionThreshold = 1e-7) {
		body.setCcdSweptSphereRadius(sphereRadius);
		body.setCcdMotionThreshold(motionThreshold);
	}

	fillConvexHull(scale: Vec4, margin: f32): PhysicsBullet.ConvexHullShape {
		// Check whether shape already exists
		let data = cast(object, MeshObject).data;
		let shape = convexHullCache.get(data);
		if (shape != null) {
			usersCache.set(data, usersCache.get(data) + 1);
			return shape;
		}

		shape = new PhysicsBullet.ConvexHullShape();
		convexHullCache.set(data, shape);
		usersCache.set(data, 1);

		let positions = data.positions.values;

		let sx: f32 = scale.x * (1.0 - margin) * (1 / 32767);
		let sy: f32 = scale.y * (1.0 - margin) * (1 / 32767);
		let sz: f32 = scale.z * (1.0 - margin) * (1 / 32767);

		if (data.raw.scale_pos != null) {
			sx *= data.raw.scale_pos;
			sy *= data.raw.scale_pos;
			sz *= data.raw.scale_pos;
		}

		for (let i = 0; i < Math.floor(positions.length / 4); ++i) {
			vec1.setX(positions[i * 4    ] * sx);
			vec1.setY(positions[i * 4 + 1] * sy);
			vec1.setZ(positions[i * 4 + 2] * sz);
			shape.addPoint(vec1, true);
		}
		return shape;
	}

	fillTriangleMesh(scale: Vec4): PhysicsBullet.TriangleMesh {
		// Check whether shape already exists
		let data = cast(object, MeshObject).data;
		let triangleMesh = triangleMeshCache.get(data);
		if (triangleMesh != null) {
			usersCache.set(data, usersCache.get(data) + 1);
			return triangleMesh;
		}

		triangleMesh = new PhysicsBullet.TriangleMesh(true, true);
		triangleMeshCache.set(data, triangleMesh);
		usersCache.set(data, 1);

		let positions = data.positions.values;
		let indices = data.indices;

		let sx: f32 = scale.x * (1 / 32767);
		let sy: f32 = scale.y * (1 / 32767);
		let sz: f32 = scale.z * (1 / 32767);

		if (data.raw.scale_pos != null) {
			sx *= data.raw.scale_pos;
			sy *= data.raw.scale_pos;
			sz *= data.raw.scale_pos;
		}

		for (let ar of indices) {
			for (let i = 0; i < Math.floor(ar.length / 3); ++i) {
				vec1.setX(positions[ar[i * 3    ] * 4    ] * sx);
				vec1.setY(positions[ar[i * 3    ] * 4 + 1] * sy);
				vec1.setZ(positions[ar[i * 3    ] * 4 + 2] * sz);
				vec2.setX(positions[ar[i * 3 + 1] * 4    ] * sx);
				vec2.setY(positions[ar[i * 3 + 1] * 4 + 1] * sy);
				vec2.setZ(positions[ar[i * 3 + 1] * 4 + 2] * sz);
				vec3.setX(positions[ar[i * 3 + 2] * 4    ] * sx);
				vec3.setY(positions[ar[i * 3 + 2] * 4 + 1] * sy);
				vec3.setZ(positions[ar[i * 3 + 2] * 4 + 2] * sz);
				triangleMesh.addTriangle(vec1, vec2, vec3);
			}
		}
		return triangleMesh;
	}

	delete() {
		PhysicsBullet.Ammo.destroy(motionState);
		PhysicsBullet.Ammo.destroy(body);

		// Delete shape if no other user is found
		if (shape == ShapeConvexHull || shape == ShapeMesh) {
			let data = cast(object, MeshObject).data;
			let i = usersCache.get(data) - 1;
			usersCache.set(data, i);
			if (i <= 0) {
				PhysicsBullet.Ammo.destroy(btshape);
				shape == ShapeConvexHull ?
					convexHullCache.remove(data) :
					triangleMeshCache.remove(data);
			}
		}
		else PhysicsBullet.Ammo.destroy(btshape);
	}
}

enum ShapeType {
	ShapeBox = 0;
	ShapeSphere = 1;
	ShapeConvexHull = 2;
	ShapeMesh = 3;
	ShapeCone = 4;
	ShapeCylinder = 5;
	ShapeCapsule = 6;
	ShapeTerrain = 7;
}

///end
