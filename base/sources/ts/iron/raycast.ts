
type ray_t = {
	origin?: vec4_t;
	dir?: vec4_t;
};

type plane_t = {
	normal?: vec4_t;
	constant?: f32;
};

let _raycast_vp_inv: mat4_t = mat4_identity();
let _raycast_p_inv: mat4_t = mat4_identity();
let _raycast_v_inv: mat4_t = mat4_identity();

function raycast_get_ray(input_x: f32, input_y: f32, camera: camera_object_t): ray_t {
	let start: vec4_t = vec4_create();
	let end: vec4_t = vec4_create();

	// Get 3D point form screen coords
	// Set two vectors with opposing z values
	start.x = (input_x / app_w()) * 2.0 - 1.0;
	start.y = -((input_y / app_h()) * 2.0 - 1.0);
	start.z = -1.0;
	end.x = start.x;
	end.y = start.y;
	end.z = 1.0;

	_raycast_p_inv = mat4_inv(camera.p);
	_raycast_v_inv = mat4_inv(camera.v);
	_raycast_vp_inv = mat4_mult_mat(_raycast_p_inv, _raycast_v_inv);
	start = vec4_apply_proj(start, _raycast_vp_inv);
	end = vec4_apply_proj(end, _raycast_vp_inv);

	// Find direction from start to end
	end = vec4_sub(end, start);
	end = vec4_norm(end);
	end.x *= camera.data.far_plane;
	end.y *= camera.data.far_plane;
	end.z *= camera.data.far_plane;

	return ray_create(start, end);
}

function raycast_box_intersect(transform: transform_t, input_x: f32, input_y: f32, camera: camera_object_t): vec4_t {
	let ray: ray_t = raycast_get_ray(input_x, input_y, camera);

	let t: transform_t = transform;
	let c: vec4_t = vec4_create(transform_world_x(t), transform_world_y(t), transform_world_z(t));
	let s: vec4_t = vec4_create(t.dim.x, t.dim.y, t.dim.z);
	return ray_intersect_box(ray, c, s);
}

function raycast_closest_box_intersect(transforms: transform_t[], input_x: f32, input_y: f32, camera: camera_object_t): transform_t {
	let intersects: transform_t[] = [];

	// Get intersects
	for (let i: i32 = 0; i < transforms.length; ++i) {
		let t: transform_t = transforms[i];
		let intersect: vec4_t = raycast_box_intersect(t, input_x, input_y, camera);
		if (!vec4_isnan(intersect)) {
			array_push(intersects, t);
		}
	}

	// No intersects
	if (intersects.length == 0) {
		return null;
	}

	// Get closest intersect
	let closest: transform_t = null;
	let inf: f32 = 100000;
	let min_dist: f32 = inf;
	for (let i: i32 = 0; i < intersects.length; ++i) {
		let t: transform_t = intersects[i];
		let dist: f32 = vec4_dist(t.loc, camera.base.transform.loc);
		if (dist < min_dist) {
			min_dist = dist;
			closest = t;
		}
	}

	return closest;
}

function plane_create(): plane_t {
	let raw: plane_t = {};
	raw.normal = vec4_create(1.0, 0.0, 0.0);
	raw.constant = 0.0;
	return raw;
}

function raycast_plane_intersect(normal: vec4_t, a: vec4_t, input_x: f32, input_y: f32, camera: camera_object_t): vec4_t {
	let ray: ray_t = raycast_get_ray(input_x, input_y, camera);

	let plane: plane_t = plane_create();
	plane_set(plane, normal, a);

	return ray_intersect_plane(ray, plane);
}

function ray_create(origin: vec4_t, dir: vec4_t): ray_t {
	let raw: ray_t = {};
	raw.origin = origin;
	raw.dir = dir;
	return raw;
}

function ray_at(raw: ray_t, t: f32): vec4_t {
	return vec4_add(vec4_mult(raw.dir, t), raw.origin);
}

function ray_dist_to_point(raw: ray_t, point: vec4_t): f32 {
	let v1: vec4_t = vec4_create();
	let dir_dist: f32 = vec4_dot(vec4_sub(point, raw.origin), raw.dir);

	// Point behind the ray
	if (dir_dist < 0) {
		return vec4_dist(raw.origin, point);
	}

	raw.dir = vec4_mult(raw.dir, dir_dist);
	raw.dir = vec4_add(raw.dir, raw.origin);

	return vec4_dist(v1, point);
}

function ray_intersects_sphere(raw: ray_t, sphere_center: vec4_t, sphere_radius: f32): bool {
	return ray_dist_to_point(raw, sphere_center) <= sphere_radius;
}

function ray_intersects_plane(raw: ray_t, plane: plane_t): bool {
	// Check if the ray lies on the plane first
	let dist_to_point: f32 = plane_dist_to_point(plane, raw.origin);
	if (dist_to_point == 0) {
		return true;
	}

	let denominator: f32 = vec4_dot(plane.normal, raw.dir);
	if (denominator * dist_to_point < 0) {
		return true;
	}

	// Ray origin is behind the plane (and is pointing behind it)
	return false;
}

function ray_dist_to_plane(raw: ray_t, plane: plane_t): f32 {
	let denominator: f32 = vec4_dot(plane.normal, raw.dir);
	if (denominator == 0) {
		// Line is coplanar, return origin
		if (plane_dist_to_point(plane, raw.origin) == 0) {
			return 0;
		}

		return -1;
	}

	let t: f32 = -(vec4_dot(raw.origin, plane.normal) + plane.constant) / denominator;

	// Return if the ray never intersects the plane
	return t >= 0 ? t : -1;
}

