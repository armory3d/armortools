
let mapping_node_def: ui_node_t = {
    id: 0,
    name: _tr("Mapping"),
    type: "MAPPING",
    x: 0,
    y: 0,
    color: 0xff522c99,
    inputs: [
        {
            id: 0,
            node_id: 0,
            name: _tr("Vector"),
            type: "VECTOR",
            color: 0xff6363c7,
            default_value: f32_array_create_xyz(0.0, 0.0, 0.0),
            min: 0.0,
            max: 1.0,
            precision: 100,
            display: 0
        },
        {
            id: 0,
            node_id: 0,
            name: _tr("Location"),
            type: "VECTOR",
            color: 0xff6363c7,
            default_value: f32_array_create_xyz(0.0, 0.0, 0.0),
            min: 0.0,
            max: 1.0,
            precision: 100,
            display: 1
        },
        {
            id: 0,
            node_id: 0,
            name: _tr("Rotation"),
            type: "VECTOR",
            color: 0xff6363c7,
            default_value: f32_array_create_xyz(0.0, 0.0, 0.0),
            min: 0.0,
            max: 360.0,
            precision: 100,
            display: 1
        },
        {
            id: 0,
            node_id: 0,
            name: _tr("Scale"),
            type: "VECTOR",
            color: 0xff6363c7,
            default_value: f32_array_create_xyz(1.0, 1.0, 1.0),
            min: 0.0,
            max: 1.0,
            precision: 100,
            display: 1
        }
    ],
    outputs: [
        {
            id: 0,
            node_id: 0,
            name: _tr("Vector"),
            type: "VECTOR",
            color: 0xff6363c7,
            default_value: f32_array_create_xyz(0.0, 0.0, 0.0),
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
