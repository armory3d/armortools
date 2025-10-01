
let group_node_def: ui_node_t = {
    id: 0,
    name: _tr("New Group"),
    type: "GROUP",
    x: 0,
    y: 0,
    color: 0xffb34f5a,
    inputs: [],
    outputs: [],
    buttons: [
        {
            name: "nodes_material_new_group_button",
            type: "CUSTOM",
            output: -1,
            default_value: f32_array_create_x(0),
            data: null,
            min: 0.0,
            max: 1.0,
            precision: 100,
            height: 1
        }
    ],
    width: 0,
    flags: 0
};
