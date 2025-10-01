
let mix_color_node_def: ui_node_t = {
    id: 0,
    name: _tr("Mix Color"),
    type: "MIX_RGB",
    x: 0,
    y: 0,
    color: 0xff448c6d,
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
        },
        {
            id: 0,
            node_id: 0,
            name: _tr("Color 1"),
            type: "RGBA",
            color: 0xffc7c729,
            default_value: f32_array_create_xyzw(0.5, 0.5, 0.5, 1.0),
            min: 0.0,
            max: 1.0,
            precision: 100,
            display: 0
        },
        {
            id: 0,
            node_id: 0,
            name: _tr("Color 2"),
            type: "RGBA",
            color: 0xffc7c729,
            default_value: f32_array_create_xyzw(0.5, 0.5, 0.5, 1.0),
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
        }
    ],
    buttons: [
        {
            name: _tr("blend_type"),
            type: "ENUM",
            output: 0,
            default_value: f32_array_create_x(0),
            data: u8_array_create_from_string(_tr("Mix") + "\n" + _tr("Darken") + "\n" + _tr("Multiply") + "\n" + _tr("Burn") + "\n" + _tr("Lighten") + "\n" + _tr("Screen") + "\n" + _tr("Dodge") + "\n" + _tr("Add") + "\n" + _tr("Overlay") + "\n" + _tr("Soft Light") + "\n" + _tr("Linear Light") + "\n" + _tr("Difference") + "\n" + _tr("Subtract") + "\n" + _tr("Divide") + "\n" + _tr("Hue") + "\n" + _tr("Saturation") + "\n" + _tr("Color") + "\n" + _tr("Value")),
            min: 0.0,
            max: 1.0,
            precision: 100,
            height: 0
        },
        {
            name: _tr("use_clamp"),
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
