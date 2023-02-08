package arm.plugin;

@:native("Ammo.btTypedObject")
extern class TypedObject {}

@:native("Ammo.btVector3")
extern class Vector3 {
	public function new(x: Float, y: Float, z: Float): Void;
	public function setValue(x: Float, y: Float, z: Float): Void;
	public function setX(x: Float): Void;
	public function setY(y: Float): Void;
	public function setZ(z: Float): Void;
	public function x(): Float;
	public function y(): Float;
	public function z(): Float;
	public function length(): Float;
	public function normalize(): Void;
}

@:native("Ammo.btQuadWord")
extern class QuadWord {
	public function setX(x: Float): Void;
	public function setY(y: Float): Void;
	public function setZ(z: Float): Void;
	public function setW(w: Float): Void;
	public function x(): Float;
	public function y(): Float;
	public function z(): Float;
	public function w(): Float;
}

@:native("Ammo.btQuaternion")
extern class Quaternion extends QuadWord {
	public function new(x: Float, y: Float, z: Float, w: Float): Void;
	public function setEuler(yaw: Float, pitch: Float, roll: Float): Void;
	public function slerp(q: Quaternion, t: Float): Quaternion;
	public function setValue(x: Float, y: Float, z: Float, w: Float): Void;
}

@:native("Ammo.btMatrix3x3")
extern class Matrix3x3 {
	public function setEulerZYX(ex: Float, ey: Float, ez: Float): Void;
	public function getRotation(q: Quaternion): Void;
	public function getRow(y: Int): Vector3;
}

@:native("Ammo.btActionInterface")
extern class ActionInterface {}

@:native("Ammo.btTransform")
extern class Transform {
	public function new(): Void;
	public function setIdentity(): Void;
	public function setOrigin(inVec: Vector3): Void;
	public function getOrigin(): Vector3;
	public function setRotation(inQuat: Quaternion): Void;
	public function getRotation(): Quaternion;
}

@:native("Ammo.btIDebugDraw")
extern class IDebugDraw  {
	public function drawLine(from: Vector3, to: Vector3, color: Vector3): Void;
	public function drawContactPoint(pointOnB: Vector3, normalOnB: Vector3, distance: Float, lifeTime: Int, color: Vector3): Void;
	public function reportErrorWarning(warningString: String): Void;
	public function draw3dText(location: Vector3, textString: String): Void;
	public function setDebugMode(debugMode: Int): Void;
	public function getDebugMode(): Int;
}

@:native("Ammo.btMotionState")
extern class MotionState {
	public function getWorldTransform(centerOfMassWorldTrans: Transform): Void;
	public function setWorldTransform(centerOfMassWorldTrans: Transform): Void;
}

@:native("Ammo.btDefaultMotionState")
extern class DefaultMotionState extends MotionState {
	public function new(worldTrans: Transform, centerOfMassOffset: Transform): Void;
}

@:native("Ammo.btRigidBodyConstructionInfo")
extern class RigidBodyConstructionInfo {
	public function new(mass: Float, motionState: MotionState, collisionShape: CollisionShape, localInertia: Vector3): Void;
	public var m_friction: Float;
	public var m_rollingFriction: Float;
}

