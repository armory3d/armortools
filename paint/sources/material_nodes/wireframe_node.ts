
let wireframe_node_def: ui_node_t = {
    id: 0,
    name: _tr("Wireframe"),
    type: "WIREFRAME",
    x: 0,
    y: 0,
    color: 0xffb34f5a,
    inputs: [
        {
            id: 0,
            node_id: 0,
            name: _tr("Size"),
            type: "VALUE",
            color: 0xffa1a1a1,
            default_value: f32_array_create_x(0.01),
            min: 0.0,
            max: 0.1,
            precision: 100,
            display: 0
        },
    ],
    outputs: [
        {
            id: 0,
            node_id: 0,
            name: _tr("Fac"),
            type: "VALUE",
            color: 0xffa1a1a1,
            default_value: f32_array_create_x(0.0),
            min: 0.0,
            max: 1.0,
            precision: 100,
            display: 0
        }
    ],
    buttons: [
        {
            name: _tr("Pixel Size"),
            type: "BOOL",
            output: 0,
            default_value: f32_array_create_x(0),
            data: null,
            min: 0.0,
            max: 1.0,
            precision: 100,
            height: 0
        }
    ],
    width: 0,
    flags: 0
};
