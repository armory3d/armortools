package arm.plugin;

#if arm_physics

import iron.math.Vec4;
import iron.math.Quat;
import iron.object.Transform;
import iron.object.MeshObject;
import iron.data.MeshData;

@:access(arm.plugin.PhysicsWorld)
class PhysicsBody extends iron.Trait {

	@:keep
	public var props = ["mass"];

	public var mass(default, set) = 0.0;

	@:keep
	function set_mass(f: Float): Float {
		if (ready) {
			remove();
			var t = new PhysicsBody();
			t.mass = f;
			object.addTrait(t);
		}
		else mass = f;
		return f;
	}

	public var friction = 0.5;
	public var restitution = 0.0;
	public var collisionMargin = 0.0;
	public var linearDamping = 0.04;
	public var angularDamping = 0.1;
	public var linearFactors = [1.0, 1.0, 1.0];
	public var angularFactors = [1.0, 1.0, 1.0];
	public var linearThreshold = 0.0;
	public var angularThreshold = 0.0;
	public var ccd = false; // Continuous collision detection
	public var trigger = false;
	public var group = 1;
	public var mask = 1;
	public var shape = ShapeBox;
	var destroyed = false;
	var bodyScaleX: Float; // Transform scale at creation time
	var bodyScaleY: Float;
	var bodyScaleZ: Float;
	var currentScaleX: Float;
	var currentScaleY: Float;
	var currentScaleZ: Float;

	var body: Bt.RigidBody = null;
	var motionState: Bt.MotionState;
	var btshape: Bt.CollisionShape;
	var ready = false;
	public var id = 0;
	public var heightData: haxe.io.Bytes = null;

	static var nextId = 0;
	static var ammoArray: Int = -1;
	static var gimpactRegistered = false;
	static var first = true;
	static var vec1: Bt.Vector3;
	static var vec2: Bt.Vector3;
	static var vec3: Bt.Vector3;
	static var quat1: Bt.Quaternion;
	static var trans1: Bt.Transform;
	static var trans2: Bt.Transform;
	static var quat = new Quat();

	static var convexHullCache = new Map<MeshData, Bt.ConvexHullShape>();
	static var triangleMeshCache = new Map<MeshData, Bt.TriangleMesh>();
	static var usersCache = new Map<MeshData, Int>();

	public function new() {
		super();

		if (first) {
			first = false;
			vec1 = new Bt.Vector3(0, 0, 0);
			vec2 = new Bt.Vector3(0, 0, 0);
			vec3 = new Bt.Vector3(0, 0, 0);
			quat1 = new Bt.Quaternion(0, 0, 0, 0);
			trans1 = new Bt.Transform();
			trans2 = new Bt.Transform();
		}

		notifyOnAdd(init);
	}

	inline function withMargin(f: Float) {
		return f - f * collisionMargin;
	}

