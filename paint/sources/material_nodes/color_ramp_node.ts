
let color_ramp_node_def: ui_node_t = {
    id: 0,
    name: _tr("Color Ramp"),
    type: "VALTORGB",
    x: 0,
    y: 0,
    color: 0xff62676d,
    inputs: [
        {
            id: 0,
            node_id: 0,
            name: _tr("Fac"),
            type: "VALUE",
            color: 0xffa1a1a1,
            default_value: f32_array_create_x(0.5),
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
            default_value: f32_array_create_xyzw(0.0, 0.0, 0.0, 1.0),
            min: 0.0,
            max: 1.0,
            precision: 100,
            display: 0
        },
        {
            id: 0,
            node_id: 0,
            name: _tr("Alpha"),
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
            name: "nodes_material_color_ramp_button",
            type: "CUSTOM",
            output: 0,
            default_value: f32_array_create_xyzwv(1.0, 1.0, 1.0, 1.0, 0.0),
            data: u8_array_create(1),
            min: 0.0,
            max: 1.0,
            precision: 100,
            height: 4.5
        }
    ],
    width: 0,
    flags: 0
};
