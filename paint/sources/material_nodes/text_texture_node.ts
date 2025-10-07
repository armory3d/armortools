
function text_texture_node_init() {
    array_push(nodes_material_texture, text_texture_node_def);
    map_set(parser_material_node_vectors, "TEX_TEXT", text_texture_node_vector);
	map_set(parser_material_node_values, "TEX_TEXT", text_texture_node_value);
}

function _parser_material_cache_tex_text_node(file: string, text: string) {
	if (map_get(data_cached_images, file) == null) {
		sys_notify_on_next_frame(function(text: string) {
			let _text_tool_text: string = context_raw.text_tool_text;
			let _text_tool_image: gpu_texture_t = context_raw.text_tool_image;
			context_raw.text_tool_text = text;
			context_raw.text_tool_image = null;

			util_render_make_text_preview();
			let file: string = "tex_text_" + text;

			// TODO: remove old cache
			map_set(data_cached_images, file, context_raw.text_tool_image);

			context_raw.text_tool_text = _text_tool_text;
			context_raw.text_tool_image = _text_tool_image;
		}, text);
	}
}

function text_texture_node_vector(node: ui_node_t, socket: ui_node_socket_t): string {
    let tex_name: string = parser_material_node_name(node);
    let text_buffer: buffer_t = node.buttons[0].default_value;
    let text: string = sys_buffer_to_string(text_buffer);
    let file: string = "tex_text_" + text;
    _parser_material_cache_tex_text_node(file, text);
    let tex: bind_tex_t = parser_material_make_bind_tex(tex_name, file);
    let texstore: string = parser_material_texture_store(node, tex, tex_name, color_space_t.AUTO);
    return texstore + ".rrr";
}

function text_texture_node_value(node: ui_node_t, socket: ui_node_socket_t): string {
    let tex_name: string = parser_material_node_name(node);
    let text_buffer: buffer_t = node.buttons[0].default_value;
    let text: string = sys_buffer_to_string(text_buffer);
    let file: string = "tex_text_" + text;
    _parser_material_cache_tex_text_node(file, text);
    let tex: bind_tex_t = parser_material_make_bind_tex(tex_name, file);
    let texstore: string = parser_material_texture_store(node, tex, tex_name, color_space_t.AUTO);
    return texstore + ".r";
}

let text_texture_node_def: ui_node_t = {
    id: 0,
    name: _tr("Text Texture"),
    type: "TEX_TEXT", // extension
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
            default_value: f32_array_create_x(1.0),
            min: 0.0,
            max: 1.0,
            precision: 100,
            display: 0
        }
    ],
    buttons: [
        {
            name: "text",
            type: "STRING",
            output: -1,
            default_value: f32_array_create_x(0), // "",
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
