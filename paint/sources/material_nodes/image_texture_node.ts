
enum color_space_t {
	AUTO, // sRGB for base color, otherwise linear
	LINEAR,
	SRGB,
	DIRECTX_NORMAL_MAP,
}

function parser_material_texture_store(node: ui_node_t, tex: bind_tex_t, tex_name: string, color_space: i32): string {
	array_push(parser_material_matcon.bind_textures, tex);
	node_shader_context_add_elem(parser_material_kong.context, "tex", "short2norm");
	node_shader_add_texture(parser_material_kong, "" + tex_name);
	let uv_name: string = "";
	if (parser_material_get_input_link(node.inputs[0]) != null) {
		uv_name = parser_material_parse_vector_input(node.inputs[0]);
	}
	else {
		uv_name = parser_material_tex_coord;
	}
	let tex_store: string = parser_material_store_var_name(node);

	if (parser_material_sample_keep_aspect) {
		node_shader_add_constant(parser_material_kong, tex_name + "_size: float2", "_size(" + tex_name + ")");
		parser_material_write(parser_material_kong, "var " + tex_store + "_size: float2 = constants." + tex_name + "_size;");
		parser_material_write(parser_material_kong, "var " + tex_store + "_ax: float = " + tex_store + "_size.x / " + tex_store + "_size.y;");
		parser_material_write(parser_material_kong, "var " + tex_store + "_ay: float = " + tex_store + "_size.y / " + tex_store + "_size.x;");
		parser_material_write(parser_material_kong, "var " + tex_store + "_uv: float2 = ((" + uv_name + ".xy / float(" + parser_material_sample_uv_scale + ") - float2(0.5, 0.5)) * float2(max(" + tex_store + "_ay, 1.0), max(" + tex_store + "_ax, 1.0))) + float2(0.5, 0.5);");
		parser_material_write(parser_material_kong, "if (" + tex_store + "_uv.x < 0.0 || " + tex_store + "_uv.y < 0.0 || " + tex_store + "_uv.x > 1.0 || " + tex_store + "_uv.y > 1.0) { discard; }");
		parser_material_write(parser_material_kong, tex_store + "_uv = " + tex_store + "_uv * float(" + parser_material_sample_uv_scale + ");");
		uv_name = tex_store + "_uv";
	}

	if (parser_material_triplanar) {
		parser_material_write(parser_material_kong, "var " + tex_store + ": float4 = float4(0.0, 0.0, 0.0, 0.0);");
		parser_material_write(parser_material_kong, "if (tex_coord_blend.x > 0.0) {" + tex_store + " += sample(" + tex_name + ", sampler_linear, " + uv_name + ".xy) * tex_coord_blend.x; }");
		parser_material_write(parser_material_kong, "if (tex_coord_blend.y > 0.0) {" + tex_store + " += sample(" + tex_name + ", sampler_linear, " + uv_name + "1.xy) * tex_coord_blend.y; }");
		parser_material_write(parser_material_kong, "if (tex_coord_blend.z > 0.0) {" + tex_store + " += sample(" + tex_name + ", sampler_linear, " + uv_name + "2.xy) * tex_coord_blend.z; }");
	}
	else {
		if (parser_material_is_frag) {
			map_set(parser_material_texture_map, tex_store, "sample(" + tex_name + ", sampler_linear, " + uv_name + ".xy)");
			parser_material_write(parser_material_kong, "var " + tex_store + ": float4 = sample(" + tex_name + ", sampler_linear, " + uv_name + ".xy);");
		}
		else {
			map_set(parser_material_texture_map, tex_store, "sample_lod(" + tex_name + ", sampler_linear, " + uv_name + ".xy, 0.0)");
			parser_material_write(parser_material_kong, "var " + tex_store + ": float4 = sample_lod(" + tex_name + ", sampler_linear, " + uv_name + ".xy, 0.0);");
		}
		if (!ends_with(tex.file, ".jpg")) { // Pre-mult alpha
			parser_material_write(parser_material_kong, tex_store + ".rgb = " + tex_store + ".rgb * " + tex_store + ".a;");
		}
	}

	if (parser_material_transform_color_space) {
		// Base color socket auto-converts from sRGB to linear
		if (color_space == color_space_t.LINEAR && parser_material_parsing_basecolor) { // Linear to sRGB
			parser_material_write(parser_material_kong, tex_store + ".rgb = pow3(" + tex_store + ".rgb, float3(2.2, 2.2, 2.2));");
		}
		else if (color_space == color_space_t.SRGB && !parser_material_parsing_basecolor) { // sRGB to linear
			parser_material_write(parser_material_kong, tex_store + ".rgb = pow3(" + tex_store + ".rgb, float3(1.0 / 2.2, 1.0 / 2.2, 1.0 / 2.2));");
		}
		else if (color_space == color_space_t.DIRECTX_NORMAL_MAP) { // DirectX normal map to OpenGL normal map
			parser_material_write(parser_material_kong, tex_store + ".y = 1.0 - " + tex_store + ".y;");
		}
	}
	return tex_store;
}