@:native("Ammo.btCollisionObject")
extern class CollisionObject {
	public static inline var ACTIVE_TAG = 1;
	public static inline var ISLAND_SLEEPING = 2;
	public static inline var WANTS_DEACTIVATION = 3;
	public static inline var DISABLE_DEACTIVATION = 4;
	public static inline var DISABLE_SIMULATION = 5;
	public static inline var CF_STATIC_OBJECT = 1;
	public static inline var CF_KINEMATIC_OBJECT = 2;
	public static inline var CF_NO_CONTACT_RESPONSE = 4;
	public static inline var CF_CHARACTER_OBJECT = 16;
	public function getWorldTransform(): Transform;
	public function setWorldTransform(trans: Transform): Void;
	public function activate(forceActivation: Bool = false): Void;
	public function setActivationState(newState: Int): Void;
	public function getUserIndex(): Int;
	public function setUserIndex(index: Int): Void;
	public function getUserPointer(): Dynamic;
	public function setUserPointer(userPointer: Dynamic): Void;
	public function isActive(): Bool;
	public function isKinematicObject(): Bool;
	public function isStaticObject(): Bool;
	public function isStaticOrKinematicObject(): Bool;
	public function setFriction(frict: Float): Void;
	public function setRollingFriction(frict: Float): Void;
	public function setRestitution(rest: Float): Void;
	public function setContactProcessingThreshold(contactProcessingThreshold: Float): Void;
	public function setCollisionShape(collisionShape: CollisionShape): Void;
	public function getCollisionShape(): CollisionShape;
	public function setCollisionFlags(flags: Int): Void;
	public function getCollisionFlags(): Int;
	public function setCcdSweptSphereRadius(radius: Float): Void;
	public function setCcdMotionThreshold(ccdMotionThreshold: Float): Void;
}

@:native("Ammo.btRigidBody")
extern class RigidBody extends CollisionObject {
	public function new(constructionInfo: RigidBodyConstructionInfo): Void;
	public function setMassProps(mass: Float, inertia: Vector3): Void;
	public function getMotionState(): MotionState;
	public function applyCentralForce(force: Vector3): Void;
	public function applyForce(force: Vector3, rel_pos: Vector3): Void;
	public function applyCentralImpulse(impulse: Vector3): Void;
	public function applyImpulse(impulse: Vector3, rel_pos: Vector3): Void;
	public function applyTorque(torque: Vector3): Void;
	public function applyTorqueImpulse(torque: Vector3): Void;
	public function clearForces(): Void;
	public function setDamping(linear: Float, angular: Float): Void;
	public function updateInertiaTensor(): Void;
	public function getCenterOfMassPosition(): Vector3;
	public function getCenterOfMassTransform(): Transform;
	public function setCenterOfMassTransform(trans: Transform): Void;
	public function getLinearVelocity(): Vector3;
	public function setLinearVelocity(lin_vel: Vector3): Void;
	public function getAngularVelocity(): Vector3;
	public function setAngularVelocity(ang_vel: Vector3): Void;
	public function setLinearFactor(linearFactor: Vector3): Void;
	public function setAngularFactor(angFac: Vector3): Void;
	public function setSleepingThresholds(linear: Float, angular: Float): Void;
	public function applyGravity(): Void;
	public function getGravity(): Vector3;
	public function setGravity(acceleration: Vector3): Void;
}

@:native("Ammo.btCollisionConfiguration")
extern class CollisionConfiguration {}

@:native("Ammo.btDefaultCollisionConfiguration")
extern class DefaultCollisionConfiguration extends CollisionConfiguration {
	public function new(): Void;
}

@:native("Ammo.btSoftBodyRigidBodyCollisionConfiguration")
extern class SoftBodyRigidBodyCollisionConfiguration extends CollisionConfiguration {
	public function new(): Void;
}

@:native("Ammo.btDispatcher")
extern class Dispatcher {
	public function getManifoldByIndexInternal(index: Int): PersistentManifold;
	public function	getNumManifolds(): Int;
}

@:native("Ammo.btCollisionDispatcher")
extern class CollisionDispatcher extends Dispatcher {
	public function new(collisionConfiguration: CollisionConfiguration): Void;
}

@:native("Ammo.btBroadphaseInterface")
extern class BroadphaseInterface {}

@:native("Ammo.btDbvtBroadphase")
extern class DbvtBroadphase extends BroadphaseInterface {
	public function new(): Void;
}

@:native("Ammo.btAxisSweep3")
extern class AxisSweep3 extends BroadphaseInterface {
	public function new(worldAabbMin: Vector3, worldAabbMax: Vector3): Void;
}

@:native("Ammo.btConstraintSolver")
extern class ConstraintSolver {}