	function init() {
		if (ready) return;
		ready = true;

		if (!Std.isOfType(object, MeshObject)) return; // No mesh data
		var transform = object.transform;
		var physics = PhysicsWorld.active;

		if (shape == ShapeBox) {
			vec1.setX(withMargin(transform.dim.x / 2));
			vec1.setY(withMargin(transform.dim.y / 2));
			vec1.setZ(withMargin(transform.dim.z / 2));
			btshape = new Bt.BoxShape(vec1);
		}
		else if (shape == ShapeSphere) {
			btshape = new Bt.SphereShape(withMargin(transform.dim.x / 2));
		}
		else if (shape == ShapeConvexHull) {
			var shapeConvex = fillConvexHull(transform.scale, collisionMargin);
			btshape = shapeConvex;
		}
		else if (shape == ShapeCone) {
			var coneZ = new Bt.ConeShapeZ(
				withMargin(transform.dim.x / 2), // Radius
				withMargin(transform.dim.z));	 // Height
			var cone: Bt.ConeShape = coneZ;
			btshape = cone;
		}
		else if (shape == ShapeCylinder) {
			vec1.setX(withMargin(transform.dim.x / 2));
			vec1.setY(withMargin(transform.dim.y / 2));
			vec1.setZ(withMargin(transform.dim.z / 2));
			var cylZ = new Bt.CylinderShapeZ(vec1);
			var cyl: Bt.CylinderShape = cylZ;
			btshape = cyl;
		}
		else if (shape == ShapeCapsule) {
			var r = transform.dim.x / 2;
			var capsZ = new Bt.CapsuleShapeZ(
				withMargin(r), // Radius
				withMargin(transform.dim.z - r * 2)); // Distance between 2 sphere centers
			var caps: Bt.CapsuleShape = capsZ;
			btshape = caps;
		}
		else if (shape == ShapeMesh) {
			var meshInterface = fillTriangleMesh(transform.scale);
			if (mass > 0) {
				var shapeGImpact = new Bt.GImpactMeshShape(meshInterface);
				shapeGImpact.updateBound();
				var shapeConcave: Bt.ConcaveShape = shapeGImpact;
				btshape = shapeConcave;
				if (!gimpactRegistered) {
					gimpactRegistered = true;
					new Bt.GImpactCollisionAlgorithm().registerAlgorithm(physics.dispatcher);
				}
			}
			else {
				var shapeBvh = new Bt.BvhTriangleMeshShape(meshInterface, true, true);
				var shapeTri: Bt.TriangleMeshShape = shapeBvh;
				var shapeConcave: Bt.ConcaveShape = shapeTri;
				btshape = shapeConcave;
			}
		}
		else if (shape == ShapeTerrain) {
			var length = heightData.length;
			if (ammoArray == -1) {
				ammoArray = Bt.Ammo._malloc(length);
			}
			// From texture bytes
			for (i in 0...length) {
				Bt.Ammo.HEAPU8[ammoArray + i] = heightData.get(i);
			}
			var slice = Std.int(Math.sqrt(length)); // Assuming square terrain data
			var axis = 2; // z
			var dataType = 5; // u8
			btshape = new Bt.HeightfieldTerrainShape(slice, slice, ammoArray, 1 / 255, 0, 1, axis, dataType, false);
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
		motionState = new Bt.DefaultMotionState(trans1, trans2); // Transform, center of mass offset

		vec1.setX(0);
		vec1.setY(0);
		vec1.setZ(0);
		var inertia = vec1;

		if (mass > 0) btshape.calculateLocalInertia(mass, inertia);
		var bodyCI = new Bt.RigidBodyConstructionInfo(mass, motionState, btshape, inertia);
		body = new Bt.RigidBody(bodyCI);

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
		if (trigger) body.setCollisionFlags(body.getCollisionFlags() | Bt.CollisionObject.CF_NO_CONTACT_RESPONSE);
		if (mass == 0.0) body.setCollisionFlags(body.getCollisionFlags() | Bt.CollisionObject.CF_STATIC_OBJECT);
		if (ccd) setCcd(transform.radius);

		bodyScaleX = currentScaleX = transform.scale.x;
		bodyScaleY = currentScaleY = transform.scale.y;
		bodyScaleZ = currentScaleZ = transform.scale.z;

		id = nextId++;
		untyped body.userIndex = id;

		physics.addBody(this);
		notifyOnRemove(removeFromWorld);

		Bt.Ammo.destroy(bodyCI);
	}

	function physicsUpdate() {
		if (!ready) return;
		var trans = body.getWorldTransform();

		var p = trans.getOrigin();
		var q = trans.getRotation();
		var qw: Bt.QuadWord = q;

		var transform = object.transform;
		transform.loc.set(p.x(), p.y(), p.z());
		transform.rot.set(qw.x(), qw.y(), qw.z(), qw.w());
		if (object.parent != null) {
			var ptransform = object.parent.transform;
			transform.loc.x -= ptransform.worldx();
			transform.loc.y -= ptransform.worldy();
			transform.loc.z -= ptransform.worldz();
		}
		transform.buildMatrix();
	}

	public function removeFromWorld() {
		PhysicsWorld.active.removeBody(this);
	}

	public function activate() {
		body.activate(false);
	}

	public function setGravity(v: Vec4) {
		vec1.setValue(v.x, v.y, v.z);
		body.setGravity(vec1);
	}

	public function applyForce(force: Vec4, loc: Vec4 = null) {
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

	public function applyImpulse(impulse: Vec4, loc: Vec4 = null) {
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

	public function applyTorque(torque: Vec4) {
		activate();
		vec1.setValue(torque.x, torque.y, torque.z);
		body.applyTorque(vec1);
	}

	public function applyTorqueImpulse(torque: Vec4) {
		activate();
		vec1.setValue(torque.x, torque.y, torque.z);
		body.applyTorqueImpulse(vec1);
	}

	public function setLinearFactor(x: Float, y: Float, z: Float) {
		vec1.setValue(x, y, z);
		body.setLinearFactor(vec1);
	}

	public function setAngularFactor(x: Float, y: Float, z: Float) {
		vec1.setValue(x, y, z);
		body.setAngularFactor(vec1);
	}

	public function getLinearVelocity(): Vec4 {
		var v = body.getLinearVelocity();
		return new Vec4(v.x(), v.y(), v.z());
	}

	public function setLinearVelocity(x: Float, y: Float, z: Float) {
		vec1.setValue(x, y, z);
		body.setLinearVelocity(vec1);
	}

	public function getAngularVelocity(): Vec4 {
		var v = body.getAngularVelocity();
		return new Vec4(v.x(), v.y(), v.z());
	}

	public function setAngularVelocity(x: Float, y: Float, z: Float) {
		vec1.setValue(x, y, z);
		body.setAngularVelocity(vec1);
	}

	public function setFriction(f: Float) {
		body.setFriction(f);
		this.friction = f;
	}

	function setScale(v: Vec4) {
		currentScaleX = v.x;
		currentScaleY = v.y;
		currentScaleZ = v.z;
		vec1.setX(v.x / bodyScaleX);
		vec1.setY(v.y / bodyScaleY);
		vec1.setZ(v.z / bodyScaleZ);
		btshape.setLocalScaling(vec1);
		var worldDyn: Bt.DynamicsWorld = PhysicsWorld.active.world;
		var worldCol: Bt.CollisionWorld = worldDyn;
		worldCol.updateSingleAabb(body);
	}

	public function syncTransform() {
		var t = object.transform;
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

	function setCcd(sphereRadius: Float, motionThreshold = 1e-7) {
		body.setCcdSweptSphereRadius(sphereRadius);
		body.setCcdMotionThreshold(motionThreshold);
	}

	function fillConvexHull(scale: Vec4, margin: kha.FastFloat): Bt.ConvexHullShape {
		// Check whether shape already exists
		var data = cast(object, MeshObject).data;
		var shape = convexHullCache.get(data);
		if (shape != null) {
			usersCache.set(data, usersCache.get(data) + 1);
			return shape;
		}

		shape = new Bt.ConvexHullShape();
		convexHullCache.set(data, shape);
		usersCache.set(data, 1);

		var positions = data.geom.positions.values;

		var sx: kha.FastFloat = scale.x * (1.0 - margin) * (1 / 32767);
		var sy: kha.FastFloat = scale.y * (1.0 - margin) * (1 / 32767);
		var sz: kha.FastFloat = scale.z * (1.0 - margin) * (1 / 32767);

		if (data.raw.scale_pos != null) {
			sx *= data.raw.scale_pos;
			sy *= data.raw.scale_pos;
			sz *= data.raw.scale_pos;
		}

		for (i in 0...Std.int(positions.length / 4)) {
			vec1.setX(positions[i * 4    ] * sx);
			vec1.setY(positions[i * 4 + 1] * sy);
			vec1.setZ(positions[i * 4 + 2] * sz);
			shape.addPoint(vec1, true);
		}
		return shape;
	}

	function fillTriangleMesh(scale: Vec4): Bt.TriangleMesh {
		// Check whether shape already exists
		var data = cast(object, MeshObject).data;
		var triangleMesh = triangleMeshCache.get(data);
		if (triangleMesh != null) {
			usersCache.set(data, usersCache.get(data) + 1);
			return triangleMesh;
		}

		triangleMesh = new Bt.TriangleMesh(true, true);
		triangleMeshCache.set(data, triangleMesh);
		usersCache.set(data, 1);

		var positions = data.geom.positions.values;
		var indices = data.geom.indices;

		var sx: kha.FastFloat = scale.x * (1 / 32767);
		var sy: kha.FastFloat = scale.y * (1 / 32767);
		var sz: kha.FastFloat = scale.z * (1 / 32767);

		if (data.raw.scale_pos != null) {
			sx *= data.raw.scale_pos;
			sy *= data.raw.scale_pos;
			sz *= data.raw.scale_pos;
		}

		for (ar in indices) {
			for (i in 0...Std.int(ar.length / 3)) {
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

	public function delete() {
		Bt.Ammo.destroy(motionState);
		Bt.Ammo.destroy(body);

		// Delete shape if no other user is found
		if (shape == ShapeConvexHull || shape == ShapeMesh) {
			var data = cast(object, MeshObject).data;
			var i = usersCache.get(data) - 1;
			usersCache.set(data, i);
			if (i <= 0) {
				Bt.Ammo.destroy(btshape);
				shape == ShapeConvexHull ?
					convexHullCache.remove(data) :
					triangleMeshCache.remove(data);
			}
		}
		else Bt.Ammo.destroy(btshape);
	}
}

@:enum abstract ShapeType(Int) from Int to Int {
	var ShapeBox = 0;
	var ShapeSphere = 1;
	var ShapeConvexHull = 2;
	var ShapeMesh = 3;
	var ShapeCone = 4;
	var ShapeCylinder = 5;
	var ShapeCapsule = 6;
	var ShapeTerrain = 7;
}

#end
