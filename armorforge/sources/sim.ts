
let sim_running: bool = false;

function sim_init() {
    physics_world_create();
}

function sim_update() {

	render_path_raytrace_ready = false;

    if (sim_running) {
        if (render_path_raytrace_frame != 1) {
            return;
        }

        let world: physics_world_t = physics_world_active;
	    physics_world_update(world);

        iron_delay_idle_sleep();

        let record: bool = true;
        if (record) {
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
    let rt: render_target_t = map_get(render_path_render_targets, "taa");
    iron_mp4_begin("/home/lubos/Desktop/test.mp4", rt._image.width, rt._image.height);
}

function sim_stop() {
    sim_running = false;
    iron_mp4_end();
}

function sim_add(o: object_t, shape: physics_shape_t, mass: f32) {
    let body: physics_body_t = physics_body_create();
    body.shape = shape;
    body.mass = mass;
    physics_body_init(body, o);
}
