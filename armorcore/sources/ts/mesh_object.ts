
type mesh_object_t = {
	base?: object_t;
	data?: mesh_data_t;
	materials?: material_data_t[];
	material_index?: i32;
	///if arm_particles
	particle_systems?: particle_sys_t[]; // Particle owner
	particle_children?: mesh_object_t[];
	particle_owner?: mesh_object_t; // Particle object
	particle_index?: i32;
	///end
	camera_dist?: f32;
	screen_size?: f32;
	frustum_culling?: bool;
	skip_context?: string; // Do not draw this context
	force_context?: string; // Draw only this context
	prev_matrix?: mat4_t;
};

let _mesh_object_last_pipeline: pipeline_t = null;
let _mesh_object_material_contexts: material_context_t[] = [];
let _mesh_object_shader_contexts: shader_context_t[] = [];

function mesh_object_create(data: mesh_data_t, materials: material_data_t[]): mesh_object_t {
	let raw: mesh_object_t = {};
	raw.material_index = 0;
	///if arm_particles
	raw.particle_index = -1;
	///end
	raw.screen_size = 0.0;
	raw.frustum_culling = true;
	raw.prev_matrix = mat4_identity();
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
	data._.refcount++;
	mesh_data_build(data);

	// Scale-up packed (-1,1) mesh coords
	raw.base.transform.scale_world = data.scale_pos;
}

function mesh_object_remove(raw: mesh_object_t) {
	///if arm_particles
	if (raw.particle_children != null) {
		for (let i: i32 = 0; i < raw.particle_children.length; ++i) {
			let c: mesh_object_t = raw.particle_children[i];
			mesh_object_remove(c);
		}
		raw.particle_children = null;
	}
	if (raw.particle_systems != null) {
		for (let i: i32 = 0; i < raw.particle_systems.length; ++i) {
			let psys: particle_sys_t = raw.particle_systems[i];
			particle_sys_remove(psys);
		}
		raw.particle_systems = null;
	}
	///end
	array_remove(scene_meshes, raw);
	raw.data._.refcount--;

	object_remove_super(raw.base);
}

function mesh_object_setup_animation(raw: mesh_object_t, oactions: scene_t[] = null) {
	///if arm_skin
	let has_action: bool = raw.base.parent != null && raw.base.parent.raw != null && raw.base.parent.raw.anim != null && raw.base.parent.raw.anim.bone_actions != null;
	if (has_action) {
		let armature_name: string = raw.base.parent.name;
		raw.base.animation = object_get_parent_armature(raw.base, armature_name).base;
		if (raw.base.animation == null) {
			raw.base.animation = anim_bone_create(armature_name).base;
		}
		if (raw.data.skin != null) {
			anim_bone_set_skin(raw.base.animation.ext, raw);
		}
	}
	///end

	///if arm_anim
	object_setup_animation_super(raw.base, oactions);
	///end
}

///if arm_particles
function mesh_object_setup_particle_system(raw: mesh_object_t, scene_name: string, pref: particle_ref_t) {
	if (raw.particle_systems == null) {
		raw.particle_systems = [];
	}
	let psys: particle_sys_t = particle_sys_create(scene_name, pref);
	array_push(raw.particle_systems, psys);
}
///end

function mesh_object_set_culled(raw: mesh_object_t, b: bool): bool {
	raw.base.culled = b;
	return b;
}

function mesh_object_cull_material(raw: mesh_object_t, context: string): bool {
	// Skip render if material does not contain current context
	if (!mesh_object_valid_context(raw, raw.materials, context)) {
		return true;
	}

	if (!raw.base.visible) {
		return mesh_object_set_culled(raw, true);
	}

	if (raw.skip_context == context) {
		return mesh_object_set_culled(raw, true);
	}

	if (raw.force_context != null && raw.force_context != context) {
		return mesh_object_set_culled(raw, true);
	}

	return mesh_object_set_culled(raw, false);
}

function mesh_object_cull_mesh(raw: mesh_object_t, context: string, camera: camera_object_t, light: light_object_t): bool {
	if (camera == null) {
		return false;
	}

	if (camera.data.frustum_culling && raw.frustum_culling) {
		// Scale radius for skinned mesh and particle system
		// TODO: define skin & particle bounds
		let radius_scale: f32 = raw.data.skin != null ? 2.0 : 1.0;
		///if arm_particles
		// particle_systems for update, particle_owner for render
		if (raw.particle_systems != null || raw.particle_owner != null) {
			radius_scale *= 1000;
		}
		///end
		if (context == "voxel") {
			radius_scale *= 100;
		}
		if (raw.data._.instanced) {
			radius_scale *= 100;
		}

		let frustum_planes: frustum_plane_t[] = camera.frustum_planes;
		if (!camera_object_sphere_in_frustum(frustum_planes, raw.base.transform, radius_scale)) {
			return mesh_object_set_culled(raw, true);
		}
	}

	raw.base.culled = false;
	return raw.base.culled;
}

