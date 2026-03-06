
#ifdef arm_physics

physics_body_t *physics_body_create() {
	physics_body_t *body = GC_ALLOC_INIT(physics_body_t, {0});
	return body;
}

void physics_body_init(physics_body_t *body, object_t *obj) {
	any_imap_set(physics_body_object_map, obj->uid, body);

	body->obj = obj;
	transform_compute_dim(obj->transform);
	body->dimx             = obj->transform->dim.x;
	body->dimy             = obj->transform->dim.y;
	body->dimz             = obj->transform->dim.z;

	f32          scale_pos = 1.0;
	i16_array_t *posa      = null;
	u32_array_t *inda      = null;

	if (body->shape == PHYSICS_SHAPE_MESH || body->shape == PHYSICS_SHAPE_HULL || body->shape == PHYSICS_SHAPE_TERRAIN) {
		mesh_object_t *mo        = obj->ext;
		mesh_data_t   *data      = mo->data;
		vec4_t         scale     = obj->transform->scale;

		i16_array_t   *positions = mesh_data_get_vertex_array(data, "pos")->values;
		u32_array_t   *indices0  = data->index_array;

		scale_pos                = scale.x * data->scale_pos;
		posa                     = positions;
		inda                     = indices0;

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

	vec4_t loc  = obj->transform->loc;
	body->_body = asim_body_create(body->shape, body->mass, body->dimx, body->dimy, body->dimz, loc.x, loc.y, loc.z, posa, inda, scale_pos);
}

void physics_body_remove(i32 uid) {
	physics_body_t *body = any_imap_get(physics_body_object_map, uid);
	if (body == null) {
		return;
	}
	imap_delete(physics_body_object_map, uid);
	asim_body_remove(body->_body);
}

void physics_body_apply_impulse(physics_body_t *body, vec4_t dir) {
	asim_body_apply_impulse(body->_body, dir.x, dir.y, dir.z);
}

void physics_body_sync_transform(physics_body_t *body) {
	transform_t *transform = body->obj->transform;
	asim_body_sync_transform(body->_body, transform->loc, transform->rot);
}

void physics_body_update(physics_body_t *body) {
	if (body->shape == PHYSICS_SHAPE_MESH) {
		return; ////
	}

	transform_t *transform = body->obj->transform;
	asim_body_get_pos(body->_body, ADDRESS(transform->loc));
	asim_body_get_rot(body->_body, ADDRESS(transform->rot));
	transform_build_matrix(transform);
}

#endif
