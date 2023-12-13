package arm;

#if arm_physics

import iron.Scene;
import iron.System;
import iron.Time;
import iron.Vec4;
import iron.RayCaster;

class PhysicsWorld {

	public static var active: PhysicsWorld = null;
	static var vec1: PhysicsBullet.Vector3 = null;
	static var vec2: PhysicsBullet.Vector3 = null;
	static var v1 = new Vec4();
	static var v2 = new Vec4();

	public var world: PhysicsBullet.DiscreteDynamicsWorld;
	public var dispatcher: PhysicsBullet.CollisionDispatcher;
	var contacts: Array<TPair> = [];
	var bodyMap = new Map<Int, PhysicsBody>();
	var timeScale = 1.0;
	var timeStep = 1 / 60;
	var maxSteps = 1;

	public static function load(done: Void->Void) {
		var b = Krom.loadBlob("data/plugins/ammo.wasm.js");
		var print = function(s: String) { Krom.log(s); };
		js.Syntax.code("(1, eval)({0})", System.bufferToString(b));
		var instantiateWasm = function(imports, successCallback) {
			var wasmbin = Krom.loadBlob("data/plugins/ammo.wasm.wasm");
			var module = new js.lib.webassembly.Module(wasmbin);
			var inst = new js.lib.webassembly.Instance(module, imports);
			successCallback(inst);
			return inst.exports;
		};
		js.Syntax.code("Ammo({print: {0}, instantiateWasm: {1}}).then({2})", print, instantiateWasm, done);
	}

	public function new() {
		active = this;
		vec1 = new PhysicsBullet.Vector3(0, 0, 0);
		vec2 = new PhysicsBullet.Vector3(0, 0, 0);
		init();
	}

	public function reset() {
		for (body in bodyMap) removeBody(body);
	}

	function init() {
		var broadphase = new PhysicsBullet.DbvtBroadphase();
		var collisionConfiguration = new PhysicsBullet.DefaultCollisionConfiguration();
		dispatcher = new PhysicsBullet.CollisionDispatcher(collisionConfiguration);
		var solver = new PhysicsBullet.SequentialImpulseConstraintSolver();
		world = new PhysicsBullet.DiscreteDynamicsWorld(dispatcher, broadphase, solver, collisionConfiguration);
		setGravity(new Vec4(0, 0, -9.81));
	}

	public function setGravity(v: Vec4) {
		vec1.setValue(v.x, v.y, v.z);
		world.setGravity(vec1);
	}

	public function addBody(pb: PhysicsBody) {
		world.addRigidBodyToGroup(pb.body, pb.group, pb.mask);
		bodyMap.set(pb.id, pb);
	}

	public function removeBody(pb: PhysicsBody) {
		if (pb.destroyed) return;
		pb.destroyed = true;
		if (world != null) world.removeRigidBody(pb.body);
		bodyMap.remove(pb.id);
		pb.delete();
	}

	public function getContacts(pb: PhysicsBody): Array<PhysicsBody> {
		if (contacts.length == 0) return null;
		var res: Array<PhysicsBody> = [];
		for (i in 0...contacts.length) {
			var c = contacts[i];
			var pb: PhysicsBody = null;
			if (c.a == untyped pb.body.userIndex) pb = bodyMap.get(c.b);
			else if (c.b == untyped pb.body.userIndex) pb = bodyMap.get(c.a);
			if (pb != null && res.indexOf(pb) == -1) res.push(pb);
		}
		return res;
	}

	public function getContactPairs(pb: PhysicsBody): Array<TPair> {
		if (contacts.length == 0) return null;
		var res: Array<TPair> = [];
		for (i in 0...contacts.length) {
			var c = contacts[i];
			if (c.a == untyped pb.body.userIndex) res.push(c);
			else if (c.b == untyped pb.body.userIndex) res.push(c);
		}
		return res;
	}

