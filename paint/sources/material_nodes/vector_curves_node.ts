
let vector_curves_node_def: ui_node_t = {
    id: 0,
    name: _tr("Vector Curves"),
    type: "CURVE_VEC",
    x: 0,
    y: 0,
    color: 0xff522c99,
    inputs: [
        {
            id: 0,
            node_id: 0,
            name: _tr("Fac"),
            type: "VALUE",
            color: 0xffa1a1a1,
            default_value: f32_array_create_x(1.0),
            min: 0.0,
            max: 1.0,
            precision: 100,
            display: 0
        },
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
    buttons: [
        {
            name: "nodes_material_vector_curves_button",
            type: "CUSTOM",
            output: 0,
            default_value: f32_array_create(96 + 3), // x - [0, 32], y - [33, 64], z - [65, 96], x_len, y_len, z_len
            data: null,
            min: 0.0,
            max: 1.0,
            precision: 100,
            height: 8.5
        }
    ],
    width: 0,
    flags: 0
};
