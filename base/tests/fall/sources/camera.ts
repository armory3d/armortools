
function camera_update() {
    let camera: camera_object_t = scene_camera;
    let move_forward: bool = keyboard_down("w");
    let move_backward: bool = keyboard_down("s");
    let strafe_left: bool = keyboard_down("a");
    let strafe_right: bool = keyboard_down("d");
    let strafe_up: bool = keyboard_down("e");
    let strafe_down: bool = keyboard_down("q");
    let fast: f32 = keyboard_down("shift") ? 2.0 : 1.0;
    let speed: f32 = 5.0;
    let dir: vec4_t = vec4_create();

    if (move_forward || move_backward || strafe_right || strafe_left || strafe_up || strafe_down) {
        let clook: vec4_t = camera_object_look(camera);
        let cright: vec4_t = camera_object_right(camera);

        if (move_forward) {
            dir = vec4_fadd(dir, clook.x, clook.y, clook.z);
        }
        if (move_backward) {
            dir = vec4_fadd(dir, -clook.x, -clook.y, -clook.z);
        }
        if (strafe_right) {
            dir = vec4_fadd(dir, cright.x, cright.y, cright.z);
        }
        if (strafe_left) {
            dir = vec4_fadd(dir, -cright.x, -cright.y, -cright.z);
        }
        if (strafe_up) {
            dir = vec4_fadd(dir, 0, 0, 1);
        }
        if (strafe_down) {
            dir = vec4_fadd(dir, 0, 0, -1);
        }
    }

    let d: f32 = sys_delta() * speed * fast;
    if (d > 0.0) {
        transform_move(camera.base.transform, dir, d);
    }

    if (mouse_down()) {
        transform_rotate(camera.base.transform, vec4_z_axis(), -mouse_movement_x / 200);
        transform_rotate(camera.base.transform, camera_object_right(camera), -mouse_movement_y / 200);
    }
}
