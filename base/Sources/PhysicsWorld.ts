
///if arm_physics

class PhysicsWorldRaw {
	world: Ammo.btDiscreteDynamicsWorld;
	dispatcher: Ammo.btCollisionDispatcher;
	contacts: pair_t[] = [];
	bodyMap: Map<i32, PhysicsBodyRaw> = new Map();
	timeScale: f32 = 1.0;
	timeStep: f32 = 1 / 60;
	maxSteps: i32 = 1;
}

class PhysicsWorld {

	static active: PhysicsWorldRaw = null;
	static vec1: Ammo.btVector3 = null;
	static vec2: Ammo.btVector3 = null;
	static v1: vec4_t = vec4_create();
	static v2: vec4_t = vec4_create();

	static load = (done: ()=>void) => {
		let b: buffer_t = krom_load_blob("data/plugins/ammo.js");
		globalThis.eval(sys_buffer_to_string(b));
		let print = (s: string) => { krom_log(s); };
		Ammo({print: print}).then(done);
	}

	static create(): PhysicsWorldRaw {
		let pw: PhysicsWorldRaw = new PhysicsWorldRaw();
		PhysicsWorld.active = pw;
		PhysicsWorld.vec1 = new Ammo.btVector3(0, 0, 0);
		PhysicsWorld.vec2 = new Ammo.btVector3(0, 0, 0);
		PhysicsWorld.init(pw);
		return pw;
	}

	static reset = (pw: PhysicsWorldRaw) => {
		for (let body of pw.bodyMap.values()) PhysicsWorld.remove_body(pw, body);
	}

	static init = (pw: PhysicsWorldRaw) => {
		let broadphase: Ammo.btDbvtBroadphase = new Ammo.btDbvtBroadphase();
		let collisionConfiguration: Ammo.btDefaultCollisionConfiguration = new Ammo.btDefaultCollisionConfiguration();
		pw.dispatcher = new Ammo.btCollisionDispatcher(collisionConfiguration);
		let solver: Ammo.btSequentialImpulseConstraintSolver = new Ammo.btSequentialImpulseConstraintSolver();
		pw.world = new Ammo.btDiscreteDynamicsWorld(pw.dispatcher, broadphase, solver, collisionConfiguration);
		PhysicsWorld.set_gravity(pw, vec4_create(0, 0, -9.81));
	}

	static set_gravity = (pw: PhysicsWorldRaw, v: vec4_t) => {
		PhysicsWorld.vec1.setValue(v.x, v.y, v.z);
		pw.world.setGravity(PhysicsWorld.vec1);
	}

	static add_body = (pw: PhysicsWorldRaw, pb: PhysicsBodyRaw) => {
		pw.world.addRigidBody(pb.body, pb.group, pb.mask);
		pw.bodyMap.set(pb.id, pb);
	}

	static remove_body = (pw: PhysicsWorldRaw, pb: PhysicsBodyRaw) => {
		if (pb.destroyed) return;
		pb.destroyed = true;
		if (pw.world != null) pw.world.removeRigidBody(pb.body);
		pw.bodyMap.delete(pb.id);
		PhysicsBody.delete(pb);
	}

	static get_contacts = (pw: PhysicsWorldRaw, pb: PhysicsBodyRaw): PhysicsBodyRaw[] => {
		if (pw.contacts.length == 0) return null;
		let res: PhysicsBodyRaw[] = [];
		for (let i: i32 = 0; i < pw.contacts.length; ++i) {
			let c: pair_t = pw.contacts[i];
			let pb: PhysicsBodyRaw = null;
			if (c.a == pb.body.userIndex) pb = pw.bodyMap.get(c.b);
			else if (c.b == pb.body.userIndex) pb = pw.bodyMap.get(c.a);
			if (pb != null && res.indexOf(pb) == -1) res.push(pb);
		}
		return res;
	}

	static get_contact_pairs = (pw: PhysicsWorldRaw, pb: PhysicsBodyRaw): pair_t[] => {
		if (pw.contacts.length == 0) return null;
		let res: pair_t[] = [];
		for (let i: i32 = 0; i < pw.contacts.length; ++i) {
			let c: pair_t = pw.contacts[i];
			if (c.a == pb.body.userIndex) res.push(c);
			else if (c.b == pb.body.userIndex) res.push(c);
		}
		return res;
	}

	static late_update = (pw: PhysicsWorldRaw) => {
		let t: f32 = time_delta() * pw.timeScale;
		if (t == 0.0) return; // Simulation paused

		pw.world.stepSimulation(pw.timeStep, pw.maxSteps, t);
		PhysicsWorld.update_contacts(pw);
		for (let body of pw.bodyMap.values()) PhysicsBody.physics_update(body);
	}