@:native("Ammo.btSequentialImpulseConstraintSolver")
extern class SequentialImpulseConstraintSolver extends ConstraintSolver {
	public function new(): Void;
}

@:native("Ammo.btDefaultSoftBodySolver")
extern class DefaultSoftBodySolver extends ConstraintSolver {
	public function new(): Void;
}

@:native("Ammo.RayResultCallback")
extern class RayResultCallback {
	public function hasHit(): Bool;
	public function new(): Void;
	public function get_m_collisionFilterGroup(): Int;
    public function set_m_collisionFilterGroup(g: Int): Void;
    public function get_m_collisionFilterMask(): Int;
    public function set_m_collisionFilterMask(m: Int): Void;
	public function get_m_collisionObject(): CollisionObject;
}

@:native("Ammo.ClosestRayResultCallback")
extern class ClosestRayResultCallback extends RayResultCallback {
	public function new(rayFromWorld: Vector3, rayToWorld: Vector3): Void;
	public function get_m_hitNormalWorld(): Vector3;
	public function get_m_hitPointWorld(): Vector3;
}

@:native("Ammo.ConcreteContactResultCallback")
extern class ConcreteContactResultCallback extends RayResultCallback {
	public function new(): Void;
}

@:native("Ammo.btCollisionWorld")
extern class CollisionWorld {
	public function rayTest(rayFromWorld: Vector3, rayToWorld: Vector3, resultCallback: RayResultCallback): Void;
	public function updateSingleAabb(colObj: CollisionObject): Void;
	public function getPairCache(): OverlappingPairCache;
	public function addCollisionObject(collisionObject: CollisionObject): Void;
	public function removeCollisionObject(collisionObject: CollisionObject): Void;
	@:native("addCollisionObject")
	public function addCollisionObjectToGroup(collisionObject: CollisionObject, collisionFilterGroup: Int, collisionFilterMask: Int): Void;
}

@:native("Ammo.btDynamicsWorld")
extern class DynamicsWorld extends CollisionWorld {
	public function addRigidBody(body: RigidBody): Void;
	@:native("addRigidBody")
	public function addRigidBodyToGroup(body: RigidBody, group: Int, mask: Int): Void;
	public function removeRigidBody(body: RigidBody): Void;
	public function addAction(action: ActionInterface): Void;
	public function removeAction(action: ActionInterface): Void;
	public function addConstraint(constraint: TypedConstraint, disableCollisionsBetweenLinkedBodies: Bool = false): Void;
	public function removeConstraint(constraint: TypedConstraint): Void;
	public function getGravity(): Vector3;
	public function setGravity(v: Vector3): Void;
	public function stepSimulation(timeStep: Float, maxSubSteps: Float = 1, fixedTimeStep: Float = 1.0 / 60.0): Void;
}

@:native("Ammo.btDiscreteDynamicsWorld")
extern class DiscreteDynamicsWorld extends DynamicsWorld {
	public function new(dispatcher: Dispatcher, pairCache: BroadphaseInterface, constraintSolver: ConstraintSolver, collisionConfiguration: CollisionConfiguration): Void;
	public function debugDrawWorld(): Void;
}

@:native("Ammo.btSoftBodyWorldInfo")
extern class SoftBodyWorldInfo {
	public function set_m_gravity(v: Vector3): Void;
}

@:native("Ammo.btSoftRigidDynamicsWorld")
extern class SoftRigidDynamicsWorld extends DynamicsWorld {
	public function new(dispatcher: Dispatcher, pairCache: BroadphaseInterface, constraintSolver: ConstraintSolver, collisionConfiguration: CollisionConfiguration, softConstraintSolver: ConstraintSolver): Void;
	public function addSoftBody(body: SoftBody, collisionFilterGroup: Int, collisionFilterMask: Int): Void;
	public function removeSoftBody(body: SoftBody): Void;
	public function getWorldInfo(): SoftBodyWorldInfo;
}

