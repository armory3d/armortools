
let scene_camera: camera_object_t;
let scene_world: world_data_t;
let scene_meshes: mesh_object_t[];
let scene_cameras: camera_object_t[];
/// if arm_audio
let scene_speakers: speaker_object_t[];
/// end
let scene_empties: object_t[];
/// if arm_anim
let scene_animations: anim_raw_t[];
/// end
let scene_embedded: map_t<string, gpu_texture_t>;

let _scene_uid_counter: i32 = 0;
let _scene_uid: i32;
let _scene_raw: scene_t;
let _scene_root: object_t;
let _scene_scene_parent: object_t;
let _scene_objects_traversed: i32;
let _scene_objects_count: i32;

function scene_create(format: scene_t): object_t {
	_scene_uid    = _scene_uid_counter++;
	scene_meshes  = [];
	scene_cameras = [];
	/// if arm_audio
	scene_speakers = [];
	/// end
	scene_empties = [];
	/// if arm_anim
	scene_animations = [];
	/// end
	scene_embedded   = map_create();
	_scene_root      = object_create();
	_scene_root.name = "Root";
	_scene_raw       = format;

	scene_world = data_get_world(format.name, format.world_ref);

	// Startup scene
	let scene_object: object_t = scene_add_scene(format.name, null);
	if (scene_cameras.length == 0) {
		iron_log("No camera found for scene '" + format.name + "'");
	}

	scene_camera        = scene_get_camera(format.camera_ref);
	_scene_scene_parent = scene_object;
	return scene_object;
}

function scene_remove() {
	for (let i: i32 = 0; i < scene_meshes.length; ++i) {
		let o: mesh_object_t = scene_meshes[i];
		mesh_object_remove(o);
	}
	for (let i: i32 = 0; i < scene_cameras.length; ++i) {
		let o: camera_object_t = scene_cameras[i];
		camera_object_remove(o);
	}
	/// if arm_audio
	for (let i: i32 = 0; i < scene_speakers.length; ++i) {
		let o: speaker_object_t = scene_speakers[i];
		speaker_object_remove(o);
	}
	/// end
	for (let i: i32 = 0; i < scene_empties.length; ++i) {
		let o: object_t = scene_empties[i];
		object_remove(o);
	}
	object_remove(_scene_root);
}

function scene_set_active(scene_name: string): object_t {
	if (_scene_root != null) {
		scene_remove();
	}

	let format: scene_t = data_get_scene_raw(scene_name);
	let o: object_t     = scene_create(format);
	return o;
}

function scene_render_frame() {
	if (render_path_commands == null) {
		return;
	}

	/// if arm_anim
	for (let i: i32 = 0; i < scene_animations.length; ++i) {
		let anim: anim_raw_t = scene_animations[i];
		anim_update(anim, sys_delta());
	}
	/// end
	for (let i: i32 = 0; i < scene_empties.length; ++i) {
		let e: object_t = scene_empties[i];
		if (e != null && e.parent != null) {
			transform_update(e.transform);
		}
	}

	// Render active camera
	scene_camera != null ? camera_object_render_frame(scene_camera) : render_path_render_frame();
}

// Objects
function scene_add_object(parent: object_t = null): object_t {
	let object: object_t = object_create();
	parent != null ? object_set_parent(object, parent) : object_set_parent(object, _scene_root);
	return object;
}

function scene_get_child(name: string): object_t {
	return object_get_child(_scene_root, name);
}

function scene_get_mesh(name: string): mesh_object_t {
	for (let i: i32 = 0; i < scene_meshes.length; ++i) {
		let m: mesh_object_t = scene_meshes[i];
		if (m.base.name == name) {
			return m;
		}
	}
	return null;
}

function scene_get_camera(name: string): camera_object_t {
	for (let i: i32 = 0; i < scene_cameras.length; ++i) {
		let c: camera_object_t = scene_cameras[i];
		if (c.base.name == name) {
			return c;
		}
	}
	return null;
}

/// if arm_audio
function scene_get_speaker(name: string): speaker_object_t {
	for (let i: i32 = 0; i < scene_speakers.length; ++i) {
		let s: speaker_object_t = scene_speakers[i];
		if (s.base.name == name) {
			return s;
		}
	}
	return null;
}
/// end

