
let scene_camera: camera_object_t;
let scene_world: world_data_t;
let scene_meshes: mesh_object_t[];
let scene_cameras: camera_object_t[];
let scene_empties: object_t[];
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
	scene_empties = [];
	scene_embedded   = map_create();
	_scene_root      = object_create();
	_scene_root.name = "Root";
	_scene_raw       = format;

	scene_world = data_get_world(format.name, format.world_ref);

	// Startup scene
	let scene_object: object_t = scene_add_scene(format.name, null);
	scene_camera        = scene_cameras[0]; // format.camera_ref
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
	for (let i: i32 = 0; i < scene_empties.length; ++i) {
		let e: object_t = scene_empties[i];
		if (e != null && e.parent != null) {
			transform_update(e.transform);
		}
	}
	camera_object_render_frame(scene_camera);
}

function scene_add_object(parent: object_t = null): object_t {
	let object: object_t = object_create();
	parent != null ? object_set_parent(object, parent) : object_set_parent(object, _scene_root);
	return object;
}

function scene_get_child(name: string): object_t {
	return object_get_child(_scene_root, name);
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
	else if (o.type == "object") {
		let object: object_t = scene_add_object(parent);
		return scene_return_object(object, o);
	}
	return null;
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
	if (object != null) {
		object.raw     = o;
		object.name    = o.name;
		object.visible = o.visible;
		scene_gen_transform(o, object.transform);
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