	static update_contacts = (pw: PhysicsWorldRaw) => {
		pw.contacts = [];
		let disp: Ammo.btDispatcher = pw.dispatcher;
		let numManifolds: i32 = disp.getNumManifolds();

		for (let i: i32 = 0; i < numManifolds; ++i) {
			let contactManifold: Ammo.btPersistentManifold = disp.getManifoldByIndexInternal(i);
			let body0: Ammo.btRigidBody = Ammo.btRigidBody.prototype.upcast(contactManifold.getBody0());
			let body1: Ammo.btRigidBody = Ammo.btRigidBody.prototype.upcast(contactManifold.getBody1());

			let numContacts: i32 = contactManifold.getNumContacts();
			let pt: Ammo.btManifoldPoint = null;
			let posA: Ammo.btVector3 = null;
			let posB: Ammo.btVector3 = null;
			let nor: Ammo.btVector3 = null;
			for (let j: i32 = 0; j < numContacts; ++j) {
				pt = contactManifold.getContactPoint(j);
				posA = pt.get_m_positionWorldOnA();
				posB = pt.get_m_positionWorldOnB();
				nor = pt.get_m_normalWorldOnB();
				let cp: pair_t = {
					a: body0.userIndex,
					b: body1.userIndex,
					posA: vec4_create(posA.x(), posA.y(), posA.z()),
					posB: vec4_create(posB.x(), posB.y(), posB.z()),
					normOnB: vec4_create(nor.x(), nor.y(), nor.z()),
					impulse: pt.getAppliedImpulse(),
					distance: pt.getDistance()
				};
				pw.contacts.push(cp);
			}
		}
	}

	static pick_closest = (pw: PhysicsWorldRaw, inputX: f32, inputY: f32): PhysicsBodyRaw => {
		let camera: camera_object_t = scene_camera;
		let start: vec4_t = vec4_create();
		let end: vec4_t = vec4_create();
		raycast_get_dir(start, end, inputX, inputY, camera);
		let hit: hit_t = PhysicsWorld.ray_cast(pw, mat4_get_loc(camera.base.transform.world), end);
		let body: PhysicsBodyRaw = (hit != null) ? hit.body : null;
		return body;
	}

	static ray_cast = (pw: PhysicsWorldRaw, from: vec4_t, to: vec4_t, group: i32 = 0x00000001, mask: i32 = 0xffffffff): hit_t => {
		let rayFrom: Ammo.btVector3 = PhysicsWorld.vec1;
		let rayTo: Ammo.btVector3 = PhysicsWorld.vec2;
		rayFrom.setValue(from.x, from.y, from.z);
		rayTo.setValue(to.x, to.y, to.z);

		let rayCallback: Ammo.ClosestRayResultCallback = new Ammo.ClosestRayResultCallback(rayFrom, rayTo);

		rayCallback.set_m_collisionFilterGroup(group);
		rayCallback.set_m_collisionFilterMask(mask);

		let worldDyn: Ammo.btDynamicsWorld = pw.world;
		let worldCol: Ammo.btCollisionWorld = worldDyn;
		worldCol.rayTest(rayFrom, rayTo, rayCallback);
		let pb: PhysicsBodyRaw = null;
		let hitInfo: hit_t = null;

		let rc: Ammo.RayResultCallback = rayCallback;
		if (rc.hasHit()) {
			let co: Ammo.btCollisionObject = rayCallback.get_m_collisionObject();
			let body: Ammo.btRigidBody = Ammo.btRigidBody.prototype.upcast(co);
			let hit: Ammo.btVector3 = rayCallback.get_m_hitPointWorld();
			vec4_set(PhysicsWorld.v1, hit.x(), hit.y(), hit.z());
			let norm: Ammo.btVector3 = rayCallback.get_m_hitNormalWorld();
			vec4_set(PhysicsWorld.v2, norm.x(), norm.y(), norm.z());
			pb = pw.bodyMap.get(body.userIndex);
			hitInfo = {
				body: pb,
				pos: PhysicsWorld.v1,
				normal: PhysicsWorld.v2
			};
		}

		Ammo.destroy(rayCallback);
		return hitInfo;
	}
}

type hit_t = {
	body?: PhysicsBodyRaw;
	pos?: vec4_t;
	normal?: vec4_t;
};

type pair_t = {
	a?: i32;
	b?: i32;
	posA?: vec4_t;
	posB?: vec4_t;
	normOnB?: vec4_t;
	impulse?: f32;
	distance?: f32;
};

///end
