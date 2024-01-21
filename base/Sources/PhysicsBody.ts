
///if arm_physics

class PhysicsBody {

	props = ["mass"];

	_mass = 0.0;

	get mass(): f32 {
		return this._mass;
	}

	set mass(f: f32) {
		if (this.ready) {
			// remove();
			let t = new PhysicsBody();
			t._mass = f;
			t.init(this.object);
			this.object.addTrait(t);
		}
		else this._mass = f;
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
	static quat = new Quat();

	static convexHullCache = new Map<MeshData, Ammo.btConvexHullShape>();
	static triangleMeshCache = new Map<MeshData, Ammo.btTriangleMesh>();
	static usersCache = new Map<MeshData, i32>();

	constructor() {
		if (PhysicsBody.first) {
			PhysicsBody.first = false;
			PhysicsBody.vec1 = new Ammo.btVector3(0, 0, 0);
			PhysicsBody.vec2 = new Ammo.btVector3(0, 0, 0);
			PhysicsBody.vec3 = new Ammo.btVector3(0, 0, 0);
			PhysicsBody.quat1 = new Ammo.btQuaternion(0, 0, 0, 0);
			PhysicsBody.trans1 = new Ammo.btTransform();
			PhysicsBody.trans2 = new Ammo.btTransform();
		}
	}

	withMargin = (f: f32) => {
		return f - f * this.collisionMargin;
	}

	init = (o: BaseObject) => {
		this.object = o;
		if (this.ready) return;
		this.ready = true;

		if (this.object.constructor != MeshObject) return; // No mesh data
		let transform = o.transform;
		let physics = PhysicsWorld.active;

		if (this.shape == ShapeType.ShapeBox) {
			PhysicsWorld.vec1.setX(this.withMargin(transform.dim.x / 2));
			PhysicsWorld.vec1.setY(this.withMargin(transform.dim.y / 2));
			PhysicsWorld.vec1.setZ(this.withMargin(transform.dim.z / 2));
			this.btshape = new Ammo.btBoxShape(PhysicsWorld.vec1);
		}
		else if (this.shape == ShapeType.ShapeSphere) {
			this.btshape = new Ammo.btSphereShape(this.withMargin(transform.dim.x / 2));
		}
		else if (this.shape == ShapeType.ShapeConvexHull) {
			let shapeConvex = this.fillConvexHull(transform.scale, this.collisionMargin);
			this.btshape = shapeConvex;
		}
		else if (this.shape == ShapeType.ShapeCone) {
			let coneZ = new Ammo.btConeShapeZ(
				this.withMargin(transform.dim.x / 2), // Radius
				this.withMargin(transform.dim.z));	 // Height
			let cone: Ammo.btConeShape = coneZ;
			this.btshape = cone;
		}
		else if (this.shape == ShapeType.ShapeCylinder) {
			PhysicsWorld.vec1.setX(this.withMargin(transform.dim.x / 2));
			PhysicsWorld.vec1.setY(this.withMargin(transform.dim.y / 2));
			PhysicsWorld.vec1.setZ(this.withMargin(transform.dim.z / 2));
			let cylZ = new Ammo.btCylinderShapeZ(PhysicsWorld.vec1);
			let cyl: Ammo.btCylinderShape = cylZ;
			this.btshape = cyl;
		}
		else if (this.shape == ShapeType.ShapeCapsule) {
			let r = transform.dim.x / 2;
			let capsZ = new Ammo.btCapsuleShapeZ(
				this.withMargin(r), // Radius
				this.withMargin(transform.dim.z - r * 2)); // Distance between 2 sphere centers
			let caps: Ammo.btCapsuleShape = capsZ;
			this.btshape = caps;
		}
		else if (this.shape == ShapeType.ShapeMesh) {
			let meshInterface = this.fillTriangleMesh(transform.scale);
			if (this.mass > 0) {
				let shapeGImpact = new Ammo.btGImpactMeshShape(meshInterface);
				shapeGImpact.updateBound();
				let shapeConcave: Ammo.btConcaveShape = shapeGImpact;
				this.btshape = shapeConcave;
				if (!PhysicsBody.gimpactRegistered) {
					PhysicsBody.gimpactRegistered = true;
					new Ammo.GImpactCollisionAlgorithm().registerAlgorithm(physics.dispatcher);
				}
			}
			else {
				let shapeBvh = new Ammo.btBvhTriangleMeshShape(meshInterface, true, true);
				let shapeTri: Ammo.btTriangleMeshShape = shapeBvh;
				let shapeConcave: Ammo.btConcaveShape = shapeTri;
				this.btshape = shapeConcave;
			}
		}
		else if (this.shape == ShapeType.ShapeTerrain) {
			let length = this.heightData.length;
			if (PhysicsBody.ammoArray == -1) {
				PhysicsBody.ammoArray = Ammo._malloc(length);
			}
			// From texture bytes
			for (let i = 0; i < length; ++i) {
				Ammo.HEAPU8[PhysicsBody.ammoArray + i] = this.heightData[i];
			}
			let slice = Math.floor(Math.sqrt(length)); // Assuming square terrain data
			let axis = 2; // z
			let dataType = 5; // u8
			this.btshape = new Ammo.btHeightfieldTerrainShape(slice, slice, PhysicsBody.ammoArray, 1 / 255, 0, 1, axis, dataType, false);
			PhysicsBody.vec1.setX(transform.dim.x / slice);
			PhysicsBody.vec1.setY(transform.dim.y / slice);
			PhysicsBody.vec1.setZ(transform.dim.z);
			this.btshape.setLocalScaling(PhysicsBody.vec1);
		}

		PhysicsBody.trans1.setIdentity();
		PhysicsBody.vec1.setX(transform.worldx());
		PhysicsBody.vec1.setY(transform.worldy());
		PhysicsBody.vec1.setZ(transform.worldz());
		PhysicsBody.trans1.setOrigin(PhysicsBody.vec1);
		PhysicsBody.quat.fromMat(transform.world);
		PhysicsBody.quat1.setValue(PhysicsBody.quat.x, PhysicsBody.quat.y, PhysicsBody.quat.z, PhysicsBody.quat.w);
		PhysicsBody.trans1.setRotation(PhysicsBody.quat1);
		PhysicsBody.trans2.setIdentity();
		this.motionState = new Ammo.btDefaultMotionState(PhysicsBody.trans1, PhysicsBody.trans2); // Transform, center of mass offset

		PhysicsBody.vec1.setX(0);
		PhysicsBody.vec1.setY(0);
		PhysicsBody.vec1.setZ(0);
		let inertia = PhysicsBody.vec1;

		if (this.mass > 0) this.btshape.calculateLocalInertia(this.mass, inertia);
		let bodyCI = new Ammo.btRigidBodyConstructionInfo(this.mass, this.motionState, this.btshape, inertia);
		this.body = new Ammo.btRigidBody(bodyCI);

		this.body.setFriction(this.friction);
		if (this.shape == ShapeType.ShapeSphere || this.shape == ShapeType.ShapeCylinder || this.shape == ShapeType.ShapeCone || this.shape == ShapeType.ShapeCapsule) {
			this.angularDamping += this.friction;
		}
		this.body.setRestitution(this.restitution);
		// this.body.setSleepingThresholds(linearThreshold, angularThreshold);
		// this.body.setDeactivationTime(deactivationTime);
		this.body.setDamping(this.linearDamping, this.angularDamping);
		this.setLinearFactor(this.linearFactors[0], this.linearFactors[1], this.linearFactors[2]);
		this.setAngularFactor(this.angularFactors[0], this.angularFactors[1], this.angularFactors[2]);
		if (this.trigger) this.body.setCollisionFlags(this.body.getCollisionFlags() | CollisionFlags.CF_NO_CONTACT_RESPONSE);
		if (this.mass == 0.0) this.body.setCollisionFlags(this.body.getCollisionFlags() | CollisionFlags.CF_STATIC_OBJECT);
		if (this.ccd) this.setCcd(transform.radius);

		this.bodyScaleX = this.currentScaleX = transform.scale.x;
		this.bodyScaleY = this.currentScaleY = transform.scale.y;
		this.bodyScaleZ = this.currentScaleZ = transform.scale.z;

		this.id = PhysicsBody.nextId++;
		this.body.userIndex = this.id;

		physics.addBody(this);

		// notifyOnRemove(removeFromWorld);

		Ammo.destroy(bodyCI);
	}

	physicsUpdate = () => {
		if (!this.ready) return;
		let trans = this.body.getWorldTransform();

		let p = trans.getOrigin();
		let q = trans.getRotation();
		let qw: Ammo.btQuadWord = q;

		let transform = this.object.transform;
		transform.loc.set(p.x(), p.y(), p.z());
		transform.rot.set(qw.x(), qw.y(), qw.z(), qw.w());
		if (this.object.parent != null) {
			let ptransform = this.object.parent.transform;
			transform.loc.x -= ptransform.worldx();
			transform.loc.y -= ptransform.worldy();
			transform.loc.z -= ptransform.worldz();
		}
		transform.buildMatrix();
	}

	removeFromWorld = () => {
		PhysicsWorld.active.removeBody(this);
	}

	activate = () => {
		this.body.activate(false);
	}

	setGravity = (v: Vec4) => {
		PhysicsBody.vec1.setValue(v.x, v.y, v.z);
		this.body.setGravity(PhysicsBody.vec1);
	}

	applyForce = (force: Vec4, loc: Vec4 = null) => {
		this.activate();
		PhysicsBody.vec1.setValue(force.x, force.y, force.z);
		if (loc == null) {
			this.body.applyCentralForce(PhysicsBody.vec1);
		}
		else {
			PhysicsBody.vec2.setValue(loc.x, loc.y, loc.z);
			this.body.applyForce(PhysicsBody.vec1, PhysicsBody.vec2);
		}
	}

	applyImpulse = (impulse: Vec4, loc: Vec4 = null) => {
		this.activate();
		PhysicsBody.vec1.setValue(impulse.x, impulse.y, impulse.z);
		if (loc == null) {
			this.body.applyCentralImpulse(PhysicsBody.vec1);
		}
		else {
			PhysicsBody.vec2.setValue(loc.x, loc.y, loc.z);
			this.body.applyImpulse(PhysicsBody.vec1, PhysicsBody.vec2);
		}
	}

	applyTorque = (torque: Vec4) => {
		this.activate();
		PhysicsBody.vec1.setValue(torque.x, torque.y, torque.z);
		this.body.applyTorque(PhysicsBody.vec1);
	}

	applyTorqueImpulse = (torque: Vec4) => {
		this.activate();
		PhysicsBody.vec1.setValue(torque.x, torque.y, torque.z);
		this.body.applyTorqueImpulse(PhysicsBody.vec1);
	}

	setLinearFactor = (x: f32, y: f32, z: f32) => {
		PhysicsBody.vec1.setValue(x, y, z);
		this.body.setLinearFactor(PhysicsBody.vec1);
	}

	setAngularFactor = (x: f32, y: f32, z: f32) => {
		PhysicsBody.vec1.setValue(x, y, z);
		this.body.setAngularFactor(PhysicsBody.vec1);
	}

	getLinearVelocity = (): Vec4 => {
		let v = this.body.getLinearVelocity();
		return new Vec4(v.x(), v.y(), v.z());
	}

	setLinearVelocity = (x: f32, y: f32, z: f32) => {
		PhysicsBody.vec1.setValue(x, y, z);
		this.body.setLinearVelocity(PhysicsBody.vec1);
	}

	getAngularVelocity = (): Vec4 => {
		let v = this.body.getAngularVelocity();
		return new Vec4(v.x(), v.y(), v.z());
	}

	setAngularVelocity = (x: f32, y: f32, z: f32) => {
		PhysicsBody.vec1.setValue(x, y, z);
		this.body.setAngularVelocity(PhysicsBody.vec1);
	}

	setFriction = (f: f32) => {
		this.body.setFriction(f);
		this.friction = f;
	}

	setScale = (v: Vec4) => {
		this.currentScaleX = v.x;
		this.currentScaleY = v.y;
		this.currentScaleZ = v.z;
		PhysicsBody.vec1.setX(v.x / this.bodyScaleX);
		PhysicsBody.vec1.setY(v.y / this.bodyScaleY);
		PhysicsBody.vec1.setZ(v.z / this.bodyScaleZ);
		this.btshape.setLocalScaling(PhysicsBody.vec1);
		let worldDyn: Ammo.btDynamicsWorld = PhysicsWorld.active.world;
		let worldCol: Ammo.btCollisionWorld = worldDyn;
		worldCol.updateSingleAabb(this.body);
	}

	syncTransform = () => {
		let t = this.object.transform;
		t.buildMatrix();
		PhysicsBody.vec1.setValue(t.worldx(), t.worldy(), t.worldz());
		PhysicsBody.trans1.setOrigin(PhysicsBody.vec1);
		PhysicsBody.quat.fromMat(t.world);
		PhysicsBody.quat1.setValue(PhysicsBody.quat.x, PhysicsBody.quat.y, PhysicsBody.quat.z, PhysicsBody.quat.w);
		PhysicsBody.trans1.setRotation(PhysicsBody.quat1);
		this.body.setWorldTransform(PhysicsBody.trans1);
		if (this.currentScaleX != t.scale.x || this.currentScaleY != t.scale.y || this.currentScaleZ != t.scale.z) this.setScale(t.scale);
		this.activate();
	}

	setCcd = (sphereRadius: f32, motionThreshold = 1e-7) => {
		this.body.setCcdSweptSphereRadius(sphereRadius);
		this.body.setCcdMotionThreshold(motionThreshold);
	}

	fillConvexHull = (scale: Vec4, margin: f32): Ammo.btConvexHullShape => {
		// Check whether shape already exists
		let data = (this.object as MeshObject).data;
		let shape = PhysicsBody.convexHullCache.get(data);
		if (shape != null) {
			PhysicsBody.usersCache.set(data, PhysicsBody.usersCache.get(data) + 1);
			return shape;
		}

		shape = new Ammo.btConvexHullShape();
		PhysicsBody.convexHullCache.set(data, shape);
		PhysicsBody.usersCache.set(data, 1);

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
			PhysicsBody.vec1.setX(positions[i * 4    ] * sx);
			PhysicsBody.vec1.setY(positions[i * 4 + 1] * sy);
			PhysicsBody.vec1.setZ(positions[i * 4 + 2] * sz);
			shape.addPoint(PhysicsBody.vec1, true);
		}
		return shape;
	}