@:native("Ammo.btSimpleDynamicsWorld")
extern class SimpleDynamicsWorld extends DynamicsWorld {
	public function new(dispatcher: Dispatcher, pairCache: BroadphaseInterface, constraintSolver: ConstraintSolver, collisionConfiguration: CollisionConfiguration): Void;
}

@:native("Ammo.btCollisionShape")
extern class CollisionShape {
	public function calculateLocalInertia(mass: Float, inertia: Vector3): Void;
	public function setMargin(margin: Float): Void;
	public function setLocalScaling(scaling: Vector3): Void;
	public function getLocalScaling(): Vector3;
}

@:native("Ammo.btCompoundShape")
extern class CompoundShape extends CollisionShape {
	public function new(enableDynamicAabbTree: Bool = true): Void;
	public function addChildShape(localTransform: Transform, shape: CollisionShape): Void;
}

@:native("Ammo.btConvexShape")
extern class ConvexShape extends CollisionShape {}

@:native("Ammo.btConcaveShape")
extern class ConcaveShape extends CollisionShape {}

@:native("Ammo.btConvexInternalShape")
extern class ConvexInternalShape extends ConvexShape {}

@:native("Ammo.btPolyhedralConvexShape")
extern class PolyhedralConvexShape extends ConvexInternalShape {}

@:native("Ammo.btBoxShape")
extern class BoxShape extends PolyhedralConvexShape {
	public function new(boxHalfExtents: Vector3): Void;
}

@:native("Ammo.btSphereShape")
extern class SphereShape extends ConvexInternalShape {
	public function new(radius: Float): Void;
}

@:native("Ammo.btStaticPlaneShape")
extern class StaticPlaneShape extends ConcaveShape {
	public function new(planeNormal: Vector3, planeConstant: Float): Void;
}

@:native("Ammo.btPolyhedralConvexAabbCachingShape")
extern class PolyhedralConvexAabbCachingShape extends PolyhedralConvexShape {}

@:native("Ammo.btConvexHullShape")
extern class ConvexHullShape extends PolyhedralConvexAabbCachingShape {
	public function new(): Void;
	public function addPoint(point: Vector3, recalculateLocalAabb: Bool = true): Void;
}

@:native("Ammo.btCapsuleShape")
extern class CapsuleShape extends ConvexInternalShape {
	public function new(radius: Float, height: Float): Void;
	public function getUpAxis(): Int;
	public function getRadius(): Float;
	public function getHalfHeight(): Float;
}

@:native("Ammo.btCapsuleShapeX")
extern class CapsuleShapeX extends CapsuleShape {
	public function new(radius: Float, height: Float): Void;
}

@:native("Ammo.btCapsuleShapeZ")
extern class CapsuleShapeZ extends CapsuleShape {
	public function new(radius: Float, height: Float): Void;
}

@:native("Ammo.btCylinderShape")
extern class CylinderShape extends ConvexInternalShape {
	public function new(halfExtents: Vector3): Void;
}

@:native("Ammo.btCylinderShapeX")
extern class CylinderShapeX extends CylinderShape {
	public function new(halfExtents: Vector3): Void;
}

@:native("Ammo.btCylinderShapeZ")
extern class CylinderShapeZ extends CylinderShape {
	public function new(halfExtents: Vector3): Void;
}

@:native("Ammo.btConeShape")
extern class ConeShape extends ConvexInternalShape {
	public function new(radius: Float, height: Float): Void;
}

@:native("Ammo.btConeShapeX")
extern class ConeShapeX extends ConeShape {
	public function new(radius: Float, height: Float): Void;
}

@:native("Ammo.btConeShapeZ")
extern class ConeShapeZ extends ConeShape {
	public function new(radius: Float, height: Float): Void;
}

@:native("Ammo.btHeightfieldTerrainShape")
extern class HeightfieldTerrainShape extends ConcaveShape {
	// heightDataType - float, double, integer, short, fixedpoint88, uchar
	public function new(heightStickWidth: Int, heightStickLength: Int, heightfieldData: Dynamic, heightScale: Float, minHeight: Float, maxHeight: Float, upAxis: Int, heightDataType: Int, flipQuadEdges: Bool): Void;
}

