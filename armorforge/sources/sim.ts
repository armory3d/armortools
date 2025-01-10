
let sim_running: bool = false;
let sim_transforms: mat4_box_t[];
let sim_object_script_map: map_t<object_t, string> = map_create();
let sim_record: bool = false;

function sim_init() {
    physics_world_create();
}

function sim_update() {

	render_path_raytrace_ready = false;

    if (sim_running) {
        // if (render_path_raytrace_frame != 1) {
            // return;
        // }

        let objects: object_t[] = map_keys(sim_object_script_map);
        for (let i: i32 = 0; i < objects.length; ++i) {
            let o: object_t = objects[i];
            let s: string = map_get(sim_object_script_map, o);
            let addr: string = i64_to_string((i64)(o.transform));
            s = "{let transform=" + addr + ";" + s + "}";
            js_eval(s);
        }

        let world: physics_world_t = physics_world_active;
	    physics_world_update(world);

        iron_delay_idle_sleep();

        if (sim_record) {
            let rt: render_target_t = map_get(render_path_render_targets, "taa");
            let pixels: buffer_t = image_get_pixels(rt._image);
            ///if (arm_metal || arm_vulkan)
            export_arm_bgra_swap(pixels);
            ///end
            iron_mp4_encode(pixels);
        }
    }
}

function sim_play() {
    sim_running = true;

    if (sim_record) {
        if (project_filepath == "") {
            console_error(tr("Save project first"));
            sim_record = false;
            return;
        }
        let path: string = path_base_dir(project_filepath) + "/output.mp4";
        let rt: render_target_t = map_get(render_path_render_targets, "taa");
        iron_mp4_begin(path, rt._image.width, rt._image.height);
    }

    // Save transforms
    sim_transforms = [];
    let pos: mesh_object_t[] = project_paint_objects;
    for (let i: i32 = 0; i < pos.length; ++i) {
        let m: mat4_box_t = { v: pos[i].base.transform.local };
        array_push(sim_transforms, m);
    }
}

function sim_stop() {
    sim_running = false;

    if (sim_record) {
        iron_mp4_end();
    }

    // Restore transforms
    let pos: mesh_object_t[] = project_paint_objects;
    for (let i: i32 = 0; i < pos.length; ++i) {
        transform_set_matrix(pos[i].base.transform, sim_transforms[i].v);

        let pb: physics_body_t = map_get(physics_body_object_map, pos[i].base.uid);
        if (pb != null) {
            physics_body_sync_transform(pb);
        }
    }
}

function sim_add_body(o: object_t, shape: physics_shape_t, mass: f32) {
    let body: physics_body_t = physics_body_create();
    body.shape = shape;
    body.mass = mass;
    physics_body_init(body, o);
}

function sim_remove_body(uid: i32) {
    physics_body_remove(uid);
}

function sim_duplicate() {
    // Mesh
    let so: mesh_object_t = context_raw.selected_object.ext;
    let dup: mesh_object_t = scene_add_mesh_object(so.data, so.materials, so.base.parent);
    transform_set_matrix(dup.base.transform, so.base.transform.local);
    array_push(project_paint_objects, dup);
    dup.base.name = so.base.name;

    // Physics
    let pb: physics_body_t = map_get(physics_body_object_map, so.base.uid);
    if (pb != null) {
        let pbdup: physics_body_t = physics_body_create();
        pbdup.shape = pb.shape;
        pbdup.mass = pb.mass;
        physics_body_init(pbdup, dup.base);
    }

    _tab_scene_paint_object_length++;
    tab_scene_sort();
}

function sim_delete() {
    let so: mesh_object_t = context_raw.selected_object.ext;
    array_remove(project_paint_objects, so);
    mesh_object_remove(so);
    sim_remove_body(so.base.uid);
    _tab_scene_paint_object_length--;
    tab_scene_sort();
}