function scene_get_empty(name: string): object_t {
	for (let i: i32 = 0; i < scene_empties.length; ++i) {
		let e: object_t = scene_empties[i];
		if (e.name == name) {
			return e;
		}
	}
	return null;
}

function scene_add_mesh_object(data: mesh_data_t, material: material_data_t, parent: object_t = null): mesh_object_t {
	let object: mesh_object_t = mesh_object_create(data, material);
	parent != null ? object_set_parent(object.base, parent) : object_set_parent(object.base, _scene_root);
	return object;
}

function scene_add_camera_object(data: camera_data_t, parent: object_t = null): camera_object_t {
	let object: camera_object_t = camera_object_create(data);
	parent != null ? object_set_parent(object.base, parent) : object_set_parent(object.base, _scene_root);
	return object;
}

/// if arm_audio
function scene_add_speaker_object(data: speaker_data_t, parent: object_t = null): speaker_object_t {
	let object: speaker_object_t = speaker_object_create(data);
	parent != null ? object_set_parent(object.base, parent) : object_set_parent(object.base, _scene_root);
	return object;
}
/// end

function scene_traverse_objects(format: scene_t, parent: object_t, objects: obj_t[]) {
	if (objects == null) {
		return;
	}
	for (let i: i32 = 0; i < objects.length; ++i) {
		let o: obj_t = objects[i];
		if (o.spawn == false) {
			continue; // Do not auto-create Scene object
		}

		let object: object_t = scene_create_object(o, format, parent);
		scene_traverse_objects(format, object, o.children);
	}
}

function scene_add_scene(scene_name: string, parent: object_t): object_t {
	if (parent == null) {
		parent      = scene_add_object();
		parent.name = scene_name;
	}
	let format: scene_t = data_get_scene_raw(scene_name);
	scene_load_embedded_data(format.embedded_datas); // Additional scene assets
	_scene_objects_traversed = 0;
	_scene_objects_count     = scene_get_objects_count(format.objects);

	if (format.objects != null && format.objects.length > 0) {
		scene_traverse_objects(format, parent, format.objects); // Scene objects
	}
	return parent;
}

function scene_get_objects_count(objects: obj_t[]): i32 {
	if (objects == null) {
		return 0;
	}
	let result: i32 = objects.length;
	for (let i: i32 = 0; i < objects.length; ++i) {
		let o: obj_t = objects[i];
		if (o.spawn == false) {
			continue; // Do not count children of non-spawned objects
		}
		if (o.children != null) {
			result += scene_get_objects_count(o.children);
		}
	}
	return result;
}

function _scene_spawn_object_tree(obj: obj_t, parent: object_t, spawn_children: bool): object_t {
	let object: object_t = scene_create_object(obj, _scene_raw, parent);
	if (spawn_children && obj.children != null) {
		for (let i: i32 = 0; i < obj.children.length; ++i) {
			let child: obj_t = obj.children[i];
			_scene_spawn_object_tree(child, object, spawn_children);
		}
	}
	return object;
}

function scene_spawn_object(name: string, parent: object_t = null, spawn_children: bool = true): object_t {
	let obj: obj_t = scene_get_raw_object_by_name(_scene_raw, name);
	return _scene_spawn_object_tree(obj, parent, spawn_children);
}

function scene_get_raw_object_by_name(format: scene_t, name: string): obj_t {
	return scene_traverse_objs(format.objects, name);
}

function scene_traverse_objs(children: obj_t[], name: string): obj_t {
	for (let i: i32 = 0; i < children.length; ++i) {
		let o: obj_t = children[i];
		if (o.name == name) {
			return o;
		}
		if (o.children != null) {
			let res: obj_t = scene_traverse_objs(o.children, name);
			if (res != null) {
				return res;
			}
		}
	}
	return null;
}

function scene_create_object(o: obj_t, format: scene_t, parent: object_t): object_t {
	let scene_name: string = format.name;

	if (o.type == "camera_object") {
		let b: camera_data_t        = data_get_camera(scene_name, o.data_ref);
		let object: camera_object_t = scene_add_camera_object(b, parent);
		return scene_return_object(object.base, o);
	}
	else if (o.type == "mesh_object") {
		if (o.material_ref == null) {
			return scene_create_mesh_object(o, format, parent, null);
		}
		else {
			let ref: string          = o.material_ref;
			let mat: material_data_t = data_get_material(scene_name, ref);
			return scene_create_mesh_object(o, format, parent, mat);
		}
	}
	/// if arm_audio
	else if (o.type == "speaker_object") {
		let object: speaker_object_t = scene_add_speaker_object(speaker_data_get_raw_by_name(format.speaker_datas, o.data_ref), parent);
		return scene_return_object(object.base, o);
	}
	/// end
	else if (o.type == "object") {
		let object: object_t = scene_add_object(parent);
		return scene_return_object(object, o);
	}
	else {
		return null;
	}
}