function ray_intersect_plane(raw: ray_t, plane: plane_t): vec4_t {
	let t: f32 = ray_dist_to_plane(raw, plane);
	if (t == -1) {
		return vec4_nan();
	}
	return ray_at(raw, t);
}

function ray_intersect_box(raw: ray_t, center: vec4_t, dim: vec4_t): vec4_t {
	// http://www.scratchapixel.com/lessons/3d-basic-lessons/lesson-7-intersecting-simple-shapes/ray-box-intersection/
	let tmin: f32;
	let tmax: f32;
	let tymin: f32;
	let tymax: f32;
	let tzmin: f32;
	let tzmax: f32;

	let half_x: f32 = dim.x / 2;
	let half_y: f32 = dim.y / 2;
	let half_z: f32 = dim.z / 2;
	let box_min_x: f32 = center.x - half_x;
	let box_min_y: f32 = center.y - half_y;
	let box_min_z: f32 = center.z - half_z;
	let box_max_x: f32 = center.x + half_x;
	let box_max_y: f32 = center.y + half_y;
	let box_max_z: f32 = center.z + half_z;

	let invdirx: f32 = 1 / raw.dir.x;
	let	invdiry: f32 = 1 / raw.dir.y;
	let invdirz: f32 = 1 / raw.dir.z;

	let origin: vec4_t = raw.origin;

	if (invdirx >= 0) {
		tmin = (box_min_x - origin.x) * invdirx;
		tmax = (box_max_x - origin.x) * invdirx;
	}
	else {
		tmin = (box_max_x - origin.x) * invdirx;
		tmax = (box_min_x - origin.x) * invdirx;
	}

	if (invdiry >= 0) {
		tymin = (box_min_y - origin.y) * invdiry;
		tymax = (box_max_y - origin.y) * invdiry;
	}
	else {
		tymin = (box_max_y - origin.y) * invdiry;
		tymax = (box_min_y - origin.y) * invdiry;
	}

	if ((tmin > tymax) || (tymin > tmax)) {
		return vec4_nan();
	}

	// These lines also handle the case where tmin or tmax is nan
	// (result of 0 * inf). x !== x returns true if x is nan
	if (tymin > tmin || tmin != tmin) {
		tmin = tymin;
	}
	if (tymax < tmax || tmax != tmax) {
		tmax = tymax;
	}

	if (invdirz >= 0) {
		tzmin = (box_min_z - origin.z) * invdirz;
		tzmax = (box_max_z - origin.z) * invdirz;
	}
	else {
		tzmin = (box_max_z - origin.z) * invdirz;
		tzmax = (box_min_z - origin.z) * invdirz;
	}

	if ((tmin > tzmax) || (tzmin > tmax)) {
		return vec4_nan();
	}
	if (tzmin > tmin || tmin != tmin) {
		tmin = tzmin;
	}
	if (tzmax < tmax || tmax != tmax) {
		tmax = tzmax;
	}

	// Return point closest to the ray (positive side)
	if (tmax < 0) {
		return vec4_nan();
	}

	return ray_at(raw, tmin >= 0 ? tmin : tmax);
}

function ray_intersect_triangle(raw: ray_t, a: vec4_t, b: vec4_t, c: vec4_t, cull_backface: bool): vec4_t {
	// Compute the offset origin, edges, and normal
	let diff: vec4_t = vec4_create();
	let edge1: vec4_t = vec4_create();
	let edge2: vec4_t = vec4_create();
	let normal: vec4_t = vec4_create();

	// from http://www.geometrictools.com/LibMathematics/Intersection/Wm5IntrRay3Triangle3.cpp
	edge1 = vec4_sub(b, a);
	edge2 = vec4_sub(c, a);
	normal = vec4_cross(edge1, edge2);

	let ddn: f32 = vec4_dot(raw.dir, normal);
	let sign: i32;

	if (ddn > 0) {
		if (cull_backface) {
			return vec4_nan();
		}
		sign = 1;
	}
	else if (ddn < 0) {
		sign = -1;
		ddn = -ddn;
	}
	else {
		return vec4_nan();
	}

	diff = vec4_sub(raw.origin, a);
	let ddqxe2: f32 = sign * vec4_dot(raw.dir, vec4_cross(diff, edge2));

	// b1 < 0, no intersection
	if (ddqxe2 < 0) {
		return vec4_nan();
	}

	let dde1xq: f32 = sign * vec4_dot(raw.dir, vec4_cross(edge1, diff));

	// b2 < 0, no intersection
	if (dde1xq < 0) {
		return vec4_nan();
	}

	// b1+b2 > 1, no intersection
	if (ddqxe2 + dde1xq > ddn) {
		return vec4_nan();
	}

	// Line intersects triangle, check if ray does.
	let qdn: f32 = -sign * vec4_dot(diff, normal);

	// t < 0, no intersection
	if (qdn < 0) {
		return vec4_nan();
	}

	// Ray intersects triangle.
	return ray_at(raw, qdn / ddn);
}

function plane_dist_to_point(raw: plane_t, point: vec4_t): f32 {
	return vec4_dot(raw.normal, point) + raw.constant;
}

function plane_set(raw: plane_t, normal: vec4_t, point: vec4_t): plane_t {
	raw.normal = vec4_clone(normal);
	raw.constant = -vec4_dot(point, raw.normal);
	return raw;
}
