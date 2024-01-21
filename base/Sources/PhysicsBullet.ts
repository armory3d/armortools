
///if arm_physics

declare function Ammo<T>(ops?: T): Promise<T & typeof Ammo>;

declare namespace Ammo {

	class btTypedObject {}

	class btVector3 {
		constructor(x: f32, y: f32, z: f32);
		setValue(x: f32, y: f32, z: f32): void;
		setX(x: f32): void;
		setY(y: f32): void;
		setZ(z: f32): void;
		x(): f32;
		y(): f32;
		z(): f32;
		length(): f32;
		normalize(): void;
	}

	class btQuadWord {
		setX(x: f32): void;
		setY(y: f32): void;
		setZ(z: f32): void;
		setW(w: f32): void;
		x(): f32;
		y(): f32;
		z(): f32;
		w(): f32;
	}

	class btQuaternion extends btQuadWord {
		constructor(x: f32, y: f32, z: f32, w: f32);
		setEuler(yaw: f32, pitch: f32, roll: f32): void;
		slerp(q: btQuaternion, t: f32): btQuaternion;
		setValue(x: f32, y: f32, z: f32, w: f32): void;
	}

	class btMatrix3x3 {
		setEulerZYX(ex: f32, ey: f32, ez: f32): void;
		getRotation(q: btQuaternion): void;
		getRow(y: i32): btVector3;
	}

	class btActionInterface {}

	class btTransform {
		constructor();
		setIdentity(): void;
		setOrigin(inVec: btVector3): void;
		getOrigin(): btVector3;
		setRotation(inQuat: btQuaternion): void;
		getRotation(): btQuaternion;
	}

	class btIDebugDraw  {
		drawLine(from: btVector3, to: btVector3, color: btVector3): void;
		drawContactPoint(pointOnB: btVector3, normalOnB: btVector3, distance: f32, lifeTime: i32, color: btVector3): void;
		reportErrorWarning(warningString: string): void;
		draw3dText(location: btVector3, textString: string): void;
		setDebugMode(debugMode: i32): void;
		getDebugMode(): i32;
	}

	class btMotionState {
		getWorldTransform(centerOfMassWorldTrans: btTransform): void;
		setWorldTransform(centerOfMassWorldTrans: btTransform): void;
	}

	class btDefaultMotionState extends btMotionState {
		constructor(worldTrans: btTransform, centerOfMassOffset: btTransform);
	}

	class btRigidBodyConstructionInfo {
		constructor(mass: f32, motionState: btMotionState, collisionShape: btCollisionShape, localInertia: btVector3);
		m_friction: f32;
		m_rollingFriction: f32;
	}

	class btCollisionObject {
		getWorldTransform(): btTransform;
		setWorldTransform(trans: btTransform): void;
		activate(forceActivation: bool): void;
		setActivationState(newState: i32): void;
		getUserIndex(): i32;
		setUserIndex(index: i32): void;
		getUserPointer(): any;
		setUserPointer(userPointer: any): void;
		isActive(): bool;
		isKinematicObject(): bool;
		isStaticObject(): bool;
		isStaticOrKinematicObject(): bool;
		setFriction(frict: f32): void;
		setRollingFriction(frict: f32): void;
		setRestitution(rest: f32): void;
		setContactProcessingThreshold(contactProcessingThreshold: f32): void;
		setCollisionShape(collisionShape: btCollisionShape): void;
		getCollisionShape(): btCollisionShape;
		setCollisionFlags(flags: i32): void;
		getCollisionFlags(): i32;
		setCcdSweptSphereRadius(radius: f32): void;
		setCcdMotionThreshold(ccdMotionThreshold: f32): void;
	}

