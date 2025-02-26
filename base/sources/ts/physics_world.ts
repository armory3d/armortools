
///if arm_physics

///include <phys_jolt.h>

declare function jolt_world_create(): void;
declare function jolt_world_update(): void;
declare function jolt_world_get_contact_pairs(): physics_pair_t; // []
declare function jolt_world_destroy(): void;
declare type physics_pair_t = {
    pos_a_x: f32;
    pos_a_y: f32;
    pos_a_z: f32;
};

type physics_world_t = {
    empty?: i32;
};

let physics_world_active: physics_world_t;

function physics_world_create(): physics_world_t {
    jolt_world_create();
    let world: physics_world_t = {};
    physics_world_active = world;
    return world;
}

function physics_world_update(world: physics_world_t) {
    jolt_world_update();

    let keys: i32[] = imap_keys(physics_body_object_map);
	for (let i: i32 = 0; i < keys.length; ++i) {
		let body: physics_body_t = map_get(physics_body_object_map, keys[i]);
		physics_body_update(body);
	}
}

function physics_world_get_contact_pairs(world: physics_world_t, body: physics_body_t): physics_pair_t[] {
    let pairs: physics_pair_t[] = [];
    let p: physics_pair_t = jolt_world_get_contact_pairs();
    array_push(pairs, p);
    return pairs;
}

function physics_world_destroy() {
    if (physics_world_active != null) {
        jolt_world_destroy();
    }
}

///end
