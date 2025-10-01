
let math2_node_def: ui_node_t = {
    id: 0,
    name: _tr("Math"),
    type: "MATH",
    x: 0,
    y: 0,
    color: 0xff62676d,
    inputs: [
        {
            id: 0,
            node_id: 0,
            name: _tr("Value"),
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
            name: _tr("Value"),
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
            name: _tr("Value"),
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
            name: _tr("operation"),
            type: "ENUM",
            output: 0,
            default_value: f32_array_create_x(0),
            data: u8_array_create_from_string(_tr("Add") + "\n" + _tr("Subtract") + "\n" + _tr("Multiply") + "\n" + _tr("Divide") + "\n" + _tr("Power") + "\n" + _tr("Logarithm") + "\n" + _tr("Square Root") + "\n" + _tr("Inverse Square Root") + "\n" + _tr("Absolute") + "\n" + _tr("Exponent") + "\n" + _tr("Minimum") + "\n" + _tr("Maximum") + "\n" + _tr("Less Than") + "\n" + _tr("Greater Than") + "\n" + _tr("Sign") + "\n" + _tr("Round") + "\n" + _tr("Floor") + "\n" + _tr("Ceil") + "\n" + _tr("Truncate") + "\n" + _tr("Fraction") + "\n" + _tr("Modulo") + "\n" + _tr("Snap") + "\n" + _tr("Ping-Pong") + "\n" + _tr("Sine") + "\n" + _tr("Cosine") + "\n" + _tr("Tangent") + "\n" + _tr("Arcsine") + "\n" + _tr("Arccosine") + "\n" + _tr("Arctangent") + "\n" + _tr("Arctan2") + "\n" + _tr("Hyperbolic Sine") + "\n" + _tr("Hyperbolic Cosine") + "\n" + _tr("Hyperbolic Tangent") + "\n" + _tr("To Radians") + "\n" + _tr("To Degrees")),
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