	class btRigidBody extends btCollisionObject {
		userIndex: i32;
		constructor(constructionInfo: btRigidBodyConstructionInfo);
		setMassProps(mass: f32, inertia: btVector3): void;
		getMotionState(): btMotionState;
		applyCentralForce(force: btVector3): void;
		applyForce(force: btVector3, rel_pos: btVector3): void;
		applyCentralImpulse(impulse: btVector3): void;
		applyImpulse(impulse: btVector3, rel_pos: btVector3): void;
		applyTorque(torque: btVector3): void;
		applyTorqueImpulse(torque: btVector3): void;
		clearForces(): void;
		setDamping(linear: f32, angular: f32): void;
		updateInertiaTensor(): void;
		getCenterOfMassPosition(): btVector3;
		getCenterOfMassTransform(): btTransform;
		setCenterOfMassTransform(trans: btTransform): void;
		getLinearVelocity(): btVector3;
		setLinearVelocity(lin_vel: btVector3): void;
		getAngularVelocity(): btVector3;
		setAngularVelocity(ang_vel: btVector3): void;
		setLinearFactor(linearFactor: btVector3): void;
		setAngularFactor(angFac: btVector3): void;
		setSleepingThresholds(linear: f32, angular: f32): void;
		applyGravity(): void;
		getGravity(): btVector3;
		setGravity(acceleration: btVector3): void;
		upcast(col_obj: btCollisionObject): btRigidBody;
	}

	class btCollisionConfiguration {}

	class btDefaultCollisionConfiguration extends btCollisionConfiguration {
		constructor();
	}

	class btSoftBodyRigidBodyCollisionConfiguration extends btCollisionConfiguration {
		constructor();
	}

	class btDispatcher {
		getManifoldByIndexInternal(index: i32): btPersistentManifold;
		getNumManifolds(): i32;
	}

	class btCollisionDispatcher extends btDispatcher {
		constructor(collisionConfiguration: btCollisionConfiguration);
	}

	class btBroadphaseInterface {}

	class btDbvtBroadphase extends btBroadphaseInterface {
		constructor();
	}

	class btAxisSweep3 extends btBroadphaseInterface {
		constructor(worldAabbMin: btVector3, worldAabbMax: btVector3);
	}

	class btConstraintSolver {}

	class btSequentialImpulseConstraintSolver extends btConstraintSolver {
		constructor();
	}

	class btDefaultSoftBodySolver extends btConstraintSolver {
		constructor();
	}

	class RayResultCallback {
		hasHit(): bool;
		constructor();
		get_m_collisionFilterGroup(): i32;
		set_m_collisionFilterGroup(g: i32): void;
		get_m_collisionFilterMask(): i32;
		set_m_collisionFilterMask(m: i32): void;
		get_m_collisionObject(): btCollisionObject;
	}

	class ClosestRayResultCallback extends RayResultCallback {
		constructor(rayFromWorld: btVector3, rayToWorld: btVector3);
		get_m_hitNormalWorld(): btVector3;
		get_m_hitPointWorld(): btVector3;
	}

	class ConcreteContactResultCallback extends RayResultCallback {
		constructor();
	}

	class btCollisionWorld {
		rayTest(rayFromWorld: btVector3, rayToWorld: btVector3, resultCallback: RayResultCallback): void;
		updateSingleAabb(colObj: btCollisionObject): void;
		getPairCache(): btOverlappingPairCache;
		removeCollisionObject(collisionObject: btCollisionObject): void;
		addCollisionObject(collisionObject: btCollisionObject, collisionFilterGroup?: i32, collisionFilterMask?: i32): void;
	}

	class btDynamicsWorld extends btCollisionWorld {
		addRigidBody(body: btRigidBody, group?: i32, mask?: i32): void;
		removeRigidBody(body: btRigidBody): void;
		addAction(action: btActionInterface): void;
		removeAction(action: btActionInterface): void;
		addConstraint(constraint: btTypedConstraint, disableCollisionsBetweenLinkedBodies: bool): void;
		removeConstraint(constraint: btTypedConstraint): void;
		getGravity(): btVector3;
		setGravity(v: btVector3): void;
		stepSimulation(timeStep: f32, maxSubSteps: f32, fixedTimeStep: f32): void;
	}

	class btDiscreteDynamicsWorld extends btDynamicsWorld {
		constructor(dispatcher: btDispatcher, pairCache: btBroadphaseInterface, constraintSolver: btConstraintSolver, collisionConfiguration: btCollisionConfiguration);
		debugDrawWorld(): void;
	}