function parser_material_make_texture(image_node: ui_node_t, tex_name: string): bind_tex_t {
	let i: i32 = image_node.buttons[0].default_value[0];
	if (i > 9000) { // 9999 - Texture deleted, use pink now
		return null;
	}
	let filepath: string = parser_material_enum_data(base_enum_texts(image_node.type)[i]);
	if (filepath == "" || string_index_of(filepath, ".") == -1) {
		return null;
	}

	return parser_material_make_bind_tex(tex_name, filepath);
}

function image_texture_node_vector(node: ui_node_t, socket: ui_node_socket_t): string {
    // Already fetched
    if (array_index_of(parser_material_parsed, parser_material_res_var_name(node, node.outputs[1])) >= 0) { // TODO: node.outputs[0]
        let varname: string = parser_material_store_var_name(node);
        return varname + ".rgb";
    }
    let tex_name: string = parser_material_node_name(node);
    let tex: bind_tex_t = parser_material_make_texture(node, tex_name);
    if (tex != null) {
        let color_space: i32 = node.buttons[1].default_value[0];
        let texstore: string = parser_material_texture_store(node, tex, tex_name, color_space);
        return texstore + ".rgb";
    }
    else {
        let tex_store: string = parser_material_store_var_name(node); // Pink color for missing texture
        parser_material_write(parser_material_kong, "var " + tex_store + ": float4 = float4(1.0, 0.0, 1.0, 1.0);");
        return tex_store + ".rgb";
    }
}

function image_texture_node_value(node: ui_node_t, socket: ui_node_socket_t): string {
    // Already fetched
    if (array_index_of(parser_material_parsed, parser_material_res_var_name(node, node.outputs[0])) >= 0) { // TODO: node.outputs[1]
        let varname: string = parser_material_store_var_name(node);
        return varname + ".a";
    }
    let tex_name: string = parser_material_node_name(node);
    let tex: bind_tex_t = parser_material_make_texture(node, tex_name);
    if (tex != null) {
        let color_space: i32 = node.buttons[1].default_value[0];
        let texstore: string = parser_material_texture_store(node, tex, tex_name, color_space);
        return texstore + ".a";
    }
    return "0.0";
}

let image_texture_node_def: ui_node_t = {
    id: 0,
    name: _tr("Image Texture"),
    type: "TEX_IMAGE",
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
            name: _tr("File"),
            type: "ENUM",
            output: -1,
            default_value: f32_array_create_x(0),
            data: u8_array_create_from_string(""),
            min: 0.0,
            max: 1.0,
            precision: 100,
            height: 0
        },
        {
            name: _tr("Color Space"),
            type: "ENUM",
            output: -1,
            default_value: f32_array_create_x(0),
            data: u8_array_create_from_string(_tr("Auto") + "\n" + _tr("Linear") + "\n" + _tr("sRGB") + "\n" + _tr("DirectX Normal Map")),
            min: 0.0,
            max: 1.0,
            precision: 100,
            height: 0
        }
    ],
    width: 0,
    flags: 0
};
