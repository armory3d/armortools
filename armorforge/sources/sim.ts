
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

        {
            let rt: render_target_t = map_get(render_path_render_targets, "taa");
            let pixels: buffer_t = image_get_pixels(rt._image);
            ///if (arm_metal || arm_vulkan)
            export_arm_bgra_swap(pixels);
            ///end
            iron_mpeg_write(pixels, rt._image.width, rt._image.height);
            iron_delay_idle_sleep();
        }
    }
}

function sim_play() {
    sim_running = true;
    iron_mpeg_begin("/home/lubos/Desktop/test.mpeg");
}

function sim_stop() {
    sim_running = false;
    iron_mpeg_end();
}

function sim_add(o: object_t, shape: physics_shape_t, mass: f32) {
    let body: physics_body_t = physics_body_create();
    body.shape = shape;
    body.mass = mass;
    physics_body_init(body, o);
}