	class btSoftBodyWorldInfo {
		set_m_gravity(v: btVector3): void;
	}

	class btSoftRigidDynamicsWorld extends btDynamicsWorld {
		constructor(dispatcher: btDispatcher, pairCache: btBroadphaseInterface, constraintSolver: btConstraintSolver, collisionConfiguration: btCollisionConfiguration, softConstraintSolver: btConstraintSolver);
		addSoftBody(body: btSoftBody, collisionFilterGroup: i32, collisionFilterMask: i32): void;
		removeSoftBody(body: btSoftBody): void;
		getWorldInfo(): btSoftBodyWorldInfo;
	}

	class btSimpleDynamicsWorld extends btDynamicsWorld {
		constructor(dispatcher: btDispatcher, pairCache: btBroadphaseInterface, constraintSolver: btConstraintSolver, collisionConfiguration: btCollisionConfiguration);
	}

	class btCollisionShape {
		calculateLocalInertia(mass: f32, inertia: btVector3): void;
		setMargin(margin: f32): void;
		setLocalScaling(scaling: btVector3): void;
		getLocalScaling(): btVector3;
	}

	class btCompoundShape extends btCollisionShape {
		constructor(enableDynamicAabbTree: bool);
		addChildShape(localTransform: btTransform, shape: btCollisionShape): void;
	}

	class btConvexShape extends btCollisionShape {}

	class btConcaveShape extends btCollisionShape {}

	class btConvexInternalShape extends btConvexShape {}

	class btPolyhedralConvexShape extends btConvexInternalShape {}

	class btBoxShape extends btPolyhedralConvexShape {
		constructor(boxHalfExtents: btVector3);
	}

	class btSphereShape extends btConvexInternalShape {
		constructor(radius: f32);
	}

	class btStaticPlaneShape extends btConcaveShape {
		constructor(planeNormal: btVector3, planeConstant: f32);
	}

	class btPolyhedralConvexAabbCachingShape extends btPolyhedralConvexShape {}

	class btConvexHullShape extends btPolyhedralConvexAabbCachingShape {
		constructor();
		addPoint(point: btVector3, recalculateLocalAabb: bool): void;
	}

	class btCapsuleShape extends btConvexInternalShape {
		constructor(radius: f32, height: f32);
		getUpAxis(): i32;
		getRadius(): f32;
		getHalfHeight(): f32;
	}

	class btCapsuleShapeX extends btCapsuleShape {
		constructor(radius: f32, height: f32);
	}

	class btCapsuleShapeZ extends btCapsuleShape {
		constructor(radius: f32, height: f32);
	}

	class btCylinderShape extends btConvexInternalShape {
		constructor(halfExtents: btVector3);
	}

	class btCylinderShapeX extends btCylinderShape {
		constructor(halfExtents: btVector3);
	}

	class btCylinderShapeZ extends btCylinderShape {
		constructor(halfExtents: btVector3);
	}

	class btConeShape extends btConvexInternalShape {
		constructor(radius: f32, height: f32);
	}

	class btConeShapeX extends btConeShape {
		constructor(radius: f32, height: f32);
	}

	class btConeShapeZ extends btConeShape {
		constructor(radius: f32, height: f32);
	}

	class btHeightfieldTerrainShape extends btConcaveShape {
		// heightDataType - float, double, integer, short, fixedpoint88, uchar
		constructor(heightStickWidth: i32, heightStickLength: i32, heightfieldData: any, heightScale: f32, minHeight: f32, maxHeight: f32, upAxis: i32, heightDataType: i32, flipQuadEdges: bool);
	}

	class btStridingMeshInterface {
		constructor();
	}

	class btIndexedMesh {
		constructor();
	}

	class btTriangleIndexVertexArray extends btStridingMeshInterface {
		constructor();
	}

