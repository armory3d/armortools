
type mesh_object_t = {
	base?: object_t;
	data?: mesh_data_t;
	materials?: material_data_t[];
	camera_dist?: f32;
	frustum_culling?: bool;
	skip_context?: string; // Do not draw this context
	force_context?: string; // Draw only this context
};

let _mesh_object_last_pipeline: gpu_pipeline_t = null;

function mesh_object_create(data: mesh_data_t, materials: material_data_t[]): mesh_object_t {
	let raw: mesh_object_t = {};
	raw.frustum_culling = true;
	raw.base = object_create(false);
	raw.base.ext = raw;
	raw.base.ext_type = "mesh_object_t";

	raw.materials = materials;
	mesh_object_set_data(raw, data);
	array_push(scene_meshes, raw);
	return raw;
}

function mesh_object_set_data(raw: mesh_object_t, data: mesh_data_t) {
	raw.data = data;
	if (data._.vertex_buffer == null) {
		mesh_data_build(data);
	}

	// Scale-up packed (-1,1) mesh coords
	raw.base.transform.scale_world = data.scale_pos;
}

function mesh_object_remove(raw: mesh_object_t) {
	array_remove(scene_meshes, raw);
	object_remove_super(raw.base);
}

function mesh_object_setup_animation(raw: mesh_object_t, oactions: scene_t[] = null) {
	///if arm_anim
	object_setup_animation_super(raw.base, oactions);
	///end
}

function mesh_object_cull_material(raw: mesh_object_t, context: string): bool {
	// Skip render if material does not contain current context
	if (!mesh_object_valid_context(raw, raw.materials, context)) {
		raw.base.culled = true;
		return true;
	}
	if (raw.skip_context == context) {
		raw.base.culled = true;
		return true;
	}
	if (raw.force_context != null && raw.force_context != context) {
		raw.base.culled = true;
		return true;
	}
	raw.base.culled = false;
	return false;
}

function mesh_object_cull_mesh(raw: mesh_object_t, context: string, camera: camera_object_t): bool {
	if (camera.data.frustum_culling && raw.frustum_culling) {
		let radius_scale: f32 = 1.0;
		let frustum_planes: frustum_plane_t[] = camera.frustum_planes;
		if (!camera_object_sphere_in_frustum(frustum_planes, raw.base.transform, radius_scale)) {
			raw.base.culled = true;
			return true;
		}
	}
	raw.base.culled = false;
	return false;
}

function mesh_object_render(raw: mesh_object_t, context: string, bind_params: string[]) {
	if (!raw.base.visible) {
		return; // Skip render if object is hidden
	}
	if (mesh_object_cull_mesh(raw, context, scene_camera)) {
		return;
	}
	if (mesh_object_cull_material(raw, context)) {
		return;
	}

	uniforms_pos_unpack = raw.data.scale_pos;
	uniforms_tex_unpack = raw.data.scale_tex;
	transform_update(raw.base.transform);

	let scontext: shader_context_t = null;
	let mcontext: material_context_t = null;
	for (let i: i32 = 0; i < raw.materials.length; ++i) {
		let mat: material_data_t = raw.materials[i];
		for (let j: i32 = 0; j < mat.contexts.length; ++j) {
			if (mat.contexts[j].name == context) {
				scontext = shader_data_get_context(mat._.shader, context);
				mcontext = mat.contexts[j];
				break;
			}
		}
	}

	if (scontext._.pipe != _mesh_object_last_pipeline) {
		gpu_set_pipeline(scontext._.pipe);
		_mesh_object_last_pipeline = scontext._.pipe;
	}
	uniforms_set_context_consts(scontext, bind_params);
	uniforms_set_obj_consts(scontext, raw.base);
	uniforms_set_material_consts(scontext, mcontext);
	gpu_set_vertex_buffer(raw.data._.vertex_buffer);
	gpu_set_index_buffer(raw.data._.index_buffer);
	gpu_draw();
}

function mesh_object_valid_context(raw: mesh_object_t, mats: material_data_t[], context: string): bool {
	for (let i: i32 = 0; i < mats.length; ++i) {
		let mat: material_data_t = mats[i];
		if (material_data_get_context(mat, context) != null) {
			return true;
		}
	}
	return false;
}

function mesh_object_compute_camera_dist(raw: mesh_object_t, cam_x: f32, cam_y: f32, cam_z: f32) {
	// Render path mesh sorting
	raw.camera_dist = vec4_fdist(cam_x, cam_y, cam_z, transform_world_x(raw.base.transform), transform_world_y(raw.base.transform), transform_world_z(raw.base.transform));
}
