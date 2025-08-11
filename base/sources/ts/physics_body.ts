
///if arm_physics

///include "../libs/asim.h"

declare function asim_body_create(shape: i32, mass: f32, dimx: f32, dimy: f32, dimz: f32, x: f32, y: f32, z: f32, triangles: f32[]): any;
declare function asim_body_apply_impulse(body: any, x: f32, y: f32, z: f32): void;
declare function asim_body_get_pos(body: any, pos: vec4_t): void; // vec4_ptr_t
declare function asim_body_get_rot(body: any, rot: quat_t): void; // quat_ptr_t
declare function asim_body_sync_transform(body: any, pos: vec4_t, rot: quat_t): void;
declare function asim_body_remove(body: any): void;

type physics_body_t = {
	_body?: any;
	shape?: physics_shape_t;
	mass?: f32;
	dimx?: f32;
	dimy?: f32;
	dimz?: f32;
	obj?: object_t;
};

enum physics_shape_t {
	BOX = 0,
	SPHERE = 1,
	HULL = 2,
	TERRAIN = 3,
	MESH = 4,
}

let physics_body_object_map: map_t<i32, physics_body_t> = map_create();

function physics_body_create(): physics_body_t {
	let body: physics_body_t = {};
	return body;
}

function physics_body_init(body: physics_body_t, obj: object_t) {
	map_set(physics_body_object_map, obj.uid, body);

	body.obj = obj;
	transform_compute_dim(obj.transform);
	body.dimx = obj.transform.dim.x;
	body.dimy = obj.transform.dim.y;
	body.dimz = obj.transform.dim.z;

	let scale_pos: f32 = 1.0;
	let posa: i16[] = null;
	let inda: u32[] = null;

	if (body.shape == physics_shape_t.MESH ||
		body.shape == physics_shape_t.HULL ||
		body.shape == physics_shape_t.TERRAIN
	) {
		let mo: mesh_object_t = obj.ext;
		let data: mesh_data_t = mo.data;
		let scale: vec4_t = obj.transform.scale;

		let positions: i16_array_t = mesh_data_get_vertex_array(data, "pos").values;
		let indices0: u32_array_t = data.index_arrays[0].values;

		scale_pos = scale.x * data.scale_pos;
		posa = positions;
		inda = indices0;

		// triangles = f32_array_create(indices0.length * 3);
		// let ar: u32_array_t = indices[0];
		// for (let i: i32 = 0; i < math_floor(ar.length / 3); ++i) {
		// 	triangles[i * 9    ] = positions[ar[i * 3    ] * 4    ] * sx;
		// 	triangles[i * 9 + 1] = positions[ar[i * 3    ] * 4 + 1] * sy;
		// 	triangles[i * 9 + 2] = positions[ar[i * 3    ] * 4 + 2] * sz;
		// 	triangles[i * 9 + 3] = positions[ar[i * 3 + 1] * 4    ] * sx;
		// 	triangles[i * 9 + 4] = positions[ar[i * 3 + 1] * 4 + 1] * sy;
		// 	triangles[i * 9 + 5] = positions[ar[i * 3 + 1] * 4 + 2] * sz;
		// 	triangles[i * 9 + 6] = positions[ar[i * 3 + 2] * 4    ] * sx;
		// 	triangles[i * 9 + 7] = positions[ar[i * 3 + 2] * 4 + 1] * sy;
		// 	triangles[i * 9 + 8] = positions[ar[i * 3 + 2] * 4 + 2] * sz;
		// }
	}

	let loc: vec4_t = obj.transform.loc;
	body._body = asim_body_create(body.shape, body.mass, body.dimx, body.dimy, body.dimz, loc.x, loc.y, loc.z, posa, inda, scale_pos);
}

function physics_body_remove(uid: i32) {
	let body: physics_body_t = map_get(physics_body_object_map, uid);
	if (body == null) {
		return;
	}
	map_delete(physics_body_object_map, uid);
	asim_body_remove(body._body);
}

function physics_body_apply_impulse(body: physics_body_t, dir: vec4_t) {
	asim_body_apply_impulse(body._body, dir.x, dir.y, dir.z);
}

function physics_body_sync_transform(body: physics_body_t) {
	let transform: transform_t = body.obj.transform;
	asim_body_sync_transform(body._body, transform.loc, transform.rot);
}

function physics_body_update(body: physics_body_t) {
	if (body.shape == physics_shape_t.MESH) {
		return; ////
	}

	let transform: transform_t = body.obj.transform;
	asim_body_get_pos(body._body, ADDRESS(transform.loc));
	asim_body_get_rot(body._body, ADDRESS(transform.rot));
	transform_build_matrix(transform);
}

///end