	class btTriangleMesh extends btTriangleIndexVertexArray {
		constructor(use32bitIndices: bool, use4componentVertices: bool);
		addTriangle(vertex0: btVector3, vertex1: btVector3, vertex2: btVector3, removeDuplicateVertices?: bool): void;
	}

	class btTriangleMeshShape extends btConcaveShape {
		constructor(meshInterface: btStridingMeshInterface);
	}

	class btBvhTriangleMeshShape extends btTriangleMeshShape {
		constructor(meshInterface: btStridingMeshInterface, useQuantizedAabbCompression: bool, buildBvh: bool);
	}

	class btGImpactMeshShape extends btConcaveShape {
		constructor(meshInterface: btStridingMeshInterface);
		updateBound(): void;
	}

	class GImpactCollisionAlgorithm {
		constructor();
		registerAlgorithm(dispatcher: btCollisionDispatcher): void;
	}

	class btVehicleTuning extends btActionInterface {
		constructor();
	}

	class btVehicleRaycaster {}

	class btDefaultVehicleRaycaster extends btVehicleRaycaster {
		constructor(world: btDynamicsWorld);
	}

	class btRaycastInfo {
		m_contactNormalWS: btVector3;
		m_contactPointWS: btVector3;
		m_suspensionLength: f32;
		m_hardPointWS: btVector3;
		m_wheelDirectionWS: btVector3;
		m_wheelAxleWS: btVector3;
		m_isInContact: bool;
	}

	class btWheelInfoConstructionInfo {
		constructor();
		m_chassisConnectionCS: btVector3;
		m_wheelDirectionCS: btVector3;
		m_wheelAxleCS: btVector3;
		m_suspensionRestLength: f32;
		m_maxSuspensionTravelCm: f32;
		m_wheelRadius: f32;
		m_suspensionStiffness: f32;
		m_wheelsDampingCompression: f32;
		m_wheelsDampingRelaxation: f32;
		m_frictionSlip: f32;
		m_maxSuspensionForce: f32;
		m_bIsFrontWheel: bool;
	}

	class btWheelInfo {
		constructor(ci: btWheelInfoConstructionInfo);
		m_raycastInfo: btRaycastInfo;
		m_worldTransform: btTransform;
		m_chassisConnectionPointCS: btVector3;
		m_wheelDirectionCS: btVector3;
		m_wheelAxleCS: btVector3;
		m_suspensionRestLength1: f32;
		m_maxSuspensionTravelCm: f32;
		getSuspensionRestLength(): f32;
		m_suspensionStiffness: f32;
		m_wheelsDampingCompression: f32;
		m_wheelsDampingRelaxation: f32;
		m_frictionSlip: f32;
		m_steering: f32;
		m_rotation: f32;
		m_deltaRotation: f32;
		m_rollInfluence: f32;
		m_maxSuspensionForce: f32;
		m_wheelsRadius: f32;
		m_engineForce: f32;
		m_brake: f32;
		m_bIsFrontWheel: bool;
		m_clippedInvContactDotSuspension: f32;
		m_suspensionRelativeVelocity: f32;
		m_wheelsSuspensionForce: f32;
		m_skidInfo: f32;
	}

	class btRaycastVehicle extends btActionInterface {
		constructor(tuning: btVehicleTuning, chassis: btRigidBody, raycaster: btVehicleRaycaster);
		setCoordinateSystem(rightIndex: i32, upIndex: i32, forwardIndex: i32): void;
		addWheel(connectionPointCS0: btVector3, wheelDirectionCS0: btVector3, wheelAxleCS: btVector3, suspensionRestLength: f32, wheelRadius: f32, tuning: btVehicleTuning, isFrontWheel: bool): btWheelInfo;
		getNumWheels(): i32;
		getWheelInfo(index: i32): btWheelInfo;
		resetSuspension(): void;
		getWheelTransformWS(wheelIndex: i32): btTransform;
		updateWheelTransform(wheelIndex: i32, interpolatedTransform: bool): void;
		applyEngineForce(force: f32, wheel: i32): void;
		setBrake(brake: f32, wheelIndex: i32): void;
		setPitchControl(pitch: f32): void;
		updateSuspension(deltaTime: f32 ): void;
		updateFriction(deltaTime: f32 ): void;
		setSteeringValue(steering: f32, wheel: i32): void;
		getRightAxis(): i32;
		getUpAxis(): i32;
		getForwardAxis(): i32;
		getForwardVector(): btVector3;
		getCurrentSpeedKmHour(): f32;
	}

