
let _compass_hitbox_x: object_t;
let _compass_hitbox_y: object_t;
let _compass_hitbox_z: object_t;
let _compass_hovered: object_t = null;
let _compass_hovered_last: object_t = null;

function compass_render() {
    if (!context_raw.show_compass) {
		return;
	}

    let cam: camera_object_t = scene_camera;
	let compass: mesh_object_t = scene_get_child(".Compass").ext;

	let _visible: bool = compass.base.visible;
	let _parent: object_t = compass.base.parent;
	let crot: quat_t = cam.base.transform.rot;
	let ratio: f32 = app_w() / app_h();
	let _P: mat4_t = cam.p;
	cam.p = mat4_ortho(-8 * ratio, 8 * ratio, -8, 8, -2, 2);
	compass.base.visible = true;
	compass.base.parent = cam.base;
	compass.base.transform.loc = vec4_create(7.4 * ratio, 7.0, -1);
	compass.base.transform.rot = quat_create(-crot.x, -crot.y, -crot.z, crot.w);
	compass.base.transform.scale = vec4_create(0.4, 0.4, 0.4);
	transform_build_matrix(compass.base.transform);
	compass.frustum_culling = false;
	mesh_object_render(compass, "overlay", null);

    if (_compass_hovered != null) {
        line_draw_color = _compass_hovered == _compass_hitbox_x ? 0xffff0000 :
                          _compass_hovered == _compass_hitbox_y ? 0xff00ff00 :
                                                                  0xff0000ff;
        line_draw_strength = 0.1;
        shape_draw_sphere(_compass_hovered.transform.world);
    }

    cam.p = _P;
	compass.base.visible = _visible;
	compass.base.parent = _parent;
}

function compass_init_hitbox() {
    if (_compass_hitbox_x != null) {
        return;
    }

    _compass_hitbox_x = object_create();
    _compass_hitbox_y = object_create();
    _compass_hitbox_z = object_create();

    _compass_hitbox_x.transform.scale = vec4_create(0.15, 0.15, 0.15);
    _compass_hitbox_y.transform.scale = vec4_create(0.15, 0.15, 0.15);
    _compass_hitbox_z.transform.scale = vec4_create(0.15, 0.15, 0.15);

    _compass_hitbox_x.transform.loc = vec4_create(1.5, 0, 0);
    _compass_hitbox_y.transform.loc = vec4_create(0, 1.5, 0);
    _compass_hitbox_z.transform.loc = vec4_create(0, 0, 1.5);

    let compass: object_t = scene_get_child(".Compass");
    object_set_parent(_compass_hitbox_x, compass);
    object_set_parent(_compass_hitbox_y, compass);
    object_set_parent(_compass_hitbox_z, compass);
}

function _compass_compare_quat(a: quat_t, b: quat_t): bool {
    return a.x == b.x && a.y == b.y && a.z == b.z && a.w == b.w;
}

function compass_update() {
    if (!context_raw.show_compass) {
		return;
	}

    if (_compass_hovered_last != _compass_hovered) {
        _compass_hovered_last = _compass_hovered;
        context_raw.ddirty = 2;
    }
    _compass_hovered = null;

	let x: f32 = mouse_view_x() / app_w();
	let y: f32 = mouse_view_y() / app_h();
	let hover: bool = x > 0.9 && x < 1.0 && y < 0.14 && y > 0.0;
	if (hover) {

        compass_init_hitbox();

        let ratio: f32 = app_w() / app_h();
        let _P: mat4_t = scene_camera.p;
	    scene_camera.p = mat4_ortho(-8 * ratio, 8 * ratio, -8, 8, -2, 2);

        transform_build_matrix(_compass_hitbox_x.transform);
        transform_build_matrix(_compass_hitbox_y.transform);
        transform_build_matrix(_compass_hitbox_z.transform);

        let ts: transform_t[] = [
            _compass_hitbox_x.transform,
            _compass_hitbox_y.transform,
            _compass_hitbox_z.transform
        ];

        let t: transform_t = raycast_closest_box_intersect(ts, mouse_view_x(), mouse_view_y(), scene_camera);
        if (t != null) {
            let cq: quat_t = scene_camera.base.transform.rot;

            if (t == _compass_hitbox_x.transform) {
                if (mouse_started()) {
                    // Flip between left / right
                    _compass_compare_quat(quat_from_euler(math_pi() / 2, 0, math_pi() / 2), cq) ?
                        // Left
                        viewport_set_view(-1, 0, 0, math_pi() / 2, 0, -math_pi() / 2) :
                        // Right
                        viewport_set_view(1, 0, 0, math_pi() / 2, 0, math_pi() / 2);
                }
                _compass_hovered = _compass_hitbox_x;
            }

            else if (t == _compass_hitbox_y.transform) {
                if (mouse_started()) {
                    _compass_compare_quat(quat_from_euler(math_pi() / 2, 0, math_pi()), cq) ?
                        // Front
                        viewport_set_view(0, -1, 0, math_pi() / 2, 0, 0) :
                        // Back
                        viewport_set_view(0, 1, 0, math_pi() / 2, 0, math_pi());
                }
                _compass_hovered = _compass_hitbox_y;
            }

            else {
                if (mouse_started()) {
                    _compass_compare_quat(quat_from_euler(0, 0, 0), cq) ?
                        // Bottom
                        viewport_set_view(0, 0, -1, math_pi(), 0, math_pi()) :
                        // Top
                        viewport_set_view(0, 0, 1, 0, 0, 0);
                }
                _compass_hovered = _compass_hitbox_z;
            }
        }

        scene_camera.p = _P;
	}
}