@:native("Ammo.btStridingMeshInterface")
extern class StridingMeshInterface {
	public function new(): Void;
}

@:native("Ammo.btIndexedMesh")
extern class IndexedMesh {
	public function new(): Void;
}

@:native("Ammo.btTriangleIndexVertexArray")
extern class TriangleIndexVertexArray extends StridingMeshInterface {
	public function new(): Void;
}

@:native("Ammo.btTriangleMesh")
extern class TriangleMesh extends TriangleIndexVertexArray {
	public function new(use32bitIndices: Bool = true, use4componentVertices: Bool = true): Void;
	public function addTriangle(vertex0: Vector3, vertex1: Vector3, vertex2: Vector3, removeDuplicateVertices: Bool = false): Void;
}

@:native("Ammo.btTriangleMeshShape")
extern class TriangleMeshShape extends ConcaveShape {
	public function new(meshInterface: StridingMeshInterface): Void;
}

@:native("Ammo.btBvhTriangleMeshShape")
extern class BvhTriangleMeshShape extends TriangleMeshShape {
	public function new(meshInterface: StridingMeshInterface, useQuantizedAabbCompression: Bool, buildBvh: Bool = true): Void;
}

@:native("Ammo.btGImpactMeshShape")
extern class GImpactMeshShape extends ConcaveShape {
	public function new(meshInterface: StridingMeshInterface): Void;
	public function updateBound(): Void;
}

@:native("Ammo.GImpactCollisionAlgorithm")
extern class GImpactCollisionAlgorithm {
	public function new(): Void;
	public function registerAlgorithm(dispatcher: CollisionDispatcher): Void;
}

@:native("Ammo.btVehicleTuning")
extern class VehicleTuning extends ActionInterface {
	public function new(): Void;
}

@:native("Ammo.btVehicleRaycaster")
extern class VehicleRaycaster {}

@:native("Ammo.btDefaultVehicleRaycaster")
extern class DefaultVehicleRaycaster extends VehicleRaycaster {
	public function new(world: DynamicsWorld): Void;
}

@:native("Ammo.btWheelInfoConstructionInfo")
extern class RaycastInfo {
	public var m_contactNormalWS: Vector3;
	public var m_contactPointWS: Vector3;
	public var m_suspensionLength: Float;
	public var m_hardPointWS: Vector3;
	public var m_wheelDirectionWS: Vector3;
	public var m_wheelAxleWS: Vector3;
	public var m_isInContact: Bool;
}

@:native("Ammo.btWheelInfoConstructionInfo")
extern class WheelInfoConstructionInfo {
	public function new(): Void;
	public var m_chassisConnectionCS: Vector3;
	public var m_wheelDirectionCS: Vector3;
	public var m_wheelAxleCS: Vector3;
	public var m_suspensionRestLength: Float;
	public var m_maxSuspensionTravelCm: Float;
	public var m_wheelRadius: Float;
	public var m_suspensionStiffness: Float;
	public var m_wheelsDampingCompression: Float;
	public var m_wheelsDampingRelaxation: Float;
	public var m_frictionSlip: Float;
	public var m_maxSuspensionForce: Float;
	public var m_bIsFrontWheel: Bool;
}

@:native("Ammo.btWheelInfo")
extern class WheelInfo {
	public function new(ci: WheelInfoConstructionInfo): Void;
    public var m_raycastInfo: RaycastInfo;
    public var m_worldTransform: Transform;
    public var m_chassisConnectionPointCS: Vector3;
    public var m_wheelDirectionCS: Vector3;
    public var m_wheelAxleCS: Vector3;
	public var m_suspensionRestLength1: Float;
	public var m_maxSuspensionTravelCm: Float;
	public function getSuspensionRestLength(): Float;
	public var m_suspensionStiffness: Float;
	public var m_wheelsDampingCompression: Float;
	public var m_wheelsDampingRelaxation: Float;
	public var m_frictionSlip: Float;
	public var m_steering: Float;
	public var m_rotation: Float;
	public var m_deltaRotation: Float;
	public var m_rollInfluence: Float;
	public var m_maxSuspensionForce: Float;
    public var m_wheelsRadius: Float;
	public var m_engineForce: Float;
	public var m_brake: Float;
	public var m_bIsFrontWheel: Bool;
    public var m_clippedInvContactDotSuspension: Float;
    public var m_suspensionRelativeVelocity: Float;
    public var m_wheelsSuspensionForce: Float;
    public var m_skidInfo: Float;
}

