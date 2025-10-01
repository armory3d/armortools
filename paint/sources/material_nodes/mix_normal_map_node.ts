
let mix_normal_map_node_def: ui_node_t = {
    id: 0,
    name: _tr("Mix Normal Map"),
    type: "MIX_NORMAL_MAP", // extension
    x: 0,
    y: 0,
    color: 0xff522c99,
    inputs: [
        {
            id: 0,
            node_id: 0,
            name: _tr("Normal Map 1"),
            type: "VECTOR",
            color: -10238109,
            default_value: f32_array_create_xyz(0.5, 0.5, 1.0),
            min: 0.0,
            max: 1.0,
            precision: 100,
            display: 0
        },
        {
            id: 0,
            node_id: 0,
            name: _tr("Normal Map 2"),
            type: "VECTOR",
            color: -10238109,
            default_value: f32_array_create_xyz(0.5, 0.5, 1.0),
            min: 0.0,
            max: 1.0,
            precision: 100,
            display: 0
        }
    ],
    outputs: [
        {
            id: 0,
            node_id: 0,
            name: _tr("Normal Map"),
            type: "VECTOR",
            color: -10238109,
            default_value: f32_array_create_xyz(0.5, 0.5, 1.0),
            min: 0.0,
            max: 1.0,
            precision: 100,
            display: 0
        }
    ],
    buttons: [
        {
            name: _tr("blend_type"),
            type: "ENUM",
            output: 0,
            default_value: f32_array_create_x(0),
            data: u8_array_create_from_string(_tr("Partial Derivative") + "\n" + _tr("Whiteout") + "\n" + _tr("Reoriented")),
            min: 0.0,
            max: 1.0,
            precision: 100,
            height: 0
        }
    ],
    width: 0,
    flags: 0
};
