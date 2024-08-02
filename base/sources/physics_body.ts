
///if arm_physics

type physics_body_t = {
    shape: physics_shape_t;
};

enum physics_shape_t {
    SPHERE = 0,
    MESH = 1,
}

let physics_body_object_map: map_t<any, any>;

function physics_body_create(): physics_body_t {
    return null;
}

function physics_body_init(body: physics_body_t, obj: obj_t) {

}

function physics_body_set_mass(body: physics_body_t, mass: f32) {

}

function physics_body_apply_impulse(body: physics_body_t, dir: vec4_t) {

}

function physics_body_sync_transform(body: physics_body_t) {

}

///end
