
///if arm_physics

class PhysicsWorld {

	static active: PhysicsWorld = null;
	static vec1: PhysicsBullet.Vector3 = null;
	static vec2: PhysicsBullet.Vector3 = null;
	static v1 = new Vec4();
	static v2 = new Vec4();

	world: PhysicsBullet.DiscreteDynamicsWorld;
	dispatcher: PhysicsBullet.CollisionDispatcher;
	contacts: TPair[] = [];
	bodyMap = new Map<i32, PhysicsBody>();
	timeScale = 1.0;
	timeStep = 1 / 60;
	maxSteps = 1;

	static load(done: ()=>void) {
		let b = Krom.loadBlob("data/plugins/ammo.wasm.js");
		let print = (s: string) => { Krom.log(s); };
		(1, eval)(System.bufferToString(b));
		let instantiateWasm = (imports, successCallback) => {
			let wasmbin = Krom.loadBlob("data/plugins/ammo.wasm.wasm");
			let module = new webassembly.Module(wasmbin);
			let inst = new webassembly.Instance(module, imports);
			successCallback(inst);
			return inst.exports;
		};
		Ammo({print: print, instantiateWasm: instantiateWasm}).then(done);
	}

	constructor() {
		active = this;
		vec1 = new PhysicsBullet.Vector3(0, 0, 0);
		vec2 = new PhysicsBullet.Vector3(0, 0, 0);
		init();
	}

	reset() {
		for (let body of bodyMap) removeBody(body);
	}

	init() {
		let broadphase = new PhysicsBullet.DbvtBroadphase();
		let collisionConfiguration = new PhysicsBullet.DefaultCollisionConfiguration();
		dispatcher = new PhysicsBullet.CollisionDispatcher(collisionConfiguration);
		let solver = new PhysicsBullet.SequentialImpulseConstraintSolver();
		world = new PhysicsBullet.DiscreteDynamicsWorld(dispatcher, broadphase, solver, collisionConfiguration);
		setGravity(new Vec4(0, 0, -9.81));
	}

	setGravity(v: Vec4) {
		vec1.setValue(v.x, v.y, v.z);
		world.setGravity(vec1);
	}

	addBody(pb: PhysicsBody) {
		world.addRigidBodyToGroup(pb.body, pb.group, pb.mask);
		bodyMap.set(pb.id, pb);
	}

	removeBody(pb: PhysicsBody) {
		if (pb.destroyed) return;
		pb.destroyed = true;
		if (world != null) world.removeRigidBody(pb.body);
		bodyMap.remove(pb.id);
		pb.delete();
	}

	getContacts(pb: PhysicsBody): PhysicsBody[] {
		if (contacts.length == 0) return null;
		let res: PhysicsBody[] = [];
		for (let i = 0; i < contacts.length; ++i) {
			let c = contacts[i];
			let pb: PhysicsBody = null;
			if (c.a == pb.body.userIndex) pb = bodyMap.get(c.b);
			else if (c.b == pb.body.userIndex) pb = bodyMap.get(c.a);
			if (pb != null && res.indexOf(pb) == -1) res.push(pb);
		}
		return res;
	}

	getContactPairs(pb: PhysicsBody): TPair[] {
		if (contacts.length == 0) return null;
		let res: TPair[] = [];
		for (let i = 0; i < contacts.length; ++i) {
			let c = contacts[i];
			if (c.a == pb.body.userIndex) res.push(c);
			else if (c.b == pb.body.userIndex) res.push(c);
		}
		return res;
	}

	lateUpdate() {
		let t = Time.delta * timeScale;
		if (t == 0.0) return; // Simulation paused

		world.stepSimulation(timeStep, maxSteps, t);
		updateContacts();
		for (let body of bodyMap) body.physicsUpdate();
	}

	updateContacts() {
		contacts = [];
		let disp: PhysicsBullet.Dispatcher = dispatcher;
		let numManifolds = disp.getNumManifolds();

		for (let i = 0; i < numManifolds; ++i) {
			let contactManifold = disp.getManifoldByIndexInternal(i);
			let body0 = PhysicsBullet.Ammo.btRigidBody.prototype.upcast(contactManifold.getBody0());
			let body1 = PhysicsBullet.Ammo.btRigidBody.prototype.upcast(contactManifold.getBody1());

			let numContacts = contactManifold.getNumContacts();
			let pt: PhysicsBullet.ManifoldPoint = null;
			let posA: PhysicsBullet.Vector3 = null;
			let posB: PhysicsBullet.Vector3 = null;
			let nor: PhysicsBullet.Vector3 = null;
			for (let j = 0; j < numContacts; ++j) {
				pt = contactManifold.getContactPoint(j);
				posA = pt.get_m_positionWorldOnA();
				posB = pt.get_m_positionWorldOnB();
				nor = pt.get_m_normalWorldOnB();
				let cp: TPair = {
					a: body0.userIndex,
					b: body1.userIndex,
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

	pickClosest(inputX: f32, inputY: f32): PhysicsBody {
		let camera = Scene.active.camera;
		let start = new Vec4();
		let end = new Vec4();
		RayCaster.getDirection(start, end, inputX, inputY, camera);
		let hit = rayCast(camera.transform.world.getLoc(), end);
		let body = (hit != null) ? hit.body : null;
		return body;
	}

	rayCast(from: Vec4, to: Vec4, group: i32 = 0x00000001, mask = 0xffffffff): THit {
		let rayFrom = vec1;
		let rayTo = vec2;
		rayFrom.setValue(from.x, from.y, from.z);
		rayTo.setValue(to.x, to.y, to.z);

		let rayCallback = new PhysicsBullet.ClosestRayResultCallback(rayFrom, rayTo);

		rayCallback.set_m_collisionFilterGroup(group);
		rayCallback.set_m_collisionFilterMask(mask);

		let worldDyn: PhysicsBullet.DynamicsWorld = world;
		let worldCol: PhysicsBullet.CollisionWorld = worldDyn;
		worldCol.rayTest(rayFrom, rayTo, rayCallback);
		let pb: PhysicsBody = null;
		let hitInfo: THit = null;

		let rc: PhysicsBullet.RayResultCallback = rayCallback;
		if (rc.hasHit()) {
			let co = rayCallback.get_m_collisionObject();
			let body = PhysicsBullet.Ammo.btRigidBody.prototype.upcast(co);
			let hit = rayCallback.get_m_hitPointWorld();
			v1.set(hit.x(), hit.y(), hit.z());
			let norm = rayCallback.get_m_hitNormalWorld();
			v2.set(norm.x(), norm.y(), norm.z());
			pb = bodyMap.get(body.userIndex);
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

type THit = {
	body: PhysicsBody;
	pos: Vec4;
	normal: Vec4;
}

type TPair = {
	a: i32;
	b: i32;
	posA: Vec4;
	posB: Vec4;
	normOnB: Vec4;
	impulse: f32;
	distance: f32;
}

///end