@:native("Ammo.btRaycastVehicle")
extern class RaycastVehicle extends ActionInterface {
	public function new(tuning: VehicleTuning, chassis: RigidBody, raycaster: VehicleRaycaster): Void;
	public function setCoordinateSystem(rightIndex: Int, upIndex: Int, forwardIndex: Int): Void;
	public function addWheel(connectionPointCS0: Vector3, wheelDirectionCS0: Vector3, wheelAxleCS: Vector3, suspensionRestLength: Float, wheelRadius: Float, tuning: VehicleTuning, isFrontWheel: Bool): WheelInfo;
	public function getNumWheels(): Int;
	public function getWheelInfo(index: Int): WheelInfo;
	public function resetSuspension(): Void;
	public function getWheelTransformWS(wheelIndex: Int): Transform;
	public function updateWheelTransform(wheelIndex: Int, interpolatedTransform: Bool = true): Void;
	public function applyEngineForce(force: Float, wheel: Int): Void;
	public function setBrake(brake: Float, wheelIndex: Int): Void;
	public function setPitchControl(pitch: Float): Void;
	public function updateSuspension(deltaTime: Float ): Void;
	public function updateFriction(deltaTime: Float ): Void;
	public function setSteeringValue(steering: Float, wheel: Int): Void;
	public function getRightAxis(): Int;
	public function getUpAxis(): Int;
	public function getForwardAxis(): Int;
	public function getForwardVector(): Vector3;
	public function getCurrentSpeedKmHour(): Float;
}

@:native("Ammo.btPersistentManifold")
extern class PersistentManifold extends TypedObject {
	public function new(): Void;
	public function getBody0(): CollisionObject;
	public function getBody1(): CollisionObject;
	public function getNumContacts(): Int;
	public function getContactPoint(index: Int): ManifoldPoint;
}

@:native("Ammo.btManifoldPoint")
extern class ManifoldPoint {
	public function new(): Void;
	public function get_m_positionWorldOnA(): Vector3;
	public function get_m_positionWorldOnB(): Vector3;
	public function get_m_normalWorldOnB(): Vector3;
	public function getDistance(): Float;
	public function getAppliedImpulse(): Float;
}

@:native("Ammo.btTypedConstraint")
extern class TypedConstraint extends TypedObject {
	public function setBreakingImpulseThreshold(threshold: Float): Void;
}

@:native("Ammo.btGeneric6DofConstraint")
extern class Generic6DofConstraint extends TypedConstraint {
	public function new(rbB: RigidBody, frameInB: Transform, useLinearReferenceFrameB: Bool): Void;
	public static inline function new2(rbA: RigidBody, rbB: RigidBody, frameInA: Transform, frameInB: Transform, useLinearReferenceFrameB: Bool = false): Generic6DofConstraint {
		var _r1 = rbA, _r2 = rbB, _fa = frameInA, _fb = frameInB, _b = useLinearReferenceFrameB;
		return untyped __js__("new Ammo.btGeneric6DofConstraint(_r1, _r2, _fa, _fb, _b)");
	}
	public function setLinearLowerLimit(linearLower: Vector3): Void;
	public function setLinearUpperLimit(linearUpper: Vector3): Void;
	public function setAngularLowerLimit(angularLower: Vector3): Void;
	public function setAngularUpperLimit(angularUpper: Vector3): Void;
	public function setParam(num: Int, value: Float, axis: Int = -1): Void;
	public function getFrameOffsetA(): Transform;
}

