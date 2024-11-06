
type slot_layer_t = {
    id?: i32;
    texpaint: image_t;
    texpaint_nor: image_t;
    texpaint_pack: image_t;

    decal_mat: mat4_t;
    parent?: slot_layer_t;
    fill_layer?: slot_material_t;
};

function slot_layer_get_object_mask(raw: slot_layer_t): i32 {
	return 0;
}

function slot_layer_create(ext: string = "", type: layer_slot_type_t = layer_slot_type_t.LAYER, parent: slot_layer_t = null): slot_layer_t {
    return null;
}

function slot_layer_clear(raw: slot_layer_t, base_color: i32 = 0x00000000, base_image: image_t = null, occlusion: f32 = 1.0, roughness: f32 = base_default_rough, metallic: f32 = 0.0) {
}

function slot_layer_is_mask(raw: slot_layer_t): bool {
    return false;
}