	class btPersistentManifold extends btTypedObject {
		constructor();
		getBody0(): btCollisionObject;
		getBody1(): btCollisionObject;
		getNumContacts(): i32;
		getContactPoint(index: i32): btManifoldPoint;
	}

	class btManifoldPoint {
		constructor();
		get_m_positionWorldOnA(): btVector3;
		get_m_positionWorldOnB(): btVector3;
		get_m_normalWorldOnB(): btVector3;
		getDistance(): f32;
		getAppliedImpulse(): f32;
	}

	class btTypedConstraint extends btTypedObject {
		setBreakingImpulseThreshold(threshold: f32): void;
	}

	class btGeneric6DofConstraint extends btTypedConstraint {
		constructor(rbB: btRigidBody, frameInB: btTransform, useLinearReferenceFrameB: bool);
		// static new2(rbA: btRigidBody, rbB: btRigidBody, frameInA: btTransform, frameInB: btTransform, useLinearReferenceFrameB: bool): Generic6DofConstraint {
		// 	let _r1 = rbA, _r2 = rbB, _fa = frameInA, _fb = frameInB, _b = useLinearReferenceFrameB;
		// 	return new Ammo.btGeneric6DofConstraint(_r1, _r2, _fa, _fb, _b);
		// }
		setLinearLowerLimit(linearLower: btVector3): void;
		setLinearUpperLimit(linearUpper: btVector3): void;
		setAngularLowerLimit(angularLower: btVector3): void;
		setAngularUpperLimit(angularUpper: btVector3): void;
		setParam(num: i32, value: f32, axis: i32): void;
		getFrameOffsetA(): btTransform;
	}

	class btGeneric6DofSpringConstraint extends btGeneric6DofConstraint {
		constructor(rbA: btRigidBody, rbB: btRigidBody, frameInA: btTransform, frameInB: btTransform, useLinearReferenceFrameB: bool);
		enableSpring(index: i32, onOff: bool): void;
		setStiffness(index: i32, stiffness: f32): void;
		setDamping(index: i32, damping: f32): void;
	}

	class btHingeConstraint extends btTypedConstraint {
		constructor(rbA: btRigidBody, rbB: btRigidBody, pivotInA: btVector3, pivotInB: btVector3, axisInA: btVector3, axisInB: btVector3, useReferenceFrameA: bool);
		setLimit(low: f32, high: f32, _softness: f32, _biasFactor: f32, _relaxationFactor: f32): void;
	}

	class btSliderConstraint extends btTypedConstraint {
		constructor(rbA: btRigidBody, rbB: btRigidBody, frameInA: btTransform, frameInB: btTransform, useReferenceFrameA: bool);
		setLowerLinLimit(lowerLimit: f32): void;
		setUpperLinLimit(upperLimit: f32): void;
		setLowerAngLimit(lowerAngLimit: f32): void;
		setUpperAngLimit(upperAngLimit: f32): void;
	}

	class btPoint2PointConstraint extends btTypedConstraint {
		constructor(rbA: btRigidBody, rbB: btRigidBody, pivotInA: btVector3, pivotInB: btVector3);
		setPivotA(pivotA: btVector3): void;
		setPivotB(pivotB: btVector3): void;
		getPivotInA(): btVector3;
		getPivotInB(): btVector3;
	}

	class Config {
		set_viterations(i: i32): void;
		set_piterations(i: i32): void;
		set_collisions(i: i32): void;
		set_kDF(f: f32): void;
		set_kDP(f: f32): void;
		set_kPR(f: f32): void;
		set_kVC(f: f32): void;
		set_kAHR(f: f32): void;
	}

	class tNodeArray {
		at(i: i32): Node;
		size(): i32;
	}