function scene_create_mesh_object(o: obj_t, format: scene_t, parent: object_t, material: material_data_t): object_t {
	// Mesh reference
	let ref: string[]       = string_split(o.data_ref, "/");
	let object_file: string = "";
	let data_ref: string    = "";
	let scene_name: string  = format.name;
	if (ref.length == 2) { // File reference
		object_file = ref[0];
		data_ref    = ref[1];
	}
	else { // Local mesh data
		object_file = scene_name;
		data_ref    = o.data_ref;
	}

	return scene_return_mesh_object(object_file, data_ref, material, parent, o);
}

function scene_return_mesh_object(object_file: string, data_ref: string, material: material_data_t, parent: object_t, o: obj_t): object_t {
	let mesh: mesh_data_t     = data_get_mesh(object_file, data_ref);
	let object: mesh_object_t = scene_add_mesh_object(mesh, material, parent);
	return scene_return_object(object.base, o);
}

function scene_return_object(object: object_t, o: obj_t): object_t {
	// Load object actions
	/// if arm_anim
	if (object != null && o.anim != null && o.anim.object_actions != null) {
		let oactions: scene_t[] = [];
		while (oactions.length < o.anim.object_actions.length) {
			array_push(oactions, null);
		}

		for (let i: i32 = 0; i < o.anim.object_actions.length; ++i) {
			let ref: string = o.anim.object_actions[i];
			if (ref == "null") { // No startup action set
				continue;
			}
			let action: scene_t = data_get_scene_raw(ref);
			oactions[i]         = action;
		}
		return scene_return_object_loaded(object, o, oactions);
	}
	else {
		/// end
		return scene_return_object_loaded(object, o, null);
		/// if arm_anim
	}
	/// end
}

function scene_return_object_loaded(object: object_t, o: obj_t, oactions: scene_t[]): object_t {
	if (object != null) {
		object.raw     = o;
		object.name    = o.name;
		object.visible = o.visible;
		scene_gen_transform(o, object.transform);
		/// if arm_anim
		object_setup_animation(object, oactions);
		/// end
	}
	return object;
}

function scene_gen_transform(object: obj_t, transform: transform_t) {
	transform.world            = object.transform != null ? mat4_from_f32_array(object.transform) : mat4_identity();
	let dec: mat4_decomposed_t = mat4_decompose(transform.world);
	transform.loc              = dec.loc;
	transform.rot              = dec.rot;
	transform.scale            = dec.scl;
	if (transform.object.parent != null) {
		transform_update(transform);
	}
}

function scene_load_embedded_data(datas: string[]) {
	if (datas == null) {
		return;
	}
	for (let i: i32 = 0; i < datas.length; ++i) {
		let file: string = datas[i];
		scene_embed_data(file);
	}
}

function scene_embed_data(file: string) {
	let image: gpu_texture_t = data_get_image(file);
	map_set(scene_embedded, file, image);
}

type scene_t = {
	name?: string;
	objects?: obj_t[];
	mesh_datas?: mesh_data_t[];
	camera_datas?: camera_data_t[];
	camera_ref?: string;
	material_datas?: material_data_t[];
	shader_datas?: shader_data_t[];
	world_datas?: world_data_t[];
	world_ref?: string;
	speaker_datas?: speaker_data_t[];
	embedded_datas?: string[]; // Preload for this scene, images only for now
};

type mesh_data_t = {
	name?: string;
	scale_pos?: f32; // Unpack pos from (-1,1) coords
	scale_tex?: f32; // Unpack tex from (-1,1) coords
	vertex_arrays?: vertex_array_t[];
	index_array?: u32_array_t; // size = 3
	_?: mesh_data_runtime_t;
};

type mesh_data_runtime_t = {
	handle?: string; // Handle used to retrieve this object in Data
	vertex_buffer?: gpu_buffer_t;
	index_buffer?: gpu_buffer_t;
	structure?: gpu_vertex_structure_t;
};

