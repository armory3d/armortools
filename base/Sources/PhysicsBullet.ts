
///if arm_physics

// @:native("Ammo.btTypedObject")
declare class TypedObject {}

// @:native("Ammo.btVector3")
declare class Vector3 {
	constructor(x: f32, y: f32, z: f32): void;
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

// @:native("Ammo.btQuadWord")
declare class QuadWord {
	setX(x: f32): void;
	setY(y: f32): void;
	setZ(z: f32): void;
	setW(w: f32): void;
	x(): f32;
	y(): f32;
	z(): f32;
	w(): f32;
}

// @:native("Ammo.btQuaternion")
declare class Quaternion extends QuadWord {
	constructor(x: f32, y: f32, z: f32, w: f32): void;
	setEuler(yaw: f32, pitch: f32, roll: f32): void;
	slerp(q: Quaternion, t: f32): Quaternion;
	setValue(x: f32, y: f32, z: f32, w: f32): void;
}

// @:native("Ammo.btMatrix3x3")
declare class Matrix3x3 {
	setEulerZYX(ex: f32, ey: f32, ez: f32): void;
	getRotation(q: Quaternion): void;
	getRow(y: i32): Vector3;
}

// @:native("Ammo.btActionInterface")
declare class ActionInterface {}

// @:native("Ammo.btTransform")
declare class Transform {
	constructor(): void;
	setIdentity(): void;
	setOrigin(inVec: Vector3): void;
	getOrigin(): Vector3;
	setRotation(inQuat: Quaternion): void;
	getRotation(): Quaternion;
}

// @:native("Ammo.btIDebugDraw")
declare class IDebugDraw  {
	drawLine(from: Vector3, to: Vector3, color: Vector3): void;
	drawContactPoint(pointOnB: Vector3, normalOnB: Vector3, distance: f32, lifeTime: i32, color: Vector3): void;
	reportErrorWarning(warningString: string): void;
	draw3dText(location: Vector3, textString: string): void;
	setDebugMode(debugMode: i32): void;
	getDebugMode(): i32;
}

// @:native("Ammo.btMotionState")
declare class MotionState {
	getWorldTransform(centerOfMassWorldTrans: Transform): void;
	setWorldTransform(centerOfMassWorldTrans: Transform): void;
}

// @:native("Ammo.btDefaultMotionState")
declare class DefaultMotionState extends MotionState {
	constructor(worldTrans: Transform, centerOfMassOffset: Transform): void;
}

// @:native("Ammo.btRigidBodyConstructionInfo")
declare class RigidBodyConstructionInfo {
	constructor(mass: f32, motionState: MotionState, collisionShape: CollisionShape, localInertia: Vector3): void;
	m_friction: f32;
	m_rollingFriction: f32;
}

// @:native("Ammo.btCollisionObject")
declare class CollisionObject {
	static ACTIVE_TAG = 1;
	static ISLAND_SLEEPING = 2;
	static WANTS_DEACTIVATION = 3;
	static DISABLE_DEACTIVATION = 4;
	static DISABLE_SIMULATION = 5;
	static CF_STATIC_OBJECT = 1;
	static CF_KINEMATIC_OBJECT = 2;
	static CF_NO_CONTACT_RESPONSE = 4;
	static CF_CHARACTER_OBJECT = 16;
	getWorldTransform(): Transform;
	setWorldTransform(trans: Transform): void;
	activate(forceActivation: bool = false): void;
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
	setCollisionShape(collisionShape: CollisionShape): void;
	getCollisionShape(): CollisionShape;
	setCollisionFlags(flags: i32): void;
	getCollisionFlags(): i32;
	setCcdSweptSphereRadius(radius: f32): void;
	setCcdMotionThreshold(ccdMotionThreshold: f32): void;
}

// @:native("Ammo.btRigidBody")
declare class RigidBody extends CollisionObject {
	constructor(constructionInfo: RigidBodyConstructionInfo): void;
	setMassProps(mass: f32, inertia: Vector3): void;
	getMotionState(): MotionState;
	applyCentralForce(force: Vector3): void;
	applyForce(force: Vector3, rel_pos: Vector3): void;
	applyCentralImpulse(impulse: Vector3): void;
	applyImpulse(impulse: Vector3, rel_pos: Vector3): void;
	applyTorque(torque: Vector3): void;
	applyTorqueImpulse(torque: Vector3): void;
	clearForces(): void;
	setDamping(linear: f32, angular: f32): void;
	updateInertiaTensor(): void;
	getCenterOfMassPosition(): Vector3;
	getCenterOfMassTransform(): Transform;
	setCenterOfMassTransform(trans: Transform): void;
	getLinearVelocity(): Vector3;
	setLinearVelocity(lin_vel: Vector3): void;
	getAngularVelocity(): Vector3;
	setAngularVelocity(ang_vel: Vector3): void;
	setLinearFactor(linearFactor: Vector3): void;
	setAngularFactor(angFac: Vector3): void;
	setSleepingThresholds(linear: f32, angular: f32): void;
	applyGravity(): void;
	getGravity(): Vector3;
	setGravity(acceleration: Vector3): void;
}

// @:native("Ammo.btCollisionConfiguration")
declare class CollisionConfiguration {}

// @:native("Ammo.btDefaultCollisionConfiguration")
declare class DefaultCollisionConfiguration extends CollisionConfiguration {
	constructor(): void;
}

// @:native("Ammo.btSoftBodyRigidBodyCollisionConfiguration")
declare class SoftBodyRigidBodyCollisionConfiguration extends CollisionConfiguration {
	constructor(): void;
}

// @:native("Ammo.btDispatcher")
declare class Dispatcher {
	getManifoldByIndexInternal(index: i32): PersistentManifold;
	getNumManifolds(): i32;
}

// @:native("Ammo.btCollisionDispatcher")
declare class CollisionDispatcher extends Dispatcher {
	constructor(collisionConfiguration: CollisionConfiguration): void;
}

// @:native("Ammo.btBroadphaseInterface")
declare class BroadphaseInterface {}

// @:native("Ammo.btDbvtBroadphase")
declare class DbvtBroadphase extends BroadphaseInterface {
	constructor(): void;
}

// @:native("Ammo.btAxisSweep3")
declare class AxisSweep3 extends BroadphaseInterface {
	constructor(worldAabbMin: Vector3, worldAabbMax: Vector3): void;
}

// @:native("Ammo.btConstraintSolver")
declare class ConstraintSolver {}

// @:native("Ammo.btSequentialImpulseConstraintSolver")
declare class SequentialImpulseConstraintSolver extends ConstraintSolver {
	constructor(): void;
}

// @:native("Ammo.btDefaultSoftBodySolver")
declare class DefaultSoftBodySolver extends ConstraintSolver {
	constructor(): void;
}

// @:native("Ammo.RayResultCallback")
declare class RayResultCallback {
	hasHit(): bool;
	constructor(): void;
	get_m_collisionFilterGroup(): i32;
    set_m_collisionFilterGroup(g: i32): void;
    get_m_collisionFilterMask(): i32;
    set_m_collisionFilterMask(m: i32): void;
	get_m_collisionObject(): CollisionObject;
}

// @:native("Ammo.ClosestRayResultCallback")
declare class ClosestRayResultCallback extends RayResultCallback {
	constructor(rayFromWorld: Vector3, rayToWorld: Vector3): void;
	get_m_hitNormalWorld(): Vector3;
	get_m_hitPointWorld(): Vector3;
}

// @:native("Ammo.ConcreteContactResultCallback")
declare class ConcreteContactResultCallback extends RayResultCallback {
	constructor(): void;
}

// @:native("Ammo.btCollisionWorld")
declare class CollisionWorld {
	rayTest(rayFromWorld: Vector3, rayToWorld: Vector3, resultCallback: RayResultCallback): void;
	updateSingleAabb(colObj: CollisionObject): void;
	getPairCache(): OverlappingPairCache;
	addCollisionObject(collisionObject: CollisionObject): void;
	removeCollisionObject(collisionObject: CollisionObject): void;
	// @:native("addCollisionObject")
	addCollisionObjectToGroup(collisionObject: CollisionObject, collisionFilterGroup: i32, collisionFilterMask: i32): void;
}

// @:native("Ammo.btDynamicsWorld")
declare class DynamicsWorld extends CollisionWorld {
	addRigidBody(body: RigidBody): void;
	// @:native("addRigidBody")
	addRigidBodyToGroup(body: RigidBody, group: i32, mask: i32): void;
	removeRigidBody(body: RigidBody): void;
	addAction(action: ActionInterface): void;
	removeAction(action: ActionInterface): void;
	addConstraint(constraint: TypedConstraint, disableCollisionsBetweenLinkedBodies: bool = false): void;
	removeConstraint(constraint: TypedConstraint): void;
	getGravity(): Vector3;
	setGravity(v: Vector3): void;
	stepSimulation(timeStep: f32, maxSubSteps: f32 = 1, fixedTimeStep: f32 = 1.0 / 60.0): void;
}

// @:native("Ammo.btDiscreteDynamicsWorld")
declare class DiscreteDynamicsWorld extends DynamicsWorld {
	constructor(dispatcher: Dispatcher, pairCache: BroadphaseInterface, constraintSolver: ConstraintSolver, collisionConfiguration: CollisionConfiguration): void;
	debugDrawWorld(): void;
}

// @:native("Ammo.btSoftBodyWorldInfo")
declare class SoftBodyWorldInfo {
	set_m_gravity(v: Vector3): void;
}

// @:native("Ammo.btSoftRigidDynamicsWorld")
declare class SoftRigidDynamicsWorld extends DynamicsWorld {
	constructor(dispatcher: Dispatcher, pairCache: BroadphaseInterface, constraintSolver: ConstraintSolver, collisionConfiguration: CollisionConfiguration, softConstraintSolver: ConstraintSolver): void;
	addSoftBody(body: SoftBody, collisionFilterGroup: i32, collisionFilterMask: i32): void;
	removeSoftBody(body: SoftBody): void;
	getWorldInfo(): SoftBodyWorldInfo;
}

// @:native("Ammo.btSimpleDynamicsWorld")
declare class SimpleDynamicsWorld extends DynamicsWorld {
	constructor(dispatcher: Dispatcher, pairCache: BroadphaseInterface, constraintSolver: ConstraintSolver, collisionConfiguration: CollisionConfiguration): void;
}

// @:native("Ammo.btCollisionShape")
declare class CollisionShape {
	calculateLocalInertia(mass: f32, inertia: Vector3): void;
	setMargin(margin: f32): void;
	setLocalScaling(scaling: Vector3): void;
	getLocalScaling(): Vector3;
}

// @:native("Ammo.btCompoundShape")
declare class CompoundShape extends CollisionShape {
	constructor(enableDynamicAabbTree: bool = true): void;
	addChildShape(localTransform: Transform, shape: CollisionShape): void;
}

// @:native("Ammo.btConvexShape")
declare class ConvexShape extends CollisionShape {}

// @:native("Ammo.btConcaveShape")
declare class ConcaveShape extends CollisionShape {}

// @:native("Ammo.btConvexInternalShape")
declare class ConvexInternalShape extends ConvexShape {}

// @:native("Ammo.btPolyhedralConvexShape")
declare class PolyhedralConvexShape extends ConvexInternalShape {}

// @:native("Ammo.btBoxShape")
declare class BoxShape extends PolyhedralConvexShape {
	constructor(boxHalfExtents: Vector3): void;
}

// @:native("Ammo.btSphereShape")
declare class SphereShape extends ConvexInternalShape {
	constructor(radius: f32): void;
}

// @:native("Ammo.btStaticPlaneShape")
declare class StaticPlaneShape extends ConcaveShape {
	constructor(planeNormal: Vector3, planeConstant: f32): void;
}

// @:native("Ammo.btPolyhedralConvexAabbCachingShape")
declare class PolyhedralConvexAabbCachingShape extends PolyhedralConvexShape {}

// @:native("Ammo.btConvexHullShape")
declare class ConvexHullShape extends PolyhedralConvexAabbCachingShape {
	constructor(): void;
	addPoint(point: Vector3, recalculateLocalAabb: bool = true): void;
}

// @:native("Ammo.btCapsuleShape")
declare class CapsuleShape extends ConvexInternalShape {
	constructor(radius: f32, height: f32): void;
	getUpAxis(): i32;
	getRadius(): f32;
	getHalfHeight(): f32;
}

// @:native("Ammo.btCapsuleShapeX")
declare class CapsuleShapeX extends CapsuleShape {
	constructor(radius: f32, height: f32): void;
}

// @:native("Ammo.btCapsuleShapeZ")
declare class CapsuleShapeZ extends CapsuleShape {
	constructor(radius: f32, height: f32): void;
}

// @:native("Ammo.btCylinderShape")
declare class CylinderShape extends ConvexInternalShape {
	constructor(halfExtents: Vector3): void;
}

// @:native("Ammo.btCylinderShapeX")
declare class CylinderShapeX extends CylinderShape {
	constructor(halfExtents: Vector3): void;
}

// @:native("Ammo.btCylinderShapeZ")
declare class CylinderShapeZ extends CylinderShape {
	constructor(halfExtents: Vector3): void;
}

// @:native("Ammo.btConeShape")
declare class ConeShape extends ConvexInternalShape {
	constructor(radius: f32, height: f32): void;
}

// @:native("Ammo.btConeShapeX")
declare class ConeShapeX extends ConeShape {
	constructor(radius: f32, height: f32): void;
}

// @:native("Ammo.btConeShapeZ")
declare class ConeShapeZ extends ConeShape {
	constructor(radius: f32, height: f32): void;
}

// @:native("Ammo.btHeightfieldTerrainShape")
declare class HeightfieldTerrainShape extends ConcaveShape {
	// heightDataType - float, double, integer, short, fixedpoint88, uchar
	constructor(heightStickWidth: i32, heightStickLength: i32, heightfieldData: any, heightScale: f32, minHeight: f32, maxHeight: f32, upAxis: i32, heightDataType: i32, flipQuadEdges: bool): void;
}

// @:native("Ammo.btStridingMeshInterface")
declare class StridingMeshInterface {
	constructor(): void;
}

// @:native("Ammo.btIndexedMesh")
declare class IndexedMesh {
	constructor(): void;
}

// @:native("Ammo.btTriangleIndexVertexArray")
declare class TriangleIndexVertexArray extends StridingMeshInterface {
	constructor(): void;
}

// @:native("Ammo.btTriangleMesh")
declare class TriangleMesh extends TriangleIndexVertexArray {
	constructor(use32bitIndices: bool = true, use4componentVertices: bool = true): void;
	addTriangle(vertex0: Vector3, vertex1: Vector3, vertex2: Vector3, removeDuplicateVertices: bool = false): void;
}

// @:native("Ammo.btTriangleMeshShape")
declare class TriangleMeshShape extends ConcaveShape {
	constructor(meshInterface: StridingMeshInterface): void;
}

// @:native("Ammo.btBvhTriangleMeshShape")
declare class BvhTriangleMeshShape extends TriangleMeshShape {
	constructor(meshInterface: StridingMeshInterface, useQuantizedAabbCompression: bool, buildBvh: bool = true): void;
}

// @:native("Ammo.btGImpactMeshShape")
declare class GImpactMeshShape extends ConcaveShape {
	constructor(meshInterface: StridingMeshInterface): void;
	updateBound(): void;
}

// @:native("Ammo.GImpactCollisionAlgorithm")
declare class GImpactCollisionAlgorithm {
	constructor(): void;
	registerAlgorithm(dispatcher: CollisionDispatcher): void;
}

// @:native("Ammo.btVehicleTuning")
declare class VehicleTuning extends ActionInterface {
	constructor(): void;
}

// @:native("Ammo.btVehicleRaycaster")
declare class VehicleRaycaster {}

// @:native("Ammo.btDefaultVehicleRaycaster")
declare class DefaultVehicleRaycaster extends VehicleRaycaster {
	constructor(world: DynamicsWorld): void;
}

// @:native("Ammo.btWheelInfoConstructionInfo")
declare class RaycastInfo {
	m_contactNormalWS: Vector3;
	m_contactPointWS: Vector3;
	m_suspensionLength: f32;
	m_hardPointWS: Vector3;
	m_wheelDirectionWS: Vector3;
	m_wheelAxleWS: Vector3;
	m_isInContact: bool;
}

// @:native("Ammo.btWheelInfoConstructionInfo")
declare class WheelInfoConstructionInfo {
	constructor(): void;
	m_chassisConnectionCS: Vector3;
	m_wheelDirectionCS: Vector3;
	m_wheelAxleCS: Vector3;
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

// @:native("Ammo.btWheelInfo")
declare class WheelInfo {
	constructor(ci: WheelInfoConstructionInfo): void;
    m_raycastInfo: RaycastInfo;
    m_worldTransform: Transform;
    m_chassisConnectionPointCS: Vector3;
    m_wheelDirectionCS: Vector3;
    m_wheelAxleCS: Vector3;
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

// @:native("Ammo.btRaycastVehicle")
declare class RaycastVehicle extends ActionInterface {
	constructor(tuning: VehicleTuning, chassis: RigidBody, raycaster: VehicleRaycaster): void;
	setCoordinateSystem(rightIndex: i32, upIndex: i32, forwardIndex: i32): void;
	addWheel(connectionPointCS0: Vector3, wheelDirectionCS0: Vector3, wheelAxleCS: Vector3, suspensionRestLength: f32, wheelRadius: f32, tuning: VehicleTuning, isFrontWheel: bool): WheelInfo;
	getNumWheels(): i32;
	getWheelInfo(index: i32): WheelInfo;
	resetSuspension(): void;
	getWheelTransformWS(wheelIndex: i32): Transform;
	updateWheelTransform(wheelIndex: i32, interpolatedTransform: bool = true): void;
	applyEngineForce(force: f32, wheel: i32): void;
	setBrake(brake: f32, wheelIndex: i32): void;
	setPitchControl(pitch: f32): void;
	updateSuspension(deltaTime: f32 ): void;
	updateFriction(deltaTime: f32 ): void;
	setSteeringValue(steering: f32, wheel: i32): void;
	getRightAxis(): i32;
	getUpAxis(): i32;
	getForwardAxis(): i32;
	getForwardVector(): Vector3;
	getCurrentSpeedKmHour(): f32;
}

// @:native("Ammo.btPersistentManifold")
declare class PersistentManifold extends TypedObject {
	constructor(): void;
	getBody0(): CollisionObject;
	getBody1(): CollisionObject;
	getNumContacts(): i32;
	getContactPoint(index: i32): ManifoldPoint;
}

// @:native("Ammo.btManifoldPoint")
declare class ManifoldPoint {
	constructor(): void;
	get_m_positionWorldOnA(): Vector3;
	get_m_positionWorldOnB(): Vector3;
	get_m_normalWorldOnB(): Vector3;
	getDistance(): f32;
	getAppliedImpulse(): f32;
}

// @:native("Ammo.btTypedConstraint")
declare class TypedConstraint extends TypedObject {
	setBreakingImpulseThreshold(threshold: f32): void;
}

// @:native("Ammo.btGeneric6DofConstraint")
declare class Generic6DofConstraint extends TypedConstraint {
	constructor(rbB: RigidBody, frameInB: Transform, useLinearReferenceFrameB: bool): void;
	static new2(rbA: RigidBody, rbB: RigidBody, frameInA: Transform, frameInB: Transform, useLinearReferenceFrameB: bool = false): Generic6DofConstraint {
		let _r1 = rbA, _r2 = rbB, _fa = frameInA, _fb = frameInB, _b = useLinearReferenceFrameB;
		return new Ammo.btGeneric6DofConstraint(_r1, _r2, _fa, _fb, _b);
	}
	setLinearLowerLimit(linearLower: Vector3): void;
	setLinearUpperLimit(linearUpper: Vector3): void;
	setAngularLowerLimit(angularLower: Vector3): void;
	setAngularUpperLimit(angularUpper: Vector3): void;
	setParam(num: i32, value: f32, axis: i32 = -1): void;
	getFrameOffsetA(): Transform;
}

// @:native("Ammo.btGeneric6DofSpringConstraint")
declare class Generic6DofSpringConstraint extends Generic6DofConstraint {
	constructor(rbA: RigidBody, rbB: RigidBody, frameInA: Transform, frameInB: Transform, useLinearReferenceFrameB: bool): void;
	enableSpring(index: i32, onOff: bool): void;
    setStiffness(index: i32, stiffness: f32): void;
  	setDamping(index: i32, damping: f32): void;
}

// @:native("Ammo.btHingeConstraint")
declare class HingeConstraint extends TypedConstraint {
	constructor(rbA: RigidBody, rbB: RigidBody, pivotInA: Vector3, pivotInB: Vector3, axisInA: Vector3, axisInB: Vector3, useReferenceFrameA: bool = false): void;
	setLimit(low: f32, high: f32, _softness: f32 = 0.9, _biasFactor: f32 = 0.3, _relaxationFactor: f32 = 1.0): void;
}

// @:native("Ammo.btSliderConstraint")
declare class SliderConstraint extends TypedConstraint {
	constructor(rbA: RigidBody, rbB: RigidBody, frameInA: Transform, frameInB: Transform, useReferenceFrameA: bool): void;
	setLowerLinLimit(lowerLimit: f32): void;
    setUpperLinLimit(upperLimit: f32): void;
    setLowerAngLimit(lowerAngLimit: f32): void;
    setUpperAngLimit(upperAngLimit: f32): void;
}

// @:native("Ammo.btPoint2PointConstraint")
declare class Point2PointConstraint extends TypedConstraint {
	constructor(rbA: RigidBody, rbB: RigidBody, pivotInA: Vector3, pivotInB: Vector3): void;
	setPivotA(pivotA: Vector3): void;
	setPivotB(pivotB: Vector3): void;
	getPivotInA(): Vector3;
	getPivotInB(): Vector3;
}

// @:native("Ammo.Config")
declare class AmmoConfig {
	set_viterations(i: i32): void;
	set_piterations(i: i32): void;
	set_collisions(i: i32): void;
	set_kDF(f: f32): void;
	set_kDP(f: f32): void;
	set_kPR(f: f32): void;
	set_kVC(f: f32): void;
	set_kAHR(f: f32): void;
}

// @:native("Ammo.tNodeArray")
declare class TNodeArray {
	at(i: i32): Node;
	size(): i32;
}

// @:native("Ammo.tMaterialArray")
declare class TMaterialArray {
	at(i: i32): Material;
}

// @:native("Ammo.tAnchorArray")
declare class TAnchorArray {
	constructor();
	at(i: i32): Anchor;
	clear(): void;
	size(): i32;
	push_back(anc: Anchor): void;
	pop_back(): void;
}

// @:native("Ammo.Node")
declare class Node {
	get_m_x(): Vector3;
	get_m_n(): Vector3;
}

// @:native("Ammo.Material")
declare class Material {
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

// @:native("Ammo.Anchor")
declare class Anchor {
	set_m_node(node: Node): void;
	get_m_node(): Node;
	set_m_local(local: Vector3): void;
	get_m_local(): Vector3;
	set_m_body(body: RigidBody): void;
	get_m_body(): RigidBody;
	set_m_influence(influence: f32): void;
	get_m_influence(): f32;
	set_m_c1(c1: Vector3): void;
	get_m_c1(): Vector3;
	set_m_c2(c2: f32): void;
	get_m_c2(): f32;
}

// @:native("Ammo.btSoftBody")
declare class SoftBody extends CollisionObject {
	get_m_nodes(): TNodeArray;
	get_m_cfg(): Config;
	get_m_materials(): TMaterialArray;
	get_m_anchors(): TAnchorArray;
	setTotalMass(mass: f32, fromfaces: bool = false): void;
	generateClusters(k: i32, maxiterations: i32 = 8192): void;
	generateBendingConstraints(distance: i32, mat: any = 0): void;
	appendAnchor(node: i32, body: RigidBody, disableCollisionBetweenLinkedBodies: bool, influence: f32): void;
	appendLink(node: Node, node1: Node, mat: Material, bcheckexist: bool = false): void;
	addForce(f: Vector3, node: i32): void;
}

// @:native("Ammo.btSoftBodyHelpers")
declare class SoftBodyHelpers {
	constructor(): void;
	CreateFromTriMesh(worldInfo: SoftBodyWorldInfo, vertices:f32[], triangles:i32[], ntriangles: i32, randomizeConstraints: bool = true): SoftBody;
}

// @:native("Ammo.btOverlappingPairCallback")
declare class OverlappingPairCallback {}

// @:native("Ammo.btGhostPairCallback")
declare class GhostPairCallback extends OverlappingPairCallback {
	constructor(): void;
}

// @:native("Ammo.btOverlappingPairCache")
declare class OverlappingPairCache {
	setInternalGhostPairCallback(ghostPairCallback: OverlappingPairCallback): void;
}

// @:native("Ammo.btGhostObject")
declare class GhostObject extends CollisionObject {
	constructor(): void;
	getNumOverlappingObjects(): i32;
	getOverlappingObject(index: i32): CollisionObject;
}

// @:native("Ammo.btPairCachingGhostObject")
declare class PairCachingGhostObject extends GhostObject {
	constructor(): void;
}

// @:native("Ammo.btKinematicCharacterController")
declare class KinematicCharacterController extends ActionInterface {
	constructor(ghostObject: PairCachingGhostObject, convexShape: ConvexShape, stepHeight: f32, upAxis: i32 = 1): void;
	setUpAxis(axis: i32): void; // setUp in cpp
	jump(): void;
	setGravity(gravity: f32): void;
	getGravity(): f32;
	canJump(): bool;
	onGround(): bool;
	setWalkDirection(walkDirection: Vector3): void;
	setVelocityForTimeInterval(velocity: Vector3, timeInterval: f32): void;
	warp(origin: Vector3): void;
	preStep(collisionWorld: CollisionWorld): void;
	playerStep(collisionWorld: CollisionWorld, dt: f32): void;
	setFallSpeed(fallSpeed: f32): void;
	setJumpSpeed(jumpSpeed: f32): void;
	setMaxJumpHeight(maxJumpHeight: f32): void;
	setMaxSlope(slopeRadians: f32): void;
	getMaxSlope(): f32;
	getGhostObject(): PairCachingGhostObject;
	setUseGhostSweepTest(useGhostObjectSweepTest: bool): void;
	setUpInterpolate(value: bool): void;
}

// @:native("Ammo")
declare class Ammo {
	static destroy(obj: any): void;
	static castObject(obj: any, to: Class<any>): any;
	static _malloc(byteSize: i32): i32;
	static HEAPU8: any;
	static HEAPF32: any;
}

///end