@:native("Ammo.btGeneric6DofSpringConstraint")
extern class Generic6DofSpringConstraint extends Generic6DofConstraint {
	public function new(rbA: RigidBody, rbB: RigidBody, frameInA: Transform, frameInB: Transform, useLinearReferenceFrameB: Bool): Void;
	public function enableSpring(index: Int, onOff: Bool): Void;
    public function setStiffness(index: Int, stiffness: Float): Void;
  	public function setDamping(index: Int, damping: Float): Void;
}

@:native("Ammo.btHingeConstraint")
extern class HingeConstraint extends TypedConstraint {
	public function new(rbA: RigidBody, rbB: RigidBody, pivotInA: Vector3, pivotInB: Vector3, axisInA: Vector3, axisInB: Vector3, useReferenceFrameA: Bool = false): Void;
	public function setLimit(low: Float, high: Float, _softness: Float = 0.9, _biasFactor: Float = 0.3, _relaxationFactor: Float = 1.0): Void;
}

@:native("Ammo.btSliderConstraint")
extern class SliderConstraint extends TypedConstraint {
	public function new(rbA: RigidBody, rbB: RigidBody, frameInA: Transform, frameInB: Transform, useReferenceFrameA: Bool): Void;
	public function setLowerLinLimit(lowerLimit: Float): Void;
    public function setUpperLinLimit(upperLimit: Float): Void;
    public function setLowerAngLimit(lowerAngLimit: Float): Void;
    public function setUpperAngLimit(upperAngLimit: Float): Void;
}

@:native("Ammo.btPoint2PointConstraint")
extern class Point2PointConstraint extends TypedConstraint {
	public function new(rbA: RigidBody, rbB: RigidBody, pivotInA: Vector3, pivotInB: Vector3): Void;
	public function setPivotA(pivotA: Vector3): Void;
	public function setPivotB(pivotB: Vector3): Void;
	public function getPivotInA(): Vector3;
	public function getPivotInB(): Vector3;
}

@:native("Ammo.Config")
extern class Config {
	public function set_viterations(i: Int): Void;
	public function set_piterations(i: Int): Void;
	public function set_collisions(i: Int): Void;
	public function set_kDF(f: Float): Void;
	public function set_kDP(f: Float): Void;
	public function set_kPR(f: Float): Void;
	public function set_kVC(f: Float): Void;
	public function set_kAHR(f: Float): Void;
}

@:native("Ammo.tNodeArray")
extern class TNodeArray {
	public function at(i: Int): Node;
	public function size(): Int;
}

@:native("Ammo.tMaterialArray")
extern class TMaterialArray {
	public function at(i: Int): Material;
}

@:native("Ammo.tAnchorArray")
extern class TAnchorArray {
	public function new();
	public function at(i: Int): Anchor;
	public function clear(): Void;
	public function size(): Int;
	public function push_back(anc: Anchor): Void;
	public function pop_back(): Void;
}

@:native("Ammo.Node")
extern class Node {
	public function get_m_x(): Vector3;
	public function get_m_n(): Vector3;
}

@:native("Ammo.Material")
extern class Material {
	public function new();
	public function set_m_kLST(kAST: Float): Void;
	public function get_m_kLST(): Void;
	public function set_m_kAST(kAST: Float): Void;
	public function get_m_kAST(): Void;
	public function set_m_kVST(kVST: Float): Void;
	public function get_m_kVST(): Float;
	public function set_m_flags(flags: Int): Void;
	public function get_m_flags(): Int;
}

@:native("Ammo.Anchor")
extern class Anchor {
	public function set_m_node(node: Node): Void;
	public function get_m_node(): Node;
	public function set_m_local(local: Vector3): Void;
	public function get_m_local(): Vector3;
	public function set_m_body(body: RigidBody): Void;
	public function get_m_body(): RigidBody;
	public function set_m_influence(influence: Float): Void;
	public function get_m_influence(): Float;
	public function set_m_c1(c1: Vector3): Void;
	public function get_m_c1(): Vector3;
	public function set_m_c2(c2: Float): Void;
	public function get_m_c2(): Float;
}

