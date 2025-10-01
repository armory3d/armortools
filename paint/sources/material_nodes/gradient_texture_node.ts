
let gradient_texture_node_def: ui_node_t = {
    id: 0,
    name: _tr("Gradient Texture"),
    type: "TEX_GRADIENT",
    x: 0,
    y: 0,
    color: 0xff4982a0,
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
        }
    ],
    outputs: [
        {
            id: 0,
            node_id: 0,
            name: _tr("Color"),
            type: "RGBA",
            color: 0xffc7c729,
            default_value: f32_array_create_xyzw(0.8, 0.8, 0.8, 1.0),
            min: 0.0,
            max: 1.0,
            precision: 100,
            display: 0
        },
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
        }
    ],
    buttons: [
        {
            name: _tr("gradient_type"),
            type: "ENUM",
            output: 0,
            default_value: f32_array_create_x(0),
            data: u8_array_create_from_string(_tr("Linear") + "\n" + _tr("Diagonal") + "\n" + _tr("Radial") + "\n" + _tr("Spherical")),
            min: 0.0,
            max: 1.0,
            precision: 100,
            height: 0
        }
    ],
    width: 0,
    flags: 0
};
