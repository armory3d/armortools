
let normal_map_node_def: ui_node_t = {
    id: 0,
    name: _tr("Normal Map"),
    type: "NORMAL_MAP",
    x: 0,
    y: 0,
    color: 0xff522c99,
    inputs: [
        {
            id: 0,
            node_id: 0,
            name: _tr("Strength"),
            type: "VALUE",
            color: 0xffa1a1a1,
            default_value: f32_array_create_x(1.0),
            min: 0.0,
            max: 2.0,
            precision: 100,
            display: 0
        },
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
    buttons: [],
    width: 0,
    flags: 0
};