	class tMaterialArray {
		at(i: i32): Material;
	}

	class tAnchorArray {
		constructor();
		at(i: i32): Anchor;
		clear(): void;
		size(): i32;
		push_back(anc: Anchor): void;
		pop_back(): void;
	}

	class Node {
		get_m_x(): btVector3;
		get_m_n(): btVector3;
	}

	class Material {
		constructor();
		set_m_kLST(kAST: f32): void;
		get_m_kLST(): void;
		set_m_kAST(kAST: f32): void;
		get_m_kAST(): void;
		set_m_kVST(kVST: f32): void;
		get_m_kVST(): f32;
		set_m_flags(flags: i32): void;
		get_m_flags(): i32;
	}

	class Anchor {
		set_m_node(node: Node): void;
		get_m_node(): Node;
		set_m_local(local: btVector3): void;
		get_m_local(): btVector3;
		set_m_body(body: btRigidBody): void;
		get_m_body(): btRigidBody;
		set_m_influence(influence: f32): void;
		get_m_influence(): f32;
		set_m_c1(c1: btVector3): void;
		get_m_c1(): btVector3;
		set_m_c2(c2: f32): void;
		get_m_c2(): f32;
	}

	class btSoftBody extends btCollisionObject {
		get_m_nodes(): tNodeArray;
		get_m_cfg(): Config;
		get_m_materials(): tMaterialArray;
		get_m_anchors(): tAnchorArray;
		setTotalMass(mass: f32, fromfaces: bool): void;
		generateClusters(k: i32, maxiterations: i32): void;
		generateBendingConstraints(distance: i32, mat: any): void;
		appendAnchor(node: i32, body: btRigidBody, disableCollisionBetweenLinkedBodies: bool, influence: f32): void;
		appendLink(node: Node, node1: Node, mat: Material, bcheckexist: bool): void;
		addForce(f: btVector3, node: i32): void;
	}

	class btSoftBodyHelpers {
		constructor();
		CreateFromTriMesh(worldInfo: btSoftBodyWorldInfo, vertices:f32[], triangles:i32[], ntriangles: i32, randomizeConstraints: bool): btSoftBody;
	}

	class btOverlappingPairCallback {}

	class btGhostPairCallback extends btOverlappingPairCallback {
		constructor();
	}

	class btOverlappingPairCache {
		setInternalGhostPairCallback(ghostPairCallback: btOverlappingPairCallback): void;
	}

	class btGhostObject extends btCollisionObject {
		constructor();
		getNumOverlappingObjects(): i32;
		getOverlappingObject(index: i32): btCollisionObject;
	}

	class btPairCachingGhostObject extends btGhostObject {
		constructor();
	}

	class btKinematicCharacterController extends btActionInterface {
		constructor(ghostObject: btPairCachingGhostObject, convexShape: btConvexShape, stepHeight: f32, upAxis: i32);
		setUpAxis(axis: i32): void; // setUp in cpp
		jump(): void;
		setGravity(gravity: f32): void;
		getGravity(): f32;
		canJump(): bool;
		onGround(): bool;
		setWalkDirection(walkDirection: btVector3): void;
		setVelocityForTimeInterval(velocity: btVector3, timeInterval: f32): void;
		warp(origin: btVector3): void;
		preStep(collisionWorld: btCollisionWorld): void;
		playerStep(collisionWorld: btCollisionWorld, dt: f32): void;
		setFallSpeed(fallSpeed: f32): void;
		setJumpSpeed(jumpSpeed: f32): void;
		setMaxJumpHeight(maxJumpHeight: f32): void;
		setMaxSlope(slopeRadians: f32): void;
		getMaxSlope(): f32;
		getGhostObject(): btPairCachingGhostObject;
		setUseGhostSweepTest(useGhostObjectSweepTest: bool): void;
		setUpInterpolate(value: bool): void;
	}

	function destroy(obj: any): void;
	function castObject(obj: any, to: any): any;
	function _malloc(byteSize: i32): i32;
	let HEAPU8: any;
	let HEAPF32: any;
}

///end