	public function lateUpdate() {
		var t = Time.delta * timeScale;
		if (t == 0.0) return; // Simulation paused

		world.stepSimulation(timeStep, maxSteps, t);
		updateContacts();
		for (body in bodyMap) body.physicsUpdate();
	}

	function updateContacts() {
		contacts = [];
		var disp: PhysicsBullet.Dispatcher = dispatcher;
		var numManifolds = disp.getNumManifolds();

		for (i in 0...numManifolds) {
			var contactManifold = disp.getManifoldByIndexInternal(i);
			var body0 = untyped PhysicsBullet.Ammo.btRigidBody.prototype.upcast(contactManifold.getBody0());
			var body1 = untyped PhysicsBullet.Ammo.btRigidBody.prototype.upcast(contactManifold.getBody1());

			var numContacts = contactManifold.getNumContacts();
			var pt: PhysicsBullet.ManifoldPoint = null;
			var posA: PhysicsBullet.Vector3 = null;
			var posB: PhysicsBullet.Vector3 = null;
			var nor: PhysicsBullet.Vector3 = null;
			for (j in 0...numContacts) {
				pt = contactManifold.getContactPoint(j);
				posA = pt.get_m_positionWorldOnA();
				posB = pt.get_m_positionWorldOnB();
				nor = pt.get_m_normalWorldOnB();
				var cp: TPair = {
					a: untyped body0.userIndex,
					b: untyped body1.userIndex,
					posA: new Vec4(posA.x(), posA.y(), posA.z()),
					posB: new Vec4(posB.x(), posB.y(), posB.z()),
					normOnB: new Vec4(nor.x(), nor.y(), nor.z()),
					impulse: pt.getAppliedImpulse(),
					distance: pt.getDistance()
				};
				contacts.push(cp);
			}
		}
	}

	public function pickClosest(inputX: Float, inputY: Float): PhysicsBody {
		var camera = Scene.active.camera;
		var start = new Vec4();
		var end = new Vec4();
		RayCaster.getDirection(start, end, inputX, inputY, camera);
		var hit = rayCast(camera.transform.world.getLoc(), end);
		var body = (hit != null) ? hit.body : null;
		return body;
	}

	public function rayCast(from: Vec4, to: Vec4, group: Int = 0x00000001, mask = 0xffffffff): THit {
		var rayFrom = vec1;
		var rayTo = vec2;
		rayFrom.setValue(from.x, from.y, from.z);
		rayTo.setValue(to.x, to.y, to.z);

		var rayCallback = new PhysicsBullet.ClosestRayResultCallback(rayFrom, rayTo);

		rayCallback.set_m_collisionFilterGroup(group);
		rayCallback.set_m_collisionFilterMask(mask);

		var worldDyn: PhysicsBullet.DynamicsWorld = world;
		var worldCol: PhysicsBullet.CollisionWorld = worldDyn;
		worldCol.rayTest(rayFrom, rayTo, rayCallback);
		var pb: PhysicsBody = null;
		var hitInfo: THit = null;

		var rc: PhysicsBullet.RayResultCallback = rayCallback;
		if (rc.hasHit()) {
			var co = rayCallback.get_m_collisionObject();
			var body = untyped PhysicsBullet.Ammo.btRigidBody.prototype.upcast(co);
			var hit = rayCallback.get_m_hitPointWorld();
			v1.set(hit.x(), hit.y(), hit.z());
			var norm = rayCallback.get_m_hitNormalWorld();
			v2.set(norm.x(), norm.y(), norm.z());
			pb = bodyMap.get(untyped body.userIndex);
			hitInfo = {
				body: pb,
				pos: v1,
				normal: v2
			};
		}

		PhysicsBullet.Ammo.destroy(rayCallback);
		return hitInfo;
	}
}

typedef THit = {
	public var body: PhysicsBody;
	public var pos: Vec4;
	public var normal: Vec4;
}

typedef TPair = {
	public var a: Int;
	public var b: Int;
	public var posA: Vec4;
	public var posB: Vec4;
	public var normOnB: Vec4;
	public var impulse: Float;
	public var distance: Float;
}

#end