type vertex_array_t = {
	attrib?: string;
	data?: string; // short4norm, short2norm
	values?: i16_array_t;
};

type camera_data_t = {
	name?: string;
	near_plane?: f32;
	far_plane?: f32;
	fov?: f32;
	aspect?: f32;
	frustum_culling?: bool;
	ortho?: f32_array_t; // Indicates ortho camera, left, right, bottom, top
};

type material_data_t = {
	name?: string;
	shader?: string;
	contexts?: material_context_t[];
	_?: material_data_runtime_t;
};

type material_data_runtime_t = {
	uid?: f32;
	shader?: shader_data_t;
};

type material_context_t = {
	name?: string;
	bind_constants?: bind_const_t[];
	bind_textures?: bind_tex_t[];
	_?: material_context_runtime_t;
};

type material_context_runtime_t = {
	textures?: gpu_texture_t[];
};

type bind_const_t = {
	name?: string;
	vec?: f32_array_t; // bool (vec[0] > 0) | i32 | f32 | vec2 | vec3 | vec4
};

type bind_tex_t = {
	name?: string;
	file?: string;
};

type shader_data_t = {
	name?: string;
	contexts?: shader_context_t[];
};

type shader_context_t = {
	name?: string;
	depth_write?: bool;
	compare_mode?: string;
	cull_mode?: string;
	vertex_shader?: string;
	fragment_shader?: string;
	shader_from_source?: bool; // Build shader at runtime using from_source()
	blend_source?: string;
	blend_destination?: string;
	alpha_blend_source?: string;
	alpha_blend_destination?: string;
	color_writes_red?: bool[]; // Per target masks
	color_writes_green?: bool[];
	color_writes_blue?: bool[];
	color_writes_alpha?: bool[];
	color_attachments?: string[]; // RGBA32, RGBA64, R8
	depth_attachment?: string;    // D32
	vertex_elements?: vertex_element_t[];
	constants?: shader_const_t[];
	texture_units?: tex_unit_t[];
	_?: shader_context_runtime_t;
};

type shader_context_runtime_t = {
	pipe?: gpu_pipeline_t;
	constants?: i32[];
	tex_units?: i32[];
	structure?: gpu_vertex_structure_t;
	vertex_shader_size?: i32;
	fragment_shader_size?: i32;
};

type vertex_element_t = {
	name?: string;
	data?: string; // "short4norm", "short2norm"
};

type shader_const_t = {
	name?: string;
	type?: string;
	link?: string;
};

type tex_unit_t = {
	name?: string;
	link?: string;
};

type speaker_data_t = {
	name?: string;
	sound?: string;
	muted?: bool;
	loop?: bool;
	stream?: bool;
	volume?: f32;
	attenuation?: f32;
	play_on_start?: bool;
};

type world_data_t = {
	name?: string;
	color?: i32;
	strength?: f32;
	irradiance?: string; // Reference to irradiance_t blob
	radiance?: string;
	radiance_mipmaps?: i32;
	envmap?: string;
	_?: world_data_runtime_t;
};

type world_data_runtime_t = {
	envmap?: gpu_texture_t;
	radiance?: gpu_texture_t;
	radiance_mipmaps?: gpu_texture_t[];
	irradiance?: f32_array_t;
};

type irradiance_t = {
	irradiance?: f32_array_t; // Blob with spherical harmonics, bands 0,1,2
};

type obj_t = {
	name?: string;
	type?: string; // object, mesh_object, camera_object, speaker_object
	data_ref?: string;
	transform?: f32_array_t;
	dimensions?: f32_array_t; // Geometry objects
	visible?: bool;
	spawn?: bool;  // Auto add object when creating scene
	anim?: anim_t; // Object animation
	material_ref?: string;
	children?: obj_t[];
	_?: obj_runtime_t;
};

type obj_runtime_t = {
	_gc?: scene_t; // Link to armpack_decode result
};

type anim_t = {
	object_actions?: string[];
	tracks?: track_t[];
	begin?: i32; // Frames, for non-sampled
	end?: i32;
	has_delta?: bool; // Delta transform
	marker_frames?: u32_array_t;
	marker_names?: string[];
};

type track_t = {
	target?: string;
	frames?: u32_array_t;
	values?: f32_array_t; // sampled - full matrix transforms, non-sampled - values
};
