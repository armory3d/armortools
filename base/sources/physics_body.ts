
///if arm_physics

///include <phys_jolt.h>

declare function jolt_body_create(shape: i32, mass: f32, dimx: f32, x: f32, y: f32, z: f32, triangles: f32[]): any;
declare function jolt_body_apply_impulse(body: any, x: f32, y: f32, z: f32): void;
declare function jolt_body_get_pos(body: any, pos: vec4_t): void;
declare function jolt_body_get_rot(body: any, rot: quat_t): void;

type physics_body_t = {
	_body?: any;
	shape?: physics_shape_t;
	mass?: f32;
	dimx?: f32;
	obj?: object_t;
};

enum physics_shape_t {
	SPHERE = 0,
	MESH = 1,
}

let physics_body_object_map: map_t<i32, physics_body_t> = map_create();

function physics_body_create(): physics_body_t {
	let body: physics_body_t = {};
	return body;
}

function physics_body_init(body: physics_body_t, obj: object_t) {
	map_set(physics_body_object_map, obj.uid, body);

	body.obj = obj;
	body.dimx = obj.transform.dim.x;

	let triangles: f32[] = null;
	if (body.shape == physics_shape_t.MESH) {
		let mo: mesh_object_t = obj.ext;
		let data: mesh_data_t = mo.data;
		let scale: vec4_t = obj.transform.scale;

		let positions: i16_array_t = mesh_data_get_vertex_array(data, "pos").values;
		let indices: u32_array_t[] = data._.indices;
		let indices0: u32_array_t = indices[0];
		let sx: f32 = scale.x * (1 / 32767);
		let sy: f32 = scale.y * (1 / 32767);
		let sz: f32 = scale.z * (1 / 32767);
		sx *= data.scale_pos;
		sy *= data.scale_pos;
		sz *= data.scale_pos;

		triangles = f32_array_create(indices0.length * 3);

		// for (let i: i32 = 0; i < indices.length; ++i) {
			let ar: u32_array_t = indices[0];
			for (let i: i32 = 0; i < math_floor(ar.length / 3); ++i) {
				triangles[i * 9    ] = positions[ar[i * 3    ] * 4    ] * sx;
				triangles[i * 9 + 1] = positions[ar[i * 3    ] * 4 + 1] * sy;
				triangles[i * 9 + 2] = positions[ar[i * 3    ] * 4 + 2] * sz;
				triangles[i * 9 + 3] = positions[ar[i * 3 + 1] * 4    ] * sx;
				triangles[i * 9 + 4] = positions[ar[i * 3 + 1] * 4 + 1] * sy;
				triangles[i * 9 + 5] = positions[ar[i * 3 + 1] * 4 + 2] * sz;
				triangles[i * 9 + 6] = positions[ar[i * 3 + 2] * 4    ] * sx;
				triangles[i * 9 + 7] = positions[ar[i * 3 + 2] * 4 + 1] * sy;
				triangles[i * 9 + 8] = positions[ar[i * 3 + 2] * 4 + 2] * sz;
			}
		// }
	}

	let loc: vec4_t = obj.transform.loc;
	body._body = jolt_body_create(body.shape, body.mass, body.dimx, loc.x, loc.y, loc.z, triangles);
}

function physics_body_apply_impulse(body: physics_body_t, dir: vec4_t) {
	jolt_body_apply_impulse(body._body, dir.x, dir.y, dir.z);
}

function physics_body_sync_transform(body: physics_body_t) {
	//
}

function physics_body_update(body: physics_body_t) {
	let transform: transform_t = body.obj.transform;
	jolt_body_get_pos(body._body, ADDRESS(transform.loc));
	jolt_body_get_rot(body._body, ADDRESS(transform.rot));
	transform_build_matrix(transform);
}

///end