@:native("Ammo.btSoftBody")
extern class SoftBody extends CollisionObject {
	public function get_m_nodes(): TNodeArray;
	public function get_m_cfg(): Config;
	public function get_m_materials(): TMaterialArray;
	public function get_m_anchors(): TAnchorArray;
	public function setTotalMass(mass: Float, fromfaces: Bool = false): Void;
	public function generateClusters(k: Int, maxiterations: Int = 8192): Void;
	public function generateBendingConstraints(distance: Int, mat: Dynamic = 0): Void;
	public function appendAnchor(node: Int, body: RigidBody, disableCollisionBetweenLinkedBodies: Bool, influence: Float): Void;
	public function appendLink(node: Node, node1: Node, mat: Material, bcheckexist: Bool = false): Void;
	public function addForce(f: Vector3, node: Int): Void;
}

@:native("Ammo.btSoftBodyHelpers")
extern class SoftBodyHelpers {
	public function new(): Void;
	function CreateFromTriMesh(worldInfo: SoftBodyWorldInfo, vertices: haxe.ds.Vector<Float>, triangles: haxe.ds.Vector<Int>, ntriangles: Int, randomizeConstraints: Bool = true): SoftBody;
}

@:native("Ammo.btOverlappingPairCallback")
extern class OverlappingPairCallback {}

@:native("Ammo.btGhostPairCallback")
extern class GhostPairCallback extends OverlappingPairCallback {
	public function new(): Void;
}

@:native("Ammo.btOverlappingPairCache")
extern class OverlappingPairCache {
	public function setInternalGhostPairCallback(ghostPairCallback: OverlappingPairCallback): Void;
}

@:native("Ammo.btGhostObject")
extern class GhostObject extends CollisionObject {
	public function new(): Void;
	public function getNumOverlappingObjects(): Int;
	public function getOverlappingObject(index: Int): CollisionObject;
}

@:native("Ammo.btPairCachingGhostObject")
extern class PairCachingGhostObject extends GhostObject {
	public function new(): Void;
}

@:native("Ammo.btKinematicCharacterController")
extern class KinematicCharacterController extends ActionInterface {
	public function new(ghostObject: PairCachingGhostObject, convexShape: ConvexShape, stepHeight: Float, upAxis: Int = 1): Void;
	public function setUpAxis(axis: Int): Void; // setUp in cpp
	public function jump(): Void;
	public function setGravity(gravity: Float): Void;
	public function getGravity(): Float;
	public function canJump(): Bool;
	public function onGround(): Bool;
	public function setWalkDirection(walkDirection: Vector3): Void;
	public function setVelocityForTimeInterval(velocity: Vector3, timeInterval: Float): Void;
	public function warp(origin: Vector3): Void;
	public function preStep(collisionWorld: CollisionWorld): Void;
	public function playerStep(collisionWorld: CollisionWorld, dt: Float): Void;
	public function setFallSpeed(fallSpeed: Float): Void;
	public function setJumpSpeed(jumpSpeed: Float): Void;
	public function setMaxJumpHeight(maxJumpHeight: Float): Void;
	public function setMaxSlope(slopeRadians: Float): Void;
	public function getMaxSlope(): Float;
	public function getGhostObject(): PairCachingGhostObject;
	public function setUseGhostSweepTest(useGhostObjectSweepTest: Bool): Void;
	public function setUpInterpolate(value: Bool): Void;
}

@:native("Ammo")
extern class Ammo {
	public static function destroy(obj: Dynamic): Void;
	public static function castObject(obj: Dynamic, to: Class<Dynamic>): Dynamic;
	public static function _malloc(byteSize: Int): Int;
	public static var HEAPU8: Dynamic;
	public static var HEAPF32: Dynamic;
}