	fillTriangleMesh = (scale: Vec4): Ammo.btTriangleMesh => {
		// Check whether shape already exists
		let data = (this.object as MeshObject).data;
		let triangleMesh = PhysicsBody.triangleMeshCache.get(data);
		if (triangleMesh != null) {
			PhysicsBody.usersCache.set(data, PhysicsBody.usersCache.get(data) + 1);
			return triangleMesh;
		}

		triangleMesh = new Ammo.btTriangleMesh(true, true);
		PhysicsBody.triangleMeshCache.set(data, triangleMesh);
		PhysicsBody.usersCache.set(data, 1);

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

	delete = () => {
		Ammo.destroy(this.motionState);
		Ammo.destroy(this.body);

		// Delete shape if no other user is found
		if (this.shape == ShapeType.ShapeConvexHull || this.shape == ShapeType.ShapeMesh) {
			let data = (this.object as MeshObject).data;
			let i = PhysicsBody.usersCache.get(data) - 1;
			PhysicsBody.usersCache.set(data, i);
			if (i <= 0) {
				Ammo.destroy(this.btshape);
				this.shape == ShapeType.ShapeConvexHull ?
					PhysicsBody.convexHullCache.delete(data) :
					PhysicsBody.triangleMeshCache.delete(data);
			}
		}
		else Ammo.destroy(this.btshape);
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
