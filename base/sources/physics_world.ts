
///if arm_physics

type physics_world_t = {
    empty: i32;
};

type physics_pair_t = {
    pos_a: vec4_t;
};

let physics_world_active: physics_world_t;

function physics_world_create() {

}

function physics_world_update(world: physics_world_t) {

}

function physics_world_get_contact_pairs(world: physics_world_t, body: physics_body_t): physics_pair_t[] {
    return null;
}

///end