function mesh_object_get_contexts(raw: mesh_object_t, context: string, materials: material_data_t[], material_contexts: material_context_t[], shader_contexts: shader_context_t[]) {
	for (let i: i32 = 0; i < materials.length; ++i) {
		let mat: material_data_t = materials[i];
		let found: bool = false;
		for (let j: i32 = 0; j < mat.contexts.length; ++j) {
			if (substring(mat.contexts[j].name, 0, context.length) == context) {
				array_push(material_contexts, mat._.contexts[j]);
				array_push(shader_contexts, shader_data_get_context(mat._.shader, context));
				found = true;
				break;
			}
		}
		if (!found) {
			array_push(material_contexts, null);
			array_push(shader_contexts, null);
		}
	}
}

function mesh_object_render(raw: mesh_object_t, context: string, bind_params: string[]) {
	if (raw.data == null || !raw.data._.ready) {
		return; // Data not yet streamed
	}
	if (!raw.base.visible) {
		return; // Skip render if object is hidden
	}
	if (mesh_object_cull_mesh(raw, context, scene_camera, _render_path_light)) {
		return;
	}
	let mesh_context: bool = raw.base.raw != null ? context == "mesh" : false;

	///if arm_particles
	if (raw.base.raw != null && raw.base.raw.particles != null && raw.base.raw.particles.is_particle && raw.particle_owner == null) {
		return; // Instancing not yet set-up by particle system owner
	}
	if (raw.particle_systems != null && mesh_context) {
		if (raw.particle_children == null) {
			raw.particle_children = [];
			for (let i: i32 = 0; i < raw.particle_systems.length; ++i) {
				let psys: particle_sys_t = raw.particle_systems[i];
				// let c: mesh_object_t = scene_get_child(psys.data.raw.instance_object);
				let o: object_t = scene_spawn_object(psys.data.instance_object);
				if (o != null) {
					let c: mesh_object_t = o.ext;
					array_push(raw.particle_children, c);
					c.particle_owner = raw;
					c.particle_index = raw.particle_children.length - 1;
				}
			}
		}
		for (let i: i32 = 0; i < raw.particle_systems.length; ++i) {
			particle_sys_update(raw.particle_systems[i], raw.particle_children[i], raw);
		}
	}
	if (raw.particle_systems != null && raw.particle_systems.length > 0 && raw.base.raw.particles != null && !raw.base.raw.particles.render_emitter) {
		return;
	}
	///end

	if (mesh_object_cull_material(raw, context)) {
		return;
	}

	// Get context
	let material_contexts: material_context_t[] = _mesh_object_material_contexts;
	let shader_contexts: shader_context_t[] = _mesh_object_shader_contexts;
	material_contexts.length = 0;
	shader_contexts.length = 0;
	mesh_object_get_contexts(raw, context, raw.materials, material_contexts, shader_contexts);

	uniforms_pos_unpack = raw.data.scale_pos;
	uniforms_tex_unpack = raw.data.scale_tex;
	transform_update(raw.base.transform);

	// Render mesh
	for (let i: i32 = 0; i < raw.data._.index_buffers.length; ++i) {

		let mi: i32 = raw.data._.material_indices[i];
		if (shader_contexts.length <= mi || shader_contexts[mi] == null) {
			continue;
		}
		raw.material_index = mi;

		let scontext: shader_context_t = shader_contexts[mi];
		if (scontext == null) {
			continue;
		}
		let elems: vertex_element_t[] = scontext.vertex_elements;

		// Uniforms
		if (scontext._.pipe_state != _mesh_object_last_pipeline) {
			g4_set_pipeline(scontext._.pipe_state);
			_mesh_object_last_pipeline = scontext._.pipe_state;
		}
		uniforms_set_context_consts(scontext, bind_params);
		uniforms_set_obj_consts(scontext, raw.base);
		if (material_contexts.length > mi) {
			uniforms_set_material_consts(scontext, material_contexts[mi]);
		}

		// VB / IB
		if (raw.data._.instanced_vb != null) {
			let vb: vertex_buffer_t = mesh_data_get(raw.data, elems);
			let vbs: vertex_buffer_t[] = [vb.buffer_, raw.data._.instanced_vb.buffer_];
			g4_set_vertex_buffers(vbs);
		}
		else {
			g4_set_vertex_buffer(mesh_data_get(raw.data, elems));
		}

		g4_set_index_buffer(raw.data._.index_buffers[i]);

		// Draw
		if (raw.data._.instanced) {
			g4_draw_inst(raw.data._.instance_count, 0, -1);
		}
		else {
			g4_draw(0, -1);
		}
	}

	raw.prev_matrix = mat4_clone(raw.base.transform.world_unpack);
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

function mesh_object_compute_screen_size(raw: mesh_object_t, camera: camera_object_t) {
	// Approx..
	// let rp = camera_render_path;
	// let screen_volume = rp.current_w * rp.current_h;
	let tr: transform_t = raw.base.transform;
	let volume: f32 = tr.dim.x * tr.dim.y * tr.dim.z;
	raw.screen_size = volume * (1.0 / raw.camera_dist);
	raw.screen_size = raw.screen_size > 1.0 ? 1.0 : raw.screen_size;
}
